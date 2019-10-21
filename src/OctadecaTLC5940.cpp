#include "OctadecaTLC5940.h"
/* Interrupt Service Routine for FTM1 this is called whenever the BLANK and XLAT combo
 * needs to be pulsed. So at this time we turn off all outputs, and previous clocked
 * in data gets latched in. We must now clock in new data to be latched with the next
 * interrupt. So make sure this interrupt finishes before the next triggers. */
OctadecaTLC5940* OctadecaTLC5940::me;
ISR(TIMER1_OVF_vect) {
  OctadecaTLC5940::me->multiplex();
}
// Initialize all TLC, start the timers and set the static me to enable multiplexing.
OctadecaTLC5940::OctadecaTLC5940() {
  me=this;
}
/* VPRG=GND and DCPRG=VCC in my design, this sets the operating mode in GSPWM mode using
 * the DC-Register, thus never using the EEPROM values. The content of the DC-Register
 * and GS-Register is undefined on starting up, and the VPRG and DCPRG are hardware tied,
 * but we can use a trick to reset the DC-Register by writing 192 bits of dummy data for
 * each TCL5940 and 1 bit extra and than latch it to overflow the input shift register.
 *
 * Note: I assume this trick resets both the DC and GS-Register, but the manual is
 * unclear (8.4.1 Data sheet)
 *
 * First set the BLANK high to prevent turning on any leds when we switch the pin to
 * output mode. */
void OctadecaTLC5940::begin() {
  // Set BLANK high to disable all outputs, there should also be a 10K pull up on this pin.
  digitalWrite(BLANK, HIGH);
  pinMode(BLANK, OUTPUT);
  digitalWrite(BLANK, HIGH);
  // Set layerpin high to disable the layer. There is a 1K pull-up on each pin.
  for (int i = 0; i < Y_LAYERS; i++) {
	DDRC|=m_layerPin[i];
	PORTC|=m_layerPin[i];
  }
  delayMicroseconds(1);
  pinMode(XLAT, OUTPUT);
  pinMode(GSCLK, OUTPUT);
  pinMode(SIN, OUTPUT);
  pinMode(SCLK, OUTPUT);
  digitalWrite(XLAT, LOW);
  digitalWrite(GSCLK, LOW);
  digitalWrite(SIN, LOW);
  digitalWrite(SCLK, LOW);
  delayMicroseconds(1);
  
  // Overflow the GS Register and latch this to reset DC-Register and GS-Register.
  for(int i = 0; i < CHNBITS + 1; i++) {
    digitalWrite(SCLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(SCLK, LOW);
    delayMicroseconds(1);
  }
  digitalWrite(XLAT, HIGH);
  delayMicroseconds(1);
  digitalWrite(XLAT, LOW);
  delayMicroseconds(1);
  
  // Set All outputs off for the first BLANK, XLAT interrupt
  for(int i = 0; i < CHNBITS; i++) {
    digitalWrite(SCLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(SCLK, LOW);
    delayMicroseconds(1);
  }

  // Normal port operation no pin connected
  TCCR1A = 0;
  // PWM phase and frequency correct TOP=ICR1, Update OCR1x at BOTTOM, TOV1 flag set on BOTTOM
  TCCR1B = 1<<WGM13;
  // Output compare registers
  OCR1A = 1;
  OCR1B = 2;
  // Input Capture Register
  ICR1 = 8192;

  // clear OCR2B on compare match (non-inverting, fast PWM mode)
  TCCR2A = 1<<WGM21 | 1 <<WGM20 | 1<<COM2B1;
  // Toggle OCR2A on compare match (fast PWM mode)
  TCCR2B = 1<<WGM22;
  // Produces 4Mhz pulse signal (250ns periode) on output pin
  OCR2A = 3;
  OCR2B = 0;

  // Enables the Timer1 Overflow interrupt
  TIFR1 |= 1<<TOV1;
  TIMSK1 = 1<<TOIE1;
  // no prescale start timer1 and timer2
  TCCR1B |= 1<<CS10;
  TCCR2B |= 1<<CS20;
  
  // double speed (f_osc / 2)
  SPSR = 1<<SPI2X;
  // enable SPI Master mode
  SPCR = 1<<SPE | 1<<MSTR;
}

/* Sets the next frame ready flag when the display switches from top layer to bottom
 * layer the display and rendering buffers get switched and an empty rendering cube is
 * prepared for a new animation frame. The nextFrameReady is set to false and the
 * animation routines can continue rendering on the empty cube. */
void OctadecaTLC5940::update() {
  m_nextFrameReady = true;
  while(m_nextFrameReady);
}

/* Sets a voxel in the rendering cube so there will be no visual anomalies. */
void OctadecaTLC5940::setVoxel(int x, int y, int z, Color c) {
  m_rgbCube[m_renderingCube][x][y][z] = c;
}

/* Gets a voxel from the displayed cube, so animations can use the current display */
Color OctadecaTLC5940::getDisplayedVoxel(int x, int y, int z) {
  return m_rgbCube[m_displayedCube][x][y][z];
}

/* Gets a voxel from the rendering cube, so animations don't need a buffer */
Color OctadecaTLC5940::getRenderingVoxel(int x, int y, int z) {
  return m_rgbCube[m_renderingCube][x][y][z];
}

// Multiplex only uses port manipulation, this allows the fastest possible timing on
// switching pins. Calculations for next cycle are done in advance after this.
void OctadecaTLC5940::multiplex() {
  // Turn off all outputs before doing anything else
  PORTB|=B00000100;         // digitalWrite(BLANK, HIGH);
  // Set XLAT to signal new data is available
  PORTB|=B00000010;         // digitalWrite(XLAT, HIGH);
  // Set nextLayer Pin LOW to turn on output all others HIGH
  PORTC=~m_nextLayerPin;
  // clear XLAT inside the BLANK signal
  PORTB&=B11111101;         // digitalWrite(XLAT, LOW);
  // Turn on all outputs, new data is latched in
  PORTB&=B11111011;         // digitalWrite(BLANK, LOW);

  // Determine the next current and next layer
  if (++m_LayerOffset == Y_LAYERS) {
	  m_LayerOffset = 0;
  }
  m_currentLayerPin = m_layerPin[m_LayerOffset];
  m_nextLayerPin    = m_layerPin[(m_LayerOffset+1)%Y_LAYERS];

  // Prepare the NEXT layer, this will be send out NEXT multiplex refresh cycle.
  if(m_LayerOffset==Y_LAYERS-1) {
    // If the next animation frame is ready, swap the rendering and displayed cube
    if(m_nextFrameReady) {
	  m_displayedCube = 1-m_displayedCube;
	  m_renderingCube = 1-m_renderingCube;
      // clear rendering canvas for the animation routines to paint on
	  memset(m_rgbCube[m_renderingCube], 0, sizeof(m_rgbCube[0]));
	  // Reset the nextFrameReady flag until a next frame is ready.
      m_nextFrameReady = false;
    }
    setChannelBuffer(0);
  } else {
    setChannelBuffer(m_LayerOffset + 1);
  }
  // Send out the color buffer, using DMA, so this takes no processor time.
  sendChannelBuffer();
}

// Prepares the color buffer to be send to the TLC's
void OctadecaTLC5940::setChannelBuffer(int y) {
  uint8_t *chn;
  for (int x = 0; x < X_LAYERS; x++) {
    for (int z = 0; z < Z_LAYERS; z++) {
      Color c = m_rgbCube[m_displayedCube][x][y][z];
      // Flip axis of the cube, otherwise (0,0,0) would be at the back of the cube
      chn = m_ledChannel[Z_LAYERS-1-z][x];
      setChannel(*chn++, c.R);
      setChannel(*chn++, c.G);
      setChannel(*chn,   c.B);
    }
  }
}

// Find the right bytes in the channelBuffer and set the right bits to include the color
void OctadecaTLC5940::setChannel(uint16_t channel, uint16_t color) {
  uint8_t *index12p = m_channelBuffer + (((CHANNELS - channel - 1) * 3) >> 1);
  if (channel & 0x01) {
    *(index12p++) = color >> 4;
    *index12p = ((uint8_t)(color << 4)) | (*index12p & 0xF);
  } else {
    *index12p = (*index12p & 0xF0) | (color >> 8);
    *(++index12p) = color & 0xFF;
  }
}

/* Sends the channel buffer for the next layer to be displayed */
void OctadecaTLC5940::sendChannelBuffer() {
  for(int i=0;i<CHNBYTES;i++) {
	SPDR = m_channelBuffer[i];    // starts transmission
	while (!(SPSR & 1<<SPIF));    // wait for transmission complete
  }
}

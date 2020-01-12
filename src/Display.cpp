#include <Arduino.h>
#include "Display.h"
#include "Math8.h"
/*---------------------------------------------------------------------------------------
 * Led cube Cartesian coordinate system (x,y,z) and visible area
 *---------------------------------------------------------------------------------------
 *
 *                                      + + + + + + + + +
 *                                     ++              ++
 *                                    + +             + +
 *                                   +  +            +  +
 *                            Y     +   +           +   +     Z
 *                            |    +    +          +    +    /
 *                            |   +     +         +     +   /
 *                            |  +      +        +      +  /
 *                            8 + + + + + + + + +       + 8
 *                            7 +      +        +      + 7
 *                            6 +     +         +     + 6
 *                            5 +    +          +    + 5
 *                            4 +   +           +   + 4
 *                            3 +  +            +  + 3
 *                            2 + +             + + 2
 *                            1 ++              ++ 1
 *                            0 + + + + + + + + + 0
 *                              0 1 2 3 4 5 6 7 8---X
 *
 *       Small Cube (3x3x3):    Medium Cube (3x6x3):    Large Cube (9x9x9):
 *         width  x = {0,1,2}     x = {0,1,2}             x = {0,1,2,3,4,5,6,7,8}
 *         height y = {0,1,2}     y = {0,1,2,3,4,5}       y = {0,1,2,3,4,5,6,7,8}
 *         depth  z = {0,1,2}     z = {0,1,2}             z = {0,1,2,3,4,5,6,7,8}
 *-------------------------------------------------------------------------------------*/
/* Interrupt Service Routine this is called whenever the BLANK and XLAT combo
 * needs to be pulsed. So at this time we turn off all outputs, and previous clocked
 * in data gets latched in. We must now clock in new data to be latched with the next
 * interrupt. So make sure this interrupt finishes before the next triggers. */
ISR(TIMER1_OVF_vect) {
  Display::instance().multiplex();
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
void Display::begin() {
  // Set BLANK high to disable all outputs, there should also be a 10K pull up on this pin.
  digitalWrite(BLANK, HIGH);
  pinMode(BLANK, OUTPUT);
  digitalWrite(BLANK, HIGH);
  // Set layerpin high to disable the layer. There is a 1K pull-up on each pin.
  for (uint8_t i = 0; i < height; i++) {
	  DDRC|=m_layerPin[i];
	  PORTC|=m_layerPin[i];
  }
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
  for(uint16_t i = 0; i < CHNBITS + 1; i++) {
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
  for(uint16_t i = 0; i < CHNBITS; i++) {
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
void Display::update() {
  m_nextFrameReady = true;
  while(m_nextFrameReady);
}

// Multiplex only uses port manipulation, this allows the fastest possible timing on
// switching pins. Calculations for next cycle are done in advance after this.
void Display::multiplex() {
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

  // Determine the next current and next next layer
  if (++m_LayerOffset == height) {
	  m_LayerOffset = 0;
  }
  m_currentLayerPin = m_layerPin[m_LayerOffset];
  m_nextLayerPin    = m_layerPin[(m_LayerOffset+1)%height];

  // Prepare the NEXT layer, this will be send out NEXT multiplex refresh cycle.
  if(m_LayerOffset==height-1) {
    // If the next animation frame is ready, swap the rendering and displayed cube
    if(m_nextFrameReady) {
	    m_displayedCube = 1-m_displayedCube;
	    m_renderingCube = 1-m_renderingCube;
      // clear rendering canvas for the animation routines to paint on
	    memset(m_rgbCube[m_renderingCube], 0, sizeof(m_rgbCube[0]));
	    // Reset the nextFrameReady flag until a next frame is ready.
      m_nextFrameReady = false;
      // Set the buffered globalBrightness to the displayedBrightness
      displayedBrightness = globalBrightness;
    }
    setChannelBuffer(0);
  } else {
    setChannelBuffer(m_LayerOffset + 1);
  }
  // Send out the color buffer, using DMA, so this takes no processor time.
  sendChannelBuffer();
}

// Prepares the color buffer to be send to the TLC's
void Display::setChannelBuffer(const uint8_t y) {
  for (uint8_t x = 0; x < width; x++) {
    for (uint8_t z = 0; z < depth; z++) {
      Color& c = m_rgbCube[m_displayedCube][x][y][z];
      uint8_t *chn = m_ledChannel[z][x];
      setChannel(*chn++, map8(displayedBrightness, c.red));
      setChannel(*chn++, map8(displayedBrightness, c.green));
      setChannel(*chn,   map8(displayedBrightness, c.blue));
    }
  }
}

// Find the right bytes in the channelBuffer and set the right bits to include the color
void Display::setChannel(const uint16_t channel, const uint8_t data) {
  uint8_t *index12p = m_channelBuffer + (((CHANNELS - channel - 1) * 3) >> 1);
  if (channel & 0x01) {
    *(index12p++) = data;
    *index12p &= 0x0F;
  } else {
    *index12p = (*index12p & 0xF0) | (data >> 4);
    *(++index12p) = data << 4;
  }
}

/* Sends the channel buffer for the next layer to be displayed */
void Display::sendChannelBuffer() {
  for(uint16_t i=0;i<CHNBYTES;i++) {
	  SPDR = m_channelBuffer[i];    // starts transmission
	  while (!(SPSR & 1<<SPIF));    // wait for transmission complete
  }
}
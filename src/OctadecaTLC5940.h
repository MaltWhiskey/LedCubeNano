#ifndef OCTADECATLC5940_H
#define OCTADECATLC5940_H
/*
    Pin configuration of the TLC5940, 28-Pin PDIP top view

          ----O----
    OUT1 |1      28| OUT0
    OUT2 |2      27| VPRG  -> GND
    OUT3 |3      26| SIN   <-
    OUT4 |4   T  25| SCLK  <-
      .  |5   L  24| XLAT  <-
      .  |6   C  23| BLANK -> 10K Resistor  -> VCC
      .  |7   5  22| GND
      .  |8   9  21| VCC   -> 104 Capacitor -> GND
      .  |9   4  20| IREF  ->  2K Resistor  -> GND
      .  |10  0  19| DCPRG -> VCC
      .  |11     18| GSCLK <-
      .  |12     17| SOUT  -> SIN next TLC
      .  |13     16| XERR
    OUT14|14     15| OUT15
          ---------

    OUT15 MSB  ....  OUT0 LSB
 */

#include <Arduino.h>
#include <stdint.h>
#include <math.h>
#include "Color.h"

// Definition of the hardware for using 2xTLC5940
#define CHANNELS    16 * 2
#define CHNBITS     CHANNELS * 12
#define CHNBYTES    CHNBITS / 8
#define SPISPEED    5000000

// Definition of the hardware layers
#define X_LAYERS	3
#define Y_LAYERS	6
#define Z_LAYERS	3

/* These are the output pins inputing to the TLC5940. XLAT, BLANK and GSCLK are generated
 * by hardware timers. If you change these pins you also need to change the timer setup
 * and pin configuration, look for the pin alternate functions in the K64 manual, K64
 * signal multiplexing and pin Assignments chapter 10.3.1. */
#define XLAT    9
#define BLANK   10
#define GSCLK   3

/* Running 18 TLC'S sending data through bit banging will take a lot of time, so I'll
 * use SPI over DMA instead. The SPI library uses SCK on pin 13, MOSI on pin 11 and MISO
 * on pin 12. We only need the data and clock pin. The pin's can be changed, see library
 * documentation on how to do that. */
#define SIN     11
#define SCLK    13

class OctadecaTLC5940 {
private:
  /* The memory of the entire cube, double buffered */
  Color m_rgbCube[2][X_LAYERS][Y_LAYERS][Z_LAYERS];
  /* One buffer is currently being used for display and the other buffer is the canvas
   * for rendering. These buffers will swap places after a call to update, this is when
   * a new frame is ready to be displayed and right before the bottom layer is about to
   * be turned on (top to bottom vertical blank). While the top layer is turned on the
   * data for the bottom layer (y=0) is being send in. */
  volatile int m_displayedCube = 0;
  volatile int m_renderingCube = 1;
  /* When the animation routines have a frame ready update is called and a buffer switch
   * will be done right before the bottom layer data is being send in */
  volatile bool m_nextFrameReady = false;
  /* Hardware LED address mapping for getting the right LED offset see PCB schematic.
   * The table has the following layout: LED0B, LED0G, LED0R, LED1B, ..., LED81G, LED81R
   * There are some holes in this table, some are spare led addresses, others are unused.
   * Every led has a common anode and 3 cathodes and every cathode has a separate address
   *
   * There are [9] rows of LEDs with [9] LEDs in each row and every led has [3] colors.*/
  uint8_t m_ledChannel[X_LAYERS][Z_LAYERS][3] = {
  {{  3,   4,   5}, {  6,   7,   8}, {  9,  10,  11}},
  {{  1,   2,   0}, { 22,  23,  24}, { 25,  26,  27}},
  {{ 17,  18,  16}, { 19,  20,  21}, { 28,  29,  30}}
  };
  /* This is the pin mapping for multiplexing the layers of the led cube, starting with
   * the bottom layer. All layers are switched with a mosfet LOW=ON, HIGH=OFF, they are
   * connected with a 1K pull up resistor as to not switch them on at boot up time. */
  uint8_t m_layerPin[6] = {1<<0,1<<1,1<<2,1<<3,1<<4,1<<5};
  /* Number of bytes that are needed to send all bits to the TLC's. (12 bits/channel) */
  uint8_t m_channelBuffer[CHNBYTES];
  /* There should be only one displayed layer that is set to LOW, all other layers should
   * be set to HIGH. The displaying of the layers will start at the bottom (y=0). Every
   * cycle turns off the current layer first and than turns on the next one. */
  uint8_t m_LayerOffset = 0;
  uint8_t m_currentLayerPin = m_layerPin[0];
  uint8_t m_nextLayerPin    = m_layerPin[1];
public:
  OctadecaTLC5940();
  void setVoxel(int x, int y, int z, Color c);
  Color getRenderingVoxel(int x, int y, int z);
  Color getDisplayedVoxel(int x, int y, int z);
  void update();
  void multiplex();
  /* Function pointer object instance to call multiplex() from the static interrupt
   * service routine declared as void ftm1_isr(void) */
  static OctadecaTLC5940* me;
  // Start timers and interrupts
  void begin();
private:
  void setChannel(uint16_t channel, uint16_t color);
  void setChannelBuffer(int layer);
  void sendChannelBuffer();
};
#endif

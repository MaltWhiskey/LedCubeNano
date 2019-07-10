#include "Cube.h"
#include "Color.h"
#include "Util.h"
/*---------------------------------------------------------------------------------------
 * Globals
 *-------------------------------------------------------------------------------------*/
Cube cube(3,3,3);
ColorWheel colorwheel(250);
NoiseGenerator generator;
/*---------------------------------------------------------------------------------------
 * Initialize setup parameters
 *-------------------------------------------------------------------------------------*/
void setup() {
  cube.begin();
  colorwheel.add(Color::RED);
  colorwheel.add(Color::GREEN);
  colorwheel.add(Color::BLUE);
}
/*---------------------------------------------------------------------------------------
 * Start the main loop
 *-------------------------------------------------------------------------------------*/
void loop() {
  cube.animate();
}

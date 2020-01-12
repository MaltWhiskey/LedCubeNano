#include <Arduino.h>
#include "animations.h"
/*---------------------------------------------------------------------------------------
 * Globals
 *-------------------------------------------------------------------------------------*/
Sinus sinus = Sinus();
Spiral spiral = Spiral();
Rainbow rainbow = Rainbow();
Rain rain = Rain();
Bounce bounce = Bounce();
Fireworks fireworks = Fireworks();
Twinkel twinkel = Twinkel();
/*---------------------------------------------------------------------------------------
 * Initialize setup parameters
 *-------------------------------------------------------------------------------------*/
void setup() {
  Animation::begin();
}
/*---------------------------------------------------------------------------------------
 * Start the main loop
 *-------------------------------------------------------------------------------------*/
void loop() {
  Animation::animate();
}
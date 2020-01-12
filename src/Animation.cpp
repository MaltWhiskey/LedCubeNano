#include <stdint.h>
#include "Animation.h"
#include "Color.h"
/*---------------------------------------------------------------------------------------
 * ANIMATION STATIC DEFINITIONS
 *-------------------------------------------------------------------------------------*/
Display& Animation::display = Display::instance();
Timer Animation::timer = Timer();
uint8_t Animation::m_iAnimations = 0;
Animation* Animation::m_pCurrentAnimation = 0;
uint16_t Animation::wheel = 0;
/*---------------------------------------------------------------------------------------
 * ANIMATION CLASS METODS (not inline)
 *---------------------------------------------------------------------------------------
 * A virtual pixel can be anywhere in between fysical pixels this will
 * illuminate fysical pixels surrounding the virtual pixel using 1/r^2 
 *-------------------------------------------------------------------------------------*/
void Animation::radiate(const Vector3& v, const Color& c, const float distance, const int8_t range) {
  for(int8_t x=v.x-range;x<=v.x+range;x++)
  for(int8_t y=v.y-range;y<=v.y+range;y++)
  for(int8_t z=v.z-range;z<=v.z+range;z++) {
    if((x>=0)&(y>=0)&(z>=0)&(x<display.width)&(y<display.height)&(z<display.depth)) {
      // distance between input vector and loop vector
      Vector3 vDistance = v - Vector3(x,y,z);
      // radius is distance between pixels * distance from lightsource
      float radius = distance*vDistance.magnitude();
      // supercharged inverse square law, use 1/(1+r^5) instead
      radius = 1/(1+radius*radius*radius*radius*radius);
      // merge with multiple light sources
      display.color(x,y,z) += Color(radius*c.r,radius*c.g,radius*c.b);
    }
  }
}

// Move the cube down, clear the top layer
void Animation::down() {
  for(uint8_t x=0; x < display.width;x++)
  for(uint8_t z=0; z < display.depth;z++) {
    display.color(x,display.height-1,z) = Color(Color::BLACK);
    for(uint8_t y=1;y < display.height;y++)
    display.color(x,y-1,z) = display.colored(x,y,z);
  }
}
// Copy the displayed cube to the rendering cube
void Animation::copy() {
  for(uint8_t x=0;x < display.width;x++)
  for(uint8_t y=0;y < display.height;y++)
  for(uint8_t z=0;z < display.depth;z++)
    display.color(x,y,z) = display.colored(x,y,z);
}

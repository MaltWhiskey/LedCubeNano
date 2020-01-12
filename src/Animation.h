#ifndef ANIMATION_H
#define ANIMATION_H
#include "Util.h"
#include "Color.h"
#include "Math3D.h"
#include "Display.h"
/*---------------------------------------------------------------------------------------
 * ANIMATION INTERFACE
 *-------------------------------------------------------------------------------------*/
class Animation {
protected:
  // Reference to the display (initialized only once)
  static Display& display;
private:
  // Timer used to call animations making rendering frequency independent
  static Timer timer;
  // Pointer to the current animation in a circulair list
  static Animation *m_pCurrentAnimation;
  // Total entries in the circulair list
  static uint8_t m_iAnimations;
  // Pointer to next animation in the circulair list
  Animation *m_pNextAnimation;
public:
  // Constructor inserts this animation in the list
  Animation() {
    m_iAnimations++;
    m_pNextAnimation=m_pCurrentAnimation;
    m_pCurrentAnimation=this;
  }
  // virtual destructor
  virtual ~Animation(){};
  // draw method needs to be overridden
  virtual bool draw(float dt) = 0;
  // init method needs to be overridden
  virtual void init() = 0;
  // animation scheduler
  static void animate() {
    // update the animation timer and get deltatime between previous frame
    timer.update();
    // draw one animation frame using the deltatime and the display dimensions
    if(m_pCurrentAnimation->draw(timer.dt())) {
      // prepare new animation after final frame of current animation is drawn
      m_pCurrentAnimation = m_pCurrentAnimation->m_pNextAnimation;
      m_pCurrentAnimation->init();
    }
    // wait for vertical blank and than switch rendering and displayed buffers
    display.update();
  }
  static void begin() {
    Animation *p=m_pCurrentAnimation;
    // create a circle from the end of the list back to the beginning
    while(p->m_pNextAnimation)
      p=p->m_pNextAnimation;
    p->m_pNextAnimation=m_pCurrentAnimation;
    // Initializes the first animation
    m_pCurrentAnimation->init();
    // Initialize the display
    display.begin();
  }
  static void radiate(const Vector3& v, const Color& c, const float distance,
    const int8_t range); 
  static void copy();
  static void down();
protected:
/*---------------------------------------------------------------------------------------
 * Animation helper variables, declared static so only 1 of each is used
 * for all instances of Animation, init method needs to initialize the
 * used variables for the active animation before calling the draw method
 * this prevents running more than one animation simultaneously, but saves
 * a lot of memory on a small microcontroller. Move these shared variables
 * to the private members of each animation if memory is not an issue.
 *-------------------------------------------------------------------------------------*/
  // Position in Color Wheel, multiplied by 256 for more resolution.
  static uint16_t wheel;
};
#endif
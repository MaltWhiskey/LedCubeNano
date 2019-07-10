#ifndef ANIMATION_H
#define ANIMATION_H
#include "Util.h"

class Animation {
public:
  virtual ~Animation();
  // run animation with respect to size
  void animate(int width, int height, int depth);
  // test if animation is still running
  bool running();
  // restarts animation next time animate is called
  void restart();
protected:
  // drawing method needs to be overridden
  virtual void draw(float dt) = 0;
  // init method needs to be overridden
  virtual void init() = 0;
private:
  // time of first frame of this animation
  unsigned long m_startTime = 0;
  // time of last frame of this animation
  unsigned long m_lastTime = 0;
  // time of current frame of this animation
  unsigned long m_currentTime = 0;
  // delta time between this frame and previous
  unsigned long m_deltaTime = 0;
  // total run time of this animation
  unsigned long m_runTime = 0;
protected:
  int width;
  int height;
  int depth;
protected:
  // animation helper variables.
  float X, Y, Z;
  // Start animations that use phase at 0
  float phase = 0;
};

class Sinus : public Animation {
private:
  void draw(float);
  void init();
};

class Twinkel : public Animation {
private:
  void draw(float);
  void init();
private:
  Timer timer1, timer2;
  float seconds;
  int loops;
};

class Rain : public Animation {
private:
  void draw(float);
  void init();
private:
  Timer timer1, timer2;
  int loops;
};

class Rainbow : public Animation {
private:
  void draw(float);
  void init();
};

class Spiral : public Animation {
private:
  void draw(float);
  void init();
private:
  Timer timer1, timer2;
  int stage, bottom, top, thickness;
};
#endif

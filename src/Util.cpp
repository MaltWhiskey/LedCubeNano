#include <arduino.h>
#include "Util.h"
/*----------------------------------------------------------------------------------------------
 * NoiseGenerator CLASS
 *--------------------------------------------------------------------------------------------*/
float NoiseGenerator::nextRandom(const float min, const float max) const {
  return min + (rand() / ((float)RAND_MAX)) * (max-min) ;
}
float NoiseGenerator::nextGaussian(const float mean, const float stdev, const float range) {
  float gauss;
  do {
	  gauss = nextGaussian(mean, stdev);
  }
  while( (gauss > range*stdev+mean) | (gauss < -range*stdev+mean) );
  return gauss;
}
float NoiseGenerator::nextGaussian(const float mean, const float stdev) {
  if(hasSpare) {
	  hasSpare = false;
	  return mean + stdev * spare;
  }
  hasSpare = true;
  float u, v, s;
  do {
    u = (rand() / ((float)RAND_MAX)) * 2.0 - 1.0;
    v = (rand() / ((float)RAND_MAX)) * 2.0 - 1.0;
    s = u * u + v * v;
  }
  while( (s >= 1.0) || (s == 0.0) );

  s = sqrt(-2.0 * log(s) / s);
  spare = v * s;
  return mean + stdev * u * s;
}

/*----------------------------------------------------------------------------------------------
 * TIMER CLASS
 *--------------------------------------------------------------------------------------------*/
Timer::Timer() {}
Timer::Timer(const float alarm) {
  operator=(alarm);
}
void Timer::operator=(const float alarm) {
  m_alarmCount = 0;
  m_startTime = 0;
  m_alarmTime = alarm;
}
uint16_t Timer::update() {
  m_currentTime = micros();
  if(m_startTime==0) {
	  m_startTime = m_currentTime;
	  m_lastTime = m_currentTime;
  }
  m_deltaTime = m_currentTime-m_lastTime;
  m_runTime = m_currentTime-m_startTime;
  m_lastTime = m_currentTime;
  if(m_alarmTime > 0) {
	  unsigned int threshold = m_runTime/(m_alarmTime * 1000000.0f);
	  if(m_alarmCount < threshold) {
	    m_alarmCount  = threshold;
      return m_alarmCount;
	  }
  }
  return 0;
}
float Timer::dt() const {
  return m_deltaTime/1000000.0f;
}
float Timer::rt() const {
  return m_runTime/1000000.0f;
}
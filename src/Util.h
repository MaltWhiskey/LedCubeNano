#ifndef UTIL_H
#define UTIL_H
/*----------------------------------------------------------------------------------------------
 * NoiseGenerator CLASS
 *----------------------------------------------------------------------------------------------
 * This class generates numbers according to a plan. The numbers can be random, from a
 * Perlin noise like distribution or from a Gaussian distribution.
 *----------------------------------------------------------------------------------------------*/
class NoiseGenerator {
private:
  bool hasSpare = false;
  float spare;
public:
  // get next normally divided value with given mean and stdev
  float nextGaussian(const float mean, const float stdev);
  // nextGaussian but with a max deviation of range * stdev
  float nextGaussian(const float mean, const float stdev, const float range);
  // get a random float value between min and max (boundaries included)
  float nextRandom(const float min, const float max) const;
};

/*----------------------------------------------------------------------------------------------
 * TIMER CLASS
 *----------------------------------------------------------------------------------------------
 * The Timer class is used to do timing specific actions. All time is in seconds
 *
 * Sets a timer for 0.10 seconds:
 * Timer t = 0.10f;
 * Timer t = Timer(0.10f);
 *
 * Returns an integer of the times the timer has counted 0.10 seconds
 * t.update();
 *----------------------------------------------------------------------------------------------*/
class Timer {
public:
  Timer();
  Timer(const float alarm);
  void operator=(const float alarm);
  uint16_t update();
  float dt() const;
  float rt() const;
private:
  // alarm time in seconds
  float m_alarmTime = 0;
  // amount of times counted to alarm
  uint16_t m_alarmCount = 0;
  // time management 
  unsigned long m_startTime = 0;
  unsigned long m_lastTime = 0;
  unsigned long m_currentTime = 0;
  unsigned long m_deltaTime = 0;
  unsigned long m_runTime = 0;
};
#endif
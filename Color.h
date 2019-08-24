#ifndef COLOR_H
#define COLOR_H
#include <stdint.h>
#include <arduino.h>

class Color {
public:
  uint16_t R;
  uint16_t G;
  uint16_t B;
public:
  static const Color BLACK;
  static const Color WHITE;
  static const Color RED;
  static const Color GREEN;
  static const Color BLUE;
  static const Color YELLOW;
  static const Color ORANGE;
  static const Color PINK;
  static const Color CYAN;
  static const Color MAGENTA;
  static const Color BROWN;
public:
  Color();
  Color(uint16_t R, uint16_t G, uint16_t B);
  Color(Color from, Color to, uint16_t step, uint16_t steps);
  bool operator==(Color);
public:
  void random();
  bool isBlack();
};

class ColorWheel {
private:
  float m_wheelPosition = 0;
  int m_steps = 0;
  int m_colors = 0;

  const static int colorbuffersize = 6;
  Color colorbuffer[colorbuffersize];
public:
  ColorWheel(int steps);
  void turn(float percentage);
  Color color(float percentage);
  void add(Color);
};
#endif

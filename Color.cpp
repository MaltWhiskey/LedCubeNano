#include "Color.h"

const Color Color::BLACK    (0x000, 0x000, 0x000);
const Color Color::WHITE    (0xFFF, 0xFFF, 0xFFF);
const Color Color::RED      (0xFFF, 0x000, 0x000);
const Color Color::GREEN    (0x000, 0xFFF, 0x000);
const Color Color::BLUE     (0x000, 0x000, 0xFFF);
const Color Color::YELLOW   (0xFFF, 0xFFF, 0x000);
const Color Color::ORANGE   (0xFFF, 0x800, 0x000);
const Color Color::PINK     (0x800, 0x300, 0x800);
const Color Color::CYAN     (0x000, 0xFFF, 0xFFF);
const Color Color::MAGENTA  (0xFFF, 0x000, 0xFFF);
const Color Color::BROWN    (0x7B0, 0x400, 0x130);

Color::Color(){
  R=0x000;
  G=0x000;
  B=0x000;
}

Color::Color(uint16_t R_, uint16_t G_, uint16_t B_){
  R=R_;
  G=G_;
  B=B_;
}

/* Creates a Color between source and target scaled by the number of steps
 * if step == 0 gives the source color
 * if step == steps gives the target color
 * if step > 0 and < steps gives a color in between */
Color::Color(Color source, Color target, uint16_t step, uint16_t steps) {
  R = source.R + (step*((long)target.R - (long)source.R))/steps;
  G = source.G + (step*((long)target.G - (long)source.G))/steps;
  B = source.B + (step*((long)target.B - (long)source.B))/steps;
}

bool Color::operator==(Color c) {
  return ((c.R==R) & (c.G==G) & (c.B==B));
}

void Color::random() {
  R=::random(4096);
  G=::random(4096);
  B=::random(4096);
}

bool Color::isBlack() {
  return (R|G|B)==0;
}

ColorWheel::ColorWheel(int steps){
  m_steps = steps;
}

void ColorWheel::add(Color color) {
  if(m_colors < colorbuffersize) {
    colorbuffer[m_colors++] = color;
  }
}
/* Get the color relative from the offset and the wheel position */
Color ColorWheel::color(float percentage) {
  percentage+=m_wheelPosition;
  percentage-=(int)percentage;
  if(percentage<0.0f) percentage++;

  int size = m_colors*m_steps;
  int index = ((int)(percentage*size))%size;
  int source = index/m_steps;
  int target = (source+1)%m_colors;
  return Color(colorbuffer[source], colorbuffer[target], index%m_steps, m_steps);
}

/* Turns the color wheel amount positions forward or backwards */
void ColorWheel::turn(float percentage) {
  m_wheelPosition+=percentage;
  m_wheelPosition-=(int)m_wheelPosition;
  if(m_wheelPosition<0.0f) m_wheelPosition++;
}

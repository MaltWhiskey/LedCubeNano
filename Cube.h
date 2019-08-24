#ifndef CUBE_H
#define CUBE_H
#include "Animation.h"
#include "OctadecaTLC5940.h"
/*       3D Led cube coordinate system

              + + + + + + + + +
             ++              ++
            + +             + +
           +  +            +  +
    Y     +   +           +   +     Z
    |    +    +          +    +    /
    |   +     +         +     +   /
    |  +      +        +      +  /
    8 + + + + + + + + + + + + + 8
    7 +      +        +      + 7
    6 +     +         +     + 6
    5 +    +          +    + 5
    4 +   +           +   + 4
    3 +  +            +  + 3
    2 + +             + + 2
    1 ++              ++ 1
    0 + + + + + + + + + 0
    0 1 2 3 4 5 6 7 8---X               */
class Cube : public OctadecaTLC5940 {
private:
  int m_Width;
  int m_Height;
  int m_Depth;
private:
  void fade(int steps);
public:
  Cube(int width, int height, int depth);
  using OctadecaTLC5940::setVoxel;
  void setVoxel(Vector3& v, Color c);
  void mergeVoxel(int x, int y, int z, Color);
  void mergeVoxel(Vector3& v, Color c);
  void radiateVoxel(Vector3& v, Color c, float distance);
  float map(float value, float currentMin, float currentMax, float newMin, float newMax);
  void down();
  void copy();
  void fade(float seconds, float dt);
  void animate();
private:
  Sinus sinus = Sinus();
  Twinkel twinkel = Twinkel();
  Rain rain = Rain();
  Rainbow rainbow = Rainbow();
  Spiral spiral = Spiral();
  Bounce bounce = Bounce();
private:
  Animation* animation = &rainbow;
  Animation* animations[6] = {
    &sinus, &twinkel, &rain, &rainbow, &spiral, &bounce };
};
#endif

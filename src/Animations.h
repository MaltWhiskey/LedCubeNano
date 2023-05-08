#ifndef ANIMATIONS_H
#define ANIMATIONS_H
#include <arduino.h>

#include "Animation.h"
#include "Color.h"
#include "Display.h"
#include "Math3D.h"
#include "Math8.h"
#include "Util.h"
/*---------------------------------------------------------------------------------------
 * Globals
 *-------------------------------------------------------------------------------------*/
NoiseGenerator generator;
/*---------------------------------------------------------------------------------------
 * SINUS
 *-------------------------------------------------------------------------------------*/
class Sinus : public Animation {
 private:
  float x1, x2;
  float z1, z2;
  float phase;

 private:
  void init() {
    phase = 0;
    x1 = generator.nextRandom(-3, 0);
    x2 = generator.nextRandom(0, 3);
    z1 = generator.nextRandom(-3, 0);
    z2 = generator.nextRandom(0, 3);
  }
  bool draw(float dt) {
    phase += PI * dt;
    wheel -= 256 * 100 * dt;
    for (uint8_t x = 0; x < display.width; x++) {
      float x3, y3, z3;
      x3 = mapf(x, 0, display.width - 1, x1, x2);
      for (uint8_t z = 0; z < display.depth; z++) {
        z3 = mapf(z, 0, display.depth - 1, z1, z2);
        y3 = sinf(phase + sqrtf(x3 * x3 + z3 * z3));
        Vector3 v = Vector3(x, mapf(y3, -1, 1, 0, display.height - 1), z);
        Color c = Color((wheel >> 8) + v.y * 10, &RainbowGradientPalette[0]);
        radiate(v, c, 1.8f, 1);
      }
    }
    return (phase / (12 * PI) >= 1);
  }
};
/*---------------------------------------------------------------------------------------
 * SPIRAL
 *-------------------------------------------------------------------------------------*/
class Spiral : public Animation {
 private:
  float phase;
  Timer timer1, timer2;
  uint8_t stage, bottom, top, thickness;

 private:
  void init() {
    phase = 0;
    bottom = 0;
    top = 0;
    thickness = 5;
    stage = 0;
    timer1 = 0.2f;
    timer2 = 2.0f;
  }
  bool draw(float dt) {
    phase += 2 * PI * dt;
    wheel += 256 * 40 * dt;

    float x1, x2, z1, z2;
    for (uint8_t y = bottom; y < top; y++)
      for (uint8_t i = 0; i < thickness; i++) {
        x1 = sinf(phase + mapf(y, 0, display.height - 1, 0, 2 * PI) +
                  i * PI / 60);
        z1 = cosf(phase + mapf(y, 0, display.height - 1, 0, 2 * PI) +
                  i * PI / 60);
        x2 = mapf(x1, -1.1f, 0.9f, 0, display.width - 1);
        z2 = mapf(z1, -1.1f, 0.9f, 0, display.depth - 1);
        Color c = Color((wheel >> 8) + x2 * 10 + y * 32 + z2 * 10,
                        &RainbowGradientPalette[0]);
        display.color(x2, y, z2) = c;
      }
    if (timer1.update()) {
      uint8_t state = 0;
      if (stage == state++) top < display.height ? top++ : stage++;
      if (stage == state++) thickness < 25 ? thickness++ : stage++;
      if (stage == state++)
        if (timer2.update()) stage++;
      if (stage == state++) bottom < display.height ? bottom++ : stage++;
      if (stage == state++) return true;
    }
    return false;
  }
};
/*---------------------------------------------------------------------------------------
 * RAINBOW
 *-------------------------------------------------------------------------------------*/
class Rainbow : public Animation {
 private:
  float phase;
  float x1, y1, z1;

 private:
  void init() {
    phase = 0;
    x1 = generator.nextRandom(5, 25);
    y1 = generator.nextRandom(5, 25);
    z1 = generator.nextRandom(5, 25);
    display.globalBrightness = 0x60;
  }
  bool draw(float dt) {
    phase += PI / 7 * dt;
    wheel += 256 * 100 * sinf(phase) * dt;

    if (phase / (2 * PI) >= 1)
      if (--display.globalBrightness == 0xff) return true;

    for (uint8_t x = 0; x < display.width; x++)
      for (uint8_t y = 0; y < display.height; y++)
        for (uint8_t z = 0; z < display.depth; z++) {
          Color c = Color((wheel >> 8) + x * x1 + y * y1 + z * z1,
                          &RainbowGradientPalette[0]);
          display.color(x, y, z) = c;
        }
    return false;
  }
};
/*---------------------------------------------------------------------------------------
 * RAIN
 *-------------------------------------------------------------------------------------*/
class Rain : public Animation {
 private:
  Timer timer1, timer2;
  uint8_t loops = 0;

 private:
  void init() {
    if (loops == 0) loops = random(3, 8);
    timer1 = generator.nextRandom(0.100f, 0.200f);
    timer2 = generator.nextRandom(1.000f, 4.000f);
  }
  bool draw(float dt) {
    wheel += 256 * 25 * dt;
    if (timer1.update()) {
      down();
      for (uint8_t d = random(1, 3); d > 0; d--) {
        uint8_t x = random(display.width);
        uint8_t z = random(display.depth);
        Color c = Color((wheel >> 8), &RainbowGradientPalette[0]);
        display.color(x, display.height - 1, z) = c;
      }
    } else {
      copy();
    }
    if (timer2.update()) {
      if (--loops == 0)
        return true;
      else
        init();
    }
    return false;
  }
};
/*---------------------------------------------------------------------------------------
 * BOUNCE
 *-------------------------------------------------------------------------------------*/
class Bounce : public Animation {
 private:
  Vector3 position;
  Vector3 velocity;
  Timer timer1;

 private:
  void init() {
    position = Vector3(display.width / 2.0f, display.height / 2.0f,
                       display.depth / 2.0f);
    velocity = Vector3(generator.nextRandom(2.0f, 5.0f),
                       generator.nextRandom(2.0f, 5.0f),
                       generator.nextRandom(2.0f, 5.0f));
    timer1 = 10.0f;
  }
  bool draw(float dt) {
    wheel += 256 * 50 * dt;
    float vy = generator.nextRandom(5.0f, 20.0f);
    Vector3 t = position + velocity * dt;
    if (t.x > display.width - 1 || t.x < 0) velocity.x = -velocity.x;
    if (t.y > display.height - 1 || t.y < 0) velocity.y = -velocity.y;
    if (t.z > display.depth - 1 || t.z < 0) velocity.z = -velocity.z;
    if (t.y > display.height - 1) velocity.y = -vy;
    position = position + velocity * dt;
    Color c = Color((wheel >> 8), &RainbowGradientPalette[0]);
    radiate(position, c, 0.8f, 1);
    return (timer1.update());
  }
};
/*---------------------------------------------------------------------------------------
 * FIREWORKS
 *-------------------------------------------------------------------------------------*/
class Fireworks : public Animation {
 private:
  uint8_t numDebris;
  Vector3 source;
  Vector3 target;
  Vector3 velocity;
  Vector3 gravity;
  Object2 missile;
  Object2 debris[20];
  uint8_t loops = 0;

 private:
  void init() {
    if (loops == 0) loops = random(3, 8);
    // calculate source normally divided from source area
    source =
        Vector3(generator.nextGaussian((float)display.width / 2.0f, 0.40f), 0,
                generator.nextGaussian((float)display.depth / 2.0f, 0.40f));
    // calculate targets normally divided from target area
    target = Vector3(
        generator.nextGaussian((float)display.width / 2.0f, 0.40f),
        generator.nextGaussian((float)8.0f * display.height / 10.0f, 0.40f),
        generator.nextGaussian((float)display.depth / 2.0f, 0.40f));
    // Assign a time in seconds to reach the target
    float t = generator.nextGaussian(0.60f, 0.25f);
    // Determine directional velocities in pixels per second
    velocity = (target - source) / t;
    // Set missile source and velocity
    missile.position = source;
    missile.velocity = velocity;
    // Set some system gravity
    gravity = Vector3(0, -2.0f, 0);
  }

  bool draw(float dt) {
    // Missile drawing mode
    if (target.y > 0) {
      Vector3 temp = missile.position;
      missile.move(dt, gravity);
      // If missile falls back to earth or moved past the target explode the
      // missile
      if ((temp.y > missile.position.y) | (missile.position.y > target.y)) {
        // Activate explode drawing mode
        target.y = 0;
        // If target is reached the missile is exploded and debris is formed
        numDebris = random(10, sizeof(debris) / sizeof(Object2));
        // Overall exploding power of particles for all debris
        float pwr = generator.nextRandom(0.5f, 1.0f);
        // starting position in the hue colorwheel
        uint8_t hue = (uint8_t)random(0, 256);
        // generate debris with power and hue
        for (int i = 0; i < numDebris; i++) {
          // Debris is formed at target and has random velocities depending on
          // overall power
          Vector3 explode = Vector3(generator.nextRandom(-pwr, pwr),
                                    generator.nextRandom(-pwr, pwr),
                                    generator.nextRandom(-pwr, pwr));
          debris[i] = {temp, explode, uint8_t(hue + random(0, 64)), 1.0f,
                       generator.nextRandom(1.0f, 3.0f)};
        }
      } else if (missile.position.inside(
                     Vector3(0, 0, 0),
                     Vector3(display.width, display.height, display.depth)))
        display.color(missile.position.x, missile.position.y,
                      missile.position.z) = Color(Color::WHITE);
    }
    // Explosion drawing mode
    if (target.y == 0) {
      uint8_t visible = 0;
      for (uint8_t i = 0; i < numDebris; i++) {
        if (debris[i].position.y > 0)
          debris[i].move(dt, gravity);
        else
          debris[i].position.y = 0;
        if (debris[i].brightness > 0) {
          visible++;
          debris[i].brightness -= dt * (1 / debris[i].seconds);
        } else {
          debris[i].brightness = 0;
        }
        Color c = Color(debris[i].hue, RainbowGradientPalette);
        c.r *= debris[i].brightness;
        c.g *= debris[i].brightness;
        c.b *= debris[i].brightness;
        if (debris[i].position.inside(
                Vector3(0, 0, 0),
                Vector3(display.width, display.height, display.depth)))
          display.color(debris[i].position.x, debris[i].position.y,
                        debris[i].position.z) += c;
      }
      if (visible == 0) {
        if (--loops == 0)
          return true;
        else
          init();
      }
    }
    return false;
  }
};
/*---------------------------------------------------------------------------------------
 * TWINKLE
 *-------------------------------------------------------------------------------------*/
class Twinkel : public Animation {
 private:
  Timer timer1;
  Timer timer2;
  Timer timer3;
  uint8_t loops = 0;

 private:
  void init() {
    if (loops == 0) loops = random(4, 10);
    timer1 = generator.nextRandom(0.025f, 0.10f);
    timer2 = generator.nextRandom(1.00f, 4.00f);
    timer3 = generator.nextRandom(0.01f, 0.04f);
  }
  bool draw(float dt) {
    if (timer3.update()) {
      for (uint8_t x = 0; x < display.width; x++)
        for (uint8_t y = 0; y < display.height; y++)
          for (uint8_t z = 0; z < display.depth; z++) {
            Color& c = display.colored(x, y, z);
            c.r = 0.9 * c.r;
            c.g = 0.9 * c.g;
            c.b = 0.9 * c.b;
            display.color(x, y, z) = c;
          }
    } else {
      copy();
    }
    if (timer1.update()) {
      display.color(random(display.width), random(display.height),
                    random(display.depth)) = Color(0, 255);
    }
    if (timer2.update()) {
      if (--loops == 0)
        return true;
      else
        init();
    }
    return false;
  }
};
#endif
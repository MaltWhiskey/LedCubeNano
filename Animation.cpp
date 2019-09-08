#include "Animation.h"
#include "Cube.h"
#include "Color.h"
#include "Util.h"

extern Cube cube;
extern ColorWheel colorwheel;
extern NoiseGenerator generator;
/*---------------------------------------------------------------------------------------
 * ANIMATION INTERFACE
 *-------------------------------------------------------------------------------------*/
Animation::~Animation() { }
void Animation::animate(int width_, int height_, int depth_) {
  width = width_;
  height = height_;
  depth = depth_;

  m_currentTime = micros();
  if(m_startTime==0) {
	m_startTime = m_currentTime;
	m_lastTime = m_currentTime;
	init();
  }
  m_deltaTime = m_currentTime-m_lastTime;
  m_runTime = m_currentTime-m_startTime;
  draw(m_deltaTime/1000000.0f);
  m_lastTime = m_currentTime;
}
void Animation::restart() {
  m_startTime=0;
}
bool Animation::running() {
  return m_startTime!=0;
}
/*---------------------------------------------------------------------------------------
 * SINUS
 *-------------------------------------------------------------------------------------*/
void Sinus::init() { }
void Sinus::draw(float dt) {
  phase += PI*dt;
  colorwheel.turn(-dt/5.0f);

  for(int x=0;x < width;x++) {
    X = cube.map(x, 0, width-1, -2, 2);
    for(int z=0;z < depth;z++) {
      Z = cube.map(z, 0, depth-1,-2,2);
      Y = sinf(phase + sqrtf(X*X + Z*Z));
      Y = round(cube.map(Y, -1, 1, 0, height-1));
      cube.setVoxel(x,(int)Y,z, colorwheel.color(Y*0.1f));
    }
  }

  if(phase/(4*PI) >= 1) {
	  phase-=4*PI;
	  restart();
  }
}
/*---------------------------------------------------------------------------------------
 * TWINKLE
 *-------------------------------------------------------------------------------------*/
void Twinkel::init() {
  timer1 = generator.nextRandom(0.05f, 0.25f);
  timer2 = generator.nextRandom(1.00f, 4.00f);
  seconds= generator.nextRandom(0.50f, 4.00f);
  if(loops==0) loops = random(4,10);
}
void Twinkel::draw(float dt) {
  cube.fade(seconds, dt);
  if(timer1.ticks()) {
    Color color;
    color.random();
	cube.setVoxel(random(width),random(height),random(depth), color);
  }
  if(timer2.ticks()) {
	  if(--loops==0) restart();
	  else init();
  }
}
/*---------------------------------------------------------------------------------------
 * RAIN
 *-------------------------------------------------------------------------------------*/
void Rain::init() {
  timer1 = generator.nextRandom(0.050f, 0.200f);
  timer2 = generator.nextRandom(1.000f, 4.000f);
  if(loops==0) loops = random(3,8);
}
void Rain::draw(float dt) {
  colorwheel.turn(dt/5.0f);

  if(timer1.ticks()) {
	  cube.down();
    for(int d=random(0,2);d>0;d--) {
	  Color color;
      int x=random(width);
      int z=random(depth);
      color = colorwheel.color(0);
      cube.setVoxel(x,height-1,z,color);
    }
  }
  else {
    cube.copy();
  }
  if(timer2.ticks()) {
	  if(--loops==0) restart();
	  else init();
  }
}
/*---------------------------------------------------------------------------------------
 * RAINBOW
 *-------------------------------------------------------------------------------------*/
void Rainbow::init() {
  X = generator.nextRandom(0.01f, 0.25f);
  Y = generator.nextRandom(0.01f, 0.25f);
  Z = generator.nextRandom(0.01f, 0.25f);
}
void Rainbow::draw(float dt) {
  phase += PI/5*dt;
  colorwheel.turn(sinf(phase)/3*dt);

  for(int x=0;x<width;x++)
  for(int y=0;y<height;y++)
  for(int z=0;z<depth;z++)
    cube.setVoxel(x,y,z, colorwheel.color(X*x+Y*y+Z*z));

  if(phase/(2*PI) >= 1) {
	  phase-=2*PI;
	  restart();
  }
}
/*---------------------------------------------------------------------------------------
 * SPIRAL
 *-------------------------------------------------------------------------------------*/
void Spiral::init() {
  phase = 0;
  bottom = 0;
  top = 0;
  thickness = 1;
  stage = 0;
  timer1 = 0.2f;
  timer2 = 2.0f;
}

void Spiral::draw(float dt){
  phase += 2*PI*dt;
  colorwheel.turn(dt/5.0f);

  for(int y=bottom;y<top;y++)
  for(int i=0;i<thickness;i++) {
    X = sinf(phase + cube.map((float)y, 0, height-1, 0, 2*PI) + i*PI/60);
    Z = cosf(phase + cube.map((float)y, 0, height-1, 0, 2*PI) + i*PI/60);
    X = cube.map((float)X, -1.1f, 0.9f, 0, width-1);
    Z = cube.map((float)Z, -1.1f, 0.9f, 0, depth-1);
    cube.setVoxel(X,y,Z,colorwheel.color(X*0.05f + y*0.2f + Z*0.05f));
  }
  if(timer1.ticks()) {
    int state = 0;
    if(stage == state++)
      top < height ? top++ : stage++;
    if(stage == state++)
      thickness < 20 ? thickness++ : stage++;
    if(stage == state++)
      if(timer2.ticks()) stage++;
    if(stage == state++)
      bottom < height ? bottom++ : stage++;
    if(stage == state++)
      restart();
  }
}
/*---------------------------------------------------------------------------------------
 * BOUNCE
 *-------------------------------------------------------------------------------------*/
void Bounce::init() {
  ball.position = Vector3(width/2.0f, height/2.0f, depth/2.0f);
  ball.velocity = Vector3(generator.nextRandom(4.0f,10.0f),
	generator.nextRandom(4.0f,10.0f), generator.nextRandom(4.0f,10.0f));
  timer = 10.0f;
}
void Bounce::draw(float dt) {
  colorwheel.turn(dt/5.0f);
  cube.radiateVoxel(ball.position, colorwheel.color(0), 1.4f);
  ball.bounce(dt, generator.nextRandom(5.0f,20.0f), width, height, depth);
  if(timer.ticks()) restart();
}

This project is made to control my 3x3x3 or 3x6x3 RGB led cubes.

The hardware and software are of my own design. The harware includes 2 TLC5940 and an Arduino Nano.
The led cube is multiplexed in layers and data is send to the TLC's using SPI.

See my cubes and other projects running here: https://www.youtube.com/channel/UCuQumwFU8Kvs-C-DP2EolAw


TODO:
Quaternion.cpp 	rotation in quaternion and vector testing and implementation
Util.cpp		   	object class has ColorBlender cb; not pretty
ColorBlender		inherit from color?
ColorWheel		faster implementation




#pragma mark-Angle between vectors
    
    float R4DVector3n::angle(const R4DVector3n& v){
        
        float theta;
        
        R4DVector3n u=v;
        R4DVector3n m=*this;
        
        theta=dot(u)/(m.magnitude()*u.magnitude());
        
        theta=RadToDegrees(acos(theta));
        
        return theta;
        
    }
    
    
    
#pragma mark-Rotate a vector about an angle using quaternions
    
    R4DVector3n R4DVector3n::rotateVectorAboutAngleAndAxis(float uAngle, R4DVector3n& uAxis){
        
        //convert our vector to a pure quaternion
        
        R4DQuaternion p(0,(*this));
        
        //normalize the axis
        uAxis.normalize();
        
        //create the real quaternion
        R4DQuaternion q(uAngle,uAxis);
        
        //convert quaternion to unit norm quaternion
        q.convertToUnitNormQuaternion();
        
        R4DQuaternion qInverse=q.inverse();
        
        R4DQuaternion rotatedVector=q*p*qInverse;
        
        return rotatedVector.v;
        
    }


------------------------------


// Rotate a vector with this quaternion.
// http://youtu.be/Ne3RNhEVSIE
// The basic equation is qpq* (the * means inverse) but we use a simplified version of that equation.
const Vector Quaternion::operator*(const Vector& V) const
{
  Quaternion p;
  p.w = 0;
  p.v = V;

  // Could do it this way:
  /*
  const Quaternion& q = (*this);
  return (q * p * q.Inverted()).v;
  */

  // But let's optimize it a bit instead.
  Vector vcV = v.Cross(V);
  return V + vcV*(2*w) + v.Cross(vcV)*2;
}

// Raising this quaternion to a power of t gives us the
// fraction t of the rotation of the quaternion (phew!)
// http://youtu.be/x1aCcyD0hqE
const Quaternion Quaternion::operator^(float t) const
{
  // Convert the quaternion back into axis/angle
  float a;
  Vector n;

  ToAxisAngle(n, a);

  // Scale the angle by t
  float at = a*t;

  // Make a new quaternion out of the new value
  return Quaternion(n, at);
}

// Spherical linear interpolation of a quaternion
// http://youtu.be/x1aCcyD0hqE
const Quaternion Quaternion::Slerp(const Quaternion& other, float t) const
{
  const Quaternion& q = *this;
  Quaternion r = other;

  // This is the quaternion equation that I discuss in the video, and it works just fine!
  // return ((r * q.Inverted()) ^ t) * q;

  // But this one is faster. For the derivation, see "3D Primer For Graphics
  // and Game Development" by Dunn and Parberry, section 10.4.13. I may go
  // over this one in a future video, but the two formulas come out the same.
  // (Hint: I'm more likely to do it if I get a request!)
  float flCosOmega = w*r.w + r.v.Dot(v);
  if (flCosOmega < 0)
  {
    // Avoid going the long way around.
    r.w = -r.w;
    r.v = -r.v;
    flCosOmega = -flCosOmega;
  }

  float k0, k1;
  if (flCosOmega > 0.9999f)
  {
    // Very close, use a linear interpolation.
    k0 = 1-t;
    k1 = t;
  }
  else
  {
    // Trig identity, sin^2 + cos^2 = 1
    float flSinOmega = sqrt(1 - flCosOmega*flCosOmega);

    // Compute the angle omega
    float flOmega = atan2(flSinOmega, flCosOmega);
    float flOneOverSinOmega = 1/flSinOmega;

    k0 = sin((1-t)*flOmega) * flOneOverSinOmega;
    k1 = sin(t*flOmega) * flOneOverSinOmega;
  }

  // Interpolate
  Quaternion result;
  result.w = q.w * k0 + r.w * k1;
  result.v = q.v * k0 + r.v * k1;

  return result;
}

void Quaternion::ToAxisAngle(Vector& vecAxis, float& flAngle) const
{
  // Convert the quaternion back into axis/angle
  if (v.LengthSqr() < 0.0001f)
    vecAxis = Vector(1, 0, 0);
  else
    vecAxis = v.Normalized();

  TAssert(fabs(vecAxis.LengthSqr() - 1) < 0.000001f);

  // This is the opposite procedure as explained in
  // http://youtu.be/SCbpxiCN0U0 w = cos(a/2) and a = acos(w)*2
  flAngle = acos(w)*2;

  // Convert to degrees
  flAngle *= 360 / ((float)M_PI * 2);
}



void Rotater::draw(float dt) {
  o+=0.05f; if((int)o > 12) o=-5;
  float square = 4.0f;

  V3 vOrigin = V3(sin(o)*3+4,cos(o)*3+4,sin(o)*3+4);
  for(int x=vOrigin.x-1;x<vOrigin.x+2;x++)
  for(int y=vOrigin.y-1;y<vOrigin.y+2;y++)
  for(int z=vOrigin.z-1;z<vOrigin.z+2;z++) {
  V3 vLight = V3(x,y,z) ;
  if(vLight.inside(width,height,depth)) {
    V3 vDistance = vOrigin - vLight;
    float r = square*vDistance.magnitude();
    float i = 1/(r*r*r+1);
    Color c = Color(0XFFF*i,0,0XFFF*i);
    cube.setVoxel(vLight, c);
  }
  }


//  cube.setVoxel(t, Color::BLUE);
  return;
  Dot dots[130] = {
  -2,-2,-2,Color::BLUE,-1,-2,-2,Color::BLUE,0,-2,-2,Color::BLUE, 1,-2,-2,Color::BLUE,2,-2,-2,Color::BLUE,
  -2,-2,-1,Color::BLUE,-1,-2,-1,Color::BLUE,0,-2,-1,Color::BLUE, 1,-2,-1,Color::BLUE,2,-2,-1,Color::BLUE,
  -2,-2, 0,Color::BLUE,-1,-2, 0,Color::BLUE,0,-2, 0,Color::BLUE, 1,-2, 0,Color::BLUE,2,-2, 0,Color::BLUE,
  -2,-2, 1,Color::BLUE,-1,-2, 1,Color::BLUE,0,-2, 1,Color::BLUE, 1,-2, 1,Color::BLUE,2,-2, 1,Color::BLUE,
  -2,-2, 2,Color::BLUE,-1,-2, 2,Color::BLUE,0,-2, 2,Color::BLUE, 1,-2, 2,Color::BLUE,2,-2, 2,Color::BLUE,

  -2, 2,-2,Color::GREEN,-1, 2,-2,Color::GREEN,0, 2,-2,Color::GREEN, 1, 2,-2,Color::GREEN,2, 2,-2,Color::RED,
  -2, 2,-1,Color::GREEN,-1, 2,-1,Color::GREEN,0, 2,-1,Color::GREEN, 1, 2,-1,Color::GREEN,2, 2,-1,Color::GREEN,
  -2, 2, 0,Color::GREEN,-1, 2, 0,Color::GREEN,0, 2, 0,Color::GREEN, 1, 2, 0,Color::GREEN,2, 2, 0,Color::GREEN,
  -2, 2, 1,Color::GREEN,-1, 2, 1,Color::GREEN,0, 2, 1,Color::GREEN, 1, 2, 1,Color::GREEN,2, 2, 1,Color::GREEN,
  -2, 2, 2,Color::GREEN,-1, 2, 2,Color::GREEN,0, 2, 2,Color::GREEN, 1, 2, 2,Color::GREEN,2,-2, 2,Color::RED,

  -2,-1,-2,Color::RED,-1,-1,-2,Color::RED,0,-1,-2,Color::RED, 1,-1,-2,Color::RED,2,-1,-2,Color::RED,
  -2, 0,-2,Color::RED,-1, 0,-2,Color::RED,0, 0,-2,Color::RED, 1, 0,-2,Color::RED,2, 0,-2,Color::RED,
  -2, 1,-2,Color::RED,-1, 1,-2,Color::RED,0, 1,-2,Color::RED, 1, 1,-2,Color::RED,2, 1,-2,Color::RED,

  -2,-1, 2,Color::MAGENTA,-1,-1, 2,Color::MAGENTA,0,-1, 2,Color::MAGENTA, 1,-1, 2,Color::MAGENTA,2,-1, 2,Color::MAGENTA,
  -2, 0, 2,Color::MAGENTA,-1, 0, 2,Color::MAGENTA,0, 0, 2,Color::MAGENTA, 1, 0, 2,Color::MAGENTA,2, 0, 2,Color::MAGENTA,
  -2, 1, 2,Color::MAGENTA,-1, 1, 2,Color::MAGENTA,0, 1, 2,Color::MAGENTA, 1, 1, 2,Color::MAGENTA,2, 1, 2,Color::MAGENTA };

  // a(ngle) or theta, convert from degrees to radians (2PI rad = 360 deg)
    ax+=2.0f;
    if(ax>90)ax=0;
    ay+=0.5f;
    az+=0.7f;
  float a,w;
  V3 v2 = V3(0,0,1).normalized();
  V3 v3 = V3(1,0,0).normalized();
  V3 v4 = V3(0,0,0).normalized();

  a = ax/360.0f * (float)PI*2;
  w=cos(a/2);
  v2 = v2*sin(a/2);
  // Quaternion rotating ax arround x angle
  Q4 q2 = Q4(w, v2);
  //q2.normalize();

  a = ay/360.0f * (float)PI*2;
  w=cos(a/2);
  v3 = v3*sin(a/2);
  Q4 q3 = Q4(w, v3);

  a = az/360.0f * (float)PI*2;
  w=cos(a/2);
  v4 = v4*sin(a/2);
  Q4 q4 = Q4(w, v4);

  Q4 q = q2;//*q3*q4;


  a = ax/360.0f * (float)PI*2;
  w=cos(a/2);
  V3 point = V3(-4,0,-3);
  point*sin(a/2);
  Q4 qp = Q4(w, point);

  qp.rotate(point);
  point+=V3(4.5,4.5,4.5);
  cube.setVoxel(point, Color::GREEN);

  point = V3(-4,0,-4);
  q2.rotate(point);
  point+=V3(4.5,4.5,4.5);
  cube.setVoxel(point, Color::RED);

  point = V3(-4,0,4);
  q2.rotate(point);
  point+=V3(4.5,4.5,4.5);
  cube.setVoxel(point, Color::RED);

  point = V3(0,0,-4);
  q3.rotate(point);
  point+=V3(4.5,4.5,4.5);
  cube.setVoxel(point, Color::BLUE);

  V3 pa[9*9];
  for(int i=0; i<9*15;i++){
    pa[i]=V3(i%15,0,i/9);
    pa[i]+=V3(0.001,0.001,0.001);
  }

  a = ax/360.0f * (float)PI*2;
  w=sin(a/2);
  for(int i=0; i<9*9;i++){
    q2.rotate(pa[i]);
    //pa[i]*=w;
    if(pa[i].inside(width,height,depth)) {
        cube.setVoxel(pa[i],Color::RED);
    }
  }

    return;
  for(int i=0;i<80;i++) {
  float x = dots[i].x;
  float y = dots[i].y;
  float z = dots[i].z;
  // Vector from object map
  V3 v1 = V3(x,y,z);

  q.rotate(v1);
  v1+=V3(4,4,4);
  v1.x=round(v1.x);
  v1.y=round(v1.y);
  v1.z=round(v1.z);
  cube.setVoxel(v1,dots[i].c);
  }
}
// a(ngle) or theta, convert from degrees to radians (2PI rad = 360 deg)
/*Q4::Q4(float a, const V3& v_) {
  a = a/360 * (float)PI*2;
  w=cos(a/2);
  v=v_*sin(a/2);
}*/

/*
void Rotater::draw(float dt) {
  Dot dots[130] = {
  -2,-2,-2,Color::BLUE,-1,-2,-2,Color::BLUE,0,-2,-2,Color::BLUE, 1,-2,-2,Color::BLUE,2,-2,-2,Color::BLUE,
  -2,-2,-1,Color::BLUE,-1,-2,-1,Color::BLUE,0,-2,-1,Color::BLUE, 1,-2,-1,Color::BLUE,2,-2,-1,Color::BLUE,
  -2,-2, 0,Color::BLUE,-1,-2, 0,Color::BLUE,0,-2, 0,Color::BLUE, 1,-2, 0,Color::BLUE,2,-2, 0,Color::BLUE,
  -2,-2, 1,Color::BLUE,-1,-2, 1,Color::BLUE,0,-2, 1,Color::BLUE, 1,-2, 1,Color::BLUE,2,-2, 1,Color::BLUE,
  -2,-2, 2,Color::BLUE,-1,-2, 2,Color::BLUE,0,-2, 2,Color::BLUE, 1,-2, 2,Color::BLUE,2,-2, 2,Color::BLUE,

  -2, 2,-2,Color::GREEN,-1, 2,-2,Color::GREEN,0, 2,-2,Color::GREEN, 1, 2,-2,Color::GREEN,2, 2,-2,Color::RED,
  -2, 2,-1,Color::GREEN,-1, 2,-1,Color::GREEN,0, 2,-1,Color::GREEN, 1, 2,-1,Color::GREEN,2, 2,-1,Color::GREEN,
  -2, 2, 0,Color::GREEN,-1, 2, 0,Color::GREEN,0, 2, 0,Color::GREEN, 1, 2, 0,Color::GREEN,2, 2, 0,Color::GREEN,
  -2, 2, 1,Color::GREEN,-1, 2, 1,Color::GREEN,0, 2, 1,Color::GREEN, 1, 2, 1,Color::GREEN,2, 2, 1,Color::GREEN,
  -2, 2, 2,Color::GREEN,-1, 2, 2,Color::GREEN,0, 2, 2,Color::GREEN, 1, 2, 2,Color::GREEN,2,-2, 2,Color::RED,

  -2,-1,-2,Color::RED,-1,-1,-2,Color::RED,0,-1,-2,Color::RED, 1,-1,-2,Color::RED,2,-1,-2,Color::RED,
  -2, 0,-2,Color::RED,-1, 0,-2,Color::RED,0, 0,-2,Color::RED, 1, 0,-2,Color::RED,2, 0,-2,Color::RED,
  -2, 1,-2,Color::RED,-1, 1,-2,Color::RED,0, 1,-2,Color::RED, 1, 1,-2,Color::RED,2, 1,-2,Color::RED,

  -2,-1, 2,Color::MAGENTA,-1,-1, 2,Color::MAGENTA,0,-1, 2,Color::MAGENTA, 1,-1, 2,Color::MAGENTA,2,-1, 2,Color::MAGENTA,
  -2, 0, 2,Color::MAGENTA,-1, 0, 2,Color::MAGENTA,0, 0, 2,Color::MAGENTA, 1, 0, 2,Color::MAGENTA,2, 0, 2,Color::MAGENTA,
  -2, 1, 2,Color::MAGENTA,-1, 1, 2,Color::MAGENTA,0, 1, 2,Color::MAGENTA, 1, 1, 2,Color::MAGENTA,2, 1, 2,Color::MAGENTA };

  float xcos=(float)cosf(ax);
  float ycos=(float)cosf(ay);
  float zcos=(float)cosf(az);
  float xsin=(float)sinf(ax);
  float ysin=(float)sinf(ay);
  float zsin=(float)sinf(az);

  for(int i=0;i<130;i++) {
  int x = dots[i].x;
  int y = dots[i].y;
  int z = dots[i].z;

  float X,Y,Z,Q;
  //rotate z
  X=(x * zcos) - (y * zsin);
  Y=(x * zsin) + (y * zcos);
  //rotate Y
  Z=(z * ycos) - (X * ysin);
  X=(z * ysin) + (X * ycos);
  //rotate X
  Q=(Y * xcos) - (Z * xsin);
  Z=(Y * xsin) + (Z * xcos);
  az+=0.0001f;
  //ax+=0.0001f;
  cube.setVoxel(X+4,Q+4,Z+4,dots[i].c);
  }

  colorwheel.turn(dt);
  if(timer.ticks()) restart();
}
*/


void LedCube::rotate(V3 angles) {
  angles.x = angles.x/360.0f * (float)PI*2;
  angles.y = angles.y/360.0f * (float)PI*2;
  angles.z = angles.z/360.0f * (float)PI*2;
  V3 vx = V3(1,0,0) * sin(angles.x/2);
  V3 vy = V3(0,1,0) * sin(angles.y/2);
  V3 vz = V3(0,0,1) * sin(angles.z/2);
  Q4 qx = Q4(cos(angles.x/2), vx);
  Q4 qy = Q4(cos(angles.y/2), vy);
  Q4 qz = Q4(cos(angles.z/2), vz);
  Q4 qt = qx*qy*qz;

  Color temp[X_LAYERS][Y_LAYERS][Z_LAYERS];
  for(int x=0;x < m_Width;x++)
  for(int y=0;y < m_Height;y++)
  for(int z=0;z < m_Depth;z++) {
  V3 vr = V3(x,y,z);
  vr-=V3(4,4,4);
  qt.rotate(vr);
  vr+=V3(4.1,4.1,4.1);
  if(vr.inside(m_Width,m_Height,m_Depth)){
      temp[(int)vr.x][(int)vr.y][(int)vr.z] = getRenderingVoxel(x,y,z);
  }
  }
  for(int x=0;x < m_Width;x++)
  for(int y=0;y < m_Height;y++)
  for(int z=0;z < m_Depth;z++) {
    setVoxel(x,y,z, temp[x][y][z]);
  }
}



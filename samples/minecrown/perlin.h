#ifndef PERLIN_H_

#define PERLIN_H_

#include <stdlib.h>

#define SAMPLE_SIZE 1024

class Perlin
{
public:

  Perlin(int octaves,float freq,float amp,int seed);


  inline float Get(float x,float y)
  {
    float vec[2];
    vec[0] = x;
    vec[1] = y;
    return perlin_noise_2D(vec);
  };

private:
  void init_perlin(int n,float p);
  float perlin_noise_2D(float vec[2]);

  float noise1(float arg);
  float noise2(float vec[2]);
  float noise3(float vec[3]);
  void normalize2(float v[2]);
  void normalize3(float v[3]);
  void init(void);

  int   mOctaves;
  float mFrequency;
  float mAmplitude;
  int   mSeed;

  int p[SAMPLE_SIZE + SAMPLE_SIZE + 2];
  float g3[SAMPLE_SIZE + SAMPLE_SIZE + 2][3];
  float g2[SAMPLE_SIZE + SAMPLE_SIZE + 2][2];
  float g1[SAMPLE_SIZE + SAMPLE_SIZE + 2];
  bool  mStart;

};

#endif


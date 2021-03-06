#ifndef WAVE_GUIDE_BORDER_H
#define WAVE_GUIDE_BORDER_H
#include <vector>

/** Struct for storing the waveguide borders of a single waveguide */
struct WaveGuideBorder
{
  std::vector<double> x1;
  std::vector<double> z1;
  std::vector<double> x2;
  std::vector<double> z2;
};

#endif

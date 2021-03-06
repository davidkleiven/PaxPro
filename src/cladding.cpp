#include "cladding.hpp"
#include <cmath>

using namespace std;

const double PI = acos(-1.0);
void Cladding::computePotential()
{
  potential = 4.0*PI*thomsonScatteringLength*electronDensity;
}

void Cladding::setElectronDensity( double eDensity )
{
  electronDensity = eDensity;
  computePotential();
};

void Cladding::setRefractiveIndex( double newdelta, double newbeta )
{
  delta = newdelta;
  beta = newbeta;
}

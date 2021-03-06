#ifndef ALTERNATING_DIRECTION_SOLVER_H
#define ALTERNATING_DIRECTION_SOLVER_H
#include "solver3D.hpp"
#include "thomasAlgorithm.hpp"
#include <complex>

typedef std::complex<double> cdouble;

/**
* Alternating Direction Implicit Method is a 3D solver that first treats
* the finite difference approximation of in the x-direction by an implicit scheme
* and y-direction by explicit scheme, then vice versa.
* The beam is assumed to propagate in the positive z-direction
*/
class ADI: public Solver3D
{
public:
  ADI(): Solver3D("ADI"){};

  /** Solves one step */
  virtual void solveStep( unsigned int step ) override;

  /** If true transparent boundary conditions will be used, otherwise Dirichlet boundary conditions are used*/
  bool useTBC{true};
private:
  ThomasAlgorithm matrixSolver;

  /** Propagates 1 step by solving x-direction implicitly */
  void xImplicit( unsigned int step );

  /** Propagates 1 step by solving y-direction implicitly */
  void yImplicit( unsigned int step );

  enum class ImplicitDirection_t {X,Y};
  /** Apply transparent boundary conditions */
  void applyTBC( cdouble diag[], cdouble rhs[], ImplicitDirection_t dir );

  double k{0.0};
  double dx{1.0};
  double dy{1.0};


};
#endif

#ifndef SOLVER_BASE_CLASS_H
#define SOLVER_BASE_CLASS_H
#include <armadillo>
#include <visa/gaussianKernel.hpp>
#include <visa/lowPassFilter.hpp>

class ParaxialSimulation;

/** Base class for all solvers */
class Solver
{
public:
  enum class Dimension_t{TWO_D, THREE_D};
  Solver( const char* name,  Dimension_t dim ): name(name), dim(dim){};
  virtual ~Solver(){};

  /** Get the dimension of which the solver domain */
  Dimension_t getDim() const { return dim; };

  /** Get the name of the solution */
  std::string getName() const { return name; };

  /** Set waveguide to operate on */
  virtual void setSimulator( ParaxialSimulation &newGuide ){ guide = &newGuide; };

  /** Get the simulator */
  const ParaxialSimulation& getSimulator() const { return *guide; };

  /** Reset the counter */
  virtual void reset(){ currentStep = 1; };

  /** Get solution 3D  */
  virtual const arma::cx_cube& getSolution3D() const;

  /** Get solution 2D */
  virtual const arma::cx_mat& getSolution() const;

  /** Get last solution 2D */
  virtual const arma::cx_vec& getLastSolution() const;

  /** Get last solution 3D */
  virtual const arma::cx_mat& getLastSolution3D() const;

  /** Get last solution 3D. Non-const version should only be used for debugging */
  virtual arma::cx_mat& getLastSolution3D();

  /** Solves the entire system */
  virtual void solve(){}; // This can be implemented at this level

  /** Propagates the solution one step */
  virtual void step() = 0;

  /** Sets the initial conditions 2D */
  virtual void setInitialConditions( const arma::cx_vec &vec ){};

  /** Set initial conditions 3D */
  virtual void setInitialConditions( const arma::cx_mat &mat ){};

  /** Filters the solution in the longitudinal direction */
  virtual void filterInLongitudinalDirection(){};

  /** Downsamples the solution in the longitudinal direction */
  virtual void downSampleLongitudinalDirection(){};

  /** Updates the dimensions of the solution arrays. Useful if the discretization is changed */
  virtual void updateDimensionsOfArrays(){};
protected:
  Dimension_t dim;
  std::string name;
  ParaxialSimulation *guide{NULL};
  visa::GaussianKernel kernel;
  visa::LowPassFilter filter;
  unsigned int currentStep{1};
};

#endif

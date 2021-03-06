#ifndef SOLVER2D_H
#define SOLVER2D_H
#include <string>
#include <complex>
#include <json/writer.h>
#include <armadillo>
#include "solver.hpp"

class WaveGuideFDSimulation;
class ParaxialEquation;
class ParaxialSimulation;
class BoundaryCondition;

typedef std::complex<double> cdouble;

/** Base class for the 2D solvers */
class Solver2D: public Solver
{
public:
  /** Enum for real or imaginary components */
  enum class Comp_t{REAL,IMAG};

  /** Enum for supported boundary conditions */
  enum class BC_t{DIRICHLET,TRANSPARENT};

  Solver2D( const char* name ):Solver(name, Dimension_t::TWO_D){};
  Solver2D( const Solver2D &other ) = delete;
  Solver2D& operator =( Solver2D &other ) = delete;

  virtual ~Solver2D();

  /** Set paraxial equation to solve */
  void setEquation( const ParaxialEquation &equation );

  /** Set the simulator */
  void setSimulator( ParaxialSimulation &newGuide ) override;

  /** Get the solution. Depricated */
  const arma::cx_mat& getSolution( unsigned int iz ) const { return *solution; }; // Depricated. iz is not used.

  /** Get the solution */
  const arma::cx_mat& getSolution() const override { return *solution; };

  /** Import solution from HDF5 file */
  bool importHDF5( const std::string &fname );

  /** Import solution from HDF5 files containing the real part and the amplitude. Depricated */
  bool importHDF5( const std::string &realpart, const std::string &amplitude );

  /** Get the real part of the solution */
  void realPart( double *realsolution ) const;

  /** Get the imaginary part of the solution */
  void imagPart( double *imagsolution ) const;

  /** Get the solution in matrix form */
  void getField( arma::mat &field ) const;

  /** Get the phase of the solution in matrix */
  void getPhase( arma::mat &phase ) const;

  /** Set boundary condition at z = 0*/
  void setInitialConditions( const arma::cx_vec &vec ) override;

  /** Set boundary condition at x=xmin and x=xmax */
  void setXBC( const cdouble valuesTop[], const cdouble valuesBottom[] ); // BC at top (x=xmax) and bottom (x=xmin)

  /** Get last solution. NOTE: Return prev solution as this is not filtered */
  virtual const arma::cx_vec& getLastSolution() const override final { return *prevSolution; };

  /** Run simulation */
  void solve();

  /** Perform one step */
  void step() override;

  /** Filter and downsample in the longitudinal direction */
  void filterInLongitudinalDirection() override;

  /** Down sample in longitudinal direction */
  void downSampleLongitudinalDirection() override;

  /** Set which boundary conditions to use. Dirichlet is default */
  void setBoundaryCondition( BC_t bc ){ boundaryCondition = bc; };

  /** Function that disables the longitudinal filtering */
  void disableLongitudinalFilter(){ longitudinalFilterDisabled = true; };

  // Virtual functions
  /** Pure virtual function for solving the system */

  /** Fill JSON object with parameters specific to this class */
  virtual void fillInfo( Json::Value &obj ) const;
protected:
  const ParaxialEquation *eq{nullptr};
  arma::cx_mat *solution{nullptr};
  arma::cx_vec *prevSolution{nullptr};
  arma::cx_vec *currentSolution{nullptr};

  unsigned int Nx{0};
  unsigned int Nz{0};
  bool ownsParaxialEquationObject{true};
  double stepX{1.0};
  double stepZ{1.0};
  double xmin{0.0};
  double zmin{0.0};
  double wavenumber{1.0};
  BC_t boundaryCondition{BC_t::DIRICHLET};
  bool longitudinalFilterDisabled{false}; // Flag for completely disable longitudinal filter

  /** Get the solution */
  arma::cx_mat& getSolution( unsigned int iz ) { return *solution; };

  /** Get solution, rear or imaginary part specified by comp */
  void realOrImagPart( double *solution, Comp_t comp ) const;

  /** Copies the solution in to the solution matrix */
  void copyCurrentSolution( unsigned int step );

  /** Solve one step */
  virtual void solveStep( unsigned int step ) = 0;

  /** Set the required parameters from the waveguide object */
  void initValuesFromWaveGuide();

  /** Set values at z = 0 */
  void setLeftBC( const cdouble values[] );

  /** Return the last element in the solution that should be included in the filtering */
  virtual arma::cx_vec& signalToFilter() const { return *currentSolution; };
};
#endif

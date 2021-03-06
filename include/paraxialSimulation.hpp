#ifndef PARAXIAL_SIMULATION_H
#define PARAXIAL_SIMULATION_H
#include <complex>
#include <H5Cpp.h>
#include <armadillo>
#include <json/writer.h>
#include "farFieldParameters.hpp"
#include "h5Attribute.hpp"
#include "postProcessing.hpp"
#include "postProcessMod.hpp"
#include "materialFunction.hpp"
#include <vector>
#include <string>
class Solver;
class ParaxialSource;
class BorderTracker;
class ArraySource;

/** Struct storing the discretization parameters */
struct Disctretization
{
  double min;
  double max;
  double step;
  unsigned int downsamplingRatio{1};
};
typedef std::complex<double> cdouble;

/** Base class for all paraxial simulations */
class ParaxialSimulation
{
public:
  /** Available boundaries in 2D */
  enum class Boundary_t {TOP, BOTTOM, LEFT, RIGHT};
  explicit ParaxialSimulation( const char* name );
  ParaxialSimulation( const ParaxialSimulation &other ) = delete;
  ParaxialSimulation& operator =( const ParaxialSimulation other ) = delete;

  virtual ~ParaxialSimulation();

  /** Set the transverse discretization */
  void setTransverseDiscretization( double xmin , double xmax, double step );

  /** Set the transverse discretization with down sampling */
  void setTransverseDiscretization( double xmin, double xmax, double step, unsigned int downsamplingRatio );

  /** Set the longitudinal discretization */
  void setLongitudinalDiscretization( double zmin , double zmax, double step );

  /** Set longitudinal discretization with down sampling */
  void setLongitudinalDiscretization( double zmin, double zmax, double step, unsigned int downsamplingRatio );

  /** Set the vertical discretization (only applicable in 3D) */
  void setVerticalDiscretization( double ymin, double ymax, double step );

  /** Get number of nodes in the transverse direction */
  unsigned int nodeNumberTransverse() const;

  /** Get number of nodes in the longitudinal direction */
  unsigned int nodeNumberLongitudinal() const;

  /** Get number of nodes in the vertical direction */
  unsigned int nodeNumberVertical() const;

  /** Get transverse discretiaztion */
  const Disctretization& transverseDiscretization() const{ return *xDisc; };

  /** Get longitudinal discretization */
  const Disctretization& longitudinalDiscretization() const { return *zDisc; };

  /** Get vertical discretization */
  const Disctretization& verticalDiscretization() const { return *yDisc; };

  /** Set the padding value used when computing the far field */
  void setFarFieldPadValue( double padValue ){ farParam.padValue=padValue; };

  /** Set the angle range where the far field will be stored */
  void setFarFieldAngleRange( double phiMin, double phiMax );

  /** Get the wavenumber in nm^{-1}*/
  double getWavenumber() const{ return wavenumber; };

  /** Get the wavelength in nm */
  double getWavelength() const;

  /** Get wave energy in eV */
  double getEnergy() const;

  /** Set wavenumber in nm^{-1}*/
  void setWavenumber( double k ){ wavenumber = k; };

  /** Set wavelength in nm */
  void setWaveLength( double lambda );

  /** Set solver */
  void setSolver( Solver &solv );

  /** Get name of the waveguide simulation */
  std::string getName() const { return name; };

  /** Perform one step */
  void step();

  /** Get intensity at position x,z */
  double getIntensity( double x, double z ) const; // Using linear interpolation

  /** Get intensity at array location ix, iz */
  double getIntensity( unsigned int ix, unsigned int iz ) const; // Returns value in matrix at (ix,iz)

  /** Get z-coordinate corresponding to the array index iz */
  double getZ( unsigned int iz ) const;

  /** Get x-coordinate corresponding to the array index ix */
  double getX ( int ix ) const;

  /** Get y-coordinate corresponding to the array index iy */
  double getY( int iy ) const;

  /** Get far field */
  const arma::vec& getFarField() const { return *farFieldModulus; };

  /** Get the exit field */
  void getExitField( arma::vec &vec ) const;

  /** Get 2D solver */
  const Solver& getSolver() const { return *solver; };

  /** Get the array index closest to x, z */
  void closestIndex( double x, double z, unsigned int &ix, unsigned int &iz ) const;

  /** Enable/disable storing of the intensity and phase for a contour plot */
  void saveContour( bool save=true ){ saveColorPlot=save; };

  /** Reset the stepper */
  void reset();

  /** Add post processing modules */
  ParaxialSimulation& operator << ( post::PostProcessingModule &module );
  ParaxialSimulation& operator << ( post::FarField &farfield );

  /** Add post processing module */
  ParaxialSimulation& addPostProcessingModule( post::PostProcessingModule &module );

  /** Add far field post processing module */
  ParaxialSimulation& addPostProcessingModule( post::FarField &farfield );

  // Virtual methods
  /** Run simulation */
  virtual void solve();

  /** Set incident field */
  virtual void setBoundaryConditions( const ParaxialSource& src );

  /** Sets the initial field from an array of values. Note the length of array needs to exactly match the number of transverse nodes */
  void setBoundaryConditions( const ArraySource &src );

  /** Fill JSON object with parameters specific to this class */
  virtual void fillInfo( Json::Value &obj ) const {};

  /** Get the material properties */
  virtual void getXrayMatProp( double x, double z, double &delta, double &beta ) const;
  virtual void getXrayMatProp( double x, double y, double z, double &delta, double &beta ) const;

  /** Save results to HDF5 file */
  virtual void save( const char* fname );

  /** Save results to HDF5 file */
  void save( const std::string &fname );

  /** Return a border tracker object. Only relevant for geometries that tracks the border i.e. waveguides */
  virtual BorderTracker* getBorderTracker(){ return NULL; };

  /** Pad the exit signal */
  virtual cdouble padExitField( double x, double z ) const { return farParam.padValue; };

  /** Add a description of the simulation that will be added in the HDF5 file */
  std::string description{""};

  /** Material function that specifies the refractive index */
  const MaterialFunction *material{nullptr};
protected:
  Solver *solver{NULL};
  Disctretization *xDisc; // Transverse
  Disctretization *zDisc; // Along optical axis
  Disctretization *yDisc; // Vertical
  arma::vec *farFieldModulus{NULL};
  double wavenumber;
  std::string name;
  const ParaxialSource* src{NULL};
  H5::H5File* file{NULL};
  H5::Group *maingroup{NULL};
  std::string groupname{"/data/"};
  std::vector<std::string> dsetnames;
  bool solverInitializedViaInit{false};
  bool saveColorPlot{true};
  FarFieldParameters farParam;
  std::vector<H5Attr> commonAttributes;
  std::vector<post::PostProcessingModule*> postProcess;
  int uid{0};

  /** Get exit field */
  void getExitField( arma::cx_vec &vec ) const;

  /** Save far field to HDF5 file */
  void saveFarField();

  /** Extra calling */
  virtual void saveSpecialDatasets( hid_t file_id, std::vector<std::string> &dset ) const{};

  template <class arrayType>
  void saveArray( arrayType &array, const char* dsetname, const std::vector<H5Attr> &attr, H5::PredType dtype );

  template <class arrayType>
  void saveArray( arrayType &array, const char* dsetname, const std::vector<H5Attr> &attr );

  template <class arrayType>
  void saveArray( arrayType &array, const char* dsetname );

  template <class arrayType>
  void saveArray( arrayType &matrix, const char* dsetname, H5::PredType dtype );

  /** Add attribute to dataset */
  void addAttribute( H5::DataSet &ds, const char* name, double value );

  /** Add attribute to dataset */
  void addAttribute( H5::DataSet &ds, const char* name, int value );

  /** Extracts the part of the far field corresponding to the angles in far field parameters */
  void extractFarField( arma::vec &newFarField ) const;

  /** Checks that the solver is ready */
  void verifySolverReady() const;

  /** Set group attributes */
  virtual void setGroupAttributes();
};
#endif

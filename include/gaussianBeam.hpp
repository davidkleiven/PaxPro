#ifndef GAUSSIAN_BEAM_H
#define GAUSSIAN_BEAM_H
#include "paraxialSource.hpp"

/** Gaussian beam source */
class GaussianBeam: public ParaxialSource
{
public:
  GaussianBeam(): ParaxialSource("gaussianBeam"){};

  /** Evaluate the beam at position x, z. Both in nano meters */
  cdouble get( double x, double z ) const override final;

  /** Gaussian beam in 3D */
  cdouble get( double x, double y, double z ) const override final;

  /** Sets the waist in nano meter */
  void setWaist( double w ){ waist = w; };

  /** Set the center of the beam */
  void setCenter( double xc, double yc );

  /** Set the angle relative to the z-axis */
  void setAngleX(double ang){ angleX = ang; };

  /** Set angle y */
  void setAngleY(double ang){angleY = ang;};


  /** Get the beam divergence */
  double beamDivergence() const;

  /** Fills a JSON object with parameters specific to this class */
  void info( Json::Value &obj ) const override;

  /** Returns the waist */
  double getWaist() const{ return waist; };
private:
  double waist{1.0};
  double centerX{0.0};
  double centerY{0.0};
  double angleX{0.0};
  double angleY{0.0};

  // Help functions
  /** Get the rayleigh range */
  double rayleighRange() const;

  /** Get the Guoy phase shift */
  double guoyPhase( double z ) const;

  /** Get the inverse radius of curvature */
  double inverseRadiusOfCurvature( double z ) const;

  /** Get the spot size */
  double spotSize( double z ) const;

  /** Phase factor due to incomming angle */
  cdouble angle_factorX(double x) const;

  /** Phase factor due to incomming angle */
  cdouble angle_factorY(double y) const;
};

#endif

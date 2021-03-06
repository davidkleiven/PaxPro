#include "alternatingDirectionSolver.hpp"
#include "paraxialSimulation.hpp"
#include <cassert>
#include <omp.h>
//#define ADI_DEBUG

using namespace std;
void ADI::xImplicit( unsigned int step )
{
  assert( guide != NULL );
  cdouble *diag = new cdouble[Nx*Ny];
  cdouble *subdiag = new cdouble[Nx*Ny-1];
  cdouble *rhs = new cdouble[Nx*Ny];
  cdouble im(0.0,1.0);

  dx = guide->transverseDiscretization().step;
  dy = guide->verticalDiscretization().step;
  double dz = guide->longitudinalDiscretization().step/2.0; // Divide by two since this is just the first half of the propagation step
  k = guide->getWavenumber();

  double z = guide->getZ(step);
  double delta, beta;

  #ifdef ADI_DEBUG
    clog << "Building matrix...\n";
  #endif

  // Build matrix system
  #pragma omp parallel for
  for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
  {
    unsigned int j = indx%Nx;
    unsigned int i = indx/Nx;
    double x = guide->getX(j);
    double y = guide->getY(i);
    guide->getXrayMatProp(x,y,z,delta,beta);
    //unsigned int indx = i*Nx+j;
    diag[indx] = 1.0/dz + im/(k*dx*dx) + beta*k + im*delta*k ;
    rhs[indx] = ( 1.0/dz - im/(k*dy*dy) )*(*prevSolution)(i,j);
    if ( j>0 )
    {
      subdiag[indx-1] = -0.5*im/(k*dx*dx);
    }
    if ( i > 0 )
    {
      rhs[indx] += 0.5*im*(*prevSolution)(i-1,j)/(k*dy*dy);
    }
    if ( i<Ny-1 )
    {
      rhs[indx] += 0.5*im*(*prevSolution)(i+1,j)/(k*dy*dy);
    }
  }
  if ( useTBC ) applyTBC( diag, rhs, ImplicitDirection_t::X );

  #ifdef ADI_DEBUG
    clog << "Solving matrix...\n";
  #endif
  // Solve the system
  matrixSolver.solve( diag, subdiag, rhs, Nx*Ny );

  #ifdef ADI_DEBUG
    clog << "Transferring solution to currentSolution array...\n";
  #endif

  // Copy the solution to the current solution
  #pragma omp parallel for
  for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
  {
    unsigned int i = indx/Nx;
    unsigned int j = indx%Nx;
    (*currentSolution)(i,j) = diag[indx];
  }
  delete [] diag;
  delete [] subdiag;
  delete [] rhs;
}

void ADI::yImplicit( unsigned int step )
{
  assert( guide != NULL );
  cdouble *diag = new cdouble[Nx*Ny];
  cdouble *subdiag = new cdouble[Nx*Ny-1];
  cdouble *rhs = new cdouble[Nx*Ny];
  cdouble im(0.0,1.0);

  dx = guide->transverseDiscretization().step;
  dy = guide->verticalDiscretization().step;
  double dz = guide->longitudinalDiscretization().step/2.0; // Divide by two, since this is only the second half of the propagation step
  k = guide->getWavenumber();

  double z = guide->getZ(step) + dz;
  double delta, beta;

  // Build matrix system
  #pragma omp parallel for
  for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
  {
    unsigned int i = indx/Ny;
    unsigned int j = indx%Ny;
    double x = guide->getX(i);
    double y = guide->getY(j);
    guide->getXrayMatProp(x,y,z,delta,beta);
    diag[indx] = 1.0/dz + im/(k*dy*dy + beta*k + im*delta*k );
    rhs[indx] = (-im/(k*dx*dx) + 1.0/dz)*(*prevSolution)(j,i);
    if ( j>0 )
    {
      subdiag[indx-1] = -0.5*im/(k*dy*dy);
    }
    if ( i > 0 )
    {
      rhs[indx] += 0.5*im*(*prevSolution)(j,i-1)/(k*dx*dx);
    }
    if ( i<Ny-1 )
    {
      rhs[indx] += 0.5*im*(*prevSolution)(j,i+1)/(k*dx*dx);
    }
  }
  if ( useTBC ) applyTBC( diag, rhs, ImplicitDirection_t::Y );

  // Solve the system
  matrixSolver.solve( diag, subdiag, rhs, Nx*Ny );

  // Copy the solution to the current solution
  #pragma omp parallel for
  for ( unsigned int indx=0;indx<Nx*Ny;indx++ )
  {
    unsigned int i = indx/Ny;
    unsigned int j = indx%Ny;
    (*currentSolution)(j,i) = diag[indx];
  }
  delete [] diag;
  delete [] subdiag;
  delete [] rhs;
}

void ADI::solveStep( unsigned int step )
{
  #ifdef ADI_DEBUG
    clog << "Solving X implicitly...\n";
    clog << "Prev max/min: " << prevSolution->max() << " " << prevSolution->min() << " ";
    clog << "- Cur max/min: " << currentSolution->max() << " " << currentSolution->min() << endl;
  #endif
  xImplicit(step);

  #ifdef ADI_DEBUG
    clog << "Copy solution to previous array...\n";
  #endif
  *prevSolution = *currentSolution;
  //copyCurrentSolution(step);

  #ifdef ADI_DEBUG
    clog << "Solving Y implicitly...\n";
  #endif
  yImplicit(step);
}

void ADI::applyTBC( cdouble diag[], cdouble rhs[], ImplicitDirection_t dir )
{
  cdouble im(0.0,1.0);
  double zero = 1E-6;
  switch( dir )
  {
    case ImplicitDirection_t::X:
      // X-direction is implicit
      #pragma omp parallel for
      for ( unsigned int i=0;i<Ny;i++ )
      {
        // ix = Nx-1
        cdouble kdx = -im*log( (*prevSolution)(i,Nx-1)/(*prevSolution)(i,Nx-2) );
        if ( kdx.real() < 0.0 ) kdx.real(0.0);
        unsigned int indx = i*Nx+Nx-1;
        if ( abs((*prevSolution)(i,Nx-2)) > zero ) diag[indx] -= 0.5*im*exp(im*kdx)/(k*dx*dx);

        // Other side ix=0
        indx = i*Nx;
        kdx = -im*log( (*prevSolution)(i,0)/(*prevSolution)(i,1) );
        if ( kdx.real() < 0.0 ) kdx.real(0.0);
        if ( abs((*prevSolution)(i,1)) > zero ) diag[indx] -= 0.5*im*exp(im*kdx)/(k*dx*dx);
      }

      // In y-direction
      #pragma omp parallel for
      for ( unsigned int i=0;i<Nx;i++ )
      {
        // y=Ny-1
        cdouble kdy = -im*log( (*prevSolution)(Ny-1,i)/(*prevSolution)(Ny-2,i) );
        if ( kdy.real() < 0.0 ) kdy.real(0.0);
        unsigned int indx = (Ny-1)*Nx+i;
        if ( abs((*prevSolution)(Ny-2,i)) > zero ) rhs[indx] += 0.5*im*exp(im*kdy)*(*prevSolution)(Ny-1,i)/(k*dy*dy);

        // Other side y = 0
        indx = i;
        kdy = -im*log((*prevSolution)(0,i)/(*prevSolution)(1,i));
        if ( kdy.real() < 0.0 ) kdy.real(0.0);
        if ( abs((*prevSolution)(1,i)) > zero ) rhs[indx] += 0.5*im*exp(im*kdy)*(*prevSolution)(0,i)/(k*dy*dy);
      }
      break;
    case ImplicitDirection_t::Y:
      // Y-direction is implicit
      #pragma omp parallel for
      for ( unsigned int i=0;i<Nx;i++ )
      {
        // In y-direction: y = Ny-1
        cdouble kdy = -im*log( (*prevSolution)(Ny-1,i)/(*prevSolution)(Ny-2,i) );
        if ( kdy.real() < 0.0 ) kdy.real(0.0);
        unsigned int indx = i*Ny+Ny-1;
        if ( abs((*prevSolution)(Ny-2,i)) > zero ) diag[indx] -= 0.5*im*exp(im*kdy)/(k*dy*dy);

        // Other side, y = 0
        indx = i*Ny;
        kdy = -im*log((*prevSolution)(0,i)/(*prevSolution)(1,i));
        if ( kdy.real() < 0.0 ) kdy.real(0.0);
        if ( abs((*prevSolution)(1,i)) > zero ) diag[indx] -= 0.5*im*exp(im*kdy)/(k*dy*dy);
      }

      // In x-direction
      #pragma omp parallel for
      for ( unsigned int i=0;i<Ny;i++ )
      {
        // x=Nx-1
        cdouble kdx = -im*log((*prevSolution)(i,Nx-1)/(*prevSolution)(i,Nx-2));
        if ( kdx.real() < 0.0 ) kdx.real(0.0);
        unsigned int indx = (Nx-1)*Ny + i;
        if ( abs((*prevSolution)(i,Nx-2)) > zero ) rhs[indx] += 0.5*im*exp(im*kdx)*(*prevSolution)(i,Nx-1)/(k*dx*dx);

        // Other side x = 0
        indx = i;
        kdx = -im*log( (*prevSolution)(i,0)/(*prevSolution)(i,1) );
        if ( kdx.real() < 0.0 ) kdx.real(0.0);
        if ( abs((*prevSolution)(i,1)) > zero ) rhs[indx] += 0.5*im*exp(im*kdx)*(*prevSolution)(i,0)/(k*dx*dx);
      }
      break;
  }
}

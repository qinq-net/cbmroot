#ifndef LKFMinuit_h
#define LKFMinuit_h

#include <iostream> 

#include "TMath.h"
#include "TMinuit.h"
#include "TFitter.h"
#include "TRandom.h"
#include <TGraph2D.h>
#include <TGraph2DErrors.h>
#include <TPolyLine3D.h>
#include <TMath.h>
#include <Math/Vector3D.h>


class TFitter;

class LKFMinuit
{
  public:                       // öffentlich

    LKFMinuit();               // der Default-Konstruktor
    int DoFit(TGraph2DErrors * gr);
    int Initialize();
    double SumDistance2(double par[]);
    double distance2(double x,double y,double z, double *p);
    void   line(double t, double *p, double &x, double &y, double &z);

    inline double* GetParFit() {return fparFit;}
    inline double  GetChi2()   {return fChi2;}
    inline double  GetChi2DoF(){return fChi2DoF;}

  private:
    TGraph2DErrors *fgr;
    TFitter *fMyFit;
    double fparFit[4];
    double fChi2;
    double fChi2DoF;
    double myFunction(double);
    static void minuitFunction(int& nDim, double* gout, double& result, double par[], int flg);
};

#endif

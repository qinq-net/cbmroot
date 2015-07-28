#include "LKFMinuit.h"
#include <TFitter.h>
#include <TMath.h>
#include <TGraph2D.h>
#include <TPolyLine3D.h>
#include <Math/Vector3D.h>
#include "FairLogger.h"

using namespace ROOT::Math;

static LKFMinuit *LKF_obj;

int LKFMinuit::Initialize(){
 LOG(INFO) << "LKFMinuit::Initialize "<<FairLogger::endl;
 fMyFit=new TFitter(2);
 fMyFit->SetFCN(LKFMinuit::minuitFunction);

 Double_t arglist[10];
 arglist[0] = -2;
 fMyFit->ExecuteCommand("SET PRINT",arglist,1);
 fMyFit->ExecuteCommand("SIMPLEX",0,0);
 return 0;
}

int LKFMinuit::DoFit(TGraph2DErrors * gr){
  fgr=gr;
  //TFitter* min = new TFitter(2);
  TFitter* min=fMyFit;
  min->SetObjectFit(gr);

   double pStart[4] = {1,1,1,1};
   min->SetParameter(0,"x0",pStart[0],0.01,0,0);
   min->SetParameter(1,"Ax",pStart[1],0.01,0,0);
   min->SetParameter(2,"y0",pStart[2],0.01,0,0);
   min->SetParameter(3,"Ay",pStart[3],0.01,0,0);
    
   Double_t arglist[10];
   arglist[0] = 1000; // number of function calls 
   arglist[1] = 0.001; // tolerance 
   min->ExecuteCommand("MIGRAD",arglist,2);
   arglist[0]=0;
   arglist[1]=0;
   arglist[2]=0;
   min->ExecuteCommand("SET NOWarnings",arglist,3); //turn off warning messages 
  //if (minos) min->ExecuteCommand("MINOS",arglist,0);
   int nvpar,nparx; 
   double amin,edm, errdef;
   min->GetStats(amin,edm,errdef,nvpar,nparx);
   fChi2=amin;
   fChi2DoF=amin/(double)nvpar;
   //min->PrintResults(1,amin);
   // get fit parameters
   //double parFit[4];
   for (int i = 0; i <4; ++i)  fparFit[i] = min->GetParameter(i);
  
 return 0;
}

double LKFMinuit::myFunction(double par){
  double result=0;

  return result;
}

// Temporary add on
//Fitting of a TGraph2D with a 3D straight line
//
// run this macro by doing: 
// 
// root>.x line3Dfit.C+
//
//Author: L. Moneta
//

// define the parameteric line equation 
void LKFMinuit::line(double t, double *p, double &x, double &y, double &z) { 
   // a parameteric line is define from 6 parameters but 4 are independent
   // x0,y0,z0,z1,y1,z1 which are the coordinates of two points on the line
   // can choose z0 = 0 if line not parallel to x-y plane and z1 = 1; 
   x = p[0] + p[1]*t; 
   y = p[2] + p[3]*t;
   z = t; 
} 

// calculate distance line-point 
double LKFMinuit::distance2(double x,double y,double z, double *p) { 
   // distance line point is D= | (xp-x0) cross  ux | 
   // where ux is direction of line and x0 is a point in the line (like t = 0) 
   XYZVector xp(x,y,z); 
   XYZVector x0(p[0], p[2], 0. ); 
   XYZVector x1(p[0] + p[1], p[2] + p[3], 1. ); 
   XYZVector u = (x1-x0).Unit(); 
   double d2 = ((xp-x0).Cross(u)) .Mag2(); 
   return d2; 
}

bool first = true; 
double LKFMinuit::SumDistance2(double par[]) { 
   // the TGraph must be a global variable
  TGraph2D * gr = fgr; //dynamic_cast<TGraph2D*>( (TVirtualFitter::GetFitter())->GetObjectFit() );
   assert(gr != 0);
   double * x = gr->GetX(); 
   double * y = gr->GetY();
   double * z = gr->GetZ();
   int npoints = gr->GetN();
   double sum = 0;
   for (int i  = 0; i < npoints; ++i) { 
      double d = distance2(x[i],y[i],z[i],par); 
      sum += d;
      //#ifdef DEBUG
      if (first) std::cout << " -D- LKFMinuit::SumDistance2: point " << i << "\t" 
                           << x[i] << "\t" 
                           << y[i] << "\t" 
                           << z[i] << "\t" 
                           << std::sqrt(d) << std::endl; 
      //#endif
   }
   if (first) 
      std::cout << " -D- LKFMinuit::SumDistance2: Total sum2 = " << sum << std::endl;
   first = false;
   return sum;
}


void LKFMinuit::minuitFunction(int& nDim, double* gout, double& result, double par[], int flg){
  //    result = LKF_obj->myFunction(par[0]);
  result = LKF_obj->SumDistance2(par);
}

LKFMinuit::LKFMinuit():
   fgr(NULL),
   fMyFit(NULL),
   fChi2(0.),
   fChi2DoF(0.)
{
  //std::cout << "LKFMinuit at " << this << std::endl;
  LKF_obj = this;
}

#ifndef L1AddMaterial_h
#define L1AddMaterial_h

#include "CbmL1Def.h"
#include "L1MaterialInfo.h"
#include "L1TrackPar.h"

//#define cnst static const fvec
#define cnst const fvec

const fvec PipeRadThick = 7.87e-3f; // 0.7 mm Aluminium
const fvec TargetRadThick = 3.73e-2f; // 125 mum Gold

inline fvec ApproximateBetheBloch( const fvec &bg2 )
{
  //
  // This is the parameterization of the Bethe-Bloch formula inspired by Geant.
  //
  // bg2  - (beta*gamma)^2
  // kp0 - density [g/cm^3]
  // kp1 - density effect first junction point
  // kp2 - density effect second junction point
  // kp3 - mean excitation energy [GeV]
  // kp4 - mean Z/A
  //
  // The default values for the kp* parameters are for silicon.
  // The returned value is in [GeV/(g/cm^2)].
  //

  const fvec &kp0 = 2.33f;
  const fvec &kp1 = 0.20f;
  const fvec &kp2 = 3.00f;
  const fvec &kp3 = 173e-9f;
  const fvec &kp4 = 0.49848f;

  const float mK  = 0.307075e-3f; // [GeV*cm^2/g]
  const float _2me  = 1.022e-3f;    // [GeV/c^2]
  const fvec &rho = kp0;
  const fvec &x0  = kp1 * 2.303f;
  const fvec &x1  = kp2 * 2.303f;
  const fvec &mI  = kp3;
  const fvec &mZA = kp4;
  const fvec &maxT = _2me * bg2;    // neglecting the electron mass

  //*** Density effect
  fvec d2( 0.f );
  const fvec x = 0.5f * log( bg2 );
  const fvec lhwI = log( 28.816f * 1e-9f * sqrt( rho * mZA ) / mI );

  fvec init = x > x1;

  d2 = fvec(init & (lhwI + x - 0.5f));
  const fvec &r = ( x1 - x ) / ( x1 - x0 );
  init = (x > x0) & (x1 > x);
  d2 = fvec(init & (lhwI + x - 0.5f + ( 0.5f - lhwI - x0 ) * r * r * r)) + fvec( (!init) & d2);

  return mK*mZA*( fvec( 1.f ) + bg2 ) / bg2*( 0.5f*log( _2me*bg2*maxT/(mI*mI) ) - bg2 / ( fvec( 1.f ) + bg2 ) - d2 );
}

inline void EnergyLossCorrection(L1TrackPar& T, const fvec& mass2, const fvec& radThick, fvec& qp0, fvec direction, fvec w = 1)
{
  const fvec& p2 = 1.f/(T.qp*T.qp);
  const fvec& E2 = mass2 + p2;

  const fvec& bethe = ApproximateBetheBloch( p2/mass2 );

  fvec tr = sqrt(fvec(1.f) + T.tx*T.tx + T.ty*T.ty) ;
  
  const fvec& dE = bethe * radThick*tr * 2.33f * 9.34961f;

  const fvec& E2Corrected = (sqrt(E2) + direction*dE) * (sqrt(E2) + direction*dE);
  fvec corr = sqrt( p2/( E2Corrected - mass2 ) );
  fvec init = fvec(!(corr == corr)) | fvec(w<1);
  corr = fvec(fvec(1.f) & init ) + fvec(corr & fvec(!(init)));

  qp0   *= corr;
  T.qp  *= corr;
  T.C40 *= corr;
  T.C41 *= corr;
  T.C42 *= corr;
  T.C43 *= corr;
  T.C44 *= corr * corr;
}

inline void L1AddMaterial( L1TrackPar &T, fvec radThick, fvec qp0, fvec w = 1, fvec mass2 = 0.1395679f*0.1395679f )
{
  cnst ONE = 1.;

  fvec tx = T.tx;
  fvec ty = T.ty;
  fvec txtx = tx*tx;
  fvec tyty = ty*ty;
  fvec txtx1 = txtx + ONE;
  fvec h = txtx + tyty;
  fvec t = sqrt(txtx1 + tyty);
  fvec h2 = h*h;
  fvec qp0t = qp0*t;
  
  cnst c1=0.0136f, c2=c1*0.038f, c3=c2*0.5f, c4=-c3/2.0f, c5=c3/3.0f, c6=-c3/4.0f;
    
  fvec s0 = (c1+c2*log(radThick) + c3*h + h2*(c4 + c5*h +c6*h2) )*qp0t;    
  //fvec a = ( (ONE+mass2*qp0*qp0t)*radThick*s0*s0 );
  fvec a = ( (t+mass2*qp0*qp0t)*radThick*s0*s0 );

  T.C22 += w*txtx1*a;
  T.C32 += w*tx*ty*a; T.C33 += w*(ONE+tyty)*a;
}

inline void L1AddMaterial( L1TrackPar &T, L1MaterialInfo &info, fvec qp0, fvec w = 1, fvec mass2 = 0.1395679f*0.1395679f )
{
  cnst  ONE = 1.f;

  fvec tx = T.tx;
  fvec ty = T.ty;
  fvec txtx = tx*tx;
  fvec tyty = ty*ty;
  fvec txtx1 = txtx + ONE;
  fvec h = txtx + tyty;
  fvec t = sqrt(txtx1 + tyty);
  fvec h2 = h*h;
  fvec qp0t = qp0*t;
  
  cnst c1=0.0136f, c2=c1*0.038f, c3=c2*0.5f, c4=-c3/2.0f, c5=c3/3.0f, c6=-c3/4.0f;
    
  fvec s0 = (c1+c2*info.logRadThick + c3*h + h2*(c4 + c5*h +c6*h2) )*qp0t;    
  //fvec a = ( (ONE+mass2*qp0*qp0t)*info.RadThick*s0*s0 );
  fvec a = ( (t+mass2*qp0*qp0t)*info.RadThick*s0*s0 );

  T.C22 += w*txtx1*a;
  T.C32 += w*tx*ty*a; T.C33 += w*(ONE+tyty)*a;
}

inline void L1AddHalfMaterial( L1TrackPar &T, L1MaterialInfo &info, fvec qp0 )
{
  cnst ONE = 1.;
  cnst mass2 = 0.1395679f*0.1395679f;

  fvec tx = T.tx;
  fvec ty = T.ty;
  fvec txtx = tx*tx;
  fvec tyty = ty*ty;
  fvec txtx1 = txtx + ONE;
  fvec h = txtx + tyty;
  fvec t = sqrt(txtx1 + tyty);
  fvec h2 = h*h;
  fvec qp0t = qp0*t;
  
  cnst c1=0.0136f, c2=c1*0.038f, c3=c2*0.5f, c4=-c3/2.0f, c5=c3/3.0f, c6=-c3/4.0f;
    
  fvec s0 = (c1+c2*(info.logRadThick + log(0.5)) + c3*h + h2*(c4 + c5*h +c6*h2) )*qp0t;    
  //fvec a = ( (ONE+mass2*qp0*qp0t)*info.RadThick*0.5*s0*s0 );
  fvec a = ( (t+mass2*qp0*qp0t)*info.RadThick*0.5*s0*s0 );

  T.C22 += txtx1*a;
  T.C32 += tx*ty*a; T.C33 += (ONE+tyty)*a;
}

inline void L1AddPipeMaterial( L1TrackPar &T, fvec qp0, fvec w = 1, fvec mass2 = 0.1395679f*0.1395679f )
{
  cnst  ONE = 1.f;

//  static const fscal RadThick=0.0009f;//0.5/18.76;
//  static const fscal logRadThick=log(RadThick);
  //const fscal RadThick=0.0009f;//0.5/18.76;

  const fscal logRadThick=log(PipeRadThick[0]);
  fvec tx = T.tx;
  fvec ty = T.ty;
  fvec txtx = tx*tx;
  fvec tyty = ty*ty;
  fvec txtx1 = txtx + ONE;
  fvec h = txtx + tyty;
  fvec t = sqrt(txtx1 + tyty);
  fvec h2 = h*h;
  fvec qp0t = qp0*t;

  cnst c1=0.0136f, c2=c1*0.038f, c3=c2*0.5f, c4=-c3/2.0f, c5=c3/3.0f, c6=-c3/4.0f;
  fvec s0 = (c1+c2*fvec(logRadThick) + c3*h + h2*(c4 + c5*h +c6*h2) )*qp0t;
  //fvec a = ( (ONE+mass2*qp0*qp0t)*RadThick*s0*s0 );
  fvec a = ( (t+mass2*qp0*qp0t)*PipeRadThick*s0*s0 );

  T.C22 += w*txtx1*a;
  T.C32 += w*tx*ty*a; T.C33 += w*(ONE+tyty)*a;
}

inline void L1AddTargetMaterial( L1TrackPar &T, fvec qp0, fvec w = 1, fvec mass2 = 0.1395679f*0.1395679f )
{
  cnst  ONE = 1.f;

  const fscal logRadThick=log(TargetRadThick[0]);
  fvec tx = T.tx;
  fvec ty = T.ty;
  fvec txtx = tx*tx;
  fvec tyty = ty*ty;
  fvec txtx1 = txtx + ONE;
  fvec h = txtx + tyty;
  fvec t = sqrt(txtx1 + tyty);
  fvec h2 = h*h;
  fvec qp0t = qp0*t;

  cnst c1=0.0136f, c2=c1*0.038f, c3=c2*0.5f, c4=-c3/2.0f, c5=c3/3.0f, c6=-c3/4.0f;
  fvec s0 = (c1+c2*fvec(logRadThick) + c3*h + h2*(c4 + c5*h +c6*h2) )*qp0t;
  //fvec a = ( (ONE+mass2*qp0*qp0t)*RadThick*s0*s0 );
  fvec a = ( (t+mass2*qp0*qp0t)*TargetRadThick*s0*s0 );

  T.C22 += w*txtx1*a;
  T.C32 += w*tx*ty*a; T.C33 += w*(ONE+tyty)*a;
}

#undef cnst

#endif

#include "L1TrackParFit.h"


#define cnst const fvec

void L1TrackParFit::Filter( L1UMeasurementInfo &info, fvec u, fvec w)
{
  fvec wi, zeta, zetawi, HCH;
  fvec F0, F1, F2, F3, F4, F5;
  fvec K1, K2, K3, K4, K5;

  zeta = info.cos_phi*fx + info.sin_phi*fy - u;

  // F = CH'
  F0 = info.cos_phi*C00 + info.sin_phi*C10;
  F1 = info.cos_phi*C10 + info.sin_phi*C11;

  HCH = ( F0*info.cos_phi + F1*info.sin_phi );

  F2 = info.cos_phi*C20 + info.sin_phi*C21;
  F3 = info.cos_phi*C30 + info.sin_phi*C31;
  F4 = info.cos_phi*C40 + info.sin_phi*C41;
  F5 = info.cos_phi*C50 + info.sin_phi*C51;

#if 0 // use mask
  const fvec mask = (HCH < info.sigma2 * 16.);
  wi = w/( (mask & info.sigma2) +HCH );
  zetawi = zeta *wi;
  chi2 +=  mask & (zeta * zetawi);
#else
  wi = w/( info.sigma2 + HCH );
  zetawi = zeta *wi;
  chi2 +=  zeta * zetawi;
#endif // 0
  NDF += w;

  K1 = F1*wi;
  K2 = F2*wi;
  K3 = F3*wi;
  K4 = F4*wi;
  K5 = F5*wi;

  fx  -= F0*zetawi;
  fy  -= F1*zetawi;
  ftx -= F2*zetawi;
  fty -= F3*zetawi;
  fqp -= F4*zetawi;
  ft  -= F5*zetawi;

  C00-= F0*F0*wi;
  C10-= K1*F0;
  C11-= K1*F1;
  C20-= K2*F0;
  C21-= K2*F1;
  C22-= K2*F2;
  C30-= K3*F0;
  C31-= K3*F1;
  C32-= K3*F2;
  C33-= K3*F3;
  C40-= K4*F0;
  C41-= K4*F1;
  C42-= K4*F2;
  C43-= K4*F3;
  C44-= K4*F4;
  C50-= K5*F0;
  C51-= K5*F1;
  C52-= K5*F2;
  C53-= K5*F3;
  C54-= K5*F4;
  C55-= K5*F5;
}

void L1TrackParFit::Filter( fvec t0, fvec dt0, fvec w)
{
  fvec wi, zeta, zetawi, HCH;
  fvec F0, F1, F2, F3, F4, F5;
  fvec K1, K2, K3, K4, K5;

  zeta = ft - t0;

  // F = CH'
  F0 = C50;
  F1 = C51;

  HCH = C55;

  F2 = C52;
  F3 = C53;
  F4 = C54;
  F5 = C55;

#if 0 // use mask
  const fvec mask = (HCH < info.sigma2 * 16.);
  wi = w/( (mask & info.sigma2) +HCH );
  zetawi = zeta *wi;
  chi2 +=  mask & (zeta * zetawi);
#else
  wi = w/( dt0*dt0 + HCH );
  zetawi = zeta *wi;
  chi2 +=  zeta * zetawi;
#endif // 0
  NDF += w;

  K1 = F1*wi;
  K2 = F2*wi;
  K3 = F3*wi;
  K4 = F4*wi;
  K5 = F5*wi;

  fx  -= F0*zetawi;
  fy  -= F1*zetawi;
  ftx -= F2*zetawi;
  fty -= F3*zetawi;
  fqp -= F4*zetawi;
  ft  -= F5*zetawi;

  C00-= F0*F0*wi;
  C10-= K1*F0;
  C11-= K1*F1;
  C20-= K2*F0;
  C21-= K2*F1;
  C22-= K2*F2;
  C30-= K3*F0;
  C31-= K3*F1;
  C32-= K3*F2;
  C33-= K3*F3;
  C40-= K4*F0;
  C41-= K4*F1;
  C42-= K4*F2;
  C43-= K4*F3;
  C44-= K4*F4;
  C50-= K5*F0;
  C51-= K5*F1;
  C52-= K5*F2;
  C53-= K5*F3;
  C54-= K5*F4;
  C55-= K5*F5;
}

void L1TrackParFit::Extrapolate // extrapolates track parameters and returns jacobian for extrapolation of CovMatrix
(
 fvec  z_out  , // extrapolate to this z position
 fvec  qp0    , // use Q/p linearisation at this value
 const L1FieldRegion &F,
 fvec *w
)
{
    //
  // Forth-order Runge-Kutta method for solution of the equation
  // of motion of a particle with parameter qp = Q /P
  //              in the magnetic field B()
  //
  //        | x |          tx
  //        | y |          ty
  // d/dz = | tx| = ft * ( ty * ( B(3)+tx*b(1) ) - (1+tx**2)*B(2) )
  //        | ty|   ft * (-tx * ( B(3)+ty+b(2)   - (1+ty**2)*B(1) )  ,
  //
  //   where  ft = c_light*qp*sqrt ( 1 + tx**2 + ty**2 ) .
  //
  //  In the following for RK step  :
  //
  //     x=x[0], y=x[1], tx=x[3], ty=x[4].
  //
  //========================================================================
  //
  //  NIM A395 (1997) 169-184; NIM A426 (1999) 268-282.
  //
  //  the routine is based on LHC(b) utility code
  //
  //========================================================================

  
  cnst ZERO = 0.0, ONE = 1.;
  cnst c_light = 0.000299792458;
  
  const fvec a[4] = {0.0f, 0.5f, 0.5f, 1.0f};
  const fvec c[4] = {1.0f/6.0f, 1.0f/3.0f, 1.0f/3.0f, 1.0f/6.0f};
  const fvec b[4] = {0.0f, 0.5f, 0.5f, 1.0f};
  
  int step4;
  fvec k[20],x0[5],x[5],k1[20];
  fvec Ax[4],Ay[4],Ax_tx[4],Ay_tx[4],Ax_ty[4],Ay_ty[4], At[4], At_tx[4], At_ty[4];

  //----------------------------------------------------------------

  fvec qp_in = fqp;
  const fvec z_in  = fz;
  const fvec h     = (z_out - fz);
  fvec hC    = h * c_light;
  x0[0] = fx; x0[1] = fy;
  x0[2] = ftx; x0[3] = fty; x0[4] = ft;
  //
  //   Runge-Kutta step
  //

  int step;
  int i;

  fvec B[4][3];
  for (step = 0; step < 4; ++step) {
    F.Get( z_in  + a[step] * h, B[step] );
  }
  
  for (step = 0; step < 4; ++step) {
    for(i=0; i < 5; ++i) {
      if(step == 0) {
        x[i] = x0[i];
      }
      else
      {
        x[i] = x0[i] + b[step] * k[step*5-5+i];
      }
    }

    fvec tx = x[2]; 
    fvec ty = x[3]; 
    fvec tx2 = tx * tx; 
    fvec ty2 = ty * ty; 
    fvec txty = tx * ty;
    fvec tx2ty21= 1.f + tx2 + ty2; 
    // if( tx2ty21 > 1.e4 ) return 1;
    fvec I_tx2ty21 = 1.f / tx2ty21 * qp0;
    fvec tx2ty2 = sqrt(tx2ty21 ) ; 
    //   fvec I_tx2ty2 = qp0 * hC / tx2ty2 ; unsused ???
    tx2ty2 *= hC; 
    fvec tx2ty2qp = tx2ty2 * qp0;
    Ax[step] = ( txty*B[step][0] + ty*B[step][2] - ( 1.f + tx2 )*B[step][1] ) * tx2ty2;
    Ay[step] = (-txty*B[step][1] - tx*B[step][2] + ( 1.f + ty2 )*B[step][0] ) * tx2ty2;

    Ax_tx[step] = Ax[step]*tx*I_tx2ty21 + ( ty*B[step][0]-2.f*tx*B[step][1])*tx2ty2qp;
    Ax_ty[step] = Ax[step]*ty*I_tx2ty21 + ( tx*B[step][0]+       B[step][2])*tx2ty2qp;
    Ay_tx[step] = Ay[step]*tx*I_tx2ty21 + (-ty*B[step][1]-       B[step][2])*tx2ty2qp;
    Ay_ty[step] = Ay[step]*ty*I_tx2ty21 + (-tx*B[step][1]+2.f*ty*B[step][0])*tx2ty2qp;

    fvec p2 = 1.f/(qp0*qp0);
    fvec m2 = 0.1395679f*0.1395679f;
    fvec v = 29.9792458f * sqrt(p2/(m2+p2));
    At[step] = sqrt(1.f + tx*tx + ty*ty)/v;
    At_tx[step] = tx/sqrt(1.f + tx*tx + ty*ty)/v;
    At_ty[step] = ty/sqrt(1.f + tx*tx + ty*ty)/v;
    
    step4 = step * 5;
    k[step4  ] = tx * h;
    k[step4+1] = ty * h;
    k[step4+2] = Ax[step] * qp0;
    k[step4+3] = Ay[step] * qp0;
    k[step4+4] = At[step] * h;
  }  // end of Runge-Kutta steps

  fvec initialised = ZERO;
  if(w) //TODO use operator {?:}
  {
    const fvec zero = ZERO;
    initialised = fvec( zero < *w );
  }
  else
  {
    const fvec one = ONE;
    const fvec zero = ZERO;
    initialised = fvec( zero < one );
  }
  
  {
    fx =   ( (x0[0]+c[0]*k[0]+c[1]*k[5+0]+c[2]*k[10+0]+c[3]*k[15+0]) & initialised) + ( (!initialised) & fx);
    fy =   ( (x0[1]+c[0]*k[1]+c[1]*k[5+1]+c[2]*k[10+1]+c[3]*k[15+1]) & initialised) + ( (!initialised) & fy);
    ftx =  ( (x0[2]+c[0]*k[2]+c[1]*k[5+2]+c[2]*k[10+2]+c[3]*k[15+2]) & initialised) + ( (!initialised) & ftx);
    fty =  ( (x0[3]+c[0]*k[3]+c[1]*k[5+3]+c[2]*k[10+3]+c[3]*k[15+3]) & initialised) + ( (!initialised) & fty);
    ft =   ( (x0[4]+c[0]*k[4]+c[1]*k[5+4]+c[2]*k[10+4]+c[3]*k[15+4]) & initialised) + ( (!initialised) & ft);
    fz =   ( z_out & initialised)  + ( (!initialised) & fz);
  }

  //
  //     Derivatives    dx/dqp
  //

  x0[0] = 0.f; x0[1] = 0.f; x0[2] = 0.f; x0[3] = 0.f; x0[4] = 0.f;

  //
  //   Runge-Kutta step for derivatives dx/dqp

  for (step = 0; step < 4; ++step) {
    for(i=0; i < 5; ++i) {
      if(step == 0) {
        x[i] = x0[i];
      } else
      {
        x[i] = x0[i] + b[step] * k1[step*5-5+i];
      }
    }
    step4 = step * 5;
    k1[step4  ] = x[2] * h;
    k1[step4+1] = x[3] * h;
    k1[step4+2] = Ax[step] + Ax_tx[step] * x[2] + Ax_ty[step] * x[3];
    k1[step4+3] = Ay[step] + Ay_tx[step] * x[2] + Ay_ty[step] * x[3];
    k1[step4+4] = At[step] + At_tx[step] * x[2] + At_ty[step] * x[3];

  }  // end of Runge-Kutta steps for derivatives dx/dqp

  fvec J[36];

  for (i = 0; i < 4; ++i ) {
    J[24+i]=x0[i]+c[0]*k1[i]+c[1]*k1[5+i]+c[2]*k1[10+i]+c[3]*k1[15+i];
  }
  J[28] = 1.;
  J[29]=x0[4]+c[0]*k1[4]+c[1]*k1[5+4]+c[2]*k1[10+4]+c[3]*k1[15+4];
  //
  //      end of derivatives dx/dqp
  //

  //     Derivatives    dx/tx
  //

  x0[0] = 0.f; x0[1] = 0.f; x0[2] = 1.f; x0[3] = 0.f; x0[4] = 0.f;

  //
  //   Runge-Kutta step for derivatives dx/dtx
  //

  for (step = 0; step < 4; ++step) {
    for(i=0; i < 5; ++i) {
      if(step == 0) {
        x[i] = x0[i];
      }
      else
        if ( i!=2 )
      {
        x[i] = x0[i] + b[step] * k1[step*5-5+i];
      }
    }
    step4 = step * 5;
    k1[step4  ] = x[2] * h;
    k1[step4+1] = x[3] * h;
        // k1[step4+2] = Ax_tx[step] * x[2] + Ax_ty[step] * x[3];
    k1[step4+3] = Ay_tx[step] * x[2] + Ay_ty[step] * x[3];
    k1[step4+4] = At_tx[step] * x[2] + At_ty[step] * x[3];
  }  // end of Runge-Kutta steps for derivatives dx/dtx

  for (i = 0; i < 4; ++i ) {
    if(i != 2)
    {
      J[12+i]=x0[i]+c[0]*k1[i]+c[1]*k1[5+i]+c[2]*k1[10+i]+c[3]*k1[15+i];
    }
  }
  //      end of derivatives dx/dtx
  J[14] = 1.f;
  J[16] = 0.f;
  J[17]=x0[4]+c[0]*k1[4]+c[1]*k1[5+4]+c[2]*k1[10+4]+c[3]*k1[15+4];
  
  //     Derivatives    dx/ty
  //

  x0[0] = 0.f; x0[1] = 0.f; x0[2] = 0.f; x0[3] = 1.f; x0[4] = 0.f;

  //
  //   Runge-Kutta step for derivatives dx/dty
  //

  for (step = 0; step < 4; ++step) {
    for(i=0; i < 5; ++i) {
      if(step == 0) {
        x[i] = x0[i];           // ty fixed
      } else 
        if(i!=3) {
        x[i] = x0[i] + b[step] * k1[step*5-5+i];
      }

    }
    step4 = step * 5;
    k1[step4  ] = x[2] * h;
    k1[step4+1] = x[3] * h;
    k1[step4+2] = Ax_tx[step] * x[2] + Ax_ty[step] * x[3];
    //    k1[step4+3] = Ay_tx[step] * x[2] + Ay_ty[step] * x[3];
    k1[step4+4] = At_tx[step] * x[2] + At_ty[step] * x[3];
  }  // end of Runge-Kutta steps for derivatives dx/dty

  for (i = 0; i < 3; ++i ) {
    J[18+i]=x0[i]+c[0]*k1[i]+c[1]*k1[5+i]+c[2]*k1[10+i]+c[3]*k1[15+i];
  }
  //      end of derivatives dx/dty
  J[21] = 1.;
  J[22] = 0.;
  J[23]=x0[4]+c[0]*k1[4]+c[1]*k1[5+4]+c[2]*k1[10+4]+c[3]*k1[15+4];
  //
  //    derivatives dx/dx and dx/dy

  for(i = 0; i < 12; ++i) 
    J[i] = 0.;
  J[0] = 1.; J[7] = 1.;
  for(i=30; i<35; i++) 
    J[i] = 0.f;
  J[35] = 1.f;

  fvec dqp = qp_in - qp0;
  
  { // update parameters
    fx += ((J[6*4+0]*dqp) & initialised);
    fy += ((J[6*4+1]*dqp) & initialised);
    ftx +=((J[6*4+2]*dqp) & initialised);
    fty +=((J[6*4+3]*dqp) & initialised);
    ft +=((J[6*4+5]*dqp) & initialised);
  }
   
  //          covariance matrix transport 
  
//   // if ( C_in&&C_out ) CbmKFMath::multQtSQ( 5, J, C_in, C_out); // TODO
//   j(0,2) = J[5*2 + 0];
//   j(1,2) = J[5*2 + 1];
//   j(2,2) = J[5*2 + 2];
//   j(3,2) = J[5*2 + 3];
//     
//   j(0,3) = J[5*3 + 0];
//   j(1,3) = J[5*3 + 1];
//   j(2,3) = J[5*3 + 2];
//   j(3,3) = J[5*3 + 3];
//     
//   j(0,4) = J[5*4 + 0];
//   j(1,4) = J[5*4 + 1];
//   j(2,4) = J[5*4 + 2];
//   j(3,4) = J[5*4 + 3];
  
  const fvec c42 = C42, c43 = C43;

  const fvec cj00 = C00 + C20*J[6*2 + 0] + C30*J[6*3 + 0] + C40*J[6*4 + 0];
  const fvec cj10 = C10 + C21*J[6*2 + 0] + C31*J[6*3 + 0] + C41*J[6*4 + 0];
  const fvec cj20 = C20 + C22*J[6*2 + 0] + C32*J[6*3 + 0] + c42*J[6*4 + 0];
  const fvec cj30 = C30 + C32*J[6*2 + 0] + C33*J[6*3 + 0] + c43*J[6*4 + 0];
  const fvec cj40 = C40 + C42*J[6*2 + 0] + C43*J[6*3 + 0] + C44*J[6*4 + 0];
  const fvec cj50 = C50 + C52*J[6*2 + 0] + C53*J[6*3 + 0] + C54*J[6*4 + 0];
 
//  const fvec cj01 = C10 + C20*J[6*2 + 1] + C30*J[6*3 + 1] + C40*J[6*4 + 1];
  const fvec cj11 = C11 + C21*J[6*2 + 1] + C31*J[6*3 + 1] + C41*J[6*4 + 1];
  const fvec cj21 = C21 + C22*J[6*2 + 1] + C32*J[6*3 + 1] + c42*J[6*4 + 1];
  const fvec cj31 = C31 + C32*J[6*2 + 1] + C33*J[6*3 + 1] + c43*J[6*4 + 1];
  const fvec cj41 = C41 + C42*J[6*2 + 1] + C43*J[6*3 + 1] + C44*J[6*4 + 1];
  const fvec cj51 = C51 + C52*J[6*2 + 1] + C53*J[6*3 + 1] + C54*J[6*4 + 1];

 // const fvec cj02 = C20*J[6*2 + 2] + C30*J[6*3 + 2] + C40*J[6*4 + 2];
 // const fvec cj12 = C21*J[6*2 + 2] + C31*J[6*3 + 2] + C41*J[6*4 + 2];
  const fvec cj22 = C22*J[6*2 + 2] + C32*J[6*3 + 2] + c42*J[6*4 + 2];
  const fvec cj32 = C32*J[6*2 + 2] + C33*J[6*3 + 2] + c43*J[6*4 + 2];
  const fvec cj42 = C42*J[6*2 + 2] + C43*J[6*3 + 2] + C44*J[6*4 + 2];
  const fvec cj52 = C52*J[6*2 + 2] + C53*J[6*3 + 2] + C54*J[6*4 + 2];

 // const fvec cj03 = C20*J[6*2 + 3] + C30*J[6*3 + 3] + C40*J[6*4 + 3];
 // const fvec cj13 = C21*J[6*2 + 3] + C31*J[6*3 + 3] + C41*J[6*4 + 3];
  const fvec cj23 = C22*J[6*2 + 3] + C32*J[6*3 + 3] + c42*J[6*4 + 3];
  const fvec cj33 = C32*J[6*2 + 3] + C33*J[6*3 + 3] + c43*J[6*4 + 3];
  const fvec cj43 = C42*J[6*2 + 3] + C43*J[6*3 + 3] + C44*J[6*4 + 3];
  const fvec cj53 = C52*J[6*2 + 3] + C53*J[6*3 + 3] + C54*J[6*4 + 3];
  
  const fvec cj24 = C42;
  const fvec cj34 = C43;
  const fvec cj44 = C44;
  const fvec cj54 = C54;   
   
 // const fvec cj05 = C50 + C20*J[17] + C30*J[23] + C40*J[29];
 // const fvec cj15 = C51 + C21*J[17] + C31*J[23] + C41*J[29];
  const fvec cj25 = C52 + C22*J[17] + C32*J[23] + C42*J[29];
  const fvec cj35 = C53 + C32*J[17] + C33*J[23] + C43*J[29];
  const fvec cj45 = C54 + C42*J[17] + C43*J[23] + C44*J[29];
  const fvec cj55 = C55 + C52*J[17] + C53*J[23] + C54*J[29];
  

  C00 = ((cj00 + cj20*J[12] + cj30*J[18] + cj40*J[24]) & initialised) + ((!initialised) & C00);
  
  C10 = ((cj10 + cj20*J[13] + cj30*J[19] + cj40*J[25]) & initialised) + ((!initialised) & C10);
  C11 = ((cj11 + cj21*J[13] + cj31*J[19] + cj41*J[25]) & initialised) + ((!initialised) & C11);
  
  C20 = ((cj20 +              cj30*J[20] + cj40*J[26]) & initialised) + ((!initialised) & C20);
  C21 = ((cj21 +              cj31*J[20] + cj41*J[26]) & initialised) + ((!initialised) & C21);  
  C22 = ((cj22 +              cj32*J[20] + cj42*J[26]) & initialised) + ((!initialised) & C22);
  
  C30 = ((cj30 + cj20*J[15] + cj40*J[27]) & initialised) + ((!initialised) & C30);
  C31 = ((cj31 + cj21*J[15] + cj41*J[27]) & initialised) + ((!initialised) & C31);
  C32 = ((cj32 + cj22*J[15] + cj42*J[27]) & initialised) + ((!initialised) & C32);
  C33 = ((cj33 + cj23*J[15] + cj43*J[27]) & initialised) + ((!initialised) & C33);
  
  C40 = ((cj40) & initialised) + ((!initialised) & C40);
  C41 = ((cj41) & initialised) + ((!initialised) & C41);
  C42 = ((cj42) & initialised) + ((!initialised) & C42);
  C43 = ((cj43) & initialised) + ((!initialised) & C43);
  C44 = ((cj44) & initialised) + ((!initialised) & C44);
  
  C50 = ((cj50 + cj20*J[17] + cj30*J[23] + cj40*J[29]) & initialised) + ((!initialised) & C50);
  C51 = ((cj51 + cj21*J[17] + cj31*J[23] + cj41*J[29]) & initialised) + ((!initialised) & C51);
  C52 = ((cj52 + cj22*J[17] + cj32*J[23] + cj42*J[29]) & initialised) + ((!initialised) & C52);
  C53 = ((cj53 + cj23*J[17] + cj33*J[23] + cj43*J[29]) & initialised) + ((!initialised) & C53);
  C54 = ((cj54 + cj24*J[17] + cj34*J[23] + cj44*J[29]) & initialised) + ((!initialised) & C54);
  C55 = ((cj55 + cj25*J[17] + cj35*J[23] + cj45*J[29]) & initialised) + ((!initialised) & C55);
}

void L1TrackParFit::L1AddPipeMaterial(fvec qp0, fvec w, fvec mass2)
{
  cnst  ONE = 1.f;

//  static const fscal RadThick=0.0009f;//0.5/18.76;
//  static const fscal logRadThick=log(RadThick);
  //const fscal RadThick=0.0009f;//0.5/18.76;

  const fscal logRadThick=log(PipeRadThick[0]);
  fvec tx = ftx;
  fvec ty = fty;
  fvec txtx = ftx*ftx;
  fvec tyty = fty*fty;
  fvec txtx1 = txtx + ONE;
  fvec h = txtx + tyty;
  fvec t = sqrt(txtx1 + tyty);
  fvec h2 = h*h;
  fvec qp0t = qp0*t;

  cnst c1=0.0136f, c2=c1*0.038f, c3=c2*0.5f, c4=-c3/2.0f, c5=c3/3.0f, c6=-c3/4.0f;
  fvec s0 = (c1+c2*fvec(logRadThick) + c3*h + h2*(c4 + c5*h +c6*h2) )*qp0t;
  //fvec a = ( (ONE+mass2*qp0*qp0t)*RadThick*s0*s0 );
  fvec a = ( (t+mass2*qp0*qp0t)*PipeRadThick*s0*s0 );

  C22 += w*txtx1*a;
  C32 += w*tx*ty*a; C33 += w*(ONE+tyty)*a;
}


void L1TrackParFit::L1AddMaterial( fvec radThick, fvec qp0, fvec w, fvec mass2 )
{
  cnst ONE = 1.;

  fvec tx = ftx;
  fvec ty = fty;
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

  C22 += w*txtx1*a;
  C32 += w*tx*ty*a; 
  C33 += w*(ONE+tyty)*a;
  
}


void L1TrackParFit::L1AddMaterial(L1MaterialInfo &info, fvec qp0, fvec w, fvec mass2)
{
  cnst  ONE = 1.f;

  fvec tx = ftx;
  fvec ty = fty;
 // fvec time = ft;
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

  C22 += w*txtx1*a;
  C32 += w*tx*ty*a; 
  C33 += w*(ONE+tyty)*a;


}


void L1TrackParFit::EnergyLossCorrection(const fvec& mass2, const fvec& radThick, fvec& qp0, fvec direction, fvec w)
{
  const fvec& p2 = 1.f/(fqp*fqp);
  const fvec& E2 = mass2 + p2;

  const fvec& bethe = ApproximateBetheBloch( p2/mass2 );

  fvec tr = sqrt(fvec(1.f) + ftx*ftx + fty*fty) ;
  
  const fvec& dE = bethe * radThick*tr * 2.33f * 9.34961f;

  const fvec& E2Corrected = (sqrt(E2) + direction*dE) * (sqrt(E2) + direction*dE);
  fvec corr = sqrt( p2/( E2Corrected - mass2 ) );
  fvec init = fvec(!(corr == corr)) | fvec(w<1);
  corr = fvec(fvec(1.f) & init ) + fvec(corr & fvec(!(init)));

  qp0   *= corr;
  fqp  *= corr;
  C40 *= corr;
  C41 *= corr;
  C42 *= corr;
  C43 *= corr;
//   C54 *= corr;
  C44 *= corr * corr;
}

#undef cnst


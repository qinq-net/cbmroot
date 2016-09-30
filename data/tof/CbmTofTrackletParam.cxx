// -------------------------------------------------------------------------
// -----                     FairTrackParam source file                 -----
// -----                  Created 27/01/05  by V. Friese               -----
// -------------------------------------------------------------------------

#include "CbmTofTrackletParam.h"

#include "FairLogger.h"

Double_t CbmTofTrackletParam::GetZr(Double_t R) const 
{ 
  Double_t P = (fTx+fTy);
  Double_t Q =  0.5*(fX*fX + fY*fY - R*R);
  Double_t Arg = P*P*0.25 - Q ;
  LOG(INFO) << " GetZr "<<R<<", P "<<P<<", Q "<<Q<<", Arg "<<Arg
            << FairLogger::endl; 

  if ( Arg > 0.) { 
    Double_t z=-P*0.5 + std::sqrt(Arg);
    LOG(INFO) << " GetZr "<<R<<", P "<<P<<", Q "<<Q<<", Arg "<<Arg<<", z "<<z
              << FairLogger::endl; 
    return z;
  }
  return 0.; 
}



ClassImp(CbmTofTrackletParam)



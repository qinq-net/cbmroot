#ifndef CBMLITCONVERTERFAIRTRACKPARAM_H_
#define CBMLITCONVERTERFAIRTRACKPARAM_H_

#include "base/CbmLitTypes.h"
#include "base/CbmLitEnums.h"

//#include "data/CbmLitHit.h"
//#include "data/CbmLitPixelHit.h"
//#include "data/CbmLitStripHit.h"
//#include "data/CbmLitTrack.h"
#include "data/CbmLitTrackParam.h"
//#include "data/CbmLitFitNode.h"
//#include "data/CbmLitTofTrack.h"

//#include "CbmTrack.h"
//#include "CbmTofTrack.h"
//#include "CbmHit.h"
//#include "CbmPixelHit.h"
//#include "CbmStripHit.h"
#include "CbmTrackParam.h"
//#include "CbmStsTrack.h"
//#include "CbmTrdTrack.h"
//#include "CbmMuchStrawHit.h"
//#include "CbmMuchGeoScheme.h"
//#include "CbmMvdHit.h"
//#include "CbmMuchTrack.h"

//#include "CbmGlobalTrack.h"
//#include "CbmTrdAddress.h"

#include "TClonesArray.h"

#include <iostream>
#include <cmath>
#include <set>
#include <cassert>


class CbmLitConverterFairTrackParam
{
public:
    
    static void FairTrackParamToCbmLitTrackParam(
                                                 const FairTrackParam* par,
                                                 CbmLitTrackParam* litPar)
    {
        litPar->SetX((litfloat)par->GetX());
        litPar->SetY((litfloat)par->GetY());
        litPar->SetZ((litfloat)par->GetZ());
        litPar->SetTx((litfloat)par->GetTx());
        litPar->SetTy((litfloat)par->GetTy());
        litPar->SetQp((litfloat)par->GetQp());
        litPar->SetTime(0);
        Double_t cov[15];
        par->CovMatrix(cov);
        std::vector<litfloat> covVec(15);
        for (UInt_t i = 0; i < 15; ++i) { covVec[i] = (litfloat) cov[i]; }
        litPar->SetCovMatrix(covVec);
        litPar->SetTimeError(4);
    }
    
    static void FairTrackParamToCbmLitTrackParam(
                                                 const CbmTrackParam* par,
                                                 CbmLitTrackParam* litPar)
    {
        litPar->SetX((litfloat)par->GetX());
        litPar->SetY((litfloat)par->GetY());
        litPar->SetZ((litfloat)par->GetZ());
        litPar->SetTx((litfloat)par->GetTx());
        litPar->SetTy((litfloat)par->GetTy());
        litPar->SetQp((litfloat)par->GetQp());
        litPar->SetTime((litfloat)par->GetTime());
        Double_t cov[15];
        par->CovMatrix(cov);
        std::vector<litfloat> covVec(15);
        for (UInt_t i = 0; i < 15; ++i) { covVec[i] = (litfloat) cov[i]; }
        litPar->SetCovMatrix(covVec);
        litPar->SetTimeError((litfloat)par->GetDTime());
    }
    
    static void CbmLitTrackParamToFairTrackParam(
                                                 const CbmLitTrackParam* litPar,
                                                 FairTrackParam* par)
    {
        par->SetX(litPar->GetX());
        par->SetY(litPar->GetY());
        par->SetZ(litPar->GetZ());
        par->SetTx(litPar->GetTx());
        par->SetTy(litPar->GetTy());
        par->SetQp(litPar->GetQp());
        Double_t cov[15];
        std::vector<litfloat> covVec(litPar->GetCovMatrix());
        for (UInt_t i = 0; i < 15; ++i) { cov[i] = (Double_t) covVec[i]; }
        par->SetCovMatrix(cov);
    }

};

#endif /*CBMLITCONVERTERFAIRTRACKPARAM_H_*/

#include "CbmEcalPointLite.h"

#include "FairLogger.h"

void CbmEcalPointLite::Print(const Option_t* /*opt*/) const
{
  LOG(INFO) << "CbmEcalPointLite: DetectorID=" << GetDetectorID() 
            << ", TrackID=" << GetTrackID() 
            << ", Eloss=" << GetEnergyLoss() 
            << ", Time=" << GetTime() << FairLogger::endl;
}

ClassImp(CbmEcalPointLite)

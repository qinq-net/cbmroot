/** CbmMCEpoch.cxx
 *@author Volker Friese v.friese@gsi.de
 *@since 13.11.2009
 *@version 1.0
 **/



#include "CbmMCEpoch.h"
#include "CbmModuleList.h"

#include "FairLogger.h"

#include "TClonesArray.h"

// -----    Default  constructor   -------------------------------------------
CbmMCEpoch::CbmMCEpoch() 
  : TNamed("MCEpoch", "MCEpoch"),
    fStartTime(0.),
    fEpochLength(0.),
    fPoints()
{
  CreateArrays();
}
// ---------------------------------------------------------------------------



// -----    Standard  constructor   ------------------------------------------
CbmMCEpoch::CbmMCEpoch(Double_t startTime,
		       Double_t epochLength) 
  : TNamed("MCEpoch", "MCEpoch"),
    fStartTime(startTime),
    fEpochLength(epochLength),
    fPoints()
{
  CreateArrays();
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmMCEpoch::~CbmMCEpoch() {
}
// ---------------------------------------------------------------------------



// -----   Add a MCPoint to the epoch   --------------------------------------
void CbmMCEpoch::AddPoint(ECbmModuleId det, FairMCPoint* point,
			  Int_t eventId, Double_t eventTime) {

  switch (det) {

  case kSts: {
    CbmStsPoint* stsPoint = (CbmStsPoint*) point;
    new ((*(fPoints[det]))[GetNofPoints(det)]) CbmStsPoint(*stsPoint,
							   eventId,
							   eventTime,
							   fStartTime);
  }
    break;

  case kMuch: {
    CbmMuchPoint* muchPoint = (CbmMuchPoint*) point;
    new ((*(fPoints[det]))[GetNofPoints(det)]) CbmMuchPoint(*muchPoint,
							    eventId,
							    eventTime,
							    fStartTime);
  }
    break;

  default: 
    LOG(FATAL) << "The functionality is not yet implemented for the detector with id " << det << FairLogger::endl;

  }

}
// ---------------------------------------------------------------------------



// -----   Clear epoch   -----------------------------------------------------
void CbmMCEpoch::Clear(Option_t*) {

  /* Note: The loop over the detetcor id works only if the corresponding
   * enum is continuous. Did not find a better solution yet. V.F.  */

  fStartTime = 0.;
  for (Int_t iDet=kRef; iDet<kNofSystems; iDet++) {
    ECbmModuleId det = ECbmModuleId(iDet);
    if ( fPoints[det] ) fPoints[det]->Clear("C");
  }

}
// ---------------------------------------------------------------------------



// -----   Get number of points in epoch   -----------------------------------
Int_t CbmMCEpoch::GetNofPoints(ECbmModuleId det) const {

  if ( ! fPoints[det] ) {
    LOG(WARNING) << "No array for detector system " << det << FairLogger::endl;
    return 0;
  }

  return fPoints[det]->GetEntriesFast();

}
// ---------------------------------------------------------------------------



// -----   Get a MCPoint from the array   ------------------------------------
FairMCPoint* CbmMCEpoch::GetPoint(ECbmModuleId det, Int_t index) {

  if ( ! fPoints[det] ) {
    LOG(WARNING) << "No array for detector system " << det << FairLogger::endl;
    return NULL;
  }

  if ( index < 0  ||  index >= GetNofPoints(det) ) {
    LOG(WARNING) << "Index " << index 
	 << "out of range for system " << det << FairLogger::endl;
    return NULL;
  }

  return ( (FairMCPoint*) fPoints[det]->At(index) );

}
// ---------------------------------------------------------------------------
      



// -----   Check for empty epoch   -------------------------------------------
Bool_t CbmMCEpoch::IsEmpty() {

  Int_t nTotal = 0;

  for (Int_t iDet=kRef; iDet<kNofSystems; iDet++) {
    ECbmModuleId det = ECbmModuleId(iDet);
    if ( fPoints[iDet] ) nTotal += GetNofPoints(det);
  }

  if (nTotal) return kFALSE;
  return kTRUE;

}
// ---------------------------------------------------------------------------



// -----   Print epoch info   ------------------------------------------------
void CbmMCEpoch::Print(Option_t* /*opt*/) const { 

  LOG(INFO) << " Start time " << fStartTime << ", Points: ";
  TString sysName;
  for (Int_t iDet = kRef; iDet<kNofSystems; iDet++) {
    ECbmModuleId det = ECbmModuleId(iDet);
    if ( fPoints[iDet] ) {
      sysName = CbmModuleList::GetModuleName(ECbmModuleId(iDet));
      LOG(INFO)<< "   " << sysName << " " << GetNofPoints(det) << " ";
    }
  }
  LOG(INFO) << FairLogger::endl;

}
// ---------------------------------------------------------------------------



// -----   Create MCPoint arrays   -------------------------------------------
void CbmMCEpoch::CreateArrays() {

  for (Int_t iDet=kRef; iDet<kNofSystems; iDet++) {
    ECbmModuleId det = ECbmModuleId(iDet);
    switch(det) {
    case kSts:  fPoints[det] = new TClonesArray("CbmStsPoint", 1000); break;
    case kMuch: fPoints[det] = new TClonesArray("CbmMuchPoint", 1000); break;
    default:    fPoints[det] = NULL; break;
    }
  }

}
// ---------------------------------------------------------------------------


ClassImp(CbmMCEpoch)


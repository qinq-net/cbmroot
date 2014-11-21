/*
 * CbmTimeSlice.cxx
 *
 *  Created on: 17.07.2012
 *      Author: friese
 */

#include <iomanip>
#include <sstream>

#include "CbmTimeSlice.h"

#include "CbmDetectorList.h"

#include "FairLogger.h"

//#include <iostream>


// -----   Default constructor   ---------------------------------------------
CbmTimeSlice::CbmTimeSlice() 
 : TNamed(),
   fStartTime(0.), 
   fDuration(0.),
   fIsEmpty(kTRUE),
   fStsData(),
   fMuchData()
{
}
// ---------------------------------------------------------------------------



// -----   Standard constructor   --------------------------------------------
CbmTimeSlice::CbmTimeSlice(Double_t start, Double_t duration)
 : TNamed(),
   fStartTime(start), 
   fDuration(duration),
   fIsEmpty(kTRUE),
   fStsData(),
   fMuchData()
{
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmTimeSlice::~CbmTimeSlice() {
}
// ---------------------------------------------------------------------------



// -----   Get data object   ------------------------------------------------
CbmDigi* CbmTimeSlice::GetData(DetectorId iDet, Int_t index) {

  CbmDigi* digi = NULL;

  switch (iDet) {

    case kSTS:
      if ( index < fStsData.size() ) digi = &(fStsData[index]);
      break;
    case kMUCH:
      if ( index < fMuchData.size() ) digi = &(fMuchData[index]);
      break;
    default:
      break;
  }

  return digi;
}
// ---------------------------------------------------------------------------


// -----   Get data array size   ----------------------------------------------
Int_t CbmTimeSlice::GetDataSize(DetectorId iDet) const {
  switch (iDet) {
    case kSTS:  return fStsData.size();
    case kMUCH: return fMuchData.size();
    default:    return 0;
  }

  return 0;
}
// ---------------------------------------------------------------------------


// -----   Copy data into the time slice   -----------------------------------
void CbmTimeSlice::InsertData(CbmDigi* data) {


  // --- Check whether time of data fits into time slice
  if ( data->GetTime() < fStartTime || data->GetTime() > GetEndTime() ) {
    LOG(ERROR) << "Attempt to insert data at t = " << fixed
               << setprecision(3) << data->GetTime()
               << " ns into time slice [" << fStartTime << ", "
               << GetEndTime() << "] !" << FairLogger::endl;
    return;
  }


  // --- If yes, copy the object into the STL vector
  Int_t iDet = data->GetSystemId();
  switch ( iDet ) {

    case kSTS: {
      CbmStsDigi* digi = static_cast<CbmStsDigi*>(data);
      fStsData.push_back(*digi);
      fIsEmpty = kFALSE;
      break;
    }

    case kMUCH: {
      CbmMuchDigi* digi = static_cast<CbmMuchDigi*>(data);
      fMuchData.push_back(*digi);
      fIsEmpty = kFALSE;
    }
    break;

    default:
      TString sysName;
      CbmDetectorList::GetSystemName(iDet, sysName);
      LOG(WARNING) << "CbmTimeSlice: System " << sysName
                   << " is not implemented yet!" << FairLogger::endl;
      break;

  }  // detector switch
}
// ---------------------------------------------------------------------------



// ----- Reset time slice   --------------------------------------------------
void CbmTimeSlice::Reset(Double_t start, Double_t duration) {

  fStsData.clear();
  fMuchData.clear();
  fIsEmpty = kTRUE;
  fStartTime = start;
  fDuration = duration;

}
// ---------------------------------------------------------------------------



// -----   Status info   -----------------------------------------------------
string CbmTimeSlice::ToString() const {
  stringstream ss;
  ss << "TimeSlice: interval [" << fixed << setprecision(3) << fStartTime
  		<< ", " << GetEndTime() << "] ns, Data:";
  ss << " STS "  << fStsData.size();
  ss << " MUCH " << fMuchData.size();
  return ss.str();
}
// ---------------------------------------------------------------------------


ClassImp(CbmTimeSlice)

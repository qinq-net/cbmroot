/*
 * CbmTimeSlice.cxx
 *
 *  Created on: 17.07.2012
 *      Author: friese
 */

#include <iomanip>
#include <sstream>

#include "CbmTimeSlice.h"

#include "FairLogger.h"
#include "CbmModuleList.h"

using std::stringstream;
using std::fixed;
using std::string;
using std::setprecision;


// -----   Constructor for flexible time-slice   ----------------------------
CbmTimeSlice::CbmTimeSlice() 
 : TNamed(),
   fStartTime(-1.),
   fLength(-1.),
   fIsFlexible(kTRUE),
   fIsEmpty(kTRUE),
   fNofData(),
   fTimeDataFirst(0.),
   fTimeDataLast(0.),
   fMatch()
{
}
// ---------------------------------------------------------------------------



// -----   Constructor for fixed-length time-slice   -------------------------
CbmTimeSlice::CbmTimeSlice(Double_t start, Double_t duration)
 : TNamed(),
   fStartTime(start), 
   fLength(duration),
   fIsFlexible(kFALSE),
   fIsEmpty(kTRUE),
   fNofData(),
   fTimeDataFirst(0.),
   fTimeDataLast(0.),
   fMatch()
{
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmTimeSlice::~CbmTimeSlice() {
}
// ---------------------------------------------------------------------------



// -----   Add data with time stamp   ----------------------------------------
Bool_t CbmTimeSlice::AddData(Int_t detector, Double_t time) {

  // Check for out of bounds
  if ( ! fIsFlexible ) {
    if ( time < fStartTime || time > GetEndTime() )  {
      LOG(ERROR) << "Trying to add data at t = " << time << " ns to "
          << "time slice [ " << fStartTime << ", " << GetEndTime()
          << " ] ns !" << FairLogger::endl;
      return kFALSE;
    } //? out of bounds
  } //? fixed-length

  // Update bookkeeping
  if ( fIsEmpty ) {
    fTimeDataFirst = time;
    fTimeDataLast = time;
  } //? time-slice empty
  else {
    if ( time < fTimeDataFirst ) fTimeDataFirst = time;
    if ( time > fTimeDataLast ) fTimeDataLast = time;
  } //? time-slice not empty

  fNofData[detector]++;
  fIsEmpty = kFALSE;

  return kTRUE;
}
// ---------------------------------------------------------------------------



// -----   Get data array size   ----------------------------------------------
Int_t CbmTimeSlice::GetNofData(Int_t detector) const {
  auto it = fNofData.find(detector);
  if ( it == fNofData.end() ) return 0;
  return it->second;
}
// ---------------------------------------------------------------------------



// ----- Reset time slice   --------------------------------------------------
void CbmTimeSlice::Reset(Double_t start, Double_t length) {
  fNofData.clear();
  fIsEmpty = kTRUE;
  fStartTime = start;
  fLength = length;
  fTimeDataFirst = 0.;
  fTimeDataLast = 0.;
  fMatch.ClearLinks();
}
// ---------------------------------------------------------------------------



// -----   Status info   -----------------------------------------------------
string CbmTimeSlice::ToString() const {
  stringstream ss;
  if ( fIsFlexible) ss << "Time slice [flexible], data: ";
  else ss << "Time slice [" << fixed << setprecision(3) << fStartTime
      << ", " << GetEndTime() << "] ns, data: ";
  if ( IsEmpty() ) ss << "empty";
  else {
    ss << "[" << fixed << setprecision(3) << fTimeDataFirst
        << ", " << fTimeDataLast << "] ns, ";
    for ( auto it = fNofData.begin(); it != fNofData.end(); it++) {
      if ( it->second ) ss << CbmModuleList::GetModuleNameCaps(it->first)
    << " " << it->second << " ";
    } //# detectors
  } //? time-slice not empty
  return ss.str();
}
// ---------------------------------------------------------------------------


ClassImp(CbmTimeSlice)

/*
 * CbmTimeSlice.cxx
 *
 *  Created on: 17.07.2012
 *      Author: friese
 */

#include <iomanip>
#include <sstream>

#include "CbmTimeSlice.h"

#include "CbmModuleList.h"

using std::stringstream;
using std::fixed;
using std::string;
using std::setprecision;


// -----   Default constructor   ---------------------------------------------
CbmTimeSlice::CbmTimeSlice() 
 : TNamed(),
   fStartTime(0.), 
   fDuration(0.),
   fIsEmpty(kTRUE),
   fMatch()
{
}
// ---------------------------------------------------------------------------



// -----   Standard constructor   --------------------------------------------
CbmTimeSlice::CbmTimeSlice(Double_t start, Double_t duration)
 : TNamed(),
   fStartTime(start), 
   fDuration(duration),
   fIsEmpty(kTRUE),
   fMatch()
{
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmTimeSlice::~CbmTimeSlice() {
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
void CbmTimeSlice::Reset(Double_t start, Double_t duration) {
  fNofData.clear();
  fIsEmpty = kTRUE;
  fStartTime = start;
  fDuration = duration;
  fMatch.ClearLinks();
}
// ---------------------------------------------------------------------------



// -----   Status info   -----------------------------------------------------
string CbmTimeSlice::ToString() const {
  stringstream ss;
  if ( fDuration < 0. ) ss << "TimeSlice: interval flexible, Data: ";
  else ss << "TimeSlice: interval [" << fixed << setprecision(3) << fStartTime
      << ", " << GetEndTime() << "] ns, Data: ";
  if ( IsEmpty() ) ss << "empty";
  else for ( auto it = fNofData.begin(); it != fNofData.end(); it++) {
    if ( it->second ) ss << CbmModuleList::GetModuleNameCaps(it->first)
    << " " << it->second << " ";
  }
  return ss.str();
}
// ---------------------------------------------------------------------------


ClassImp(CbmTimeSlice)

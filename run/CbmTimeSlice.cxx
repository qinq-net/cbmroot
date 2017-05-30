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

//#include <iostream>
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
   fStsData(),
   fMuchData(),
   fTofData(),
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
   fStsData(),
   fMuchData(),
   fTofData(),
   fMatch()
{
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmTimeSlice::~CbmTimeSlice() {
}
// ---------------------------------------------------------------------------



// -----   Get data object   ------------------------------------------------
CbmDigi* CbmTimeSlice::GetData(ECbmModuleId iDet, UInt_t index) {

  CbmDigi* digi = NULL;

  switch (iDet) {

    case kSts:
      if ( index < fStsData.size() ) digi = &(fStsData[index]);
      break;
    case kMuch:
      if ( index < fMuchData.size() ) digi = &(fMuchData[index]);
      break;
    case kTof:
      if ( index < fTofData.size() ) digi = &(fTofData[index]);
      break;
    default:
      break;
  }

  return digi;
}
// ---------------------------------------------------------------------------


// -----   Get data array size   ----------------------------------------------
Int_t CbmTimeSlice::GetDataSize(ECbmModuleId iDet) const {
  switch (iDet) {
    case kSts:  return fStsData.size();
    case kMuch: return fMuchData.size();
    case kTof:  return fTofData.size();
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

  // --- Update match object, if links are present in the data
  // --- The match counts the data from each event, each with weight = 1.
  // --- This was chosen because the relative weighting between the
  // --- detector systems is unclear.
  CbmMatch* dataMatch = data->GetMatch();
  if ( dataMatch) {
  	for ( Int_t iLink = 0; iLink < dataMatch->GetNofLinks(); iLink++ ) {
  		Int_t event = dataMatch->GetLink(iLink).GetEntry();
  		Int_t file  = dataMatch->GetLink(iLink).GetFile();
  		fMatch.AddLink(1., -1, event, file);
  	}
  }

  // --- Copy the data object into the STL vector
  Int_t iDet = data->GetSystemId();
  switch ( iDet ) {

    case kSts: {
      CbmStsDigi* digi = static_cast<CbmStsDigi*>(data);
      fStsData.push_back(*digi);
      fIsEmpty = kFALSE;
      break;
    }

    case kMuch: {
      CbmMuchDigi* digi = static_cast<CbmMuchDigi*>(data);
      fMuchData.push_back(*digi);
      fIsEmpty = kFALSE;
      break;
   }

    case kTof: {
      CbmTofDigiExp* digi = static_cast<CbmTofDigiExp*>(data);
      fTofData.push_back(*digi);
      fIsEmpty = kFALSE;
      break;
   }

    default:
      TString sysName = CbmModuleList::GetModuleName(iDet);
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
  fTofData.clear();
  fIsEmpty = kTRUE;
  fStartTime = start;
  fDuration = duration;
  fMatch.ClearLinks();

}
// ---------------------------------------------------------------------------



// -----   Consistency check   -----------------------------------------------
Bool_t CbmTimeSlice::SelfTest() {

	// --- Skip if empty
	if ( IsEmpty() ) {
		LOG(DEBUG) << "TimeSlice: time slice is empty; self test OK."
				      << FairLogger::endl;
		return kTRUE;
	}

	LOG(DEBUG) << "TimeSlice: performing self test"
			       << FairLogger::endl;
	Bool_t status = kTRUE;

	// --- Check STS
	Double_t tCurrent  = -1.;
	Double_t tPrevious = -1.;
	for (Int_t index = 0; index < GetDataSize(kSts); index++) {
		tCurrent = GetData(kSts, index)->GetTime();
		if ( tCurrent < GetStartTime() ) {
			LOG(WARNING) << "STS index " << index << ": t = " << tCurrent
					         << " ns is before time slice start = " << GetStartTime()
					         << " ns " << FairLogger::endl;
			status = kFALSE;
		} //? Time before time slice start
		if ( tCurrent > GetEndTime() ) {
			LOG(WARNING) << "STS index " << index << ": t = " << tCurrent
					         << " ns is after time slice end " << GetEndTime()
					         << " ns" << FairLogger::endl;
			status = kFALSE;
		} //? Time after time slice end
		if ( tCurrent < tPrevious ) {
			LOG(WARNING) << "STS index " << index << ": t = " << tCurrent
					         << " ns, previous was t = " << tPrevious
					         << " ns" << FairLogger::endl;
			status = kFALSE;
		} //? Time before last data
	} // STS data loop
	LOG(DEBUG) << "\t STS: data " << GetDataSize(kSts)
			      << ( status ? " OK" : " FAILED" ) << FairLogger::endl;

	LOG(DEBUG) << "TimeSlice: self test"
			       << ( status ? " OK" : " FAILED" ) << FairLogger::endl;
	return status;
}
// ---------------------------------------------------------------------------



// -----   Status info   -----------------------------------------------------
string CbmTimeSlice::ToString() const {
  stringstream ss;
  ss << "TimeSlice: interval [" << fixed << setprecision(3) << fStartTime
  		<< ", " << GetEndTime() << "] ns, Data:";
  ss << " STS "  << fStsData.size();
  ss << " MUCH " << fMuchData.size();
  ss << " TOF "  << fMuchData.size();
  return ss.str();
}
// ---------------------------------------------------------------------------


ClassImp(CbmTimeSlice)

/** @file CbmMCBuffer.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17 July 2012
 **/


#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "TString.h"

#include "FairLogger.h"

#include "CbmDaqBuffer.h"
#include "CbmModuleList.h"

using std::setprecision;
using std::stringstream;
using std::fixed;
using std::string;
using std::pair;
using std::multimap;

// -----   Initialisation of static variables   ------------------------------
CbmDaqBuffer* CbmDaqBuffer::fgInstance = NULL;
// ---------------------------------------------------------------------------



// -----   Constructor   -----------------------------------------------------
CbmDaqBuffer::CbmDaqBuffer() {
}
// ---------------------------------------------------------------------------


// -----   Destructor   ------------------------------------------------------
CbmDaqBuffer::~CbmDaqBuffer() {
}
// ---------------------------------------------------------------------------



// -----   Time of first raw data   ------------------------------------------
Double_t CbmDaqBuffer::GetFirstTime() const {
  Double_t time = -1.;
  Bool_t firstDetector = kTRUE;
  for (Int_t iDet = kRef; iDet < kNofSystems; iDet++) {
    if ( GetSize(iDet) ) {
      if ( firstDetector ) {
        time = GetFirstTime(iDet);
        firstDetector = kFALSE;
      } //? first detector with data
      else time = ( time < GetFirstTime(iDet) ? time : GetFirstTime(iDet) );
    }  //? detector has data
  } //# detectors
  return time;
}
// ---------------------------------------------------------------------------



// -----   Time of first raw data for detector  ------------------------------
Double_t CbmDaqBuffer::GetFirstTime(Int_t iDet) const {
  if ( iDet < kRef || iDet >= kNofSystems ) return -1.;
  if ( ! GetSize(iDet) ) return -1.;
  assert ( (fData[iDet].begin())->second );
  return (fData[iDet].begin())->second->GetTime();
}
// ---------------------------------------------------------------------------



// -----   Time of last raw data   -------------------------------------------
Double_t CbmDaqBuffer::GetLastTime() const {
  Double_t time = -1.;
  Bool_t firstDetector = kTRUE;
  for (Int_t iDet = kRef; iDet < kNofSystems; iDet++) {
    if ( GetSize(iDet) ) {
      if ( firstDetector ) {
        time = GetLastTime(iDet);
        firstDetector = kFALSE;
      } //? first detector
      else time = ( time > GetLastTime(iDet) ? time : GetLastTime(iDet) );
    } //? detector has data
  } //# detectors
  return time;
}
// ---------------------------------------------------------------------------



// -----   Time of last raw data for detector  -------------------------------
Double_t CbmDaqBuffer::GetLastTime(Int_t iDet) const {
  if ( iDet < kRef || iDet >= kNofSystems ) return -1.;
  if ( ! GetSize(iDet) ) return -1.;
  assert ( (--fData[iDet].begin())->second );
  return (--fData[iDet].end())->second->GetTime();
}
// ---------------------------------------------------------------------------



// -----   Access to next data   ---------------------------------------------
CbmDigi* CbmDaqBuffer::GetNextData(Int_t iDet) {

  // --- Check for system ID
  if ( iDet >= kNofSystems ) {
    LOG(WARNING) << "DaqBuffer: Illegal system ID " << iDet
                 << FairLogger::endl;
    return NULL;
  }

  // --- Check for empty buffer
  if ( ! fData[iDet].size() ) return NULL;

  // --- Get data from buffer
  CbmDigi* digi = NULL;
  multimap<Double_t, CbmDigi*>::iterator it = fData[iDet].begin();
  CbmDigi* test = it->second;
  digi = test;
  fData[iDet].erase(it);

  return digi;
}
// ---------------------------------------------------------------------------



// -----   Access to next data with time limit  ------------------------------
CbmDigi* CbmDaqBuffer::GetNextData(Int_t iDet, Double_t time) {


  // --- Check for system ID
  if ( iDet >= kNofSystems ) {
    LOG(WARNING) << "DaqBuffer: Illegal system ID " << iDet
                 << FairLogger::endl;
    return NULL;
  }

  // --- Check for empty buffer
  if ( ! fData[iDet].size() ) return NULL;

  // --- Get data from buffer
  CbmDigi* digi = NULL;
  multimap<Double_t, CbmDigi*>::iterator it = fData[iDet].begin();
  CbmDigi* test = it->second;
  if ( test->GetTime() < time ) {
    digi = test;
    fData[iDet].erase(it);
  }
  return digi;

}
// ---------------------------------------------------------------------------



// -----   Number of objects in buffer   -------------------------------------
Int_t CbmDaqBuffer::GetSize() const {

  Int_t size = 0;
  for (Int_t iDet = kRef; iDet < kNofSystems; iDet++)
    size += fData[iDet].size();
  return size;

}
// ---------------------------------------------------------------------------



// -----   Number of objects in buffer for given detector   ------------------
Int_t CbmDaqBuffer::GetSize(Int_t det) const {
  if ( det < kRef || det > kNofSystems) return 0;
  return fData[det].size();
}
// ---------------------------------------------------------------------------



// -----   Insert data into buffer   -----------------------------------------
void CbmDaqBuffer::InsertData(CbmDigi* digi) {

  if ( ! digi ) LOG(FATAL) << "DaqBuffer: invalid digi pointer"
                           << FairLogger::endl;

  Int_t iDet = digi->GetSystemId();
  if ( iDet >= kNofSystems) {
    LOG(WARNING) << "DaqBuffer: Illegal system ID " << iDet
                 << FairLogger::endl;
    return;
  }

  pair<Double_t, CbmDigi*> value (digi->GetTime(), digi);
  fData[iDet].insert(value);

  LOG(DEBUG2) << "DaqBuffer: Inserting digi, detectorID "
              << digi->GetAddress() << ", time " << digi->GetTime()
              << FairLogger::endl;

}
// ---------------------------------------------------------------------------



// -----   Instance   --------------------------------------------------------
CbmDaqBuffer* CbmDaqBuffer::Instance() {
  if ( ! fgInstance ) fgInstance = new CbmDaqBuffer();
  return fgInstance;
}
// ---------------------------------------------------------------------------



// -----   Print status   ----------------------------------------------------
void CbmDaqBuffer::PrintStatus() const {
  TString sysName;
  Int_t size = GetSize();
  LOG(INFO) << "DaqBuffer: Status ";
  if ( ! size ) {
    LOG(INFO) << "empty" << FairLogger::endl;
    return;
  }
  for (Int_t det = kRef; det < kNofSystems; det++) {
    if ( GetSize(det) ) {
      sysName = CbmModuleList::GetModuleNameCaps(det);
      LOG(INFO) << sysName << " " << GetSize(det) << "  ";
    }
  }
  LOG(INFO) << "\t     " << "Total: " << GetSize() << " from "
            << fixed << setprecision(3) << GetFirstTime() << " ns to "
            << GetLastTime() << " ns" << FairLogger::endl;
}
// ---------------------------------------------------------------------------



// -----   Status to string   ------------------------------------------------
string CbmDaqBuffer::ToString() const {
  stringstream ss;
  ss << "DaqBuffer: ";
  Int_t size = GetSize();
  if ( ! size ) {
    ss << "empty";
    return ss.str();
  }
  TString sysName;
  for (Int_t det = kRef; det < kNofSystems; det++) {
    if ( GetSize(det) ) {
      sysName = CbmModuleList::GetModuleNameCaps(det);
      ss << sysName << " " << GetSize(det) << "  ";
    }
  }
  ss << "Total: " << size << " from " << fixed << setprecision(3)
    << GetFirstTime() << " ns to " << GetLastTime() << " ns";

  return ss.str();
}
// ---------------------------------------------------------------------------


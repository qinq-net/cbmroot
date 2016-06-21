/** @file CbmTbDaqTestBuffer.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 13 December 2013
 ** updated 21.06.16 F.Uhlig
 **/


#include <iostream>
#include <iomanip>

#include "TString.h"

#include "FairLogger.h"

#include "CbmTbDaqTestBuffer.h"
//#include "CbmDetectorList.h"



// -----   Initialisation of static variables   ------------------------------
CbmTbDaqTestBuffer* CbmTbDaqTestBuffer::fgInstance = NULL;
// ---------------------------------------------------------------------------



// -----   Constructor   -----------------------------------------------------
CbmTbDaqTestBuffer::CbmTbDaqTestBuffer() : fData() {
}
// ---------------------------------------------------------------------------


// -----   Destructor   ------------------------------------------------------
CbmTbDaqTestBuffer::~CbmTbDaqTestBuffer() {
}
// ---------------------------------------------------------------------------




// -----   Access to next data   ---------------------------------------------
CbmDigi* CbmTbDaqTestBuffer::GetNextData(Double_t time) {


  // --- Check for empty buffer
  if ( ! fData.size() ) return NULL;

  // --- Get data from buffer
  CbmDigi* digi = NULL;
  multimap<Double_t, CbmDigi*>::iterator it = fData.begin();
  CbmDigi* test = it->second;
  if ( test->GetTime() < time ) {
    digi = test;
    fData.erase(it);
  }
  return digi;

}
// ---------------------------------------------------------------------------




// -----   Insert data into buffer   -----------------------------------------
void CbmTbDaqTestBuffer::InsertData(CbmDigi* digi) {

  if ( ! digi ) LOG(FATAL) << "DaqTestBuffer: invalid digi pointer"
                           << FairLogger::endl;

  pair<Double_t, CbmDigi*> value (digi->GetTime(), digi);
  fData.insert(value);

  LOG(DEBUG2) << "DaqTestBuffer: Inserting digi, detectorID "
              << digi->GetAddress() << ", time " << digi->GetTime()
              << FairLogger::endl;

}
// ---------------------------------------------------------------------------



// -----   Instance   --------------------------------------------------------
CbmTbDaqTestBuffer* CbmTbDaqTestBuffer::Instance() {
  if ( ! fgInstance ) fgInstance = new CbmTbDaqTestBuffer();
  return fgInstance;
}
// ---------------------------------------------------------------------------



// -----   Print status   ----------------------------------------------------
void CbmTbDaqTestBuffer::PrintStatus() const {
  TString sysName;
  Int_t size = GetSize();
  LOG(INFO) << "DaqTestBuffer: Status ";
  if ( ! size ) {
    LOG(INFO) << "empty" << FairLogger::endl;
    return;
  }
  LOG(INFO) << GetSize() << " digis from "
            << fixed << setprecision(9) << GetTimeFirst() * 1.e-9 << " s to "
            << GetTimeLast() *1.e-9 << " s" << FairLogger::endl;
}
// ---------------------------------------------------------------------------


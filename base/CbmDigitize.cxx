/** @file CbmDigitize.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 01.06.2018
 **/

#include "CbmDigitize.h"

#include "FairEventHeader.h"
#include "FairRunAna.h"

#include <cassert>



// -----   Default constructor   --------------------------------------------
CbmDigitize::CbmDigitize() :
    FairTask("Digitizer"),
    fEventMode(kFALSE),
    fCurrentInput(-1),
    fCurrentEvent(-1),
    fCurrentEventTime(0.),
    fDaqBuffer(nullptr) {
}
// --------------------------------------------------------------------------



// -----   Default constructor   --------------------------------------------
CbmDigitize::CbmDigitize(const char* name) :
    FairTask(name),
    fEventMode(kFALSE),
    fCurrentInput(-1),
    fCurrentEvent(-1),
    fCurrentEventTime(0.),
    fDaqBuffer(nullptr) {
}
// --------------------------------------------------------------------------



// -----   Get event information   ------------------------------------------
void CbmDigitize::GetEventInfo() {

  // --- The event number is taken from the FairRootManager
  fCurrentEvent = FairRootManager::Instance()->GetEntryNr();

  // --- In a FairRunAna, take input number and time from FairEventHeader
  if ( FairRunAna::Instance() ) {
    FairEventHeader* event = FairRunAna::Instance()->GetEventHeader();
    assert ( event );
    fCurrentInput   = event->GetInputFileId();
    fCurrentEventTime = event->GetEventTime();
  } //? FairRunAna
  else {  // no FairRunAna
    fCurrentInput = 0;
    fCurrentEventTime = 0.;
  } //? not FairRunAna

}
// --------------------------------------------------------------------------


// -----   Send a digi to the DAQ   -----------------------------------------
void CbmDigitize::SendDigi(CbmDigi* digi) {
  assert(fDaqBuffer);
  fDaqBuffer->InsertData(digi);
}
// --------------------------------------------------------------------------



// -----   Destructor   -----------------------------------------------------
CbmDigitize::~CbmDigitize() {
}
// --------------------------------------------------------------------------


ClassImp(CbmDigitize)

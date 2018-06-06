/** @file CbmDigitizer.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 01.06.2018
 **/

#include "CbmDigitizer.h"

#include "FairEventHeader.h"
#include "FairRunAna.h"

#include <cassert>



// -----   Default constructor   --------------------------------------------
CbmDigitizer::CbmDigitizer() :
    FairTask("Digitizer"),
    fEventMode(kFALSE),
    fCurrentInput(-1),
    fCurrentEvent(-1),
    fCurrentEventTime(0.),
    fDaqBuffer(nullptr) {
}
// --------------------------------------------------------------------------



// -----   Default constructor   --------------------------------------------
CbmDigitizer::CbmDigitizer(const char* name) :
    FairTask(name),
    fEventMode(kFALSE),
    fCurrentInput(-1),
    fCurrentEvent(-1),
    fCurrentEventTime(0.),
    fDaqBuffer(nullptr) {
}
// --------------------------------------------------------------------------



// -----   Get event information   ------------------------------------------
void CbmDigitizer::GetEventInfo() {

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
void CbmDigitizer::SendDigi(CbmDigi* digi) {
  assert(fDaqBuffer);
  fDaqBuffer->InsertData(digi);
}
// --------------------------------------------------------------------------



// -----   Destructor   -----------------------------------------------------
CbmDigitizer::~CbmDigitizer() {
}
// --------------------------------------------------------------------------


ClassImp(CbmDigitizer)

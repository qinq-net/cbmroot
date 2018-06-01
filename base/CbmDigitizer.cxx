/** @file CbmDigitizer.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 01.06.2018
 **/

#include "CbmDigitizer.h"

#include <cassert>



// -----   Default constructor   --------------------------------------------
CbmDigitizer::CbmDigitizer() :
    FairTask("Digitizer"),
    fEventMode(kFALSE),
    fDaqBuffer(nullptr) {
}
// --------------------------------------------------------------------------



// -----   Default constructor   --------------------------------------------
CbmDigitizer::CbmDigitizer(const char* name) :
    FairTask(name),
    fEventMode(kFALSE),
    fDaqBuffer(nullptr) {
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

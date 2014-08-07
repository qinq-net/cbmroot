// -------------------------------------------------------------------------
// -----                  CbmMvdSensorTask source file              -----
// -----                  Created 02.02.2012 by M. Deveaux            -----
// -------------------------------------------------------------------------

#include "CbmMvdSensorTask.h"
#include "TClonesArray.h"
#include "TObjArray.h"



// -----   Default constructor   -------------------------------------------
CbmMvdSensorTask::CbmMvdSensorTask() {
  fInputBuffer     = NULL;
  fOutputBuffer    = NULL;
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdSensorTask::~CbmMvdSensorTask() {
  fInputBuffer->Delete();
 
  fOutputBuffer->Delete();
}
// -------------------------------------------------------------------------

ClassImp(CbmMvdSensorTask)

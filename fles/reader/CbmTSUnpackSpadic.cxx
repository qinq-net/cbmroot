// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmTSUnpackSpadic                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmTSUnpackSpadic.h"

#include "FairLogger.h"

CbmTSUnpackSpadic::CbmTSUnpackSpadic()
  : CbmTSUnpack()
{
}

CbmTSUnpackSpadic::~CbmTSUnpackSpadic()
{
}

Bool_t CbmTSUnpackSpadic::Init()
{
  LOG(INFO) << "Initializing" << FairLogger::endl; 
  return kTRUE;
}

Bool_t CbmTSUnpackSpadic::DoUnpack(const fles::Timeslice& ts, size_t componen)
{
  return kTRUE;
}

void CbmTSUnpackSpadic::Reset()
{
}

/*
void CbmTSUnpackSpadic::Register()
{
}
*/


ClassImp(CbmTSUnpackSpadic)

// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                              CbmTSUnpackTrb                       -----
// -----                    Created 07.11.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmTSUnpackTrb.h"

#include "TimesliceReader.hpp"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include <iostream>

CbmTSUnpackTrb::CbmTSUnpackTrb()
  : CbmTSUnpack(),
    fTrbBridge(new TrbBridge())
 //   fTrbRaw(new TClonesArray("CbmTrbRawMessage", 10)),
{
}

CbmTSUnpackTrb::~CbmTSUnpackTrb()
{
}

Bool_t CbmTSUnpackTrb::Init()
{
  LOG(INFO) << "Initializing" << FairLogger::endl; 

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }
  //  ioman->Register("TrbRawMessage", "spadic raw data", fTrbRaw, kTRUE);

  return kTRUE;
}

Bool_t CbmTSUnpackTrb::DoUnpack(const fles::Timeslice& ts, size_t component)
{
  LOG(DEBUG) << "Unpacking Trb Data" << FairLogger::endl; 

  Int_t skipMS = 1;
  fTrbEventList = fTrbBridge->extractEventsFromTimeslice(ts, skipMS);

  LOG(INFO) << "Found " << fTrbEventList.size() << " TRB events in time slice"
	    << FairLogger::endl;

  return kTRUE;
}


void CbmTSUnpackTrb::Reset()
{
  fTrbEventList.clear();
  //   fTrbRaw->Clear();
}

void CbmTSUnpackTrb::Finish()
{
  //
}


/*
void CbmTSUnpackTrb::Register()
{
}
*/


ClassImp(CbmTSUnpackTrb)

// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                           CbmROCUnpackMuch                         -----
// -----                    Created 13.12.2014 by F. Uhlig                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------
#include "CbmROCUnpackMuch.h"

#include "CbmMuchBeamTimeDigi.h"
#include "CbmTbDaqBuffer.h"
#include "CbmSourceLmdNew.h"
#include "CbmDaqMap.h"

#include "CbmMuchAddress.h"

#include "FairRunOnline.h"
#include "FairLogger.h"

#include "TClonesArray.h"

CbmROCUnpackMuch::CbmROCUnpackMuch()
  : CbmROCUnpack(),
    fBuffer(CbmTbDaqBuffer::Instance()),
    fDaqMap(NULL),
    fSource(NULL),
    fMuchDigis(new TClonesArray("CbmMuchBeamTimeDigi", 10)),
    fMuchBaselineDigis(new TClonesArray("CbmMuchBeamTimeDigi", 10))
{
}
  
CbmROCUnpackMuch::~CbmROCUnpackMuch()
{
}
  
  
Bool_t CbmROCUnpackMuch::Init()
{
  FairRunOnline* online = FairRunOnline::Instance();
  fSource = static_cast<CbmSourceLmdNew*>(online->GetSource());

  fDaqMap = fSource->GetDaqMap();

  FairRootManager* ioman = FairRootManager::Instance();
  ioman->Register("MuchDigi", "MUCH raw data", fMuchDigis, fPersistence);
  ioman->Register("MuchBaselineDigi", "MUCH baseline data", 
		  fMuchBaselineDigis, fPersistence);

  return kTRUE;
}
 
Bool_t CbmROCUnpackMuch::DoUnpack(roc::Message* Message, ULong_t hitTime)
{
  
  // --- Get absolute time, NXYTER and channel number
  Int_t rocId      = Message->getRocNumber();
  Int_t nxyterId   = Message->getNxNumber();
  Int_t nxChannel  = Message->getNxChNum();
  Int_t charge     = Message->getNxAdcValue();
  
  // --- Get detector element from DaqMap
  Int_t station = fDaqMap->GetMuchStation(rocId);
  Int_t layer   = 0;
  Int_t side    = 0;
  Int_t module  = 0;
  Int_t sector  = nxyterId;
  Int_t channel = nxChannel;

  // --- Construct unique address
  UInt_t address = CbmMuchAddress::GetAddress(station, layer, side,
					      module, sector, channel);
  
  // --- Create digi
  CbmMuchBeamTimeDigi* digi = new CbmMuchBeamTimeDigi(address, charge, hitTime);

  // In case of normal data insert the digi into the buffer.
  // In case of baseline data insert the digi only if the roc 
  // is already in baseline mode.  
  if ( !fSource->IsBaselineFill() ) {
    fBuffer->InsertData(digi);
  } else {
    if ( fSource->IsBaselineFill(rocId) ) { fBuffer->InsertData(digi); }
  }

  return kTRUE;
}

void CbmROCUnpackMuch::FillOutput(CbmDigi* digi)
{
  if ( !fSource->IsBaselineRetrieve() ) {
    new( (*fMuchDigis)[fMuchDigis->GetEntriesFast()])
      CbmMuchBeamTimeDigi(*(dynamic_cast<CbmMuchBeamTimeDigi*>(digi)));
  } else {
    new( (*fMuchBaselineDigis)[fMuchBaselineDigis->GetEntriesFast()])
      CbmMuchBeamTimeDigi(*(dynamic_cast<CbmMuchBeamTimeDigi*>(digi)));
  }

}

void CbmROCUnpackMuch::Reset()
{
  // The next block is needed do to the problem that the TClonesArray is
  // very large after usage (2.5 M entries). This somehow slows down
  // the Clear of the container even if no entries are inside by 2-3 orders
  // of magnitude which slows down the execution of the whole program by 
  // more then a factor of 10.
  // TODO:
  // It has to be checked if this is a bug in Root by producing a small
  // example program to demonstarete the issue
  if (fMuchBaselineDigis->GetEntriesFast() > 1000) {
    fMuchBaselineDigis->Delete();
    fMuchBaselineDigis->Expand(10);
  } 
  fMuchDigis->Clear();
  fMuchBaselineDigis->Clear();

}

ClassImp(CbmROCUnpackMuch)

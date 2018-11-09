/** @file CbmInputChain.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 09.11.2018
 **/

#include "CbmInputChain.h"

#include <cassert>
#include "TFile.h"
#include "FairMCEventHeader.h"
#include "FairLogger.h"


// -----   Default constructor   ---------------------------------------------
CbmInputChain::CbmInputChain() :
        TObject(),
        fChain(nullptr),
        fRate(0.),
        fCurrentEntryId(-1),
        fDeltaDist(nullptr),
        fNofUsedEntries(0) {
}
// ---------------------------------------------------------------------------



// -----   Constructor   -----------------------------------------------------
CbmInputChain::CbmInputChain(TChain* chain, Double_t rate) :
        TObject(),
        fChain(chain),
        fRate(rate),
        fCurrentEntryId(-1),
        fDeltaDist(nullptr),
        fNofUsedEntries(0) {

  if (rate > 0.) {
    Double_t mean = 1.e9 / rate;       // mean time between events
    fDeltaDist = new TF1("DeltaDist", "exp(-x/[0])/[0]", 0., 10. * mean);
    fDeltaDist->SetParameter(0, mean);
  }
  else
    LOG(FATAL) << "InputChain: illegal non-positive rate " << rate
               << FairLogger::endl;

}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmInputChain::~CbmInputChain() {
  if (fDeltaDist) delete fDeltaDist;
}
// ---------------------------------------------------------------------------



// -----   Time difference to next event   -----------------------------------
Double_t CbmInputChain::GetDeltaT() {
  assert(fDeltaDist);
  return fDeltaDist->GetRandom();
}
// ---------------------------------------------------------------------------



// -----   Get next entry from chain   ---------------------------------------
Int_t CbmInputChain::GetNextEntry() {

  assert(fChain);
  fCurrentEntryId++;

  // Return -1 if all entries in the chain were processed
  if (fCurrentEntryId >= GetNofEntries()) return -1;

  // Get entry from chain
  Int_t nBytes = fChain->GetEntry(fCurrentEntryId, 1);
  fNofUsedEntries++;

  // Warning if no data were read
  if (nBytes <= 0)
    LOG(WARNING) << "InputChain: " << nBytes << " Bytes read from tree!"
                 << FairLogger::endl;

  return fCurrentEntryId;

}
// ---------------------------------------------------------------------------

ClassImp(CbmInputChain)


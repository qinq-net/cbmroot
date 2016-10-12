
// -------------------------------------------------------------------------
// -----                    CbmDigiManager source file                 -----
// -----                  Created 08/05/07  by V. Friese               -----
// -------------------------------------------------------------------------
// Includes from cbmroot
#include "CbmDigiManager.h"

#include "CbmDigi.h"

#include "FairRootManager.h"
#include "FairLogger.h"

// Includes from ROOT
#include "TClonesArray.h"

// Includes from c++
#include <iomanip>

/// pair included
using std::left;
using std::right;
using std::setw;
using std::fixed;
using std::setprecision;
using std::pair;

// -----   Default constructor   -------------------------------------------
CbmDigiManager::CbmDigiManager() 
  : FairTask("DigiManager"),
    fDigiMap(),
    fTimer()
{
  for (Int_t iSys=0; iSys<16; iSys++) fDigis[iSys] = NULL;
  fSystem[0] = "";
  fSystem[1] = "MVD";
  fSystem[2] = "STS";
  fSystem[3] = "RICH";
  fSystem[4] = "MUCH";
  fSystem[5] = "TRD";
  fSystem[6] = "TOF";
  fSystem[7] = "ECAL";
  fSystem[8] = "PSD";
  for (Int_t iSys=9; iSys<16; iSys++) fSystem[iSys] = "";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmDigiManager::~CbmDigiManager() {}
// -------------------------------------------------------------------------



// -----   Virtual public method Exec   ------------------------------------
void CbmDigiManager::Exec(Option_t* /*opt*/) {

  // Clear map and timer
  fTimer.Start();
  fDigiMap.clear();

  // Loop over all digi collections
  Int_t nDigis[16];
  for (Int_t iSys=0; iSys<16; iSys++) {
    nDigis[iSys] = 0;
    if ( ! fDigis[iSys] ) continue;

    // Loop over digis in this collection
    nDigis[iSys] = fDigis[iSys]->GetEntriesFast();
    for (Int_t iDigi=0; iDigi<nDigis[iSys]; iDigi++) {
      CbmDigi* digi = (CbmDigi*) fDigis[iSys]->At(iDigi);
      if ( digi->GetSystemId() != iSys ) {
	LOG(FATAL) << "Digi with wrong system ID "
	     << digi->GetSystemId() << " in digi collection " << iSys
	     << "(" << fSystem[iSys] << ")" << FairLogger::endl;
      }
      Int_t iDet  = digi->GetDetectorId();
      Int_t iChan = digi->GetChannelNr();
      pair<Int_t, Int_t> a(iDet, iChan);

      // Check for existing entries in the map (should not happen)
      if (fDigiMap.find(a) != fDigiMap.end() ) {
	LOG(WARNING) << "Multiple entry in "
	     << fSystem[iSys] << " digi collection; detector " << iDet
	     << ", channel " << iChan << FairLogger::endl;
	continue;
      }

      // Enter digi into the map
      fDigiMap[a] = digi;

    }   // Loop over digis in the collection
  }     // Loop over collections

    

  // Control output
  fTimer.Stop();
  LOG(INFO) << "+ ";
  LOG(INFO) << setw(15) << left << fName << ": " << setprecision(4) << setw(8)
       << fixed << right << fTimer.RealTime()
       << " s";
  for (Int_t iSys=0; iSys<15; iSys++)
    if ( fDigis[iSys] ) LOG(INFO) << ", " << fSystem[iSys] 
			     << " " << nDigis[iSys];
  LOG(INFO) << FairLogger::endl;

}
// -------------------------------------------------------------------------




// -----   Public method GetDigi   -----------------------------------------
CbmDigi* CbmDigiManager::GetDigi(Int_t iDetector, Int_t iChannel) {

  pair<Int_t, Int_t> a(iDetector, iChannel);

  if ( fDigiMap.find(a) == fDigiMap.end() ) return NULL;
  else return fDigiMap[a];

}
// -------------------------------------------------------------------------



      
// -----   Virtual private method Init   -----------------------------------
InitStatus CbmDigiManager::Init() {

  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman ) LOG(FATAL) << "No FairRootManager" << FairLogger::endl;

  Int_t nColl = 0;
  for (Int_t iSys=0; iSys<16; iSys++) {

    TString digiName = fSystem[iSys];
    digiName += "Digi";
    fDigis[iSys] = (TClonesArray*) ioman->GetObject(digiName.Data());
    if ( fDigis[iSys] ) nColl++;
  }

  LOG(INFO) << GetName() << "::Init: " << nColl 
       << " digi collection";
  if ( nColl != 1 ) LOG(INFO) << "s";
  LOG(INFO) << " found." << FairLogger::endl;

  return kSUCCESS;

}
// -------------------------------------------------------------------------



ClassImp(CbmDigiManager)

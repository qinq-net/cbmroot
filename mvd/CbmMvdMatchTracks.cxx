// -------------------------------------------------------------------------
// -----                  CbmMvdMatchTracks source file                -----
// -----                  Created 24/11/05  by V. Friese               -----
// -------------------------------------------------------------------------
#include "CbmMvdMatchTracks.h"

#include "CbmMvdHit.h"
#include "CbmMvdTrack.h"
#include "CbmMvdTrackMatch.h"

#include "FairMCPoint.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include <iostream>
#include <iomanip>
#include <map>

using std::cout;
using std::endl;
using std::left;
using std::right;
using std::setw;
using std::fixed;
using std::setprecision;
using std::map;

// -----   Default constructor   -------------------------------------------
CbmMvdMatchTracks::CbmMvdMatchTracks() 
  : FairTask("STSMatchTracks") {
  fTracks  = NULL;
  fPoints  = NULL;
  fHits    = NULL;
  fMatches = NULL;
  fTime    = 0.;
  fNTrackMatches = 0.;
  fNAllHits      = 0.;   
  fNTrueHits     = 0.;
  fNEvents        = 0;
  fNEventsFailed  = 0;
}
// -------------------------------------------------------------------------



// -----   Standard constructor  -------------------------------------------
CbmMvdMatchTracks::CbmMvdMatchTracks(Int_t iVerbose) 
  : FairTask("MVDMatchTracks", iVerbose) {
  fTracks  = NULL;
  fPoints  = NULL;
  fHits    = NULL;
  fMatches = NULL;
  fTime    = 0.;
  fNTrackMatches = 0.;
  fNAllHits      = 0.;   
  fNTrueHits     = 0.;
  fNEvents        = 0;
  fNEventsFailed  = 0;
}
// -------------------------------------------------------------------------



// -----   Constructor with task name   ------------------------------------
CbmMvdMatchTracks::CbmMvdMatchTracks(const char* name, Int_t iVerbose) 
  : FairTask(name, iVerbose) {
  fTracks  = NULL;
  fPoints  = NULL;
  fHits    = NULL;
  fMatches = NULL;
  fTime    = 0.;
  fNTrackMatches = 0.;
  fNAllHits      = 0.;   
  fNTrueHits     = 0.;
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMvdMatchTracks::~CbmMvdMatchTracks() { }
// -------------------------------------------------------------------------



// -----   Virtual public method Exec   ------------------------------------
void CbmMvdMatchTracks::Exec(Option_t* opt) {

    /*

  // Timer
  fTimer.Start();
  Bool_t warn = kFALSE;

  // Clear output array
  fMatches->Clear();

  // Create some pointers and variables
  CbmMvdTrack* track = NULL;
  CbmMvdHit*   hit  = NULL;
  FairMCPoint*  point = NULL;
  Int_t nHits    = 0;
  Int_t nMCTracks = 0;
  Int_t iPoint    = 0;
  Int_t iMCTrack  = 0;
  Int_t nAll      = 0;
  Int_t nTrue     = 0;
  Int_t nWrong    = 0;
  Int_t nFake     = 0;
  Int_t nHitSum     = 0;
  Int_t nTrueSum    = 0;
  Int_t nWrongSum   = 0;
  Int_t nFakeSum    = 0;
  Int_t nMCTrackSum = 0;
  map<Int_t, Int_t>::iterator it;

  // Loop over StsTracks
  Int_t nTracks = fTracks->GetEntriesFast();
  for (Int_t iTrack=0; iTrack<nTracks; iTrack++) {
    track = (CbmStsTrack*) fTracks->At(iTrack);
    if ( ! track) {
      cout << "-W- CbmMvdMatchTracks::Exec: Empty MvdTrack at " 
	   << iTrack << endl;
      warn = kTRUE;
      continue;
    }
    nHits = track->GetNMvdHits();
    nAll = nTrue = nWrong = nFake = nMCTracks = 0;
    fMatchMap.clear();
    if (fVerbose > 2) cout << endl << "Track " << iTrack << ", Hits " 
			   << nHits << endl;

    // Loop over MvdHits of track
    for (Int_t iHit=0; iHit<nHits; iHit++) {

      hit = (CbmMvdHitMatch*) fHits->At(track->GetMvdHitIndex(iHit));
      if ( ! hit ) {
	cout << "-E- CbmMvdMatchTracks::Exec: "
	     << "No MvdHit " << iHit << " for track " << iTrack << endl;
	warn = kTRUE;
	continue;
      }

      iMCTrack = hit->GetTrackID();
      if ( fVerbose > 2 ) cout << "Track " << iTrack << ", MAPS hit "
			       << track->GetMvdHitIndex(iHit) 
			       << ", StsPoint " << iPoint << ", MCTrack "
			       << iMCTrack << endl;
      fMatchMap[iMCTrack]++;
    }

 
    // Search for best matching MCTrack
    iMCTrack = -1;
    for (it=fMatchMap.begin(); it!=fMatchMap.end(); it++) {
      if (fVerbose > 2) cout << it->second 
			     << " common points wth MCtrack " 
			     << it->first << endl;
      nMCTracks++;
      nAll += it->second;
      if ( it->second > nTrue ) {
	iMCTrack = it->first;
	nTrue    = it->second;	
      }
    }
    nWrong = nAll - nTrue;
    if (fVerbose>1) cout << "-I- CbmMvdMatchTracks: StsTrack " << iTrack 
			 << ", MCTrack " << iMCTrack << ", true " 
			 << nTrue << ", wrong " << nWrong << ", fake " 
			 << nFake << ", #MCTracks " << nMCTracks << endl;

    // Create MvdTrackMatch
    new ((*fMatches)[iTrack]) CbmMvdTrackMatch(iMCTrack, nTrue, 
					       nWrong, nFake, 
					       nMCTracks);

    // Some statistics
    nHitSum     += nHits;
    nTrueSum    += nTrue;
    nWrongSum   += nWrong;
    nFakeSum    += nFake;
    nMCTrackSum += nMCTracks;

  } // Track loop

  // Event statistics
  fTimer.Stop();
  Double_t qTrue = 0.;
  if ( nHitSum) qTrue  = Double_t(nTrueSum)  / Double_t(nHitSum) * 100.;
  if (fVerbose > 1) {
    Double_t qWrong = Double_t(nWrongSum) / Double_t(nHitSum) * 100.;
    Double_t qFake  = Double_t(nFakeSum)  / Double_t(nHitSum) * 100.;
    Double_t qMC    = Double_t(nMCTrackSum) / Double_t(nTracks);
    cout << endl;
    cout << "-------------------------------------------------------" 
	 << endl;
    cout << "-I-              Sts Track Matching                 -I-"
	 << endl;
    cout << "Reconstructed StsTracks : " << nTracks << endl;;
    cout << "True  hit assignments   : " << qTrue  << " %" << endl;
    cout << "Wrong hit assignments   : " << qWrong << " %" << endl;
    cout << "Fake  hit assignments   : " << qFake  << " %" << endl;
    cout << "MCTracks per StsTrack   : " << qMC << endl;
    cout << "--------------------------------------------------------" 
	 << endl;
  }
  if (fVerbose == 1) {
    if ( warn) cout << "- ";
    else       cout << "+ ";
    cout << setw(15) << left << fName << ": " << setprecision(4) << setw(8)
	 << fixed << right << fTimer.RealTime() << " s, matches " 
	 << nTracks << ", hit quota " << qTrue << " %" << endl;
  }
  
  // Run statistics
  if ( warn ) fNEventsFailed++;
  else {
    fNEvents++;
    fTime          += fTimer.RealTime();
    fNTrackMatches += Double_t(nTracks);
    fNAllHits      += Double_t(nHitSum);
    fNTrueHits     += Double_t(nTrueSum);
  }
*/
}
// -------------------------------------------------------------------------



// -----   Virtual private method Init   -----------------------------------
InitStatus CbmMvdMatchTracks::Init() {
  
  // Get FairRootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if (! ioman) {
    cout << "-E- CbmMvdMatchTracks::Init: "
	 << "RootManager not instantised!" << endl;
    return kFATAL;
  }

  // Get StsHit Array
  fHits = (TClonesArray*) ioman->GetObject("MvdHit");
  if ( ! fHits) {
    cout << "-W- CbmMvdMatchTracks::Init: No MvdHit array!"
	 << endl;
  }

  // Get MvdTrack Array
  fTracks = (TClonesArray*) ioman->GetObject("StsTrack");
  if ( ! fTracks ) {
    cout << "-E- CbmMvdMatchTracks::Init: No MvdTrack array!" << endl;
    return kERROR;
  }

  fStsMatches=(TClonesArray*) ioman->GetObject("StsTrackMatch");

  // Get StsPoint array
  fPoints = (TClonesArray*) ioman->GetObject("MvdPoint");
  if ( ! fPoints ) {
    cout << "-E- CbmMvdMatchTracks::Init: No MvdPoint array!" << endl;
    return kERROR;
  }

  // Create and register MvdTrackMatch array
  fMatches = new TClonesArray("CbmMvdTrackMatch",100);
  ioman->Register("MvdTrackMatch", "Mvd", fMatches, kTRUE);

  return kSUCCESS;

}
// -------------------------------------------------------------------------



    
// -----   Virtual private method Finish   ---------------------------------
void CbmMvdMatchTracks::Finish() {

  cout << endl;
  cout << "============================================================"
       << endl;
  cout << "=====   " << GetName() << ": Run summary " << endl;
  cout << "===== " << endl;
  cout << "===== Good events   : " << setw(6) << fNEvents << endl;
  cout << "===== Failed events : " << setw(6) << fNEventsFailed << endl;
  cout << "===== Average time  : " << setprecision(4) << setw(8) << right
       << fTime / Double_t(fNEvents)  << " s" << endl;
  cout << "===== " << endl;
  cout << "===== Tracks per event  : " << fixed << setprecision(0)
       << fNTrackMatches / Double_t(fNEvents) << endl;
  cout << setprecision(2);
  cout << "===== True hits         : " << fixed << setw(6) << right
       << fNTrueHits / fNAllHits * 100. << " %" << endl;
  cout << "============================================================"
       << endl;

}
// -------------------------------------------------------------------------


    
ClassImp(CbmMvdMatchTracks)


      

      

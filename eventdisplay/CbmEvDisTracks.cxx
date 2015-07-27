// -------------------------------------------------------------------------
// -----                  following FairMCTracks source file                  -----
// -----                  Created 10/12/07  by M. Al-Turany            -----
// -------------------------------------------------------------------------
#include "CbmEvDisTracks.h"

#include "FairEventManager.h"           // for FairEventManager
#include "FairRootManager.h"            // for FairRootManager

#include "Riosfwd.h"                    // for ostream
#include "TClonesArray.h"               // for TClonesArray
#include "TEveManager.h"                // for TEveManager, gEve
#include "TEvePathMark.h"               // for TEvePathMark
#include "TEveTrack.h"                  // for TEveTrackList, TEveTrack
#include "TEveTrackPropagator.h"        // for TEveTrackPropagator
#include "TEveVector.h"                 // for TEveVector, TEveVectorT
#include "TGeoTrack.h"                  // for TGeoTrack
#include "TMathBase.h"                  // for Max, Min
#include "TObjArray.h"                  // for TObjArray
#include "TParticle.h"                  // for TParticle

#include "CbmTofTracklet.h"             // for reconstructed Tof Tracks

#include <string.h>                     // for NULL, strcmp
#include <iostream>                     // for operator<<, basic_ostream, etc

using std::cout;
using std::endl;

// -----   Default constructor   -------------------------------------------
CbmEvDisTracks::CbmEvDisTracks()
  : FairTask("CbmEvDisTracks", 0),
    fTrackList(NULL),
    fTrPr(NULL),
    fEventManager(NULL),
    fEveTrList(NULL),
    fEvent(""),
    fTrList(NULL),
    MinEnergyLimit(-1.),
    MaxEnergyLimit(-1.),
    PEnergy(-1.)
{
}
// -------------------------------------------------------------------------


// -----   Standard constructor   ------------------------------------------
CbmEvDisTracks::CbmEvDisTracks(const char* name, Int_t iVerbose)
  : FairTask(name, iVerbose),
    fTrackList(NULL),
    fTrPr(NULL),
    fEventManager(NULL),
    fEveTrList(new TObjArray(16)),
    fEvent(""),
    fTrList(NULL),
    MinEnergyLimit(-1.),
    MaxEnergyLimit(-1.),
    PEnergy(-1.)
{
}
// -------------------------------------------------------------------------
InitStatus CbmEvDisTracks::Init()
{
  if(fVerbose>1) { cout<<  "CbmEvDisTracks::Init()" << endl; }
  FairRootManager* fManager = FairRootManager::Instance();
  fTrackList = (TClonesArray*)fManager->GetObject("TofTracks");
  if(fTrackList==0) {
    cout << "CbmEvDisTracks::Init() branch " << GetName() << " Not found! Task will be deactivated "<< endl;
    SetActive(kFALSE);
  }
  if(fVerbose>2) { cout<<  "CbmEvDisTracks::Init() get track list" << fTrackList<< endl; }
  if(fVerbose>2) { cout<<  "CbmEvDisTracks::Init() create propagator" << endl; }
  fEventManager =FairEventManager::Instance();
  if(fVerbose>2) { cout<<  "CbmEvDisTracks::Init() get instance of FairEventManager " << endl; }
  fEvent = "Current Event";
  MinEnergyLimit=fEventManager->GetEvtMinEnergy();
  MaxEnergyLimit=fEventManager->GetEvtMaxEnergy();
  PEnergy=0;
  if(IsActive()) { return kSUCCESS; }
  else { return kERROR; }
}
// -------------------------------------------------------------------------
void CbmEvDisTracks::Exec(Option_t* option)
{

  if (IsActive()) {

    if(fVerbose>1) { cout << " CbmEvDisTracks::Exec starting"<< endl; }
    CbmTofTracklet* tr;
    const Double_t* point;

    Reset();

    for (Int_t i=0; i<fTrackList->GetEntriesFast(); i++)  {
      if(fVerbose>2) { cout << "CbmEvDisTracks::Exec "<< i << endl; }
      tr=(CbmTofTracklet *)fTrackList->At(i);
      if(NULL == tr) continue;
      Int_t Np=tr->GetNofHits();
      fTrList= GetTrGroup(tr->GetNofHits());
      TParticle* P=new TParticle();
      TEveTrack* track= new TEveTrack(P, tr->GetPidHypo(), fTrPr);
      Int_t iCol=Np;
      if(iCol>4) iCol++;
      track->SetLineColor(iCol);
      track->SetMarkerColor(iCol);
      track->SetMarkerSize(2.);
      //track->SetMarkerDraw(kTRUE);
      //insert starting point 

      track->SetPoint(0,tr->GetFitX(0.),tr->GetFitY(0.),0.);
      TEveVector pos0= TEveVector(tr->GetFitX(0.),tr->GetFitY(0.),0.);
      TEvePathMark* path0 = new TEvePathMark();
      path0->fV=pos0 ;
      track->AddPathMark(*path0);

      for (Int_t n=0; n<Np; n++) {
        //point=tr->GetPoint(n);
	point=tr->GetFitPoint(n);
        track->SetPoint(n+1,point[0],point[1],point[2]);
        if(fVerbose>3) 
	  { cout <<Form("   CbmEvDisTracks::Exec SetPoint %d, %6.2f, %6.2f, %6.2f, %6.2f ",
			n,point[0],point[1],point[2],point[3]) << endl; } 
      
	TEveVector pos= TEveVector(point[0],point[1],point[2]);
        TEvePathMark* path = new TEvePathMark();
        path->fV=pos ;
        path->fTime=point[3];
        if(n==0) {
          TEveVector Mom= TEveVector(P->Px(), P->Py(),P->Pz());
          path->fP=Mom;
        }
        if(fVerbose>5) { cout << "Path marker added " << path << endl; }

        track->AddPathMark(*path);

        if(fVerbose>5) { cout << "Path marker added " << path << endl; }
      }
      fTrList->AddElement(track);
      if(fVerbose>3) { cout << i<<". track added: " << track->GetName() << endl; }
    }
    for (Int_t i=0; i<fEveTrList->GetEntriesFast(); i++) {
      // TEveTrackList *TrListIn=( TEveTrackList *) fEveTrList->At(i);
      //TrListIn->FindMomentumLimits(TrListIn, kFALSE);
    }
    fEventManager->SetEvtMaxEnergy(MaxEnergyLimit);
    fEventManager->SetEvtMinEnergy(MinEnergyLimit);
    gEve->Redraw3D(kFALSE);
  }
}
// -----   Destructor   ----------------------------------------------------
CbmEvDisTracks::~CbmEvDisTracks()
{
}
// -------------------------------------------------------------------------
void CbmEvDisTracks::SetParContainers()
{

}

// -------------------------------------------------------------------------
void CbmEvDisTracks::Finish()
{

}
// -------------------------------------------------------------------------
void CbmEvDisTracks::Reset()
{
  for (Int_t i=0; i<fEveTrList->GetEntriesFast(); i++) {
    TEveTrackList*  ele=( TEveTrackList*) fEveTrList->At(i);
    gEve->RemoveElement(ele,fEventManager);
  }
  fEveTrList->Clear();
}

TEveTrackList* CbmEvDisTracks::GetTrGroup(Int_t ihmul)
{
  const char *gr=Form("hmul%d",ihmul);
  fTrList=0;
  for (Int_t i=0; i<fEveTrList->GetEntriesFast(); i++) {
    TEveTrackList* TrListIn=( TEveTrackList*) fEveTrList->At(i);
    if ( strcmp(TrListIn->GetName(),gr)==0 ) {
      fTrList= TrListIn;
      break;
    }
  }
  if(fTrList ==0) {
    fTrPr=new TEveTrackPropagator();
    fTrList= new  TEveTrackList(gr,fTrPr );
    Int_t iCol=ihmul; if(iCol>4) iCol++;
    fTrList->SetMainColor(iCol);
    fEveTrList->Add(fTrList);
    gEve->AddElement( fTrList ,fEventManager );
    fTrList->SetRnrLine(kTRUE);
  }
  return fTrList;
}

ClassImp(CbmEvDisTracks)



// -------------------------------------------------------------------------
// -----                  following FairMCTracks source file           -----
// -----                  Created 10/12/07  by M. Al-Turany            -----
// -------------------------------------------------------------------------
#define TOFDisplay 1                    // =1 means active, other: without Label and not relying on TEvePointSet
#define TOFTtErr 1                      // =1 means active, other: not relying on VelocityError of CbmTofTracklet

#include "CbmEvDisTracks.h"

#include "FairEventManager.h"           // for FairEventManager
#include "FairRootManager.h"            // for FairRootManager

//#include "Riosfwd.h"                    // for ostream
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
#include "TEvePointSet.h"               // for TEvePointSetArray
#include "TMath.h"                      // for Power, Abs and Sqrt

#include "CbmTofTracklet.h"             // for reconstructed Tof Tracks

#include <string.h>                     // for NULL, strcmp
#include <iostream>                     // for operator<<, basic_ostream, etc

using std::cout;
using std::endl;

ClassImp(CbmEvDisTracks);
CbmEvDisTracks *CbmEvDisTracks::fInstance = 0;

// -----   Default constructor   -------------------------------------------
CbmEvDisTracks::CbmEvDisTracks()
  : FairTask("CbmEvDisTracks", 0),
    fTrackList(NULL),
    fTrPr(NULL),
    fEventManager(NULL),
    fEveTrList(NULL),
    fEvePSList(NULL),
    fEvent(""),
    fTrList(NULL),
    fPSList(NULL),
    MinEnergyLimit(-1.),
    MaxEnergyLimit(-1.),
    PEnergy(-1.),
    fRenderP(kFALSE),
    fRenderT(kTRUE)
{
  if( !fInstance ) fInstance = this;
}
// -------------------------------------------------------------------------


// -----   Standard constructor   ------------------------------------------
CbmEvDisTracks::CbmEvDisTracks(const char* name, Int_t iVerbose, Bool_t renderP, Bool_t renderT)
  : FairTask(name, iVerbose),
    fTrackList(NULL),
    fTrPr(NULL),
    fEventManager(NULL),
    fEveTrList(new TObjArray(16)),
    fEvePSList(new TObjArray(8)),
    fEvent(""),
    fTrList(NULL),
    fPSList(NULL),
    MinEnergyLimit(-1.),
    MaxEnergyLimit(-1.),
    PEnergy(-1.),
    fRenderP(renderP),
    fRenderT(renderT)
{
  if( !fInstance ) fInstance = this;
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
  fEventManager = FairEventManager::Instance();
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

    if(fVerbose>3) { cout << " CbmEvDisTracks::Exec starting with verbosity "<<fVerbose 
			  << " and option "<<option<< endl; }
    CbmTofTracklet* tr;
    const Double_t* point;
    CbmTofHit* hit;

    Reset();

    if(fVerbose>4) { cout << " CbmEvDisTracks:: NTrks "<< fTrackList->GetEntries() << endl; }

    for(Int_t iOpt=0; iOpt<2; iOpt++)
    for (Int_t i=0; i<fTrackList->GetEntriesFast(); i++)  {
      if(fVerbose>4) { cout << "CbmEvDisTracks::Exec "<< i << endl; }
      tr=(CbmTofTracklet *)fTrackList->At(i);
      if(NULL == tr) continue;
      Int_t Np=tr->GetNofHits();

      #if TOFDisplay ==1         //List for TEvePointSets
      fPSList= GetPSGroup(Np,iOpt);
      #endif

      fTrList= GetTrGroup(tr->GetNofHits(),iOpt);      
      TParticle* P=new TParticle();
      TEveTrack* track= new TEveTrack(P, tr->GetPidHypo(), fTrPr);
      Int_t iCol=Np;
      if(iCol>4) iCol++;
      track->SetAttLineAttMarker(fTrList); //set defaults
      track->SetLineColor(iCol);
      track->SetMarkerColor(iCol);
      track->SetMarkerSize(2.);
      //track->SetMarkerDraw(kTRUE);  

      track->SetPoint(0,tr->GetFitX(0.),tr->GetFitY(0.),0.); //insert starting point 
      TEveVector pos0= TEveVector(tr->GetFitX(0.),tr->GetFitY(0.),0.);
      TEvePathMark* path0 = new TEvePathMark();
      path0->fV=pos0 ;
      track->AddPathMark(*path0);

      Double_t      pbuf[3], vbuf[3];
      TEveRecTrack  rt;
      rt.fIndex  = i;
      pbuf[0]=0.;   
      pbuf[1]=0.;   
      pbuf[2]=1./tr->GetTt(); // velocity
      rt.fP.Set(pbuf);
      vbuf[0]=tr->GetFitX(0.);
      vbuf[1]=tr->GetFitY(0.);
      vbuf[2]=0.;
      rt.fV.Set(vbuf);
      track->SetName(Form("TEveTrack %d", rt.fIndex));
      
      //track->SetStdTitle();
      Double_t beta, beta_err, res_x, res_y, res_z, res_t;
      switch(iOpt){
        case 0:
          track->SetStdTitle();
          break;
        case 1:
          #if TOFDisplay ==1           //setting content of label depending on available information
           beta=(1/tr->GetTt())/29.98;
           #if TOFTtErr ==1
            beta_err=beta*(tr->GetTtErr()/tr->GetTt());
            track->SetTitle(Form("%s\nChiSqDoF = %2.2f\nbeta = %1.3f +/- %1.3f",track->GetName(),tr->GetChiSq(),beta,beta_err));
           #else
            track->SetTitle(Form("%s\nChiSqDoF = %2.2f\nbeta = %1.3f",track->GetName(),tr->GetChiSq(),beta));
           #endif       
          #else
           track->SetStdTitle();
          #endif
          break;
      }

      #if TOFDisplay ==1
      // initialize TEvePointSet to show Datapoints belonging to track
      TEvePointSetArray* psa= new TEvePointSetArray(Form("TEveTrack Points %d",i),"");
      psa->SetMarkerColor(iCol);
      psa->SetMarkerSize(2.0);
      psa->SetMarkerStyle(4);
      psa->InitBins("Hits",Np,0.5,Np+0.5);
      #endif

      for (Int_t n=0; n<Np; n++) {
	switch(iOpt){
	case 0:
	  point=tr->GetPoint(n); //pointer to membervaribale so GetFitPoint() would also change GetPoint()
          #if TOFDisplay ==1
          // follwing belongs to filling and labeling of PointSetArray
          psa->Fill(point[0],point[1],point[2],n+1);
          hit=tr->GetTofHitPointer(n);
          res_x=(point[0]-tr->GetFitX(point[2]))/hit->GetDx();
          res_y=(point[1]-tr->GetFitY(point[2]))/hit->GetDy();
          res_t=(point[3]-tr->GetFitT(point[2]))/hit->GetTimeError();
          res_z=0;
          psa->GetBin(n+1)->SetTitle(Form("%s\nPointId = %d\nResiduals:\nX = %1.3f\nY = %1.3f\nT = %1.3f",track->GetName(),tr->GetHitIndex(n),res_x,res_y,res_t));
          #endif
	  break;
	case 1:
	  point=tr->GetFitPoint(n);
	  break;
	}        
        track->SetPoint(n+1,point[0],point[1],point[2]);
        if(fVerbose>5) 
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
      #if TOFDisplay ==1
      if (iOpt==0){
        fPSList->AddElement(psa);
      }
      #endif
      track->SortPathMarksByTime();
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
    //gEve->DoRedraw3D();
    //gEve->Redraw3D(kTRUE);
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
  #if TOFDisplay ==1
  for (Int_t i=0; i<fEvePSList->GetEntriesFast(); i++) {
    TEveElementList*  ele=( TEveElementList*) fEvePSList->At(i);
    gEve->RemoveElement(ele,fEventManager);
  }
  fEvePSList->Clear();
  #endif
}

Char_t *gr;
TEveTrackList* CbmEvDisTracks::GetTrGroup(Int_t ihmul,Int_t iOpt)
{
  switch(iOpt){
  case 0:
    gr=Form("Trkl_hmul%d",ihmul);
    break;
  case 1:
    gr=Form("FTrkl_hmul%d",ihmul);
    break;
  }
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
    #if TOFDisplay ==1
    if(iOpt==1){  // delete if-condition to return to old code
      gEve->AddElement( fTrList ,fEventManager );
    }
    #else
    gEve->AddElement(fTrList, fEventManager );
    #endif
    fTrList->SetRecurse(kTRUE);
    switch(iOpt){
    case 0: //  display points
      fTrList->SetRnrPoints(kTRUE);
      fTrList->SetRnrLine(kFALSE);
      fTrList->SetMarkerSize(2.);
      fTrList->SetRnrChildren(fRenderP); // default not shown
      break;
    case 1:  //display fit line
      fTrList->SetRnrLine(kTRUE);
      fTrList->SetLineWidth(2.);
      fTrList->SetRnrChildren(fRenderT); // default not shown
      break;
    default:
      ;
    }
  }
  return fTrList;
}
#if TOFDisplay ==1
TEveElementList* CbmEvDisTracks::GetPSGroup(Int_t ihmul,Int_t iOpt)
{
  gr=Form("PTrkl_hmul%d",ihmul);
  fPSList=0;
  for (Int_t i=0; i<fEvePSList->GetEntriesFast(); i++) {
    TEveElementList* l=( TEveElementList*) fEvePSList->At(i);
    if ( strcmp(l->GetName(),gr)==0 ) {
      fPSList= l;
      break;
    }
  }
  if(fPSList ==0) {
    fPSList= new TEveElementList(gr);
    Int_t iCol=ihmul; if(iCol>4) iCol++;
    fPSList->SetMainColor(iCol);
    fEvePSList->Add(fPSList);
    gEve->AddElement(fPSList,fEventManager);
    fPSList->SetRnrChildren(fRenderP);
  }
  return fPSList;
}
#endif

ClassImp(CbmEvDisTracks)



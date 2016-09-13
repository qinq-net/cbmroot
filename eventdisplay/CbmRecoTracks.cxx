/********************************************************************************
 *    Copyright (C) 2016 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
// -------------------------------------------------------------------------
// -----                        CbmRecoTracks source file              -----
// -----                  Created 12/02/16  by T. Ablyazimov           -----
// -------------------------------------------------------------------------
#include "CbmRecoTracks.h"

#include "FairEventManager.h"           // for FairEventManager
#include "FairRootManager.h"            // for FairRootManager
#include "FairLogger.h"

#include "Riosfwd.h"                    // for ostream
#include "TClonesArray.h"               // for TClonesArray
#include "TEveManager.h"                // for TEveManager, gEve
#include "TEvePathMark.h"               // for TEvePathMark
#include "TEveTrackPropagator.h"        // for TEveTrackPropagator
#include "TEveVector.h"                 // for TEveVector, TEveVectorT
#include "TGeoTrack.h"                  // for TGeoTrack
#include "TMathBase.h"                  // for Max, Min
#include "TObjArray.h"                  // for TObjArray
#include "TParticle.h"                  // for TParticle
#include "CbmGlobalTrack.h"
#include "rich/CbmRichRing.h"
#include "CbmStsTrack.h"

#include <string.h>                     // for NULL, strcmp
#include <iostream>                     // for operator<<, basic_ostream, etc


// -----   Default constructor   -------------------------------------------
CbmRecoTracks::CbmRecoTracks()
  : FairTask("FairMCTracks", 0),
    fGlobalTracks(NULL),
    fMvdHits(NULL),
    fStsHits(NULL),
    fStsTracks(NULL),
    fRichRings(NULL),
    fRichHits(NULL),
    fMuchPixelHits(NULL),
    fMuchTracks(NULL),
    fTrdHits(NULL),
    fTrdTracks(NULL),
    fTofHits(NULL),
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
CbmRecoTracks::CbmRecoTracks(const char* name, Int_t iVerbose)
  : FairTask(name, iVerbose),
    fGlobalTracks(NULL),
    fMvdHits(NULL),
    fStsHits(NULL),
    fStsTracks(NULL),
    fRichRings(NULL),
    fRichHits(NULL),
    fMuchPixelHits(NULL),
    fMuchTracks(NULL),
    fTrdHits(NULL),
    fTrdTracks(NULL),
    fTofHits(NULL),
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
InitStatus CbmRecoTracks::Init()
{
  LOG(DEBUG) <<  "FairMCTracks::Init()" << FairLogger::endl; 
  FairRootManager* fManager = FairRootManager::Instance();
  fGlobalTracks = (TClonesArray*) fManager->GetObject("GlobalTrack");
  fStsTracks = (TClonesArray*)fManager->GetObject("StsTrack");
  fMvdHits = (TClonesArray*) fManager->GetObject("MvdHit");
  fStsHits = (TClonesArray*) fManager->GetObject("StsHit");
  fRichRings = (TClonesArray*) fManager->GetObject("RichRing");
  fRichHits = (TClonesArray*) fManager->GetObject("RichHit");
  fMuchPixelHits = (TClonesArray*) fManager->GetObject("MuchPixelHit");
  fMuchTracks = (TClonesArray*) fManager->GetObject("MuchTrack");
  fTrdHits = (TClonesArray*) fManager->GetObject("TrdHit");
  fTrdTracks = (TClonesArray*) fManager->GetObject("TrdTrack");
  fTofHits = (TClonesArray*) fManager->GetObject("TofHit");
  
  if(fGlobalTracks == 0) {
    LOG(ERROR) << "FairMCTracks::Init()  branch " << GetName() << "GlobalTrack branch not found! Task will be deactivated "<< FairLogger::endl;
    SetActive(kFALSE);
  }
  
  LOG(DEBUG1) <<  "FairMCTracks::Init() get track list" << fStsTracks << FairLogger::endl; 
  LOG(DEBUG1) <<  "FairMCTracks::Init()  create propagator" << FairLogger::endl; 
  fEventManager =FairEventManager::Instance();
  LOG(DEBUG1) <<  "FairMCTracks::Init() get instance of FairEventManager " << FairLogger::endl; 
  fEvent = "Current Event";
  MinEnergyLimit=fEventManager->GetEvtMinEnergy();
  MaxEnergyLimit=fEventManager->GetEvtMaxEnergy();
  PEnergy=0;
  if(IsActive()) { return kSUCCESS; }
  else { return kERROR; }
}

void CbmRecoTracks::HandlePixelHit(TEveTrack* eveTrack, Int_t& n, const CbmPixelHit* hit, TEveVector* pMom = 0)
{
    eveTrack->SetPoint(n, hit->GetX(), hit->GetY(), hit->GetZ());
    TEveVector pos = TEveVector(hit->GetX(), hit->GetY(), hit->GetZ());
    TEvePathMark path;
    path.fV = pos;
    path.fTime= 0;
    
    if (pMom)
        path.fP = *pMom;
    
    eveTrack->AddPathMark(path);
    ++n;
}

void CbmRecoTracks::HandleTrack(TEveTrack* eveTrack, Int_t& n, const CbmTrack* recoTrack)
{
   Int_t nofHits = recoTrack->GetNofHits();
   
   for (Int_t i = 0; i < nofHits; ++i)
   {
      HitType hitType = recoTrack->GetHitType(i);
      Int_t hitIndex = recoTrack->GetHitIndex(i);
      const CbmPixelHit* pixelHit = 0;
      
      switch (hitType)
      {
         case kRICHHIT:
            pixelHit = static_cast<const CbmPixelHit*> (fRichHits->At(hitIndex));
            break;
            
         case kMUCHPIXELHIT:
            pixelHit = static_cast<const CbmPixelHit*> (fMuchPixelHits->At(hitIndex));
            break;
            
         case kTRDHIT:
            pixelHit = static_cast<const CbmPixelHit*> (fTrdHits->At(hitIndex));
            break;
            
         case kTOFHIT:
            pixelHit = static_cast<const CbmPixelHit*> (fTofHits->At(hitIndex));
            break;
      }
      
      if (0 != pixelHit)
         HandlePixelHit(eveTrack, n, pixelHit);
   }
}

void CbmRecoTracks::HandleStsTrack(TEveTrack* eveTrack, Int_t& n, const CbmStsTrack* stsTrack)
{
    for (Int_t i = 0; i < stsTrack->GetNofMvdHits(); ++i)
    {
        const CbmPixelHit* hit = static_cast<const CbmPixelHit*> (fMvdHits->At(stsTrack->GetMvdHitIndex(i)));
        
        if (0 == n)
        {
            TVector3 mom3;
            stsTrack->GetParamFirst()->Momentum(mom3);
            TEveVector mom = TEveVector(mom3.X(), mom3.Y(), mom3.Z());
            HandlePixelHit(eveTrack, n, hit, &mom);
        }
        else
            HandlePixelHit(eveTrack, n, hit);
    }
    
    for (Int_t i = 0; i < stsTrack->GetNofStsHits(); ++i)
    {
        const CbmPixelHit* hit = static_cast<const CbmPixelHit*> (fStsHits->At(stsTrack->GetStsHitIndex(i)));
        
        if (0 == n)
        {
            TVector3 mom3;
            stsTrack->GetParamFirst()->Momentum(mom3);
            TEveVector mom = TEveVector(mom3.X(), mom3.Y(), mom3.Z());
            HandlePixelHit(eveTrack, n, hit, &mom);
        }
        else
            HandlePixelHit(eveTrack, n, hit);
    }
}

void CbmRecoTracks::Exec(Option_t* /*option*/)
{

  if (IsActive()) {

    LOG(DEBUG1) << " CbmRecoTracks::Exec "<< FairLogger::endl; 

    Reset();
    
    Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();

    for (Int_t i = 0; i < nofGlobalTracks; ++i) 
    {
        LOG(DEBUG3) << "CbmRecoTracks::Exec " << i << FairLogger::endl;
        const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTracks->At(i));
        Int_t stsId = globalTrack->GetStsTrackIndex();
        Int_t richId = globalTrack->GetRichRingIndex();
        Int_t muchId = globalTrack->GetMuchTrackIndex();
        Int_t trdId = globalTrack->GetTrdTrackIndex();
        Int_t tofId = globalTrack->GetTofHitIndex();
        
        if (0 > stsId)
            continue;
        
        const CbmStsTrack* stsTrack = static_cast<const CbmStsTrack*> (fStsTracks->At(stsId));
        
        if (0 == stsTrack)
           continue;
        
        Int_t pdg = stsTrack->GetPidHypo();
        TParticle P;
        P.SetPdgCode(pdg);
        fTrList = GetTrGroup(&P);
        TEveTrack* eveTrack = new TEveTrack(&P, pdg, fTrPr);
        eveTrack->SetLineColor(fEventManager->Color(pdg));
        Int_t n = 0;
        HandleStsTrack(eveTrack, n, stsTrack);
        //LOG(INFO) << "GetPidHypo: " << stsTrack->GetPidHypo() << FairLogger::endl;
        
        if (-1 < richId)
        {
            const CbmRichRing* r = static_cast<const CbmRichRing*> (fRichRings->At(richId));
            const CbmPixelHit* rh = static_cast<const CbmPixelHit*> (fRichHits->At(r->GetHit(0)));
            CbmPixelHit h(*rh);
            h.SetX(r->GetCenterX());
            h.SetY(r->GetCenterY());
            HandlePixelHit(eveTrack, n, &h);
        }
        else if (-1 < muchId)
            HandleTrack(eveTrack, n, static_cast<const CbmTrack*> (fMuchTracks->At(muchId)));
        
        if (-1 < trdId)
            HandleTrack(eveTrack, n, static_cast<const CbmTrack*> (fTrdTracks->At(trdId)));
        
        if (-1 < tofId)
            HandlePixelHit(eveTrack, n, static_cast<const CbmPixelHit*> (fTofHits->At(tofId)));     
      
      fTrList->AddElement(eveTrack);
      LOG(DEBUG3) << "track added " << eveTrack->GetName() << FairLogger::endl; 

    }
   
    //fEventManager->SetEvtMaxEnergy(MaxEnergyLimit);
    //fEventManager->SetEvtMinEnergy(MinEnergyLimit);
    gEve->Redraw3D(kFALSE);
  }
}
// -----   Destructor   ----------------------------------------------------
CbmRecoTracks::~CbmRecoTracks()
{
}
// -------------------------------------------------------------------------
void CbmRecoTracks::SetParContainers()
{

}

// -------------------------------------------------------------------------
void CbmRecoTracks::Finish()
{

}
// -------------------------------------------------------------------------
void CbmRecoTracks::Reset()
{
  for (Int_t i=0; i<fEveTrList->GetEntriesFast(); i++) {
    TEveTrackList*  ele=( TEveTrackList*) fEveTrList->At(i);
    gEve->RemoveElement(ele,fEventManager);
  }
  fEveTrList->Clear();
}

TEveTrackList* CbmRecoTracks::GetTrGroup(TParticle* P)
{
    char name_buf[128];
    sprintf(name_buf, "reco_%s", P->GetName());
  fTrList=0;
  for (Int_t i=0; i<fEveTrList->GetEntriesFast(); i++) {
    TEveTrackList* TrListIn=( TEveTrackList*) fEveTrList->At(i);
    if ( strcmp(TrListIn->GetName(),name_buf)==0 ) {
      fTrList= TrListIn;
      break;
    }
  }
  if(fTrList ==0) {
    fTrPr=new TEveTrackPropagator();
    fTrList= new  TEveTrackList(name_buf,fTrPr );
    fTrList->SetMainColor(fEventManager->Color(P->GetPdgCode()));
    fEveTrList->Add(fTrList);
    gEve->AddElement( fTrList ,fEventManager );
    fTrList->SetRnrLine(kTRUE);
  }
  return fTrList;
}

ClassImp(CbmRecoTracks)



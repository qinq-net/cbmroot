/** CbmLitFindGlobalTracks.cxx
 * @author Andrey Lebedev <andrey.lebedev@gsi.de>
 * @since 2009
 * @version 1.0
 **/

#include "CbmLitFindGlobalTracks.h"
#include "base/CbmLitToolFactory.h"
#include "base/CbmLitTrackingGeometryConstructor.h"
#include "data/CbmLitHit.h"
#include "data/CbmLitPixelHit.h"
#include "data/CbmLitStripHit.h"
#include "data/CbmLitTrack.h"
#include "utils/CbmLitConverter.h"
#include "utils/CbmLitMemoryManagment.h"
#include "propagation/CbmLitTGeoTrackPropagator.h"

#include "CbmHit.h"
#include "CbmPixelHit.h"
#include "CbmStripHit.h"
#include "CbmStsTrack.h"
#include "CbmTrdTrack.h"
#include "CbmMuchTrack.h"
#include "CbmGlobalTrack.h"
#include "CbmTofTrack.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "CbmStsHit.h"
#include "CbmKFParticleInterface.h"

#include "TClonesArray.h"

#include <iostream>
#include <cmath>

CbmLitFindGlobalTracks::CbmLitFindGlobalTracks()
 : FairTask("CbmLitFindGlobalTracks"),
   fDet(),

   fStsTracks(NULL),
   fMvdHits(NULL),
   fStsHits(NULL),
   fMuchPixelHits(NULL),
   fMuchStrawHits(NULL),
   fMuchTracks(NULL),
   fTrdHits(NULL),
   fTrdTracks(NULL),
   fTofHits(NULL),
   fEvents(NULL),
   fTofTracks(NULL),
   fGlobalTracks(NULL),
   fPrimVertex(NULL),

   fLitStsTracks(),
   fLitHits(),
   fLitTofHits(),
   fLitOutputTracks(),
   fLitOutputTofTracks(),

   fFinder(),
   fMerger(),
   fFitter(),
   fPropagator(),

   fTrackingType("branch"),
   fMergerType("nearest_hit"),
   fFitterType("lit_kalman"),

   fTrackingWatch(),
   fMergerWatch(),

   fEventNo(0)
{

}

CbmLitFindGlobalTracks::~CbmLitFindGlobalTracks()
{

}

InitStatus CbmLitFindGlobalTracks::Init()
{
   fDet.DetermineSetup();
   std::cout << fDet.ToString();

   ReadAndCreateDataBranches();

   InitTrackReconstruction();

   fTrackingWatch.Reset();
   fMergerWatch.Reset();

   return kSUCCESS;
}

void CbmLitFindGlobalTracks::Exec(
   Option_t* opt)
{
   if (fTrdTracks != NULL) fTrdTracks->Delete();
   if (fMuchTracks != NULL) fMuchTracks->Delete();
   if (fTofTracks != NULL) fTofTracks->Delete();
   fGlobalTracks->Clear();
   
   if (fEvents)
   {
      Int_t nEvents = fEvents->GetEntriesFast();
      LOG(DEBUG) << GetName() << ": reading time slice with " << nEvents << " events " << FairLogger::endl;
      
      for (Int_t iEvent = 0; iEvent < nEvents; iEvent++)
      {
         CbmEvent* event = static_cast<CbmEvent*> (fEvents->At(iEvent));
         ConvertInputData(event);
         RunTrackReconstruction();
         ConvertOutputData(event);
         CalculateLength(event);
         CalculatePrimaryVertexParameters(event);
         ClearArrays();
         std::cout << "CbmLitFindGlobalTracks::Exec event: " << event->GetNumber() << std::endl;
      } //# events
   } //? event branch present
   else
   {// Old event-by-event simulation without event branch
      ConvertInputData(0);
      RunTrackReconstruction();
      ConvertOutputData(0);
      CalculateLength(0);
      CalculatePrimaryVertexParameters(0);
      ClearArrays();
      std::cout << "CbmLitFindGlobalTracks::Exec event: " << fEventNo++ << std::endl;
   }
}

void CbmLitFindGlobalTracks::SetParContainers()
{
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb = ana->GetRuntimeDb();

   rtdb->getContainer("FairBaseParSet");
   rtdb->getContainer("CbmGeoMuchPar");
}

void CbmLitFindGlobalTracks::Finish()
{
   PrintStopwatchStatistics();
}

void CbmLitFindGlobalTracks::ReadAndCreateDataBranches()
{
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("Init","CbmRootManager is not instantiated"); }

   if (fDet.GetDet(kMvd)) {
	   fMvdHits = (TClonesArray*) ioman->GetObject("MvdHit");
	   if (NULL == fMvdHits) { Fatal("Init","No MvdHit array!"); }
   }

   //STS data
   fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
   if (NULL == fStsTracks) { Fatal("Init","No StsTrack array!"); }
   fStsHits = (TClonesArray*) ioman->GetObject("StsHit");
   if (NULL == fStsHits) { Fatal("Init","No StsHit array!"); }

   //MUCH data
   if (fDet.GetDet(kMuch)) {
      fMuchPixelHits = (TClonesArray*) ioman->GetObject("MuchPixelHit");
      fMuchStrawHits = (TClonesArray*) ioman->GetObject("MuchStrawHit");
      if (NULL == fMuchPixelHits && NULL == fMuchStrawHits) { Fatal("Init", "No MuchPixelHit or MuchStrawHit array!"); }
      if (fMuchPixelHits) { std::cout << "-I- MuchPixelHit branch found in tree" << std::endl; }
      if (fMuchStrawHits) { std::cout << "-I- MuchStrawHit branch found in tree" << std::endl; }
   }

   //TRD data
   if (fDet.GetDet(kTrd)) {
      fTrdHits = (TClonesArray*) ioman->GetObject("TrdHit");
      if (NULL == fTrdHits) { Fatal("Init", "No TRDHit array!"); }
      std::cout << "-I- TRDHit branch found in tree" << std::endl;
   }

   //TOF data
   if (fDet.GetDet(kTof)) {
      fTofHits = (TClonesArray*) ioman->GetObject("TofHit");
      if (NULL == fTofHits) { Fatal("Init", "No TofHit array!"); }
      std::cout << "-I- TofHit branch found in tree" << std::endl;
   }
   
   fEvents = dynamic_cast<TClonesArray*> (ioman->GetObject("Event"));

   // Create and register track arrays
   fGlobalTracks = new TClonesArray("CbmGlobalTrack",100);
   ioman->Register("GlobalTrack", "Global", fGlobalTracks, IsOutputBranchPersistent("GlobalTrack"));

   if (fDet.GetDet(kMuch)) {
      fMuchTracks = new TClonesArray("CbmMuchTrack", 100);
      ioman->Register("MuchTrack", "Much", fMuchTracks, IsOutputBranchPersistent("MuchTrack"));
   }

   if (fDet.GetDet(kTrd)) {
      fTrdTracks = new TClonesArray("CbmTrdTrack", 100);
      ioman->Register("TrdTrack", "Trd", fTrdTracks, IsOutputBranchPersistent("TrdTrack"));
   }

   if (fDet.GetDet(kTof)) {
      fTofTracks = new TClonesArray("CbmTofTrack", 100);
      ioman->Register("TofTrack", "Tof", fTofTracks, IsOutputBranchPersistent("TofTrack"));
   }

   //fPrimVertex = (CbmVertex*) ioman->GetObject("PrimaryVertex");
   // Get pointer to PrimaryVertex object from IOManager if it exists
   // The old name for the object is "PrimaryVertex" the new one
   // "PrimaryVertex." Check first for the new name
   fPrimVertex = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex."));
   if (nullptr == fPrimVertex) {
    fPrimVertex = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex"));
   }
   if (nullptr == fPrimVertex) {
  //   LOG(FATAL) << "No primary vertex" << FairLogger::endl;
  }


}

void CbmLitFindGlobalTracks::InitTrackReconstruction()
{
   if (fDet.GetElectronSetup()) {
      if (fTrackingType == "branch" || fTrackingType == "nn" || fTrackingType == "nn_parallel") {
         std::string st("e_");
         st += fTrackingType;
         fFinder = CbmLitToolFactory::CreateTrackFinder(st);
      } else {
         TObject::Fatal("CbmLitFindGlobalTracks","Tracking type not found");
      }
   } else {
      if (fTrackingType == "branch" || fTrackingType == "nn" || fTrackingType == "nn_parallel") {
         std::string st("mu_");
         st += fTrackingType;
         fFinder = CbmLitToolFactory::CreateTrackFinder(st);
      } else {
         TObject::Fatal("CbmLitFindGlobalTracks","Tracking type not found");
      }
   }

   if (fDet.GetDet(kTof)) {
      if (fMergerType == "nearest_hit" || fMergerType == "all_hits") {
         fMerger = CbmLitToolFactory::CreateHitToTrackMerger("tof_" + fMergerType);
      } else {
         TObject::Fatal("CbmLitFindGlobalTracks","Merger type not found");
      }
   }

   if (fFitterType == "lit_kalman") {
      fFitter = CbmLitToolFactory::CreateTrackFitter("lit_kalman");
   } else {
      TObject::Fatal("CbmLitFindGlobalTracks","Fitter type not found");
   }

   fPropagator = CbmLitToolFactory::CreateTrackPropagator("lit");
}

void CbmLitFindGlobalTracks::ConvertInputData(CbmEvent* event)
{
   CbmLitConverter::StsTrackArrayToTrackVector(event, fStsTracks, fLitStsTracks);
   std::cout << "-I- CbmLitFindGlobalTracks: Number of STS tracks: " << fLitStsTracks.size() << std::endl;

   if (fMuchPixelHits) { CbmLitConverter::HitArrayToHitVector(event, kMuchPixelHit, fMuchPixelHits, fLitHits); }
   if (fMuchStrawHits) { CbmLitConverter::HitArrayToHitVector(event, kMuchStrawHit, fMuchStrawHits, fLitHits); }
   if (fTrdHits) {
      CbmLitConverter::HitArrayToHitVector(event, kTrdHit, fTrdHits, fLitHits);
      //If MUCH-TRD setup, than shift plane id for the TRD hits
      if (fDet.GetDet(kMuch) && fDet.GetDet(kTrd)) {
         Int_t nofStations = CbmLitTrackingGeometryConstructor::Instance()->GetNofMuchStations();
         for (Int_t i = 0; i < fLitHits.size(); i++) {
            CbmLitHit* hit = fLitHits[i];
            if (hit->GetSystem() == kLITTRD) { hit->SetDetectorId(kLITTRD, hit->GetStation() + nofStations); }
         }
      }
   }
   std::cout << "-I- CbmLitFindGlobalTracks: Number of hits: " << fLitHits.size() << std::endl;

   if (fTofHits) {
      CbmLitConverter::HitArrayToHitVector(event, kTofHit, fTofHits, fLitTofHits);
      std::cout << "-I- CbmLitFindGlobalTracks: Number of TOF hits: " << fLitTofHits.size() << std::endl;
   }
}

void CbmLitFindGlobalTracks::ConvertOutputData(CbmEvent* event)
{
   CbmLitConverter::LitTrackVectorToGlobalTrackArray(event, fLitOutputTracks, fLitOutputTofTracks, fGlobalTracks, fStsTracks, fTrdTracks, fMuchTracks, fTofTracks);
}

void CbmLitFindGlobalTracks::CalculateLength(CbmEvent* event)
{
   if (fTofTracks == NULL || fGlobalTracks == NULL) return;
   
   CbmVertex* primVertex = event ? event->GetVertex() : fPrimVertex;

   /* Calculate the length of the global track
    * starting with (0, 0, 0) and adding all
    * distances between hits
    */
   Int_t nofTofTracks = event ? event->GetNofData(kTofTrack) : fTofTracks->GetEntriesFast();
   for (Int_t i = 0; i < nofTofTracks; ++i) {
      Int_t itt = event ? event->GetIndex(kTofTrack, i) : i;
      CbmTofTrack* tofTrack = static_cast<CbmTofTrack*>(fTofTracks->At(itt));
      CbmGlobalTrack* globalTrack = static_cast<CbmGlobalTrack*>(fGlobalTracks->At(tofTrack->GetTrackIndex()));
      if (globalTrack == NULL) { continue; }

      std::vector<Double_t> X, Y, Z;
      if (primVertex == NULL) {
    	  X.push_back(0.);
    	  Y.push_back(0.);
    	  Z.push_back(0.);
      } else {
    	  X.push_back(primVertex->GetX());
    	  Y.push_back(primVertex->GetY());
    	  Z.push_back(primVertex->GetZ());
      }

      // get track segments indices
      Int_t stsId = globalTrack->GetStsTrackIndex();
      Int_t trdId = globalTrack->GetTrdTrackIndex();
      Int_t muchId = globalTrack->GetMuchTrackIndex();
      Int_t tofId = tofTrack->GetTofHitIndex();//globalTrack->GetTofHitIndex();

      if (stsId > -1) {
         const CbmStsTrack* stsTrack = static_cast<const CbmStsTrack*>(fStsTracks->At(stsId));
         Int_t nofStsHits = stsTrack->GetNofStsHits();
         for(Int_t ih = 0; ih < nofStsHits; ih++) {
            CbmStsHit* hit = (CbmStsHit*) fStsHits->At(stsTrack->GetHitIndex(ih));
            X.push_back(hit->GetX());
            Y.push_back(hit->GetY());
            Z.push_back(hit->GetZ());
         }
      }

      if (muchId > -1) {
         const CbmTrack* muchTrack = static_cast<const CbmTrack*>(fMuchTracks->At(muchId));
         Int_t nofMuchHits = muchTrack->GetNofHits();
         for(Int_t ih = 0; ih < nofMuchHits; ih++) {
            HitType hitType = muchTrack->GetHitType(ih);
            if (hitType == kMuchPixelHit) {
               CbmPixelHit* hit = (CbmPixelHit*) fMuchPixelHits->At(muchTrack->GetHitIndex(ih));
               X.push_back(hit->GetX());
               Y.push_back(hit->GetY());
               Z.push_back(hit->GetZ());
            } else if (hitType == kMuchStrawHit) {

            }
         }
      }

      if (trdId > -1) {
         const CbmTrack* trdTrack = static_cast<const CbmTrack*>(fTrdTracks->At(trdId));
         Int_t nofTrdHits = trdTrack->GetNofHits();
         for(Int_t ih = 0; ih < nofTrdHits; ih++) {
            CbmPixelHit* hit = (CbmPixelHit*) fTrdHits->At(trdTrack->GetHitIndex(ih));
            X.push_back(hit->GetX());
            Y.push_back(hit->GetY());
            Z.push_back(hit->GetZ());
         }
      }

      if (tofId > -1) {
         const CbmPixelHit* hit = static_cast<const CbmPixelHit*>(fTofHits->At(tofId));
         X.push_back(hit->GetX());
         Y.push_back(hit->GetY());
         Z.push_back(hit->GetZ());
      }

      // Calculate distances between hits
      Double_t length = 0.;
      for (Int_t j = 0; j < X.size() - 1; ++j) {
         Double_t dX = X[j] - X[j+1];
         Double_t dY = Y[j] - Y[j+1];
         Double_t dZ = Z[j] - Z[j+1];
         length += std::sqrt(dX*dX + dY*dY + dZ*dZ);
      }

      if (globalTrack->GetTofHitIndex() == tofTrack->GetTofHitIndex()) globalTrack->SetLength(length);
      tofTrack->SetTrackLength(length);
   }
}

//void CbmLitFindGlobalTracks::CalculateLength()
//{
//   /* Calculate the length of the global track
//    * starting with (0, 0, 0) and adding all
//    * distances between hits
//    */
//
//	// Reduce step to calculate track length more accurately
//	CbmLitTGeoTrackPropagator::MAXIMUM_PROPAGATION_STEP_SIZE = 1.0;
//
//	TrackPtrVector litTracks;
//	CbmLitConverter::GlobalTrackArrayToLitTrackVector(fGlobalTracks, fStsTracks, fTrdTracks, fMuchTracks, fMvdHits, fStsHits, fTrdHits, fMuchStrawHits, fMuchPixelHits, fTofHits, litTracks);
//	Int_t nofTracks = litTracks.size();
//	for (UInt_t iTrack = 0; iTrack < nofTracks; iTrack++) {
//		CbmGlobalTrack* globalTrack = static_cast<CbmGlobalTrack*>(fGlobalTracks->At(iTrack));
//		if (globalTrack->GetStsTrackIndex() >= 0 && globalTrack->GetTofHitIndex() >= 0) {
//			CbmLitTrack* track = litTracks[iTrack];
//			CbmLitTrackParam par = *track->GetParamFirst();
//			LitStatus propStatus = fPropagator->Propagate(&par, 0.0, 211, NULL);
//			track->SetParamFirst(&par);
//			fFitter->Fit(track);
//			globalTrack->SetLength(track->GetLength());
//		} else {
//			globalTrack->SetLength(-1.);
//		}
//	}
//
//	CbmLitTGeoTrackPropagator::MAXIMUM_PROPAGATION_STEP_SIZE = 10.;
//
//	// free memory
//	for (UInt_t iTrack = 0; iTrack < nofTracks; iTrack++) {
//		CbmLitTrack* track = litTracks[iTrack];
//		Int_t nofHits = track->GetNofHits();
//		for (Int_t iHit = 0; iHit < nofHits; iHit++) {
//			delete track->GetHit(iHit);
//		}
//		track->ClearHits();
//	}
//	for_each(litTracks.begin(), litTracks.end(), DeleteObject());
//}

void CbmLitFindGlobalTracks::CalculatePrimaryVertexParameters(CbmEvent* event)
{
    if (0 == fGlobalTracks)
        return;
    
    CbmVertex* primVertex = event ? event->GetVertex() : fPrimVertex;
    
    if (0 == primVertex)
       return;
    
    Int_t nofGlobalTracks = event ? event->GetNofData(kGlobalTrack) : fGlobalTracks->GetEntriesFast();
    
    for (Int_t i0 = 0; i0 < nofGlobalTracks; ++i0)
    {
      Int_t i = event ? event->GetIndex(kGlobalTrack, i0) : i0;
      CbmGlobalTrack* globalTrack = static_cast<CbmGlobalTrack*> (fGlobalTracks->At(i));
      Int_t stsId = globalTrack->GetStsTrackIndex();
      CbmStsTrack* stsTrack = static_cast<CbmStsTrack*>(fStsTracks->At(stsId));
      FairTrackParam vtxTrackParam;
      float chiSqPrimary = 0.f;
      CbmKFParticleInterface::ExtrapolateTrackToPV(stsTrack, primVertex, &vtxTrackParam, chiSqPrimary);
      globalTrack->SetParamPrimaryVertex(&vtxTrackParam);
    }
}

void CbmLitFindGlobalTracks::ClearArrays()
{
   // Free memory
   for_each(fLitStsTracks.begin(), fLitStsTracks.end(), DeleteObject());
   for_each(fLitOutputTracks.begin(), fLitOutputTracks.end(), DeleteObject());
   for_each(fLitHits.begin(), fLitHits.end(), DeleteObject());
   for_each(fLitTofHits.begin(), fLitTofHits.end(), DeleteObject());
   for_each(fLitOutputTofTracks.begin(), fLitOutputTofTracks.end(), DeleteObject());
   fLitStsTracks.clear();
   fLitOutputTracks.clear();
   fLitHits.clear();
   fLitTofHits.clear();
   fLitOutputTofTracks.clear();
}

void CbmLitFindGlobalTracks::RunTrackReconstruction()
{
   // Track finding in TRD or MUCH
   if (fDet.GetDet(kMuch) || fDet.GetDet(kTrd)) {
      fTrackingWatch.Start(kFALSE);
      fFinder->DoFind(fLitHits, fLitStsTracks, fLitOutputTracks);
      fTrackingWatch.Stop();
   }
   // Merging of TOF hits to global tracks
   if (fDet.GetDet(kTof)) {
      // If there are no TRD or MUCH than merge STS tracks with TOF
      if (!(fDet.GetDet(kMuch) || fDet.GetDet(kTrd))) {
         for(TrackPtrIterator it = fLitStsTracks.begin(); it != fLitStsTracks.end(); it++) {
            CbmLitTrack* track = new CbmLitTrack(*(*it));
            fLitOutputTracks.push_back(track);
         }
      }

      // Selection of tracks to be merged with TOF
      if (fDet.GetDet(kMuch) || fDet.GetDet(kTrd)) {
         SelectTracksForTofMerging();
      } else {
         for (TrackPtrIterator it = fLitOutputTracks.begin(); it != fLitOutputTracks.end(); it++) {
            (*it)->SetQuality(kLITGOODMERGE);
         }
      }

      fMergerWatch.Start(kFALSE);
      fMerger->DoMerge(fLitTofHits, fLitOutputTracks, fLitOutputTofTracks);
      fMergerWatch.Stop();
   }

   // Refit found tracks
   for(TrackPtrIterator it = fLitOutputTracks.begin(); it != fLitOutputTracks.end(); it++) {
      CbmLitTrack* track = *it;
      fFitter->Fit(track);
   }
}

void CbmLitFindGlobalTracks::SelectTracksForTofMerging()
{
   // The aim of this procedure is to select only those tracks
   // which have at least one hit in the last station group.
   // Only those tracks will be propagated further and merged
   // with TOF hits.

   Int_t nofStations = CbmLitTrackingGeometryConstructor::Instance()->GetNofMuchTrdStations();
   //   Int_t stationCut = nofStations - 4;
   // TODO: Fix this issue in a better way. This is done only as an ugly fix
   // FU 19.09.13 
   Int_t stationCut = nofStations - 3;

   for(TrackPtrIterator it = fLitOutputTracks.begin(); it != fLitOutputTracks.end(); it++) {
      CbmLitTrack* track = *it;
      if (track->GetQuality() == kLITBAD) { continue; }
      const CbmLitHit* hit = track->GetHit(track->GetNofHits() - 1);
      if (hit->GetStation() >= stationCut) {
         // OK select this track for further merging with TOF
         track->SetQuality(kLITGOODMERGE);
      }
   }
}

void CbmLitFindGlobalTracks::PrintStopwatchStatistics()
{
   std::cout << "Stopwatch: " << std::endl;
   std::cout << "tracking: counts=" << fTrackingWatch.Counter()
             << ", real=" << fTrackingWatch.RealTime()/fTrackingWatch.Counter()
             << "/" << fTrackingWatch.RealTime()
             << " s, cpu=" << fTrackingWatch.CpuTime()/fTrackingWatch.Counter()
             << "/" << fTrackingWatch.CpuTime() << std::endl;
   std::cout << "fitter: real=" << fMergerWatch.Counter()
             << ", real=" << fMergerWatch.RealTime()/fMergerWatch.Counter()
             << "/" << fMergerWatch.RealTime()
             << " s, cpu=" << fMergerWatch.CpuTime()/fMergerWatch.Counter()
             << "/" << fMergerWatch.CpuTime() << std::endl;
}

ClassImp(CbmLitFindGlobalTracks);

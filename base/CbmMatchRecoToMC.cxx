/*
 * CbmMatchRecoToMC.cxx
 *
 *  Created on: Oct 17, 2013
 *      Author: andrey
 */

#include "CbmMatchRecoToMC.h"
#include "CbmMatch.h"
#include "CbmTrackMatchNew.h"
#include "CbmCluster.h"
#include "CbmHit.h"
#include "CbmStsHit.h"
#include "CbmTrack.h"
#include "CbmStsTrack.h"
#include "CbmDigi.h"
#include "FairMCPoint.h"
#include "FairLogger.h"
#include "TClonesArray.h"

#include "CbmTofDigi.h"       // in cbmdata/tof
#include "CbmTofDigiExp.h"    // in cbmdata/tof
#include "CbmTofHit.h"        // in cbmdata/tof

#include "CbmMCDataArray.h"
#include "CbmMCDataManager.h"

#include "CbmMCTrack.h"////
#include "CbmStsAddress.h"////

#include "CbmRichRing.h"
#include "CbmRichHit.h"
#include "CbmRichDigi.h"
#include "CbmRichPoint.h"

using std::vector;
using std::pair;

Int_t CbmMatchRecoToMC::fEventNumber = 0;

CbmMatchRecoToMC::CbmMatchRecoToMC() :
   FairTask("CbmMatchRecoToMC"),
   fIncludeMvdHitsInStsTrack(kFALSE),
   fMCTracks(NULL),
   //fMCTracksArray(NULL),
   fStsPoints(NULL),
   fStsDigis(NULL),
   fStsClusters(NULL),
   fStsHits(NULL),
   fStsTracks(NULL),
   fStsDigiMatches(NULL),
   fStsClusterMatches(NULL),
   fStsHitMatches(NULL),
   fStsTrackMatches(NULL),
   fRichDigis(NULL),
   fRichHits(NULL),
   fRichRings(NULL),
   fRichMcPoints(NULL),
   fRichTrackMatches(NULL),
   fTrdPoints(NULL),
   fTrdDigis(NULL),
   fTrdClusters(NULL),
   fTrdHits(NULL),
   fTrdTracks(NULL),
   fTrdDigiMatches(NULL),
   fTrdClusterMatches(NULL),
   fTrdHitMatches(NULL),
   fTrdTrackMatches(NULL),
   fMuchPoints(NULL),
   fMuchPixelDigis(NULL),
   fMuchStrawDigis(NULL),
   fMuchClusters(NULL),
   fMuchPixelHits(NULL),
   fMuchStrawHits(NULL),
   fMuchTracks(NULL),
   fMuchPixelDigiMatches(NULL),
   fMuchStrawDigiMatches(NULL),
   fMuchClusterMatches(NULL),
   fMuchPixelHitMatches(NULL),
   fMuchStrawHitMatches(NULL),
   fMuchTrackMatches(NULL),
   fMvdPoints(NULL),
   fMvdHits(NULL),
   fMvdDigiMatches(NULL),
   fMvdHitMatches(NULL),
   fMvdCluster(NULL),
   fMvdClusterMatches(NULL),
   fTofPoints(NULL),
   fTofDigis(NULL),
   fTofHits(NULL),
   fbDigiExpUsed(kFALSE),
   fTofDigiMatchesPoints(NULL),
   fTofDigiMatches(NULL),
   fTofHitMatches(NULL)
{

}

CbmMatchRecoToMC::~CbmMatchRecoToMC()
{
   if (fStsClusterMatches != NULL) {
      fStsClusterMatches->Delete();
      delete fStsClusterMatches;
   }
   if (fStsHitMatches != NULL) {
      fStsHitMatches->Delete();
      delete fStsHitMatches;
   }
   if (fStsTrackMatches) {
      fStsTrackMatches->Delete();
      delete fStsTrackMatches;
   }

   if (fRichTrackMatches) {
      fRichTrackMatches->Delete();
      delete fRichTrackMatches;
   }

   if (fTrdClusterMatches != NULL) {
      fTrdClusterMatches->Delete();
      delete fTrdClusterMatches;
   }
   if (fTrdHitMatches != NULL) {
      fTrdHitMatches->Delete();
      delete fTrdHitMatches;
   }
   if (fTrdTrackMatches) {
      fTrdTrackMatches->Delete();
      delete fTrdTrackMatches;
   }

   if (fMuchClusterMatches != NULL) {
      fMuchClusterMatches->Delete();
      delete fMuchClusterMatches;
   }
   if (fMuchPixelHitMatches != NULL) {
      fMuchPixelHitMatches->Delete();
      delete fMuchPixelHitMatches;
   }
   if (fMuchStrawHitMatches != NULL) {
      fMuchStrawHitMatches->Delete();
      delete fMuchStrawHitMatches;
   }
   if (fMuchTrackMatches) {
      fMuchTrackMatches->Delete();
      delete fMuchTrackMatches;
   }

   if (fMvdClusterMatches != NULL)
      {
      fMvdClusterMatches->Delete();
      delete fMvdClusterMatches;
      }
   if (fMvdHitMatches != NULL) {
      fMvdHitMatches->Delete();
      delete fMvdHitMatches;
   }

   if (fTofHitMatches != NULL) {
      fTofHitMatches->Delete();
      delete fTofHitMatches;
   }
}

void CbmMatchRecoToMC::SetIncludeMvdHitsInStsTrack(Bool_t includeMvdHitsInStsTrack)
{
	fIncludeMvdHitsInStsTrack = includeMvdHitsInStsTrack;
}

InitStatus CbmMatchRecoToMC::Init()
{
   ReadAndCreateDataBranches();

   return kSUCCESS;
}

void CbmMatchRecoToMC::Exec(
    Option_t* /*opt*/)
{
   if (fStsClusterMatches != NULL) fStsClusterMatches->Delete();
   if (fStsHitMatches != NULL) fStsHitMatches->Delete();
   if (fStsTrackMatches != NULL) fStsTrackMatches->Delete();
   if (fRichTrackMatches != NULL) fRichTrackMatches->Delete();
   if (fTrdClusterMatches != NULL) fTrdClusterMatches->Delete();
   if (fTrdHitMatches != NULL) fTrdHitMatches->Delete();
   if (fTrdTrackMatches != NULL) fTrdTrackMatches->Delete();
   if (fMuchClusterMatches != NULL) fMuchClusterMatches->Delete();
   if (fMuchPixelHitMatches != NULL) fMuchPixelHitMatches->Delete();
   if (fMuchStrawHitMatches != NULL) fMuchStrawHitMatches->Delete();
   if (fMuchTrackMatches != NULL) fMuchTrackMatches->Delete();
   if (fMvdHitMatches != NULL) fMvdHitMatches->Delete();
   if (fMvdClusterMatches != NULL) fMvdClusterMatches->Delete();
   if (fTofHitMatches != NULL) fTofHitMatches->Delete();

   //MVD
   if (fMvdDigiMatches && fMvdHits && fMvdHitMatches && !fMvdCluster) {// MC->digi->hit
       MatchHitsMvd(fMvdDigiMatches, fMvdHits, fMvdHitMatches);
   } else if (fMvdDigiMatches && fMvdCluster && fMvdClusterMatches) {
	  MatchClusters(fMvdDigiMatches, fMvdCluster, fMvdClusterMatches);// MC->digi->cluster->hit
	  MatchHits(fMvdClusterMatches, fMvdHits, fMvdHitMatches);
   }

   // STS
   if (fStsDigis && fStsClusters) { // digi->cluster
      if (fStsDigiMatches)
         MatchClusters(fStsDigiMatches, fStsClusters, fStsClusterMatches);
      else
         MatchStsClusters(fStsDigis, fStsClusters, fStsClusterMatches);
      if ( fStsHits ) { // cluster->hit->track
      	MatchHitsSts(fStsClusterMatches, fStsHits, fStsHitMatches);
      	MatchStsTracks(fMvdHitMatches, fStsHitMatches, fMvdPoints, fStsPoints, fStsTracks, fStsTrackMatches);
      } //? hit array
   } //? digi and cluster arrays

   //RICH
   if (fRichDigis && fRichHits && fRichMcPoints && fRichRings && fRichTrackMatches) {
       MatchRichRings(fRichRings, fRichHits, fRichDigis, fRichMcPoints, fMCTracks, fRichTrackMatches);
   }

   // TRD
   if (fTrdDigis && fTrdClusters && fTrdHits) { // MC->digi->cluster->hit->track
      MatchClusters(fTrdDigiMatches, fTrdClusters, fTrdClusterMatches);
      MatchHits(fTrdClusterMatches, fTrdHits, fTrdHitMatches);
      MatchTracks(fTrdHitMatches, fTrdPoints, fTrdTracks, fTrdTrackMatches);
   } else if (fTrdHits) { // MC->hit->track
      MatchHitsToPoints(fTrdPoints, fTrdHits, fTrdHitMatches);
      MatchTracks(fTrdHitMatches, fTrdPoints, fTrdTracks, fTrdTrackMatches);
   }

   // MUCH
   MatchHits(fMuchStrawDigiMatches, fMuchStrawHits, fMuchStrawHitMatches);
   if (fMuchPixelDigis && fMuchClusters && fMuchPixelHits) {
      MatchClusters(fMuchPixelDigiMatches, fMuchClusters, fMuchClusterMatches);
      MatchHits(fMuchClusterMatches, fMuchPixelHits, fMuchPixelHitMatches);
   } else {
      MatchHitsToPoints(fMuchPoints, fMuchPixelHits, fMuchPixelHitMatches);
   }
   MatchTracks(fMuchPixelHitMatches, fMuchPoints, fMuchTracks, fMuchTrackMatches);
   MatchTracks(fMuchStrawHitMatches, fMuchPoints, fMuchTracks, fMuchTrackMatches);

   // TOF: (Digi->MC)+(Hit->Digi)=>(Hit->MC)
   if (kTRUE == fbDigiExpUsed)
      MatchHitsTofDigiExp( fTofDigiMatchesPoints, fTofDigis, fTofDigiMatches, fTofHits, fTofHitMatches);
      else MatchHitsTof( fTofDigiMatchesPoints, fTofDigis, fTofDigiMatches, fTofHits, fTofHitMatches);

   //static Int_t eventNo = 0;
   LOG(INFO) << "CbmMatchRecoToMC::Exec eventNo=" << fEventNumber++ << FairLogger::endl;
}

void CbmMatchRecoToMC::Finish()
{

}

void CbmMatchRecoToMC::ReadAndCreateDataBranches()
{
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) {
      LOG(FATAL) << "CbmMatchRecoToMC::ReadAndCreateDataBranches() NULL FairRootManager." << FairLogger::endl;
   }

   CbmMCDataManager* mcManager = (CbmMCDataManager*)ioman->GetObject("MCDataManager");
   
   if (0 == mcManager)
      LOG(FATAL) << "CbmMatchRecoToMC::ReadAndCreateDataBranches() NULL MCDataManager." << FairLogger::endl;
      
   fMCTracks = mcManager->InitBranch("MCTrack");

    //fMCTracksArray= (TClonesArray*) ioman->GetObject("MCTrack");


   // STS
   fStsPoints=mcManager->InitBranch("StsPoint");
   fStsDigis = (TClonesArray*) ioman->GetObject("StsDigi");
   fStsClusters = (TClonesArray*) ioman->GetObject("StsCluster");
   fStsHits = (TClonesArray*) ioman->GetObject("StsHit");
   fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
   fStsDigiMatches = (TClonesArray*) ioman->GetObject("StsDigiMatch");
   if (fStsClusters != NULL) {
      fStsClusterMatches = new TClonesArray("CbmMatch", 100);
      ioman->Register("StsClusterMatch", "STS", fStsClusterMatches, IsOutputBranchPersistent("StsClusterMatch"));
   }
   if (fStsHits != NULL) {
      fStsHitMatches = new TClonesArray("CbmMatch", 100);
      ioman->Register("StsHitMatch", "STS", fStsHitMatches, IsOutputBranchPersistent("StsHitMatch"));
   }
   if (fStsTracks != NULL) {
      fStsTrackMatches = new TClonesArray("CbmTrackMatchNew", 100);
      ioman->Register("StsTrackMatch", "STS", fStsTrackMatches, IsOutputBranchPersistent("StsTrackMatch"));
   }

   //RICH
   fRichDigis = (TClonesArray*) ioman->GetObject("RichDigi");
   fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
   fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
   fRichMcPoints = mcManager->InitBranch("RichPoint");
   if (fRichRings != NULL) {
      fRichTrackMatches = new TClonesArray("CbmTrackMatchNew", 100);
      ioman->Register("RichRingMatch", "RICH", fRichTrackMatches, IsOutputBranchPersistent("RichRingMatch"));
   }

   // TRD
   fTrdPoints = mcManager->InitBranch("TrdPoint");
   fTrdDigis = (TClonesArray*) ioman->GetObject("TrdDigi");
   fTrdClusters = (TClonesArray*) ioman->GetObject("TrdCluster");
   fTrdHits = (TClonesArray*) ioman->GetObject("TrdHit");
   fTrdTracks = (TClonesArray*) ioman->GetObject("TrdTrack");
   fTrdDigiMatches = (TClonesArray*) ioman->GetObject("TrdDigiMatch");
   if (fTrdClusters != NULL) {
	  fTrdClusterMatches = (TClonesArray*) ioman->GetObject("TrdClusterMatch");
	  if (NULL == fTrdClusterMatches) {
	     fTrdClusterMatches = new TClonesArray("CbmMatch", 100);
	     ioman->Register("TrdClusterMatch", "TRD", fTrdClusterMatches, IsOutputBranchPersistent("TrdClusterMatch"));
	  }
   }
   if (fTrdHits != NULL) {
	  fTrdHitMatches = (TClonesArray*) ioman->GetObject("TrdHitMatch");
	  if (NULL == fTrdHitMatches) {
         fTrdHitMatches = new TClonesArray("CbmMatch", 100);
         ioman->Register("TrdHitMatch", "TRD", fTrdHitMatches, IsOutputBranchPersistent("TrdHitMatch"));
	  }
   }
   if (fTrdTracks != NULL) {
	  fTrdTrackMatches = (TClonesArray*) ioman->GetObject("TrdTrackMatch");
	  if (NULL == fTrdTrackMatches) {
	     fTrdTrackMatches = new TClonesArray("CbmTrackMatchNew", 100);
	     ioman->Register("TrdTrackMatch", "TRD", fTrdTrackMatches, IsOutputBranchPersistent("TrdTrackMatch"));
	  }
   }

   // MUCH
   fMuchPoints = mcManager->InitBranch("MuchPoint");
   fMuchPixelDigis = (TClonesArray*) ioman->GetObject("MuchDigi");
   fMuchStrawDigis = (TClonesArray*) ioman->GetObject("MuchStrawDigi");
   fMuchClusters = (TClonesArray*) ioman->GetObject("MuchCluster");
   fMuchPixelHits = (TClonesArray*) ioman->GetObject("MuchPixelHit");
   fMuchStrawHits = (TClonesArray*) ioman->GetObject("MuchStrawHit");
   fMuchTracks = (TClonesArray*) ioman->GetObject("MuchTrack");
   fMuchPixelDigiMatches = (TClonesArray*) ioman->GetObject("MuchDigiMatch");
   fMuchStrawDigiMatches = (TClonesArray*) ioman->GetObject("MuchStrawDigiMatch");
   if (fMuchClusters != NULL) {
      fMuchClusterMatches = new TClonesArray("CbmMatch", 100);
      ioman->Register("MuchClusterMatch", "MUCH", fMuchClusterMatches, IsOutputBranchPersistent("MuchClusterMatch"));
   }
   if (fMuchPixelHits) {
      fMuchPixelHitMatches = new TClonesArray("CbmMatch", 100);
      ioman->Register("MuchPixelHitMatch", "MUCH", fMuchPixelHitMatches, IsOutputBranchPersistent("MuchPixelHitMatch"));
   }
   if (fMuchStrawHits) {
      fMuchStrawHitMatches = new TClonesArray("CbmMatch", 100); //AZ
      ioman->Register("MuchStrawHitMatch", "MUCH", fMuchStrawHitMatches, IsOutputBranchPersistent("MuchStrawHitMatch")); //AZ
   }
   if (fMuchTracks) {
      fMuchTrackMatches = new TClonesArray("CbmTrackMatchNew", 100);
      ioman->Register("MuchTrackMatch", "MUCH", fMuchTrackMatches, IsOutputBranchPersistent("MuchTrackMatch"));
   }

   // MVD
   fMvdPoints = mcManager->InitBranch("MvdPoint");
   fMvdHits = (TClonesArray*) ioman->GetObject("MvdHit");
   fMvdCluster = (TClonesArray*) ioman->GetObject("MvdCluster");
   fMvdDigiMatches = (TClonesArray*) ioman->GetObject("MvdDigiMatch");
   if (fMvdCluster != NULL) {
      fMvdClusterMatches = new TClonesArray("CbmMatch", 100);
      ioman->Register("MvdClusterMatch", "MVD", fMvdClusterMatches, IsOutputBranchPersistent("MvdClusterMatch"));
   }

   if (fMvdHits != NULL) {
      fMvdHitMatches = new TClonesArray("CbmMatch", 100);
      ioman->Register("MvdHitMatch", "MVD", fMvdHitMatches, IsOutputBranchPersistent("MvdHitMatch"));
   }

   // TOF
   fTofPoints = mcManager->InitBranch("TofPoint");
   fTofDigis  = (TClonesArray*) ioman->GetObject("TofDigi");
   if (NULL != fTofDigis) {
      if ( TString("CbmTofDigiExp") == fTofDigis->GetClass()->GetName() )
         fbDigiExpUsed = kTRUE;
   }
   fTofHits   = (TClonesArray*) ioman->GetObject("TofHit");

   fTofDigiMatchesPoints = (TClonesArray*) ioman->GetObject("TofDigiMatchPoints");
   fTofDigiMatches       = (TClonesArray*) ioman->GetObject("TofDigiMatch");

   if (NULL != fTofDigis && fTofHits != NULL) {
      fTofHitMatches = new TClonesArray("CbmMatch", 100);
      ioman->Register("TofHitMatch", "TOF", fTofHitMatches, IsOutputBranchPersistent("TofHitMatch"));
   }
}


void CbmMatchRecoToMC::MatchClusters(
      const TClonesArray* digiMatches,
      const TClonesArray* clusters,
      TClonesArray* clusterMatches)
{
   if (!(digiMatches && clusters && clusterMatches)) return;
   Int_t nofClusters = clusters->GetEntriesFast();
   for (Int_t iCluster = 0; iCluster < nofClusters; iCluster++) {
      CbmCluster* cluster = static_cast<CbmCluster*>(clusters->At(iCluster));
      CbmMatch* clusterMatch = new ((*clusterMatches)[iCluster]) CbmMatch();
      Int_t nofDigis = cluster->GetNofDigis();
      for (Int_t iDigi = 0; iDigi < nofDigis; iDigi++) {
         const CbmMatch* digiMatch = static_cast<const CbmMatch*>(digiMatches->At(cluster->GetDigi(iDigi)));
         clusterMatch->AddLinks(*digiMatch);
      }  //# digis in cluster
   } //# clusters
}

void CbmMatchRecoToMC::MatchStsClusters(
      const TClonesArray* digi,
      const TClonesArray* clusters,
      TClonesArray* clusterMatches)
{
   if (!(digi && clusters && clusterMatches)) return;
   Int_t nofClusters = clusters->GetEntriesFast();
   for (Int_t iCluster = 0; iCluster < nofClusters; iCluster++) {
      CbmCluster* cluster = static_cast<CbmCluster*>(clusters->At(iCluster));
      CbmMatch* clusterMatch = new ((*clusterMatches)[iCluster]) CbmMatch();
      Int_t nofDigis = cluster->GetNofDigis();
      for (Int_t iDigi = 0; iDigi < nofDigis; iDigi++) {
         const CbmMatch* digiMatch = (static_cast<const CbmDigi*>(digi->At(cluster->GetDigi(iDigi))))->GetMatch();
         clusterMatch->AddLinks(*digiMatch);
      } //# digis in cluster
   }
}

void CbmMatchRecoToMC::MatchHits(
      const TClonesArray* matches,
      const TClonesArray* hits,
      TClonesArray* hitMatches)
{
   if (!(matches && hits && hitMatches)) return;
   Int_t nofHits = hits->GetEntriesFast();
   for (Int_t iHit = 0; iHit < nofHits; iHit++) {
      CbmHit* hit = static_cast<CbmHit*>(hits->At(iHit));
      CbmMatch* hitMatch = new ((*hitMatches)[iHit]) CbmMatch();
      const CbmMatch* clusterMatch = static_cast<const CbmMatch*>(matches->At(hit->GetRefId()));
      hitMatch->AddLinks(*clusterMatch);
   }
}

void CbmMatchRecoToMC::MatchHitsSts(
      const TClonesArray* cluMatches,
      const TClonesArray* hits,
      TClonesArray* hitMatches)
{
   if (!(cluMatches && hits && hitMatches)) return;
   Int_t nofHits = hits->GetEntriesFast();
   for (Int_t iHit = 0; iHit < nofHits; iHit++) {
      CbmStsHit* hit = static_cast<CbmStsHit*>(hits->At(iHit));
      CbmMatch* hitMatch = new ((*hitMatches)[iHit]) CbmMatch();
      const CbmMatch* frontClusterMatch = static_cast<const CbmMatch*>(cluMatches->At(hit->GetFrontClusterId()));
      const CbmMatch* backClusterMatch = static_cast<const CbmMatch*>(cluMatches->At(hit->GetBackClusterId()));
      hitMatch->AddLinks(*frontClusterMatch);
      hitMatch->AddLinks(*backClusterMatch);
   }
}

void CbmMatchRecoToMC::MatchHitsMvd(
      const TClonesArray* matches,
      const TClonesArray* hits,
      TClonesArray* hitMatches)
{
  if (!(matches && hits && hitMatches)) return;
  Int_t nofHits = hits->GetEntriesFast();
  for (Int_t iHit = 0; iHit < nofHits; iHit++) {
    CbmPixelHit* hit = static_cast<CbmPixelHit*>(hits->At(iHit));
    CbmMatch* hitMatch = new ((*hitMatches)[iHit]) CbmMatch();
    const CbmMatch* digiMatch = static_cast<const CbmMatch*>(matches->At(hit->GetRefId()));
    hitMatch->AddLinks(*digiMatch);
  }
}

void CbmMatchRecoToMC::MatchHitsTof(
      const TClonesArray* DigiPntMatches,
      const TClonesArray* digis,
      const TClonesArray* HitDigiMatches,
      const TClonesArray* hits,
      TClonesArray* hitMatches)
{
   if (!(DigiPntMatches && digis && HitDigiMatches && hits && hitMatches)) return;

   Int_t iNbTofDigis = digis->GetEntriesFast();
   Int_t nofHits = hits->GetEntriesFast();
   CbmTofDigi * pTofDigi;
   CbmMatch   * pMatchDigiPnt;

   for (Int_t iHit = 0; iHit < nofHits; iHit++) {
      CbmTofHit* hit         = static_cast<CbmTofHit*>(hits->At(iHit));
      CbmMatch* hitDigiMatch = static_cast<CbmMatch*>( HitDigiMatches->At(iHit) );
      CbmMatch* hitMatch = new ((*hitMatches)[iHit]) CbmMatch();

      Int_t iNbDigisHit = hitDigiMatch->GetNofLinks();
      for (Int_t iDigi = 0; iDigi < iNbDigisHit; iDigi++) {
         CbmLink lDigi    = hitDigiMatch->GetLink(iDigi);
         Int_t   iDigiIdx = lDigi.GetIndex();

         if( iNbTofDigis <= iDigiIdx )
         {
            LOG(ERROR)<<"CbmTofHitFinderQa::FillHistos => Digi index from Hit #"
                <<iHit
                <<" is bigger than nb entries in Digis arrays => ignore it!!!"<<FairLogger::endl;
            continue;
         } // if( iNbTofDigis <= iDigiIdx )

         pTofDigi      = static_cast<CbmTofDigi*> (digis->At( iDigiIdx ));
         pMatchDigiPnt = static_cast<CbmMatch*>   (DigiPntMatches->At( iDigiIdx ));

         Int_t iNbPointsDigi = pMatchDigiPnt->GetNofLinks();
         CbmLink lTruePoint    = pMatchDigiPnt->GetMatchedLink(); // Point generating the Digi
         Int_t   iTruePointIdx = lTruePoint.GetIndex();
         for( Int_t iPoint = 0; iPoint < iNbPointsDigi; iPoint ++)
         {
            CbmLink lPoint    = pMatchDigiPnt->GetLink(iPoint);
            Int_t   iPointIdx = lPoint.GetIndex();

            if( iPointIdx == iTruePointIdx )
               hitMatch->AddLink(CbmLink(pTofDigi->GetTot(), iPointIdx, 
                                         lPoint.GetEntry(), lPoint.GetFile())); // Point generating the Digi
               else hitMatch->AddLink(CbmLink(0, iPointIdx, lPoint.GetEntry(),
                                              lPoint.GetFile())); // Point whose Digi was hidden by True one
         } // for( Int_t iPoint = 0; iPoint < iNbPointsDigi; iPoint ++)
      } // for (Int_t iDigi = 0; iDigi < iNbDigisHit; iDigi++)
   } // for (Int_t iHit = 0; iHit < nofHits; iHit++)
}
void CbmMatchRecoToMC::MatchHitsTofDigiExp(
      const TClonesArray* DigiPntMatches,
      const TClonesArray* digis,
      const TClonesArray* HitDigiMatches,
      const TClonesArray* hits,
      TClonesArray* hitMatches)
{
   if (!(DigiPntMatches && digis && HitDigiMatches && hits && hitMatches)) return;

   Int_t iNbTofDigis = digis->GetEntriesFast();
   Int_t nofHits = hits->GetEntriesFast();
   CbmTofDigiExp * pTofDigi;
   CbmMatch   * pMatchDigiPnt;

   for (Int_t iHit = 0; iHit < nofHits; iHit++) {
      CbmTofHit* hit         = static_cast<CbmTofHit*>(hits->At(iHit));
      CbmMatch* hitDigiMatch = static_cast<CbmMatch*>( HitDigiMatches->At(iHit) );
      CbmMatch* hitMatch = new ((*hitMatches)[iHit]) CbmMatch();

      Int_t iNbDigisHit = hitDigiMatch->GetNofLinks();
      for (Int_t iDigi = 0; iDigi < iNbDigisHit; iDigi++) {
         CbmLink lDigi    = hitDigiMatch->GetLink(iDigi);
         Int_t   iDigiIdx = lDigi.GetIndex();

         if( iNbTofDigis <= iDigiIdx )
         {
            LOG(ERROR)<<"CbmTofHitFinderQa::FillHistos => Digi index from Hit #"
                <<iHit
                <<" is bigger than nb entries in Digis arrays => ignore it!!!"<<FairLogger::endl;
            continue;
         } // if( iNbTofDigis <= iDigiIdx )

         pTofDigi      = static_cast<CbmTofDigiExp*> (digis->At( iDigiIdx ));
         pMatchDigiPnt = static_cast<CbmMatch*>      (DigiPntMatches->At( iDigiIdx ));

         Int_t iNbPointsDigi = pMatchDigiPnt->GetNofLinks();
         CbmLink lTruePoint    = pMatchDigiPnt->GetMatchedLink(); // Point generating the Digi
         Int_t   iTruePointIdx = lTruePoint.GetIndex();
         for( Int_t iPoint = 0; iPoint < iNbPointsDigi; iPoint ++)
         {
            CbmLink lPoint    = pMatchDigiPnt->GetLink(iPoint);
            Int_t   iPointIdx = lPoint.GetIndex();

            if( iPointIdx == iTruePointIdx )
               hitMatch->AddLink(CbmLink(pTofDigi->GetTot(), iPointIdx, 
                                         lPoint.GetEntry(), lPoint.GetFile() ) ); // Point generating the Digi
               else hitMatch->AddLink(CbmLink(0, iPointIdx, lPoint.GetEntry(), 
                                              lPoint.GetFile())); // Point whose Digi was hidden by True one
         } // for( Int_t iPoint = 0; iPoint < iNbPointsDigi; iPoint ++)
      } // for (Int_t iDigi = 0; iDigi < iNbDigisHit; iDigi++)
    } // for (Int_t iHit = 0; iHit < nofHits; iHit++)
}

void CbmMatchRecoToMC::MatchHitsToPoints(
      CbmMCDataArray* points,
      const TClonesArray* hits,
      TClonesArray* hitMatches)
{
   if (!(hits && hitMatches)) return;
   Int_t nofHits = hits->GetEntriesFast();
   for (Int_t iHit = 0; iHit < nofHits; iHit++) {
      CbmHit* hit = static_cast<CbmHit*>(hits->At(iHit));
      CbmMatch* hitMatch = new ((*hitMatches)[iHit]) CbmMatch();
      const FairMCPoint* point = static_cast<const FairMCPoint*>(points->Get(0, fEventNumber, hit->GetRefId()));
      hitMatch->AddLink(CbmLink(point->GetEnergyLoss(), hit->GetRefId(), fEventNumber));
   }
}

void CbmMatchRecoToMC::MatchTracks(
      const TClonesArray* hitMatches,
      CbmMCDataArray* points,
      const TClonesArray* tracks,
      TClonesArray* trackMatches)
{
   if (!(hitMatches && points && tracks && trackMatches)) return;

   Bool_t editMode = (trackMatches->GetEntriesFast() != 0);

   Int_t nofTracks = tracks->GetEntriesFast();
   for (Int_t iTrack = 0; iTrack < nofTracks; iTrack++) {
      const CbmTrack* track = static_cast<const CbmTrack*>(tracks->At(iTrack));
      CbmTrackMatchNew* trackMatch = (editMode) ?
            static_cast<CbmTrackMatchNew*>(trackMatches->At(iTrack)) :
               new ((*trackMatches)[iTrack]) CbmTrackMatchNew();
      Int_t nofHits = track->GetNofHits();
      for (Int_t iHit = 0; iHit < nofHits; iHit++) {
	if ((track->GetHitType(iHit) == kMUCHPIXELHIT && hitMatches == fMuchStrawHitMatches) || (track->GetHitType(iHit) == kMUCHSTRAWHIT && hitMatches == fMuchPixelHitMatches)) continue; //AZ
         const CbmMatch* hitMatch = static_cast<CbmMatch*>(hitMatches->At(track->GetHitIndex(iHit)));
         Int_t nofLinks = hitMatch->GetNofLinks();
         for (Int_t iLink = 0; iLink < nofLinks; iLink++) {
            const FairMCPoint* point = static_cast<const FairMCPoint*>(points->Get(hitMatch->GetLink(iLink)));
            if (NULL == point) continue;
			////fix low energy cut case on STS
			if (CbmStsAddress::GetSystemId(point->GetDetectorID()) == kSTS ){
				Int_t mcTrackId = point->GetTrackID();
				CbmMCTrack *mcTrack = (CbmMCTrack*) fMCTracks->Get(hitMatch->GetLink(iLink).GetFile(), hitMatch->GetLink(iLink).GetEntry(), mcTrackId);
				if(mcTrack->GetNPoints(kSTS) < 2)
					continue;
			}
			////
            trackMatch->AddLink(CbmLink(1., point->GetTrackID(), hitMatch->GetLink(iLink).GetEntry()));
         }
      }
      if ( ! trackMatch->GetNofLinks() ) continue;
      // Calculate number of true and wrong hits
      Int_t trueCounter = trackMatch->GetNofTrueHits();
      Int_t wrongCounter = trackMatch->GetNofWrongHits();
      for (Int_t iHit = 0; iHit < nofHits; iHit++) {
	 if ((track->GetHitType(iHit) == kMUCHPIXELHIT && hitMatches == fMuchStrawHitMatches) || (track->GetHitType(iHit) == kMUCHSTRAWHIT && hitMatches == fMuchPixelHitMatches)) continue; //AZ
         const CbmMatch* hitMatch = static_cast<CbmMatch*>(hitMatches->At(track->GetHitIndex(iHit)));
         Int_t nofLinks = hitMatch->GetNofLinks();
         Bool_t hasTrue = false;
         for (Int_t iLink = 0; iLink < nofLinks; iLink++) {
            const FairMCPoint* point = static_cast<const FairMCPoint*>(points->Get(hitMatch->GetLink(iLink)));
            if (NULL == point) continue;
            if (/*point->GetEventID() == trackMatch->GetMatchedLink().GetEntry() && */point->GetTrackID() == trackMatch->GetMatchedLink().GetIndex()) {
               hasTrue = true;
               break;
            }
         }
         if (hasTrue) trueCounter++; else wrongCounter++;
      }
      trackMatch->SetNofTrueHits(trueCounter);
      trackMatch->SetNofWrongHits(wrongCounter);
     // LOG(DEBUG) << iTrack << " "; track->Print(); LOG(DEBUG) << " " << trackMatch->ToString()
     // << FairLogger::endl;
   }
}

void CbmMatchRecoToMC::MatchStsTracks(
	  const TClonesArray* mvdHitMatches,
      const TClonesArray* stsHitMatches,
	   CbmMCDataArray* mvdPoints,
      CbmMCDataArray* stsPoints,
      const TClonesArray* tracks,
      TClonesArray* trackMatches)
{
   if (!((stsHitMatches && stsPoints && tracks && trackMatches)
		   && ((fIncludeMvdHitsInStsTrack) ? mvdHitMatches && mvdPoints : true))) return;

   Bool_t editMode = (trackMatches->GetEntriesFast() != 0);

   Int_t nofTracks = tracks->GetEntriesFast();
   for (Int_t iTrack = 0; iTrack < nofTracks; iTrack++) {
      const CbmStsTrack* track = static_cast<const CbmStsTrack*>(tracks->At(iTrack));
      CbmTrackMatchNew* trackMatch = (editMode) ?
            static_cast<CbmTrackMatchNew*>(trackMatches->At(iTrack)) :
               new ((*trackMatches)[iTrack]) CbmTrackMatchNew();
      Int_t nofStsHits = track->GetNofStsHits();
      for (Int_t iHit = 0; iHit < nofStsHits; iHit++) {
         const CbmMatch* hitMatch = static_cast<CbmMatch*>(stsHitMatches->At(track->GetHitIndex(iHit)));
         Int_t nofLinks = hitMatch->GetNofLinks();
         for (Int_t iLink = 0; iLink < nofLinks; iLink++) {
	        const CbmLink& link = hitMatch->GetLink(iLink);
            const FairMCPoint* point = static_cast<const FairMCPoint*>(stsPoints->Get(link));
            if (NULL == point) continue;
			////fix low energy cut case on STS
			if (CbmStsAddress::GetSystemId(point->GetDetectorID()) == kSTS ){
				Int_t mcTrackId = point->GetTrackID();
				CbmMCTrack *mcTrack = (CbmMCTrack*) fMCTracks->Get(link.GetFile(), link.GetEntry(), mcTrackId);
				if(mcTrack->GetNPoints(kSTS) < 2)
					continue;
			}
			////
            trackMatch->AddLink(CbmLink(1., point->GetTrackID(), link.GetEntry(), link.GetFile()));
         }
      }

      // Include MVD hits in STS matching if needed.
      if (fIncludeMvdHitsInStsTrack) {
         Int_t nofMvdHits = track->GetNofMvdHits();
         for (Int_t iHit = 0; iHit < nofMvdHits; iHit++) {
            const CbmMatch* hitMatch = static_cast<CbmMatch*>(mvdHitMatches->At(track->GetMvdHitIndex(iHit)));
            Int_t nofLinks = hitMatch->GetNofLinks();
            for (Int_t iLink = 0; iLink < nofLinks; iLink++) {
        	   const CbmLink& link = hitMatch->GetLink(iLink);
	           const FairMCPoint* point = static_cast<const FairMCPoint*>(mvdPoints->Get(link));
               if (NULL == point) continue;
               trackMatch->AddLink(CbmLink(1., point->GetTrackID(), link.GetEntry(), link.GetFile()));
            }
         }
      }

      if ( ! trackMatch->GetNofLinks() ) continue;
      // Calculate number of true and wrong hits
      Int_t trueCounter = trackMatch->GetNofTrueHits();
      Int_t wrongCounter = trackMatch->GetNofWrongHits();
      for (Int_t iHit = 0; iHit < nofStsHits; iHit++) {
         const CbmMatch* hitMatch = static_cast<CbmMatch*>(stsHitMatches->At(track->GetHitIndex(iHit)));
         Int_t nofLinks = hitMatch->GetNofLinks();
         Bool_t hasTrue = false;
         for (Int_t iLink = 0; iLink < nofLinks; iLink++) {
            const FairMCPoint* point = static_cast<const FairMCPoint*>(stsPoints->Get(hitMatch->GetLink(iLink)));
            if (NULL == point) continue;
            if (point->GetTrackID() == trackMatch->GetMatchedLink().GetIndex()) {
               hasTrue = true;
               break;
            }
         }
         if (hasTrue) trueCounter++; else wrongCounter++;
      }

      // Include MVD hits in STS track matching if needed
      if (fIncludeMvdHitsInStsTrack) {
    	 Int_t nofMvdHits = track->GetNofMvdHits();
         for (Int_t iHit = 0; iHit < nofMvdHits; iHit++) {
            const CbmMatch* hitMatch = static_cast<CbmMatch*>(mvdHitMatches->At(track->GetMvdHitIndex(iHit)));
            Int_t nofLinks = hitMatch->GetNofLinks();
            Bool_t hasTrue = false;
            for (Int_t iLink = 0; iLink < nofLinks; iLink++) {
               const FairMCPoint* point = static_cast<const FairMCPoint*>(mvdPoints->Get(hitMatch->GetLink(iLink)));
               if (NULL == point) continue;
               if (/*point->GetEventID() == trackMatch->GetMatchedLink().GetEntry() && */point->GetTrackID() == trackMatch->GetMatchedLink().GetIndex()) {
                  hasTrue = true;
                  break;
               }
            }
            if (hasTrue) trueCounter++; else wrongCounter++;
         }
      }
      trackMatch->SetNofTrueHits(trueCounter);
      trackMatch->SetNofWrongHits(wrongCounter);
     // LOG(DEBUG) << iTrack << " "; track->Print(); LOG(DEBUG) << " " << trackMatch->ToString()
     // << FairLogger::endl;
   }
}


void CbmMatchRecoToMC::MatchRichRings(
    const TClonesArray* richRings,
    const TClonesArray* richHits,
    const TClonesArray* richDigis,
    CbmMCDataArray* richMcPoints,
    CbmMCDataArray* mcTracks,
    TClonesArray* ringMatches)
{
    // Loop over RichRings
    Int_t nRings = richRings->GetEntriesFast();
    for (Int_t iRing = 0; iRing < nRings; iRing++) {
        const CbmRichRing* ring = static_cast<const CbmRichRing*>(richRings->At(iRing));
        if (NULL == ring) continue;

        CbmTrackMatchNew* ringMatch = new ((*ringMatches)[iRing]) CbmTrackMatchNew();

        Int_t nofHits = ring->GetNofHits();
        for (Int_t iHit=0; iHit < nofHits; iHit++) {
            const CbmRichHit* hit = static_cast<const CbmRichHit*>(richHits->At(ring->GetHit(iHit)));
            if ( NULL == hit ) continue;

            vector<pair<Int_t, Int_t> > motherIds = GetMcTrackMotherIdsForRichHit(hit, richDigis, richMcPoints, mcTracks);
            for (UInt_t i = 0; i < motherIds.size(); i++) {
                ringMatch->AddLink(1., motherIds[i].second, motherIds[i].second);
            }
        }
        
        if (ringMatch->GetNofLinks() != 0) {

            Int_t bestTrackId = ringMatch->GetMatchedLink().GetIndex();
            Int_t bestTrackEventId = ringMatch->GetMatchedLink().GetEntry();

            Int_t trueCounter = 0;
            Int_t wrongCounter = 0;
            for (Int_t iHit = 0; iHit < nofHits; iHit++) {
                const CbmRichHit* hit = static_cast<const CbmRichHit*>(richHits->At(ring->GetHit(iHit)));
                if ( NULL == hit ) continue;
                vector<pair<Int_t, Int_t> > motherIds = GetMcTrackMotherIdsForRichHit(hit, richDigis, richMcPoints, mcTracks);
                if(std::find(motherIds.begin(), motherIds.end(), std::make_pair(bestTrackEventId, bestTrackId)) != motherIds.end()) {
                    trueCounter++;
                } else {
                    wrongCounter++;
                }

            }

            ringMatch->SetNofTrueHits(trueCounter);
            ringMatch->SetNofWrongHits(wrongCounter);
        } else {
            ringMatch->SetNofTrueHits(0);
            ringMatch->SetNofWrongHits(0);
        }

    }// Ring loop
}


vector<pair<Int_t, Int_t> > CbmMatchRecoToMC::GetMcTrackMotherIdsForRichHit(
        const CbmRichHit* hit,
        const TClonesArray* richDigis,
        CbmMCDataArray* richPoints,
        CbmMCDataArray* mcTracks)
{
    vector<pair<Int_t, Int_t> > result;
    if ( NULL == hit ) return result;
    Int_t digiIndex = hit->GetRefId();
    if (digiIndex < 0) return result;
    const CbmRichDigi* digi = static_cast<const CbmRichDigi*>(richDigis->At(digiIndex));
    if (NULL == digi) return result;
    CbmMatch* digiMatch = digi->GetMatch();

    vector<CbmLink> links = digiMatch->GetLinks();
    for (UInt_t i = 0; i < links.size(); i++) {
        Int_t pointId = links[i].GetIndex();
        if (pointId < 0) continue; // noise hit

        const CbmRichPoint* pMCpt = static_cast<const CbmRichPoint*>(richPoints->Get(0, fEventNumber, pointId));
        if ( NULL == pMCpt ) continue;
        Int_t mcTrackIndex = pMCpt->GetTrackID();
        if ( mcTrackIndex < 0 ) continue;
        //TODO: Currently we support only legacy mode of CbmMCDataArray
        const CbmMCTrack *mcTrack = static_cast<const CbmMCTrack*>(mcTracks->Get(0, fEventNumber, mcTrackIndex));
       // CbmMCTrack* pMCtr = (CbmMCTrack*) mcTracks->At(mcTrackIndex);
        if ( NULL == mcTrack ) continue;
        if ( mcTrack->GetPdgCode() != 50000050) continue; // select only Cherenkov photons
        Int_t motherId = mcTrack->GetMotherId();
        // several photons can have same mother track
        // count only unique motherIds
        pair<Int_t, Int_t> val = std::make_pair(fEventNumber, motherId);
        if(std::find(result.begin(), result.end(), val) == result.end()) {
            result.push_back(val);
        }
    }

    return result;
}

vector<Int_t> CbmMatchRecoToMC::GetMcTrackMotherIdsForRichHit(
        const CbmRichHit* hit,
        const TClonesArray* richDigis,
        const TClonesArray* richPoints,
        const TClonesArray* mcTracks)
{
    vector<Int_t> result;
    if ( NULL == hit ) return result;
    Int_t digiIndex = hit->GetRefId();
    if (digiIndex < 0) return result;
    const CbmRichDigi* digi = static_cast<const CbmRichDigi*>(richDigis->At(digiIndex));
    if (NULL == digi) return result;
    CbmMatch* digiMatch = digi->GetMatch();

    vector<CbmLink> links = digiMatch->GetLinks();
    for (UInt_t i = 0; i < links.size(); i++) {
        Int_t pointId = links[i].GetIndex();
        if (pointId < 0) continue; // noise hit

        const CbmRichPoint* pMCpt = static_cast<const CbmRichPoint*>(richPoints->At(pointId));
        if ( NULL == pMCpt ) continue;
        Int_t mcTrackIndex = pMCpt->GetTrackID();
        if ( mcTrackIndex < 0 ) continue;
        //TODO: Currently we support only legacy mode of CbmMCDataArray
        const CbmMCTrack *mcTrack = static_cast<const CbmMCTrack*>(mcTracks->At(mcTrackIndex));
       // CbmMCTrack* pMCtr = (CbmMCTrack*) mcTracks->At(mcTrackIndex);
        if ( NULL == mcTrack ) continue;
        if ( mcTrack->GetPdgCode() != 50000050) continue; // select only Cherenkov photons
        Int_t motherId = mcTrack->GetMotherId();
        // several photons can have same mother track
        // count only unique motherIds
        if(std::find(result.begin(), result.end(), motherId) == result.end()) {
            result.push_back(motherId);
        }
    }

    return result;
}

ClassImp(CbmMatchRecoToMC);

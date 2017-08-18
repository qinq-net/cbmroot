


// --- Includes from CBM
#include "CbmD0TrackSelection.h"
#include "CbmD0TrackCandidate.h"
#include "CbmGlobalTrack.h"
#include "CbmTofHit.h"
#include "CbmTrdTrack.h"
#include "CbmRichRing.h"
#include "CbmStsTrack.h"
#include "CbmVertex.h"
#include "CbmL1PFFitter.h"
#include "CbmMCTrack.h"
#include "CbmMvdPoint.h"
#include "CbmKF.h"
#include "CbmTrackMatchNew.h"
#include "CbmKFParticleInterface.h"


// --- Includes from ROOT
#include "TClonesArray.h"
#include "TVector3.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TNtuple.h"

// --- Includes from KF
#include "KFParticle.h"
#include "KFPVertex.h"

// --- Includes from L1
#include "L1Field.h"

// Includes from C++
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

//-- Include from Fair --//
#include "FairLogger.h"
#include "FairRootManager.h"


using namespace std;

// -------------------------------------------------------------------------
CbmD0TrackSelection::CbmD0TrackSelection()
 : FairTask(),
   fNHitsOfLongTracks(1),
   fEventNumber(0),
   fMcPoints(NULL),
   fStsTrackArray(NULL),
   fGlobalTracks(NULL),
   fTrdTracks(NULL),
   fTofHits(NULL),
   fRichRings(NULL),
   fStsTrackArrayP(NULL),
   fStsTrackArrayN(NULL),
   fMCTrackArrayP(NULL),
   fMCTrackArrayN(NULL),
   fStsTrackMatches(NULL),
   fKaonParticleArray(NULL),
   fPionParticleArray(NULL),
   fListMCTracks(NULL),
   fMvdHitMatchArray(NULL),
   fKFParticleArray(NULL),
   fD0KaonTrackArray(NULL),
   fD0PionTrackArray(NULL),
   fvtx(),
   kfpInterface(NULL),
   fPidMode(""),
   fFit(NULL),
   fPrimVtx(NULL),
   fField(NULL),
   bUseMCInfo(kFALSE),
   fPVCutPassed(0),
   fPVCutNotPassed(0),
   fNoHPassed(0),
   fNoHNotPassed(0),
   fCutPt(0.),
   fCutP(0.),
   fCutChi2(0.),
   fCutIP(0.)
{
  Fatal( "CbmD0TrackSelection: Do not use the standard constructor","Wrong constructor");
    
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0TrackSelection::CbmD0TrackSelection(char* name, Int_t iVerbose, Double_t cutP, Double_t cutPt, Double_t cutChi2, Double_t cutIP)
  : FairTask(name, iVerbose),
    fNHitsOfLongTracks(1),
    fEventNumber(0),
    fMcPoints(NULL),
    fStsTrackArray(NULL),
    fGlobalTracks(NULL),
    fTrdTracks(NULL),
    fTofHits(NULL),
    fRichRings(NULL),
    fStsTrackArrayP(NULL),
    fStsTrackArrayN(NULL),
    fMCTrackArrayP(NULL),
    fMCTrackArrayN(NULL),
    fStsTrackMatches(NULL),
    fKaonParticleArray(NULL),
    fPionParticleArray(NULL),
    fListMCTracks(NULL),
    fMvdHitMatchArray(NULL),
    fKFParticleArray(NULL),
    fD0KaonTrackArray(NULL),
    fD0PionTrackArray(NULL),
    fvtx(),
    kfpInterface(NULL),
    fPidMode(""),
    fFit(NULL),
    fPrimVtx(NULL),
    fField(NULL),
    bUseMCInfo(kFALSE),
    fPVCutPassed(0),
    fPVCutNotPassed(0),
    fNoHPassed(0),
    fNoHNotPassed(0),
    fCutPt(cutPt),
    fCutP(cutP),
    fCutChi2(cutChi2),
    fCutIP(cutIP)
{
  /*
fEventNumber = 0;
fPVCutPassed = 0;
fPVCutNotPassed = 0;
fCutP  = cutP;
fCutPt = cutPt;
fCutChi2 = cutChi2;
fCutIP = cutIP;
fNHitsOfLongTracks=1;
fNoHPassed=0;
fNoHNotPassed=0;
  */
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0TrackSelection::~CbmD0TrackSelection() {

    delete fMCTrackArrayP;
    delete fMCTrackArrayN;
    delete fKaonParticleArray;
    delete fPionParticleArray;


}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0TrackSelection::SetPIDMode(TString pidMode) {

    if(pidMode == "MC" || pidMode == "TOF" || pidMode == "NONE" || pidMode == "GLOBAL")
      fPidMode = pidMode;
    else
      {
      LOG(WARNING) << "not supported PID mode, PID modes are: MC, TOF, NONE, GLOBAL. PID mode set to NONE" << FairLogger::endl;
      fPidMode = "NONE";
      }
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
InitStatus CbmD0TrackSelection::Init() {

    FairRootManager* ioman = FairRootManager::Instance();

    fMCTrackArrayP         = new TClonesArray("CbmMCTrack",100);
    fMCTrackArrayN         = new TClonesArray("CbmMCTrack",100);
    fKaonParticleArray     = new TClonesArray("KFParticle",100);
    fPionParticleArray     = new TClonesArray("KFParticle",100);
    fD0PionTrackArray      = new TClonesArray("CbmD0TrackCandidate", 100);
    fD0KaonTrackArray      = new TClonesArray("CbmD0TrackCandidate", 100);

    ioman->Register("PositiveMCTracks",  "Open Charm Mc Positiv", fMCTrackArrayP,  IsOutputBranchPersistent("PositiveMCTracks"));
    ioman->Register("NegativeMCTracks",  "Open Charm Mc Negativ", fMCTrackArrayN,  IsOutputBranchPersistent("NegativeMCTracks"));
    ioman->Register("CbmD0KaonParticles", "Open Charm Kaon Particles", fKaonParticleArray, IsOutputBranchPersistent("CbmD0KaonParticles"));
    ioman->Register("CbmD0PionParticles", "Open Charm Pion Particles", fPionParticleArray, IsOutputBranchPersistent("CbmD0PionParticles"));
    ioman->Register("CbmD0PionTrackArray", "Open Charm Pion Tracks", fD0PionTrackArray , IsOutputBranchPersistent("CbmD0PionTrackArray"));
    ioman->Register("CbmD0KaonTrackArray", "Open Charm Kaon Tracks", fD0KaonTrackArray , IsOutputBranchPersistent("CbmD0KaonTrackArray"));

    fMcPoints        = (TClonesArray*) ioman->GetObject("MvdPoint");
    fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
    fStsTrackArray   = (TClonesArray*) ioman->GetObject("StsTrack");
    fMvdHitMatchArray= (TClonesArray*) ioman->GetObject("MvdHitMatch");
    fGlobalTracks    = (TClonesArray*) ioman->GetObject("GlobalTrack");
    fTrdTracks       = (TClonesArray*) ioman->GetObject("TrdTrack");
    fRichRings       = (TClonesArray*) ioman->GetObject("RichRing");
    fTofHits         = (TClonesArray*) ioman->GetObject("TofHit");
    fListMCTracks    = (TClonesArray*) ioman->GetObject("MCTrack");
//    fPrimVtx         = (CbmVertex*) ioman->GetObject("PrimaryVertex");

    // Get pointer to PrimaryVertex object from IOManager if it exists
    // The old name for the object is "PrimaryVertex" the new one
    // "PrimaryVertex." Check first for the new name
    fPrimVtx = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex."));
    if (nullptr == fPrimVtx) {
      fPrimVtx = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex"));
    }

    fvtx[0] = fPrimVtx->GetX();
    fvtx[1] = fPrimVtx->GetY();
    fvtx[2] = fPrimVtx->GetZ();

    if(! fStsTrackArray) {Fatal("CbmD0TrackSelection: StsTrackArray not found (!)"," That's bad. ");}
    if(! fMvdHitMatchArray) {Fatal("CbmD0TrackSelection: MVDHitMatchArray not found","Good bye");}

    //    CbmKF* kalman = CbmKF::Instance();
    fFit          = new CbmL1PFFitter();
    kfpInterface  = new CbmKFParticleInterface();

    return kSUCCESS;
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmD0TrackSelection::ClearArrays()
{
fMCTrackArrayP->Clear();
fMCTrackArrayN->Clear();
fKaonParticleArray->Clear();
fPionParticleArray->Clear();
fD0KaonTrackArray->Clear();
fD0PionTrackArray->Clear();

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0TrackSelection::Exec(Option_t* /*option*/)
{
CbmGlobalTrack*      globalTrack;
CbmStsTrack*         stsTrack;
//CbmTrackMatchNew*    mcTrackMatch;
CbmMCTrack*          mcTrack;
TVector3             vertex;
//Int_t NAcceptedTrackP = 0;
//Int_t NAcceptedTrackN = 0;
//Int_t count=0;
Int_t pidHypo=211;
//Int_t mcTrackIndex = -1;
Int_t i_MCtracksP = -1;
Int_t i_MCtracksN = -1;

Float_t   ip;   
Float_t   ipx;   
Float_t   ipy;   
Float_t   pt;    
Float_t   p;     
Float_t   Chi2 = 0;

ClearArrays();

fEventNumber++;
cout << endl;
LOG(INFO) << "||--------------------------  Event: " << fEventNumber << " --------------------------||" << FairLogger::endl;

Int_t nTracks = fGlobalTracks->GetEntriesFast();

LOG(INFO) <<"CbmD0TrackSelection: Entries: " << nTracks << FairLogger::endl;

if(nTracks==0)
  {
   LOG(INFO) << "CbmD0TrackSelection:: No CbmStsTracks found, ignoring this event." << FairLogger::endl;
   return;
  }

// --- Loop over reconstructed tracks ---
for ( Int_t itr=0; itr<nTracks; itr++ )
{
globalTrack = (CbmGlobalTrack*) fGlobalTracks->At(itr);

if(globalTrack->GetStsTrackIndex()!= -1)
        stsTrack    = (CbmStsTrack*) fStsTrackArray->At(globalTrack->GetStsTrackIndex()); else continue;

pidHypo = GetPid(globalTrack);

if(pidHypo != -321 && pidHypo != 211) continue;

FairTrackParam *param = new FairTrackParam();
kfpInterface->ExtrapolateTrackToPV(stsTrack, fPrimVtx, param, Chi2);
        if ( Chi2 < fCutChi2 ) continue;

	KFParticle newParticle;
	kfpInterface->SetKFParticleFromStsTrack(&*stsTrack,&newParticle, pidHypo);
        pt        = newParticle.GetPt();
        p         = newParticle.GetP();

	newParticle.TransportToPoint(&*fvtx);

   ip        = GetImpactParameterRadius( &newParticle);
   ipx       = GetImpactParameterX( &newParticle );
   ipy       = GetImpactParameterY( &newParticle );

const FairTrackParam *e_track = stsTrack->GetParamFirst();
Double_t       	      qp   = e_track->GetQp();

	// --- single track pre-selection ---

	if ( p  < fCutP       ) continue;
        if ( pt < fCutPt      ) continue;
        if ( ip > fCutIP      ) continue;
	/////////////////////////////////////////////////////

    if(bUseMCInfo)
    {
        LOG(INFO) << "found possible opencharm track candidate, use MC-Data to start QA" << FairLogger::endl;
      	mcTrack = GetMCTrackFromTrackID(itr);// (CbmMCTrack*) fListMCTracks->At(mcTrackIndex);

	if(mcTrack)// --- Save only positive MCtracks ---
	{
	    if (qp>0)
	    {
	    TClonesArray& MCTrackArrayP = *fMCTrackArrayP;
	    i_MCtracksP = MCTrackArrayP.GetEntriesFast();
	    new( MCTrackArrayP[i_MCtracksP] ) CbmMCTrack(*mcTrack);
	    }

            // --- Save only negative MCtracks ---
            if (qp<0)
	    {
	    i_MCtracksN = fMCTrackArrayN->GetEntriesFast();
	    new((*fMCTrackArrayN)[i_MCtracksN]) CbmMCTrack(*mcTrack);
	    }
	}
        else ;


    } 	
	if(qp < 0)
	{
	    // --- Save Kaon to  ParicleArray from Track ---
	    TClonesArray& clrefKaon = *fKaonParticleArray;
	    Int_t sizeKaon = clrefKaon.GetEntriesFast();
	    new (clrefKaon[sizeKaon]) KFParticle(newParticle);

	    // --- Save Kaon to  TrackArray for QA       ---
	    TClonesArray& clrefKaonTr = *fD0KaonTrackArray;
	    Int_t sizeKaonTr = clrefKaonTr.GetEntriesFast();
	    new (clrefKaonTr[sizeKaonTr]) CbmD0TrackCandidate(itr, globalTrack->GetStsTrackIndex(), Chi2, ip, ipx, ipy, stsTrack->GetNofMvdHits(), stsTrack->GetNofStsHits(), -321 );

	}

	if(qp > 0)
	{
	    // --- Save Pion to  ParicleArray from Track ---
	    TClonesArray& clrefPion = *fPionParticleArray;
	    Int_t sizePion = clrefPion.GetEntriesFast();
	    new (clrefPion[sizePion]) KFParticle(newParticle);

            // --- Save Pion to  TrackArray for QA       ---
	    TClonesArray& clrefPionTr = *fD0PionTrackArray;
	    Int_t sizePionTr = clrefPionTr.GetEntriesFast();
	    new (clrefPionTr[sizePionTr]) CbmD0TrackCandidate(itr, globalTrack->GetStsTrackIndex(), Chi2, stsTrack->GetNofMvdHits(), stsTrack->GetNofStsHits(), 211 );
	}

	}// for loop track1
LOG(INFO) << "Found " << fKaonParticleArray->GetEntriesFast() << " Kaon candidates and " << fPionParticleArray->GetEntriesFast() << " Pion candidates" << FairLogger::endl;
LOG(INFO) << "|| ------------------------  End of event  ------------------------|| " << FairLogger::endl;
}
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
Int_t CbmD0TrackSelection::GetPid(CbmGlobalTrack* globalTrack)
{
CbmStsTrack*         stsTrack=NULL;
CbmTrdTrack*         trdTrack=NULL;
CbmRichRing*         richRing=NULL;
CbmTofHit*           tofHit=NULL;
//CbmTrackMatchNew*    mcTrackMatch;
//Int_t                mcTrackIndex;
CbmMCTrack*          mcTrack=NULL;

if(globalTrack->GetStsTrackIndex()!= -1) stsTrack    = (CbmStsTrack*) fStsTrackArray->At(globalTrack->GetStsTrackIndex());
if(globalTrack->GetTrdTrackIndex()!= -1) trdTrack    = (CbmTrdTrack*) fTrdTracks->At(globalTrack->GetTrdTrackIndex());     else trdTrack = NULL;
if(globalTrack->GetRichRingIndex()!= -1) richRing    = (CbmRichRing*) fRichRings->At(globalTrack->GetRichRingIndex());     else richRing = NULL;
if(globalTrack->GetTofHitIndex()!= -1)   tofHit      = (CbmTofHit*) fTofHits->At(globalTrack->GetTofHitIndex());           else tofHit = NULL;

KMinusRefit(stsTrack);

const FairTrackParam* par1 = stsTrack->GetParamFirst();
Double_t       	      qp   = par1->GetQp();

if(fPidMode == "NONE")
  {
  if(qp < 0.0)
    return -321;
  else
    return 211;
  }

else if(fPidMode == "MC")
    {
	mcTrack = GetMCTrackFromTrackID(globalTrack->GetStsTrackIndex());
       
	if(mcTrack && trdTrack && tofHit)
	{
          return mcTrack->GetPdgCode();
	} else return -1;
    }

else if(fPidMode == "TOF")
   {
       if(tofHit)
       {
        return 1;
       }
       else return 0;
   }

else if(fPidMode == "GLOBAL")
      {
          return 0;
      }
else return -1;

}
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
CbmMCTrack* CbmD0TrackSelection::GetMCTrackFromTrackID(Int_t trackID)
{
    CbmTrackMatchNew* mcTrackMatch;
    CbmMCTrack*       mcTrack;
    Int_t             mcTrackIndex = -1;

     mcTrackMatch       = (CbmTrackMatchNew*) fStsTrackMatches->At(trackID);
     if(!mcTrackMatch)
     	{
      	LOG(FATAL) << "TrackMatch problem "<< fEventNumber << FairLogger::endl;
       	}
     mcTrackIndex = mcTrackMatch->GetMatchedLink().GetIndex();
     if(mcTrackIndex>fListMCTracks->GetEntriesFast())
      	{
       	LOG(FATAL) << "McIndexProblem at index "<< mcTrackIndex << FairLogger::endl;
       	}
     if(mcTrackIndex<0)
        {
	    // bg event or delta electron
            return NULL;
      	}
     mcTrack = (CbmMCTrack*) fListMCTracks->At(mcTrackIndex);
     return mcTrack;
}
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
void CbmD0TrackSelection::KMinusRefit(CbmStsTrack* stsTrack)
{
const FairTrackParam* par1 = stsTrack->GetParamFirst();
Double_t       	      qp   = par1->GetQp();

 vector<CbmStsTrack> vRTracks(1);
 vRTracks[0] = *stsTrack;

 vector<int> vPID(1);
 vPID[0] = -321;

if(qp < 0.0)
    fFit->Fit(vRTracks, vPID);

return;
}
// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
void CbmD0TrackSelection::CheckMvdMatch(CbmStsTrack* stsTrack, Int_t mcTrackIndex,Int_t& goodMatch, Int_t& badMatch)
{
goodMatch=0;
badMatch=0;
Int_t nrOfMvdHits = stsTrack->GetNofMvdHits();
Int_t mcTrackId = 0;
Int_t nrOfLinks = 0;
//Int_t mcPointId = 0;

const CbmMvdPoint* point = NULL;
for(Int_t iHit = 0; iHit < nrOfMvdHits; iHit++)	
	{
        CbmMatch* mvdMatch = (CbmMatch*)fMvdHitMatchArray->At(stsTrack->GetMvdHitIndex(iHit));
	if(mvdMatch)
		{
		nrOfLinks = mvdMatch->GetNofLinks();
		}	
	else 
		{
		continue; // any kind of error in the matching
		}
	for(Int_t iLink = 0; iLink < nrOfLinks; iLink++)
		{
		Int_t pointIndex = mvdMatch->GetLink(iLink).GetIndex();
		if(pointIndex < fMcPoints->GetEntriesFast())	
 		point = (CbmMvdPoint*)fMcPoints->At(pointIndex);
		if (NULL == point)
			{
			continue; //delta or background event
			}
		else
			mcTrackId = point->GetTrackID();
		if(mcTrackId == mcTrackIndex)
			{
			goodMatch++;
			}
		else 
			badMatch++;
		}
	}    
}


// -------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0TrackSelection::GetImpactParameterRadius( KFParticle* particle ){

    Float_t imx  = particle->GetX();
    Float_t imy  = particle->GetY();
    Float_t imr  = sqrt(imx*imx + imy*imy);

   return  imr;
}



//-----------------------------------------------------------------------------------------

Double_t  CbmD0TrackSelection::GetImpactParameterX( KFParticle* particle ){

    Double_t imx  = particle->GetX();

   return  imx;
}



//-----------------------------------------------------------------------------------------

Double_t  CbmD0TrackSelection::GetImpactParameterY( KFParticle* particle ){

    Double_t imy  = particle->GetY();

   return  imy;
}



//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void CbmD0TrackSelection::SetCuts(Double_t p, Double_t pt, Double_t Chi2, Double_t IP){
    fCutP  = p;
    fCutPt = pt;
    fCutChi2 = Chi2;
    fCutIP = IP;
}
//-----------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------

void CbmD0TrackSelection::Finish(){

	//cout << endl << endl << " * * * * * \t strack - information \t * * * * * ";
  
}
// -------------------------------------------------------------------------------


ClassImp(CbmD0TrackSelection)

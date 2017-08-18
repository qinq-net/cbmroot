/**
 * \file CbmAnaConversion2Fitter.cxx
 *
 *
 * \author ??
 * \date ??
 **/

#include "CbmAnaConversion2Fitter.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "FairTrackParam.h"
#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"
#include "CbmHistManager.h"
#include "detector/CbmRichGeoManager.h"
#include "CbmStsKFTrackFitter.h"
#include "CbmVertex.h"
#include "CbmStsHit.h"
#include "../../data/mvd/CbmMvdHit.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>

#include "CbmStsKFSecondaryVertexFinder.h"

using namespace std;
using boost::assign::list_of;



CbmAnaConversion2Fitter::CbmAnaConversion2Fitter()
  : fMcTracks(nullptr),
    fStsTracks(nullptr),
    fStsTrackMatches(nullptr),
    fGlobalTracks(nullptr),
    fPrimVertex(nullptr),
    fArrayStsHit(nullptr),
    fArrayMvdHit(nullptr)

{
}

CbmAnaConversion2Fitter::~CbmAnaConversion2Fitter()
{
}

void CbmAnaConversion2Fitter::Init()
{

	cout << "CbmAnaConversion2Fitter::Init" << endl;
	FairRootManager* ioman = FairRootManager::Instance();
	if (nullptr == ioman) { Fatal("CbmAnaConversion2Fitter::Init","RootManager not instantised!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( nullptr == fMcTracks) { Fatal("CbmAnaConversion2Fitter::Init","No MCTrack array!"); }

	fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	if ( nullptr == fStsTracks) { Fatal("CbmAnaConversion2Fitter::Init","No StsTrack array!"); }

	fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
	if (nullptr == fStsTrackMatches) { Fatal("CbmAnaConversion2Fitter::Init","No StsTrackMatch array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (nullptr == fGlobalTracks) { Fatal("CbmAnaConversion2Fitter::Init","No GlobalTrack array!"); }


        // Get pointer to PrimaryVertex object from IOManager if it exists
        // The old name for the object is "PrimaryVertex" the new one
        // "PrimaryVertex." Check first for the new name
        fPrimVertex = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex."));
        if (nullptr == fPrimVertex) {
          fPrimVertex = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex"));
        }
        if (nullptr == fPrimVertex) { 
          LOG(FATAL) << "No PrimaryVertex array!" << FairLogger::endl;
        }

	fArrayStsHit = (TClonesArray*) ioman->GetObject("StsHit");
	if(nullptr == fArrayStsHit) { Fatal("CbmAnaConversion2Fitter::Init","No StsHit array!"); }

	fArrayMvdHit = (TClonesArray*) ioman->GetObject("MvdHit");
	if(nullptr == fArrayMvdHit) { Fatal("CbmAnaConversion2Fitter::Init","No MvdHit array!"); }

	InitHistograms();

}



void CbmAnaConversion2Fitter::InitHistograms()
{
	cout << "CbmAnaConversion2Fitter::InitHistograms() " << endl;
}



void CbmAnaConversion2Fitter::Exec(int fEventNum)
{
	int Event = fEventNum;

	cout << " " << endl;
	cout << "CbmAnaConversion2Fitter::Exec()   Event: " << Event << endl;

	test();


}



void CbmAnaConversion2Fitter::test()
{
	Int_t nofMcTracks = fMcTracks->GetEntriesFast();
	for (int i = 0; i < nofMcTracks; i++) {
		CbmMCTrack* mctrack = (CbmMCTrack*)fMcTracks->At(i);
		if (mctrack == nullptr) continue;
		CbmMCTrack* mcTrackMother = (CbmMCTrack*) fMcTracks->At(mctrack->GetMotherId());
		int motherPdg = -1;
		if (mcTrackMother != nullptr) motherPdg = mcTrackMother->GetPdgCode();

		int PdgCode = mctrack->GetPdgCode();
		if (TMath::Abs(PdgCode) != 11) continue;
		float xStartPoint = mctrack->GetStartX();
		float yStartPoint = mctrack->GetStartY();
		float zStartPoint = mctrack->GetStartZ();
		cout << " --> " << " particle pdg: " << PdgCode << "; mother pdg: " << motherPdg << "; xVertex = " << xStartPoint << "; yVertex = " << yStartPoint << "; zVertex = " << zStartPoint << " <-- Coordinates of start vertex [cm]" << endl;
		
		
	}

	Int_t ngTracks = fGlobalTracks->GetEntriesFast();
	for (Int_t iTr = 0; iTr < ngTracks; iTr++) {
		CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(iTr);
		if(nullptr == gTrack) continue;
		int stsInd = gTrack->GetStsTrackIndex();
		// ========================================================================================
		if (stsInd < 0) continue;    
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
		if (stsTrack == nullptr) continue;
		CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*) fStsTrackMatches->At(stsInd);
		if (stsMatch == nullptr) continue;
		if (stsMatch->GetNofLinks() <= 0) continue;
		int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
		if (stsMcTrackId < 0) continue;
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
		if (mcTrack1 == nullptr) continue;
		int pdgSTS = mcTrack1->GetPdgCode();
		int motherIdSTS = mcTrack1->GetMotherId();
		double momentumSTS = mcTrack1->GetP();
		if(TMath::Abs(pdgSTS) != 11) continue;
		CbmMCTrack* mcTrackMother = (CbmMCTrack*) fMcTracks->At(mcTrack1->GetMotherId());
		int motherPdg = -1;
		if (mcTrackMother != nullptr) motherPdg = mcTrackMother->GetPdgCode();

		cout << "num of STS hits: " << stsTrack->GetNofStsHits() << endl;
		cout << " -->  MCTrue info ==> particle pdg: " << mcTrack1->GetPdgCode() << "; motherPdg = " << motherPdg << endl;
		cout << "                  ==> position:   xVertex = " << mcTrack1->GetStartX() << "; yVertex = " << mcTrack1->GetStartY() << "; zVertex = " << mcTrack1->GetStartZ() << " <-- Coordinates of start vertex in [cm]" << endl;
		cout << "                  ==> momentum:   Px = " << mcTrack1->GetPx() << "; Py = " << mcTrack1->GetPy() << "; Pz = " << mcTrack1->GetPz() << endl;


/*// ========================================================================================
	    CbmStsKFSecondaryVertexFinder SVFinder;
	    SVFinder.AddTrack(stsTrack);
		SVFinder.SetMassConstraint(); // no constraints
	      SVFinder.Fit();
	      CbmVertex sv;
	      Double_t mass, mass_err;
	      SVFinder.GetVertex(sv);
	      //SVFinder.GetFittedTrack(0, &KFitted );
	      //SVFinder.GetFittedTrack(1, &PFitted );
	      SVFinder.GetMass(&mass, &mass_err);
	      if( sv.GetNDF()<=0) continue;
	      Double_t dx = sv.GetX();
	      Double_t dy = sv.GetY();
	      Double_t dz = sv.GetZ();
 cout << "     secondary vertex finder ==>   Fit results : " << endl;
 cout << "                                          position : " << " x = " << dx << "; y = " << dy << "; z = " << dz << endl;
 cout << "                                          mass : " << mass << endl;
 //cout << "                                          momentum : " << " px = " << momentum.X() << "; py = " << momentum.Y() << "; pz = " << momentum.Z() << endl;*/
// ========================================================================================

/*// ========================================================================================    TEST extract first hit from the track and make a fit to this vertex
   Int_t stsHitIndex;
   Int_t mvdHitIndex;
   CbmStsHit* stsHit;
   CbmMvdHit* mvdHit;
   cout << " NoHits: MVD = " << stsTrack->GetNofMvdHits() << "; STS = " << stsTrack->GetNofStsHits() << "; both = " << stsTrack->GetNofHits() << endl;
   for(Int_t iHit = 0; iHit < stsTrack->GetNofMvdHits(); iHit++) {
     // Get hit index
     mvdHitIndex = stsTrack->GetMvdHitIndex(iHit);
     // Get hit
     mvdHit = (CbmMvdHit*) fArrayMvdHit->At(mvdHitIndex);
     Double_t x_new = mvdHit->GetX();
     Double_t y_new = mvdHit->GetY();
     Double_t z_new = mvdHit->GetZ();
     cout << "MVD hit : " << " x = " << x_new << "; y = " << y_new << "; z = " << z_new << endl;
   }
   for(Int_t iHit = 0; iHit < stsTrack->GetNofStsHits(); iHit++) {
     stsHitIndex = stsTrack->GetStsHitIndex(iHit);
     stsHit = (CbmStsHit*) fArrayStsHit->At(stsHitIndex);
     Double_t x_new = stsHit->GetX();
     Double_t y_new = stsHit->GetY();
     Double_t z_new = stsHit->GetZ();
     cout << "STS hit : " << " x = " << x_new << "; y = " << y_new << "; z = " << z_new << endl;
   }
   CbmVertex* vtx = new CbmVertex();
   TMatrixFSym* covMat = new TMatrixFSym(3);
   if(stsTrack->GetNofMvdHits() != 0){
         mvdHitIndex = stsTrack->GetMvdHitIndex(0);
         mvdHit = (CbmMvdHit*) fArrayMvdHit->At(mvdHitIndex);
         vtx->SetVertex(mvdHit->GetX(), mvdHit->GetY(), mvdHit->GetZ(), 0, 0, 0, *covMat);
   } else if (stsTrack->GetNofStsHits() != 0){
         stsHitIndex = stsTrack->GetStsHitIndex(0);
         stsHit = (CbmStsHit*) fArrayStsHit->At(stsHitIndex);
         vtx->SetVertex(stsHit->GetX(), stsHit->GetY(), stsHit->GetZ(), 0, 0, 0, *covMat);
   }*/
// ========================================================================================






		CbmVertex* vtx = new CbmVertex();
		TMatrixFSym* covMat = new TMatrixFSym(3);
		vtx->SetVertex(mcTrack1->GetStartX(), mcTrack1->GetStartY(), mcTrack1->GetStartZ(), 0, 0, 0, *covMat);

		CbmStsKFTrackFitter fitter;
		FairTrackParam neu_track;
//		fitter.FitToVertex(stsTrack, fPrimVertex, &neu_track);
		fitter.FitToVertex(stsTrack, vtx, &neu_track);


		TVector3 position;
		TVector3 momentum;

		neu_track.Position(position);
		neu_track.Momentum(momentum);
 cout << "     Fit results : " << endl;
 cout << "                  position : " << " x = " << position.X() << "; y = " << position.Y() << "; z = " << position.Z() << endl;
 cout << "                  momentum : " << " px = " << momentum.X() << "; py = " << momentum.Y() << "; pz = " << momentum.Z() << endl;

		// ========================================================================================
	}

}




void CbmAnaConversion2Fitter::Finish()
{
	gDirectory->mkdir("Fitter");
	gDirectory->cd("Fitter");

	gDirectory->cd("..");

}

/**
 * \file CbmAnaConversion.cxx
 *
 * \author Tariq Mahmoud <t.mahmoud@gsi.de>
 * \date 2012
 **/

#include "CbmAnaConversion.h"

#include "TH1D.h"
#include "TH1.h"
#include "TH3.h"
#include "TCanvas.h"
#include "TClonesArray.h"

#include "CbmMCTrack.h"
#include "FairTrackParam.h"
#include "CbmRichHit.h"
#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmRichPoint.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"

#include "std/utils/CbmLitUtils.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>


#define M2E 2.6112004954086e-7

using namespace std;
using boost::assign::list_of;

CbmAnaConversion::CbmAnaConversion()
  : FairTask("CbmAnaConversion"),
    fhGammaZ(NULL),
    fhGammaZ_selected(NULL),
    fTest(NULL),
    fhNofElPrim(NULL),
    fhNofElSec(NULL),
    fhNofElAll(NULL),
    fhTomography(NULL),
    fhTomography_XZ(NULL),
    fhTomography_YZ(NULL),
    fhTomography_uptoRICH(NULL),
    fhTomography_RICH_complete(NULL),
    fhTomography_RICH_beampipe(NULL),
    fhTomography_STS_lastStation(NULL),    
    fhTomography_RICH_frontplate(NULL),    
    fhTomography_RICH_backplate(NULL), 
    fhTomography_reco(NULL),
    fhTomography_reco_XZ(NULL),
    fhTomography_reco_YZ(NULL),
    fhConversion(NULL),
    fhConversion_prob(NULL),
    fhConversion_energy(NULL),
    fhConversion_p(NULL),
    fhInvariantMass_test(NULL),
    fhInvariantMass_test2(NULL),
    fhInvariantMass_test3(NULL),
    fhInvariantMassReco_test(NULL),
    fhInvariantMassReco_test2(NULL),
    fhInvariantMassReco_test3(NULL),
    fhInvariantMass_MC_all(NULL),
    fhInvariantMass_MC_omega(NULL),
    fhInvariantMass_MC_pi0(NULL),
    fhInvariantMassReco_pi0(NULL),
    fhMomentum_MCvsReco(NULL),     
    fhMomentum_MCvsReco_diff(NULL),   
    fhInvariantMass_recoMomentum1(NULL),
    fhInvariantMass_recoMomentum2(NULL),
    fRichPoints(NULL),
    fMcTracks(NULL),
    fStsTracks(NULL),
    fStsTrackMatches(NULL),
    fGlobalTracks(NULL),
    fEventNum(0),
    test(0),
    testint(0),
    fAnalyseMode(0),
    fHistoList(),
    fHistoList_MC(),
    fHistoList_tomography(),
    fMCTracklist(),
    fMCTracklist_all(),
    fMCTracklist_omega(),
    fRecoTracklist(),
    fRecoTracklistEPEM(),
    fRecoMomentum()    
{
}

CbmAnaConversion::~CbmAnaConversion()
{

}

InitStatus CbmAnaConversion::Init()
{
   cout << "CbmAnaConversion::Init"<<endl;
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmAnaConversion::Init","RootManager not instantised!"); }

   fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
   if ( NULL == fRichPoints) { Fatal("CbmAnaConversion::Init","No RichPoint array!"); }

   fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( NULL == fMcTracks) { Fatal("CbmAnaConversion::Init","No MCTrack array!"); }

   fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
   if ( NULL == fStsTracks) { Fatal("CbmAnaConversion::Init","No StsTrack array!"); }

   fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
  if (NULL == fStsTrackMatches) { Fatal("CbmAnaConversion::Init","No StsTrackMatch array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (NULL == fGlobalTracks) { Fatal("CbmAnaConversion::Init","No GlobalTrack array!"); }

   InitHistograms();
   
     
   
	testint = 0;
	test = 0;

   return kSUCCESS;
}

void CbmAnaConversion::InitHistograms()
{
	fHistoList.clear();
	fHistoList_MC.clear();
	fHistoList_tomography.clear();
	
	fhGammaZ = new TH1D("fhGammaZ", "fhGammaZ;Z [cm];Entries", 300, 0.0, 300.);
	fhGammaZ_selected = new TH1D("fhGammaZ_selected", "fhGammaZ_selected;Z [cm];Entries", 600, 0.0, 600.);
	
	fTest = new TH2D("fTest", "fTest; X; Y", 200, -100, 100, 400, -200, 200);
	
	fhNofElPrim	= new TH1D("fhNofElPrim", "fhNofElPrim;Nof prim El;Entries", 10., -0.5, 9.5);
	fhNofElSec	= new TH1D("fhNofElSec", "fhNofElSec;Nof Sec El;Entries", 20., -0.5, 19.5);
	fhNofElAll	= new TH1D("fhNofElAll", "fhNofElAll;Nof All El;Entries", 30., -0.5, 29.5);
	
	// for all events including gammas and gamma-conversion
	fhTomography	= new TH3D("fhTomography", "Tomography/fhTomography;X [cm];Y [cm];Z [cm]", 200, -200., 200., 200, -200., 200., 500, 0., 1000.);
	fhTomography_XZ	= new TH2D("fhTomography_XZ", "fhTomography_XZ;X [cm];Z [cm]", 1600, -400., 400., 2400, 0., 1200.);
	fhTomography_YZ	= new TH2D("fhTomography_YZ", "fhTomography_YZ;Y [cm];Z [cm]", 1600, -400., 400., 2400, 0., 1200.);
	fHistoList_tomography.push_back(fhTomography);
	fHistoList_tomography.push_back(fhTomography_XZ);
	fHistoList_tomography.push_back(fhTomography_YZ);
	
	fhTomography_uptoRICH			= new TH2D("fhTomography_uptoRICH", "fhTomography_uptoRICH;X [cm];Y [cm]", 400, -200., 200., 400, -200., 200.);
	fhTomography_RICH_complete		= new TH2D("fhTomography_RICH_complete", "fhTomography_RICH_complete;X [cm];Y [cm]", 400, -200., 200., 400, -200., 200.);
	fhTomography_RICH_beampipe		= new TH2D("fhTomography_RICH_beampipe", "fhTomography_RICH_beampipe;X [cm];Y [cm]", 400, -100., 100., 200, -50., 50.);
	fhTomography_STS_end			= new TH2D("fhTomography_STS_end", "fhTomography_STS_end;X [cm];Y [cm]", 400, -200., 200., 200, -100., 100.);
	fhTomography_STS_lastStation	= new TH2D("fhTomography_STS_lastStation", "fhTomography_STS_lastStation;X [cm];Y [cm]", 200, -100., 100., 200, -100., 100.);
	fhTomography_RICH_frontplate	= new TH2D("fhTomography_RICH_frontplate", "fhTomography_RICH_frontplate;X [cm];Y [cm]", 400, -200., 200., 400, -200., 200.);
	fhTomography_RICH_backplate		= new TH2D("fhTomography_RICH_backplate", "fhTomography_RICH_backplate;X [cm];Y [cm]", 400, -200., 200., 400, -200., 200.);
	fHistoList_tomography.push_back(fhTomography_uptoRICH);
	fHistoList_tomography.push_back(fhTomography_RICH_complete);
	fHistoList_tomography.push_back(fhTomography_RICH_beampipe);
	fHistoList_tomography.push_back(fhTomography_STS_end);
	fHistoList_tomography.push_back(fhTomography_STS_lastStation);
	fHistoList_tomography.push_back(fhTomography_RICH_frontplate);
	fHistoList_tomography.push_back(fhTomography_RICH_backplate);
	
	
	// tomography from reconstructed tracks
	fhTomography_reco		= new TH3D("fhTomography_reco", "Tomography/fhTomography_reco;X [cm];Y [cm];Z [cm]", 200, -200., 200., 200, -200., 200., 500, 0., 1000.);
	fhTomography_reco_XZ	= new TH2D("fhTomography_reco_XZ", "fhTomography_reco_XZ;X [cm];Z [cm]", 1600, -400., 400., 2400, 0., 1200.);
	fhTomography_reco_YZ	= new TH2D("fhTomography_reco_YZ", "fhTomography_reco_YZ;Y [cm];Z [cm]", 1600, -400., 400., 2400, 0., 1200.);
	fHistoList.push_back(fhTomography_reco);
	fHistoList.push_back(fhTomography_reco_XZ);
	fHistoList.push_back(fhTomography_reco_YZ);
	
	
	
	fhConversion		= new TH1D("fhConversion", "fhConversion;Z [cm];# conversions", 1200, 0., 1200.);
	fhConversion_prob	= new TH1D("fhConversion_prob", "fhConversion_prob;Z [cm];# conversions", 1200, 0., 1200.);
	fHistoList.push_back(fhConversion);
	fHistoList.push_back(fhConversion_prob);
	
	fhConversion_energy	= new TH1D("fhConversion_energy", "fhConversion_energy;energy;#", 1000, 0., 100.);
	fhConversion_p		= new TH1D("fhConversion_p", "fhConversion_p;p;#", 1000, 0., 100.);
	fHistoList.push_back(fhConversion_energy);
	fHistoList.push_back(fhConversion_p);
	
	
	// for UrQMD events (invariant mass from pi0 -> gamma + gamma
	fhInvariantMass_test	= new TH1D("fhInvariant", "fhInvariant;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_test2	= new TH1D("fhInvariant2", "fhInvariant2;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_test3	= new TH1D("fhInvariant3", "fhInvariant3;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList.push_back(fhInvariantMass_test);
	fHistoList.push_back(fhInvariantMass_test2);
	fHistoList.push_back(fhInvariantMass_test3);
	
	fhInvariantMassReco_test	= new TH1D("fhInvariantReco", "fhInvariantReco;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMassReco_test2	= new TH1D("fhInvariantReco2", "fhInvariantReco2;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMassReco_test3	= new TH1D("fhInvariantReco3", "fhInvariantReco3;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList.push_back(fhInvariantMassReco_test);
	fHistoList.push_back(fhInvariantMassReco_test2);
	fHistoList.push_back(fhInvariantMassReco_test3);
	
	fhInvariantMassReco_pi0 = new TH1D("fhInvariantReco_pi0", "fhInvariantReco_pi0;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList.push_back(fhInvariantMassReco_pi0);
	
	fhInvariantMass_MC_all		= new TH1D("fhInvariantMass_MC_all", "fhInvariantMass_MC_all;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_MC_omega	= new TH1D("fhInvariantMass_MC_omega", "fhInvariantMass_MC_omega;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_MC_pi0		= new TH1D("fhInvariantMass_MC_pi0", "fhInvariantMass_MC_pi0;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList_MC.push_back(fhInvariantMass_MC_all);
	fHistoList_MC.push_back(fhInvariantMass_MC_omega);
	fHistoList_MC.push_back(fhInvariantMass_MC_pi0);
	
	
	// for reconstructed tracks
	fhMomentum_MCvsReco			= new TH2D("fhMomentum_MCvsReco","fhMomentum_MCvsReco;MC;Reco", 400, 0., 40., 400, 0., 40.); 
	fhMomentum_MCvsReco_diff	= new TH1D("fhMomentum_MCvsReco_diff","fhMomentum_MCvsReco_diff;(MC-Reco)/MC", 500, -0.01, 4.); 
	fHistoList.push_back(fhMomentum_MCvsReco);
	fHistoList.push_back(fhMomentum_MCvsReco_diff);
	
	fhInvariantMass_recoMomentum1 = new TH1D("fhInvariantMass_recoMomentum1","fhInvariantMass_recoMomentum1;mass;#", 2000, 0., 2.);
	fhInvariantMass_recoMomentum2 = new TH1D("fhInvariantMass_recoMomentum2","fhInvariantMass_recoMomentum2;mass;#", 2000, 0., 2.);
	fHistoList.push_back(fhInvariantMass_recoMomentum1);
	fHistoList.push_back(fhInvariantMass_recoMomentum2);

}



void CbmAnaConversion::Exec(Option_t* option)
{
	cout << "===================================================" << endl;
   cout << "CbmAnaConversion, event No. " <<  fEventNum << endl;
   fEventNum++;

   fMCTracklist.clear();
   fMCTracklist_all.clear();
   fMCTracklist_omega.clear();
   fRecoTracklist.clear();
   fRecoTracklistEPEM.clear();
   
   int countPrimEl = 0;
   int countSecEl = 0;
   int countAllEl  = 0;
   int countGammaEl = 0;
   int countMothers = 0;
   int countPrimPart = 0;   


   Int_t nofPoints = fRichPoints->GetEntriesFast();
   for (int i = 0; i < nofPoints; i++) {
	   CbmRichPoint* point = (CbmRichPoint*)fRichPoints->At(i);
	   if (point == NULL) continue;
	   //cout << point->GetX() << "\t" << point->GetY() << endl;
	   fTest->Fill(point->GetX(), point->GetY());
   }
   

	// =============================================
	// START - Analyse MC tracks
	Int_t nofMcTracks = fMcTracks->GetEntriesFast();
	for (int i = 0; i < nofMcTracks; i++) {

		CbmMCTrack* mctrack = (CbmMCTrack*)fMcTracks->At(i);
		if (mctrack == NULL) continue;
   
   
		TomographyMC(mctrack); // analyse gamma-conversions with MC data
   
		FillMCTracklist(mctrack);
		FillMCTracklist_all(mctrack);


	   
	   if (mctrack->GetMotherId() == -1) {
	   		countPrimPart++;
	   }   
	   if (mctrack->GetMotherId() == -1 && TMath::Abs( mctrack->GetPdgCode())  == 11) {
		  countPrimEl++;
	   }   
	   if (mctrack->GetMotherId() != -1 && TMath::Abs( mctrack->GetPdgCode())  == 11) {
		  countSecEl++;
	   }   
	   if (TMath::Abs( mctrack->GetPdgCode())  == 11) {
		   countAllEl++;
	   }

		if (TMath::Abs( mctrack->GetPdgCode())  == 11) {
			int motherId = mctrack->GetMotherId();
			if (motherId == -1) continue;
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
			//cout << mcMotherPdg << endl;

			if (mcMotherPdg == 22) {
				TVector3 v;
				mctrack->GetStartVertex(v);
				fhGammaZ->Fill(v.Z());
				countGammaEl++;
			}
		}
	}
	
	InvariantMassTest();
	cout << "fMCTracklist_omega: " << fMCTracklist_all.size() << endl;
	if(fMCTracklist_all.size() > 1) {
		//InvariantMassTest_omega();
		CalculateInvMass_MC_2particles();
	}
   
	fhNofElPrim->Fill(countPrimEl);
	fhNofElSec->Fill(countSecEl);
	fhNofElAll->Fill(countAllEl);
   
   if(countPrimEl == 2 && countAllEl == 4) {
   		testint++;
   		for (int i = 0; i < nofMcTracks; i++) {
			CbmMCTrack* mctrack = (CbmMCTrack*)fMcTracks->At(i);
			if (TMath::Abs( mctrack->GetPdgCode())  == 11) {
				int motherId = mctrack->GetMotherId();
				if (motherId == -1) continue;
	   			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
	   			int mcMotherPdg  = -1;
	    		if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
	    
				if (mcMotherPdg == 22) {
					TVector3 v;
					mctrack->GetStartVertex(v);
					fhGammaZ_selected->Fill(v.Z());
		  		}
	   		}
	   }
   }

//	cout << "Nof Prim El = " << countPrimEl << endl;
//	cout << "Nof Sec El = " << countSecEl << endl;
//	cout << "Nof All El = " << countAllEl << endl;
//	cout << "Nof Gamma El = " << countGammaEl << endl;
//	cout << "Nof McTracks = " << nofMcTracks << endl;
//	cout << "Nof PrimPart = " << countPrimPart << endl;


	// END - Analyse MC tracks
	// =============================================
   
   
   
	// =============================================
	// START - Analyse reconstructed tracks
	Int_t ngTracks = fGlobalTracks->GetEntriesFast();
	for (Int_t i = 0; i < ngTracks; i++) {
       CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(i);
       if(NULL == gTrack) continue;
       int stsInd = gTrack->GetStsTrackIndex();
       int richInd = gTrack->GetRichRingIndex();
       int trdInd = gTrack->GetTrdTrackIndex();
       int tofInd = gTrack->GetTofHitIndex();

       if (stsInd < 0) continue;
       CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
       if (stsTrack == NULL) continue;
       CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*) fStsTrackMatches->At(stsInd);
       if (stsMatch == NULL) continue;
       int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
       if (stsMcTrackId < 0) continue;
       CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
       if (mcTrack1 == NULL) continue;
       int pdg = TMath::Abs(mcTrack1->GetPdgCode());
       int motherId = mcTrack1->GetMotherId();
       double momentum = mcTrack1->GetP();
       stsMatch->GetTrueOverAllHitsRatio();
       
       TomographyReco(mcTrack1);
       FillRecoTracklist(mcTrack1);
       
       
       TVector3 stsMomentumVec; 
       stsTrack->GetParamFirst()->Momentum(stsMomentumVec);
       Double_t stsMomentum = stsMomentumVec.Mag();
       TVector3 mcMomentumVec;
       mcTrack1->GetMomentum(mcMomentumVec);
       Double_t mcMomentum = mcMomentumVec.Mag();
       fhMomentum_MCvsReco->Fill(mcMomentum, stsMomentum);
       fhMomentum_MCvsReco_diff->Fill(TMath::Abs(mcMomentum-stsMomentum)/mcMomentum);
       
       FillRecoTracklistEPEM(mcTrack1, stsMomentumVec);
       
	}
	InvariantMassTestReco();
	InvariantMassTest_4epem();
	
	// END - analyse reconstructed tracks
	// =============================================
   

   // example of how to get refitted momenta at primary vertex
  /*
   CbmL1PFFitter fPFFitter;
   vector<CbmStsTrack> stsTracks;
   stsTracks.resize(1);
   stsTracks[0] = *stsTrack;
   vector<L1FieldRegion> vField;
   vector<float> chiPrim;
   fPFFitter.GetChiToVertex(stsTracks, vField, chiPrim, fKFVertex, 3e6);
   cand.chi2sts = stsTracks[0].GetChiSq() / stsTracks[0].GetNDF();
   cand.chi2Prim = chiPrim[0];
   const FairTrackParam* vtxTrack = stsTracks[0].GetParamFirst();

   vtxTrack->Position(cand.position);
   vtxTrack->Momentum(cand.momentum);*/


// =========================================================================================================================
// ============================================== END - EXEC function ======================================================
// =========================================================================================================================
}



void CbmAnaConversion::Finish()
{
	Probability();
	
	
	cout << "test " << test << endl;
	
/*	
	TCanvas* c = new TCanvas();
	c->SetWindowSize(800, 1600);
	DrawH1(fTest);
	//fhGammaZ->Write();
	
	TCanvas* c2 = new TCanvas();
	c2->SetWindowSize(2000, 1600);
	DrawH1(fhGammaZ_selected);
	
	TCanvas* c3 = new TCanvas();
	c3->SetWindowSize(2000, 1600);
	gPad->Divide(3,1);
	c3->cd(1);
	DrawH1(fhNofElPrim);
	c3->cd(2);
	DrawH1(fhNofElSec);
	c3->cd(3);
	DrawH1(fhNofElAll);
	
	
	//TCanvas* c4 = new TCanvas();
	//c4->SetWindowSize(2000, 1600);
	//DrawH1(fhTomography);
*/
	
	// Write histograms to a file
	gDirectory->mkdir("analysis-conversion");
	gDirectory->cd("analysis-conversion");
	for (Int_t i = 0; i < fHistoList.size(); i++){
		fHistoList[i]->Write();
	}
	
	gDirectory->mkdir("MC data");
	gDirectory->cd("MC data");
	for (Int_t i = 0; i < fHistoList_MC.size(); i++){
		fHistoList_MC[i]->Write();
	}
	gDirectory->cd("..");
	
	gDirectory->mkdir("Tomography");
	gDirectory->cd("Tomography");
	for (Int_t i = 0; i < fHistoList_tomography.size(); i++){
		fHistoList_tomography[i]->Write();
	}
	
	
	gDirectory->cd("../..");
	
// =========================================================================================================================
// ============================================== END - FINISH function ====================================================
// =========================================================================================================================
}








KinematicParams CbmAnaConversion::CalculateKinematicParams(
      const CbmMCTrack* mctrackP,
      const CbmMCTrack* mctrackM)
{
    KinematicParams params;

    TVector3 momP;  //momentum e+
    mctrackP->GetMomentum(momP);
    Double_t energyP = TMath::Sqrt(momP.Mag2() + M2E);
    TLorentzVector lorVecP(momP, energyP);

    TVector3 momM;  //momentum e-
    mctrackM->GetMomentum(momM);
    Double_t energyM = TMath::Sqrt(momM.Mag2() + M2E);
    TLorentzVector lorVecM(momM, energyM);

    TVector3 momPair = momP + momM;
    Double_t energyPair = energyP + energyM;
    Double_t ptPair = momPair.Perp();
    Double_t pzPair = momPair.Pz();
    Double_t yPair = 0.5*TMath::Log((energyPair+pzPair)/(energyPair-pzPair));
    Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
    Double_t theta = 180.*anglePair/TMath::Pi();
    Double_t minv = 2.*TMath::Sin(anglePair / 2.)*TMath::Sqrt(momM.Mag()*momP.Mag());

    params.momentumMag = momPair.Mag();
    params.pt = ptPair;
    params.rapidity = yPair;
    params.minv = minv;
    params.angle = theta;
    return params;
}

Double_t CbmAnaConversion::Invmass_2gammas(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2) 
// calculation of invariant mass from two gammas (m=0)
{
    TVector3 mom1;
    mctrack1->GetMomentum(mom1);
    Double_t energy1 = TMath::Sqrt(mom1.Mag2());
    TLorentzVector lorVec1(mom1, energy1);

    TVector3 mom2;
    mctrack2->GetMomentum(mom2);
    Double_t energy2 = TMath::Sqrt(mom2.Mag2());
    TLorentzVector lorVec2(mom2, energy2);
    
    TLorentzVector sum = lorVec1 + lorVec2;
	return sum.Mag();
}


Double_t CbmAnaConversion::Invmass_gepem(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3) 
// calculation of invariant mass from two electrons/positrons and one gamma (-> mctrack3)
{
    TVector3 mom1;
    mctrack1->GetMomentum(mom1);
    Double_t energy1 = TMath::Sqrt(mom1.Mag2() + M2E);
    TLorentzVector lorVec1(mom1, energy1);

    TVector3 mom2;
    mctrack2->GetMomentum(mom2);
    Double_t energy2 = TMath::Sqrt(mom2.Mag2() + M2E);
    TLorentzVector lorVec2(mom2, energy2);

    TVector3 mom3;
    mctrack3->GetMomentum(mom3);
    Double_t energy3 = TMath::Sqrt(mom3.Mag2());
    TLorentzVector lorVec3(mom3, energy3);
    
    TLorentzVector sum = lorVec1 + lorVec2 + lorVec3;
	return sum.Mag();
}


Double_t CbmAnaConversion::Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2) 
// calculation of invariant mass from two electrons/positrons
{
    TVector3 mom1;
    mctrack1->GetMomentum(mom1);
    Double_t energy1 = TMath::Sqrt(mom1.Mag2() + M2E);
    TLorentzVector lorVec1(mom1, energy1);

    TVector3 mom2;
    mctrack2->GetMomentum(mom2);
    Double_t energy2 = TMath::Sqrt(mom2.Mag2() + M2E);
    TLorentzVector lorVec2(mom2, energy2);
    
    TLorentzVector sum = lorVec1 + lorVec2;
	return sum.Mag();
}


Double_t CbmAnaConversion::Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4)
// calculation of invariant mass from four electrons/positrons
{
    TVector3 mom1;
    mctrack1->GetMomentum(mom1);
    Double_t energy1 = TMath::Sqrt(mom1.Mag2() + M2E);
    TLorentzVector lorVec1(mom1, energy1);

    TVector3 mom2;
    mctrack2->GetMomentum(mom2);
    Double_t energy2 = TMath::Sqrt(mom2.Mag2() + M2E);
    TLorentzVector lorVec2(mom2, energy2);

    TVector3 mom3;
    mctrack3->GetMomentum(mom3);
    Double_t energy3 = TMath::Sqrt(mom3.Mag2() + M2E);
    TLorentzVector lorVec3(mom3, energy3);

    TVector3 mom4;
    mctrack4->GetMomentum(mom4);
    Double_t energy4 = TMath::Sqrt(mom4.Mag2() + M2E);
    TLorentzVector lorVec4(mom4, energy4);
    
    TLorentzVector sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;
	return sum.Mag();
}




Double_t CbmAnaConversion::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
// calculation of invariant mass from four electrons/positrons
{
    Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
    TLorentzVector lorVec1(part1, energy1);

    Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
    TLorentzVector lorVec2(part2, energy2);

    Double_t energy3 = TMath::Sqrt(part3.Mag2() + M2E);
    TLorentzVector lorVec3(part3, energy3);

    Double_t energy4 = TMath::Sqrt(part4.Mag2() + M2E);
    TLorentzVector lorVec4(part4, energy4);
    
    TLorentzVector sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;
	return sum.Mag();
}






void CbmAnaConversion::TomographyMC(CbmMCTrack* mctrack)
// doing tomography from gamma -> e+ e- decays, MC DATA
{
		if (TMath::Abs( mctrack->GetPdgCode())  == 11) { 
			int motherId = mctrack->GetMotherId();
			if (motherId != -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (NULL != mother) mcMotherPdg = mother->GetPdgCode();

			if (mcMotherPdg == 22) {
				TVector3 v;
				mctrack->GetStartVertex(v);
				fhGammaZ->Fill(v.Z());
			  
				fhTomography->Fill(v.X(), v.Y(), v.Z());
				fhTomography_XZ->Fill(v.X(), v.Z());
				fhTomography_YZ->Fill(v.Y(), v.Z());
				fhConversion_energy->Fill(mctrack->GetEnergy());
				fhConversion_p->Fill(mctrack->GetP());
				
				
				if(v.z() >= 0 && v.Z() <= 170) { // only in region before the RICH
					fhTomography_uptoRICH->Fill(v.X(), v.Y());
				}
				if(v.z() >= 170 && v.Z() <= 400) { // only in region of the RICH detector
					fhTomography_RICH_complete->Fill(v.X(), v.Y());
				}
				if(v.z() >= 220 && v.Z() <= 280) { // only in region of RICH beampipe, without being distorted by mirrors or camera
					fhTomography_RICH_beampipe->Fill(v.X(), v.Y());
				}
				if(v.z() >= 100 && v.Z() <= 170) { // only in a downstream part of STS and magnet
					fhTomography_STS_end->Fill(v.X(), v.Y());
				}
				if(v.z() >= 98 && v.Z() <= 102) { // only in a downstream part of STS and magnet, only last STS station
					fhTomography_STS_lastStation->Fill(v.X(), v.Y());
				}
				if(v.z() >= 179 && v.Z() <= 181) { // only in region of RICH frontplate
					fhTomography_RICH_frontplate->Fill(v.X(), v.Y());
				}
				if(v.z() >= 369 && v.Z() <= 371) { // only in region of RICH backplate
					fhTomography_RICH_backplate->Fill(v.X(), v.Y());
				}
				
				fhConversion->Fill(v.z());
			}
			}
		}
}


void CbmAnaConversion::TomographyReco(CbmMCTrack* mctrack)
// doing tomography from gamma -> e+ e- decays, RECONSTRUCTED DATA
{
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) {
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (NULL != mother) mcMotherPdg = mother->GetPdgCode();

			if (mcMotherPdg == 22) {
				TVector3 v;
				mctrack->GetStartVertex(v);
			  
				fhTomography_reco->Fill(v.X(), v.Y(), v.Z());
				fhTomography_reco_XZ->Fill(v.X(), v.Z());
				fhTomography_reco_YZ->Fill(v.Y(), v.Z());
			}
		}
	}
}




void CbmAnaConversion::Probability() 
{
	for(int i=1; i<=fhConversion->GetNbinsX(); i++) {
		int temp = 0;
		for(int run=1; run<=i; run++ ) {
			temp += fhConversion->GetBinContent(run);
		}
		fhConversion_prob->SetBinContent(i, temp);
	}
}



void CbmAnaConversion::FillMCTracklist(CbmMCTrack* mctrack) 
// only decay pi0 -> gamma gamma considered
{	
		if (TMath::Abs( mctrack->GetPdgCode())  == 22) { 
			int motherId = mctrack->GetMotherId();
			if (motherId != -1) {
				CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
				int mcMotherPdg  = -1;
				if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
				if (mcMotherPdg == 111) {	// pdg code 111 = pi0
					fMCTracklist.push_back(mctrack);
				}
			}
		}
}



void CbmAnaConversion::FillMCTracklist_all(CbmMCTrack* mctrack) 
// fill all electrons/positrons in tracklist
{	
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) { 
			int motherId = mctrack->GetMotherId();
			if (motherId != -1) {
				fMCTracklist_all.push_back(mctrack);
			}
	}
}




void CbmAnaConversion::FillMCTracklist_omega(CbmMCTrack* mctrack) 
// only decay omega -> e+ e- considered
{
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) {
		int motherId = mctrack->GetMotherId();
		if (motherId == -1 && fAnalyseMode == 3) {	// fAnalyseMode == 3 -> pluto files
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (NULL != mother) mcMotherPdg = mother->GetPdgCode();                                                                                                                                                                                                                                                                     
				fMCTracklist_omega.push_back(mctrack);
				cout << "OMEGA FILL: pdg " << mctrack->GetPdgCode() << "\t motherid " << motherId << endl;
		}
		if (motherId != -1 && fAnalyseMode == 2) {	// fAnalyseMode == 2 -> urqmd files
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
			if (mcMotherPdg == 223) {	// pdg code 223 = omega
				fMCTracklist_omega.push_back(mctrack);
				cout << "OMEGA FILL: pdg " << mctrack->GetPdgCode() << "\t motherid " << motherId << endl;
			}
		}
		
	}
}





void CbmAnaConversion::FillRecoTracklist(CbmMCTrack* mctrack) 
{
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) { 
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
		CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
		int mcMotherPdg  = -1;
		if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
		if (mcMotherPdg == 111 || mcMotherPdg == 22) {	// pdg code 111 = pi0, 22 = gamma
			fRecoTracklist.push_back(mctrack);
		//	cout << "pdg " << mctrack->GetPdgCode() << "\t motherid " << motherId << endl;
			test++;
		}
		}
	}
}




void CbmAnaConversion::FillRecoTracklistEPEM(CbmMCTrack* mctrack, TVector3 stsMomentum) 
{
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) { 
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			fRecoTracklistEPEM.push_back(mctrack);
			fRecoMomentum.push_back(stsMomentum);
		}
	}
}




void CbmAnaConversion::CalculateInvMass_MC_2particles()
{
	for(int i=0; i<fMCTracklist_all.size(); i++) {
		for(int j=i+1; j<fMCTracklist_all.size(); j++) {
			Double_t invmass = Invmass_2particles(fMCTracklist_all[i],fMCTracklist_all[j]);
			
			int motherId_i = fMCTracklist_all[i]->GetMotherId();
			int motherId_j = fMCTracklist_all[j]->GetMotherId();
			
			CbmMCTrack* mother_i = (CbmMCTrack*) fMcTracks->At(motherId_i);
			int mcMotherPdg_i  = -1;
			if (NULL != mother_i) mcMotherPdg_i = mother_i->GetPdgCode();
		
			CbmMCTrack* mother_j = (CbmMCTrack*) fMcTracks->At(motherId_j);
			int mcMotherPdg_j  = -1;
			if (NULL != mother_j) mcMotherPdg_j = mother_j->GetPdgCode();
			
			if(motherId_i == motherId_j && ( (fMCTracklist_all[i]->GetPdgCode() == 11 && fMCTracklist_all[j]->GetPdgCode() == -11) || (fMCTracklist_all[i]->GetPdgCode() == -11 && fMCTracklist_all[j]->GetPdgCode() == 11))) {
				fhInvariantMass_MC_all->Fill(invmass);
				//cout << "e+e- decay detected! MotherId " << motherId_i << "\t invariant mass: " << invmass << endl;
				
				if(mcMotherPdg_i == 223) {
					fhInvariantMass_MC_omega->Fill(invmass);
					cout << "-- mother particle of decay: omega" << endl;
				}
				if(mcMotherPdg_i == 111) {
					fhInvariantMass_MC_pi0->Fill(invmass);
					cout << "-- mother particle of decay: pi0" << endl;
				}
			}
		}
	}



}



void CbmAnaConversion::InvariantMassTest() 
// calculation of invariant mass via pi0-> gamma gamma, ONLY FROM MC DATA!
{
	for(int i=0; i<fMCTracklist.size(); i++) {
		for(int j=i+1; j<fMCTracklist.size(); j++) {
			
			//if(fMCTracklist[i]->GetPx() != fMCTracklist[j]->GetPx() && fMCTracklist[i]->GetPy() != fMCTracklist[j]->GetPy()) {
			Double_t invmass = Invmass_2gammas(fMCTracklist[i],fMCTracklist[j]);
			fhInvariantMass_test->Fill(invmass);
			TVector3 vi;
			fMCTracklist[i]->GetStartVertex(vi);
			TVector3 vj;
			fMCTracklist[j]->GetStartVertex(vj);
			
			int motherId_i = fMCTracklist[i]->GetMotherId();
			int motherId_j = fMCTracklist[j]->GetMotherId();
			
			if(motherId_i == motherId_j) {
				fhInvariantMass_test2->Fill(invmass);
				
				if(invmass < 0.001) {
				//	cout << "gamma1 " << fMCTracklist[i]->GetPx() << "\t" << fMCTracklist[i]->GetPy() << "\t" << fMCTracklist[i]->GetPz() << endl;
				//	cout << "gamma2 " << fMCTracklist[j]->GetPx() << "\t" << fMCTracklist[j]->GetPy() << "\t" << fMCTracklist[j]->GetPz() << endl;
				}
			}
			if(vi.Z() == vj.Z() && vi.Y() == vj.Y() && vi.X() == vj.X()) {
				fhInvariantMass_test3->Fill(invmass);
			}
			//}
		}
	}
}





void CbmAnaConversion::InvariantMassTest_4epem()
// Calculating invariant mass of 4 ep/em, using MC data AND reconstructed momentum
{
	cout << "4epem start" << endl;
	cout << fRecoTracklistEPEM.size() << endl;
	int fill = 0;
	for(int i=0; i<fRecoTracklistEPEM.size(); i++) {
		for(int j=i+1; j<fRecoTracklistEPEM.size(); j++) {
			for(int k=j+1; k<fRecoTracklistEPEM.size(); k++) {
				for(int l=k+1; l<fRecoTracklistEPEM.size(); l++) {
					if(fRecoTracklistEPEM[i]->GetPdgCode() + fRecoTracklistEPEM[j]->GetPdgCode() + fRecoTracklistEPEM[k]->GetPdgCode() + fRecoTracklistEPEM[l]->GetPdgCode() != 0) continue;
					
					
					int motherId1 = fRecoTracklistEPEM[i]->GetMotherId();
					int motherId2 = fRecoTracklistEPEM[j]->GetMotherId();
					int motherId3 = fRecoTracklistEPEM[k]->GetMotherId();
					int motherId4 = fRecoTracklistEPEM[l]->GetMotherId();
					
					if(motherId1 == motherId2 && motherId1 == motherId3 && motherId1 == motherId4) continue;
					
					if( (motherId1 == motherId2 && motherId3 == motherId4) ||
						(motherId1 == motherId3 && motherId2 == motherId4) ||
						(motherId1 == motherId4 && motherId2 == motherId3) ) {
					
						int grandmotherId1 = -1;
						int grandmotherId2 = -1;
						int grandmotherId3 = -1;
						int grandmotherId4 = -1;
						
						int mcMotherPdg1  = -1;
						int mcMotherPdg2  = -1;
						int mcMotherPdg3  = -1;
						int mcMotherPdg4  = -1;
						int mcGrandmotherPdg1  = -1;
						int mcGrandmotherPdg2  = -1;
						int mcGrandmotherPdg3  = -1;
						int mcGrandmotherPdg4  = -1;
						
						
						if (motherId1 != -1) {
							CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
							if (NULL != mother1) mcMotherPdg1 = mother1->GetPdgCode();
							grandmotherId1 = mother1->GetMotherId();
							if(grandmotherId1 != -1) {
								CbmMCTrack* grandmother1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
								if (NULL != grandmother1) mcGrandmotherPdg1 = grandmother1->GetPdgCode();
							}
						}
						if (motherId2 != -1) {
							CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
							if (NULL != mother2) mcMotherPdg2 = mother2->GetPdgCode();
							grandmotherId2 = mother2->GetMotherId();
							if(grandmotherId2 != -1) {
								CbmMCTrack* grandmother2 = (CbmMCTrack*) fMcTracks->At(grandmotherId2);
								if (NULL != grandmother2) mcGrandmotherPdg2 = grandmother2->GetPdgCode();
							}
						}
						if (motherId3 != -1) {
							CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
							if (NULL != mother3) mcMotherPdg3 = mother3->GetPdgCode();
							grandmotherId3 = mother3->GetMotherId();
							if(grandmotherId3 != -1) {
								CbmMCTrack* grandmother3 = (CbmMCTrack*) fMcTracks->At(grandmotherId3);
								if (NULL != grandmother3) mcGrandmotherPdg3 = grandmother3->GetPdgCode();
							}
						}
						if (motherId4 != -1) {
							CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);
							if (NULL != mother4) mcMotherPdg4 = mother4->GetPdgCode();
							grandmotherId4 = mother4->GetMotherId();
							if(grandmotherId4 != -1) {
								CbmMCTrack* grandmother4 = (CbmMCTrack*) fMcTracks->At(grandmotherId4);
								if (NULL != grandmother4) mcGrandmotherPdg4 = grandmother4->GetPdgCode();
							}
						}
					
					//if( (motherId_i == motherId_j && motherId_k == motherId_l) ||
					//	(motherId_i == motherId_k && motherId_j == motherId_l) ||
					//	(motherId_i == motherId_l && motherId_j == motherId_k) )
					//	{
					
					if(grandmotherId1 == grandmotherId2 && grandmotherId1 == grandmotherId3 && grandmotherId1 == grandmotherId4) {
						if(grandmotherId1 == -1) continue;
						if(mcGrandmotherPdg1 != 111) continue;
					Double_t invmass1 = Invmass_4particlesRECO(fRecoMomentum[i], fRecoMomentum[j], fRecoMomentum[k], fRecoMomentum[l]);
					Double_t invmass2 = Invmass_4particles(fRecoTracklistEPEM[i], fRecoTracklistEPEM[j], fRecoTracklistEPEM[k], fRecoTracklistEPEM[l]);
					
					cout << "######################################################################" << endl;
					cout << "index: " << i << "\t" << j << "\t" << k << "\t" << l << endl;
					cout << "motherid: " << motherId1 << "\t" << motherId2 << "\t" << motherId3 << "\t" << motherId4 << endl;
					cout << "motherpdf: " << mcMotherPdg1 << "\t" << mcMotherPdg2 << "\t" << mcMotherPdg3 << "\t" << mcMotherPdg4 << endl;
					cout << "grandmotherid: " << grandmotherId1 << "\t" << grandmotherId2 << "\t" << grandmotherId3 << "\t" << grandmotherId4 << endl;
					cout << "pdg: " << fRecoTracklistEPEM[i]->GetPdgCode() << "\t" << fRecoTracklistEPEM[j]->GetPdgCode() << "\t" << fRecoTracklistEPEM[k]->GetPdgCode() << "\t" << fRecoTracklistEPEM[l]->GetPdgCode() << endl;
					cout << "invmass reco: " << invmass1 << "\t invmass mc: " << invmass2 << endl;
					
					fhInvariantMass_recoMomentum1->Fill(invmass1);
					fhInvariantMass_recoMomentum2->Fill(invmass2);
					fill++;
					}
					}
				}
			}
		}
	}
	cout << "Filled events: " << fill << endl;
	cout << "4epem end" << endl;
}







void CbmAnaConversion::InvariantMassTest_omega() 
// calculation of invariant mass via omega -> e+ e-, ONLY FROM MC DATA!
{
	for(int i=0; i<fMCTracklist_omega.size(); i++) {
		for(int j=i+1; j<fMCTracklist_omega.size(); j++) {
			Double_t invmass = Invmass_2particles(fMCTracklist_omega[i],fMCTracklist_omega[j]);
			TVector3 vi;
			fMCTracklist_omega[i]->GetStartVertex(vi);
			TVector3 vj;
			fMCTracklist_omega[j]->GetStartVertex(vj);
			
			int motherId_i = fMCTracklist_omega[i]->GetMotherId();
			int motherId_j = fMCTracklist_omega[j]->GetMotherId();
			
			if(motherId_i == motherId_j && ( (fMCTracklist_omega[i]->GetPdgCode() == 11 && fMCTracklist_omega[j]->GetPdgCode() == -11) || (fMCTracklist_omega[i]->GetPdgCode() == -11 && fMCTracklist_omega[j]->GetPdgCode() == 11))) {
				fhInvariantMass_MC_omega->Fill(invmass);
				cout << "Omega decay detected! MotherId " << motherId_i << "\t invariant mass: " << invmass << endl;
			}
			if(vi.Z() == vj.Z() && vi.Y() == vj.Y() && vi.X() == vj.X()) {
				//fhInvariantMass_test3->Fill(invmass);
			}
		}
	}
}




void CbmAnaConversion::InvariantMassTestReco() 
// calculation of invariant mass via pi0 -> .. -> e+ e- e+ e-, ONLY FROM RECONSTRUCTED TRACKS!
{
	cout << "InvariantMassTestReco - Start..." << endl;
	cout << "InvariantMassTestReco - Size of fRecoTracklist:\t " << fRecoTracklist.size() << endl;
	if(fRecoTracklist.size() >= 4) {
		for(int i=0; i<fRecoTracklist.size(); i++) {
			for(int j=i+1; j<fRecoTracklist.size()-1; j++) {
				for(int k=j+1; k<fRecoTracklist.size()-2; k++) {
					for(int l=k+1; l<fRecoTracklist.size()-3; l++) {
					
						if(fRecoTracklist[i]->GetPdgCode() + fRecoTracklist[j]->GetPdgCode() + fRecoTracklist[k]->GetPdgCode() + fRecoTracklist[l]->GetPdgCode() != 0) continue;
					
						int motherId1 = fRecoTracklist[i]->GetMotherId();
						int motherId2 = fRecoTracklist[j]->GetMotherId();
						int motherId3 = fRecoTracklist[k]->GetMotherId();
						int motherId4 = fRecoTracklist[l]->GetMotherId();
						int grandmotherId1 = -1;
						int grandmotherId2 = -1;
						int grandmotherId3 = -1;
						int grandmotherId4 = -1;
						
						int mcMotherPdg1  = -1;
						int mcMotherPdg2  = -1;
						int mcMotherPdg3  = -1;
						int mcMotherPdg4  = -1;
						int mcGrandmotherPdg1  = -1;
						int mcGrandmotherPdg2  = -1;
						int mcGrandmotherPdg3  = -1;
						int mcGrandmotherPdg4  = -1;
						
						
						if (motherId1 != -1) {
							CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
							if (NULL != mother1) mcMotherPdg1 = mother1->GetPdgCode();
							grandmotherId1 = mother1->GetMotherId();
							if(grandmotherId1 != -1) {
								CbmMCTrack* grandmother1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
								if (NULL != grandmother1) mcGrandmotherPdg1 = grandmother1->GetPdgCode();
							}
						}
						if (motherId2 != -1) {
							CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
							if (NULL != mother2) mcMotherPdg2 = mother2->GetPdgCode();
							grandmotherId2 = mother2->GetMotherId();
							if(grandmotherId2 != -1) {
								CbmMCTrack* grandmother2 = (CbmMCTrack*) fMcTracks->At(grandmotherId2);
								if (NULL != grandmother2) mcGrandmotherPdg2 = grandmother2->GetPdgCode();
							}
						}
						if (motherId3 != -1) {
							CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
							if (NULL != mother3) mcMotherPdg3 = mother3->GetPdgCode();
							grandmotherId3 = mother3->GetMotherId();
							if(grandmotherId3 != -1) {
								CbmMCTrack* grandmother3 = (CbmMCTrack*) fMcTracks->At(grandmotherId3);
								if (NULL != grandmother3) mcGrandmotherPdg3 = grandmother3->GetPdgCode();
							}
						}
						if (motherId4 != -1) {
							CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);
							if (NULL != mother4) mcMotherPdg4 = mother4->GetPdgCode();
							grandmotherId4 = mother4->GetMotherId();
							if(grandmotherId4 != -1) {
								CbmMCTrack* grandmother4 = (CbmMCTrack*) fMcTracks->At(grandmotherId4);
								if (NULL != grandmother4) mcGrandmotherPdg4 = grandmother4->GetPdgCode();
							}
						}
					
					
						if(motherId1 == motherId2 && motherId2 == motherId3 && motherId3 == motherId4) {
							Double_t invmass = Invmass_4particles(fRecoTracklist[i], fRecoTracklist[j], fRecoTracklist[k], fRecoTracklist[l]);
							fhInvariantMassReco_pi0->Fill(invmass);
					//		cout << "Decay pi0 -> e+ e- e+ e- detected!" << endl;
						}
						
						if(motherId1 == motherId2 && motherId1 == motherId3) {
						
						}
						if(motherId1 == motherId2 && motherId1 == motherId4) {
						
						}
						if(motherId1 == motherId3 && motherId1 == motherId4) {
						
						}
						if(motherId2 == motherId3 && motherId2 == motherId4) {
							
						}
						
					//	if(  ((motherId1 == motherId2 && motherId3 == motherId4) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg3 == 22 || mcMotherPdg3 == 111))
					//	  || ((motherId1 == motherId3 && motherId2 == motherId4) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg2 == 22 || mcMotherPdg2 == 111))
					//	  || ((motherId1 == motherId4 && motherId2 == motherId3) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg2 == 22 || mcMotherPdg2 == 111))) {
						if(  ((motherId1 == motherId2 && motherId3 == motherId4) && (mcMotherPdg1 == 22) && (mcMotherPdg3 == 22) && grandmotherId1 == grandmotherId3)
						  || ((motherId1 == motherId3 && motherId2 == motherId4) && (mcMotherPdg1 == 22) && (mcMotherPdg2 == 22) && grandmotherId1 == grandmotherId2)
						  || ((motherId1 == motherId4 && motherId2 == motherId3) && (mcMotherPdg1 == 22) && (mcMotherPdg2 == 22) && grandmotherId1 == grandmotherId2)) {
							Double_t invmass = Invmass_4particles(fRecoTracklist[i], fRecoTracklist[j], fRecoTracklist[k], fRecoTracklist[l]);
							fhInvariantMassReco_pi0->Fill(invmass);
					//		cout << "Decay pi0 -> 2gamma -> e+ e-! \t MotherId " << motherId1 << "\t" << motherId2 << "\t" << motherId3 << "\t" << motherId4 <<
					//			 "\t GrandmotherId " << grandmotherId1 << "\t" << grandmotherId2 << "\t" << grandmotherId3 << "\t" << grandmotherId4 << endl;
						
						}
					}
				}
			}
		}
	}
	cout << "InvariantMassTestReco - End!" << endl;
}






int CbmAnaConversion::GetTest()
{
	return testint;
}


int CbmAnaConversion::GetNofEvents()
{
	return fEventNum;
}


void CbmAnaConversion::SetMode(int mode = 0)
	// mode 1 = tomography
	// mode 2 = urqmd
	// mode 3 = pluto
{
	fAnalyseMode = mode;
}



ClassImp(CbmAnaConversion)


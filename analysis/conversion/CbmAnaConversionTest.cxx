/**
 * \file CbmAnaConversionTest.cxx
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2015
 **/

#include "CbmAnaConversionTest.h"

#include "CbmMCTrack.h"
#include "FairRootManager.h"
#include "CbmRichPoint.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"
#include "CbmRichRing.h"
#include "CbmTrackMatchNew.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"
#include "../../littrack/cbm/elid/CbmLitGlobalElectronId.h"

#include <algorithm>
#include <map>



using namespace std;



CbmAnaConversionTest::CbmAnaConversionTest()
  : fRichPoints(NULL),
    fRichRings(NULL),
    fRichRingMatches(NULL),
    fMcTracks(NULL),
    fStsTracks(NULL),
    fStsTrackMatches(NULL),
    fGlobalTracks(NULL),
    fPrimVertex(NULL),
    fKFVertex(),
    fHistoList_test(),
    electronidentifier(NULL),
    fElectrons_gtid(),
	fElectrons_mcid(),
	fElectrons_richInd(),
	fElectrons_pi0mcid(),
	fElectrons_same(),
	fElectrons_nofPerPi0(NULL),
	fElectrons_nofPerPi0_withRichInd(NULL),
	fhElectronsTest_invmass(NULL),
	fhTest_ParticlesPerEvent(NULL),
	fhTest_RICHelectronsPerEvent(NULL),
	fhTest_PhotonsPerEvent_RICHonly(NULL),
	fhTest_PhotonsPerEvent_STSandRICH(NULL),
	fhTest_ReconstructedPi0PerEvent(NULL),
	fhTest_invmass(NULL),
	fVector_gt(),
	fVector_momenta(),
	fVector_chi(),
	fVector_gtIndex(),
	fVector_reconstructedPhotons_FromSTSandRICH(),
	fVector_electronRICH_gt(),
	fVector_electronRICH_momenta(),
	fVector_electronRICH_reconstructedPhotons(),
	fVector_electronRICH_gt_erased(),
	fVector_electronRICH_momenta_erased()
{
}

CbmAnaConversionTest::~CbmAnaConversionTest()
{
}


void CbmAnaConversionTest::Init()
{
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

	fPrimVertex = (CbmVertex*) ioman->GetObject("PrimaryVertex");
	if (NULL == fPrimVertex) { Fatal("CbmAnaConversion::Init","No PrimaryVertex array!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if (NULL == fRichRings) { Fatal("CbmAnaConversion::Init","No RichRing array!"); }

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if (NULL == fRichRingMatches) { Fatal("CbmAnaConversion::Init","No RichRingMatch array!"); }

	InitHistos();
	electronidentifier = new CbmLitGlobalElectronId();
	electronidentifier->Init();
}


void CbmAnaConversionTest::InitHistos()
{
	fHistoList_test.clear();

	fElectrons_nofPerPi0 = new TH1I("fElectrons_nofPerPi0", "fElectrons_nofPerPi0; nof; #", 7, -0.5, 6.5);
	fHistoList_test.push_back(fElectrons_nofPerPi0);
	fElectrons_nofPerPi0_withRichInd = new TH1I("fElectrons_nofPerPi0_withRichInd", "fElectrons_nofPerPi0_withRichInd; nof; #", 7, -0.5, 6.5);
	fHistoList_test.push_back(fElectrons_nofPerPi0_withRichInd);

	fhElectronsTest_invmass = new TH1D("fhElectronsTest_invmass", "fhElectronsTest_invmass; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_test.push_back(fhElectronsTest_invmass);


	fhTest_PhotonsPerEvent_RICHonly = new TH1I("fhTest_PhotonsPerEvent_RICHonly", "fhTest_PhotonsPerEvent_RICHonly; nof; #", 501, -0.5, 500.5);
	fHistoList_test.push_back(fhTest_PhotonsPerEvent_RICHonly);
	fhTest_PhotonsPerEvent_STSandRICH = new TH1I("fhTest_PhotonsPerEvent_STSandRICH", "fhTest_PhotonsPerEvent_STSandRICH; nof; #", 501, -0.5, 500.5);
	fHistoList_test.push_back(fhTest_PhotonsPerEvent_STSandRICH);
	fhTest_ParticlesPerEvent = new TH1I("fhTest_ParticlesPerEvent", "fhTest_ParticlesPerEvent; nof; #", 1001, -0.5, 1000.5);
	fHistoList_test.push_back(fhTest_ParticlesPerEvent);
	fhTest_ReconstructedPi0PerEvent = new TH1I("fhTest_ReconstructedPi0PerEvent", "fhTest_ReconstructedPi0PerEvent; nof; #", 101, -0.5, 100.5);
	fHistoList_test.push_back(fhTest_ReconstructedPi0PerEvent);
	fhTest_RICHelectronsPerEvent = new TH1I("fhTest_RICHelectronsPerEvent", "fhTest_RICHelectronsPerEvent; nof; #", 501, -0.5, 500.5);
	fHistoList_test.push_back(fhTest_RICHelectronsPerEvent);
	fhTest_invmass = new TH1D("fhTest_invmass", "fhTest_invmass; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_test.push_back(fhTest_invmass);
}







void CbmAnaConversionTest::Finish()
{
	//gDirectory->cd("analysis-conversion");
	gDirectory->mkdir("Test");
	gDirectory->cd("Test");
	for (UInt_t i = 0; i < fHistoList_test.size(); i++){
		fHistoList_test[i]->Write();
	}
	gDirectory->cd("..");
}




void CbmAnaConversionTest::Exec()
{
	cout << "CbmAnaConversionTest: Exec()" << endl;

	if (fPrimVertex != NULL){
		fKFVertex = CbmKFVertex(*fPrimVertex);
	} else {
		Fatal("CbmAnaConversionTest::Exec","No PrimaryVertex array!");
	}

	fElectrons_mcid.clear();
	fElectrons_gtid.clear();
	fElectrons_richInd.clear();
	fElectrons_pi0mcid.clear();
	fElectrons_same.clear();


	Int_t nGTracks = fGlobalTracks->GetEntriesFast();
	for (Int_t i = 0; i < nGTracks; i++){
		CbmGlobalTrack* gTrack  = (CbmGlobalTrack*) fGlobalTracks->At(i);
		if(NULL == gTrack) continue;
		int stsInd = gTrack->GetStsTrackIndex();
		int richInd = gTrack->GetRichRingIndex();
		if (stsInd < 0) continue;
		if (richInd < 0) continue;
		
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
		if (stsTrack == NULL) continue;
		
		CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*)fStsTrackMatches->At(stsInd);
		if (stsMatch == NULL) continue;
		int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
		if (stsMcTrackId < 0) continue;
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
		if (mcTrack1 == NULL) continue;

		CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(richInd);
		if (richMatch == NULL) continue;
		int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
		if (richMcTrackId < 0) continue;
		CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
		if (mcTrack2 == NULL) continue;

		int pdg = TMath::Abs(mcTrack1->GetPdgCode());
		int motherId = mcTrack1->GetMotherId();
		
		if(motherId < 0) continue;
		CbmMCTrack* mcTrack1mother = (CbmMCTrack*) fMcTracks->At(motherId);
		if (mcTrack1mother == NULL) continue;
		int grandmotherId = mcTrack1mother->GetMotherId();
		if(grandmotherId < 0) continue;
		CbmMCTrack* mcTrack1grandmother = (CbmMCTrack*) fMcTracks->At(grandmotherId);
		if (mcTrack1grandmother == NULL) continue;
		
		int pdg_mother = TMath::Abs(mcTrack1mother->GetPdgCode());
		int pdg_grandmother = TMath::Abs(mcTrack1grandmother->GetPdgCode());
		
		cout << "CbmAnaConversionTest: pdg particle/mother/grandmother: " << pdg << " / " << pdg_mother << " / " << pdg_grandmother << endl;

		Int_t sameRichSts = (stsMcTrackId == richMcTrackId);

		if(TMath::Abs(pdg) == 11 && (pdg_mother == 111 || pdg_grandmother == 111) ) {
			fElectrons_richInd.push_back(richInd);
			fElectrons_gtid.push_back(i);
			fElectrons_mcid.push_back(stsMcTrackId);
			if(pdg_mother == 111) fElectrons_pi0mcid.push_back(motherId);
			if(pdg_grandmother == 111) fElectrons_pi0mcid.push_back(grandmotherId);
			fElectrons_same.push_back(sameRichSts);
		}

	}
	
	GetNofRichElectrons();
	
	
	DoSTSonlyAnalysis();
}



void CbmAnaConversionTest::GetNofRichElectrons()
{

		std::multimap<int,int> electronMap;
		for(unsigned int i=0; i<fElectrons_pi0mcid.size(); i++) {
			electronMap.insert ( std::pair<int,int>(fElectrons_pi0mcid[i], i) );
		}

		int check = 0;
		int nofRich = 0;
		for(std::map<int,int>::iterator it=electronMap.begin(); it!=electronMap.end(); ++it) {
			if(it == electronMap.begin()) {
				check = 1;
				if(fElectrons_same[it->second] > 0) nofRich++;
			}
			if(it != electronMap.begin()) {
				std::map<int,int>::iterator zwischen = it;
				zwischen--;
				int id = it->first;
				int id_old = zwischen->first;
				if(id == id_old) {
					check++;
					if(fElectrons_same[it->second] > 0) nofRich++;
					if(check > 3) {
						std::map<int,int>::iterator zwischen2 = zwischen--;
						std::map<int,int>::iterator zwischen3 = zwischen2--;
						Double_t invmass = CalcInvMass(it->second, zwischen->second, zwischen2->second, zwischen3->second);
						if(invmass > 0) fhElectronsTest_invmass->Fill(invmass);
						if(nofRich == 4) {
						
						
						}
					}
				}
				else {
					fElectrons_nofPerPi0->Fill(check);
					fElectrons_nofPerPi0_withRichInd->Fill(nofRich);
					check=1;
					nofRich = 0;
					if(fElectrons_same[it->second] > 0) nofRich++;
				}
			}
		}
}



Double_t CbmAnaConversionTest::CalcInvMass(Int_t e1, Int_t e2, Int_t e3, Int_t e4)
{
	
	CbmMCTrack* mcTrack_e1 = (CbmMCTrack*) fMcTracks->At(fElectrons_mcid[e1]);
	CbmMCTrack* mcTrack_e2 = (CbmMCTrack*) fMcTracks->At(fElectrons_mcid[e2]);
	CbmMCTrack* mcTrack_e3 = (CbmMCTrack*) fMcTracks->At(fElectrons_mcid[e3]);
	CbmMCTrack* mcTrack_e4 = (CbmMCTrack*) fMcTracks->At(fElectrons_mcid[e4]);
	
	if(mcTrack_e1->GetPdgCode() + mcTrack_e2->GetPdgCode() + mcTrack_e3->GetPdgCode() + mcTrack_e4->GetPdgCode() ) return -1;

	TLorentzVector lorentz1;
	mcTrack_e1->Get4Momentum(lorentz1);
	TLorentzVector lorentz2;
	mcTrack_e2->Get4Momentum(lorentz2);
	TLorentzVector lorentz3;
	mcTrack_e3->Get4Momentum(lorentz3);
	TLorentzVector lorentz4;
	mcTrack_e4->Get4Momentum(lorentz4);

	TLorentzVector sum;
	sum = lorentz1 + lorentz2 + lorentz3 + lorentz4;
	
	return sum.Mag();
}




void CbmAnaConversionTest::DoSTSonlyAnalysis()
{
	fVector_gt.clear();
	fVector_momenta.clear();
	fVector_chi.clear();
	fVector_gtIndex.clear();
	fVector_reconstructedPhotons_FromSTSandRICH.clear();
	
	fVector_electronRICH_momenta.clear();
	fVector_electronRICH_gt.clear();
	fVector_electronRICH_momenta_erased.clear();
	fVector_electronRICH_gt_erased.clear();
	fVector_electronRICH_reconstructedPhotons.clear();

	Int_t nofRICHelectrons = 0;

	Int_t ngTracks = fGlobalTracks->GetEntriesFast();
	fhTest_ParticlesPerEvent->Fill(ngTracks);
	for (Int_t i = 0; i < ngTracks; i++) {
		CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(i);
		if(NULL == gTrack) continue;
		int stsInd = gTrack->GetStsTrackIndex();
		int richInd = gTrack->GetRichRingIndex();
//		int trdInd = gTrack->GetTrdTrackIndex();
//		int tofInd = gTrack->GetTofHitIndex();

		if (stsInd < 0) continue;
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
		if (stsTrack == NULL) continue;
		CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*) fStsTrackMatches->At(stsInd);
		if (stsMatch == NULL) continue;
		if(stsMatch->GetNofLinks() <= 0) continue;
		int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
		if (stsMcTrackId < 0) continue;
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
		if (mcTrack1 == NULL) continue;



		// calculate refitted momenta at primary vertex
		TVector3 refittedMomentum;
		CbmL1PFFitter fPFFitter;
		vector<CbmStsTrack> stsTracks;
		stsTracks.resize(1);
		stsTracks[0] = *stsTrack;
		vector<L1FieldRegion> vField;
		vector<float> chiPrim;
		fPFFitter.GetChiToVertex(stsTracks, vField, chiPrim, fKFVertex, 3e6);
		//cand.chi2sts = stsTracks[0].GetChiSq() / stsTracks[0].GetNDF();
		//cand.chi2Prim = chiPrim[0];
		const FairTrackParam* vtxTrack = stsTracks[0].GetParamFirst();
		vtxTrack->Momentum(refittedMomentum);
		float result_chi = chiPrim[0];



		// Doing refit of momenta with electron assumption
		CbmL1PFFitter fPFFitter_electron;
		vector<CbmStsTrack> stsTracks_electron;
		stsTracks_electron.resize(1);
		stsTracks_electron[0] = *stsTrack;
		vector<L1FieldRegion> vField_electron;
		vector<float> chiPrim_electron;
		vector<int> pidHypo_electron;
		pidHypo_electron.push_back(11);
		fPFFitter_electron.Fit(stsTracks_electron, pidHypo_electron); 
		fPFFitter_electron.GetChiToVertex(stsTracks_electron, vField_electron, chiPrim_electron, fKFVertex, 3e6);

		TVector3 refittedMomentum_electron;
		const FairTrackParam* vtxTrack_electron = stsTracks_electron[0].GetParamFirst();
		vtxTrack_electron->Momentum(refittedMomentum_electron);
		float result_chi_electron = chiPrim_electron[0];
		float result_ndf_electron = stsTracks_electron[0].GetNDF();
		Double_t stsHits = stsTrack->GetNofHits();
		
		
		Double_t chiCut = 0;
		if(refittedMomentum_electron.Perp() < 0.4) {
			chiCut = 31. - 70.*refittedMomentum_electron.Perp();
		}
		if(refittedMomentum_electron.Perp() >= 0.4) {
			chiCut = 3;
		}
		
		if(result_chi_electron > chiCut) continue;

		if(stsHits > 10) {
			fVector_momenta.push_back(refittedMomentum_electron);
			fVector_chi.push_back(result_chi_electron);
			fVector_gtIndex.push_back(i);
			fVector_gt.push_back(gTrack);
		}


		if (richInd < 0) continue;
		CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(richInd);
		if (richMatch == NULL) continue;
		int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
		if (richMcTrackId < 0) continue;
		CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
		if (mcTrack2 == NULL) continue;

		Bool_t electron_rich = electronidentifier->IsRichElectron(i, refittedMomentum_electron.Mag());
		if(electron_rich) {
			fVector_electronRICH_momenta.push_back(refittedMomentum_electron);
			fVector_electronRICH_gt.push_back(gTrack);
			//
			fVector_electronRICH_momenta_erased.push_back(refittedMomentum_electron);
			fVector_electronRICH_gt_erased.push_back(gTrack);
			nofRICHelectrons++;
		}

	}

	fhTest_RICHelectronsPerEvent->Fill(nofRICHelectrons);

	CombineElectrons_FromRICH();
	CombineElectrons_FromSTSandRICH();
	CombinePhotons();

}



void CbmAnaConversionTest::CombineElectrons_FromSTSandRICH()
{
	Int_t nof_sts = fVector_momenta.size();
	Int_t nof_rich = fVector_electronRICH_momenta_erased.size();
	cout << "CbmAnaConversionTest: CombineElectrons_FromSTSandRICH, nof sts/rich - " << nof_sts << " / " << nof_rich << endl;
	Int_t nofPhotons = 0;
	if(nof_sts + nof_rich >= 2) {
		for(int a=0; a<nof_sts; a++) {
			for(int b=0; b<nof_rich; b++) {
				Int_t check1 = (fVector_gt[a]->GetParamLast()->GetQp() > 0);	// positive or negative charge (qp = charge over momentum ratio)
				Int_t check2 = (fVector_electronRICH_gt_erased[b]->GetParamLast()->GetQp() > 0);
				Int_t test = check1 + check2;
				if(test != 1) continue;		// need one electron and one positron
				//if(fElectrons_momentaChi[a] > 10 || fElectrons_momentaChi[b] > 10) continue;
				
				CbmLmvmKinematicParams params1 = CalculateKinematicParamsReco(fVector_momenta[a], fVector_electronRICH_momenta_erased[b]);
				
				// standard fixed opening angle cut
				//Double_t openingAngleCut = 1;
				
				// opening angle cut depending on pt of e+e- pair
				//Double_t openingAngleCut = 1.8 - 0.6 * params1.fPt;
				Double_t openingAngleCut = 1.5 - 0.5 * params1.fPt;
				
				Double_t invMassCut = 0.03;
				
				Int_t IsPhoton_openingAngle1	= (params1.fAngle < openingAngleCut);
				Int_t IsPhoton_invMass1			= (params1.fMinv < invMassCut);
				
				
				if(IsPhoton_openingAngle1 && IsPhoton_invMass1) {
					nofPhotons++;
					vector<int> pair; // = {a, b};
					pair.push_back(a);
					pair.push_back(b);
					fVector_reconstructedPhotons_FromSTSandRICH.push_back(pair);
					//fhElectrons_invmass_cut->Fill(params1.fMinv);
					
				}
			}
		}
	}
	fhTest_PhotonsPerEvent_STSandRICH->Fill(nofPhotons);
	cout << "CbmAnaConversionTest: CombineElectrons: Crosscheck - nof reconstructed photons: " << nofPhotons << endl;
}



void CbmAnaConversionTest::CombinePhotons()
{
	Int_t nof_STSandRICH	= fVector_reconstructedPhotons_FromSTSandRICH.size();
	Int_t nof_RICH			= fVector_electronRICH_reconstructedPhotons.size();
	cout << "CbmAnaConversionTest: CombinePhotons, nof - " << nof_STSandRICH << endl;
	Int_t nofPi0 = 0;
	if(nof_STSandRICH + nof_RICH >= 2) {
		for(int a=0; a<nof_STSandRICH; a++) {
			for(int b=0; b<nof_RICH; b++) {
				Int_t electron11 = fVector_reconstructedPhotons_FromSTSandRICH[a][0];
				Int_t electron12 = fVector_reconstructedPhotons_FromSTSandRICH[a][1];
				Int_t electron21 = fVector_electronRICH_reconstructedPhotons[b][0];
				Int_t electron22 = fVector_electronRICH_reconstructedPhotons[b][1];
			
				if(electron12 == electron21 || electron12 == electron22) continue;
			
				Double_t invmass = Invmass_4particlesRECO(fVector_momenta[electron11], fVector_electronRICH_momenta_erased[electron12], fVector_electronRICH_momenta[electron21], fVector_electronRICH_momenta[electron22]);
				
				fhTest_invmass->Fill(invmass);
				
				//Double_t pt = Pt_4particlesRECO(momenta[electron11], momenta[electron12], momenta[electron21], momenta[electron22]);
				//Double_t rap = Rap_4particlesRECO(momenta[electron11], momenta[electron12], momenta[electron21], momenta[electron22]);
				
				//Double_t opening_angle = OpeningAngleBetweenPhotons(momenta, reconstructedPhotons[a], reconstructedPhotons[b]);
				
				
				//CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(momenta[electron11], momenta[electron12], momenta[electron21], momenta[electron22]);
				
				nofPi0++;
			}
		}
	}
	fhTest_ReconstructedPi0PerEvent->Fill(nofPi0);
}







void CbmAnaConversionTest::CombineElectrons_FromRICH()
{
	Int_t nof = fVector_electronRICH_momenta.size();
	cout << "CbmAnaConversionTest: CombineElectronsFromRICH, nof - " << nof << endl;
	Int_t nofPhotons = 0;
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t check1 = (fVector_electronRICH_gt[a]->GetParamLast()->GetQp() > 0);	// positive or negative charge (qp = charge over momentum ratio)
				Int_t check2 = (fVector_electronRICH_gt[b]->GetParamLast()->GetQp() > 0);
				Int_t test = check1 + check2;
				if(test != 1) continue;		// need one electron and one positron
				//if(fElectrons_momentaChi[a] > 10 || fElectrons_momentaChi[b] > 10) continue;
				
				CbmLmvmKinematicParams params1 = CalculateKinematicParamsReco(fVector_electronRICH_momenta[a], fVector_electronRICH_momenta[b]);
				
				// standard fixed opening angle cut
				//Double_t openingAngleCut = 1;
				
				// opening angle cut depending on pt of e+e- pair
				//Double_t openingAngleCut = 1.8 - 0.6 * params1.fPt;
				Double_t openingAngleCut = 1.5 - 0.5 * params1.fPt;
				
				Double_t invMassCut = 0.03;
				
				Int_t IsPhoton_openingAngle1	= (params1.fAngle < openingAngleCut);
				Int_t IsPhoton_invMass1			= (params1.fMinv < invMassCut);
				
				
				if(IsPhoton_openingAngle1 && IsPhoton_invMass1) {
					nofPhotons++;
					vector<int> pair; // = {a, b};
					pair.push_back(a);
					pair.push_back(b);
					fVector_electronRICH_reconstructedPhotons.push_back(pair);
					//fhElectrons_invmass_cut->Fill(params1.fMinv);
					
					fVector_electronRICH_momenta_erased.erase(fVector_electronRICH_momenta_erased.begin() + a - 1);
					fVector_electronRICH_momenta_erased.erase(fVector_electronRICH_momenta_erased.begin() + b - 1);
					
				}
			}
		}
	}
	fhTest_PhotonsPerEvent_RICHonly->Fill(nofPhotons);
	cout << "CbmAnaConversionTest: CombineElectrons: Crosscheck - nof reconstructed photons: " << nofPhotons << endl;
}


















CbmLmvmKinematicParams CbmAnaConversionTest::CalculateKinematicParamsReco(const TVector3 electron1, const TVector3 electron2)
{
	CbmLmvmKinematicParams params;

    Double_t energyP = TMath::Sqrt(electron1.Mag2() + M2E);
    TLorentzVector lorVecP(electron1, energyP);

    Double_t energyM = TMath::Sqrt(electron2.Mag2() + M2E);
    TLorentzVector lorVecM(electron2, energyM);

    TVector3 momPair = electron1 + electron2;
    Double_t energyPair = energyP + energyM;
    Double_t ptPair = momPair.Perp();
    Double_t pzPair = momPair.Pz();
    Double_t yPair = 0.5*TMath::Log((energyPair+pzPair)/(energyPair-pzPair));
    Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
    Double_t theta = 180.*anglePair/TMath::Pi();
    Double_t minv = 2.*TMath::Sin(anglePair / 2.)*TMath::Sqrt(electron1.Mag()*electron2.Mag());

    params.fMomentumMag = momPair.Mag();
    params.fPt = ptPair;
    params.fRapidity = yPair;
    params.fMinv = minv;
    params.fAngle = theta;
    return params;
}



Double_t CbmAnaConversionTest::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
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
    
    TLorentzVector sum;
    sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;    

	return sum.Mag();
}


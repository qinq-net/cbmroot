/**
 * \file CbmAnaConversionRecoFull.cxx
 *
 * 
 * 
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2014
 **/

#include "CbmAnaConversionRecoFull.h"

// standard includes
#include <iostream>

// includes from ROOT
#include "TRandom3.h"

// included from CbmRoot
#include "FairRootManager.h"
#include "CbmMCTrack.h"
#include "FairRootManager.h"
#include "CbmRichPoint.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"
#include "CbmRichRing.h"
#include "CbmTrackMatchNew.h"
#include "CbmDrawHist.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"
#include "../../littrack/cbm/elid/CbmLitGlobalElectronId.h"


#define M2E 2.6112004954086e-7
using namespace std;



CbmAnaConversionRecoFull::CbmAnaConversionRecoFull()
  : fRichPoints(NULL),
    fRichRings(NULL),
    fRichRingMatches(NULL),
    fMcTracks(NULL),
    fStsTracks(NULL),
    fStsTrackMatches(NULL),
    fGlobalTracks(NULL),
    fPrimVertex(NULL),
    fKFVertex(),
    fHistoList_recofull(),
    fhElectrons(NULL),
    electronidentifier(NULL),
    fhMomentumFits(NULL),
	fhMomentumFits_electronRich(NULL),
	fhMomentumFits_pi0reco(NULL),
    fElectrons_track(),
    fElectrons_momenta(),
	fElectrons_momentaChi(),
	fElectrons_mctrackID(),
    fhElectrons_invmass(NULL),
    fhElectrons_invmass_cut(NULL),
    fVector_photons_pairs(),
	fVector_photons_momenta(),
    fhPhotons_invmass(NULL),
	fhPhotons_invmass_cut(NULL),
	fhPhotons_invmass_cut_chi1(NULL),
	fhPhotons_invmass_cut_chi3(NULL),
	fhPhotons_invmass_cut_chi5(NULL),
	fhPhotons_invmass_cut_chi10(NULL),
	fhPhotons_invmass_cut_chi25(NULL),
	fhPhotons_invmass_cut_chi40(NULL),
	fhPhotons_invmass_cut_chi65(NULL),
	fhPhotons_invmass_cut_chi80(NULL),
	fhPhotons_invmass_vs_chi(NULL),
	fhPhotons_startvertex_vs_chi(NULL),
	fhPhotons_angleBetween(NULL),
	fhPhotons_MC_motherpdg(NULL),
	fhPhotons_MC_invmass1(NULL),
	fhPhotons_MC_invmass2(NULL),
	fhPhotons_MC_invmass3(NULL),
	fhPhotons_MC_invmass4(NULL),
	fhPhotons_MC_startvertexZ(NULL),
	fhPhotons_MC_motherIdCut(NULL),
    timer(),
    fTime(0.)
{
}

CbmAnaConversionRecoFull::~CbmAnaConversionRecoFull()
{
}


void CbmAnaConversionRecoFull::Init()
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



void CbmAnaConversionRecoFull::InitHistos()
{
	fHistoList_recofull.clear();

	fhElectrons = new TH1I("fhElectrons", "fhElectrons;; #", 8, 0., 8.);
	fHistoList_recofull.push_back(fhElectrons);
	fhElectrons->GetXaxis()->SetBinLabel(1, "electrons rich");
	fhElectrons->GetXaxis()->SetBinLabel(2, "electrons trd");
	fhElectrons->GetXaxis()->SetBinLabel(3, "electrons tof");
	fhElectrons->GetXaxis()->SetBinLabel(4, "electrons rich+trd");
	fhElectrons->GetXaxis()->SetBinLabel(5, "electrons rich+tof");
	fhElectrons->GetXaxis()->SetBinLabel(6, "electrons trd+tof");
	fhElectrons->GetXaxis()->SetBinLabel(7, "electrons rich+trd+tof");
	fhElectrons->GetXaxis()->SetBinLabel(8, "electrons 2 out of 3");


	fhMomentumFits = new TH1D("fhMomentumFits", "fhMomentumFits; chi; #", 10000, 0., 1000.);
	fHistoList_recofull.push_back(fhMomentumFits);
	fhMomentumFits_electronRich = new TH1D("fhMomentumFits_electronRich", "fhMomentumFits_electronRich; chi; #", 10000, 0., 1000.);
	fHistoList_recofull.push_back(fhMomentumFits_electronRich);
	fhMomentumFits_pi0reco = new TH1D("fhMomentumFits_pi0reco", "fhMomentumFits_pi0reco; chi; #", 10000, 0., 1000.);
	fHistoList_recofull.push_back(fhMomentumFits_pi0reco);


	fhElectrons_invmass = new TH1D("fhElectrons_invmass", "fhElectrons_invmass; invariant mass; #", 600, -0.0025, 2.9975);
	fhElectrons_invmass_cut = new TH1D("fhElectrons_invmass_cut", "fhElectrons_invmass_cut; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhElectrons_invmass);
	fHistoList_recofull.push_back(fhElectrons_invmass_cut);
	
	fhPhotons_invmass = new TH1D("fhPhotons_invmass", "fhPhotons_invmass; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass);
	fhPhotons_invmass_cut = new TH1D("fhPhotons_invmass_cut", "fhPhotons_invmass_cut; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut);
	fhPhotons_angleBetween = new TH1D("fhPhotons_angleBetween", "fhPhotons_angleBetween; opening angle [deg]; #", 1000, 0, 100);
	fHistoList_recofull.push_back(fhPhotons_angleBetween);


	fhPhotons_invmass_cut_chi1 = new TH1D("fhPhotons_invmass_cut_chi1", "fhPhotons_invmass_cut_chi1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi1);
	fhPhotons_invmass_cut_chi3 = new TH1D("fhPhotons_invmass_cut_chi3", "fhPhotons_invmass_cut_chi3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi3);
	fhPhotons_invmass_cut_chi5 = new TH1D("fhPhotons_invmass_cut_chi5", "fhPhotons_invmass_cut_chi5; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi5);
	fhPhotons_invmass_cut_chi10 = new TH1D("fhPhotons_invmass_cut_chi10", "fhPhotons_invmass_cut_chi10; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi10);
	fhPhotons_invmass_cut_chi25 = new TH1D("fhPhotons_invmass_cut_chi25", "fhPhotons_invmass_cut_chi25; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi25);
	fhPhotons_invmass_cut_chi40 = new TH1D("fhPhotons_invmass_cut_chi40", "fhPhotons_invmass_cut_chi40; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi40);
	fhPhotons_invmass_cut_chi65 = new TH1D("fhPhotons_invmass_cut_chi65", "fhPhotons_invmass_cut_chi65; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi65);
	fhPhotons_invmass_cut_chi80 = new TH1D("fhPhotons_invmass_cut_chi80", "fhPhotons_invmass_cut_chi80; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi80);
	
	fhPhotons_invmass_vs_chi = new TH2D("fhPhotons_invmass_vs_chi", "fhPhotons_invmass_vs_chi; invariant mass; chi", 600, -0.0025, 2.9975, 1000, 0., 100.);
	fHistoList_recofull.push_back(fhPhotons_invmass_vs_chi);
	fhPhotons_startvertex_vs_chi = new TH2D("fhPhotons_startvertex_vs_chi", "fhPhotons_startvertex_vs_chi; startvertex; chi", 420, -5., 100., 1000, 0., 200.);
	fHistoList_recofull.push_back(fhPhotons_startvertex_vs_chi);

	fhPhotons_MC_motherpdg = new TH1D("fhPhotons_MC_motherpdg", "fhPhotons_MC_motherpdg; pdg; #", 1000, 0., 1000.);
	fHistoList_recofull.push_back(fhPhotons_MC_motherpdg);
	fhPhotons_MC_invmass1 = new TH1D("fhPhotons_MC_invmass1", "fhPhotons_MC_minvmass1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_invmass1);
	fhPhotons_MC_invmass2 = new TH1D("fhPhotons_MC_invmass2", "fhPhotons_MC_minvmass2; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_invmass2);
	fhPhotons_MC_invmass3 = new TH1D("fhPhotons_MC_invmass3", "fhPhotons_MC_minvmass3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_invmass3);
	fhPhotons_MC_invmass4 = new TH1D("fhPhotons_MC_invmass4", "fhPhotons_MC_minvmass4; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_invmass4);

	fhPhotons_MC_startvertexZ = new TH1D("fhPhotons_MC_startvertexZ", "fhPhotons_MC_startvertexZ; startvertexZ [cm]; #", 1000, 0., 100.);
	fHistoList_recofull.push_back(fhPhotons_MC_startvertexZ);
	
	fhPhotons_MC_motherIdCut = new TH1D("fhPhotons_MC_motherIdCut", "fhPhotons_MC_motherIdCut; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_motherIdCut);

}


void CbmAnaConversionRecoFull::Finish()
{
	gDirectory->mkdir("RecoFull");
	gDirectory->cd("RecoFull");
	for (Int_t i = 0; i < fHistoList_recofull.size(); i++){
		fHistoList_recofull[i]->Write();
	}
	gDirectory->cd("..");


	cout << "CbmAnaConversionRecoFull: Realtime - " << fTime << endl;


}

void CbmAnaConversionRecoFull::Exec()
{
	timer.Start();


	if (fPrimVertex != NULL){
		fKFVertex = CbmKFVertex(*fPrimVertex);
	} else {
		Fatal("CbmAnaConversion::Exec","No PrimaryVertex array!");
	}


	fElectrons_track.clear();
	fElectrons_momenta.clear();
	fElectrons_momentaChi.clear();
	fElectrons_mctrackID.clear();
	fVector_photons_pairs.clear();

	Int_t nofGT_richsts = 0;

	// everything related to reconstructed data
	Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
	cout << "CbmAnaConversionRecoFull: number of global tracks " << nofGlobalTracks << endl;
	for (int iG = 0; iG < nofGlobalTracks; iG++){
		CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(iG);
		if(NULL == gTrack) continue;
		int stsInd = gTrack->GetStsTrackIndex();
		int richInd = gTrack->GetRichRingIndex();
		if (richInd < 0) continue;
		if (stsInd < 0) continue;
		
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

		if(stsMcTrackId != richMcTrackId) continue;
		nofGT_richsts++;

		int pdg = TMath::Abs(mcTrack1->GetPdgCode());


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
		fhMomentumFits->Fill(result_chi);

		Int_t pidHypo = gTrack->GetPidHypo();

		Bool_t electron_rich = electronidentifier->IsRichElectron(iG, refittedMomentum.Mag());
		Bool_t electron_trd = electronidentifier->IsTrdElectron(iG, refittedMomentum.Mag());
		Bool_t electron_tof = electronidentifier->IsTofElectron(iG, refittedMomentum.Mag());

		//cout << "CbmAnaConversionRecoFull: electron rich/trd/tof = " << electron_rich << "/" << electron_trd << "/" << electron_tof << endl;

		if(electron_rich) fhElectrons->Fill(0);
		if(electron_trd) fhElectrons->Fill(1);
		if(electron_tof) fhElectrons->Fill(2);
		if(electron_rich && electron_trd) fhElectrons->Fill(3);
		if(electron_rich && electron_tof) fhElectrons->Fill(4);
		if(electron_trd && electron_tof) fhElectrons->Fill(5);
		if(electron_rich && electron_trd && electron_tof) fhElectrons->Fill(6);
		if( (electron_rich && electron_trd) || (electron_rich && electron_tof) || (electron_trd && electron_tof) ) fhElectrons->Fill(7);


		if(electron_rich) {
			fElectrons_track.push_back(gTrack);
			fElectrons_momenta.push_back(refittedMomentum);
			fElectrons_momentaChi.push_back(result_chi);
			fElectrons_mctrackID.push_back(richMcTrackId);
			fhMomentumFits_electronRich->Fill(result_chi);
			TVector3 startvertex;
			mcTrack2->GetStartVertex(startvertex);
			fhPhotons_startvertex_vs_chi->Fill(startvertex.Z(), result_chi);
		}
	}
	cout << "CbmAnaConversionRecoFull: number of global tracks in STS and RICH " << nofGT_richsts << endl;

	CombineElectrons();
	CombinePhotons();

	timer.Stop();
	fTime += timer.RealTime();
}



void CbmAnaConversionRecoFull::CombineElectrons()
{
	Int_t nof = fElectrons_momenta.size();
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t check1 = (fElectrons_track[a]->GetParamLast()->GetQp() > 0);	// positive or negative charge (qp = charge over momentum ratio)
				Int_t check2 = (fElectrons_track[b]->GetParamLast()->GetQp() > 0);
				Int_t test = check1 + check2;
				if(test != 1) continue;		// need one electron and one positron
				//if(fElectrons_momentaChi[a] > 10 || fElectrons_momentaChi[b] > 10) continue;
				
				CbmLmvmKinematicParams params1 = CalculateKinematicParamsReco(fElectrons_momenta[a], fElectrons_momenta[b]);
				
				Double_t openingAngleCut = 1;
				Double_t invMassCut = 0.03;
				
				Int_t IsPhoton_openingAngle1	= (params1.fAngle < openingAngleCut);
				Int_t IsPhoton_invMass1			= (params1.fMinv < invMassCut);
				
				if(IsPhoton_openingAngle1 && IsPhoton_invMass1) {
					vector<int> pair; // = {a, b};
					pair.push_back(a);
					pair.push_back(b);
					fVector_photons_pairs.push_back(pair);
					fhElectrons_invmass_cut->Fill(params1.fMinv);
					//fVector_photons_momenta.push_back(params1.momPair);
				}
			}
		}
	}
}


Double_t CbmAnaConversionRecoFull::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
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



CbmLmvmKinematicParams CbmAnaConversionRecoFull::CalculateKinematicParamsReco(const TVector3 electron1, const TVector3 electron2)
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




void CbmAnaConversionRecoFull::CombinePhotons()
{
	Int_t nof = fVector_photons_pairs.size();
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t electron11 = fVector_photons_pairs[a][0];
				Int_t electron12 = fVector_photons_pairs[a][1];
				Int_t electron21 = fVector_photons_pairs[b][0];
				Int_t electron22 = fVector_photons_pairs[b][1];
			
				Double_t invmass = Invmass_4particlesRECO(fElectrons_momenta[electron11], fElectrons_momenta[electron12], fElectrons_momenta[electron21], fElectrons_momenta[electron22]);
				fhPhotons_invmass->Fill(invmass);
				
				Double_t opening_angle = OpeningAngleBetweenPhotons(fVector_photons_pairs[a], fVector_photons_pairs[b]);
				fhPhotons_angleBetween->Fill(opening_angle);
				
				if(opening_angle < 8) {
					fhPhotons_invmass_cut->Fill(invmass);
					fhMomentumFits_pi0reco->Fill(fElectrons_momentaChi[electron11]);
					fhMomentumFits_pi0reco->Fill(fElectrons_momentaChi[electron12]);
					fhMomentumFits_pi0reco->Fill(fElectrons_momentaChi[electron21]);
					fhMomentumFits_pi0reco->Fill(fElectrons_momentaChi[electron22]);
					
					fhPhotons_invmass_vs_chi->Fill(invmass, fElectrons_momentaChi[electron11]);
					fhPhotons_invmass_vs_chi->Fill(invmass, fElectrons_momentaChi[electron12]);
					fhPhotons_invmass_vs_chi->Fill(invmass, fElectrons_momentaChi[electron21]);
					fhPhotons_invmass_vs_chi->Fill(invmass, fElectrons_momentaChi[electron22]);
					
					Double_t chicut = 1.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi1->Fill(invmass);
					}
					chicut = 3.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi3->Fill(invmass);
					}
					chicut = 5.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi5->Fill(invmass);
					}
					chicut = 10.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi10->Fill(invmass);
					}
					chicut = 25.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi25->Fill(invmass);
					}
					chicut = 40.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi40->Fill(invmass);
					}
					chicut = 65.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi65->Fill(invmass);
					}
					chicut = 80.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi80->Fill(invmass);
					}
					
					
					
					// CROSSCHECK WITH MC-TRUE DATA!
					CbmMCTrack* mctrack11 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron11]);
					CbmMCTrack* mctrack12 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron12]);
					CbmMCTrack* mctrack21 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron21]);
					CbmMCTrack* mctrack22 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron22]);
					
					Int_t motherId11 = mctrack11->GetMotherId();
					Int_t motherId12 = mctrack12->GetMotherId();
					Int_t motherId21 = mctrack21->GetMotherId();
					Int_t motherId22 = mctrack22->GetMotherId();
					
					TVector3 startvertex11;
					mctrack11->GetStartVertex(startvertex11);
					TVector3 startvertex12;
					mctrack12->GetStartVertex(startvertex12);
					TVector3 startvertex21;
					mctrack21->GetStartVertex(startvertex21);
					TVector3 startvertex22;
					mctrack22->GetStartVertex(startvertex22);
					
					fhPhotons_MC_startvertexZ->Fill(startvertex11.Z());
					fhPhotons_MC_startvertexZ->Fill(startvertex12.Z());
					fhPhotons_MC_startvertexZ->Fill(startvertex21.Z());
					fhPhotons_MC_startvertexZ->Fill(startvertex22.Z());
					
					if(motherId11 == motherId12 && motherId21 == motherId22) {
						fhPhotons_MC_invmass1->Fill(invmass);
						if(motherId11 != -1 && motherId21 != -1) {
							CbmMCTrack* mothermctrack11 = (CbmMCTrack*)fMcTracks->At(motherId11);
							CbmMCTrack* mothermctrack21 = (CbmMCTrack*)fMcTracks->At(motherId21);
							
							Int_t grandmotherId11 = mothermctrack11->GetMotherId();
							Int_t grandmotherId21 = mothermctrack21->GetMotherId();
							
							if(grandmotherId11 == grandmotherId21) {
								fhPhotons_MC_invmass2->Fill(invmass);
								if(grandmotherId11 == -1) continue;
								CbmMCTrack* pi0mctrack11 = (CbmMCTrack*)fMcTracks->At(grandmotherId11);
								if(pi0mctrack11->GetMotherId() == -1) {
									fhPhotons_MC_motherIdCut->Fill(invmass);
								}
							}
							if(grandmotherId11 != grandmotherId21) {
								fhPhotons_MC_invmass3->Fill(invmass);
							}
						}
					}
					
					if(motherId11 != motherId12 || motherId21 != motherId22) {
						fhPhotons_MC_invmass4->Fill(invmass);
					}
					
					
					
					if(motherId11 != -1 && motherId12 != -1 && motherId21 != -1 && motherId22 != -1) {
						CbmMCTrack* mothermctrack11 = (CbmMCTrack*)fMcTracks->At(motherId11);
						CbmMCTrack* mothermctrack12 = (CbmMCTrack*)fMcTracks->At(motherId12);
						CbmMCTrack* mothermctrack21 = (CbmMCTrack*)fMcTracks->At(motherId21);
						CbmMCTrack* mothermctrack22 = (CbmMCTrack*)fMcTracks->At(motherId22);
					
						fhPhotons_MC_motherpdg->Fill(mothermctrack11->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack12->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack21->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack22->GetPdgCode());
					}
				}
			}
		}
	}
}



Double_t CbmAnaConversionRecoFull::OpeningAngleBetweenPhotons(vector<int> photon1, vector<int> photon2)
{
	Double_t energy1 = TMath::Sqrt(fElectrons_momenta[photon1[0]].Mag2() + M2E);
	TLorentzVector lorVec1(fElectrons_momenta[photon1[0]], energy1);
	
	Double_t energy2 = TMath::Sqrt(fElectrons_momenta[photon1[1]].Mag2() + M2E);
	TLorentzVector lorVec2(fElectrons_momenta[photon1[1]], energy2);
	
	Double_t energy3 = TMath::Sqrt(fElectrons_momenta[photon2[0]].Mag2() + M2E);
	TLorentzVector lorVec3(fElectrons_momenta[photon2[0]], energy3);
	
	Double_t energy4 = TMath::Sqrt(fElectrons_momenta[photon2[1]].Mag2() + M2E);
	TLorentzVector lorVec4(fElectrons_momenta[photon2[1]], energy4);

	TLorentzVector lorPhoton1 = lorVec1 + lorVec2;
	TLorentzVector lorPhoton2 = lorVec3 + lorVec4;

	Double_t angleBetweenPhotons = lorPhoton1.Angle(lorPhoton2.Vect());
	Double_t theta = 180.*angleBetweenPhotons/TMath::Pi();

	return theta;
}


/*
void CbmAnaConversionRecoFull::CombineElectrons()
{
	Int_t nof = fElectrons_momenta.size();
	if(nof >= 4) {
		for(int a=0; a<nof-3; a++) {
			for(int b=a; b<nof-2; b++) {
				for(int c=b; c<nof-1; c++) {
					for(int d=c; d<nof; d++) {
						Int_t check1 = (fElectrons_track[a]->GetParamLast()->GetQp() > 0);	// positive or negative charge (qp = charge over momentum ratio)
						Int_t check2 = (fElectrons_track[b]->GetParamLast()->GetQp() > 0);
						Int_t check3 = (fElectrons_track[c]->GetParamLast()->GetQp() > 0);
						Int_t check4 = (fElectrons_track[d]->GetParamLast()->GetQp() > 0);
						Int_t test = check1 + check2 + check3 + check4;
						if(test != 2) continue;		// need two electrons and two positrons
						
						
						Double_t invmass = Invmass_4particlesRECO(fElectrons_momenta[a], fElectrons_momenta[b], fElectrons_momenta[c], fElectrons_momenta[d]);
						fhElectrons_invmass->Fill(invmass);
						
						
						CbmLmvmKinematicParams params1 = CalculateKinematicParamsReco(fElectrons_momenta[a], fElectrons_momenta[b]);
						CbmLmvmKinematicParams params2 = CalculateKinematicParamsReco(fElectrons_momenta[a], fElectrons_momenta[c]);
						CbmLmvmKinematicParams params3 = CalculateKinematicParamsReco(fElectrons_momenta[a], fElectrons_momenta[d]);
						CbmLmvmKinematicParams params4 = CalculateKinematicParamsReco(fElectrons_momenta[b], fElectrons_momenta[c]);
						CbmLmvmKinematicParams params5 = CalculateKinematicParamsReco(fElectrons_momenta[b], fElectrons_momenta[d]);
						CbmLmvmKinematicParams params6 = CalculateKinematicParamsReco(fElectrons_momenta[c], fElectrons_momenta[d]);
						
						Double_t openingAngleCut = 1;
						Int_t IsPhoton_openingAngle1 = (params1.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle2 = (params2.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle3 = (params3.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle4 = (params4.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle5 = (params5.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle6 = (params6.fAngle < openingAngleCut);
						
						Double_t invMassCut = 0.03;
						Int_t IsPhoton_invMass1 = (params1.fMinv < invMassCut);
						Int_t IsPhoton_invMass2 = (params2.fMinv < invMassCut);
						Int_t IsPhoton_invMass3 = (params3.fMinv < invMassCut);
						Int_t IsPhoton_invMass4 = (params4.fMinv < invMassCut);
						Int_t IsPhoton_invMass5 = (params5.fMinv < invMassCut);
						Int_t IsPhoton_invMass6 = (params6.fMinv < invMassCut);
						
						if(IsPhoton_openingAngle1 && IsPhoton_openingAngle6 && IsPhoton_invMass1 && IsPhoton_invMass6 && (check1 + check2 == 1) && (check3 + check4 == 1)) {
							fhElectrons_invmass_cut->Fill(invmass);
						}
						if(IsPhoton_openingAngle2 && IsPhoton_openingAngle5 && IsPhoton_invMass2 && IsPhoton_invMass5 && (check1 + check3 == 1) && (check2 + check4 == 1)) {
							fhElectrons_invmass_cut->Fill(invmass);
						}
						if(IsPhoton_openingAngle3 && IsPhoton_openingAngle4 && IsPhoton_invMass3 && IsPhoton_invMass4 && (check1 + check4 == 1) && (check2 + check3 == 1)) {
							fhElectrons_invmass_cut->Fill(invmass);
						}
					}
				}
			}
		}
	}
}
*/

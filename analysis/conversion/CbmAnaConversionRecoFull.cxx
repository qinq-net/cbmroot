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
 :  electronidentifier(NULL),
	fRichPoints(NULL),
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
	fElectrons_track(NULL),
	fElectrons_momenta(NULL),
	fhElectrons_invmass(NULL),
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

	fhElectrons = new TH1I("fhElectrons", "fhElectrons; nof photons per event; #", 7, 0., 7.);
	fHistoList_recofull.push_back(fhElectrons);
	fhElectrons->GetXaxis()->SetBinLabel(1, "electrons rich");
	fhElectrons->GetXaxis()->SetBinLabel(2, "electrons trd");
	fhElectrons->GetXaxis()->SetBinLabel(3, "electrons tof");
	fhElectrons->GetXaxis()->SetBinLabel(4, "electrons rich+trd");
	fhElectrons->GetXaxis()->SetBinLabel(5, "electrons rich+tof");
	fhElectrons->GetXaxis()->SetBinLabel(6, "electrons trd+tof");
	fhElectrons->GetXaxis()->SetBinLabel(7, "electrons rich+trd+tof");


	fhElectrons_invmass = new TH1D("fhElectrons_invmass", "fhElectrons_invmass; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhElectrons_invmass);
	
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


		if(electron_rich) {
			fElectrons_track.push_back(gTrack);
			fElectrons_momenta.push_back(refittedMomentum);
		}
	}
	cout << "CbmAnaConversionRecoFull: number of global tracks in STS and RICH " << nofGT_richsts << endl;

	CombineElectrons();

	timer.Stop();
	fTime += timer.RealTime();
}



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
					}
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


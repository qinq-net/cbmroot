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
    fHistoList_test(),
    fElectrons_gtid(),
	fElectrons_mcid(),
	fElectrons_richInd(),
	fElectrons_pi0mcid(),
	fElectrons_same(),
	fElectrons_nofPerPi0(NULL),
	fElectrons_nofPerPi0_withRichInd(NULL),
	fhElectrons_invmass(NULL)
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
}


void CbmAnaConversionTest::InitHistos()
{
	fHistoList_test.clear();

	fElectrons_nofPerPi0 = new TH1I("fElectrons_nofPerPi0", "fElectrons_nofPerPi0; nof; #", 7, -0.5, 6.5);
	fHistoList_test.push_back(fElectrons_nofPerPi0);
	fElectrons_nofPerPi0_withRichInd = new TH1I("fElectrons_nofPerPi0_withRichInd", "fElectrons_nofPerPi0_withRichInd; nof; #", 7, -0.5, 6.5);
	fHistoList_test.push_back(fElectrons_nofPerPi0_withRichInd);

	fhElectrons_invmass = new TH1D("fhElectrons_invmass", "fhElectrons_invmass; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_test.push_back(fhElectrons_invmass);

}







void CbmAnaConversionTest::Finish()
{
	//gDirectory->cd("analysis-conversion");
	gDirectory->mkdir("Test");
	gDirectory->cd("Test");
	for (Int_t i = 0; i < fHistoList_test.size(); i++){
		fHistoList_test[i]->Write();
	}
	gDirectory->cd("..");
}




void CbmAnaConversionTest::Exec()
{
	cout << "CbmAnaConversionTest: Exec()" << endl;
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
}



void CbmAnaConversionTest::GetNofRichElectrons()
{

		std::multimap<int,int> electronMap;
		for(int i=0; i<fElectrons_pi0mcid.size(); i++) {
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
						fhElectrons_invmass->Fill(invmass);
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



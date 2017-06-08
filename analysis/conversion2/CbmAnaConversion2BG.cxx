/**
 * \file CbmAnaConversion2BG.cxx
 *
 * Explanaiton of background
 *
 * \author ??
 * \date ??
 **/

#include "CbmAnaConversion2BG.h"



// included from CbmRoot
#include "FairRootManager.h"
#include "CbmKFParticleFinder.h"
#include "CbmKFParticleFinderQA.h"
#include "KFParticleTopoReconstructor.h"
#include "KFTopoPerformance.h"
#include "CbmStsTrack.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"
#include "TDatabasePDG.h"
#include "CbmGlobalTrack.h"
#include "CbmStsKFTrackFitter.h"
#include "CbmRichRing.h"

#include "KFMCParticle.h"
#include "KFParticleFinder.h"
#include "CbmKF.h"
#include "CbmKFParticle.h"


#define M2E 2.6112004954086e-7

using namespace std;



CbmAnaConversion2BG::CbmAnaConversion2BG()
 : fKFMcParticles(nullptr),
   fMcTracks(nullptr),
   fStsTracks(nullptr),
   fStsTrackMatches(nullptr),
   fGlobalTracks(nullptr),
   fRichRings(nullptr),
   fRichRingMatches(nullptr),


   timer(),

   fTime(0.)
{
}

CbmAnaConversion2BG::~CbmAnaConversion2BG()
{
}

void CbmAnaConversion2BG::Init()
{
	FairRootManager* ioman = FairRootManager::Instance();
	if (nullptr == ioman) { Fatal("CbmAnaConversion2BG::Init","RootManager not instantised!"); }

	fKFMcParticles = (TClonesArray*) ioman->GetObject("KFMCParticles");
	if ( nullptr == fKFMcParticles) { Fatal("CbmAnaConversion2BG::Init","No KFMCParticles array!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( nullptr == fMcTracks) { Fatal("CbmAnaConversion2BG::Init","No MCTrack array!"); }

	fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	if ( nullptr == fStsTracks) { Fatal("CbmAnaConversion2BG::Init","No StsTrack array!"); }

	fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
	if (nullptr == fStsTrackMatches) { Fatal("CbmAnaConversion2BG::Init","No StsTrackMatch array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (nullptr == fGlobalTracks) { Fatal("CbmAnaConversion2BG::Init","No GlobalTrack array!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if (nullptr == fRichRings) { Fatal("CbmAnaConversion2BG::Init","No RichRing array!"); }

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if (nullptr == fRichRingMatches) { Fatal("CbmAnaConversion2BG::Init","No RichRingMatch array!"); }


	cout << " CbmAnaConversion2BG::Init() " << endl;
	InitHistos();
}


void CbmAnaConversion2BG::InitHistos()
{


	cout << " CbmAnaConversion2BG::InitHistos() " << endl;

}







void CbmAnaConversion2BG::Finish()
{


}




void CbmAnaConversion2BG::Exec(CbmMCTrack* mctrack1, CbmMCTrack* mctrack2, CbmMCTrack* mctrack3, CbmMCTrack* mctrack4, TVector3 refmomentum1, TVector3 refmomentum2, TVector3 refmomentum3, TVector3 refmomentum4, Double_t invmassRecoPi0, TH1D* Case1, TH1D* Case2, TH1D* Case3, TH1D* Case4, TH1D* Case5, TH1D* Case6, TH1D* Case7, TH1D* Case8, TH1D* Case9, TH1D* Case10, TH1D* PdgCase8, TH1D* testsameMIDcase8, TH1D* testsameGRIDcase8, TH1D* PdgCase8mothers, TH1D* case8GRIDInvMassGamma, TH1D* case8GRIDOAGamma, TH2D* Case1ZYPos)
{
	timer.Start();

	int pdg1 = mctrack1->GetPdgCode();
	int pdg2 = mctrack2->GetPdgCode();
	int pdg3 = mctrack3->GetPdgCode();
	int pdg4 = mctrack4->GetPdgCode();
	int motherId1 = mctrack1->GetMotherId();
	int motherId2 = mctrack2->GetMotherId();
	int motherId3 = mctrack3->GetMotherId();
	int motherId4 = mctrack4->GetMotherId();

	int NumOfOthers = 0;
	if( TMath::Abs(pdg1) != 11) NumOfOthers++;
	if( TMath::Abs(pdg2) != 11) NumOfOthers++;
	if( TMath::Abs(pdg3) != 11) NumOfOthers++;
	if( TMath::Abs(pdg4) != 11) NumOfOthers++;

	// cases 7-10
	if(NumOfOthers == 4) Case10->Fill(invmassRecoPi0);
	if(NumOfOthers == 3) Case9->Fill(invmassRecoPi0);
	// case 8 !!!!
	if(NumOfOthers == 2){ 
		Case8->Fill(invmassRecoPi0);
		PdgCase8->Fill(pdg1);
		PdgCase8->Fill(pdg2);
		PdgCase8->Fill(pdg3);
		PdgCase8->Fill(pdg4);



		std::vector<int> testM;
		if (TMath::Abs(pdg1) == 11) testM.push_back(mctrack1->GetMotherId());
		if (TMath::Abs(pdg2) == 11) testM.push_back(mctrack2->GetMotherId());
		if (TMath::Abs(pdg3) == 11) testM.push_back(mctrack3->GetMotherId());
		if (TMath::Abs(pdg4) == 11) testM.push_back(mctrack4->GetMotherId());

		if (testM.size() == 2){
			if (testM[0] == testM[1]) testsameMIDcase8->Fill(invmassRecoPi0);

/*			if(invmassRecoPi0<0.2){
				Double_t OA1 = CalculateOpeningAngleReco(refmomentum1, refmomentum2);
				Double_t OA2 = CalculateOpeningAngleReco(refmomentum3, refmomentum4);
				cout << "normal OA --> A1 = " << OA1 << "; A2 = " << OA2 << endl;
				Double_t OA3 = CalculateOpeningAngleReco(refmomentum1, refmomentum3);
				Double_t OA4 = CalculateOpeningAngleReco(refmomentum2, refmomentum4);
				Double_t OA5 = CalculateOpeningAngleReco(refmomentum1, refmomentum4);
				Double_t OA6 = CalculateOpeningAngleReco(refmomentum2, refmomentum3);
				cout << "tested OA --> A3 = " << OA3 << "; A4 = " << OA4 << endl;
				cout << "tested OA --> A5 = " << OA5 << "; A5 = " << OA6 << endl;
			}*/

		}

		CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
		CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
		CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
		CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);

		std::vector<int> testGR;
		if (TMath::Abs(pdg1) == 11 && nullptr != mother1) testGR.push_back(mother1->GetMotherId());
		if (TMath::Abs(pdg2) == 11 && nullptr != mother2) testGR.push_back(mother2->GetMotherId());
		if (TMath::Abs(pdg3) == 11 && nullptr != mother3) testGR.push_back(mother3->GetMotherId());
		if (TMath::Abs(pdg4) == 11 && nullptr != mother4) testGR.push_back(mother4->GetMotherId());

		if (testGR.size() == 2){
			if (testGR[0] == testGR[1]){ 
				Double_t invmasstwopart1 = Invmass_2particlesRECO(refmomentum1, refmomentum2);
				Double_t OA1 = CalculateOpeningAngleReco(refmomentum1, refmomentum2);
				Double_t invmasstwopart2 = Invmass_2particlesRECO(refmomentum3, refmomentum4);
				Double_t OA2 = CalculateOpeningAngleReco(refmomentum3, refmomentum4);
				testsameGRIDcase8->Fill(invmassRecoPi0);
				case8GRIDInvMassGamma->Fill(invmasstwopart1);
				case8GRIDInvMassGamma->Fill(invmasstwopart2);
				case8GRIDOAGamma->Fill(OA1);
				case8GRIDOAGamma->Fill(OA2);
			}
		}

		testGR.clear();
		testM.clear();

		if (nullptr != mother1) PdgCase8mothers->Fill(mother1->GetPdgCode());
		if (nullptr != mother2) PdgCase8mothers->Fill(mother2->GetPdgCode());
		if (nullptr != mother3) PdgCase8mothers->Fill(mother3->GetPdgCode());
		if (nullptr != mother4) PdgCase8mothers->Fill(mother4->GetPdgCode());

	}
	if(NumOfOthers == 1) Case7->Fill(invmassRecoPi0);
	// cases 1-6
	if(NumOfOthers == 0){
		if (motherId1 != -1 && motherId2 != -1 && motherId3 != -1 && motherId4 != -1){
			CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
			CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
			CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
			CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);
			if (nullptr != mother1 && nullptr != mother2 && nullptr != mother3 && nullptr != mother4){
				int mcMotherPdg1 = mother1->GetPdgCode();
				int mcMotherPdg2 = mother2->GetPdgCode();
				int mcMotherPdg3 = mother3->GetPdgCode();
				int mcMotherPdg4 = mother4->GetPdgCode();
				int grandmotherId1 = mother1->GetMotherId();
				int grandmotherId2 = mother2->GetMotherId();
				int grandmotherId3 = mother3->GetMotherId();
				int grandmotherId4 = mother4->GetMotherId();
				// case 1
				if(motherId1 == motherId2 && motherId3 == motherId4 && grandmotherId1 == grandmotherId3){ 
					if (grandmotherId1 != -1){
						CbmMCTrack* grm1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
						if (nullptr != grm1 && grm1->GetPdgCode() == 111){ // && mctrack1->GetStartZ() < 4.5 && mctrack3->GetStartZ() < 4.5){
							Case1->Fill(invmassRecoPi0); 
							Case1ZYPos->Fill(mctrack1->GetStartZ(), mctrack1->GetStartY()); 
							Case1ZYPos->Fill(mctrack3->GetStartZ(), mctrack3->GetStartY());
						}
					}
				}
				if(motherId1 != motherId2 && motherId3 != motherId4){
					if (grandmotherId1 == grandmotherId3 && grandmotherId1 == grandmotherId4 && grandmotherId1 == grandmotherId2){
						Case2->Fill(invmassRecoPi0);
					} else {
						Case6->Fill(invmassRecoPi0);
					}
				}
					
				if(motherId1 == motherId2 && motherId3 == motherId4 && grandmotherId1 != grandmotherId3) Case3->Fill(invmassRecoPi0);

				if((motherId1 == motherId2 && motherId3 != motherId4) || (motherId1 != motherId2 && motherId3 == motherId4)){
					if ((grandmotherId1 == grandmotherId3 && grandmotherId1 == grandmotherId4 && grandmotherId1 != grandmotherId2) ||
                        (grandmotherId1 == grandmotherId2 && grandmotherId1 == grandmotherId4 && grandmotherId1 != grandmotherId3) ||
                        (grandmotherId1 == grandmotherId2 && grandmotherId1 == grandmotherId3 && grandmotherId1 != grandmotherId4) ||
                        (grandmotherId2 == grandmotherId3 && grandmotherId2 == grandmotherId4 && grandmotherId2 != grandmotherId1) || 
                        (grandmotherId2 == grandmotherId1 && grandmotherId2 == grandmotherId4 && grandmotherId2 != grandmotherId3) || 
                        (grandmotherId2 == grandmotherId1 && grandmotherId2 == grandmotherId3 && grandmotherId2 != grandmotherId4) || 
                        (grandmotherId3 == grandmotherId1 && grandmotherId3 == grandmotherId2 && grandmotherId3 != grandmotherId4) ||
                        (grandmotherId3 == grandmotherId1 && grandmotherId3 == grandmotherId4 && grandmotherId3 != grandmotherId2) || 
                        (grandmotherId3 == grandmotherId2 && grandmotherId3 == grandmotherId4 && grandmotherId3 != grandmotherId1) ||
                        (grandmotherId4 == grandmotherId1 && grandmotherId4 == grandmotherId2 && grandmotherId4 != grandmotherId3) || 
                        (grandmotherId4 == grandmotherId1 && grandmotherId4 == grandmotherId3 && grandmotherId4 != grandmotherId2) || 
                        (grandmotherId4 == grandmotherId2 && grandmotherId4 == grandmotherId3 && grandmotherId4 != grandmotherId1)   ){
						Case4->Fill(invmassRecoPi0);
					} else {
						Case5->Fill(invmassRecoPi0);
					}
				}
			}
		}
	}


	timer.Stop();
	fTime += timer.RealTime();
}

Double_t CbmAnaConversion2BG::Invmass_2particlesRECO(const TVector3 part1, const TVector3 part2)
{
	Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
	TLorentzVector lorVec1(part1, energy1);

	Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
	TLorentzVector lorVec2(part2, energy2);

	TLorentzVector sum;
	sum = lorVec1 + lorVec2;    

	return sum.Mag();
}

Double_t CbmAnaConversion2BG::CalculateOpeningAngleReco(TVector3 electron1, TVector3 electron2)
{
	Double_t energyP = TMath::Sqrt(electron1.Mag2() + M2E);
	TLorentzVector lorVecP(electron1, energyP);

	Double_t energyM = TMath::Sqrt(electron2.Mag2() + M2E);
	TLorentzVector lorVecM(electron2, energyM);

	Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
	Double_t theta = 180.*anglePair/TMath::Pi();

	return theta;
}

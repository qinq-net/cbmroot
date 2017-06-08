/**
 * \file CbmAnaConversionReco.cxx
 *
 * 
 * 
 *
 * \author ????
 * \date ?????
 **/

#include "CbmAnaConversion2Reconstruction.h"

// standard includes
#include <iostream>
#include <boost/assign/list_of.hpp>

// includes from ROOT
#include "TRandom3.h"

// included from CbmRoot
#include "FairRootManager.h"
#include "CbmMCTrack.h"
#include "CbmGlobalTrack.h"
#include "CbmRichRing.h"
#include "CbmRichElectronIdAnn.h"
#include "CbmRichRingLight.h"
#include "CbmRichHit.h"
#include "TCanvas.h"


#define M2E 2.6112004954086e-7
using namespace std;
using boost::assign::list_of;

CbmAnaConversion2Reconstruction::CbmAnaConversion2Reconstruction()
  : fRichRings(nullptr),
	fRichHits(nullptr),
        fMcTracks(nullptr),
	fGlobalTracks(nullptr),
	fRecoTracklistEPEM(),
        fRecoTracklistEPEM_ids(),
        fRecoTracklistEPEM_gtid(),
  	fRecoRefittedMomentum_dir(),
	fRecoRefittedMomentum_conv(),
	fRecoRefittedMomentum(),
	STS_Id(),
	fRecoTracklistEPEMwithRICH(),
        fRecoTracklistEPEM_idswithRICH(),
        fRecoTracklistEPEM_gtidwithRICH(),
	fRecoRefittedMomentumwithRICH(),
	fHistoList_gg(),
	fHistoList_gee(),
	fHistoList_eeee(),

	fhEPEM_openingAngle_gee_mc(nullptr),
	fhEPEM_openingAngle_gee_refitted(nullptr),
	fhEPEM_openingAngle_gee_mc_dalitz(nullptr),
	fhEPEM_openingAngle_gee_refitted_dalitz(nullptr),
	fhEPEM_openingAngle_betweenGammas_mc(nullptr),
	fhEPEM_openingAngle_betweenGammas_reco(nullptr),
	fhEPEM_invmass_eeee_refitted(nullptr),
	fhEPEM_invmass_eeee_mc(nullptr),
        OAsmallest(nullptr),
        OpeningAngleMCgg(nullptr),
        OpeningAngleRecogg(nullptr),
        fhEPEM_invmass_gg_mc(nullptr),
        fhEPEM_invmass_gg_refitted(nullptr),
        fhEPEM_invmass_gee_mc(nullptr),
        fhEPEM_invmass_gee_refitted(nullptr)
{
}

CbmAnaConversion2Reconstruction::~CbmAnaConversion2Reconstruction()
{
}


void CbmAnaConversion2Reconstruction::Init()
{
	FairRootManager* ioman = FairRootManager::Instance();
	if (nullptr == ioman) { Fatal("CbmAnaConversion::Init","RootManager not instantised!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( nullptr == fMcTracks) { Fatal("CbmAnaConversion::Init","No MCTrack array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (nullptr == fGlobalTracks) { Fatal("CbmAnaConversion::Init","No GlobalTrack array!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if (nullptr == fRichRings) { Fatal("CbmAnaConversion::Init","No RichRing array!"); }

	fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
	if ( nullptr == fRichHits) { Fatal("CbmAnaConversion::Init","No RichHit array!"); }

	InitHistos();

}


void CbmAnaConversion2Reconstruction::InitHistos()
{
	fHistoList_gg.clear();
	fHistoList_gee.clear();
	fHistoList_eeee.clear();

	fhEPEM_openingAngle_gee_mc					= new TH1D("fhEPEM_openingAngle_gee_mc","fhEPEM_openingAngle_gee_mc (between e+e- from g);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_gee.push_back(fhEPEM_openingAngle_gee_mc);
	fhEPEM_openingAngle_gee_refitted			= new TH1D("fhEPEM_openingAngle_gee_refitted","fhEPEM_openingAngle_gee_refitted (between e+e- from g);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_gee.push_back(fhEPEM_openingAngle_gee_refitted);
	fhEPEM_openingAngle_gee_mc_dalitz			= new TH1D("fhEPEM_openingAngle_gee_mc_dalitz","fhEPEM_openingAngle_gee_mc_dalitz (between e+e- from pi0);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_gee.push_back(fhEPEM_openingAngle_gee_mc_dalitz);
	fhEPEM_openingAngle_gee_refitted_dalitz		= new TH1D("fhEPEM_openingAngle_gee_refitted_dalitz","fhEPEM_openingAngle_gee_refitted_dalitz (between e+e- from pi0);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_gee.push_back(fhEPEM_openingAngle_gee_refitted_dalitz);
	fhEPEM_openingAngle_betweenGammas_mc		= new TH1D("fhEPEM_openingAngle_betweenGammas_mc","fhEPEM_openingAngle_betweenGammas_mc;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_gg.push_back(fhEPEM_openingAngle_betweenGammas_mc);
	fhEPEM_openingAngle_betweenGammas_reco		= new TH1D("fhEPEM_openingAngle_betweenGammas_reco","fhEPEM_openingAngle_betweenGammas_reco;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_gg.push_back(fhEPEM_openingAngle_betweenGammas_reco);

	OAsmallest			= new TH1D("OAsmallest","OAsmallest (between e+e- and e+e- from pi0);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_eeee.push_back(OAsmallest);

	OpeningAngleMCgg			= new TH1D("OpeningAngleMCgg","OpeningAngleMCgg (between e+e- and e+e- from pi0->gg);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_gg.push_back(OpeningAngleMCgg);

	OpeningAngleRecogg			= new TH1D("OpeningAngleRecogg","OpeningAngleRecogg (between e+e- and e+e- from pi0->gg);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_gg.push_back(OpeningAngleRecogg);

	fhEPEM_invmass_gg_mc = new TH1D("fhEPEM_invmass_gg_mc","fhEPEM_invmass_gg_mc;invariant mass in GeV/c^{2};#", 400, -0.1, 0.5);
	fHistoList_gg.push_back(fhEPEM_invmass_gg_mc);
	fhEPEM_invmass_gg_refitted = new TH1D("fhEPEM_invmass_gg_refitted","fhEPEM_invmass_gg_refitted;invariant mass in GeV/c^{2};#", 400, -0.1, 0.5);
	fHistoList_gg.push_back(fhEPEM_invmass_gg_refitted);

	fhEPEM_invmass_gee_mc = new TH1D("fhEPEM_invmass_gee_mc","fhEPEM_invmass_gee_mc;invariant mass in GeV/c^{2};#", 400, -0.1, 0.5);
	fHistoList_gee.push_back(fhEPEM_invmass_gee_mc);
	fhEPEM_invmass_gee_refitted = new TH1D("fhEPEM_invmass_gee_refitted","fhEPEM_invmass_gee_refitted;invariant mass in GeV/c^{2};#", 400, -0.1, 0.5);
	fHistoList_gee.push_back(fhEPEM_invmass_gee_refitted);

	fhEPEM_invmass_eeee_mc						= new TH1D("fhEPEM_invmass_eeee_mc","fhEPEM_invmass_eeee_mc;invariant mass in GeV/c^{2};#", 400, -0.1, 0.5);
	fHistoList_eeee.push_back(fhEPEM_invmass_eeee_mc);
	fhEPEM_invmass_eeee_refitted				= new TH1D("fhEPEM_invmass_eeee_refitted","fhEPEM_invmass_eeee_refitted;invariant mass in GeV/c^{2};#", 400, -0.1, 0.5);
	fHistoList_eeee.push_back(fhEPEM_invmass_eeee_refitted);

}







void CbmAnaConversion2Reconstruction::Finish()
{

	gDirectory->mkdir("Reconstruction2");
	gDirectory->cd("Reconstruction2");
	
	gDirectory->mkdir("pi0 -> gg");
	gDirectory->cd("pi0 -> gg");
	for (UInt_t i = 0; i < fHistoList_gg.size(); i++){
		fHistoList_gg[i]->Write();
	}
	gDirectory->cd("..");
	
	gDirectory->mkdir("pi0 -> ge+e-");
	gDirectory->cd("pi0 -> ge+e-");
	for (UInt_t i = 0; i < fHistoList_gee.size(); i++){
		fHistoList_gee[i]->Write();
	}
	gDirectory->cd("..");
	
	gDirectory->mkdir("pi0 -> e+e-e+e-");
	gDirectory->cd("pi0 -> e+e-e+e-");
	for (UInt_t i = 0; i < fHistoList_eeee.size(); i++){
		fHistoList_eeee[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->cd("..");

//	TCanvas* c333 = new TCanvas();
//	fhEPEM_invmass_eeee_refitted-> Draw();

}


void CbmAnaConversion2Reconstruction::SetTracklistReco(vector<CbmMCTrack*> MCTracklist_CbmAnaConv2, vector<CbmMCTrack*> MCTracklist_CbmAnaConv2withRICH, vector<TVector3> reffitedMomentum_CbmAnaConv2, vector<TVector3> reffitedMomentum_CbmAnaConv2withRICH, int DecayedParticlePdg, int fEventNum, vector<Int_t> fmcTrack_STS_Id)
//void CbmAnaConversion2Reconstruction::SetTracklistReco(vector<CbmMCTrack*> MCTracklist_CbmAnaConv2, vector<int> ids_CbmAnaConv2, vector<Int_t> GlobalTrackId_CbmAnaConv2, vector<TVector3> reffitedMomentum_CbmAnaConv2)
{
	Int_t fDecayedParticlePdg = DecayedParticlePdg;
	Int_t fEvent = fEventNum;
	STS_Id = fmcTrack_STS_Id;

	fRecoTracklistEPEM = MCTracklist_CbmAnaConv2;
	fRecoRefittedMomentum = reffitedMomentum_CbmAnaConv2;

// fake
//	fRecoTracklistEPEM = MCTracklist_CbmAnaConv2withRICH;
//	fRecoRefittedMomentum = reffitedMomentum_CbmAnaConv2withRICH;

	fRecoTracklistEPEMwithRICH = MCTracklist_CbmAnaConv2withRICH;
	fRecoRefittedMomentumwithRICH = reffitedMomentum_CbmAnaConv2withRICH;


	if(fRecoTracklistEPEM.size() < 4 || fRecoTracklistEPEM.size() < 4) return;
	for(unsigned int i=0; i<fRecoTracklistEPEM.size(); i++) {
//		if(i%10 == 0) cout << "CbmAnaConversion2Reconstruction: InvariantMassTest_4epem - iteration i = " << i << endl;
		for(unsigned int j=i+1; j<fRecoTracklistEPEM.size(); j++) {
			for(unsigned int k=j+1; k<fRecoTracklistEPEM.size(); k++) {
				for(unsigned int l=k+1; l<fRecoTracklistEPEM.size(); l++) {
					if(fRecoTracklistEPEM.at(i)->GetPdgCode() + fRecoTracklistEPEM.at(j)->GetPdgCode() + fRecoTracklistEPEM.at(k)->GetPdgCode() + fRecoTracklistEPEM.at(l)->GetPdgCode() != 0) continue;
					
					int motherId1 = fRecoTracklistEPEM.at(i)->GetMotherId();
					int motherId2 = fRecoTracklistEPEM.at(j)->GetMotherId();
					int motherId3 = fRecoTracklistEPEM.at(k)->GetMotherId();
					int motherId4 = fRecoTracklistEPEM.at(l)->GetMotherId();

					int STSmcId1 = STS_Id.at(i);
					int STSmcId2 = STS_Id.at(j);
					int STSmcId3 = STS_Id.at(k);
					int STSmcId4 = STS_Id.at(l);
					//cout << "STSmcIds: \t " << STSmcId1 << "/" << STSmcId2 << "/" << STSmcId3 << "/" << STSmcId4 << endl;
					if (STSmcId1 == STSmcId2 || STSmcId1 == STSmcId3 || STSmcId1 == STSmcId4 || STSmcId2 == STSmcId3 || STSmcId2 == STSmcId4 || STSmcId3 == STSmcId4) continue; // not used twice

					//================================== decay pi0 -> e+ e- e+ e-
					if(motherId1 == motherId2 && motherId1 == motherId3 && motherId1 == motherId4) {
						if (motherId1 != -1) {
							int mcMotherPdg1  = -1;
							CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
							if(nullptr != mother1) mcMotherPdg1 = mother1->GetPdgCode();
							if( (mcMotherPdg1 == fDecayedParticlePdg) ) { 
								Double_t invmass1 = 0;
								Double_t invmass3 = 0;
								invmass1 = Invmass_4particles(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));	// true MC values
								invmass3 = Invmass_4particlesRECO(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
								cout << "CbmAnaConversion2Reconstruction, event No. " << fEventNum << endl;
								cout << "Decay pi0 -> e+e-e+e- detected!\t\t mc mass: " << invmass1 << " \t reco mass: " << invmass3 << endl;
								cout << "motherids: " << motherId1 << "/" << motherId2 << "/" << motherId3 << "/" << motherId4 << "\t motherpdg: " << mcMotherPdg1 << "\t mctrack mass: " << mother1->GetMass() << endl;
//								cout << "particleids: " << fRecoTracklistEPEM.at(i)->GetPt() << "/" << fRecoTracklistEPEM.at(j)->GetPt() << "/" << fRecoTracklistEPEM.at(k)->GetPt() << "/" << fRecoTracklistEPEM.at(l)->GetPt() << "\t motherpdg: " << mcMotherPdg1 << "\t mctrack mass: " << mother1->GetMass() << endl;
								cout << "pdgs " << fRecoTracklistEPEM.at(i)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(j)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(k)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(l)->GetPdgCode() << endl;

								fhEPEM_invmass_eeee_mc->Fill(invmass1);
								fhEPEM_invmass_eeee_refitted->Fill(invmass3);

								TVector3 start1;
								fRecoTracklistEPEM.at(i)->GetStartVertex(start1);
								TVector3 start2;
								fRecoTracklistEPEM.at(j)->GetStartVertex(start2);
								TVector3 start3;
								fRecoTracklistEPEM.at(k)->GetStartVertex(start3);
								TVector3 start4;
								fRecoTracklistEPEM.at(l)->GetStartVertex(start4);
								cout << "start: " << start1.Z() << "/" << start2.Z() << "/" << start3.Z() << "/" << start4.Z() << endl;

								Double_t opening_angle1_mc1 = 0;
								Double_t opening_angle1_mc2 = 0;
								Double_t opening_angle1_mc3 = 0;
								Double_t opening_angle1_mc4 = 0;
								Double_t opening_angle1_refitted1 = 0;
								Double_t opening_angle1_refitted2 = 0;
								Double_t opening_angle1_refitted3 = 0;
								Double_t opening_angle1_refitted4 = 0;
								Double_t sum12 = 0;
								Double_t sum34 = 0;
								if (fRecoTracklistEPEM.at(i)->GetPdgCode() + fRecoTracklistEPEM.at(j)->GetPdgCode() == 0){
									opening_angle1_mc1 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j));
									opening_angle1_mc2 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));
									//sum12 = opening_angle1_mc1 + opening_angle1_mc2;
									opening_angle1_refitted1 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j));
									opening_angle1_refitted2 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
									sum12 = opening_angle1_refitted1 + opening_angle1_refitted2;
									if (fRecoTracklistEPEM.at(i)->GetPdgCode() + fRecoTracklistEPEM.at(k)->GetPdgCode() == 0){
										opening_angle1_mc3 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(k));
										opening_angle1_mc4 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(l));
										//sum34 = opening_angle1_mc3 + opening_angle1_mc4;
										opening_angle1_refitted3 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(k));
										opening_angle1_refitted4 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(l));
										sum34 = opening_angle1_refitted3 + opening_angle1_refitted4;
									} else {
										opening_angle1_mc3 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(l));
										opening_angle1_mc4 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k));
										//sum34 = opening_angle1_mc3 + opening_angle1_mc4;
										opening_angle1_refitted3 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(l));
										opening_angle1_refitted4 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k));
										sum34 = opening_angle1_refitted3 + opening_angle1_refitted4;
									}
									if (sum12 > sum34) {
										//OAsmallest->Fill(opening_angle1_mc3); 
										//OAsmallest->Fill(opening_angle1_mc4);
										OAsmallest->Fill(opening_angle1_refitted3); 
										OAsmallest->Fill(opening_angle1_refitted4);
									} else {
										//OAsmallest->Fill(opening_angle1_mc1); 
										//OAsmallest->Fill(opening_angle1_mc2);
										OAsmallest->Fill(opening_angle1_refitted1); 
										OAsmallest->Fill(opening_angle1_refitted2);
									}
								} else {
									opening_angle1_mc1 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(k));
									opening_angle1_mc2 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(l));
									opening_angle1_mc3 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(l));
									opening_angle1_mc4 = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(j));
									//sum12 = opening_angle1_mc1 + opening_angle1_mc2;
									//sum34 = opening_angle1_mc3 + opening_angle1_mc4;
									opening_angle1_refitted1 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(k));
									opening_angle1_refitted2 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(l));
									opening_angle1_refitted3 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(l));
									opening_angle1_refitted4 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(j));
									sum12 = opening_angle1_refitted1 + opening_angle1_refitted2;
									sum34 = opening_angle1_refitted3 + opening_angle1_refitted4;
									if (sum12 > sum34) {
										//OAsmallest->Fill(opening_angle1_mc3); 
										//OAsmallest->Fill(opening_angle1_mc4);
										OAsmallest->Fill(opening_angle1_refitted3); 
										OAsmallest->Fill(opening_angle1_refitted4);
									} else {
										//OAsmallest->Fill(opening_angle1_mc1); 
										//OAsmallest->Fill(opening_angle1_mc2);
										OAsmallest->Fill(opening_angle1_refitted1); 
										OAsmallest->Fill(opening_angle1_refitted2);
									}
								}
								cout << "opening_angle:\t\t " << opening_angle1_mc1 << "\t" << opening_angle1_mc2 << "\t" << opening_angle1_mc3 << "\t" << opening_angle1_mc4 << endl;

							}
						}
					}


					//================================== decay pi0 -> gamma(e+e-) e+e-
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

						CbmMCTrack* grandmother1 = nullptr;

						if (motherId1 == -1 || motherId2 == -1 || motherId3 == -1 || motherId4 == -1) continue;
						CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
						if (nullptr != mother1) mcMotherPdg1 = mother1->GetPdgCode();
						grandmotherId1 = mother1->GetMotherId();
						if(grandmotherId1 != -1) {
							grandmother1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
							if (nullptr != grandmother1) mcGrandmotherPdg1 = grandmother1->GetPdgCode();
						}
						CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
						if (nullptr != mother2) mcMotherPdg2 = mother2->GetPdgCode();
						grandmotherId2 = mother2->GetMotherId();
						CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
						if (nullptr != mother3) mcMotherPdg3 = mother3->GetPdgCode();
						grandmotherId3 = mother3->GetMotherId();
						CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);
						if (nullptr != mother4) mcMotherPdg4 = mother4->GetPdgCode();
						grandmotherId4 = mother4->GetMotherId();

						if(motherId1 == motherId2 && motherId3 == motherId4) {
							if(NofDaughters(motherId1) != 2 || NofDaughters(motherId3) != 2) continue;
							if( (grandmotherId1 == motherId3 && mcMotherPdg3 == fDecayedParticlePdg) || (motherId1 == grandmotherId3 && mcMotherPdg1 == fDecayedParticlePdg)) {
								Double_t invmass1 = 0;
								Double_t invmass3 = 0;
								invmass1 = Invmass_4particles(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));	// true MC values
								invmass3 = Invmass_4particlesRECO(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
								cout << "CbmAnaConversion2Reconstruction, event No. " << fEventNum << endl;
								cout << "Decay pi0 -> gamma(e+e-) e+e- detected!\t\t mc mass: " << invmass1 << "\t, reco mass: " << invmass3 << endl;
								cout << "motherids: " << motherId1 << "/" << motherId2 << "/" << motherId3 << "/" << motherId4 << "\t motherpdg: " << mcMotherPdg1 << "\t mctrack mass: " << mother1->GetMass() << endl;
								cout << "pdgs " << fRecoTracklistEPEM.at(i)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(j)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(k)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(l)->GetPdgCode() << endl;

								fhEPEM_invmass_gee_mc->Fill(invmass1);
								fhEPEM_invmass_gee_refitted->Fill(invmass3);

								TVector3 start1;
								fRecoTracklistEPEM.at(i)->GetStartVertex(start1);
								TVector3 start2;
								fRecoTracklistEPEM.at(j)->GetStartVertex(start2);
								TVector3 start3;
								fRecoTracklistEPEM.at(k)->GetStartVertex(start3);
								TVector3 start4;
								fRecoTracklistEPEM.at(l)->GetStartVertex(start4);
								cout << "start: " << start1.Z() << "/" << start2.Z() << "/" << start3.Z() << "/" << start4.Z() << endl;

								if(mcGrandmotherPdg1 == fDecayedParticlePdg) {	// case: i,j = electrons from gamma, k,l = electrons from pi0
									Double_t invmass2 = Invmass_2particles(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j));	// true MC values
									Double_t invmass4 = Invmass_2particlesRECO(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j));
								cout << "Decay gamma -> e+e- detected!\t\t mc mass: " << invmass2 << "\t, reco mass: " << invmass4 << endl;
								}
								if(mcMotherPdg1 == fDecayedParticlePdg) {	// case: i,j = electrons from pi0, k,l = electrons from gamma
									Double_t invmass2 = Invmass_2particles(fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));	// true MC values
									Double_t invmass4 = Invmass_2particlesRECO(fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
								cout << "Decay gamma -> e+e- detected!\t\t mc mass: " << invmass2 << "\t, reco mass: " << invmass4 << endl;
								}

								CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));

								if(mcGrandmotherPdg1 == fDecayedParticlePdg) {	// case: i,j = electrons from gamma, k,l = electrons from pi0
									Double_t opening_angle1_mc = 0;
									Double_t opening_angle1_refitted = 0;
									opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j));
									opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j));
									fhEPEM_openingAngle_gee_mc->Fill(opening_angle1_mc);
									fhEPEM_openingAngle_gee_refitted->Fill(opening_angle1_refitted);
									
									cout << "Opening angle dalitz --> " << endl;
									Double_t opening_angle1_mc_dalitz = 0;
									Double_t opening_angle1_refitted_dalitz = 0;
									opening_angle1_mc_dalitz = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));
									opening_angle1_refitted_dalitz = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
									fhEPEM_openingAngle_gee_mc_dalitz->Fill(opening_angle1_mc_dalitz);
									fhEPEM_openingAngle_gee_refitted_dalitz->Fill(opening_angle1_refitted_dalitz);
								}
								
								if(mcMotherPdg1 == fDecayedParticlePdg) {	// case: i,j = electrons from pi0, k,l = electrons from gamma
									Double_t opening_angle1_mc = 0;
									Double_t opening_angle1_refitted = 0;
									opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));
									opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
									fhEPEM_openingAngle_gee_mc->Fill(opening_angle1_mc);
									fhEPEM_openingAngle_gee_refitted->Fill(opening_angle1_refitted);
									
									cout << "Opening angle dalitz --> " << endl;
									Double_t opening_angle1_mc_dalitz = 0;
									Double_t opening_angle1_refitted_dalitz = 0;
									opening_angle1_mc_dalitz = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j));
									opening_angle1_refitted_dalitz = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j));
									fhEPEM_openingAngle_gee_mc_dalitz->Fill(opening_angle1_mc_dalitz);
									fhEPEM_openingAngle_gee_refitted_dalitz->Fill(opening_angle1_refitted_dalitz);
								}
							}
						}
						if(motherId1 == motherId3 && motherId2 == motherId4) {
							if(NofDaughters(motherId1) != 2 || NofDaughters(motherId2) != 2) continue;
							if( (grandmotherId1 == motherId2 && mcMotherPdg2 == fDecayedParticlePdg) || (motherId1 == grandmotherId2 && mcMotherPdg1 == fDecayedParticlePdg)) {
								Double_t invmass1 = 0;
								Double_t invmass3 = 0;
								invmass1 = Invmass_4particles(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));	// true MC values
								invmass3 = Invmass_4particlesRECO(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
								cout << "CbmAnaConversion2Reconstruction, event No. " << fEventNum << endl;
								cout << "Decay pi0 -> gamma(e+e-) e+e- detected!\t\t mc mass: " << invmass1 << "\t, reco mass: " << invmass3 << endl;
								cout << "motherids: " << motherId1 << "/" << motherId2 << "/" << motherId3 << "/" << motherId4 << "\t motherpdg: " << mcMotherPdg1 << "\t mctrack mass: " << mother1->GetMass() << endl;
								cout << "pdgs " << fRecoTracklistEPEM.at(i)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(j)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(k)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(l)->GetPdgCode() << endl;
								TVector3 start1;
								fRecoTracklistEPEM.at(i)->GetStartVertex(start1);
								TVector3 start2;
								fRecoTracklistEPEM.at(j)->GetStartVertex(start2);
								TVector3 start3;
								fRecoTracklistEPEM.at(k)->GetStartVertex(start3);
								TVector3 start4;
								fRecoTracklistEPEM.at(l)->GetStartVertex(start4);
								cout << "start: " << start1.Z() << "/" << start2.Z() << "/" << start3.Z() << "/" << start4.Z() << endl;
								if(mcGrandmotherPdg1 == fDecayedParticlePdg) {	// case: i,k = electrons from gamma, j,l = electrons from pi0
									Double_t invmass2 = Invmass_2particles(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(k));	// true MC values
									Double_t invmass4 = Invmass_2particlesRECO(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(k));
								cout << "Decay gamma -> e+e- detected!\t\t mc mass: " << invmass2 << "\t, reco mass: " << invmass4 << endl;
								}
								if(mcMotherPdg1 == fDecayedParticlePdg) {	// case: i,k = electrons from pi0, j,l = electrons from gamma
									Double_t invmass2 = Invmass_2particles(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(l));	// true MC values
									Double_t invmass4 = Invmass_2particlesRECO(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(l));
								cout << "Decay gamma -> e+e- detected!\t\t mc mass: " << invmass2 << "\t, reco mass: " << invmass4 << endl;
								}

								CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
								if(mcGrandmotherPdg1 == fDecayedParticlePdg) {
									Double_t opening_angle1_mc = 0;
									Double_t opening_angle1_refitted = 0;
									opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(k));
									opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(k));
									fhEPEM_openingAngle_gee_mc->Fill(opening_angle1_mc);
									fhEPEM_openingAngle_gee_refitted->Fill(opening_angle1_refitted);
									
									cout << "Opening angle dalitz --> " << endl;
									Double_t opening_angle1_mc_dalitz = 0;
									Double_t opening_angle1_refitted_dalitz = 0;
									opening_angle1_mc_dalitz = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(l));
									opening_angle1_refitted_dalitz = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(l));
									fhEPEM_openingAngle_gee_mc_dalitz->Fill(opening_angle1_mc_dalitz);
									fhEPEM_openingAngle_gee_refitted_dalitz->Fill(opening_angle1_refitted_dalitz);
								}
								
								if(mcMotherPdg1 == fDecayedParticlePdg) {
									Double_t opening_angle1_mc = 0;
									Double_t opening_angle1_refitted = 0;
									opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(l));
									opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(l));
									fhEPEM_openingAngle_gee_mc->Fill(opening_angle1_mc);
									fhEPEM_openingAngle_gee_refitted->Fill(opening_angle1_refitted);
									
									cout << "Opening angle dalitz --> " << endl;
									Double_t opening_angle1_mc_dalitz = 0;
									Double_t opening_angle1_refitted_dalitz = 0;
									opening_angle1_mc_dalitz = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(k));
									opening_angle1_refitted_dalitz = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(k));
									fhEPEM_openingAngle_gee_mc_dalitz->Fill(opening_angle1_mc_dalitz);
									fhEPEM_openingAngle_gee_refitted_dalitz->Fill(opening_angle1_refitted_dalitz);
								}
							}
						}
						if(motherId1 == motherId4 && motherId2 == motherId3) {
							if(NofDaughters(motherId1) != 2 || NofDaughters(motherId2) != 2) continue;
							if( (grandmotherId1 == motherId2 && mcMotherPdg2 == fDecayedParticlePdg) || (motherId1 == grandmotherId2 && mcMotherPdg1 == fDecayedParticlePdg)) {
								Double_t invmass1 = 0;
								Double_t invmass3 = 0;
								invmass1 = Invmass_4particles(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));	// true MC values
								invmass3 = Invmass_4particlesRECO(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
								cout << "CbmAnaConversion2Reconstruction, event No. " << fEventNum << endl;
								cout << "Decay pi0 -> gamma(e+e-) e+e- detected!\t\t mc mass: " << invmass1 << "\t, reco mass: " << invmass3 << endl;
								cout << "motherids: " << motherId1 << "/" << motherId2 << "/" << motherId3 << "/" << motherId4 << "\t motherpdg: " << mcMotherPdg1 << "\t mctrack mass: " << mother1->GetMass() << endl;
								cout << "pdgs " << fRecoTracklistEPEM.at(i)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(j)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(k)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(l)->GetPdgCode() << endl;
								TVector3 start1;
								fRecoTracklistEPEM.at(i)->GetStartVertex(start1);
								TVector3 start2;
								fRecoTracklistEPEM.at(j)->GetStartVertex(start2);
								TVector3 start3;
								fRecoTracklistEPEM.at(k)->GetStartVertex(start3);
								TVector3 start4;
								fRecoTracklistEPEM.at(l)->GetStartVertex(start4);
								cout << "start: " << start1.Z() << "/" << start2.Z() << "/" << start3.Z() << "/" << start4.Z() << endl;
								if(mcGrandmotherPdg1 == fDecayedParticlePdg) {	// case: i,l = electrons from gamma, k,j = electrons from pi0
									Double_t invmass2 = Invmass_2particles(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(l));	// true MC values
									Double_t invmass4 = Invmass_2particlesRECO(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(l));
								cout << "Decay gamma -> e+e- detected!\t\t mc mass: " << invmass2 << "\t, reco mass: " << invmass4 << endl;
								}
								if(mcMotherPdg1 == fDecayedParticlePdg) {	// case: i,l = electrons from pi0, k,j = electrons from gamma
									Double_t invmass2 = Invmass_2particles(fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(j));	// true MC values
									Double_t invmass4 = Invmass_2particlesRECO(fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(j));
								cout << "Decay gamma -> e+e- detected!\t\t mc mass: " << invmass2 << "\t, reco mass: " << invmass4 << endl;
								}

								CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
								if(mcGrandmotherPdg1 == fDecayedParticlePdg) {
									Double_t opening_angle1_mc = 0;
									Double_t opening_angle1_refitted = 0;
									opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(l));
									opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(l));
									fhEPEM_openingAngle_gee_mc->Fill(opening_angle1_mc);
									fhEPEM_openingAngle_gee_refitted->Fill(opening_angle1_refitted);
									
									cout << "Opening angle dalitz --> " << endl;
									Double_t opening_angle1_mc_dalitz = 0;
									Double_t opening_angle1_refitted_dalitz = 0;
									opening_angle1_mc_dalitz = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k));
									opening_angle1_refitted_dalitz = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k));
									fhEPEM_openingAngle_gee_mc_dalitz->Fill(opening_angle1_mc_dalitz);
									fhEPEM_openingAngle_gee_refitted_dalitz->Fill(opening_angle1_refitted_dalitz);
								}
								
								if(mcMotherPdg1 == fDecayedParticlePdg) {
									Double_t opening_angle1_mc = 0;
									Double_t opening_angle1_refitted = 0;
									opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k));
									opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k));
									fhEPEM_openingAngle_gee_mc->Fill(opening_angle1_mc);
									fhEPEM_openingAngle_gee_refitted->Fill(opening_angle1_refitted);
									
									cout << "Opening angle dalitz --> " << endl;
									Double_t opening_angle1_mc_dalitz = 0;
									Double_t opening_angle1_refitted_dalitz = 0;
									opening_angle1_mc_dalitz = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(l));
									opening_angle1_refitted_dalitz = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(l));
									fhEPEM_openingAngle_gee_mc_dalitz->Fill(opening_angle1_mc_dalitz);
									fhEPEM_openingAngle_gee_refitted_dalitz->Fill(opening_angle1_refitted_dalitz);
								}
							}
						}

						//================================== decay pi0 -> gamma gamma -> e+e- e+e-
						if(grandmotherId1 == grandmotherId2 && grandmotherId1 == grandmotherId3 && grandmotherId1 == grandmotherId4) {
							if(mcGrandmotherPdg1 != fDecayedParticlePdg) continue; 
							Double_t invmass1 = 0;
							Double_t invmass3 = 0;
							invmass1 = Invmass_4particles(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));	// true MC values
							invmass3 = Invmass_4particlesRECO(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
							cout << "CbmAnaConversion2Reconstruction, event No. " << fEventNum << endl;
							cout << "Decay pi0 -> gamma gamma -> e+e- e+e- detected!\t\t mc mass: " << invmass1 << "\t, reco mass: " << invmass3 << endl;
							cout << "motherids: " << motherId1 << "/" << motherId2 << "/" << motherId3 << "/" << motherId4 << "\t grandmotherpdg: " << mcGrandmotherPdg1 << "\t mctrack mass: " << grandmother1->GetMass() << endl;
							cout << "grandmotherid: " << grandmotherId1 << "/" << grandmotherId2 << "/" << grandmotherId3 << "/" << grandmotherId4 << endl;
							cout << "pdgs " << fRecoTracklistEPEM.at(i)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(j)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(k)->GetPdgCode() << "/" << fRecoTracklistEPEM.at(l)->GetPdgCode() << endl;

/*				Double_t OA1 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j));
				Double_t OA2 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
				cout << "reco: normal OA --> A1 = " << OA1 << "; A2 = " << OA2 << endl;
				Double_t OA3 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(k));
				Double_t OA4 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(l));
				Double_t OA5 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(l));
				Double_t OA6 = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k));
				cout << "reco: tested OA --> A3 = " << OA3 << "; A4 = " << OA4 << endl;
				cout << "reco: tested OA --> A5 = " << OA5 << "; A5 = " << OA6 << endl;*/

						fhEPEM_invmass_gg_mc->Fill(invmass1);
						fhEPEM_invmass_gg_refitted->Fill(invmass3);

cout << "\t \t mc true info: "  << endl;
cout << "particle 1: \t" << fRecoTracklistEPEM.at(i)->GetPdgCode() << ";\t pt = " << fRecoTracklistEPEM.at(i)->GetPt() << ";\t X = " << fRecoTracklistEPEM.at(i)->GetStartX() << ";\t Y = " << fRecoTracklistEPEM.at(i)->GetStartY() << ";\t Z = " << fRecoTracklistEPEM.at(i)->GetStartZ() << ";\t E = " << fRecoTracklistEPEM.at(i)->GetEnergy() << endl;
cout << "particle 2: \t" << fRecoTracklistEPEM.at(j)->GetPdgCode() << ";\t pt = " << fRecoTracklistEPEM.at(j)->GetPt() << ";\t X = " << fRecoTracklistEPEM.at(j)->GetStartX() << ";\t Y = " << fRecoTracklistEPEM.at(j)->GetStartY() << ";\t Z = " << fRecoTracklistEPEM.at(j)->GetStartZ() << ";\t E = " << fRecoTracklistEPEM.at(j)->GetEnergy() << endl;
cout << "particle 3: \t" << fRecoTracklistEPEM.at(k)->GetPdgCode() << ";\t pt = " << fRecoTracklistEPEM.at(k)->GetPt() << ";\t X = " << fRecoTracklistEPEM.at(k)->GetStartX() << ";\t Y = " << fRecoTracklistEPEM.at(k)->GetStartY() << ";\t Z = " << fRecoTracklistEPEM.at(k)->GetStartZ() << ";\t E = " << fRecoTracklistEPEM.at(k)->GetEnergy() << endl;
cout << "particle 4: \t" << fRecoTracklistEPEM.at(l)->GetPdgCode() << ";\t pt = " << fRecoTracklistEPEM.at(l)->GetPt() << ";\t X = " << fRecoTracklistEPEM.at(l)->GetStartX() << ";\t Y = " << fRecoTracklistEPEM.at(l)->GetStartY() << ";\t Z = " << fRecoTracklistEPEM.at(l)->GetStartZ() << ";\t E = " << fRecoTracklistEPEM.at(l)->GetEnergy() << endl;

						Double_t opening_angle1_mc = 0;
						Double_t opening_angle2_mc = 0;
						Double_t opening_angle1_refitted = 0;
						Double_t opening_angle2_refitted = 0;
						
						if(motherId1 == motherId2) {
							opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j));
							opening_angle2_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));
							OpeningAngleMCgg->Fill(opening_angle1_mc);
							OpeningAngleMCgg->Fill(opening_angle2_mc);
							opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j));
							opening_angle2_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
							OpeningAngleRecogg->Fill(opening_angle1_refitted);
							OpeningAngleRecogg->Fill(opening_angle2_refitted);
						}
						if(motherId1 == motherId3) {
							opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(k));
							opening_angle2_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(l));
							OpeningAngleMCgg->Fill(opening_angle1_mc);
							OpeningAngleMCgg->Fill(opening_angle2_mc);
							opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(k));
							opening_angle2_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(l));
							OpeningAngleRecogg->Fill(opening_angle1_refitted);
							OpeningAngleRecogg->Fill(opening_angle2_refitted);
						}
						if(motherId1 == motherId4) {
							opening_angle1_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(l));
							opening_angle2_mc = CalculateOpeningAngleMC(fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k));
							OpeningAngleMCgg->Fill(opening_angle1_mc);
							OpeningAngleMCgg->Fill(opening_angle2_mc);
							opening_angle1_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(l));
							opening_angle2_refitted = CalculateOpeningAngleReco(fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k));
							OpeningAngleRecogg->Fill(opening_angle1_refitted);
							OpeningAngleRecogg->Fill(opening_angle2_refitted);
						}
						Double_t openingAngleBetweenGammas = CalculateOpeningAngleBetweenGammasMC(fRecoTracklistEPEM.at(i), fRecoTracklistEPEM.at(j), fRecoTracklistEPEM.at(k), fRecoTracklistEPEM.at(l));
//						fhEPEM_openingAngle_betweenGammas_mc->Fill(openingAngleBetweenGammas);
						Double_t openingAngleBetweenGammasReco = CalculateOpeningAngleBetweenGammasReco(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
//						fhEPEM_openingAngle_betweenGammas_reco->Fill(openingAngleBetweenGammasReco);
						CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(fRecoRefittedMomentum.at(i), fRecoRefittedMomentum.at(j), fRecoRefittedMomentum.at(k), fRecoRefittedMomentum.at(l));
						}
					}
				}
			}
		}
	}
							cout << " " << endl;
}

	// ========================================================================================

	// ========================================================================================

Double_t CbmAnaConversion2Reconstruction::Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2)
// calculation of invariant mass from four electrons/positrons
{
	TLorentzVector lorVec1;
	mctrack1->Get4Momentum(lorVec1);
    
	TLorentzVector lorVec2;
	mctrack2->Get4Momentum(lorVec2); 

	TLorentzVector sum;
	sum = lorVec1 + lorVec2; 

	return sum.Mag();
}

Double_t CbmAnaConversion2Reconstruction::Invmass_2particlesRECO(const TVector3 part1, const TVector3 part2)
// calculation of invariant mass from four electrons/positrons
{
	Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
	TLorentzVector lorVec1(part1, energy1);

	Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
	TLorentzVector lorVec2(part2, energy2);

	TLorentzVector sum;
	sum = lorVec1 + lorVec2;    

	return sum.Mag();
}

Double_t CbmAnaConversion2Reconstruction::Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4)
// calculation of invariant mass from four electrons/positrons
{
	TLorentzVector lorVec1;
	mctrack1->Get4Momentum(lorVec1);
    
	TLorentzVector lorVec2;
	mctrack2->Get4Momentum(lorVec2);
    
	TLorentzVector lorVec3;
	mctrack3->Get4Momentum(lorVec3);
    
	TLorentzVector lorVec4;
	mctrack4->Get4Momentum(lorVec4);

	TLorentzVector sum;
	sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;    

	return sum.Mag();
}

Double_t CbmAnaConversion2Reconstruction::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
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

Int_t CbmAnaConversion2Reconstruction::NofDaughters(Int_t motherId) 
{
	Int_t nofDaughters = 0;
	for(unsigned int i=0; i<fRecoTracklistEPEM.size(); i++) {
		Int_t motherId_temp = fRecoTracklistEPEM.at(i)->GetMotherId();
		if(motherId == motherId_temp) nofDaughters++;
	}
	return nofDaughters;
}


CbmLmvmKinematicParams CbmAnaConversion2Reconstruction::CalculateKinematicParamsReco(const TVector3 electron1, const TVector3 electron2)
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


CbmLmvmKinematicParams CbmAnaConversion2Reconstruction::CalculateKinematicParams_4particles(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
{
	CbmLmvmKinematicParams params;

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

    TVector3 momPair = part1 + part2 + part3 + part4;
    Double_t energyPair = energy1 + energy2 + energy3 + energy4;
    Double_t ptPair = momPair.Perp();
    Double_t pzPair = momPair.Pz();
    Double_t yPair = 0.5*TMath::Log((energyPair+pzPair)/(energyPair-pzPair));
    Double_t minv = sum.Mag();

    params.fMomentumMag = momPair.Mag();
    params.fPt = ptPair;
    params.fRapidity = yPair;
    params.fMinv = minv;
    params.fAngle = 0;
    return params;
}




Double_t CbmAnaConversion2Reconstruction::CalculateOpeningAngleReco(TVector3 electron1, TVector3 electron2)
{
	Double_t energyP = TMath::Sqrt(electron1.Mag2() + M2E);
	TLorentzVector lorVecP(electron1, energyP);

	Double_t energyM = TMath::Sqrt(electron2.Mag2() + M2E);
	TLorentzVector lorVecM(electron2, energyM);

	Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
	Double_t theta = 180.*anglePair/TMath::Pi();

	cout << "CalculateOpeningAngleReco = " << theta << endl;

	return theta;
}


Double_t CbmAnaConversion2Reconstruction::CalculateOpeningAngleMC(CbmMCTrack* mctrack1, CbmMCTrack* mctrack2)
{
	TVector3 electron1;
	mctrack1->GetMomentum(electron1);
	Double_t energyP = TMath::Sqrt(electron1.Mag2() + M2E);
	TLorentzVector lorVecP(electron1, energyP);

	TVector3 electron2;
	mctrack2->GetMomentum(electron2);
	Double_t energyM = TMath::Sqrt(electron2.Mag2() + M2E);
	TLorentzVector lorVecM(electron2, energyM);

	Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
	Double_t theta = 180.*anglePair/TMath::Pi();

	cout << "CalculateOpeningAngleMC = " << theta << "\t" << mctrack1->GetPdgCode() << " / " << mctrack2->GetPdgCode() << "\t" << "pt:    " << mctrack1->GetPt() << " / " << mctrack2->GetPt() << endl;

	return theta;
}

Double_t CbmAnaConversion2Reconstruction::CalculateOpeningAngleBetweenGammasMC(CbmMCTrack* mctrack1, CbmMCTrack* mctrack2, CbmMCTrack* mctrack3, CbmMCTrack* mctrack4)
{
	Double_t openingAngle;
	TLorentzVector gamma1;
	TLorentzVector gamma2;

	if(mctrack1->GetMotherId() == mctrack2->GetMotherId() && mctrack3->GetMotherId() == mctrack4->GetMotherId()) {
		CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(mctrack1->GetMotherId());
		mother1->Get4Momentum(gamma1);
		CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(mctrack3->GetMotherId());
		mother2->Get4Momentum(gamma2);
	}
	if(mctrack1->GetMotherId() == mctrack3->GetMotherId() && mctrack2->GetMotherId() == mctrack4->GetMotherId()) {
		CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(mctrack1->GetMotherId());
		mother1->Get4Momentum(gamma1);
		CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(mctrack2->GetMotherId());
		mother2->Get4Momentum(gamma2);
	}
	if(mctrack1->GetMotherId() == mctrack4->GetMotherId() && mctrack2->GetMotherId() == mctrack3->GetMotherId()) {
		CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(mctrack1->GetMotherId());
		mother1->Get4Momentum(gamma1);
		CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(mctrack2->GetMotherId());
		mother2->Get4Momentum(gamma2);
	}

	Double_t angle = gamma1.Angle(gamma2.Vect()); 
	openingAngle = 180.*angle/TMath::Pi();

	return openingAngle;
}


Double_t CbmAnaConversion2Reconstruction::CalculateOpeningAngleBetweenGammasReco(TVector3 electron1, TVector3 electron2, TVector3 electron3, TVector3 electron4)
{
	Double_t energy1 = TMath::Sqrt(electron1.Mag2() + M2E);
	TLorentzVector lorVec1(electron1, energy1);
	
	Double_t energy2 = TMath::Sqrt(electron2.Mag2() + M2E);
	TLorentzVector lorVec2(electron2, energy2);
	
	Double_t energy3 = TMath::Sqrt(electron3.Mag2() + M2E);
	TLorentzVector lorVec3(electron3, energy3);
	
	Double_t energy4 = TMath::Sqrt(electron4.Mag2() + M2E);
	TLorentzVector lorVec4(electron4, energy4);

	TLorentzVector lorPhoton1 = lorVec1 + lorVec2;
	TLorentzVector lorPhoton2 = lorVec3 + lorVec4;

	Double_t angleBetweenPhotons = lorPhoton1.Angle(lorPhoton2.Vect());
	Double_t theta = 180.*angleBetweenPhotons/TMath::Pi();

	return theta;
}
































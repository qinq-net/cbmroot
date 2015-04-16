/**
 * \file CbmAnaConversionKF.cxx
 *
 * This class only extract some results from the KFParticle package,
 * which are relevant for the conversion analysis.
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2014
 **/

#include "CbmAnaConversionKF.h"


// included from CbmRoot
#include "FairRootManager.h"
#include "CbmKFParticleFinder.h"
#include "CbmKFParticleFinderQA.h"
#include "KFParticleTopoReconstructor.h"
#include "CbmStsTrack.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"



using namespace std;



CbmAnaConversionKF::CbmAnaConversionKF()
 : fKFMcParticles(NULL),
   fMcTracks(NULL),
   fStsTracks(NULL),
   fStsTrackMatches(NULL),
   fKFparticle(NULL),
   fKFparticleFinderQA(NULL),
   fKFtopo(NULL),
   trackindexarray(),
   particlecounter(0),
   particlecounter_2daughters(0),
   particlecounter_3daughters(0),
   particlecounter_4daughters(0),
   particlecounter_all(0),
   fhPi0_NDaughters(NULL),
   fNofGeneratedPi0_allEvents(0),
   fNofPi0_kfparticle_allEvents(0),
   fNofGeneratedPi0(0),
   fNofPi0_kfparticle(0),
   fhPi0Ratio(NULL),
   fhPi0_mass(NULL),
   fSignalIds(),
   fGhostIds(),
   fHistoList_kfparticle(),
   timer(),
   fTime(0.)
{
}

CbmAnaConversionKF::~CbmAnaConversionKF()
{
}


void CbmAnaConversionKF::Init()
{
	FairRootManager* ioman = FairRootManager::Instance();
	if (NULL == ioman) { Fatal("CbmAnaConversionKF::Init","RootManager not instantised!"); }

	fKFMcParticles = (TClonesArray*) ioman->GetObject("KFMCParticles");
	if ( NULL == fKFMcParticles) { Fatal("CbmAnaConversionKF::Init","No KFMCParticles array!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( NULL == fMcTracks) { Fatal("CbmAnaConversionKF::Init","No MCTrack array!"); }

	fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	if ( NULL == fStsTracks) { Fatal("CbmAnaConversionKF::Init","No StsTrack array!"); }

	fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
	if (NULL == fStsTrackMatches) { Fatal("CbmAnaConversionKF::Init","No StsTrackMatch array!"); }
	

	fKFtopo = fKFparticle->GetTopoReconstructor();

	InitHistos();
}


void CbmAnaConversionKF::InitHistos()
{
	fHistoList_kfparticle.clear();

	// #############################################
	// Histograms related to KFParticle results
	fhPi0_NDaughters	= new TH1D("fhPi0_NDaughters","fhPi0_NDaughters;number of daughers;#", 4, 0.5, 4.5);
	fhPi0Ratio			= new TH1D("fhPi0Ratio", "fhPi0Ratio; ratio;#", 1000, 0., 0.02);
	fhPi0_mass			= new TH1D("fhPi0_mass", "fhPi0_mass;mass;#", 500, 0., 0.5);
	fHistoList_kfparticle.push_back(fhPi0_NDaughters);
	fHistoList_kfparticle.push_back(fhPi0Ratio);
	fHistoList_kfparticle.push_back(fhPi0_mass);


}







void CbmAnaConversionKF::Finish()
{
	//gDirectory->cd("analysis-conversion");
	gDirectory->mkdir("KF");
	gDirectory->cd("KF");
	for (Int_t i = 0; i < fHistoList_kfparticle.size(); i++){
		fHistoList_kfparticle[i]->Write();
	}
	gDirectory->cd("..");

	cout << "CbmAnaConversionKF: Realtime - " << fTime << endl;
}




void CbmAnaConversionKF::Exec()
{
	//KFParticle_Analysis();
	test2();

}



void CbmAnaConversionKF::SetKF(CbmKFParticleFinder* kfparticle, CbmKFParticleFinderQA* kfparticleQA)
{
	fKFparticle = kfparticle;
	fKFparticleFinderQA = kfparticleQA;
	if(fKFparticle) {
		cout << "CbmAnaConversionKF: kf works" << endl;
	}
	else {
		cout << "CbmAnaConversionKF: kf works not" << endl;
	}
}


void CbmAnaConversionKF::SetSignalIds(std::vector<int> *signalids) 
{  
	fSignalIds.clear();
	fSignalIds = *signalids;
}

void CbmAnaConversionKF::SetGhostIds(std::vector<int> *ghostids) 
{
	fGhostIds.clear();
	fGhostIds = *ghostids;
}



/*
void CbmAnaConversionKF::KFParticle_Analysis()
{
	timer.Start();

	int testkf = fKFtopo->NPrimaryVertices();
	cout << "KFParticle_Analysis - test kf NPrimaryVertices: " << testkf << endl;

	const KFPTrackVector* kftrackvector;
	kftrackvector = fKFtopo->GetTracks();
	cout << "KFParticle_Analysis - trackvector: " << kftrackvector->Size() << endl;
	KFParticle testparticle;
	KFPTrack testtrack;
	const int bla = 2;
	kftrackvector->GetTrack(testtrack, bla);
	cout << "KFParticle_Analysis - test p: " << testtrack.GetP() << endl;
	kfvector_int pdgvector;
	pdgvector = kftrackvector->PDG();
	//cout << "pdg tests: " << pdgvector[1] << "\t" << pdgvector[2] << "\t" << pdgvector[3] << "\t" << pdgvector[4] << endl;
	
	
	int nofpions = 0;
	//nofpions = kftrackvector->NPions();
	cout << "KFParticle_Analysis - number of pions: " << nofpions << endl;
	
	int testpi = 0;
	for(int i=0; i<kftrackvector->Size(); i++) {
		if(TMath::Abs(pdgvector[i]) == 211) testpi++;
	}
	cout << "KFParticle_Analysis - testpi: " << testpi << endl;
	
	//kftrackvector->PrintTracks();
	
	
	vector<KFParticle> particlevector;
	particlevector = fKFtopo->GetParticles();
	

	cout << "KFParticle_Analysis - particlevector size: " << particlevector.size() << "\t";
	for(int i=0; i<particlevector.size(); i++) {
		if(particlevector[i].NDaughters() > 3) {
			cout << particlevector[i].NDaughters() << "\t";
		}
		if(particlevector[i].GetPDG() == 111) {
			cout << "blubb" << particlevector[i].NDaughters() << "\t";
			if(particlevector[i].NDaughters() > 2) {
				particlecounter++;
			}
			
			std::vector<int> daughterid;
			daughterid = particlevector[i].DaughterIds();
			cout << "daughterids: ";
			for(int j=0; j<daughterid.size(); j++) {
				cout << daughterid[j] << "/";
				cout << "pdg-" << particlevector[daughterid[j]].GetPDG() << "-";
			}
			particlecounter_all++;
		}
	}
	cout << endl;
	
	
	
	cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
	cout << "KFParticle_Analysis - SignalIds size: " << fSignalIds.size() << "\t";
	for(int i=0; i<fSignalIds.size(); i++) {
		Int_t pdg = (fKFtopo->GetParticles()[fSignalIds[i]]).GetPDG();
		Int_t daughters = (fKFtopo->GetParticles()[fSignalIds[i]]).NDaughters();
		vector<int> daughterIds = (fKFtopo->GetParticles()[fSignalIds[i]]).DaughterIds();
		float mass = 0;
		float mass_sigma = 0;
		(fKFtopo->GetParticles()[fSignalIds[i]]).GetMass(mass, mass_sigma);
		if(pdg == 111) {
			cout << "CbmAnaConversionKF: pi0 found, signal id: " << (fKFtopo->GetParticles()[fSignalIds[i]]).Id() << "\t daughters: " << daughters << "\t";
			cout << "daughter ids: ";
			Int_t electronids[4] = {0};
			for(int j=0; j<daughterIds.size(); j++) {
				vector<int> granddaughterIds = (fKFtopo->GetParticles()[daughterIds[j]]).DaughterIds();
				cout << daughterIds[j] << " (" << granddaughterIds[0] << ",pdg" << (fKFtopo->GetParticles()[granddaughterIds[0]]).GetPDG() << "/" << granddaughterIds[1] << ",pdg" << (fKFtopo->GetParticles()[granddaughterIds[1]]).GetPDG() << ")" << " / ";
				electronids[j*2] = granddaughterIds[0];
				electronids[j*2+1] = granddaughterIds[1];
			}
			cout << endl;

			cout << "the 4 electrons and their grandmotherids: ";
			CbmMCTrack * electrontracks[4];
			for(int k=0; k<4; k++) {
				electrontracks[k] = (CbmMCTrack*) fMcTracks->At(electronids[k]);
				CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(electrontracks[k]->GetMotherId());
				if(mother == NULL) continue;
				CbmMCTrack* grandmother = (CbmMCTrack*) fMcTracks->At(mother->GetMotherId());
				if(grandmother == NULL) continue;
				cout << "gmid" << mother->GetMotherId() << "pdg" << grandmother->GetPdgCode() << " / ";
				
			}
			cout << endl;

			particlecounter++;
			if(daughters == 2) { particlecounter_2daughters++; }
			if(daughters == 3) { particlecounter_3daughters++; }
			if(daughters == 4) { particlecounter_4daughters++; }
			fhPi0_NDaughters->Fill(daughters);
			fhPi0_mass->Fill(mass);
			fNofPi0_kfparticle++;
		}
	}
	
	cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
	cout << "KFParticle_Analysis - GhostIds size: " << fGhostIds.size() << "\t";
	for(int i=0; i<fGhostIds.size(); i++) {
		Int_t pdg = (fKFtopo->GetParticles()[fGhostIds[i]]).GetPDG();
		Int_t daughters = (fKFtopo->GetParticles()[fGhostIds[i]]).NDaughters();
		if(pdg == 111) {
			cout << "pi0 found!\t";
			particlecounter++;
			if(daughters == 2) { particlecounter_2daughters++; }
			if(daughters == 3) { particlecounter_3daughters++; }
			if(daughters == 4) { particlecounter_4daughters++; }
			//fhPi0_NDaughters->Fill(daughters);
		}
	}
	
	cout << endl;
	cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
	
	
	

//	trackindexarray.clear();
//	trackindexarray = fKFtopo->GetPVTrackIndexArray();
//	cout << "trackindexarray: " << trackindexarray.size() << endl;
	
//	KFVertex vertex;
//	vertex = fKFtopo->GetPrimKFVertex();
//	int testnof = 0;
//	testnof = vertex.GetNContributors();
//	cout << "nof contributors: " << testnof << endl;
	
//	KFParticle particle;
//	particle = fKFtopo->GetPrimVertex();
	
	
//	for(int i=0; i < kftrackvector->Size(); i++) {
		//const kfvector_int& tempPDG;
		//tempPDG = kftrackvector->PDG;
		//fKFVertex->At(trackindexarray[i]);
	
//	}



	test();

	timer.Stop();
	fTime += timer.RealTime();
}
*/


void CbmAnaConversionKF::test() {
	int nofparticles = fKFMcParticles->GetEntriesFast();
	cout << "CbmAnaConversionKF: test nof " << nofparticles << endl;
	for(int i=0; i<nofparticles; i++) {
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fKFMcParticles->At(i);
		int pdg = TMath::Abs(mcTrack1->GetPdgCode());
		if(pdg == 111) {
			cout << "CbmAnaConversionKF: test successful!" << endl;
		}
	}


	vector< vector<int> > ids;
	const vector<KFParticle>& particles = fKFparticle->GetTopoReconstructor()->GetParticles();
	for(int iPart=0; iPart<fSignalIds.size(); iPart++)
	{
		if(particles[fSignalIds[iPart]].GetPDG() != 111) continue;
		//some cuts on pi0 if needed
  
		const KFParticle& pi0 = particles[fSignalIds[iPart]];
		vector<int> electrons;
		for(int iGamma=0; iGamma<pi0.NDaughters(); iGamma++) {
			const int GammaID = pi0.DaughterIds()[iGamma];
			const KFParticle& Gamma = particles[GammaID];
			for(int iElectron=0; iElectron<Gamma.NDaughters(); iElectron++) {
				int ElectronID = Gamma.DaughterIds()[iElectron];
				const KFParticle& Electron = particles[ElectronID];
				int STStrackID = Electron.DaughterIds()[0];
				electrons.push_back(STStrackID);
			}
		}
		ids.push_back(electrons);
	}

	if(ids.size() > 0) {
		cout << "NEW TEST: (sts ids) ";
		for(int i=0; i<ids.size(); i++) {
			for(int j=0; j<4; j++) {
				cout << " " << ids[i][j];
			}
			cout << " | ";
		}
		cout << endl;


	cout << "MC-pdgs: ";
	CbmMCTrack* mcTracks[4];
	for(int i=0; i<ids.size(); i++) {
		for(int j=0; j<4; j++) {
			CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(ids[i][j]);
			if (stsTrack == NULL) return;
			CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*)fStsTrackMatches->At(ids[i][j]);
			if (stsMatch == NULL) return;
			int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
			if (stsMcTrackId < 0) return;
			mcTracks[j] = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
			if (mcTracks[j] == NULL) return;
			
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(mcTracks[j]->GetMotherId());
			CbmMCTrack* grandmother = (CbmMCTrack*) fMcTracks->At(mother->GetMotherId());
			
	
			cout << " " << stsMcTrackId << "/" << mcTracks[j]->GetPdgCode() << "(motherid" << mcTracks[j]->GetMotherId() << ",motherpdg" << mother->GetPdgCode() << ",grandmotherpdg" << grandmother->GetPdgCode() << ",grandmotherid" << mother->GetMotherId() << ")";
		}
	}
	cout << endl;
	Double_t mass = Invmass_4particles(mcTracks[0], mcTracks[1], mcTracks[2], mcTracks[3]);
	cout << "mass: " << mass << endl;

	}

}


Double_t CbmAnaConversionKF::Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4)
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
    cout << "mc: \t" << sum.Px() << " / " << sum.Py() << " / " << sum.Pz() << " / " << sum.E() << "\t => mag = " << sum.Mag() << endl;

	return sum.Mag();
}




void CbmAnaConversionKF::test2()
{
	const KFPTrackVector* kftrackvector;
	kftrackvector = fKFtopo->GetTracks();
	cout << "CbmAnaConversionKF: size of kftrackvector: " << kftrackvector->Size() << endl;

}




/**
 * \file CbmAnaConversionRecoFull.h
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2015
 **/
 
 
#ifndef CBM_ANA_CONVERSION_RECOFULL
#define CBM_ANA_CONVERSION_RECOFULL


// included from ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TClonesArray.h>
#include <TStopwatch.h>

// included from CbmRoot
#include "CbmMCTrack.h"
#include "CbmVertex.h"
#include "CbmKFVertex.h"
#include "../../littrack/cbm/elid/CbmLitGlobalElectronId.h"
#include "../dielectron/CbmLmvmKinematicParams.h"




using namespace std;


class CbmAnaConversionRecoFull
{

public:
	CbmAnaConversionRecoFull();
	virtual ~CbmAnaConversionRecoFull();

	void Init();
	void InitHistos();
	void Finish();
	void Exec();

	void CombineElectrons(vector<CbmGlobalTrack*> gtrack, vector<TVector3> momenta, vector<float> momentaChi, vector<int> mctrackID, vector< vector<int> > reconstructedPhotons, Int_t index);
	Double_t Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	Double_t Pt_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	Double_t Rap_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);

	CbmLmvmKinematicParams CalculateKinematicParamsReco(const TVector3 electron1, const TVector3 electron2);
	CbmLmvmKinematicParams CalculateKinematicParams_4particles(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	
	void CombinePhotons(vector<CbmGlobalTrack*> gtrack, vector<TVector3> momenta, vector<float> momentaChi, vector<int> mctrackID, vector< vector<int> > reconstructedPhotons, Int_t index);
	Double_t OpeningAngleBetweenPhotons2(vector<int> photon1, vector<int> photons2);
	Double_t OpeningAngleBetweenPhotons(vector<TVector3> momenta, vector<int> photon1, vector<int> photons2);

	//void CombineElectronsRefit();
	//void CombinePhotonsRefit();
	//Double_t OpeningAngleBetweenPhotonsRefit(vector<int> photon1, vector<int> photons2);

	void CombinePhotonsDirection();

	void CombineElectrons();
	void CombinePhotons();
	
	void MixedEventTest();

private:
	TClonesArray* fRichPoints;
	TClonesArray* fRichRings;
	TClonesArray* fRichRingMatches;
	TClonesArray* fMcTracks;
	TClonesArray* fStsTracks;
	TClonesArray* fStsTrackMatches;
	TClonesArray* fGlobalTracks;
	CbmVertex *fPrimVertex;
	CbmKFVertex fKFVertex;

	vector<TH1*> fHistoList_recofull;
	vector<TH1*> fHistoList_recofull_1;
	vector<TH1*> fHistoList_recofull_2;
	vector<TH1*> fHistoList_recofull_3;
	vector<TH1*> fHistoList_recofull_4;
	vector<TH1*> fHistoList_recofull_new[5];

	TH1I * fhElectrons;

	CbmLitGlobalElectronId* electronidentifier;
	
	TH1D * fhMomentumFits;
	TH1D * fhMomentumFits_electronRich;
	TH1D * fhMomentumFits_pi0reco;

	vector<CbmGlobalTrack*> fElectrons_track;
	vector<TVector3> fElectrons_momenta;
	vector<float> fElectrons_momentaChi;
	vector<int> fElectrons_mctrackID;
	
	
	
	vector<CbmGlobalTrack*>	fElectrons_track_1;
	vector<TVector3>		fElectrons_momenta_1;
	vector<float>			fElectrons_momentaChi_1;
	vector<int>				fElectrons_mctrackID_1;
	vector< vector<int> >	fVector_photons_pairs_1;
	
	vector<CbmGlobalTrack*>	fElectrons_track_2;
	vector<TVector3>		fElectrons_momenta_2;
	vector<float>			fElectrons_momentaChi_2;
	vector<int>				fElectrons_mctrackID_2;
	vector< vector<int> >	fVector_photons_pairs_2;
	
	vector<CbmGlobalTrack*>	fElectrons_track_3;
	vector<TVector3>		fElectrons_momenta_3;
	vector<float>			fElectrons_momentaChi_3;
	vector<int>				fElectrons_mctrackID_3;
	vector< vector<int> >	fVector_photons_pairs_3;
	
	vector<CbmGlobalTrack*>	fElectrons_track_4;
	vector<TVector3>		fElectrons_momenta_4;
	vector<float>			fElectrons_momentaChi_4;
	vector<int>				fElectrons_mctrackID_4;
	vector< vector<int> >	fVector_photons_pairs_4;
	
	vector<CbmGlobalTrack*>	fElectrons_track_new[5];
	vector<TVector3>		fElectrons_momenta_new[5];
	vector<float>			fElectrons_momentaChi_new[5];
	vector<int>				fElectrons_mctrackID_new[5];
	vector< vector<int> >	fVector_photons_pairs_new[5];


	TH1D * fhElectrons_invmass;
	TH1D * fhElectrons_invmass_cut;
	TH1D * fhElectrons_nofPerEvent;
	TH1D * fhPhotons_nofPerEvent;
	
	
	vector< vector<int> > fVector_photons_pairs;
	vector<TVector3> fVector_photons_momenta;
	TH1D * fhPhotons_invmass;
	TH1D * fhPhotons_invmass_cut;
	TH1D * fhPhotons_invmass_cut_chi1;
	TH1D * fhPhotons_invmass_cut_chi3;
	TH1D * fhPhotons_invmass_cut_chi5;
	TH1D * fhPhotons_invmass_cut_chi10;
	TH1D * fhPhotons_invmass_cut_chi25;
	TH1D * fhPhotons_invmass_cut_chi40;
	TH1D * fhPhotons_invmass_cut_chi65;
	TH1D * fhPhotons_invmass_cut_chi80;
	
	TH1D * fhPhotons_invmass_cut_ptBin1;
	TH1D * fhPhotons_invmass_cut_ptBin2;
	TH1D * fhPhotons_invmass_cut_ptBin3;
	TH1D * fhPhotons_invmass_cut_ptBin4;
	
	TH2D * fhPhotons_invmass_vs_chi;
	TH2D * fhPhotons_startvertex_vs_chi;
	TH1D * fhPhotons_angleBetween;
	TH2D * fhPhotons_invmass_vs_pt;
	TH2D * fhPhotons_rapidity_vs_pt;
	TH2D * fhPhotons_invmass_vs_openingAngle;
	TH2D * fhPhotons_openingAngle_vs_momentum;
	
	TH1D * fhPhotons_MC_motherpdg;
	TH1D * fhPhotons_MC_invmass1;
	TH1D * fhPhotons_MC_invmass2;
	TH1D * fhPhotons_MC_invmass3;
	TH1D * fhPhotons_MC_invmass4;
	TH1D * fhPhotons_MC_startvertexZ;
	TH1D * fhPhotons_MC_motherIdCut;

	TH1D * fhPhotons_Refit_chiDiff;
	TH1D * fhPhotons_Refit_momentumDiff;
	TH1D * fhPhotons_Refit_chiDistribution;
	TH1D * fhPhotons_RefitPion_chiDistribution;



	vector<CbmGlobalTrack*> fElectrons_track_refit;
	vector<TVector3> fElectrons_momenta_refit;
	vector< vector<int> > fVector_photons_pairs_refit;
	TH1D * fhPhotons_invmass_refit;
	TH1D * fhPhotons_invmass_refit_cut;
	
	vector< vector<int> > fVector_photons_pairs_direction;
	TH1D * fhPhotons_invmass_direction;
	TH1D * fhPhotons_invmass_direction_cut;
	TH1D * fhPhotons_boostAngle;
	TH1D * fhPhotons_boostAngleMC;
	TH1D * fhPhotons_boostAngleTest;
	
	TH1D * fhPhotons_tX;
	TH1D * fhPhotons_tY;
	
	// histograms for index = 1
	TH1D * fhElectrons_nofPerEvent_1;
	TH1D * fhPhotons_nofPerEvent_1;
	TH1D * fhPhotons_invmass_1;
	TH1D * fhPhotons_invmass_ptBin1_1;
	TH1D * fhPhotons_invmass_ptBin2_1;
	TH1D * fhPhotons_invmass_ptBin3_1;
	TH1D * fhPhotons_invmass_ptBin4_1;
	TH1D * fhElectrons_invmass_1;
	TH2D * fhPhotons_invmass_vs_pt_1;
	
	// histograms for index = 2
	TH1D * fhElectrons_nofPerEvent_2;
	TH1D * fhPhotons_nofPerEvent_2;
	TH1D * fhPhotons_invmass_2;
	TH1D * fhPhotons_invmass_ptBin1_2;
	TH1D * fhPhotons_invmass_ptBin2_2;
	TH1D * fhPhotons_invmass_ptBin3_2;
	TH1D * fhPhotons_invmass_ptBin4_2;
	TH1D * fhElectrons_invmass_2;
	TH2D * fhPhotons_invmass_vs_pt_2;
	
	// histograms for index = 3
	TH1D * fhElectrons_nofPerEvent_3;
	TH1D * fhPhotons_nofPerEvent_3;
	TH1D * fhPhotons_invmass_3;
	TH1D * fhPhotons_invmass_ptBin1_3;
	TH1D * fhPhotons_invmass_ptBin2_3;
	TH1D * fhPhotons_invmass_ptBin3_3;
	TH1D * fhPhotons_invmass_ptBin4_3;
	TH1D * fhElectrons_invmass_3;
	TH2D * fhPhotons_invmass_vs_pt_3;
	
	// histograms for index = 4
	TH1D * fhElectrons_nofPerEvent_4;
	TH1D * fhPhotons_nofPerEvent_4;
	TH1D * fhPhotons_invmass_4;
	TH1D * fhPhotons_invmass_ptBin1_4;
	TH1D * fhPhotons_invmass_ptBin2_4;
	TH1D * fhPhotons_invmass_ptBin3_4;
	TH1D * fhPhotons_invmass_ptBin4_4;
	TH1D * fhElectrons_invmass_4;
	TH2D * fhPhotons_invmass_vs_pt_4;



	TH1D * fhElectrons_nofPerEvent_new[5];
	TH1D * fhPhotons_nofPerEvent_new[5];
	TH1D * fhPi0_nofPerEvent_new[5];
	TH1D * fhPhotons_invmass_new[5];
	TH1D * fhPhotons_invmass_ptBin1_new[5];
	TH1D * fhPhotons_invmass_ptBin2_new[5];
	TH1D * fhPhotons_invmass_ptBin3_new[5];
	TH1D * fhPhotons_invmass_ptBin4_new[5];
	TH1D * fhPhotons_invmass_ptBin5_new[5];
	TH1D * fhPhotons_invmass_ptBin6_new[5];
	TH1D * fhElectrons_invmass_new[5];
	TH2D * fhPhotons_invmass_vs_pt_new[5];
	TH1D * fhPhotons_invmass_MCcut1_new[5];
	TH1D * fhPhotons_invmass_MCcut2_new[5];
	TH1D * fhPhotons_invmass_MCcut3_new[5];
	TH1D * fhPhotons_invmass_MCcut4_new[5];
	TH2D * fhPhotons_pt_vs_rap_new[5];
	TH1D * fhElectrons_openingAngle_sameSign_new[5];



	// test with mixed event method for estimation of invariant mass background
	vector<TVector3>	fMixedEventsElectrons[5];
	TH1D *				fhMixedEventsTest_invmass[5];

	// timer
	TStopwatch timer;
	Double_t fTime;

	CbmAnaConversionRecoFull(const CbmAnaConversionRecoFull&);
	CbmAnaConversionRecoFull operator=(const CbmAnaConversionRecoFull&);

	ClassDef(CbmAnaConversionRecoFull,1)
};

#endif


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

	void CombineElectrons();
	Double_t Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	Double_t Pt_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);

	CbmLmvmKinematicParams CalculateKinematicParamsReco(const TVector3 electron1, const TVector3 electron2);
	
	void CombinePhotons();
	Double_t OpeningAngleBetweenPhotons(vector<int> photon1, vector<int> photons2);

	void CombineElectronsRefit();
	void CombinePhotonsRefit();
	Double_t OpeningAngleBetweenPhotonsRefit(vector<int> photon1, vector<int> photons2);


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


	TH1I * fhElectrons;

	CbmLitGlobalElectronId* electronidentifier;
	
	TH1D * fhMomentumFits;
	TH1D * fhMomentumFits_electronRich;
	TH1D * fhMomentumFits_pi0reco;

	vector<CbmGlobalTrack*> fElectrons_track;
	vector<TVector3> fElectrons_momenta;
	vector<float> fElectrons_momentaChi;
	vector<int> fElectrons_mctrackID;


	TH1D * fhElectrons_invmass;
	TH1D * fhElectrons_invmass_cut;
	
	
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
	TH2D * fhPhotons_invmass_vs_chi;
	TH2D * fhPhotons_startvertex_vs_chi;
	TH1D * fhPhotons_angleBetween;
	TH2D * fhPhotons_invmass_vs_pt;
	
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



	vector<CbmGlobalTrack*> fElectrons_track_refit;
	vector<TVector3> fElectrons_momenta_refit;
	vector< vector<int> > fVector_photons_pairs_refit;
	TH1D * fhPhotons_invmass_refit;
	TH1D * fhPhotons_invmass_refit_cut;

	// timer
	TStopwatch timer;
	Double_t fTime;

	CbmAnaConversionRecoFull(const CbmAnaConversionRecoFull&);
	CbmAnaConversionRecoFull operator=(const CbmAnaConversionRecoFull&);

	ClassDef(CbmAnaConversionRecoFull,1)
};

#endif

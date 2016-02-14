/**
 * \file CbmAnaConversionTest.h
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2015
 **/
 
 
#ifndef CBM_ANA_CONVERSION_TEST
#define CBM_ANA_CONVERSION_TEST

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
#include "CbmLmvmKinematicParams.h"
#include "CbmGlobalTrack.h"
#include "CbmLitGlobalElectronId.h"




using namespace std;


class CbmAnaConversionTest
{

public:
	CbmAnaConversionTest();
	virtual ~CbmAnaConversionTest();

	void Init();
	void InitHistos();
	void Finish();

	void Exec();

	void GetNofRichElectrons();
	Double_t CalcInvMass(Int_t e1, Int_t e2, Int_t e3, Int_t e4);

	void DoSTSonlyAnalysis();
	void CombineElectrons_FromSTSandRICH();
	void CombinePhotons();
	void CombineElectrons_FromRICH();
	CbmLmvmKinematicParams CalculateKinematicParamsReco(const TVector3 electron1, const TVector3 electron2);
	Double_t Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	
	void CombineElectrons_STSonly();
	void CombinePhotons_STSonly();
	
	Bool_t HasRichInd(Int_t gtIndex, Int_t arrayIndex);

	void MixedEventTest_3p1();
	void MixedEventTest_STSonly();



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

	vector<TH1*> fHistoList_test;	// list of all histograms related to rich rings

	CbmLitGlobalElectronId* electronidentifier;

	vector<int> fElectrons_gtid;
	vector<int> fElectrons_mcid;
	vector<int> fElectrons_richInd;
	vector<int> fElectrons_pi0mcid;
	vector<int> fElectrons_same;

	TH1I *fElectrons_nofPerPi0;
	TH1I *fElectrons_nofPerPi0_withRichInd;

	TH1D *fhElectronsTest_invmass;


	// histograms for STS only analysis
	TH1I * fhTest_ParticlesPerEvent;
	TH1I * fhTest_RICHelectronsPerEvent;
	TH1I * fhTest_PhotonsPerEvent_RICHonly;
	TH1I * fhTest_PhotonsPerEvent_STSandRICH;
	TH1I * fhTest_ReconstructedPi0PerEvent;
	TH1D * fhTest_invmass;
	TH1D * fhTest_invmass_pCut;

	// arrays for CombinePhotons_STSonly()
	TH1D * fhTest_invmass_RICHindex0;
	TH1D * fhTest_invmass_RICHindex1;
	TH1D * fhTest_invmass_RICHindex2;
	TH1D * fhTest_invmass_RICHindex3;
	TH1D * fhTest_invmass_RICHindex4;

	TH2D * fhTest_invmass_MCcutAll;

	// array with all refitted momenta
	vector<TVector3>	fVector_AllMomenta;


	// arrays for STS only analysis (i.e. signal only in STS is required, not necessarily in RICH)
	vector<CbmGlobalTrack*>	fVector_gt;
	vector<TVector3>		fVector_momenta;
	vector<double>			fVector_chi;
	vector<int>				fVector_gtIndex;
	vector<int>				fVector_richIndex;
	vector<int>				fVector_mcIndex;
	vector<bool>			fVector_withRichSignal;
	vector< vector<int> >	fVector_reconstructedPhotons_FromSTSandRICH;

	// arrays for electrons, that have been identified in RICH as electrons
	vector<CbmGlobalTrack*>	fVector_electronRICH_gt;
	vector<int>				fVector_electronRICH_gtIndex;
	vector<int>				fVector_electronRICH_mcIndex;
	vector<TVector3>		fVector_electronRICH_momenta;
	vector< vector<int> >	fVector_electronRICH_reconstructedPhotons;
	
	// additional arrays, for CombineElectrons_STSonly() and CombinePhotons_STSonly()
	vector< vector<int> >	fVector_reconstructedPhotons_STSonly;
	


	// EVENT MIXING 
	Int_t	globalEventNo;
//	for STS only mixed events creation
//	TH1D * fhTest_eventMixing_RICHindex0;
//	TH1D * fhTest_eventMixing_RICHindex1;
//	TH1D * fhTest_eventMixing_RICHindex2;
//	TH1D * fhTest_eventMixing_RICHindex3;
//	TH1D * fhTest_eventMixing_RICHindex4;
	vector< vector<TVector3> >	fMixedTest_STSonly_photons;
	vector<int>					fMixedTest_STSonly_eventno;
	vector< vector<bool> >		fMixedTest_STSonly_hasRichInd;
	TH1D * 						fhTest_eventMixing_STSonly_2p2;
	TH1D * 						fhTest_eventMixing_STSonly_3p1;
	TH1D * 						fhTest_eventMixing_STSonly_4p0;

//	for other event mixing (3 + 1)
	vector< vector<TVector3> >	fMixedTest_3p1_photons;
	vector<int>					fMixedTest_3p1_eventno;
	vector<int>					fMixedTest_3p1_combined;
	TH1D * 						fhTest_eventMixing_3p1;
	TH1D * 						fhTest_eventMixing_3p1_pCut;


	CbmAnaConversionTest(const CbmAnaConversionTest&);
	CbmAnaConversionTest operator=(const CbmAnaConversionTest&);

	ClassDef(CbmAnaConversionTest,1)
};

#endif

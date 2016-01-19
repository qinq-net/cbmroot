/**
 * \file CbmAnaConversionTest2.h
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2016
 **/
 
 
#ifndef CBM_ANA_CONVERSION_TEST2
#define CBM_ANA_CONVERSION_TEST2

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
#include "../dielectron/CbmLmvmKinematicParams.h"
#include "CbmGlobalTrack.h"
#include "../../littrack/cbm/elid/CbmLitGlobalElectronId.h"




using namespace std;


class CbmAnaConversionTest2
{

public:
	CbmAnaConversionTest2();
	virtual ~CbmAnaConversionTest2();

	void Init();
	void InitHistos();
	void Finish();

	void Exec();

	void InvariantMassTest_3RICH();
	Int_t NofDaughters(Int_t motherId);




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

	vector<TH1*> fHistoList_test2;	// list of all histograms related to rich rings


	// arrays for CombinePhotons_STSonly()
	TH1D * fhTest2_invmass_RICHindex0;
	TH1D * fhTest2_invmass_RICHindex1;
	TH1D * fhTest2_invmass_RICHindex2;
	TH1D * fhTest2_invmass_RICHindex3;
	TH1D * fhTest2_invmass_RICHindex4;




	// arrays for STS only analysis (i.e. signal only in STS is required, not necessarily in RICH)
	vector<CbmGlobalTrack*>	fVector_gt;
	vector<TVector3>		fVector_momenta;
	vector<CbmMCTrack*>		fVector_mctrack;
	vector<double>			fVector_chi;
	vector<int>				fVector_gtIndex;
	vector<int>				fVector_richIndex;



	TH1D * fhTest2_invmass_gee_mc;
	TH1D * fhTest2_invmass_gee_refitted;
	TH1D * fhTest2_invmass_gg_mc;
	TH1D * fhTest2_invmass_gg_refitted;
	TH1D * fhTest2_invmass_all_mc;
	TH1D * fhTest2_invmass_all_refitted;

	TH2D * fhTest2_pt_vs_rap_gee;
	TH2D * fhTest2_pt_vs_rap_gg;
	TH2D * fhTest2_pt_vs_rap_all;

	TH1D * fhPi0_startvertexElectrons_gee;
	TH1D * fhPi0_startvertexElectrons_gg;
	TH1D * fhPi0_startvertexElectrons_all;


	CbmAnaConversionTest2(const CbmAnaConversionTest2&);
	CbmAnaConversionTest2 operator=(const CbmAnaConversionTest2&);

	ClassDef(CbmAnaConversionTest2,1)
};

#endif

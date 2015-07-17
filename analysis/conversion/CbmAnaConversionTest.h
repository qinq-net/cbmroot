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






private:
	TClonesArray* fRichPoints;
	TClonesArray* fRichRings;
	TClonesArray* fRichRingMatches;
	TClonesArray* fMcTracks;
	TClonesArray* fStsTracks;
	TClonesArray* fStsTrackMatches;
	TClonesArray* fGlobalTracks;
	CbmVertex *fPrimVertex;

	vector<TH1*> fHistoList_test;	// list of all histograms related to rich rings


	vector<int> fElectrons_gtid;
	vector<int> fElectrons_mcid;
	vector<int> fElectrons_richInd;
	vector<int> fElectrons_pi0mcid;

	TH1I *fElectrons_nofPerPi0;
	TH1I *fElectrons_nofPerPi0_withRichInd;


	CbmAnaConversionTest(const CbmAnaConversionTest&);
	CbmAnaConversionTest operator=(const CbmAnaConversionTest&);

	ClassDef(CbmAnaConversionTest,1)
};

#endif

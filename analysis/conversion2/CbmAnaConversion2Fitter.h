/**
 * \file CbmAnaConversion2Fitter.h
 *
 * \author ??
 * \date ??
 **/
 
 
#ifndef CBM_ANA_CONVERSION2_FITTER
#define CBM_ANA_CONVERSION2_FITTER

// included from ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TClonesArray.h>

// included from CbmRoot
#include "CbmMCTrack.h"
#include "CbmVertex.h"


using namespace std;


class CbmAnaConversion2Fitter
{

public:
	CbmAnaConversion2Fitter();
	virtual ~CbmAnaConversion2Fitter();

	void Init();
	void InitHistograms();
	void Finish();
	void Exec(int fEventNum);

	void test();


private:
	TClonesArray* fMcTracks;
	TClonesArray* fStsTracks;
	TClonesArray* fStsTrackMatches;
	TClonesArray* fGlobalTracks;
	TClonesArray *fArrayStsHit;
	TClonesArray *fArrayMvdHit;

	CbmVertex* fPrimVertex;



	CbmAnaConversion2Fitter(const CbmAnaConversion2Fitter&);
	CbmAnaConversion2Fitter operator=(const CbmAnaConversion2Fitter&);

	ClassDef(CbmAnaConversion2Fitter,1)
};

#endif

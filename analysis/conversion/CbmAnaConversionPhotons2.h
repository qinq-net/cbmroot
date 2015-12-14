/**
 * \file CbmAnaConversionPhotons2.h
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2015
 **/
 
 
#ifndef CBM_ANA_CONVERSION_PHOTONS2
#define CBM_ANA_CONVERSION_PHOTONS2

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




using namespace std;


class CbmAnaConversionPhotons2
{

public:
	CbmAnaConversionPhotons2();
	virtual ~CbmAnaConversionPhotons2();

	void Init();
	void InitHistos();
	void Finish();
	void Exec();

	void AnalyseElectronsReco();
	void AnalyseElectronsRecoWithRICH();



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

	vector<TH1*> fHistoList_photons;			// list of all histograms related to rich rings
	vector<TH1*> fHistoList_photons_withRICH;	// list of all histograms related to rich rings


	vector<Int_t>			fRecoTracklist_gtIndex;
	vector<Int_t>			fRecoTracklist_mcIndex;
	vector<TVector3>		fRecoTracklist_momentum;
	vector<Double_t>		fRecoTracklist_chi;
	vector<CbmMCTrack*>		fRecoTracklist_mctrack;

	vector<Int_t>			fRecoTracklist_withRICH_gtIndex;
	vector<Int_t>			fRecoTracklist_withRICH_mcIndex;
	vector<TVector3>		fRecoTracklist_withRICH_momentum;
	vector<Double_t>		fRecoTracklist_withRICH_chi;
	vector<CbmMCTrack*>		fRecoTracklist_withRICH_mctrack;




	// distribution of opening angles from reconstructed momenta
	TH1D * fh2Electrons_angle_all;
	TH1D * fh2Electrons_angle_combBack;
	TH1D * fh2Electrons_angle_allSameG;

	// distribution of opening angles from reconstructed momenta, with application of opening angle cuts
	TH1D * fh2Electrons_angle_all_cuts;
	TH1D * fh2Electrons_angle_combBack_cuts;
	TH1D * fh2Electrons_angle_allSameG_cuts;
	
	// histogram for comparison of different opening angle cuts and their influence on signal and background amounts
	TH1I * fh2Electrons_angle_CUTcomparison;
	TH1I * fh2Electrons_angle_CUTcomparison_chi;
	
	TH1I * fh2Electrons_angle_CUTcomparison_withRICH;
	TH1I * fh2Electrons_angle_CUTcomparison_withRICH_chi;


	// distribution of invariant masses from reconstructed momenta
	TH1D * fh2Electrons_invmass_all;
	TH1D * fh2Electrons_invmass_combBack;
	TH1D * fh2Electrons_invmass_allSameG;

	// distribution of invariant masses from reconstructed momenta with cut on opening angle
	TH1D * fh2Electrons_invmass_all_cut;
	TH1D * fh2Electrons_invmass_combBack_cut;
	TH1D * fh2Electrons_invmass_allSameG_cut;

	// opening angle vs pt from reconstructed data
	TH2D * fh2Electrons_angleVSpt_all;
	TH2D * fh2Electrons_angleVSpt_combBack;
	TH2D * fh2Electrons_angleVSpt_allSameG;

	// invariant mass vs pt from reconstructed data
	TH2D * fh2Electrons_invmassVSpt_all;
	TH2D * fh2Electrons_invmassVSpt_combBack;
	TH2D * fh2Electrons_invmassVSpt_allSameG;



	// distribution of opening angles from reconstructed momenta, with RICH
	TH1D * fh2Electrons_angle_withRICH_all;
	TH1D * fh2Electrons_angle_withRICH_combBack;
	TH1D * fh2Electrons_angle_withRICH_allSameG;


	// opening angle vs pt from reconstructed data, with RICH
	TH2D * fh2Electrons_angleVSpt_withRICH_all;
	TH2D * fh2Electrons_angleVSpt_withRICH_combBack;
	TH2D * fh2Electrons_angleVSpt_withRICH_allSameG;


	// distribution of opening angles from reconstructed momenta, with RICH
	TH1D * fh2Electrons_invmass_withRICH_all;
	TH1D * fh2Electrons_invmass_withRICH_combBack;
	TH1D * fh2Electrons_invmass_withRICH_allSameG;


	// opening angle vs pt from reconstructed data, with RICH
	TH2D * fh2Electrons_invmassVSpt_withRICH_all;
	TH2D * fh2Electrons_invmassVSpt_withRICH_combBack;
	TH2D * fh2Electrons_invmassVSpt_withRICH_allSameG;

	// timer
	TStopwatch timer;
	Double_t fTime;

	CbmAnaConversionPhotons2(const CbmAnaConversionPhotons2&);
	CbmAnaConversionPhotons2 operator=(const CbmAnaConversionPhotons2&);

	ClassDef(CbmAnaConversionPhotons2,1)
};

#endif

/**
 * \file CbmAnaConversion2BG.h
 *
 * \author ??
 * \date ??
 **/
 
 
#ifndef CBM_ANA_CONVERSION2_BG
#define CBM_ANA_CONVERSION2_BG

// included from ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TClonesArray.h>
#include <TStopwatch.h>

// included from CbmRoot
#include "CbmStsTrack.h"
#include "CbmMCTrack.h"
#include "CbmVertex.h"
#include "CbmKFParticle.h"
#include "KFParticle.h"
#include "CbmLmvmKinematicParams.h"




using namespace std;


class CbmAnaConversion2BG
{

public:
	CbmAnaConversion2BG();
	CbmAnaConversion2BG(const CbmAnaConversion2BG&) = delete;
	CbmAnaConversion2BG operator=(const CbmAnaConversion2BG&) = delete;

	virtual ~CbmAnaConversion2BG();

	void Init();
	void InitHistos();
	void Finish();
	void Exec(CbmMCTrack* mctrack1, CbmMCTrack* mctrack2, CbmMCTrack* mctrack3, CbmMCTrack* mctrack4, TVector3 refmomentum1, TVector3 refmomentum2, TVector3 refmomentum3, TVector3 refmomentum4, Double_t invmassRecoPi0, TH1D* Case1, TH1D* Case2, TH1D* Case3, TH1D* Case4, TH1D* Case5, TH1D* Case6, TH1D* Case7, TH1D* Case8, TH1D* Case9, TH1D* Case10, TH1D* PdgCase8, TH1D* testsameMIDcase8, TH1D* testsameGRIDcase8, TH1D* PdgCase8mothers, TH1D* case8GRIDInvMassGamma, TH1D* case8GRIDOAGamma, TH2D* Case1ZYPos);
	Double_t Invmass_2particlesRECO(const TVector3 part1, const TVector3 part2);
	Double_t CalculateOpeningAngleReco(TVector3 electron1, TVector3 electron2);


private:
	TClonesArray* fKFMcParticles;
	TClonesArray* fMcTracks;
	TClonesArray* fStsTracks;
	TClonesArray* fStsTrackMatches;
	TClonesArray* fGlobalTracks;
	TClonesArray* fRichRings;
	TClonesArray* fRichRingMatches;



	// timer
	TStopwatch timer;
	Double_t fTime;

	ClassDef(CbmAnaConversion2BG,1)
};

#endif

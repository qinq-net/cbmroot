/**
 * \file CbmAnaConversion2Reconstruction.h
 *
 * \author ???
 * \date ????
 **/
 
 
#ifndef CBM_ANA_CONVERSION2_RECONSTRUCTION
#define CBM_ANA_CONVERSION2_RECONSTRUCTION

// included from ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TClonesArray.h>
#include <TStopwatch.h>

// included from CbmRoot
#include "CbmMCTrack.h"
#include "CbmVertex.h"
#include "CbmLitGlobalElectronId.h"
#include "CbmLmvmKinematicParams.h"



using namespace std;


class CbmAnaConversion2Reconstruction
{

public:
	CbmAnaConversion2Reconstruction();
	CbmAnaConversion2Reconstruction(const CbmAnaConversion2Reconstruction&) = delete;
	CbmAnaConversion2Reconstruction operator=(const CbmAnaConversion2Reconstruction&) = delete;
	virtual ~CbmAnaConversion2Reconstruction();

	void Init();
	void InitHistos();
	void Finish();


	void SetTracklistReco(vector<CbmMCTrack*> MCTracklist_CbmAnaConv2, vector<CbmMCTrack*> MCTracklist_CbmAnaConv2withRICH, vector<TVector3> reffitedMomentum_CbmAnaConv2, vector<TVector3> reffitedMomentum_CbmAnaConv2withRICH, int DecayedParticlePdg, int fEventNum, vector<Int_t> fmcTrack_STS_Id);
//	void SetTracklistReco(vector<CbmMCTrack*> MCTracklist_CbmAnaConv2, vector<int> ids_CbmAnaConv2, vector<Int_t> GlobalTrackId_CbmAnaConv2, vector<TVector3> reffitedMomentum_CbmAnaConv2);
	Double_t	Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4);
	Double_t	Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	Int_t NofDaughters(Int_t motherId);
	Double_t Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2);
	Double_t Invmass_2particlesRECO(const TVector3 part1, const TVector3 part2);
	CbmLmvmKinematicParams CalculateKinematicParamsReco(const TVector3 electron1, const TVector3 electron2);
	CbmLmvmKinematicParams CalculateKinematicParams_4particles(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	Double_t CalculateOpeningAngleReco(TVector3 electron1, TVector3 electron2);
	Double_t CalculateOpeningAngleMC(CbmMCTrack* mctrack1, CbmMCTrack* mctrack2);
	Double_t CalculateOpeningAngleBetweenGammasMC(CbmMCTrack* mctrack1, CbmMCTrack* mctrack2, CbmMCTrack* mctrack3, CbmMCTrack* mctrack4);
	Double_t CalculateOpeningAngleBetweenGammasReco(TVector3 electron1, TVector3 electron2, TVector3 electron3, TVector3 electron4);




private:

   TClonesArray* fRichRings;
   TClonesArray* fRichHits;
   TClonesArray* fMcTracks;
   TClonesArray* fGlobalTracks;

	vector<CbmMCTrack*>	fRecoTracklistEPEM;
	vector<int>			fRecoTracklistEPEM_ids;
	vector<Int_t>		fRecoTracklistEPEM_gtid;	// globalTrack Id
	vector<TVector3>	fRecoRefittedMomentum_dir;
	vector<TVector3>	fRecoRefittedMomentum_conv;
	vector<TVector3>	fRecoRefittedMomentum;
	vector<Int_t>		STS_Id; // mc match id

	vector<CbmMCTrack*>	fRecoTracklistEPEMwithRICH;
	vector<int>			fRecoTracklistEPEM_idswithRICH;
	vector<Int_t>		fRecoTracklistEPEM_gtidwithRICH;
	vector<TVector3>	fRecoRefittedMomentumwithRICH;

	vector<TH1*> fHistoList_gg;
	vector<TH1*> fHistoList_gee;
	vector<TH1*> fHistoList_eeee;

	TH1D * fhEPEM_openingAngle_gee_mc;
	TH1D * fhEPEM_openingAngle_gee_refitted;
	TH1D * fhEPEM_openingAngle_gee_mc_dalitz;
	TH1D * fhEPEM_openingAngle_gee_refitted_dalitz;
	TH1D * fhEPEM_openingAngle_betweenGammas_mc;
	TH1D * fhEPEM_openingAngle_betweenGammas_reco;
	TH1D * fhEPEM_invmass_eeee_refitted;
	TH1D * fhEPEM_invmass_eeee_mc;
	TH1D * OAsmallest;
	TH1D * OpeningAngleMCgg;
	TH1D * OpeningAngleRecogg;
	TH1D * fhEPEM_invmass_gg_mc;
	TH1D * fhEPEM_invmass_gg_refitted;
	TH1D * fhEPEM_invmass_gee_mc;
	TH1D * fhEPEM_invmass_gee_refitted;




	ClassDef(CbmAnaConversion2Reconstruction,1)
};

#endif

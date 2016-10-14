#ifndef CBMRICHMIRRORSORTINGCORRECTION_H
#define CBMRICHMIRRORSORTINGCORRECTION_H

#include "FairTask.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "TGeoNavigator.h"
#include "CbmRichRingLight.h"
#include "TVector3.h"
#include "CbmHistManager.h"

using namespace std;


class CbmRichMirrorSortingCorrection : public FairTask
{
public:
	/*
	 * Constructor.
	 */
	CbmRichMirrorSortingCorrection();

	/*
	 * Destructor.
	 */
    virtual ~CbmRichMirrorSortingCorrection();

    /**
     * \brief Inherited from FairTask.
     */
    virtual InitStatus Init();

    void InitHistProjection();

    void InitHistProjectionList();

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Exec(
    		Option_t* option);

    void GetPmtNormal(Int_t NofPMTPoints, vector<Double_t> &normalPMT, Double_t &normalCste);

    void ComputeR2(vector<Double_t> &ptR2Center, vector<Double_t> &ptR2Mirr, vector<Double_t> ptM, vector<Double_t> ptC, vector<Double_t> ptR1, TGeoNavigator* navi, TString option, TString mirrorTileName);

    void ComputeP(vector<Double_t> &ptPMirr, vector<Double_t> &ptPR2, vector<Double_t> normalPMT, vector<Double_t> ptM, vector<Double_t> ptR2Mirr, Double_t constantePMT);

    /*
	 * Function filling the diffX, diffY and distance histograms, from the outPos vector.
	 */
    void FillHistProjection(TVector3 outPosIdeal, TVector3 outPosUnCorr, TVector3 outPos, CbmRichRingLight ringLight, vector<Double_t> normalPMT, Double_t constantePMT, string str);

    void DrawHistProjection();

    void setOutputDir(TString s) { fOutputDir = s; }

    void setStudyName(TString s) { fStudyName = s; }

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Finish();


private:

    UInt_t fEventNb;
    CbmRichRingFitterCOP* fCopFit;
    CbmRichRingFitterEllipseTau* fTauFit;
    TString fOutputDir;
    TString fStudyName;
    CbmHistManager* fHM, *fHM2;
    std::map<string,TH1D*> fDiffHistoMap;
    Double_t fTrackCenterDistanceIdeal;
    Double_t fTrackCenterDistanceCorrected;
    Double_t fTrackCenterDistanceUncorrected;

    TClonesArray* fGlobalTracks;
    TClonesArray* fRichRings;
    TClonesArray* fMCTracks;
    TClonesArray* fMirrorPoints;
    TClonesArray* fRefPlanePoints;
    TClonesArray* fPmtPoints;
    TClonesArray* fRichProjections;
    TClonesArray* fTrackParams;
    TClonesArray* fRichRingMatches;
    TClonesArray* fStsTrackMatches;

    ClassDef(CbmRichMirrorSortingCorrection, 1);
};

#endif

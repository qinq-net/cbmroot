#ifndef CBMRICHMIRRORSORTING_H
#define CBMRICHMIRRORSORTING_H

#include "FairTask.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "CbmRichMirror.h"
#include "TGeoNavigator.h"

#include <vector>
#include <fstream>
#include <iomanip>

using namespace std;

class TClonesArray;
class TH1D;
class TH2D;


class CbmRichMirrorSorting : public FairTask
{
public:
	/*
	 * Constructor.
	 */
	CbmRichMirrorSorting();

	/*
	 * Destructor.
	 */
    virtual ~CbmRichMirrorSorting();

    /**
     * \brief Inherited from FairTask.
     */
    virtual InitStatus Init();

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Exec(
    		Option_t* option);

    void setOutputDir(TString s) { fOutputDir = s; }

    void ComputeAngles();

    void GetTrackPosition(Double_t &x, Double_t &y);

    void GetPmtNormal(Int_t NofPMTPoints, vector<Double_t> &normalPMT, Double_t &normalCste);

    void ComputeR2(vector<Double_t> &ptR2Center, vector<Double_t> &ptR2Mirr, vector<Double_t> ptM, vector<Double_t> ptC, vector<Double_t> ptR1, TGeoNavigator* navi, TString s);

    void ComputeP(vector<Double_t> &ptPMirr, vector<Double_t> &ptPR2, vector<Double_t> normalPMT, vector<Double_t> ptM, vector<Double_t> ptR2Mirr, Double_t constantePMT);

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Finish();


private:

    UInt_t fEventNb;
    TClonesArray* fGlobalTracks;
    TClonesArray* fRichRings;
    TClonesArray* fMCTracks;
    CbmRichRingFitterCOP* fCopFit;
    CbmRichRingFitterEllipseTau* fTauFit;
    TString fOutputDir;
    std::map<string, vector<CbmRichMirror*>> fMirrorMap;

    TClonesArray* fMirrorPoints;
    TClonesArray* fRefPlanePoints;
    TClonesArray* fPmtPoints;
    TClonesArray* fRichProjections;

    CbmRichMirrorSorting(const CbmRichMirrorSorting&);
    CbmRichMirrorSorting operator=(const CbmRichMirrorSorting&);

    ClassDef(CbmRichMirrorSorting, 1);
};

#endif

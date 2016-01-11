#ifndef CBMRICHCORRECTION_H
#define CBMRICHCORRECTION_H


#include "FairTask.h"
#include "CbmHistManager.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "CbmRichPoint.h"
#include "CbmRichRing.h"
#include "CbmRichRingLight.h"
#include <vector>
#include <map>
#include "TString.h"
#include "CbmRichRecGeoPar.h"
#include "TGeoNavigator.h"

using namespace std;

class TClonesArray;
class TH1D;
class TH2D;


class CbmRichCorrection : public FairTask
{
private:
    static const int kMAX_NOF_HITS = 100; // Maximum number of hits in ring

public:
	/*
	 * Constructor.
	 */
    CbmRichCorrection();

	/*
	 * Destructor.
	 */
    virtual ~CbmRichCorrection();

    /**
     * \brief Inherited from FairTask.
     */
    virtual InitStatus Init();

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Exec(
          Option_t* option);

    /**
     * \brief Inherited from FairTask.
     */
    virtual void Finish();

    /*
     * Histogram initialization.
     */
    void InitHist();

    /*
     * Fill the PMT plane with hits, ONLY for event with SEVERAL particles.
     */
    void MatchFinder();

    /*
     *
     */
    void FillPMTMap(const Char_t* mirr_path, CbmRichPoint* pPoint);

    /*
     *
     */
    void FillPMTMapEllipse(const Char_t* mirr_path, Float_t CenterX, Float_t CenterY);

    /*
     * From incoming track on the mirrors, do reflection of its trajectory and extrapolation of its intersection on the PMT plane.
     */
    void ProjectionProducer(TClonesArray* projectedPoint);

    /*
     * Get pmt normal from 3 different points on the plane.
     */
    void GetPmtNormal(Int_t NofPMTPoints, Double_t &normalX, Double_t &normalY, Double_t &normalZ, Double_t &normalCste);

    /*
     * Calculate mean sphere center coordinates from all the mirror tiles (to be used for the reconstruction step).
     */
    void GetMeanSphereCenter(TGeoNavigator *navi, Double_t &sphereX, Double_t &sphereY, Double_t &sphereZ);

    void RotateAndCopyHitsToRingLight(const CbmRichRing* ring1, CbmRichRingLight* ring2);

    /*
     * Draw histograms.
     */
    void DrawHist();

    /*
     * Draw histograms from root file.
     */
    void DrawHistFromFile(TString fileName);

    /*
     * Set output directory for images.
     */
    void SetOutputDir(TString dir) {fOutputDir = dir;}

    /*
     * Set run title. It is also a part of the file name of image files.
     */
    void SetRunTitle(TString title) {fRunTitle = title;}

    /*
     * Set to TRUE if you want to draw histograms.
     */
    void SetDrawHist(Bool_t b) {fDrawHist = b;}


private:
    TClonesArray* fRichHits; // Array of RICH hits
    TClonesArray* fRichRings; // Array of found RICH rings
    TClonesArray* fRichMirrorPoints;
    TClonesArray* fRichProjections;
    TClonesArray* fRichMCPoints;
    TClonesArray* fMCTracks;
    TClonesArray* fRichRingMatches;
    TClonesArray* fRichRefPlanePoints;
    TClonesArray* fRichPoints;
    TClonesArray* fGlobalTracks;
    CbmHistManager* fHM;
    CbmRichRecGeoPar fGP;

    UInt_t fEventNum; // Event counter
    UInt_t fMirrCounter;
    Bool_t fDrawHist;
    Double_t fArray[3];

    std::map<string,string> fPathsMap;
    std::map<string,string> fPathsMapEllipse;

    TString fOutputDir; // Output directory to store figures
    TString fRunTitle; // Title of the run

    CbmRichRingFitterCOP* fCopFit;
    CbmRichRingFitterEllipseTau* fTauFit;

    CbmRichCorrection(const CbmRichCorrection&);
    CbmRichCorrection operator=(const CbmRichCorrection&);

    ClassDef(CbmRichCorrection, 1);
};

#endif

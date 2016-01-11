#ifndef CBMRICHALIGNMENT_H
#define CBMRICHALIGNMENT_H


#include "FairTask.h"
#include "CbmHistManager.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include <vector>

using namespace std;

class TClonesArray;
class TH1D;
class TH2D;


class CbmRichAlignment : public FairTask
{
private:
    static const int kMAX_NOF_HITS = 100; // Maximum number of hits in ring

public:
	/*
	 * Constructor.
	 */
    CbmRichAlignment();

	/*
	 * Destructor.
	 */
    virtual ~CbmRichAlignment();

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
     * Get x and y positions on the PMT plane from the extrapolated track.
     */
    void GetTrackPosition(Double_t *x, Double_t *y);

    /*
     * Is the ring, whose index is given as a parameter, a ring from MC simulation (true) or not (false).
     */
    Bool_t IsRingMcElectron(Int_t ringIndex);

    /*
     * Calculate the mean ring center position, using the reference data set from CERN.
     */
    void CalculateMeanRingCenterPosition();

    /*
     * Calculate the distances between C and C', the Theta and Phi angles for a given event and draw the corresponding distributions.
     */
    void CalculateAnglesAndDrawDistrib();

    /*
     * Draw histograms.
     */
    void DrawHist();

    /*
     * Fit each slices of the 2D histo, fit result with sinusoidal and draw.
     */
    std::vector<Float_t> DrawFit();

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
     * Set axis rotation title. It is also a part of the file name of image files.
     */
    void SetAxisRotTitle(TString title) {fAxisRotTitle = title;}

    /*
     * Set to TRUE if you want to draw histograms.
     */
    void SetDrawHist(Bool_t b) {fDrawHist = b;}

    /*
     * Set to TRUE if you want to analysis simulation data.
     */
    void SetIsSimulationAna(Bool_t b) {fIsSimulationAna = b;}

    /*
     * Set to TRUE if you want to analysis simulation data.
     */
    void SetIsMeanPosition(Bool_t b) {fIsMeanPosition = b;}

    /*
     * Write the ring centers into a txt file, with name fileName.
     */
    void WriteToFile(TString fileName, Double_t Xposition, Double_t Yposition);


private:
    TClonesArray* fRichHits; // Array of RICH hits
    TClonesArray* fRichRings; // Array of found RICH rings
    TClonesArray* fRichProjections;
    TClonesArray* fRichPoints;
    TClonesArray* fMCTracks;
    TClonesArray* fRichRingMatches;
    TClonesArray* fRichMirrorPoints;
    CbmHistManager* fHM;

    UInt_t fEventNum; // Event counter
    UInt_t counter;
    Double_t XmeanCircle;
    Double_t YmeanCircle;
    Double_t XmeanEllipse;
    Double_t YmeanEllipse;
    Bool_t fDrawHist;
    Bool_t fIsSimulationAna; // If true: simulation analysis; Else: analysis from beamtime data
    Bool_t fIsMeanPosition;
    vector<float> fPhi;

    TString fOutputDir; // Output directory to store figures
    TString fRunTitle; // Title of the run
    TString fAxisRotTitle; // Rotation around which axis

    CbmRichRingFitterCOP* fCopFit;
    CbmRichRingFitterEllipseTau* fTauFit;

    CbmRichAlignment(const CbmRichAlignment&);
    CbmRichAlignment operator=(const CbmRichAlignment&);

    ClassDef(CbmRichAlignment, 1);
};

#endif


#ifndef CBM_RICH_SMALL_PROTOTYPE_QA
#define CBM_RICH_SMALL_PROTOTYPE_QA

#include "FairTask.h"
class TClonesArray;
class CbmRichRing;
class CbmHistManager;

#include <vector>
#include <map>

using namespace std;

class CbmRichSmallPrototypeQa : public FairTask
{
    
public:
    /**
     * \brief Standard constructor.
     */
    CbmRichSmallPrototypeQa();
    
    /**
     * \brief Standard destructor.
     */
    virtual ~CbmRichSmallPrototypeQa() {}
    
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
    
    
    /**
     * \brief Set output directory where you want to write results (figures and json).
     * \param[in] dir Path to the output directory.
     */
    void SetOutputDir(const string& dir) {fOutputDir = dir;}
    
    
private:
    
    /**
     * \brief Initialize histograms.
     */
    void InitHistograms();
    
    /**
     *  \brief Draw histograms.
     */
    void DrawHist();
    
    /**
     * Return low edges of the histogram bins which matches pixel structure.
     */
    vector<Double_t> GetHistBins(Bool_t isX);

    void DrawCircle(
                    CbmRichRing* ring);
    
    void DrawEvent();

    
    /**
     * \brief Copy constructor.
     */
    CbmRichSmallPrototypeQa(const CbmRichSmallPrototypeQa&);
    
    /**
     * \brief Assignment operator.
     */
    CbmRichSmallPrototypeQa& operator=(const CbmRichSmallPrototypeQa&);
    
    CbmHistManager* fHM;
    
    Int_t fEventNum;
    
    string fOutputDir; // output dir for results
    
  

    TClonesArray* fMCTracks;
	TClonesArray* fRichPoints;
	TClonesArray* fRichDigis;
	TClonesArray* fRichHits;
	TClonesArray* fRichRings;
    TClonesArray* fRichRingMatches;
	TClonesArray* fRefPlanePoints;
    
    ClassDef(CbmRichSmallPrototypeQa,1)
};

#endif


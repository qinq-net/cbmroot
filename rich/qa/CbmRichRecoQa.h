
#ifndef CBM_RICH_RECO_QA
#define CBM_RICH_RECO_QA

#include "FairTask.h"
class TClonesArray;
class CbmRichRing;
class TCanvas;
class CbmHistManager;
class TH1D;
class TH2D;
class TH2;
class TH3;

#include <vector>
#include <map>

using namespace std;

class CbmRichRecoQa : public FairTask
{
    
public:
    /**
     * \brief Standard constructor.
     */
    CbmRichRecoQa();
    
    /**
     * \brief Standard destructor.
     */
    virtual ~CbmRichRecoQa() {}
    
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
     * \brief Copy constructor.
     */
    CbmRichRecoQa(const CbmRichRecoQa&);
    
    /**
     * \brief Assignment operator.
     */
    CbmRichRecoQa& operator=(const CbmRichRecoQa&);
    
    
    CbmHistManager* fHM;
    
    Int_t fEventNum;
    
    string fOutputDir; // output dir for results
    
    TClonesArray* fMCTracks;
    TClonesArray* fRichPoints;
    TClonesArray* fRichDigis;
    TClonesArray* fRichHits;
    TClonesArray* fRichRings;
    TClonesArray* fRichRingMatches;
    TClonesArray* fGlobalTracks;
    TClonesArray* fStsTracks;
    TClonesArray* fStsTrackMatches;
    
    vector<TCanvas*> fCanvas;
    
    ClassDef(CbmRichRecoQa,1)
};

#endif


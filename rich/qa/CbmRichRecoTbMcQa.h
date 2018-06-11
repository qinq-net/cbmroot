
#ifndef CBM_RICH_RECO_TB_MC_QA
#define CBM_RICH_RECO_TB_MC_QA

#include "FairTask.h"
class TClonesArray;
class CbmHistManager;
class CbmRichPoint;
class CbmMCTrack;

#include <vector>
#include <map>

using namespace std;

class CbmRichRecoTbMcQa : public FairTask
{
    
public:
    /**
     * \brief Standard constructor.
     */
    CbmRichRecoTbMcQa();
    
    /**
     * \brief Standard destructor.
     */
    virtual ~CbmRichRecoTbMcQa() {}
    
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
    void SetOutputDir(
            const string& dir) {fOutputDir = dir;}
    
    /**
         * \brief Draw histogram from file
         */
    void DrawFromFile(
          const string& fileName,
		  const string& outputDir);
    
    static void DrawTimeLog (
            const string& hMainName,
            CbmHistManager* hm,
            Int_t nofLogEvents,
            bool withNoise = false);

private:
    
    /**
     * \brief Initialize histograms.
     */
    void InitHistograms();

    void DrawHist();

    void ProcessMc();

    bool IsCherenkovPhoton(
            CbmRichPoint* point);

    bool IsCherenkovPhotonFromPrimaryElectron(
            CbmRichPoint* point);

    bool IsMcPrimaryElectron(
            const CbmMCTrack* mctrack);






    /**
     * \brief Copy constructor.
     */
    CbmRichRecoTbMcQa(const CbmRichRecoTbMcQa&);
    
    /**
     * \brief Assignment operator.
     */
    CbmRichRecoTbMcQa& operator=(const CbmRichRecoTbMcQa&);
    
    
    CbmHistManager* fHM;
    
    Int_t fEventNum;
    
    Double_t fEventTime;
    Int_t fNofLogEvents;
    string fOutputDir; // output dir for results
    
    TClonesArray* fMCTracks;
    TClonesArray* fRichPoints;
    TClonesArray* fStsPoints;
    
    ClassDef(CbmRichRecoTbMcQa,1)
};

#endif


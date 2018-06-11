
#ifndef CBM_RICH_RECO_TB_RECO_QA
#define CBM_RICH_RECO_TB_RECO_QA

#include "FairTask.h"
class CbmMCDataArray;
class TClonesArray;
class CbmHistManager;
class CbmRichPoint;
class CbmMCTrack;

#include <vector>
#include <map>

using namespace std;

class CbmRichRecoTbRecoQa : public FairTask
{
    
public:
    /**
     * \brief Standard constructor.
     */
    CbmRichRecoTbRecoQa();
    
    /**
     * \brief Standard destructor.
     */
    virtual ~CbmRichRecoTbRecoQa() {}
    
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
    
    /**
         * \brief Draw histogram from file
         */
    void DrawFromFile(
          const string& fileName,
		  const string& outputDir);
    
private:
    
    /**
     * \brief Initialize histograms.
     */
    void InitHistograms();

    void DrawHist();

    void Process();



    /**
     * \brief Copy constructor.
     */
    CbmRichRecoTbRecoQa(const CbmRichRecoTbRecoQa&);
    
    /**
     * \brief Assignment operator.
     */
    CbmRichRecoTbRecoQa& operator=(const CbmRichRecoTbRecoQa&);
    
    
    CbmHistManager* fHM;
    
    Int_t fTimeSliceNum;
    
    Int_t fNofLogEvents;
    string fOutputDir; // output dir for results
    
    CbmMCDataArray* fMCTracks;
    CbmMCDataArray* fRichPoints;
    CbmMCDataArray* fStsPoints;
    TClonesArray* fRichDigis;
    TClonesArray* fRichHits;
    
    ClassDef(CbmRichRecoTbRecoQa,1)
};

#endif


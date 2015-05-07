
#ifndef CBMJPSIANATASK
#define CBMJPSIANATASK

#include "FairTask.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TClonesArray.h"
#include "CbmHistManager.h"
#include "CbmKFVertex.h"
#include "CbmVertex.h"
#include "CbmAnaJpsiCandidate.h"



using namespace std;

class CbmAnaJpsiTask : public FairTask
{

public:
   /**
    * \brief Standard constructor.
    */
	CbmAnaJpsiTask();

   /**
    * \brief Standard destructor.
    */
   virtual ~CbmAnaJpsiTask();

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
   



private:
  Int_t fEventNum;
  
  TClonesArray* fMcTracks;
  TClonesArray* fStsPoints;
  TClonesArray* fStsHits;
  TClonesArray* fStsTracks;
  TClonesArray* fStsTrackMatches;
  TClonesArray* fRichPoints; 
  TClonesArray* fRichHits;
  TClonesArray* fRichRings;
  TClonesArray* fRichRingMatches;
  TClonesArray* fTrdPoints; 
  TClonesArray* fTrdHits;
  TClonesArray* fTrdTracks;
  TClonesArray* fTrdTrackMatches;
  TClonesArray* fTofPoints; 
  TClonesArray* fTofHits;
  TClonesArray* fGlobalTracks;
  
  CbmVertex* fPrimVertex;
  CbmKFVertex fKFVertex;

  vector<CbmAnaJpsiCandidate> fCandidates; // array of JPsi candidates


  CbmHistManager* fHM;

  /*
   * \brief Creates 1D histograms for different track source types.
   * \param[in] name Base name of the histograms.
   * \param[in] axisX X axis title.
   * \param[in] axisY Y axis title.
   * \param[in] nBins Number of bins in the histogram.
   * \param[in] min Minimum value.
   * \param[in] max Maximum value.
   */
  void CreateSourceTypesH1(
        const string& name,
        const string& axisX,
        const string& axisY,
        double nBins,
        double min,
        double max);

    void InitHist();

    void McPair();

    void MCPointPMT();

    void RichHitPMT();

    /**
     * \brief Fill fCandidates array with JPsiCandidates. Candidate should have STS, RICH, TRD, TOF local segments.
     *
     */
    void FillCandidates();

    /**
     * \brief Assign MC info to the candidates.
     */
    void AssignMcToCandidates();




   /**
    * \brief Copy constructor.
    */
   CbmAnaJpsiTask(const CbmAnaJpsiTask&);

   /**
    * \brief Assignment operator.
    */
   CbmAnaJpsiTask& operator=(const CbmAnaJpsiTask&);


	ClassDef(CbmAnaJpsiTask,1)
};

#endif


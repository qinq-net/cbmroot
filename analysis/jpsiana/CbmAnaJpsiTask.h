
#ifndef CBMJPSIANATASK
#define CBMJPSIANATASK

#include "FairTask.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TClonesArray.h"
#include "CbmHistManager.h"
//#include "../dielectron/CbmLmvmCandidate.h"
#include "CbmKFVertex.h"
#include "CbmVertex.h"



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
  TClonesArray* fRichPoints; 
  TClonesArray* fTrdPoints; 
  TClonesArray* fTofPoints; 
  TClonesArray* fStsHits;
  TClonesArray* fRichHits;
  TClonesArray* fTrdHits;
  TClonesArray* fTofHits;
  TClonesArray* fStsTracks;
  TClonesArray* fRichRings;
  TClonesArray* fTrdTracks;
  TClonesArray* fGlobalTracks;
  
  CbmVertex* fPrimVertex;
  CbmKFVertex fKFVertex;


  CbmHistManager* fHM;

    void InitHist();
    
    void DrawHist();

    void McPair();

    void MCPointPMT();

    void RichHitPMT();

    void FillCandidates();


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


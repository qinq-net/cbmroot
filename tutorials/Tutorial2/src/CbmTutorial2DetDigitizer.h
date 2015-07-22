// --------------------------------------------------------------------------
// -----          Header for the CbmTutorial2DetDigitizer               ------
// -----              Created 06.06.08 by F.Uhlig                      ------
// --------------------------------------------------------------------------



#ifndef CBMTUTORIAL2DETDIGITIZER_H
#define CBMTUTORIAL2DETDIGITIZER_H


#include "FairTask.h"

#include "TVector3.h"

class TClonesArray;

class CbmTutorial2DetDigiPar;

    class CbmTutorial2DetDigitizer : public FairTask {
public:

    

    /** Default constructor **/
    CbmTutorial2DetDigitizer();

    /** Standard constructor **/
    CbmTutorial2DetDigitizer(const char *name, const char *title="CBM Task");

    /** Destructor **/
    virtual ~CbmTutorial2DetDigitizer();

    /** Initialisation **/
    virtual InitStatus ReInit();
    virtual InitStatus Init();
    virtual void SetParContainers();

    /** Executed task **/
    virtual void Exec(Option_t * option);

    /** Finish task **/
    virtual void Finish();

    void AddHit(TVector3 &posHit, TVector3 &posHitErr,
		Int_t TrackID, Int_t PlaneID, Int_t ref, Double_t ELoss,
		Double_t ELossTR, Double_t ELossdEdX);
    void Register();

private:

    TClonesArray *fTutorial2DetPoints; //! Tutorial2 Det MC points
    //    TClonesArray *fDigiCollection; //! TRD hits
    //TClonesArray *fListStack;         //Tracks

    CbmTutorial2DetDigiPar *fDigiPar;

    //    Double_t fDx;               //!
    //Double_t fDy;               //!
    //Int_t fNHits;

    ClassDef(CbmTutorial2DetDigitizer,1)

    };
#endif //CBMTUTORIALDETDIGITIZER_H

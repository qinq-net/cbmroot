/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *              GNU Lesser General Public Licence (LGPL) version 3,             *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#ifndef CBMCHECKTIMING_H
#define CBMCHECKTIMING_H

#include "FairTask.h"

#include "TString.h"

class TClonesArray;
class TH1;

class CbmCheckTiming : public FairTask
{
  public:

    /** Default constructor **/
    CbmCheckTiming();

    CbmCheckTiming(const CbmCheckTiming&) = delete;
    CbmCheckTiming operator=(const CbmCheckTiming&) = delete;

    /** Constructor with parameters (Optional) **/
    //  CbmCheckTiming(Int_t verbose);


    /** Destructor **/
    ~CbmCheckTiming();


    /** Initiliazation of task at the beginning of a run **/
    virtual InitStatus Init();

    /** ReInitiliazation of task when the runID changes **/
    virtual InitStatus ReInit();


    /** Executed for each event. **/
    virtual void Exec(Option_t*);

    /** Load the parameter container from the runtime database **/
    virtual void SetParContainers();

    /** Finish task called at the end of the run **/
    virtual void Finish();

    /** Switch OFF check for correct time order of the digis **/
    void SetCheckTimeOrder(Bool_t val = kFALSE) { fCheckTimeOrdering = val; }

    /** Switch ON check for inter system offsets **/
    void SetCheckInterSystemOffset(Bool_t val = kTRUE)
       { fCheckInterSystemOffset = val; }

    void SetOffsetSearchRange(Int_t val = 1000)
       { fOffsetRange = val; }
    
  private:

    void CheckTimeOrder();
    Int_t CheckIfSorted(TClonesArray*, TH1*, Double_t&, TString);

    void CheckInterSystemOffset();
    void FillSystemOffsetHistos(TClonesArray*, TH1*, const Double_t);
    void CreateHistos();
    void WriteHistos();
    

    /** Input array from previous already existing data level **/
    TClonesArray* fT0Digis = nullptr;
    TClonesArray* fStsDigis = nullptr;
    TClonesArray* fMuchDigis = nullptr;
    TClonesArray* fTofDigis = nullptr;

    // Variables to store the previous digi time
    Double_t fPrevTimeT0 = 0.;
    Double_t fPrevTimeSts = 0.;
    Double_t fPrevTimeMuch = 0.;
    Double_t fPrevTimeTof = 0.;

    // 
    Int_t fNrTs = 0;

    Int_t fNrOfT0Errors = 0;
    Int_t fNrOfT0Digis = 0;
    Int_t fNrOfStsErrors = 0;
    Int_t fNrOfStsDigis = 0;
    Int_t fNrOfMuchErrors = 0;
    Int_t fNrOfMuchDigis = 0;
    Int_t fNrOfTofErrors = 0;
    Int_t fNrOfTofDigis = 0;
    
    Bool_t fCheckTimeOrdering = kTRUE;
    Bool_t fCheckInterSystemOffset = kTRUE;

    Int_t fOffsetRange = 1000;
    Int_t fBinWidth = 1;

    TH1* fT0StsDiff = nullptr;
    TH1* fT0MuchDiff = nullptr;
    TH1* fT0TofDiff = nullptr;

    TH1* fT0T0Diff = nullptr;
    TH1* fStsStsDiff = nullptr;
    TH1* fMuchMuchDiff = nullptr;
    TH1* fTofTofDiff = nullptr;

    TString fOutFileName{"test.root"};
    
    ClassDef(CbmCheckTiming,1);
};

#endif

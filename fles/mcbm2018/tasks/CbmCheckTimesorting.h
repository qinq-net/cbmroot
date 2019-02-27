/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *              GNU Lesser General Public Licence (LGPL) version 3,             *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#ifndef CBMCHECKTIMESORTING_H
#define CBMCHECKTIMESORTING_H

#include "FairTask.h"

class TClonesArray;

class CbmCheckTimesorting : public FairTask
{
  public:

    /** Default constructor **/
    CbmCheckTimesorting();

    CbmCheckTimesorting(const CbmCheckTimesorting&) = delete;
    CbmCheckTimesorting operator=(const CbmCheckTimesorting&) = delete;

    /** Constructor with parameters (Optional) **/
    //  CbmCheckTimesorting(Int_t verbose);


    /** Destructor **/
    ~CbmCheckTimesorting();


    /** Initiliazation of task at the beginning of a run **/
    virtual InitStatus Init();

    /** ReInitiliazation of task when the runID changes **/
    virtual InitStatus ReInit();


    /** Executed for each event. **/
    virtual void Exec(Option_t* opt);

    /** Load the parameter container from the runtime database **/
    virtual void SetParContainers();

    /** Finish task called at the end of the run **/
    virtual void Finish();

  private:

    Int_t CheckIfSorted(TClonesArray* array, Double_t& prevTime, TString detector);

    /** Input array from previous already existing data level **/
    TClonesArray* fT0Digis;
    TClonesArray* fStsDigis;
    TClonesArray* fMuchDigis;
    TClonesArray* fTofDigis;

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
    
    /** Output array to  new data level**/
    //  TClonesArray* <OutputDataLevel>;


    ClassDef(CbmCheckTimesorting,1);
};

#endif

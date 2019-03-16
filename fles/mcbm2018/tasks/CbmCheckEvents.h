/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *              GNU Lesser General Public Licence (LGPL) version 3,             *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#ifndef CBMCHECKEVENTS_H
#define CBMCHECKEVENTS_H

#include "FairTask.h"

#include "TString.h"

class TClonesArray;
class TH1;
class TH2;

class CbmCheckEvents : public FairTask
{
  public:

    /** Default constructor **/
    CbmCheckEvents();

    CbmCheckEvents(const CbmCheckEvents&) = delete;
    CbmCheckEvents operator=(const CbmCheckEvents&) = delete;

    /** Constructor with parameters (Optional) **/
    //  CbmCheckEvents(Int_t verbose);


    /** Destructor **/
    ~CbmCheckEvents();


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

  private:


    /** Input array from previous already existing data level **/
    TClonesArray* fT0Digis{nullptr};
    TClonesArray* fStsDigis{nullptr};
    TClonesArray* fMuchDigis{nullptr};
    TClonesArray* fTofDigis{nullptr};
    TClonesArray* fEvents{nullptr};

    // Variables to store the previous digi time
    Double_t fPrevEventTime{0.};

    //
    Int_t fNrTs{0};

    ClassDef(CbmCheckEvents,1);
};

#endif

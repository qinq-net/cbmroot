/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *              GNU Lesser General Public Licence (LGPL) version 3,             *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#ifndef CBMMCBM2018EVENTBUILDER_H
#define CBMMCBM2018EVENTBUILDER_H

#include "FairTask.h"

#include "CbmDefs.h"
#include "CbmDigi.h"

#include <array>
#include <set>
#include <tuple>

class TClonesArray;

 typedef std::tuple<CbmDigi*, ECbmModuleId, Int_t> digituple;

struct classcomp {
  bool operator() (const digituple& lhs, const digituple& rhs) const
  {
//    CbmDigi* digi_lhs = std::get<0>(lhs);
//    CbmDigi* digi_rhs = std::get<0>(rhs);
//    Double_t time_lhs = std::get<0>(lhs)->GetTime();
//    Double_t time_rhs = std::get<0>(rhs)->GetTime();
    return std::get<0>(lhs)->GetTime() < std::get<0>(rhs)->GetTime();
  }
};

class CbmMcbm2018EventBuilder : public FairTask
{
  public:

    /** Default constructor **/
    CbmMcbm2018EventBuilder();

    CbmMcbm2018EventBuilder(const CbmMcbm2018EventBuilder&) = delete;
    CbmMcbm2018EventBuilder operator=(const CbmMcbm2018EventBuilder&) = delete;

    /** Constructor with parameters (Optional) **/
    //  CbmMcbm2018EventBuilder(Int_t verbose);


    /** Destructor **/
    ~CbmMcbm2018EventBuilder();


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

    void AddDigi(ECbmModuleId,Int_t);

    Int_t fNrTs = 0;
    Double_t fPrevTime = 0;

    /** Input array from previous already existing data level **/
    TClonesArray* fT0Digis = nullptr;
    TClonesArray* fStsDigis = nullptr;
    TClonesArray* fMuchDigis = nullptr;
    TClonesArray* fTofDigis = nullptr;

    std::array<TClonesArray*, ECbmModuleId::kNofSystems> fLinkArray;
        
    std::multiset<digituple, classcomp> fSet;

//    std::set<std::pair<ECbmModuleId, Int_t>> fSet;
//    std::vector<digituple> fVect;
    std::vector<std::pair<ECbmModuleId, Int_t>> fVect;
//  std::set<std::tuple<CbmDigi*, Int_t, Int_t>> fSet;
//  std::set<std::tuple<Double_t, Int_t, Int_t>> fSet; // time, system(->TClonesarray), entry in TClonesarray
    ClassDef(CbmMcbm2018EventBuilder,1);
};

#endif

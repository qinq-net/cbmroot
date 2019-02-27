/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *              GNU Lesser General Public Licence (LGPL) version 3,             *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#include "CbmCheckTimesorting.h"

#include "CbmTofDigiExp.h"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include <iomanip> 
using std::fixed;
using std::setprecision;

// ---- Default constructor -------------------------------------------
CbmCheckTimesorting::CbmCheckTimesorting()
  : FairTask("CbmCheckTimesorting")
  , fT0Digis{nullptr}
  , fStsDigis{nullptr}
  , fMuchDigis{nullptr}
  , fTofDigis{nullptr}

{

}

// ---- Destructor ----------------------------------------------------
CbmCheckTimesorting::~CbmCheckTimesorting()
{

}

// ----  Initialisation  ----------------------------------------------
void CbmCheckTimesorting::SetParContainers()
{
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <CbmCheckTimesortingDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmCheckTimesorting::Init()
{

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  // Get a pointer to the previous already existing data level
  
    fT0Digis = static_cast<TClonesArray*>(ioman->GetObject("CbmT0Digi"));
    if ( ! fT0Digis ) {
      LOG(info) << "No TClonesArray with T0 digis found.";
    }

    fStsDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmStsDigi"));
    if ( ! fStsDigis ) {
      LOG(info) << "No TClonesArray with STS digis found.";
    }

    fMuchDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmMuchBeamTimeDigi"));
    if ( ! fMuchDigis ) {
      LOG(info) << "No TClonesArray with MUCH digis found.";
    }

    fTofDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmTofDigi"));
    if ( ! fTofDigis ) {
      LOG(info) << "No TClonesArray with TOF digis found.";
    }

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmCheckTimesorting::ReInit()
{
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmCheckTimesorting::Exec(Option_t* /*option*/)
{

  fNrTs++;
   
  if (fT0Digis) {
    Int_t nrT0Digis = fT0Digis->GetEntriesFast();
    fNrOfT0Digis += nrT0Digis; 
    fNrOfT0Errors += CheckIfSorted(fT0Digis, fPrevTimeT0, "T0");
  }
  if (fStsDigis) {
    Int_t nrStsDigis = fStsDigis->GetEntriesFast();
    fNrOfStsDigis += nrStsDigis; 
    fNrOfStsErrors += CheckIfSorted(fStsDigis, fPrevTimeSts, "Sts");
  }
  if (fMuchDigis) {
    Int_t nrMuchDigis = fMuchDigis->GetEntriesFast();
    fNrOfMuchDigis += nrMuchDigis; 
    fNrOfMuchErrors += CheckIfSorted(fMuchDigis, fPrevTimeMuch, "Much");
  }
  if (fTofDigis) {
    Int_t nrTofDigis = fTofDigis->GetEntriesFast();
    fNrOfTofDigis += nrTofDigis; 
    fNrOfTofErrors += CheckIfSorted(fTofDigis, fPrevTimeTof, "Tof");
  }

}

Int_t CbmCheckTimesorting::CheckIfSorted(TClonesArray* array, Double_t& prevTime, TString detector) 
{
  Int_t nrOfErrors=0;
  Int_t nrDigis=array->GetEntriesFast();

  for (Int_t i = 0; i < nrDigis; ++i) {
 
     CbmDigi* digi = static_cast<CbmDigi*>(array->At(i));

     Double_t diffTime = digi->GetTime() - prevTime;

     if (diffTime < 0.) {
       LOG(info)  << fixed << setprecision(15) << diffTime << "ns";
       LOG(info) << "Previous " << detector << " digi (" << fixed << setprecision(15) 
                  << prevTime * 1.e-9 
                  << ") has a larger time than the current one ("
                  << digi->GetTime() * 1.e-9 << ") for digi " 
                  << i << " of ts " << fNrTs; 
       nrOfErrors++;
     }
     
     prevTime = digi->GetTime();

  } 

  return nrOfErrors;
}

// ---- Finish --------------------------------------------------------
void CbmCheckTimesorting::Finish()
{
  LOG(info) << "Total number of T0 out of order digis: " << fNrOfT0Errors;
  LOG(info) << "Total number of T0 digis: " << fNrOfT0Digis;
  LOG(info) << "Total number of Sts out of order digis: " << fNrOfStsErrors;
  LOG(info) << "Total number of Sts digis: " << fNrOfStsDigis;
  LOG(info) << "Total number of Much out of order digis: " << fNrOfMuchErrors;
  LOG(info) << "Total number of Much digis: " << fNrOfMuchDigis;
  LOG(info) << "Total number of Tof out of order digis: " << fNrOfTofErrors;
  LOG(info) << "Total number of Tof digis: " << fNrOfTofDigis;
}

ClassImp(CbmCheckTimesorting)

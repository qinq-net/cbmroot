/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *              GNU Lesser General Public Licence (LGPL) version 3,             *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#include "CbmCheckEvents.h"

#include "CbmTofDigiExp.h"
#include "CbmMuchBeamTimeDigi.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunOnline.h"

#include "TClonesArray.h"
#include "TH1.h"
#include "TH2.h"
#include "THttpServer.h"

#include <iomanip>
using std::fixed;
using std::setprecision;

// ---- Default constructor -------------------------------------------
CbmCheckEvents::CbmCheckEvents()
  : FairTask("CbmCheckEvents")
{

}

// ---- Destructor ----------------------------------------------------
CbmCheckEvents::~CbmCheckEvents()
{

}

// ----  Initialisation  ----------------------------------------------
void CbmCheckEvents::SetParContainers()
{
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <CbmCheckEventsDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmCheckEvents::Init()
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

  fEvents = static_cast<TClonesArray*>(ioman->GetObject("CbmEvent"));
  if ( ! fEvents ) {
    LOG(fatal) << "No TClonesArray with events found.";
  }
  
  return kSUCCESS;
}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmCheckEvents::ReInit()
{
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmCheckEvents::Exec(Option_t* /*option*/)
{

  fNrTs++;
  LOG(debug) << "executing TS " << fNrTs;

  Int_t nrEvents=fEvents->GetEntriesFast();
  Int_t nrT0Digis=fT0Digis->GetEntriesFast();
  Int_t nrStsDigis=fStsDigis->GetEntriesFast();
  Int_t nrMuchDigis=fMuchDigis->GetEntriesFast();
  Int_t nrTofDigis=fTofDigis->GetEntriesFast();

  LOG(debug) << "Events: " << nrEvents;
  LOG(debug) << "T0Digis: " << nrT0Digis;   
  LOG(debug) << "StsDigis: " << nrStsDigis;
  LOG(debug) << "MuchDigis: " << nrMuchDigis;
  LOG(debug) << "TofDigis: " << nrTofDigis;

}


// ---- Finish --------------------------------------------------------
void CbmCheckEvents::Finish()
{
}

ClassImp(CbmCheckEvents)

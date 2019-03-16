/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *              GNU Lesser General Public Licence (LGPL) version 3,             *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
#include "CbmMcbm2018EventBuilder.h"

#include "CbmTofDigiExp.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunOnline.h"

#include "TClonesArray.h"
#include "TH1.h"
#include "THttpServer.h"

#include <iomanip> 
using std::fixed;
using std::setprecision;

// ---- Default constructor -------------------------------------------
CbmMcbm2018EventBuilder::CbmMcbm2018EventBuilder()
  : FairTask("CbmMcbm2018EventBuilder")
{

}

// ---- Destructor ----------------------------------------------------
CbmMcbm2018EventBuilder::~CbmMcbm2018EventBuilder()
{

}

// ----  Initialisation  ----------------------------------------------
void CbmMcbm2018EventBuilder::SetParContainers()
{
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <CbmMcbm2018EventBuilderDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmMcbm2018EventBuilder::Init()
{

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  // Get a pointer to the previous already existing data level
  
  fT0Digis = static_cast<TClonesArray*>(ioman->GetObject("CbmT0Digi"));
  if ( ! fT0Digis ) {
    LOG(info) << "No TClonesArray with T0 digis found.";
  } else {
    // Misuse kHodo for the T0 detector
    fLinkArray[kHodo] = fT0Digis;
  }
  
  fStsDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmStsDigi"));
  if ( ! fStsDigis ) {
    LOG(info) << "No TClonesArray with STS digis found.";
  } else {
    fLinkArray[kSts] = fStsDigis;
  }
  
  fMuchDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmMuchBeamTimeDigi"));
  if ( ! fMuchDigis ) {
    fMuchDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmMuchDigi"));
    if ( ! fMuchDigis ) {
      fMuchDigis = static_cast<TClonesArray*>(ioman->GetObject("MuchDigi"));
      if ( ! fMuchDigis ) {
        LOG(info) << "No TClonesArray with MUCH digis found.";
      } else {
       fLinkArray[kMuch] = fMuchDigis;
      }
    } else {
     fLinkArray[kMuch] = fMuchDigis;
    }
  } else {
    fLinkArray[kMuch] = fMuchDigis;
  }
  
  
  fTofDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmTofDigi"));
  if ( ! fTofDigis ) {
    LOG(info) << "No TClonesArray with TOF digis found.";
  } else {
    fLinkArray[kTof] = fTofDigis;
  }

  // Register output array (CbmEvent)
  fEvents = new TClonesArray("CbmEvent",100);
  ioman->Register("CbmEvent", "Cbm Event", fEvents,
                                        IsOutputBranchPersistent("CbmEvent"));

  if ( ! fEvents ) LOG(FATAL) << "Output branch was not created" << FairLogger::endl;


  fDiffTime = new TH1F("fDiffTime","Time difference between two consecutive digis;time diff [ns];Counts", 420, -100.5, 1999.5);
    
  return kSUCCESS;
}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmMcbm2018EventBuilder::ReInit()
{
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmMcbm2018EventBuilder::Exec(Option_t* /*option*/)
{

  LOG_IF(info, fNrTs%1000 == 0) <<"Begin of TS " << fNrTs;
  fNrTs++;
 
  InitSorter();

  BuildEvents();

  LOG(debug) << "Found " << fEventVector.size() << " events";

  FillHisto();
  
  DefineGoodEvents();

  LOG(debug) << "Found " << fEventVector.size() << " triggered events";

  FillOutput();


}

void CbmMcbm2018EventBuilder::InitSorter()
{
  // Fill the first entry of each TClonesarray to the std::set
  // The sorting should be done using the time of the digi which
  // can be received using the GetTime() function of CbmDigi
  
  Int_t nrT0Digis{0};
  if (fT0Digis) nrT0Digis = fT0Digis->GetEntriesFast();
  Int_t nrStsDigis{0};
  if (fStsDigis) nrStsDigis = fStsDigis->GetEntriesFast();
  Int_t nrMuchDigis{0};
  if (fMuchDigis) nrMuchDigis = fMuchDigis->GetEntriesFast();
  Int_t nrTofDigis{0};
  if (fTofDigis) nrTofDigis = fTofDigis->GetEntriesFast();

  LOG(debug) << "T0Digis: " << nrT0Digis;
  LOG(debug) << "StsDigis: " << nrStsDigis;
  LOG(debug) << "MuchDigis: " << nrMuchDigis;
  LOG(debug) << "TofDigis: " << nrTofDigis;

  CbmDigi* digi = nullptr;
  
  if (nrT0Digis>0) {
    AddDigiToSorter(kHodo, 0);
  }
  if (nrStsDigis>0) {
    AddDigiToSorter(kSts, 0);
  }
  if (nrMuchDigis>0) {
    AddDigiToSorter(kMuch, 0);
  }
  if (nrTofDigis>0) {
    AddDigiToSorter(kTof, 0);
  }

  for ( const auto& _tuple: fSorter) { 
    LOG(debug) << "Array, Entry(" << std::get<1>(_tuple) << ", " 
	       << std::get<2>(_tuple) << "): " << fixed 
	       << setprecision(15) << std::get<0>(_tuple)->GetTime() << " ns";
  }

  // Get the first element of the set from which one gets the first
  // element of the tuple (digi) from which one gets the smallest time
  // of all digis of the new TS
  if (fSorter.size() > 0) {
    fPrevTime = std::get<0>(*(fSorter.begin()))->GetTime();
    fStartTimeEvent = fPrevTime;
  }
}

void CbmMcbm2018EventBuilder::BuildEvents()
{
  // Create a first CbmEvent
  fCurrentEvent = new CbmEvent(fCurEv++, fStartTimeEvent, 0.);
  
  while (fSorter.size() > 0) {

    // Extract the needed information from the first element of the set
    // The first element is the one with the smallest time
    digituple _tuple = *(fSorter.begin());
    ECbmModuleId _system = std::get<1>(_tuple);        
    Int_t _entry = std::get<2>(_tuple); 
    Double_t time = std::get<0>(_tuple)->GetTime();
    Double_t difftime = time - fPrevTime;

    // Decide if the digi belongs to the current event or if
    // it starts a new event
    if (! IsDigiInEvent(time)) {  
      fCurrentEvent->SetEndTime(fPrevTime);
      fEventVector.push_back(fCurrentEvent);
      // Create then next CbmEvent
      fStartTimeEvent = time;
      fCurrentEvent = new CbmEvent(fCurEv++, fStartTimeEvent, 0.);
    }
    AddDigiToEvent(_system,_entry);

    if (fFillHistos) fVect.emplace_back(std::make_pair(_system,_entry));

    // Remove the first element from the set and insert the next digi
    // from the same system
    fSorter.erase(fSorter.begin());
    AddDigiToSorter(_system, ++_entry);

    fPrevTime = time;
  }
  fCurrentEvent->SetEndTime(fPrevTime);
  fEventVector.push_back(fCurrentEvent);  
}

Bool_t CbmMcbm2018EventBuilder::IsDigiInEvent(Double_t time)
{
  // here the different possibilities have to be implemented
  if (EventBuilderAlgo::FixedTimeWindow == fEventBuilderAlgo) {
    return ((time-fStartTimeEvent < fFixedTimeWindow) ? kTRUE : kFALSE); 
  } else {
    return ((time-fPrevTime < fMaximumTimeGap) ? kTRUE : kFALSE); 
  }
}

Bool_t CbmMcbm2018EventBuilder::HasTrigger(CbmEvent* event)
{
  Bool_t hasTrigger{kTRUE};
  if (fT0Digis && fTriggerMinT0Digis > 0) {
    hasTrigger = hasTrigger && (event->GetNofData(kT0Digi) >= fTriggerMinT0Digis);
  }
  if (fStsDigis && fTriggerMinStsDigis > 0) {
    hasTrigger = hasTrigger && (event->GetNofData(kStsDigi) >= fTriggerMinStsDigis);
  }
  if (fMuchDigis && fTriggerMinMuchDigis > 0) {
    hasTrigger = hasTrigger && (event->GetNofData(kMuchDigi) >= fTriggerMinMuchDigis);
  }
  if (fTofDigis && fTriggerMinTofDigis > 0) {
    hasTrigger = hasTrigger && (event->GetNofData(kTofDigi) >= fTriggerMinTofDigis);
  }

  return hasTrigger;
}

void CbmMcbm2018EventBuilder::FillHisto()
{
  fPrevTime = 0.;
 
  ECbmModuleId prevSystem{kNofSystems};
  Int_t prevEntry{-1};

 
  for ( const auto& _pair: fVect) {
    ECbmModuleId _system = _pair.first;
    Int_t _entry = _pair.second;
    CbmDigi* digi = static_cast<CbmDigi*>(fLinkArray[_system]->At(_entry));
    Double_t difftime = digi->GetTime() - fPrevTime;
    if (fFillHistos) fDiffTime->Fill(difftime);
    if (difftime < 0.) {
      fErrors++;
      LOG(info) << fixed << setprecision(15)
                << "DiffTime: " << difftime *1.e-9
		<< "  Previous digi(" << prevSystem << ", "
                << prevEntry << "): "
                << fPrevTime * 1.e-9 << ", Current digi(" 
                << _system << ", " <<_entry  << "): "    
                << digi->GetTime() * 1.e-9;
    }
    fPrevTime = digi->GetTime();
    prevSystem = _system;
    prevEntry = _entry;
  }
  fVect.clear();  
}

void CbmMcbm2018EventBuilder::DefineGoodEvents()
{
  for (auto it = fEventVector.begin(); it != fEventVector.end(); /*increased in the loop*/ ) {
    if (!HasTrigger((*it))) {
      delete (*it);
      it = fEventVector.erase(it);
    } else {
      ++it;
    }
  }
}

void CbmMcbm2018EventBuilder::FillOutput()
{
  // Clear TClonesArray before usage.
  fEvents->Delete();

  // Move CbmEvent from temporary vector to TClonesArray
  for (auto event: fEventVector) {
    LOG(debug) << "Vector: " << event->ToString();
    new ( (*fEvents)[fEvents->GetEntriesFast()] ) CbmEvent(std::move(*event));
     LOG(debug) << "TClonesArray: " 
               << static_cast<CbmEvent*>(fEvents->At(fEvents->GetEntriesFast()-1))->ToString();
  }

  // Clear event vector after usage
  // Need to delete the object the pointer points to first
  int counter = 0;
  for (auto event:  fEventVector) {
     LOG(debug) << "Event " << counter << " has " << event->GetNofData() << " digis"; 
     delete event;
     counter++;
  }

  fEventVector.clear();

}

void CbmMcbm2018EventBuilder::AddDigiToEvent(ECbmModuleId _system, Int_t _entry)
{
  // Fill digi index into event
  switch (_system) {
    case kMvd:  fCurrentEvent->AddData(kMvdDigi,  _entry); break;
    case kSts:  fCurrentEvent->AddData(kStsDigi,  _entry); break;
    case kRich: fCurrentEvent->AddData(kRichDigi, _entry); break;
    case kMuch: fCurrentEvent->AddData(kMuchDigi, _entry); break;
    case kTrd:  fCurrentEvent->AddData(kTrdDigi,  _entry); break;
    case kTof:  fCurrentEvent->AddData(kTofDigi,  _entry); break;
    case kPsd:  fCurrentEvent->AddData(kPsdDigi,  _entry); break;
    case kHodo: fCurrentEvent->AddData(kT0Digi,   _entry); break;  
    break;
  }
}

void CbmMcbm2018EventBuilder::AddDigiToSorter(ECbmModuleId _system, Int_t _entry)
{
  LOG(debug) << "Entry: " << _entry;
  if (_entry < fLinkArray[_system]->GetEntriesFast()) {
    CbmDigi* digi = static_cast<CbmDigi*>(fLinkArray[_system]->At(_entry));
    fSorter.emplace(std::make_tuple(digi, _system, _entry));
  }
}

// ---- Finish --------------------------------------------------------
void CbmMcbm2018EventBuilder::Finish()
{
  if (fFillHistos) {
    TFile* old = gFile;
    TFile* outfile = TFile::Open(fOutFileName,"RECREATE");
    
    fDiffTime->Write();
    
    outfile->Close();
    delete outfile;
    
    gFile = old;
  }
  LOG(info) << "Total errors: " << fErrors;
}


ClassImp(CbmMcbm2018EventBuilder)

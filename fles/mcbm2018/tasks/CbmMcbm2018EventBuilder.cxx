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
    LOG(info) << "No TClonesArray with MUCH digis found.";
  } else {
    fLinkArray[kMuch] = fMuchDigis;
  }
  
  
  fTofDigis = static_cast<TClonesArray*>(ioman->GetObject("CbmTofDigi"));
  if ( ! fTofDigis ) {
    LOG(info) << "No TClonesArray with TOF digis found.";
  } else {
    fLinkArray[kTof] = fTofDigis;
  }

  fDiffTime = new TH1F("fDiffTime","Time difference between two consecutive digis;time diff [ns];Counts", 420, -100.5, 1999.5);
    
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

  FillHisto();
  
  DefineGoodEvents();

  FillOutput();
  // Cleanup of local data structures
  fVect.clear();
  fSet.erase (fSet.begin(), fSet.end());

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
    AddDigi(kHodo, 0);
  }
  if (nrStsDigis>0) {
    AddDigi(kSts, 0);
  }
  if (nrMuchDigis>0) {
    AddDigi(kMuch, 0);
  }
  if (nrTofDigis>0) {
    AddDigi(kTof, 0);
  }

  for ( const auto& _tuple: fSet) { 
    LOG(debug) << "Array, Entry(" << std::get<1>(_tuple) << ", " 
	       << std::get<2>(_tuple) << "): " << fixed 
	       << setprecision(15) << std::get<0>(_tuple)->GetTime() << " ns";
  }

  // Reset variables which are only valid inside one TS
  fPrevEntry = -1;
  // Get the first element of the set from which one gets the first
  // element of the tuple (digi) from which one gets the smallest time
  // of all digis of the new TS
  fPrevTime = std::get<0>(*(fSet.begin()))->GetTime();

}

void CbmMcbm2018EventBuilder::BuildEvents()
{
  while (fSet.size() > 0) {
    LOG(debug) << "Length: " << fSet.size();
    digituple _tuple = *(fSet.begin());
    ECbmModuleId _system = std::get<1>(_tuple);        
    Int_t _entry = std::get<2>(_tuple); 
    LOG(debug) << "System, Entry: " << _system << ", "
	       << _entry;
    // Here must be the event condition 
    fVect.emplace_back(std::make_pair(_system,_entry));
    fSet.erase(fSet.begin());
    _entry++;
    AddDigi(_system, _entry);
    LOG(debug) << "Length: " << fSet.size();
  }
}

void CbmMcbm2018EventBuilder::FillHisto()
{
  //  LOG(info) << "Size of Vector: " << fVect.size();
  for ( const auto& _pair: fVect) {
    ECbmModuleId _system = _pair.first;
    Int_t _entry = _pair.second;
    CbmDigi* digi = static_cast<CbmDigi*>(fLinkArray[_system]->At(_entry));
    Double_t difftime = digi->GetTime() - fPrevTime;
    if (fFillHistos) fDiffTime->Fill(difftime);
    if (difftime < 0.) fErrors++;
    LOG_IF(info, difftime < 0.) << fixed << setprecision(15)
                                << "DiffTime: " << difftime *1.e-9
				<< "  Previous digi(" << fPrevSystem << ", "
                                << fPrevEntry << "): "
                                << fPrevTime * 1.e-9 << ", Current digi(" 
                                << _system << ", " <<_entry  << "): "    
                                << digi->GetTime() * 1.e-9;
    fPrevTime = digi->GetTime();
    fPrevSystem = _system;
    fPrevEntry = _entry;
  }
}
void CbmMcbm2018EventBuilder::DefineGoodEvents()
{
}

void CbmMcbm2018EventBuilder::FillOutput()
{
}

void CbmMcbm2018EventBuilder::AddDigi(ECbmModuleId _system, Int_t _entry)
{
  LOG(debug) << "Entry: " << _entry;
  if (_entry < fLinkArray[_system]->GetEntriesFast()) {
    CbmDigi* digi = static_cast<CbmDigi*>(fLinkArray[_system]->At(_entry));
    //    Double_t difftime = digi->GetTime() - fPrevTime;
    /*
    if (fFillHistos) fDiffTime->Fill(difftime);
    if (difftime < 0.) fErrors++;
    LOG_IF(info, difftime < 0.) << fixed << setprecision(15)
                                << "DiffTime: " << difftime *1.e-9
				<< "  Previous digi(" << fPrevSystem << ", "
                                << fPrevEntry << "): "
                                << fPrevTime * 1.e-9 << ", Current digi(" 
                                << _system << ", " <<_entry  << "): "    
                                << digi->GetTime() * 1.e-9;
    LOG(debug) << "Tof: " << fixed << setprecision(15) << digi->GetTime();
    */
    fSet.emplace(std::make_tuple(digi, _system, _entry));
    /*
    fPrevTime = digi->GetTime();
    fPrevSystem = _system;
    fPrevEntry = _entry;
    */
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

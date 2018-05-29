/** @file CbmDigitization.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 18.05.2018
 **/

#include "CbmDigitization.h"

#include <cassert>
#include "TClonesArray.h"
#include "TGeoManager.h"
#include "TROOT.h"
#include "FairFileSource.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "FairMonitor.h"
#include "FairParAsciiFileIo.h"
#include "FairParRootFileIo.h"
#include "FairRuntimeDb.h"
#include "CbmDaq.h"
#include "CbmMuchDigitizeGem.h"
#include "CbmMvdDigitizer.h"
#include "CbmPsdSimpleDigitizer.h"
#include "CbmRichDigitizer.h"
#include "CbmRunAna.h"
#include "CbmStsDigitize.h"
#include "CbmSetup.h"
#include "CbmTofDigitizerBDF.h"
#include "CbmTrdDigitizerPRF.h"




// -----   Constructor   ----------------------------------------------------
CbmDigitization::CbmDigitization() :
  TNamed("CbmDigitization", "Digitisation Run"),
  fDigitizers(),
  fInputFiles(),
  fEventRates(),
  fOutFile(),
  fParRootFile(),
  fParAsciiFiles(),
  fEventMode(kFALSE),
  fTimeSliceInterval(-1.),
  fGenerateRunInfo(kTRUE),
  fMonitor(kTRUE),
  fRun(0)
{
  SetDefaultBranches();
}
// --------------------------------------------------------------------------



// -----   Destructor   -----------------------------------------------------
CbmDigitization::~CbmDigitization() {
}
// --------------------------------------------------------------------------



// -----   Add an input file   ----------------------------------------------
Int_t CbmDigitization::AddInput(TString fileName, Double_t eventRate) {
  if ( gSystem->AccessPathName(fileName) )
    LOG(FATAL) << fName << ": input file " << fileName << " does not exist!"
      << FairLogger::endl;
  fInputFiles.push_back(fileName);
  fEventRates.push_back(eventRate);
  return fInputFiles.size();
}
// --------------------------------------------------------------------------



// -----   Check input file   -----------------------------------------------
Int_t CbmDigitization::CheckInputFile() {

  LOG(INFO) << fName << ": check input branches..." << FairLogger::endl;
  Int_t nBranches = 0;
  TFile* file = new TFile(fInputFiles[0]);
  assert(file);
  LOG(INFO) << fName << ": opening first input file " << fInputFiles[0]
            << FairLogger::endl;
  TTree* tree = dynamic_cast<TTree*>(file->Get("cbmsim"));
  assert(tree);

  for (auto it = fDigitizers.begin(); it != fDigitizers.end(); it++) {
    TBranch* branch = tree->GetBranch(it->second->GetBranchName().Data());
    if ( branch ) {
      LOG(INFO) << fName << ": found branch " << it->second->GetBranchName()
          << " for system " << CbmModuleList::GetModuleNameCaps(it->first)
          << FairLogger::endl;
      it->second->SetPresent();
      nBranches++;
    } //? branch found
  } //# systems

  FairMCEventHeader* header = new FairMCEventHeader();
  tree->SetBranchAddress("MCEventHeader.", &header);
  tree->GetEntry(0);
  fRun = header->GetRunID();
  LOG(INFO) << fName << ": run id is " << fRun << FairLogger::endl;

  delete tree;
  file->Close();
  delete file;
  return nBranches;
  delete header;
}
// --------------------------------------------------------------------------



// -----   Create the digitisers   ------------------------------------------
Int_t CbmDigitization::CreateDigitizers() {

  LOG(INFO) << fName << ": create default digitisers: ";
  Int_t nDigis = 0;
  for (auto it = fDigitizers.begin(); it != fDigitizers.end(); it++) {

    // --- Skip if marked inactive
    if ( ! it->second->IsActive() ) continue;

    // --- Skip if MC data branch is not present
    if ( ! it->second->IsPresent() ) continue;

    // --- Skip if a digitizer was set explicitly
    if ( it->second->GetDigitizer() != nullptr ) continue;

    Int_t system = it->first;
    switch (system) {
      case kMvd:
        fDigitizers[system]->SetDigitizer(new CbmMvdDigitizer());
        LOG(INFO) << "MVD "; nDigis++; break;
      case kSts:
        fDigitizers[system]->SetDigitizer(new CbmStsDigitize());
        LOG(INFO) << "STS "; nDigis++; break;
      case kRich:
        fDigitizers[system]->SetDigitizer(new CbmRichDigitizer());
        LOG(INFO) << "RICH "; nDigis++; break;
     case kMuch:
        fDigitizers[system]->SetDigitizer(new CbmMuchDigitizeGem());
        LOG(INFO) << "MUCH "; nDigis++; break;
      case kTrd:
        fDigitizers[system]->SetDigitizer(new CbmTrdDigitizerPRF());
        LOG(INFO) << "TRD "; nDigis++; break;
      case kTof:
        fDigitizers[system]->SetDigitizer(new CbmTofDigitizerBDF());
        LOG(INFO) << "TOF "; nDigis++; break;
      case kPsd:
        fDigitizers[system]->SetDigitizer(new CbmPsdSimpleDigitizer());
        LOG(INFO) << "PSD "; nDigis++; break;
     default: LOG(FATAL) << fName << ": unknown system " << system
          << FairLogger::endl; break;
    } //? system
  } //# present systems
  LOG(INFO) << FairLogger::endl;

  return nDigis;
}
// --------------------------------------------------------------------------



// -----   Deactivate a system   --------------------------------------------
void CbmDigitization::Deactivate(Int_t system) {
  if ( fDigitizers.find(system) != fDigitizers.end() )
    fDigitizers[system]->SetActive(kFALSE);
}
// --------------------------------------------------------------------------



// -----   Get a system geometry tag   --------------------------------------
TString CbmDigitization::GetGeoTag(Int_t system, TGeoManager* geo) {

  TString geoTag;
  TString sysName = CbmModuleList::GetModuleName(system);
  Int_t sysLength = sysName.Length() + 1;
  gGeoManager->CdTop();
  TGeoNode* cave = gGeoManager->GetCurrentNode();  // cave
  for (Int_t iNode = 0; iNode < cave->GetNdaughters(); iNode++) {
    TString volName = cave->GetDaughter(iNode)->GetVolume()->GetName();
     if ( volName.Contains(sysName.Data(), TString::kIgnoreCase) ) {
       geoTag = TString(volName(sysLength, volName.Length() - sysLength));
       break;
     } //? node is MUCH
  } //# top level nodes

  return geoTag;
}
// --------------------------------------------------------------------------



// -----   Execute digitisation run   ---------------------------------------
void CbmDigitization::Run(Int_t event1, Int_t event2) {

  // --- Run info
  std::cout << std::endl << std::endl;
  LOG(INFO) << "==================================================="
      << FairLogger::endl;
  if ( fEventMode ) LOG(INFO) << fName << ": running in event-by-event mode"
      << FairLogger::endl << FairLogger::endl;
  else {
    LOG(INFO) << fName << ": time-slice interval is ";
    if ( fTimeSliceInterval < 0. ) LOG(INFO) << "infinity.";
    else LOG(INFO) << fTimeSliceInterval << "ns.";
    LOG(INFO) << FairLogger::endl << FairLogger::endl;
  }

  // --- Look for input branches
  Int_t nBranches = CheckInputFile();
  TString word = (nBranches == 1 ? "branch" : "branches");
  LOG(INFO) << fName << ": " << nBranches << " input " << word << " found"
      << FairLogger::endl << FairLogger::endl;

  // --- Create default digitizers
  Int_t nDigis = CreateDigitizers();
  word = (nDigis == 1 ? " digitiser" : " digitisers");
  LOG(INFO) << fName << ": " << nDigis << word << " instantiated."
      << FairLogger::endl << FairLogger::endl;

  // --- Create CbmRunAna
  CbmRunAna* run = new CbmRunAna();
  run->SetGenerateRunInfo(fGenerateRunInfo);
  if ( fGenerateRunInfo ) LOG(INFO) << "Generate run info" << FairLogger::endl;
  FairMonitor::GetMonitor()->EnableMonitor(fMonitor);

  // --- Add input files
  Int_t nInputs = fInputFiles.size();
  for (Int_t iInput = 0; iInput < nInputs; iInput++) {
    FairFileSource* source = new FairFileSource(fInputFiles.at(iInput));
    source->SetEventMeanTime(1.e9 / fEventRates.at(iInput));
    run->SetSource(source);
    LOG(INFO) << fName << ": use input file " << fInputFiles.at(iInput)
        << FairLogger::endl;
  }

  // --- Set output file
  run->SetOutputFile(fOutFile);
  LOG(INFO) << fName << ": output file is " << fOutFile
      << FairLogger::endl << FairLogger::endl;

  // --- Register digitisers
  for (auto it = fDigitizers.begin(); it != fDigitizers.end(); it++) {
    FairTask* digitizer = it->second->GetDigitizer();
    if ( it->second->IsActive() && digitizer != nullptr ) {
      run->AddTask(digitizer);
      LOG(INFO) << fName << ": added task " << digitizer->GetName()
          << FairLogger::endl;
    } //? active and digitizer instance present
  } //# digitizers

  // --- Register DAQ
  if ( ! fEventMode ) {
    CbmDaq* daq = new CbmDaq();
    daq->SetTimeSliceInterval(fTimeSliceInterval);
    run->AddTask(daq);
  }

  // --- Set runtime database
  std::cout << std::endl;
  LOG(INFO) << fName << ": setting runtime DB " << FairLogger::endl;
  LOG(INFO) << fName << ": ROOT I/O is " << fParRootFile << FairLogger::endl;
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIoRoot = new FairParRootFileIo();
  parIoRoot->open(fParRootFile.Data(), "UPDATE");
  rtdb->setFirstInput(parIoRoot);

  // --- Get geometry from runtime database
  rtdb->getContainer("FairGeoParSet");
  rtdb->initContainers(fRun);

  // --- Add parameter files for TRD and TOF
  TString tofGeo = GetGeoTag(kTof, gGeoManager);
  TString trdGeo = GetGeoTag(kTrd, gGeoManager);
  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
  TString parFile;
  parFile = srcDir + "/parameters/trd/trd_" + trdGeo + ".digi.par";
  LOG(INFO) << fName << ": adding " << parFile << FairLogger::endl;
  AddParameterAsciiFile(parFile);
  parFile = srcDir + "/parameters/tof/tof_" + tofGeo + ".digi.par";
  LOG(INFO) << fName << ": adding " << parFile << FairLogger::endl;
  AddParameterAsciiFile(parFile);
  parFile = srcDir + "/parameters/tof/tof_" + tofGeo + ".digibdf.par";
  LOG(INFO) << fName << ": adding " << parFile << FairLogger::endl;
  AddParameterAsciiFile(parFile);
  FairParAsciiFileIo* parIoAscii = new FairParAsciiFileIo();
  parIoAscii->open(&fParAsciiFiles, "in");
  rtdb->setSecondInput(parIoAscii);

  // --- Delete TGeoManager (will be initialised again from FairRunAna)
  if (gROOT->GetVersionInt() >= 60602) {
    gGeoManager->GetListOfVolumes()->Delete();
    gGeoManager->GetListOfShapes()->Delete();
    delete gGeoManager;
  }
  LOG(INFO) << "==================================================="
      << FairLogger::endl;


  // --- Initialise run
  std::cout << std::endl << std::endl;
  LOG(INFO) << "==================================================="
      << FairLogger::endl;
  LOG(INFO) << fName << ": initialising run..." << FairLogger::endl;
  run->Init();
  rtdb->setOutput(parIoRoot);
  rtdb->saveOutput();
  LOG(INFO) << "==================================================="
      << FairLogger::endl;

  // --- Run digitisation
  std::cout << std::endl << std::endl << std::endl;
  LOG(INFO) << "==================================================="
      << FairLogger::endl;
  LOG(INFO) << fName << ": starting run..." << FairLogger::endl;
  if ( event2 < 0 ) {
    if ( event1 >= 0 ) run->Run(0, event1 - 1);  // Run event1 events
    else run->Run();                             // Run all events in input
  }
  else {
    if ( event1 < 0 ) event1 = 0;
    if ( event1 <= event2 ) run->Run(event1, event2);  // Run from event1 to event2
    else run->Run(event1, event1);                     // Run only event1
  }
  std::cout << std::endl;
  LOG(INFO) << fName << ": run finished." << FairLogger::endl;
  LOG(INFO) << "==================================================="
      << FairLogger::endl;

  // --- Resource monitoring
  std::cout << std::endl << std::endl;
  LOG(INFO) << fName << ": CPU consumption" << FairLogger::endl;
  if (fMonitor) { FairMonitor::GetMonitor()->Print(); }

}
// --------------------------------------------------------------------------



// -----   Set default info   -----------------------------------------------
void CbmDigitization::SetDefaultBranches() {
  fDigitizers[kMvd]  = new CbmDigitizeInfo(kMvd,  "MvdPoint"  );
  fDigitizers[kSts]  = new CbmDigitizeInfo(kSts,  "StsPoint"  );
  fDigitizers[kRich] = new CbmDigitizeInfo(kRich, "RichPoint" );
  fDigitizers[kMuch] = new CbmDigitizeInfo(kMuch, "MuchPoint" );
  fDigitizers[kTrd]  = new CbmDigitizeInfo(kTrd,  "TrdPoint"  );
  fDigitizers[kTof]  = new CbmDigitizeInfo(kTof,  "TofPoint"  );
  fDigitizers[kPsd]  = new CbmDigitizeInfo(kPsd,  "PsdPoint"  );
}
// --------------------------------------------------------------------------



// -----   Set digitizer explicitly   ---------------------------------------
void CbmDigitization::SetDigitizer(Int_t system, FairTask* digitizer,
                               TString branch, Bool_t persistent) {

  // Digitizer already present: replace
  if ( fDigitizers.find(system) != fDigitizers.end() ) {
    LOG(WARNING) << fName << ": replacing "
        << fDigitizers[system]->GetDigitizer()->GetName()
        << " by " << digitizer->GetName() << FairLogger::endl;
    if ( ! branch.IsNull() ) fDigitizers[system]->SetBranchName(branch);
    fDigitizers[system]->SetDigitizer(digitizer);
    fDigitizers[system]->SetActive();
    fDigitizers[system]->SetPersistent(persistent);
  } //? digitizer present

  // Digitizer not yet present: add
  else fDigitizers[system] = new CbmDigitizeInfo(system, branch, digitizer,
                                                 kFALSE, kTRUE, persistent);

}
// --------------------------------------------------------------------------



// -----   Set the output file   --------------------------------------------
void CbmDigitization::SetOutputFile(TString path) {

  // --- If the directory does not yet exist, create it
  const char* directory = gSystem->DirName(path.Data());
  if ( gSystem->AccessPathName(directory) ) {
    Int_t success  = gSystem->mkdir(directory, kTRUE);
    if ( success == -1 ) LOG(FATAL) << fName << ": output directory "
        << directory << " does not exist and cannot be created!"
        << FairLogger::endl;
    else LOG(INFO) << fName << ": created directory " << directory
        << FairLogger::endl;
  }

  fOutFile = path;
}
// --------------------------------------------------------------------------



// -----    Set the ROOT parameter file   -----------------------------------
void CbmDigitization::SetParameterRootFile(TString fileName) {
  if ( gSystem->AccessPathName(fileName) )
    LOG(FATAL) << fName << ": parameter file " << fileName
    << " does not exist!" << FairLogger::endl;
  fParRootFile = fileName;
}
// --------------------------------------------------------------------------



ClassImp(CbmDigitization);


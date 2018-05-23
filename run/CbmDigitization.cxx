/** @file CbmDigitization.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 18.05.2018
 **/

#include "CbmDigitization.h"

#include <cassert>
#include "TClonesArray.h"
#include "FairFileSource.h"
#include "FairLogger.h"
#include "FairParAsciiFileIo.h"
#include "FairParRootFileIo.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "CbmMuchDigitizeGem.h"
#include "CbmMvdDigitizer.h"
#include "CbmPsdSimpleDigitizer.h"
#include "CbmRichDigitizer.h"
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
  fParFile(),
  fEventMode(kFALSE)
{
  SetDefaults();
}
// --------------------------------------------------------------------------



// -----   Destructor   -----------------------------------------------------
CbmDigitization::~CbmDigitization() {
}
// --------------------------------------------------------------------------



// -----   Add an input file   ----------------------------------------------
Int_t CbmDigitization::AddInput(TString fileName, Double_t eventRate) {
  fInputFiles.push_back(fileName);
  fEventRates.push_back(eventRate);
  return fInputFiles.size();
}
// --------------------------------------------------------------------------



// -----   Check input arrays   ---------------------------------------------
Int_t CbmDigitization::CheckInputArrays() {

  TClonesArray* test = nullptr;
  Int_t nArrays = 0;

  // --- Get FairRootManager instance
  FairRootManager* ioman = FairRootManager::Instance();
  assert ( ioman );

  // --- Search arrays for detector systems
  for (Int_t module = kRef; module < kNofSystems; module++) {
    if ( fDigitizers.find(module) != fDigitizers.end() ) {
      TString array = fDigitizers[module]->GetBranchName();
      test = dynamic_cast<TClonesArray*> (ioman->GetObject(array.Data()));
      if ( test ) {
        LOG(INFO) << fName << ": found array " << array << FairLogger::endl;
        fDigitizers[module]->SetPresent();
        nArrays++;
      } //? Array found
    } //? Array name known
  } //# Modules

  return nArrays;
}
// --------------------------------------------------------------------------



// -----   Check input arrays   ---------------------------------------------
Int_t CbmDigitization::CheckInputFile() {

  LOG(INFO) << fName << ": check input branches..." << FairLogger::endl;
  Int_t nBranches = 0;
  TFile* file = new TFile(fInputFiles[0]);
  assert(file);
  LOG(INFO) << fName << ": opening input file " << fInputFiles[0]
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

  delete tree;
  file->Close();
  delete file;
  return nBranches;
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



// -----   Execute digitisation run   ---------------------------------------
void CbmDigitization::Run(Int_t event1, Int_t event2) {

  // --- Look for input branches
  std::cout << std::endl << std::endl;
  LOG(INFO) << "==================================================="
      << FairLogger::endl;
  Int_t nBranches = CheckInputFile();
  TString word = (nBranches == 1 ? "branch" : "branches");
  LOG(INFO) << fName << ": " << nBranches << " input " << word << " found"
      << FairLogger::endl << FairLogger::endl;

  // --- Create default digitizers
  Int_t nDigis = CreateDigitizers();
  word = (nDigis == 1 ? " digitiser" : " digitisers");
  LOG(INFO) << fName << ": " << nDigis << word << " instantiated."
      << FairLogger::endl << FairLogger::endl;

  // --- Create FairRunAna
  FairRunAna* run = new FairRunAna();

  // --- Add input files
  Int_t nInputs = fInputFiles.size();
  for (Int_t iInput = 0; iInput < nInputs; iInput++) {
    FairFileSource* source = new FairFileSource(fInputFiles.at(iInput));
    //source->SetEventMeanTime(fEventRates.at(iInput));
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

  // --- Set runtime database
  std::cout << std::endl;
  LOG(INFO) << fName << ": setting runtime DB " << FairLogger::endl;
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIoRoot = new FairParRootFileIo();
  FairParAsciiFileIo* parIoAscii = new FairParAsciiFileIo();
  parIoRoot->open(fParFile.Data(), "UPDATE");
  rtdb->setFirstInput(parIoRoot);
  LOG(INFO) << fName << ": ROOt I/O is " << fParFile << FairLogger::endl;
  LOG(INFO) << "==================================================="
      << FairLogger::endl;


  // --- Initialise run
  std::cout << std::endl << std::endl;
  LOG(INFO) << "==================================================="
      << FairLogger::endl;
  LOG(INFO) << fName << ": initialising run..." << FairLogger::endl;
  run->Init();

  // --- Run digitisation
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




}
// --------------------------------------------------------------------------



// -----   Set default info   -----------------------------------------------
void CbmDigitization::SetDefaults() {
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


ClassImp(CbmDigitization);


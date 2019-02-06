/** @file CbmRunTransport.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 31.01.2019
 **/

#include "CbmRunTransport.h"

#include <cassert>
#include <iostream>
#include <string>
#include "TGeoManager.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "FairLogger.h"
#include "FairMonitor.h"
#include "FairParRootFileIo.h"
#include "FairPrimaryGenerator.h"
#include "FairRuntimeDb.h"
#include "FairRunSim.h"
#include "FairSystemInfo.h"
#include "FairUrqmdGenerator.h"
#include "CbmFieldMap.h"
#include "CbmFieldPar.h"
#include "CbmPlutoGenerator.h"
#include "CbmSetup.h"
#include "CbmTarget.h"
#include "CbmUnigenGenerator.h"



// -----   Constructor   ----------------------------------------------------
CbmRunTransport::CbmRunTransport() :
  TNamed("CbmRunTransport", "Transport Run"),
  fSetup(CbmSetup::Instance()),
  fTarget(nullptr),
  fEventGen(new FairPrimaryGenerator),
  fRun(new FairRunSim()),
  fOutFileName(),
  fParFileName(),
  fGeoFileName(),
  fGenerators(),
  fRealTimeInit(0.),
  fRealTimeRun(0.),
  fCpuTime(0.),
  fVertexSmearZ(kTRUE),
  fEngine(kGEANT3),
  fGenerateRunInfo(kFALSE)
{
  // TODO: I do not like instantiating FairRunSim from this constructor;
  // It should be done in Run(). However, the presence of a FairRunSim
  // is required by CbmUnigenGenerator. Not a good construction; should
  // be done better.
}
// --------------------------------------------------------------------------



// -----   Destructor   -----------------------------------------------------
CbmRunTransport::~CbmRunTransport() {
  LOG(DEBUG) << GetName() << ": Destructing" << FairLogger::endl;
  if ( fRun ) delete fRun;
  LOG(DEBUG) << GetName() << ": Destructing finshed." << FairLogger::endl;
}
// --------------------------------------------------------------------------



// -----   Add a file-based input   -----------------------------------------
void CbmRunTransport::AddInput(const char* fileName,
                               ECbmGenerator genType) {

  FairGenerator* generator = NULL;

  if ( gSystem->AccessPathName(fileName) ) {
    LOG(FATAL) << GetName() << ": Input file " << fileName << " not found!"
        << FairLogger::endl;
    return;
  }

  switch(genType) {
    case kUnigen: generator = new CbmUnigenGenerator(TString(fileName)); break;
    case kUrqmd: generator = new FairUrqmdGenerator(fileName); break;
    case kPluto: generator = new CbmPlutoGenerator(fileName); break;
  }

  assert(generator);
  fEventGen->AddGenerator(generator);

}
// --------------------------------------------------------------------------



// -----   Add a generator-based input   ------------------------------------
void CbmRunTransport::AddInput(FairGenerator* generator) {
  assert(generator);
  fEventGen->AddGenerator(generator);
}
// --------------------------------------------------------------------------



// -----   Load a standard setup   ------------------------------------------
void CbmRunTransport::LoadSetup(const char* setupName) {

  TString srcDir = gSystem->Getenv("VMCWORKDIR");
  TString setupFile = srcDir + "/geometry/setup/setup_" + setupName + ".C";
  if ( gSystem->AccessPathName(setupFile) ) {
    LOG(FATAL) << GetName() << ": Macro " << setupFile << " not found!"
        << FairLogger::endl;
    return;
  }
  TString setupFunc = "setup_";
  setupFunc = setupFunc + setupName + "()";
  LOG(INFO) << GetName() << ": Loading macro " << setupFile
      << FairLogger::endl;
  gROOT->LoadMacro(setupFile);
  gROOT->ProcessLine(setupFunc);

}
// --------------------------------------------------------------------------



// -----   Create and register the setup modules   --------------------------
void CbmRunTransport::RegisterSetup() {

}
// --------------------------------------------------------------------------



// -----   Execute transport run   ------------------------------------------
void CbmRunTransport::Run(Int_t nEvents) {

  // --- Timer
  TStopwatch timer;

  // --- Check presence of required requisites
  if ( fOutFileName.IsNull() ) {
    LOG(FATAL) << GetName() << ": No output file specified!"
        << FairLogger::endl;
  }
  if ( fParFileName.IsNull() ) {
    LOG(FATAL) << GetName() << ": No parameter file specified!"
        << FairLogger::endl;
  }
  std::cout << std::endl << std::endl;

  // --- Set transport engine
  const char* engineName = "";
  switch(fEngine) {
    case kGEANT3: engineName = "TGeant3"; break;
    case kGEANT4: engineName = "TGeant4"; break;
    default: {
      LOG(WARNING) << GetName() << ": Unknown transport engine "
          << FairLogger::endl;
        engineName = "TGeant3";
        break;
    }
  } //? engine
  LOG(INFO) << GetName() << ": Using engine " << engineName
      << FairLogger::endl;
  fRun->SetName(engineName);

  // --- Set output file name
  fRun->SetOutputFile(fOutFileName);

  // --- Set media file
  LOG(INFO) << GetName() << ": Media file is media.geo" << FairLogger::endl;
  fRun->SetMaterials("media.geo");

  // --- Create and register the setup modules
  TString macroName = gSystem->Getenv("VMCWORKDIR");
  macroName += "/macro/run/modules/registerSetup.C";
  gROOT->LoadMacro(macroName);
  gROOT->ProcessLine("registerSetup()");

  // --- Create and register the target
  if ( fTarget ) {
    fTarget->Print();
    fRun->AddModule(fTarget);
  }
  else LOG(WARNING) << GetName() << ": No target defined!"
      << FairLogger::endl;

  // --- Create the magnetic field
  LOG(INFO) << GetName() << ": Register magnetic field" << FairLogger::endl;
  CbmFieldMap* magField = fSetup->CreateFieldMap();
  assert(magField);
  fRun->SetField(magField);


  // --- Set event vertex smearing in z according to target thickness
  // TODO: This is only correct if the target position is the origin.
  // The desired behaviour if the target is shifted has yet to be defined.
  if ( fVertexSmearZ ) {

    if ( fTarget ) {
      Double_t rot = 0.;
      fTarget->GetRotation(rot);
      assert(TMath::Abs(rot) < 85.); // avoid division by zero
      rot *= TMath::DegToRad();
      Double_t dz = fTarget->GetThickness() / TMath::Cos(rot);
      fEventGen->SetTarget(0., dz);
      fEventGen->SmearVertexZ(kTRUE);
      LOG(INFO) << GetName() << ": Event vertex z will be distributed from "
          << -0.5*dz << " cm to " << 0.5*dz << "cm." << FairLogger::endl;
    } //? Target present
    else {
      fEventGen->SetTarget(0., 0.);
      fEventGen->SmearVertexZ(kFALSE);
      LOG(INFO) << GetName()
          << ": No target; event vertex z will be fixed at 0."
          << FairLogger::endl;
    } //? No target present

  } //? Vertex smearing enabled

  else {
    fEventGen->SetTarget(0., 0.);
    fEventGen->SmearVertexZ(kFALSE);
    LOG(INFO) << GetName()
        << ": Event vertex z will be fixed at 0."
        << FairLogger::endl;
  } //? Vertex smearing disabled


  // --- Register the primary generator
  fRun->SetGenerator(fEventGen);


  // --- Trigger generation of run info
  fRun->SetGenerateRunInfo(fGenerateRunInfo);


  // --- Initialise run
  fRun->Init();

  // --- Runtime database
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  CbmFieldPar* fieldPar =
      static_cast<CbmFieldPar*>(rtdb->getContainer("CbmFieldPar"));
  fieldPar->SetParameters(magField);
  fieldPar->setChanged();
  FairParRootFileIo* parOut = new FairParRootFileIo(kTRUE);
  parOut->open(fParFileName.Data());
  rtdb->setOutput(parOut);
  rtdb->saveOutput();
  rtdb->print();

  // --- Measure time for initialisation
  timer.Stop();
  fRealTimeInit = timer.RealTime();

  // --- Start run
  timer.Start(kFALSE);  // without reset
  fRun->Run(nEvents);
  timer.Stop();
  fRealTimeRun = timer.RealTime() - fRealTimeInit;
  fCpuTime = timer.CpuTime();

  // --- Create a geometry file if required
  if ( ! fGeoFileName.IsNull() ) fRun->CreateGeometryFile(fGeoFileName);

  // --- Screen log
  std::cout << std::endl << std::endl;
  LOG(INFO) << GetName() << ": Run finished successfully" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Wall time for Init : " << fRealTimeInit << " s "
      << FairLogger::endl;
  LOG(INFO) << GetName() << ": Wall time for Run  : " << fRealTimeRun << " s "
      << FairLogger::endl;
  LOG(INFO) << GetName() << ": Output file    : " << fOutFileName
      << FairLogger::endl;
  LOG(INFO) << GetName() << ": Parameter file : " << fParFileName
      << FairLogger::endl;
  if ( ! fGeoFileName.IsNull() ) LOG(INFO) << GetName() << ": Geometry file: "
      << fGeoFileName << FairLogger::endl;
  std::cout << std::endl << std::endl;


  // --- Remove TGeoManager
  // To avoid crashes when exiting. Reason for this behaviour is unknown.
  if (gGeoManager) {
    if (gROOT->GetVersionInt() >= 60602) {
      gGeoManager->GetListOfVolumes()->Delete();
      gGeoManager->GetListOfShapes()->Delete();
    }
    delete gGeoManager;
  }

}
// --------------------------------------------------------------------------



// -----   Set the beam angle distribution   --------------------------------
void CbmRunTransport::SetBeamAngle(Double_t x0, Double_t y0,
                                   Double_t sigmaX, Double_t sigmaY) {
  assert(fEventGen);
  fEventGen->SetBeamAngle(x0, y0, sigmaX, sigmaY);
}
// --------------------------------------------------------------------------



// -----   Set the beam position   ------------------------------------------
void CbmRunTransport::SetBeamPosition(Double_t x0, Double_t y0,
                                      Double_t sigmaX, Double_t sigmaY) {
  assert(fEventGen);
  fEventGen->SetBeam(x0, y0, sigmaX, sigmaY);
}
// --------------------------------------------------------------------------



// -----   Set geometry file name   -----------------------------------------
void CbmRunTransport::SetGeoFileName(TString fileName) {

  // Check for the directory
  std::string name = fileName.Data();
  Int_t found = name.find_last_of("/");
  if ( found >= 0 ) {
    TString geoDir = name.substr(0, found);
    if ( gSystem->AccessPathName(geoDir.Data()) ) {
      LOG(ERROR) << GetName() << ": Directory for geometry file " << geoDir
          << " does not exist; the file will not be created."
          << FairLogger::endl;
      return;
    } //? Directory of geometry file does not exist
  } //? File name contains directory path

  fGeoFileName = fileName;

}
// --------------------------------------------------------------------------



// -----   Set parameter file name   ----------------------------------------
void CbmRunTransport::SetParFileName(TString fileName) {

  // --- If file does not exist, check the directory
  if ( gSystem->AccessPathName(fileName) ) {
    std::string name = fileName.Data();
    Int_t found = name.find_last_of("/");
    if ( found >= 0 ) {
      TString parDir = name.substr(0, found);
      if ( gSystem->AccessPathName(parDir.Data()) ) {
        LOG(FATAL) << GetName() << ": Parameter directory " << parDir
            << " does not exist!" << FairLogger::endl;
        return;
      } //? Directory of parameter file does not exist
    } //? File name contains directory path
  } //? Parameter file does not exist

  fParFileName = fileName;

}
// --------------------------------------------------------------------------



// -----   Set output file name   -------------------------------------------
void CbmRunTransport::SetOutFileName(TString fileName) {

  // Check for the directory
  std::string name = fileName.Data();
  Int_t found = name.find_last_of("/");
  if ( found >= 0 ) {
    TString outDir = name.substr(0, found);
    if ( gSystem->AccessPathName(outDir.Data()) ) {
      LOG(FATAL) << GetName() << ": Output directory " << outDir
          << " does not exist!" << FairLogger::endl;
      return;
    } //? Directory of output file does not exist
  } //? File name contains directory path

  fOutFileName = fileName;

}
// --------------------------------------------------------------------------



// -----   Define the target   ----------------------------------------------
void CbmRunTransport::SetTarget(const char* medium, Double_t thickness,
                                Double_t diameter) {

  if ( fTarget ) {
    LOG(WARNING) << GetName() << ": Overriding existing target! "
        << FairLogger::endl;
    delete fTarget;
  }
  fTarget = new CbmTarget(medium, thickness, diameter);

}
// --------------------------------------------------------------------------



// -----   Define the target position   -------------------------------------
void CbmRunTransport::SetTargetPosition(Double_t x, Double_t y, Double_t z) {

  if ( ! fTarget ) LOG(ERROR) << GetName()
      << ": No target defined; statement ignored!" << FairLogger::endl;
  else fTarget->SetPosition(x, y, z);

}
// --------------------------------------------------------------------------



// -----   Define the target rotation   -------------------------------------
void CbmRunTransport::SetTargetRotation(Double_t angle) {

  if ( ! fTarget )  LOG(ERROR) << GetName()
      << ": No target defined; statement ignored!" << FairLogger::endl;
  else fTarget->SetRotation(angle);

}
// --------------------------------------------------------------------------



// -----   Enable vertex distribution in x and y   --------------------------
void CbmRunTransport::SetVertexSmearXY(Bool_t choice) {
  assert(fEventGen);
  fEventGen->SmearGausVertexXY(choice);
}
// --------------------------------------------------------------------------




ClassImp(CbmRunTransport);


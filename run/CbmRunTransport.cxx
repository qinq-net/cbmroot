/** @file CbmRunTransport.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 31.01.2019
 **/

#include "CbmRunTransport.h"


#include <cassert>
#include <iostream>
#include <string>
#include "TG4RunConfiguration.h"
#include "TGeant3.h"
#include "TGeant3TGeo.h"
#include "TGeant4.h"
#include "TGeoManager.h"
#include "TPythia6Decayer.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TVirtualMC.h"
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
#include "CbmStack.h"
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
fEngine(kGeant3),
fStackFilter(new CbmStackFilter()),
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
  //if ( fRun ) delete fRun;
  LOG(DEBUG) << GetName() << ": Destructing finished." << FairLogger::endl;
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



// -----   Configure the TVirtualMC   ---------------------------------------
void CbmRunTransport::ConfigureVMC() {

  std::cout << std::endl;
  LOG(INFO) << GetName() << ": Configuring VMC..." << FairLogger::endl;
  TVirtualMC* vmc = nullptr;

  if ( fEngine == kGeant3 ) {
    TString* gModel = fRun->GetGeoModel();
    if ( strncmp(gModel->Data(), "TGeo", 4) == 0 ) {
      LOG(INFO) << GetName() << ": Create TGeant3TGeo" << FairLogger::endl;
      vmc = new TGeant3TGeo("C++ Interface to Geant3 with TGeo");
    } //? Geant3 with TGeo
    else {
      LOG(INFO) << GetName() << ": Create TGeant3" << FairLogger::endl;
      vmc = new TGeant3("C++ Interface to Geant3");
    } //? Native Geant3
    Geant3Settings(dynamic_cast<TGeant3*>(vmc));
  } //? Geant3

  else if ( fEngine == kGeant4 ) {
    LOG(INFO) << GetName() << ": Create TGeant4" << FairLogger::endl;
    TG4RunConfiguration* runConfig
    = new TG4RunConfiguration("geomRoot", "QGSP_BERT_EMV+optical",
                              "stepLimiter+specialCuts");
    vmc = new TGeant4("TGeant4", "C++ Interface to Geant4", runConfig);
    Geant4Settings(dynamic_cast<TGeant4*>(vmc));
  } //? Geant4

  else {
    LOG(FATAL) << GetName() << ": unknown transport engine!"
        << FairLogger::endl;
  }

  // Common VMC settings
  if ( vmc ) VMCSettings(vmc);

  // Create stack
  std::unique_ptr<CbmStack> stack(new CbmStack());
  stack->SetFilter(fStackFilter);
  if ( vmc ) vmc->SetStack(stack.release());

}
// --------------------------------------------------------------------------



// -----   Specific settings for GEANT3   -----------------------------------
void CbmRunTransport::Geant3Settings(TGeant3* vmcg3) {

  assert(vmcg3);
  LOG(INFO) << GetName() << ": Configuring Geant3" << FairLogger::endl;

  // TODO: These settings were taken from g3config.C. Their meanings will
  // have to be looked up and documented.
  vmcg3->SetTRIG(1);         //Number of events to be processed
  vmcg3->SetSWIT(4, 100);
  vmcg3->SetDEBU(0, 0, 1);
  vmcg3->SetRAYL(1);
  vmcg3->SetSTRA(0);
  vmcg3->SetAUTO(1);         //Select automatic STMIN etc... calc. (AUTO 1) or manual (AUTO 0)
  vmcg3->SetABAN(2);         //Restore 3.16 behaviour for abandoned tracks
  vmcg3->SetOPTI(2);         //Select optimisation level for GEANT geometry searches (0,1,2)
  vmcg3->SetERAN(5.e-7);
  vmcg3->SetCKOV(1);        // cerenkov photons

}
// --------------------------------------------------------------------------



// -----   Specific settings for GEANT4   -----------------------------------
void CbmRunTransport::Geant4Settings(TGeant4* vmc) {

  assert(vmc);

  // TODO: These settings were taken over from g4Config.C. To be documented.

  // --- Set external decayer (Pythia) if required
  if(FairRunSim::Instance()->IsExtDecayer()){
    TVirtualMCDecayer* decayer = TPythia6Decayer::Instance();
    vmc->SetExternalDecayer(decayer);
    LOG(INFO) << GetName() << ": Using Phythia6 decayer"
        << FairLogger::endl;
  }

  // --- Random seed and maximum number of steps
  Text_t buffer[50];
  sprintf(buffer,"/random/setSeeds %i  %i ",gRandom->GetSeed(), gRandom->GetSeed());
  vmc->ProcessGeantCommand(buffer);
  vmc->SetMaxNStep(10000);  // default is 30000

  // --- Execute Geant4 configuration macro
  TString configMacro(gSystem->Getenv("VMCWORKDIR"));
  configMacro += "/gconfig/g4config.in";
  LOG(INFO) << GetName() << ": Using Geant4 configuration from "
      << configMacro << FairLogger::endl;
  vmc->ProcessGeantMacro(configMacro.Data());

}
// --------------------------------------------------------------------------




// -----   Load a standard setup   ------------------------------------------
void CbmRunTransport::LoadSetup(const char* setupName) {

  // TODO: The macro code should be transferred into compiled code.

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

  //LOG(INFO) << fSetup->ToString() << FairLogger::endl;

}
// --------------------------------------------------------------------------



// -----   Register radiation length   --------------------------------------
void CbmRunTransport::RegisterRadLength(Bool_t choice) {
  assert(fRun);
  fRun->SetRadLenRegister(choice);
  LOG(INFO) << GetName() << ": Radiation length register is enabled"
      << FairLogger::endl;
}
// --------------------------------------------------------------------------



// -----   Create and register the setup modules   --------------------------
void CbmRunTransport::RegisterSetup() {

}
// --------------------------------------------------------------------------



// -----   Set correct decay modes for pi0 and eta   ------------------------
void CbmRunTransport::PiAndEtaDecay(TVirtualMC* vmc) {

  assert(vmc);
  LOG(INFO) << GetName() << ": Set decay modes for pi0 and eta"
      << FairLogger::endl;

  // Decay modes for eta mesons (PDG 2016)
  Int_t modeEta[6][3];    // decay modes
  Float_t brEta[6];       // branching ratios in %

  // --- eta -> gamma gamma
  modeEta[0][0] = 22;
  modeEta[0][1] = 22;
  modeEta[0][2] =  0;
  brEta[0] = 39.41;

  // --- eta -> pi0 pi0 pi0
  modeEta[1][0] = 111;
  modeEta[1][1] = 111;
  modeEta[1][2] = 111;
  brEta[1] = 32.68;

  // --- eta -> pi+ pi- pi0
  modeEta[2][0] =  211;
  modeEta[2][1] = -211;
  modeEta[2][2] =  111;
  brEta[2] = 22.92;

  // --- eta -> pi+ pi- gamma
  modeEta[3][0] =  211;
  modeEta[3][1] = -211;
  modeEta[3][2] =   22;
  brEta[3] = 4.22;

  // --- eta -> e+ e- gamma
  modeEta[4][0] =  11;
  modeEta[4][1] = -11;
  modeEta[4][2] =  22;
  brEta[4] = 0.69;

  // --- eta -> pi0 gamma gamma
  modeEta[5][0] = 111;
  modeEta[5][1] =  22;
  modeEta[5][2] =  22;
  brEta[5] = 2.56e-2;

  // --- Set the eta decays
  vmc->SetDecayMode(221, brEta, modeEta);

  // --- Decay modes for pi0
  Int_t modePi[6][3];    // decay modes
  Float_t brPi[6];       // branching ratios in %

  // --- pi0 -> gamma gamma
  modePi[0][0] = 22;
  modePi[0][1] = 22;
  modePi[0][2] =  0;
  brPi[0] = 98.823;

  // --- pi0 -> e+ e- gamma
  modePi[1][0] =  11;
  modePi[1][1] = -11;
  modePi[1][2] =  22;
  brPi[1] = 1.174;

  // --- No other channels for pi0
  for (Int_t iMode = 2; iMode < 6; iMode++) {
    modePi[iMode][0] = 0;
    modePi[iMode][1] = 0;
    modePi[iMode][2] = 0;
    brPi[iMode] = 0.;
  }

  // --- Set the pi0 decays
  vmc->SetDecayMode(111, brPi, modePi);

}
// --------------------------------------------------------------------------



// -----   Execute transport run   ------------------------------------------
void CbmRunTransport::Run(Int_t nEvents) {

  // --- Timer
  TStopwatch timer;

  //LOG(INFO) << "geant321  " << gSystem->Load("libgeant321.so") << FairLogger::endl;;


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
    case kGeant3: engineName = "TGeant3"; break;
    case kGeant4: engineName = "TGeant4"; break;
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


  // --- Set VMC configuration
  // This will work only with FairRoot v18 and later
  std::function<void()> f = std::bind(&CbmRunTransport::ConfigureVMC, this);
  fRun->SetSimSetup(f);


  // --- Initialise run
  fRun->Init();


  // --- Set correct decay modes for pi0 and eta
  PiAndEtaDecay(gMC);


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



// -----   VMC settings   ---------------------------------------------------
void CbmRunTransport::VMCSettings(TVirtualMC* vmc) {

  LOG(INFO) << GetName() << ": Configuring VMC" << FairLogger::endl;
  assert(vmc);

  // TODO: These settings were taken over from SetCuts.C. Their meanings
  // will have to be looked up and documented.

  // Processes
  vmc->SetProcess("PAIR",1); // pair production
  vmc->SetProcess("COMP",1); // Compton scattering
  vmc->SetProcess("PHOT",1); // photo electric effect
  vmc->SetProcess("PFIS",0); // photofission
  vmc->SetProcess("DRAY",1); // delta-ray
  vmc->SetProcess("ANNI",1); // annihilation
  vmc->SetProcess("BREM",1); // bremsstrahlung
  vmc->SetProcess("HADR",1); // hadronic process
  vmc->SetProcess("MUNU",1); // muon nuclear interaction
  vmc->SetProcess("DCAY",1); // decay
  vmc->SetProcess("LOSS",1); // energy loss
  vmc->SetProcess("MULS",1); // multiple scattering
  Double_t cut1=1.0E-3; //GeV
  Double_t tofmax = 1.0; //seconds

  // Cuts
  Double_t energyCut = 1.e-3; // GeV
  Double_t tofCut = 1.0; // s
  vmc->SetCut("CUTGAM", energyCut);   // gammas (GeV)
  vmc->SetCut("CUTELE", energyCut);   // electrons (GeV)
  vmc->SetCut("CUTNEU", energyCut);   // neutral hadrons (GeV)
  vmc->SetCut("CUTHAD", energyCut);   // charged hadrons (GeV)
  vmc->SetCut("CUTMUO", energyCut);   // muons (GeV)
  vmc->SetCut("BCUTE",  energyCut);   // electron bremsstrahlung (GeV)
  vmc->SetCut("BCUTM",  energyCut);   // muon and hadron bremsstrahlung(GeV)
  vmc->SetCut("DCUTE",  energyCut);   // delta-rays by electrons (GeV)
  vmc->SetCut("DCUTM",  energyCut);   // delta-rays by muons (GeV)
  vmc->SetCut("PPCUTM", energyCut);   // direct pair production by muons (GeV)
  vmc->SetCut("TOFMAX", tofCut);      // time of flight cut in seconds

}
// --------------------------------------------------------------------------



ClassImp(CbmRunTransport);


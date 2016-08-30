// --------------------------------------------------------------------------
//
// Macro for reconstruction of simulated events with standard settings
//
// HitProducers in MVD, RICH, TRD, TOF, ECAL
// Digitizer and HitFinder in STS
// FAST MC for ECAL
// STS track finding and fitting (L1 / KF)
// TRD track finding and fitting (L1 / KF)
// RICH ring finding (ideal) and fitting
// Global track finding (ideal), rich assignment
// Primary vertex finding (ideal)
// Matching of reconstructed and MC tracks in STS, RICH and TRD
//
// V. Friese   24/02/2006
// Version     24/04/2007 (V. Friese)
//
// --------------------------------------------------------------------------


void run_reco_timeslice( Int_t ifile = 1, Int_t nEvents = 2500)
 {

  // ========================================================================
  //          Adjust this part according to your requirements

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 0;
  TString setup="sis100_electron";
  // Input file (MC events)
   TString outDir  = "data/";
   TString inFile = "raw_slice.root";
   TString parFile = "params.root";
   TString outFile = "slice_reco.root";
//   TString logLevel = "DEBUG2";  
   TString logLevel = "INFO";


  //  Digitisation files.
  // Add TObjectString containing the different file names to
  // a TList which is passed as input to the FairParAsciiFileIo.
  // The FairParAsciiFileIo will take care to create on the fly 
  // a concatenated input parameter file which is then used during
  // the reconstruction.

  TList *parFileList = new TList();

  TString inDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = inDir + "/parameters/";

  TString setupFile = inDir + "/geometry/setup/" + setup + "_setup.C";
  TString setupFunct = setup;
  setupFunct += "_setup()";

  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);

  TObjString stsDigiFile = paramDir;
  parFileList->Add(&stsDigiFile);
//  cout << "macro/run/run_reco.C using: " << stsDigi << endl;

gSystem->Load("libMinuit2"); // Nedded for rich ellipse fitter

  // In general, the following parts need not be touched
  // ========================================================================

  TString mvdTag       = "v15a";
  TString stsTag       = "v15c";
  TString mvdMatBudget="mvd/mvd_matbudget_" + mvdTag + ".root";
  TString stsMatBudget="sts/sts_matbudget_" + stsTag + ".root";
  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------


  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *run = new FairRunAna();
  CbmEvBuildSource* src=new CbmEvBuildSource(inFile);
  run->SetSource(src);
  run->SetOutputFile(outFile);
  // ------------------------------------------------------------------------

  CbmMCDataManager* evManager=new CbmMCDataManager("evManager");
  evManager->AddFile("urqmd.mc.root");
  run->AddTask(evManager);

  // ---- Set the log level 	
  FairLogger* gLogger = FairLogger::GetLogger();
  gLogger->SetLogScreenLevel(logLevel.Data());

  // =========================================================================
  // ===                      STS local reconstruction                     ===
  // =========================================================================

/*
  CbmEventMatching* evMatch=new CbmEventMatching();
  evMatch->SetWriteTree();
  run->AddTask(evMatch);
*/
  // -----   STS Cluster Finder   --------------------------------------------
//  FairTask* stsClusterFinder = new CbmStsClusterFinder();
//  run->AddTask(stsClusterFinder);
  // -------------------------------------------------------------------------


  // -----   STS digitizer   -------------------------------------------------
  // --- The following settings correspond to the settings for the old
  // --- digitizer in run_reco.C
  Double_t dynRange       =   40960.;  // Dynamic range [e]
  Double_t threshold      =    4000.;  // Digitisation threshold [e]
  Int_t nAdc              =      32;   // Number of ADC channels (12 bit)
  Double_t timeResolution =       5.;  // time resolution [ns]
  Double_t deadTime       = 9999999.;  // infinite dead time (integrate entire event)
  Double_t noise          =       0.;  // ENC [e]
  Int_t digiModel         = 2;  // Model: 1 = uniform charge distribution along track
  CbmStsDigitize* stsDigi=new CbmStsDigitize(digiModel);
  stsDigi->SetParameters(dynRange, threshold, nAdc, timeResolution,
  		                   deadTime, noise);
  // -----   STS Cluster Finder   --------------------------------------------
  FairTask* stsClusterFinder = new CbmStsFindClusters();
  run->AddTask(stsClusterFinder);
  // -------------------------------------------------------------------------
  // -----   STS hit finder   ------------------------------------------------
  FairTask* stsFindHits = new CbmStsFindHits();
  run->AddTask(stsFindHits);
  // -------------------------------------------------------------------------


  // -----  STS hit matching   -----------------------------------------------
//  FairTask* stsMatchHits = new CbmStsMatchHits();
//  run->AddTask(stsMatchHits);
  // -------------------------------------------------------------------------

  // ---  STS track finding   ------------------------------------------------
  CbmKF* kalman = new CbmKF();
  run->AddTask(kalman);
  CbmL1* l1 = new CbmL1("CbmL1");
  TString mvdMatBudgetFileName = paramDir + mvdMatBudget;
  TString stsMatBudgetFileName = paramDir + stsMatBudget;
  l1->SetStsMaterialBudgetFileName(stsMatBudgetFileName.Data());
  l1->SetMvdMaterialBudgetFileName(mvdMatBudgetFileName.Data());
  run->AddTask(l1);

  CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
  FairTask* stsFindTracks = new CbmStsFindTracks(iVerbose, stsTrackFinder);
  run->AddTask(stsFindTracks);
  // -------------------------------------------------------------------------


  // ---   STS track matching   ----------------------------------------------
//  FairTask* stsMatchTracks = new CbmStsMatchTracks(0);
//  run->AddTask(stsMatchTracks);
  // -------------------------------------------------------------------------

/*
  // ---   STS track fitting   -----------------------------------------------
  CbmStsTrackFitter* stsTrackFitter = new CbmStsKFTrackFitter();
  FairTask* stsFitTracks = new CbmStsFitTracks(stsTrackFitter, 0);
  run->AddTask(stsFitTracks);
  // -------------------------------------------------------------------------
*/
  CbmMatchRecoToMC* matchTask = new CbmMatchRecoToMC();
  run->AddTask(matchTask);

//  CbmEcalAnalysisSts* anaSts=new CbmEcalAnalysisSts("ana", 1);
//  run->AddTask(anaSts);

/*  
  // -----   STS hit finding QA   ----------------------------------------
  FairTask* stsFTQa = new CbmStsFindTracksQa(4,0.7,1000);
  run->AddTask(stsFTQa);
  // ------------------------------------------------------------------------
*/

    // ----- PID for KF Particle Finder --------------------------------------------
  CbmKFParticleFinderPID* kfParticleFinderPID = new CbmKFParticleFinderPID();
  kfParticleFinderPID->SetPIDMode(0);
  kfParticleFinderPID->SetSIS100(); 
//  kfParticleFinderPID->UseMuch();
//  kfParticleFinderPID->SetNMinMuchHitsForLMVM(11);
//  kfParticleFinderPID->SetNMinMuchHitsForJPsi(11);
  run->AddTask(kfParticleFinderPID);
  
  // ----- KF Particle Finder --------------------------------------------
  CbmKFParticleFinder* kfParticleFinder = new CbmKFParticleFinder();
  kfParticleFinder->SetPIDInformation(kfParticleFinderPID);
//   kfParticleFinder->SetPVToZero();
//   kfParticleFinder->SetSuperEventAnalysis();
  run->AddTask(kfParticleFinder);
/*
  // ----- KF Particle Finder QA --------------------------------------------
  CbmKFParticleFinderQA* kfParticleFinderQA = new CbmKFParticleFinderQA("CbmKFParticleFinderQA", 0, kfParticleFinder->GetTopoReconstructor(),"slice_histoFile.root");
  kfParticleFinderQA->SetPrintEffFrequency(100);
  kfParticleFinderQA->SetEffFileName("slice_effFile.root");
//   kfParticleFinderQA->SetSuperEventAnalysis();
  run->AddTask(kfParticleFinderQA);
*/
  CbmEventMatching* evMatch=new CbmEventMatching();
  evMatch->SetWriteTree();
  run->AddTask(evMatch);
 
  // ===                 End of STS local reconstruction                   ===
  // =========================================================================

 // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data());
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------


  // -----   Intialise and run   --------------------------------------------
  run->Init();
  stsDigi->InitSetup();
  stsDigi->SetSensorTypes();
  stsDigi->SetModuleParameters();
  stsDigi->SetSensorConditions();
//  cout << CbmStsSetup::Instance()->Init(gGeoManager) << endl;
  cout << "Starting run " << gGeoManager << endl;
  run->Run(0, nEvents);
  // ------------------------------------------------------------------------

  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is " << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------

//  delete run;

  cout << " Test passed" << endl;
	cout << " All ok " << endl;
}

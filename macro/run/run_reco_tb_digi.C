// --------------------------------------------------------------------------
//
// Macro for reconstruction of simulated time slices
// Using event building from STS digis and event-based reconstruction chain
//
// STS chain only:
// Event building from STS digis
// STS cluster finder (event-based)
// STS hit finder (event-based)
// STS track finder (L1)
// Simple QA
//
// Run the macro run_digi_tb.C before to generate the input file .raw.root.
//
// V. Friese   14/03/2017
//
// --------------------------------------------------------------------------


using std::cout;
using std::endl;

void run_reco_tb_digi()
 {

  // =========================================================================
  // ===                      Settings                                     ===
  // =========================================================================
  
  
  // --- File names
  TString setupName = "sis100_electron";
  TString outDir  = "data/";
  TString inFile  = outDir + setupName + "_test.raw.root";   // Input file (MC events)
  TString mcFile  = outDir + setupName + "_test.mc.root";    // Transport file
  TString parFile = outDir + setupName + "_params.root";     // Parameter file
  TString outFile = outDir + setupName + "_test.reco.root";  // Output file

  // Log level
  TString logLevel = "INFO";  // switch to DEBUG or DEBUG1,... for more info
  TString logVerbosity = "LOW"; // switch to MEDIUM or HIGH for more info

  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ========================================================================



  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *run = new FairRunAna();
  run->SetInputFile(inFile);
  run->SetOutputFile(outFile);
  run->SetGenerateRunInfo(kTRUE);
  Bool_t hasFairMonitor = Has_Fair_Monitor();
  if (hasFairMonitor) {
    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  }
  // ------------------------------------------------------------------------


  // ---- Set the log level 	
  FairLogger::GetLogger()->SetLogScreenLevel(logLevel.Data());
  FairLogger::GetLogger()->SetLogVerbosityLevel(logVerbosity.Data());
  
  
  

  // =========================================================================
  // ===                      Reconstruction chain                         ===
  // =========================================================================
  
  // --- Event builder (from digis)
  // --- In order to perform event-by-event reconstruction, use the 
  // --- ideal event builder (CbmBuildEventsIdeal) instead.
  run->AddTask(new CbmBuildEventsSimple());
  
  // --- STS cluster finder (event-based)
  CbmStsFindClustersStream* stsCluster = new CbmStsFindClustersStream();
  stsCluster->UseEventMode();
  run->AddTask(stsCluster);

  // --- STS hit finder (event-based)
  run->AddTask(new CbmStsFindHitsEvents());
  
  // --- STS track finder (event-based)
  run->AddTask(new CbmKF());
  run->AddTask(new CbmL1());
  CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
  FairTask* stsFindTracks = new CbmStsFindTracksEvents(stsTrackFinder);
  run->AddTask(stsFindTracks);
  
  // --- Simple QA task to check basics
  run->AddTask(new CbmStsRecoQa());
 




  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open(parFile.Data());
  rtdb->setFirstInput(parIo1);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------


  // -----   Initialise and run   -------------------------------------------
  run->Init();
  cout << "Starting run " << gGeoManager << endl;
  run->Run();
  // ------------------------------------------------------------------------


  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished successfully." << endl;
  cout << "Output file is " << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------


  cout << " Test passed" << endl;
  cout << " All ok " << endl;
  RemoveGeoManager();
}

// --------------------------------------------------------------------------
//
// Macro for reconstruction of simulated time slices
// Using time-based reconstruction in the STS
//
// STS cluster finder (time-based)
// STS hit finder (time-based)
// STS track finder (time-based)
// Event building from STS tracks
// Simple QA
//
// V. Friese   14/03/2017
//
// --------------------------------------------------------------------------

using std::cout;
using std::endl;

void run_reco_tb_track(TString dataSet = "test", Int_t nSlices = -1)
 {

  // =========================================================================
  // ===                      Settings                                     ===
  // =========================================================================
  
  
  // --- File names
  TString setupName = "sis100_electron";
  TString outDir  = "data/";
  TString inFile  = dataSet + ".raw.root";     // Input file (digis)
  TString parFile = dataSet + ".par.root";     // Parameter file
  TString outFile = dataSet + ".rec.root";     // Output file

  // Log level
  TString logLevel = "INFO";  // switch to DEBUG or DEBUG1,... for more info
  TString logVerbosity = "LOW"; // switch to MEDIUM or HIGH for more info
  


  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  //gSystem->Load("libLittrack.so");

  // ========================================================================



  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------


  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *run = new FairRunAna();
  FairFileSource* inputSource = new FairFileSource(inFile);
  run->SetSource(inputSource);
  run->SetOutputFile(outFile);
  run->SetGenerateRunInfo(kTRUE);

  TString monitorFile{outFile};
  monitorFile.ReplaceAll("rec","rec.monitor");
  FairMonitor::GetMonitor()->EnableMonitor(kTRUE, monitorFile);
  // ------------------------------------------------------------------------


  // ---- Set the log level   -----------------------------------------------
  FairLogger::GetLogger()->SetLogScreenLevel(logLevel.Data());
  FairLogger::GetLogger()->SetLogVerbosityLevel(logVerbosity.Data());
  // ------------------------------------------------------------------------
  
  
  

  // =========================================================================
  // ===                      Reconstruction chain                         ===
  // =========================================================================
  

  // --- STS cluster finder
  // --- Parameters for STS modules
  CbmStsFindClusters* stsCluster = new CbmStsFindClusters();
  run->AddTask(stsCluster);
  
  // --- STS hit finder
  CbmStsFindHits* stsHit = new CbmStsFindHits();
  stsHit->SetDTime(20.);
  run->AddTask(stsHit);
  
  // --- STS track finder
  run->AddTask(new CbmKF());
  CbmL1* l1 = new CbmL1();
  l1->SetDataMode(1);
  run->AddTask(l1);
  CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
  FairTask* stsFindTracks = new CbmStsFindTracks(0, stsTrackFinder);
  run->AddTask(stsFindTracks);
  
  // --- Event builder (track-based)
  run->AddTask(new CbmBuildEventsFromTracksReal());
    
  // --- Simple QA task to check basics
  run->AddTask(new CbmStsRecoQa());
  





  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data(),"UPDATE");
  rtdb->setFirstInput(parIo1);
  // ------------------------------------------------------------------------


  // -----   Initialise and run   --------------------------------------------
  run->Init();

  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  rtdb->print();

  cout << "Starting run " << gGeoManager << endl;
  if ( nSlices < 0 ) run->Run();
  else run->Run(nSlices);
  // ------------------------------------------------------------------------


  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  FairMonitor::GetMonitor()->Print();
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

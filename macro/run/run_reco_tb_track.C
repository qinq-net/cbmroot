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

void run_reco_tb_track()
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
  
  // TOF digitisation parameters
  // For some reason that nobody seems to be able to explain, the TOF
  // digitisation parameters cannot be retrieved from the parameter file,
  // in which they are supposed to be stored during digitisation.
  // They are needed, however, for the TOF cluster finder. So, we read them
  // here from the ASCII file. Note that there is no guarantee that you use
  // the same parameters during digitisation and reconstruction this way.
  // One cannot help asking what we need a parameter database for, then.
  TList *parFileList = new TList();
  TString inDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = inDir + "/parameters/";
  TString setupFile = inDir + "/geometry/setup/setup_" + setupName + ".C";
  TString setupFunct = "setup_";
  setupFunct += setupName;
  setupFunct += "()";
  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);
  TString geoTag;
  CbmSetup* setupObject = CbmSetup::Instance();
  if ( setupObject->GetGeoTag(kTof, geoTag) ) {
  	TObjString* tofFile = new TObjString(paramDir + "tof/tof_" + geoTag + ".digi.par");
  	TObjString* tofBdfFile = new TObjString(paramDir + "tof/tof_" + geoTag + ".digibdf.par");
  	parFileList->Add(tofFile);
  	parFileList->Add(tofBdfFile);
  }
  
  

  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  gSystem->Load("libLittrack.so");

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
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  // ------------------------------------------------------------------------


  // -----   Initialise and run   --------------------------------------------
  run->Init();

  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  rtdb->print();

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

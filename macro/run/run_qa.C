// --------------------------------------------------------------------------
//
// Macro for STS QA
//
// V. Friese   13/01/2006
//
// --------------------------------------------------------------------------
void run_qa(Int_t nEvents = 1, const char* setupName = "sis100_electron")
{

  // ========================================================================
  //          Adjust this part according to your requirements

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 1;

  TString outDir  = "data/";

  // MC file
  TString simFile = outDir + setupName + "_test.mc.root";   // Input file (MC events)

  // Reco file
  TString recFile = outDir + setupName + "_test.eds.root";  // Output file

  // Parameter file
  TString parFile = outDir + setupName + "_params.root";    // Parameter file

  // Output file
  TString outFile = outDir + setupName + "_test.qa.root";   // Output file
  
  TString srcDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = srcDir + "/parameters/";


  // -----   Load the geometry setup   -------------------------------------
  std::cout << std::endl;
  TString setupFile = srcDir + "/geometry/setup/setup_" + setupName + ".C";
  TString setupFunct = "setup_";
  setupFunct = setupFunct + setupName + "()";
  std::cout << "-I- " << "run_qa" << ": Loading macro " << setupFile << std::endl;
  gROOT->LoadMacro(setupFile);
  gROOT->ProcessLine(setupFunct);

  // In general, the following parts need not be touched
  // ========================================================================



  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  // -----  Analysis run   --------------------------------------------------
  FairRunAna *fRun= new FairRunAna();
  fRun->SetInputFile(simFile);
  fRun->AddFriend(recFile);
  fRun->SetOutputFile(outFile);
  Bool_t hasFairMonitor = Has_Fair_Monitor();
  if (hasFairMonitor) {
    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  }
  // ------------------------------------------------------------------------


  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open(parFile.Data());
  rtdb->setFirstInput(parIo1);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------


  // -----   STS track finder QA   ------------------------------------------
  // Is legacy; to be checked
  /*CbmStsFindTracksQa* stsTrackFindQa = new CbmStsFindTracksQa(4, 0.7,
							      iVerbose);
  fRun->AddTask(stsTrackFindQa);
  */
  // ------------------------------------------------------------------------
    
    // -------- Performance of PID algorithm ------------
    CbmTrdTracksPidQa*  trdTrackPidQa =
        new CbmTrdTracksPidQa("PidPerformance");
    fRun->AddTask(trdTrackPidQa);
    // -------------------------------------------------
    
 // ------  Rich ring finder QA ---------------------------------------------
   Int_t normType = 1; //0 - normalize by number of MC hits
                      //1 - normalize by number of found hits;
//  CbmRichRingQa* richQa   =  new CbmRichRingQa("Qa","qa", iVerbose,normType);
//  fRun->AddTask(richQa);

  // -----   Intialise and run   --------------------------------------------
  fRun->Init();
  cout << "Starting run" << endl;
  fRun->Run(0,nEvents);
  // ------------------------------------------------------------------------



  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is "    << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------

  if (hasFairMonitor) {
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo sysInfo;
    Float_t maxMemory=sysInfo.GetMaxMemory();
    cout << "<DartMeasurement name=\"MaxMemory\" type=\"numeric/double\">";
    cout << maxMemory;
    cout << "</DartMeasurement>" << endl;

    Float_t cpuUsage=ctime/rtime;
    cout << "<DartMeasurement name=\"CpuLoad\" type=\"numeric/double\">";
    cout << cpuUsage;
    cout << "</DartMeasurement>" << endl;

    FairMonitor* tempMon = FairMonitor::GetMonitor();
    tempMon->Print();
  }
  //  delete run;

  cout << " Test passed" << endl;
  cout << " All ok " << endl;
}

// --------------------------------------------------------------------------
//
// Macro for STS QA
//
// V. Friese   13/01/2006
//
// --------------------------------------------------------------------------

TString caveGeom="";
TString pipeGeom="";
TString magnetGeom="";
TString mvdGeom="";
TString stsGeom="";
TString richGeom="";
TString muchGeom="";
TString shieldGeom="";
TString trdGeom="";
TString tofGeom="";
TString ecalGeom="";
TString platformGeom="";
TString psdGeom="";
Double_t psdZpos=0.;
Double_t psdXpos=0.;

TString mvdTag="";
TString stsTag="";
TString trdTag="";
TString tofTag="";

TString stsDigi="";
TString trdDigi="";
TString tofDigi="";

TString mvdMatBudget="";
TString stsMatBudget="";

TString  fieldMap="";
Double_t fieldZ=0.;
Double_t fieldScale=0.;
Int_t    fieldSymType=0;

TString defaultInputFile="";

void run_qa(Int_t nEvents = 1, const char* setup = "sis100_electron")
{

  // ========================================================================
  //          Adjust this part according to your requirements

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 1;

  TString outDir  = "data/";

  // MC file
  TString simFile = outDir + setup + "_test.mc.root";   // Input file (MC events)

  // Reco file
  TString recFile = outDir + setup + "_test.eds.root";  // Output file

  // Parameter file
  TString parFile = outDir + setup + "_params.root";    // Parameter file

  // Output file
  TString outFile = outDir + setup + "_test.qa.root";   // Output file
  
  TString inDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = inDir + "/parameters/";

  TString setupFile = inDir + "/geometry/setup/" + setup + "_setup.C";
  TString setupFunct = setup;
  setupFunct += "_setup()";

  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);

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

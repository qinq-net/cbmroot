// --------------------------------------------------------------------------
//
// Macro for time-based digitisation
//
// Includes digitizer tasks for STS and TOF
//
// Run the macro run_mc.C before.
//
// Default settings here are: 
// Very large time slice, to have all events in one time slice
// Very small interaction rate (10 kHz), so the results should be
// comparable to the event-by-event simulation.
// 
// --------------------------------------------------------------------------


void run_digi_tb(Int_t nEvents = 2, const char* setupName = "sis100_electron")
{
  // ========================================================================
  //          Adjust this part according to your requirements

  TString outDir  = "data/";
  TString inFile  = outDir + setupName + "_test.mc.root";   // Input file (MC events)
  TString parFile = outDir + setupName + "_params.root";    // Parameter file
  TString outFile = outDir + setupName + "_test.raw.root";  // Output file
  
  // Specify interaction rate in 1/s
  Double_t eventRate = 1.e4;
  
  // Specify duration of time slices in output [ns]
  Double_t timeSliceSize = 100000000.;
  
  // Specify log level (INFO, DEBUG, DEBUG1, ...)
  TString logLevel = "INFO";

  TString inDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = inDir + "/parameters/";

  TString setupFile = inDir + "/geometry/setup/setup_" + setupName + ".C";
  TString setupFunct = TString("setup_") + setupName + "()";

  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);

  // In general, the following parts need not be touched
  // ========================================================================

  TList *parFileList = new TList();
  TString geoTag;
  CbmSetup* setup = CbmSetup::Instance();

  // - TOF digitisation parameters
  if ( setup->GetGeoTag(kTof, geoTag) ) {
  	TObjString* tofFile = new TObjString(paramDir + "tof/tof_" + geoTag + ".digi.par");
  	TObjString* tofBdfFile = new TObjString(paramDir + "tof/tof_" + geoTag + ".digibdf.par");
  	parFileList->Add(tofFile);
  	parFileList->Add(tofBdfFile);
  }

  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------


  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  Bool_t hasFairMonitor = Has_Fair_Monitor();
  if (hasFairMonitor) {
    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  }
  // ------------------------------------------------------------------------

  // -----   Reconstruction run   -------------------------------------------
  CbmRunAna *run = new CbmRunAna();
  run->SetAsync();                         // asynchroneous mode
  run->SetInputFile(inFile);
  run->SetOutputFile(outFile);
  run->SetEventMeanTime(1.e9 / eventRate);
  FairRootManager::Instance()->SetUseFairLinks(kTRUE);
  // ------------------------------------------------------------------------


  // ---- Set the log level 	
  FairLogger::GetLogger()->SetLogScreenLevel(logLevel.Data());


  // ----- STS digitiser
  CbmStsDigitize* stsDigi = new CbmStsDigitize();
  run->AddTask(stsDigi);
  
  CbmTofDigitizerBDF* tofDigi = new CbmTofDigitizerBDF("TOF Digitizer BDF");
  tofDigi->SetInputFileName( paramDir + "tof/test_bdf_input.root");
//  tofDigi->SetHistoFileName( outDir + "tof.digi.hst.root" ); // Uncomment to save control histograms
  tofDigi->SetMonitorHistograms(kFALSE);
  run->AddTask(tofDigi);
 
  // ----- DAQ
  FairTask* daq = new CbmDaqNew(timeSliceSize);
  run->AddTask(daq);
  
 
  


  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data(),"UPDATE");
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  // ------------------------------------------------------------------------


  // -----   Intialise and run   --------------------------------------------
  run->Init();

  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  rtdb->print();

  cout << "Starting run" << endl;
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
  RemoveGeoManager();
}

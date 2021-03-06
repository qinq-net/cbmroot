void build_events(Int_t nEvents = 1, TString fileName="")
{

  // ========================================================================
  //          Adjust this part according to your requirements

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 1;

  // MC file

  TString srcDir = gSystem->Getenv("VMCWORKDIR");

  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  // -----  Analysis run   --------------------------------------------------
  FairRunOnline *fRun= new FairRunOnline();
  fRun->ActivateHttpServer( 100, 8080 ); // refresh each 100 events

  FairFileSource* inputSource = new FairFileSource(fileName);
  fRun->SetSource(inputSource);

  FairRootFileSink* outputSink = new FairRootFileSink("data/events.root");
  fRun->SetSink(outputSink);

  // Define output file for FairMonitor histograms
//  TString monitorFile{outFile};
//  monitorFile.ReplaceAll("qa","qa.monitor");
  FairMonitor::GetMonitor()->EnableMonitor(kFALSE);
  // ------------------------------------------------------------------------

  CbmMcbm2018EventBuilder* eventBuilder = new CbmMcbm2018EventBuilder();
  eventBuilder->SetEventBuilderAlgo(EventBuilderAlgo::MaximumTimeGap);
  eventBuilder->SetMaximumTimeGap(100.);
//  eventBuilder->SetEventBuilderAlgo(EventBuilderAlgo::FixedTimeWindow);
//  eventBuilder->SetFixedTimeWindow(60.);
  eventBuilder->SetTriggerMinNumberT0(1);
  eventBuilder->SetTriggerMinNumberSts(1);
  eventBuilder->SetTriggerMinNumberMuch(1);
  eventBuilder->SetTriggerMinNumberTof(1);
  fRun->AddTask(eventBuilder);

  // -----  Parameter database   --------------------------------------------
//  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
//  FairParRootFileIo* parIo1 = new FairParRootFileIo();
//  parIo1->open(parFile.Data(),"UPDATE");
//  rtdb->setFirstInput(parIo1);
  // ------------------------------------------------------------------------


  // -----   Intialise and run   --------------------------------------------
  fRun->Init();

//  rtdb->setOutput(parIo1);
//  rtdb->saveOutput();
//  rtdb->print();

  cout << "Starting run" << endl;
  fRun->Run(0,nEvents);
  // ------------------------------------------------------------------------



  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------

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

//  RemoveGeoManager();
  cout << " Test passed" << endl;
  cout << " All ok " << endl;
}

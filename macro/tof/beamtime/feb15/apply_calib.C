// -----------------------------------------------------------------------------
// ----- apply_calib.C                                                     -----
// -----                                                                   -----
// ----- created by C. Simon on 2016-04-26                                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

void apply_calib(Int_t nEvents = -1, char *cFileId="CernSps05Mar0041")
{
  TStopwatch timer;
  timer.Start();

//  gLogger->SetLogScreenLevel("FATAL");
  gLogger->SetLogScreenLevel("ERROR");
//  gLogger->SetLogScreenLevel("WARNING");
//  gLogger->SetLogScreenLevel("INFO");
//  gLogger->SetLogScreenLevel("DEBUG");
//  gLogger->SetLogScreenLevel("DEBUG1");
//  gLogger->SetLogScreenLevel("DEBUG2");
//  gLogger->SetLogScreenLevel("DEBUG3");

//  gLogger->SetLogVerbosityLevel("LOW");
  gLogger->SetLogVerbosityLevel("MEDIUM");
//  gLogger->SetLogVerbosityLevel("HIGH");

  TString workDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = workDir + "/macro/tof/beamtime/feb15";

  TObjString unpParFile = paramDir + "/parUnpack_basic.txt";
  TObjString calParFile = paramDir + "/parCalib_basic.txt";
  TObjString mapParFile = paramDir + "/parMapCernFeb2015.txt";

  TList *parFileList = new TList();
  parFileList->Add(&unpParFile);
  parFileList->Add(&calParFile);
  parFileList->Add(&mapParFile);

  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  
  FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
  parIo1->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);

  TString cOutfileId = Form("%s",cFileId);

  TString outFile = paramDir + "/unpack_" + cOutfileId + ".out.root";

  FairLmdSource* source = new FairLmdSource();
  source->AddPath("/mnt/nas-herrmann2/cern-feb15/",Form("%s*.lmd",cFileId));
//  source->AddFile("/mnt/nas-herrmann2/cern-feb15/CernSps05Mar0041_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_0002.lmd"); // 4 TDCs offset by 200 s AND reference epoch overflow!!!
//  source->AddFile("/mnt/nas-herrmann2/cern-feb15/Mockup04Mar2002_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_0000.lmd");


  TTrbUnpackTof* tofTrbDataUnpacker = new TTrbUnpackTof(10,1,31,0,0);
  tofTrbDataUnpacker->SetInspection(kFALSE);
  tofTrbDataUnpacker->SetSaveRawData(kFALSE);
  source->AddUnpacker( tofTrbDataUnpacker );

  TMbsUnpTofMonitor* tofUnpMonitor = new TMbsUnpTofMonitor("Tof Unp Moni");

  TTofTrbTdcCalib* tofCalibTrb = new TTofTrbTdcCalib();
  tofCalibTrb->CreateCalibration(kFALSE);
  tofCalibTrb->SaveCalibData(kFALSE);
  tofCalibTrb->SetFineTimeMethod(0);
  tofCalibTrb->SetToTMethod(0);
  tofCalibTrb->SetMinEntriesBinByBin(100000);
  tofCalibTrb->SetTimeContinuum(kTRUE);
  tofCalibTrb->SetTrailingOffsetCycles(2);
  tofCalibTrb->SetLowerLinearFineLimit(31);  // feb15
  tofCalibTrb->SetUpperLinearFineLimit(480); // feb15
//  tofCalibTrb->SetLowerLinearFineLimit(16);  // nov15
//  tofCalibTrb->SetUpperLinearFineLimit(490); // nov15
  tofCalibTrb->SetReferenceBoard(0);
  tofCalibTrb->SetToTSingleLeading(-100.);

  TMbsMappingTof* tofMapping = new TMbsMappingTof("Tof Mapping");
  tofMapping->SetSaveDigis(kTRUE);
  tofMapping->SetFillHistogramms(kFALSE);

  CbmTofOnlineDisplay* display = new CbmTofOnlineDisplay();
  display->MonitorTdcOccupancy(kFALSE);
  display->MonitorCts(kFALSE);
  display->MonitorSebStatus(kFALSE);
  display->MonitorTdcStatus(kFALSE);
  display->MonitorFSMockup(kFALSE);
  display->MonitorInspection(kFALSE);
  display->MonitorCalibration(kFALSE);
//  display->SetUpdateInterval(10000);
  display->SetUpdateInterval(100000000); // disables live updates (speed-up)


  run->AddTask(tofUnpMonitor);
  run->AddTask(tofCalibTrb);
  run->AddTask(tofMapping);
  run->AddTask(display);

  run->SetSource(source);
  run->SetOutputFile(outFile);
  run->SetAutoFinish(kFALSE);

  run->Init();

  cout << "Starting run" << endl;
  run->Run(nEvents);

  cout << "Finishing run" << endl;
  run->Finish();

  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Run finished successfully." << endl;
  cout << "Output file is " << outFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
}


// -----------------------------------------------------------------------------
// ----- build_events.C                                                    -----
// -----                                                                   -----
// ----- created by C. Simon on 2018-05-24                                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

void build_events(const TString& tInputFilePath, const TString& tInputFileName, Int_t iNInputFiles,
                  Double_t dEventWindow, Int_t iTriggerSet, Int_t iMultiplicity)
{
  // ---------------------------------------------------------------------------

  TString tLogLevel = "ERROR";
  TString tLogVerbosity = "LOW";
  gErrorIgnoreLevel = kWarning;
  gDebug = 0;

  TTree::SetMaxTreeSize(1000000000000LL); // [B] here: 1 TB

  // ---------------------------------------------------------------------------

  TStopwatch tTimer;
  tTimer.Start();

  Bool_t bHasFairMonitor = Has_Fair_Monitor();
  if(bHasFairMonitor)
  {
    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  }

  // ---------------------------------------------------------------------------

  FairLogger::GetLogger()->SetLogScreenLevel(tLogLevel.Data());
  FairLogger::GetLogger()->SetLogVerbosityLevel(tLogVerbosity.Data());

  // ---------------------------------------------------------------------------

  FairFileSource* tFileSource = new FairFileSource(tInputFilePath + "/001/data/" + tInputFileName);

  for(Int_t iInputFile = 2; iInputFile <= iNInputFiles; iInputFile++)
  {
    tFileSource->AddFile(tInputFilePath + TString::Format("/%03d/data/", iInputFile) + tInputFileName);
  }

  // ---------------------------------------------------------------------------

  CbmRunAna* tRun = new CbmRunAna();
  tRun->SetAsync(kTRUE);
  tRun->SetContainerStatic(kTRUE);
  tRun->SetSource(tFileSource);
  tRun->SetOutputFile("data/unpack.out.root");

  // ---------------------------------------------------------------------------

  CbmTofBuildDigiEvents* tEventBuilder = new CbmTofBuildDigiEvents();
  tEventBuilder->SetEventWindow(dEventWindow);

  switch(iTriggerSet)
  {
    case 0:
      tEventBuilder->SetTriggerCounter(5, 1, 0, 1);
      tEventBuilder->SetTriggerCounter(5, 2, 0, 1);
      break;

    case 1:
      tEventBuilder->SetTriggerCounter(5, 1, 0, 1);
      tEventBuilder->SetTriggerCounter(5, 2, 0, 1);
      tEventBuilder->SetTriggerCounter(4, 0, 0, 2);
      tEventBuilder->SetTriggerCounter(9, 0, 1, 2);
      tEventBuilder->SetTriggerCounter(9, 2, 0, 2);
      tEventBuilder->SetTriggerCounter(9, 2, 1, 2);
      break;

    default:
      std::cout << "-E- build_events: trigger set not implemented!" << std::endl;
      return;
  }

  tEventBuilder->SetTriggerMultiplicity(iMultiplicity);
  tEventBuilder->SetPreserveMCBacklinks(kFALSE);

  // ---------------------------------------------------------------------------

  tRun->AddTask(tEventBuilder);

  // ---------------------------------------------------------------------------

  std::cout << "-I- build_events: initializing run..." << std::endl;
  tRun->Init();

  std::cout << "-I- build_events: starting run..." << std::endl;
  tRun->Run(0, 0);

  // ---------------------------------------------------------------------------

  tTimer.Stop();
  Double_t dRealTime = tTimer.RealTime();
  Double_t dCPUTime = tTimer.CpuTime();

  std::cout << std::endl << std::endl;
  std::cout << "Macro finished successfully." << std::endl;
  std::cout << "Real time " << dRealTime << " s, CPU time " << dCPUTime << " s" << std::endl;
  std::cout << std::endl;

  // ---------------------------------------------------------------------------

  if(bHasFairMonitor)
  {
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo tSysInfo;
    Float_t fMaxMemory = tSysInfo.GetMaxMemory();
    std::cout << "<DartMeasurement name=\"MaxMemory\" type=\"numeric/double\">";
    std::cout << fMaxMemory;
    std::cout << "</DartMeasurement>" << endl;

    Float_t fCPUUsage = dCPUTime/dRealTime;
    std::cout << "<DartMeasurement name=\"CpuLoad\" type=\"numeric/double\">";
    std::cout << fCPUUsage;
    std::cout << "</DartMeasurement>" << endl;

    FairMonitor::GetMonitor()->Print();
  }

  // ---------------------------------------------------------------------------

  FairRootManager::Instance()->CloseOutFile();

  // ---------------------------------------------------------------------------
}

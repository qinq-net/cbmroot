// --------------------------------------------------------------------------
//
// Macro for reconstruction of simulated mCBM events with standard settings
//
// Event-by-event reconstruction; requires appropriate digitization before
// (see mcbm_digi.C)
//
// Local reconstruction in MVD, STS, MUCH, TRD and TOF
// Binned tracker for track reconstruction
//
// V. Friese   11.06.2018
//
// --------------------------------------------------------------------------


void mcbm_reco_event(
    Int_t nEvents = 3,
    TString dataset ="test",
    const char* setupName = "sis18_mcbm_25deg_long"
)
{
  // ========================================================================
  //          Adjust this part according to your requirements

  // --- Logger settings ----------------------------------------------------
  TString logLevel     = "INFO";
  TString logVerbosity = "LOW";
  // ------------------------------------------------------------------------


  // -----   Environment   --------------------------------------------------
  TString myName = "mcbm_reco";  // this macro's name for screen output
  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
  // ------------------------------------------------------------------------


  // -----   File names   ---------------------------------------------------
  TString rawFile = dataset + ".event.raw.root";
  TString parFile = dataset + ".par.root";
  TString recFile = dataset + ".rec.root";
  // ------------------------------------------------------------------------


  // -----   Remove old CTest runtime dependency file  ----------------------
  TString dataDir = gSystem->DirName(dataset);
  TString dataName = gSystem->BaseName(dataset);
  TString depFile = Remove_CTest_Dependency_File(dataDir, "mcbm_reco_event",
                                                 dataName);
  // ------------------------------------------------------------------------


  // -----   Load the geometry setup   -------------------------------------
  std::cout << std::endl;
  TString setupFile = srcDir + "/geometry/setup/setup_" + setupName + ".C";
  TString setupFunct = "setup_";
  setupFunct = setupFunct + setupName + "()";
  std::cout << "-I- " << myName << ": Loading macro " << setupFile << std::endl;
  gROOT->LoadMacro(setupFile);
  gROOT->ProcessLine(setupFunct);
  CbmSetup* setup = CbmSetup::Instance();
  // ------------------------------------------------------------------------


  // -----   Parameter files as input to the runtime database   -------------
  std::cout << std::endl;
  std::cout << "-I- " << myName << ": Defining parameter files " << std::endl;
  TList *parFileList = new TList();
  TString geoTag;

  // - TRD digitisation parameters
  if ( setup->GetGeoTag(kTrd, geoTag) ) {
  	TObjString* trdFile = new TObjString(srcDir + "/parameters/trd/trd_" + geoTag + ".digi.par");
  	parFileList->Add(trdFile);
    std::cout << "-I- " << myName << ": Using parameter file "
    		      << trdFile->GetString() << std::endl;
  }

  // - TOF digitisation parameters
  if ( setup->GetGeoTag(kTof, geoTag) ) {
  	TObjString* tofFile = new TObjString(srcDir + "/parameters/tof/tof_" + geoTag + ".digi.par");
  	parFileList->Add(tofFile);
    std::cout << "-I- " << myName << ": Using parameter file "
    		      << tofFile->GetString() << std::endl;
  	TObjString* tofBdfFile = new TObjString(srcDir + "/parameters/tof/tof_" + geoTag + ".digibdf.par");
  	parFileList->Add(tofBdfFile);
    std::cout << "-I- " << myName << ": Using parameter file "
    		      << tofBdfFile->GetString() << std::endl;
  }
  // ------------------------------------------------------------------------

  // In general, the following parts need not be touched
  // ========================================================================


  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------


  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------


  // -----   Input file   ---------------------------------------------------
  std::cout << std::endl;
  std::cout << "-I- " << myName << ": Using input file " << rawFile << std::endl;
  // ------------------------------------------------------------------------


  // -----   FairRunAna   ---------------------------------------------------
  FairRunAna *run = new FairRunAna();
  run->SetInputFile(rawFile);
  run->SetOutputFile(recFile);
  run->SetGenerateRunInfo(kTRUE);
  Bool_t hasFairMonitor = Has_Fair_Monitor();
  if (hasFairMonitor) FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  // ------------------------------------------------------------------------


  // -----   Logger settings   ----------------------------------------------
  FairLogger::GetLogger()->SetLogScreenLevel(logLevel.Data());
  FairLogger::GetLogger()->SetLogVerbosityLevel(logVerbosity.Data());
  // ------------------------------------------------------------------------


  // -----   Local reconstruction in MVD   ----------------------------------
  if ( setup->IsActive(kMvd) ) {

    CbmMvdClusterfinder* mvdCluster =
            new CbmMvdClusterfinder("MVD Cluster Finder", 0, 0);
    run->AddTask(mvdCluster);
    std::cout << "-I- " << myName << ": Added task "
        << mvdCluster->GetName() << std::endl;

    CbmMvdHitfinder* mvdHit = new CbmMvdHitfinder("MVD Hit Finder", 0, 0);
    mvdHit->UseClusterfinder(kTRUE);
    run->AddTask(mvdHit);
    std::cout << "-I- " << myName << ": Added task "
        << mvdHit->GetName() << std::endl;

  }
  // ------------------------------------------------------------------------


  // -----   Local reconstruction in STS   ----------------------------------
  if ( setup->IsActive(kSts) ) {

    CbmStsFindClusters* stsCluster = new CbmStsFindClusters();
    stsCluster->UseEventMode();
    run->AddTask(stsCluster);
    std::cout << "-I- " << myName << ": Added task "
        << stsCluster->GetName() << std::endl;

    FairTask* stsHit = new CbmStsFindHitsEvents();
    run->AddTask(stsHit);
    std::cout << "-I- " << myName << ": Added task "
        << stsHit->GetName() << std::endl;

  }
  // ------------------------------------------------------------------------


  // -----   Local reconstruction in MUCH   ---------------------------------
  if ( setup->IsActive(kMuch) ) {

        // --- Parameter file name
        TString geoTag;
        setup->GetGeoTag(kMuch, geoTag);
        Int_t muchFlag=0;
        if (geoTag.Contains("mcbm")) muchFlag=1;

        TString parFile = gSystem->Getenv("VMCWORKDIR");
        parFile = parFile + "/parameters/much/much_" + geoTag(0,4)
                    + "_digi_sector.root";
        std::cout << "-I- " << myName << ": Using parameter file "
            << parFile << std::endl;

        // --- Hit finder for GEMs
        FairTask* muchHitGem = new CbmMuchFindHitsGem(parFile.Data(),muchFlag);
        run->AddTask(muchHitGem);
        std::cout << "-I- " << myName << ": Added task "
            << muchHitGem->GetName() << FairLogger::endl;

  }
  // ------------------------------------------------------------------------


  // -----   Local reconstruction in TRD   ----------------------------------
  if ( setup->IsActive(kTrd) ) {

    Double_t triggerThreshold = 0.5e-6;   // SIS100
    Bool_t   triangularPads = false;      // Bucharest triangular pad-plane layout
    CbmTrdClusterFinderFast* trdCluster = new CbmTrdClusterFinderFast();
    trdCluster->SetNeighbourTrigger(true);
    trdCluster->SetTriggerThreshold(triggerThreshold);
    trdCluster->SetNeighbourRowTrigger(false);
    trdCluster->SetPrimaryClusterRowMerger(true);
    trdCluster->SetTriangularPads(triangularPads);
    run->AddTask(trdCluster);
    std::cout << "-I- " << myName << ": Added task "
        << trdCluster->GetName() << std::endl;

    CbmTrdHitProducerCluster* trdHit = new CbmTrdHitProducerCluster();
    trdHit->SetTriangularPads(triangularPads);
    run->AddTask(trdHit);
    std::cout << "-I- " << myName << ": Added task "
        << trdHit->GetName() << std::endl;

  }
  // ------------------------------------------------------------------------


  // -----   Local reconstruction in TOF   ----------------------------------
  if ( setup->IsActive(kTof) ) {
    CbmTofSimpClusterizer* tofCluster
          = new CbmTofSimpClusterizer("TOF Simple Clusterizer", 0);
    tofCluster->SetOutputBranchPersistent("TofHit",          kTRUE);
    tofCluster->SetOutputBranchPersistent("TofDigiMatch",    kTRUE);
    run->AddTask(tofCluster);
    std::cout << "-I- " << myName << ": Added task "
        << tofCluster->GetName() << std::endl;
  }
  // -------------------------------------------------------------------------


  // -----   Track reconstruction   ------------------------------------------
  Double_t beamWidthX = 0.1;
  Double_t beamWidthY = 0.1;
  CbmBinnedTrackerTask* trackerTask = new CbmBinnedTrackerTask(kTRUE,
                                                               beamWidthX,
                                                               beamWidthY);
  run->AddTask(trackerTask);
   // ------------------------------------------------------------------------


  // -----  Parameter database   --------------------------------------------
  std::cout << std::endl << std::endl;
  std::cout << "-I- " << myName << ": Set runtime DB" << std::endl;
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data(), "UPDATE");
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  // ------------------------------------------------------------------------


  // -----   Run initialisation   -------------------------------------------
  std::cout << std::endl;
  std::cout << "-I- " << myName << ": Initialise run" << std::endl;
  run->Init();
  // ------------------------------------------------------------------------


  // -----   Database update   ----------------------------------------------
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  rtdb->print();
  // ------------------------------------------------------------------------


  // -----   Start run   ----------------------------------------------------
  std::cout << std::endl << std::endl;
  std::cout << "-I- " << myName << ": Starting run" << std::endl;
  run->Run(0, nEvents);
  // ------------------------------------------------------------------------


  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  std::cout << std::endl << std::endl;
  std::cout << "Macro finished successfully." << std::endl;
  std::cout << "Output file is " << recFile << std::endl;
  std::cout << "Parameter file is " << parFile << std::endl;
  std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s"
  		      << std::endl;
  std::cout << std::endl;
  std::cout << " Test passed" << std::endl;
  std::cout << " All ok " << std::endl;
  // ------------------------------------------------------------------------


  // -----   Resource monitoring   ------------------------------------------
  if ( Has_Fair_Monitor() ) {      // FairRoot Version >= 15.11
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo sysInfo;
    Float_t maxMemory=sysInfo.GetMaxMemory();
    std::cout << "<DartMeasurement name=\"MaxMemory\" type=\"numeric/double\">";
    std::cout << maxMemory;
    std::cout << "</DartMeasurement>" << std::endl;

    Float_t cpuUsage=ctime/rtime;
    std::cout << "<DartMeasurement name=\"CpuLoad\" type=\"numeric/double\">";
    std::cout << cpuUsage;
    std::cout << "</DartMeasurement>" << std::endl;

    FairMonitor* tempMon = FairMonitor::GetMonitor();
    tempMon->Print();
  }

  // Function needed for CTest runtime dependency
  Generate_CTest_Dependency_File(depFile);
  RemoveGeoManager();
}

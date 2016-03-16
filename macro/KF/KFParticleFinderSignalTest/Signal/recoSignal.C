//---------------------------------------------------------------------------------
// @author  M. Zyzak
// @version 1.0
// @since   15.08.14
// 
// macro to reconstruct signal events for KFParticleFinder
//_________________________________________________________________________________

void recoSignal(Int_t nEvents = 10000) {

  //macro to check the setup put into the simulation
  gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/determine_setup.C");
  
  // ========================================================================
  //          Adjust this part according to your requirements

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 0;

  TString inFile = "signal.mc.root";
  TString parFile = "signal.params.root";
  TString outFile = "signal.reco.root";

  //  Digitisation files.
  // Add TObjectString containing the different file names to
  // a TList which is passed as input to the FairParAsciiFileIo.
  // The FairParAsciiFileIo will take care to create on the fly 
  // a concatenated input parameter file which is then used during
  // the reconstruction.
  TList *parFileList = new TList();

  TString workDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = workDir + "/parameters";

  TObjString stsDigiFile = paramDir + "/sts/sts_v13d_std.digi.par";
  parFileList->Add(&stsDigiFile);

  TObjString trdDigiFile =  paramDir + "/trd/trd_v13g.digi.par";
  parFileList->Add(&trdDigiFile);

  TObjString tofDigiFile =  paramDir + "/tof/tof_v13b.digi.par";
  parFileList->Add(&tofDigiFile);


  // In general, the following parts need not be touched
  // ========================================================================


  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------


  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *run = new FairRunAna();
  run->SetInputFile(inFile);
  run->SetOutputFile(outFile);
  // ------------------------------------------------------------------------

  // ----- MC Data Manager   ------------------------------------------------
  CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
  mcManager->AddFile(inFile);
  run->AddTask(mcManager);
  // ------------------------------------------------------------------------

  // =========================================================================
  // ===             Detector Response Simulation (Digitiser)              ===
  // ===                          (where available)                        ===
  // =========================================================================


//   if(IsMvd(parFile))
  {
    // -----   MVD Digitiser   -------------------------------------------------
    CbmMvdDigitizer* mvdDigitise = new CbmMvdDigitizer("MVD Digitiser", 0, iVerbose);
    run->AddTask(mvdDigitise);
    // -------------------------------------------------------------------------
// 
//   // -----   MVD Clusterfinder   ---------------------------------------------
//   CbmMvdClusterfinder* mvdCluster = new CbmMvdClusterfinder("MVD Clusterfinder", 0, iVerbose);
//   run->AddTask(mvdCluster);
//   // -------------------------------------------------------------------------
  }

  // -----   STS digitizer   -------------------------------------------------
  // -----   The parameters of the STS digitizer are set such as to match
  // -----   those in the old digitizer. Change them only if you know what you
  // -----   are doing.
  Double_t dynRange       =   40960.;  // Dynamic range [e]
  Double_t threshold      =    4000.;  // Digitisation threshold [e]
  Int_t nAdc              =    4096;   // Number of ADC channels (12 bit)
  Double_t timeResolution =       5.;  // time resolution [ns]
  Double_t deadTime       = 9999999.;  // infinite dead time (integrate entire event)
  Double_t noise          =       0.;  // ENC [e]
  Int_t digiModel         =       1;   // Model: 1 = uniform charge distribution along track

  CbmStsDigitize* stsDigi = new CbmStsDigitize(digiModel);
  stsDigi->SetParameters(dynRange, threshold, nAdc, timeResolution,
                       deadTime, noise);
  run->AddTask(stsDigi);


  // =========================================================================
  // ===                     MVD local reconstruction                      ===
  // =========================================================================

  // -----   MVD Hit Finder   ------------------------------------------------
//   if(IsMvd(parFile))
  {
    CbmMvdHitfinder* mvdHitfinder = new CbmMvdHitfinder("MVD Hit Finder", 0, iVerbose);
    run->AddTask(mvdHitfinder);
  }
  // -------------------------------------------------------------------------

  // ===                 End of MVD local reconstruction                   ===
  // =========================================================================




  // =========================================================================
  // ===                      STS local reconstruction                     ===
  // =========================================================================


  // -----   STS Cluster Finder   --------------------------------------------
  FairTask* stsClusterFinder = new CbmStsFindClusters();
  run->AddTask(stsClusterFinder);
  // -------------------------------------------------------------------------


  // -----   STS hit finder   ------------------------------------------------
  FairTask* stsFindHits = new CbmStsFindHits();
  run->AddTask(stsFindHits);
  // -------------------------------------------------------------------------


//   // -----  STS hit matching   -----------------------------------------------
//   FairTask* stsMatchHits = new CbmStsMatchHits();
//   run->AddTask(stsMatchHits);
//   // -------------------------------------------------------------------------

  // ---   STS track matching   ----------------------------------------------
   CbmMatchRecoToMC* matchTask = new CbmMatchRecoToMC();
   run->AddTask(matchTask);
  // -------------------------------------------------------------------------
   
  // ---  STS track finding   ------------------------------------------------
  CbmKF* kalman = new CbmKF();
  run->AddTask(kalman);
  CbmL1* l1 = new CbmL1("CbmL1",1, 3);
  TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
  const TString stsMatBudgetFileName = parDir + "/sts/sts_matbudget_v13d.root";
  const TString mvdMatBudgetFileName = parDir + "/mvd/mvd_matbudget_v14b.root";
  l1->SetStsMaterialBudgetFileName(stsMatBudgetFileName.Data());
  l1->SetMvdMaterialBudgetFileName(mvdMatBudgetFileName.Data());
  run->AddTask(l1);
  CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
  FairTask* stsFindTracks = new CbmStsFindTracks(iVerbose, stsTrackFinder);
  run->AddTask(stsFindTracks);
  // -------------------------------------------------------------------------

  if (IsTrd(parFile)) {
    // ----- TRD reconstruction-----------------------------------------
    CbmTrdRadiator *radiator = new CbmTrdRadiator(kTRUE , "H++");
    if (trdHitProducerType == "smearing") {
      CbmTrdHitProducerSmearing* trdHitProd = new CbmTrdHitProducerSmearing(radiator);
      trdHitProd->SetUseDigiPar(false);
      run->AddTask(trdHitProd);
    } else if (trdHitProducerType == "digi") {
      CbmTrdDigitizer* trdDigitizer = new CbmTrdDigitizer(radiator);
      run->AddTask(trdDigitizer);

      CbmTrdHitProducerDigi* trdHitProd = new CbmTrdHitProducerDigi();
      run->AddTask(trdHitProd);
    } else if (trdHitProducerType == "clustering") {
      // ----- TRD clustering -----
        CbmTrdDigitizerPRF* trdDigiPrf = new CbmTrdDigitizerPRF(radiator);
        run->AddTask(trdDigiPrf);

        CbmTrdClusterFinderFast* trdCluster = new CbmTrdClusterFinderFast();
        run->AddTask(trdCluster);

        CbmTrdHitProducerCluster* trdHit = new CbmTrdHitProducerCluster();
        run->AddTask(trdHit);
      // ----- End TRD Clustering -----
    }
    // ------------------------------------------------------------------------
  }

  if (IsTof(parFile)) {
    // ------ TOF hits --------------------------------------------------------
      CbmTofHitProducerNew* tofHitProd = new CbmTofHitProducerNew("TOF HitProducerNew",iVerbose);
      tofHitProd->SetInitFromAscii(kFALSE);
      run->AddTask(tofHitProd);
    // ------------------------------------------------------------------------
  }

  if (IsTof(parFile) || IsRich(parFile) || IsTrd(parFile)) {
    // ------ Global track reconstruction -------------------------------------
    CbmLitFindGlobalTracks* finder = new CbmLitFindGlobalTracks();
    //CbmLitFindGlobalTracksParallel* finder = new CbmLitFindGlobalTracksParallel();
    // Tracking method to be used
    // "branch" - branching tracking
    // "nn" - nearest neighbor tracking
    // "nn_parallel" - nearest neighbor parallel tracking
    TString globalTrackingType = "nn";
    finder->SetTrackingType(std::string(globalTrackingType));

    // Hit-to-track merger method to be used
    // "nearest_hit" - assigns nearest hit to track
    // "all_hits" - assigns all hits in the searching area to track
    finder->SetMergerType("nearest_hit");

    run->AddTask(finder);
  }
  
  if (IsRich(parFile)) {
    CbmRichHitProducer* richHitProd  = new CbmRichHitProducer();
    run->AddTask(richHitProd);

    CbmRichReconstruction* richReco = new CbmRichReconstruction();
    run->AddTask(richReco);

    CbmRichMatchRings* matchRings = new CbmRichMatchRings();
    run->AddTask(matchRings);
  }
  
  // ---   STS track matching   ----------------------------------------------
  CbmMatchRecoToMC* matchTask2 = new CbmMatchRecoToMC();
  if(IsMvd(parFile))
  matchTask2->SetIncludeMvdHitsInStsTrack(1);
  run->AddTask(matchTask2);
  // -------------------------------------------------------------------------

  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data());
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------


  // -----   Intialise and run   --------------------------------------------
  run->Init();
  //add ions to the TDatabasePDG
  KFPartEfficiencies eff;
  for(int jParticle=125; jParticle<133; jParticle++)
  {
    TDatabasePDG* pdgDB = TDatabasePDG::Instance();

    if(!pdgDB->GetParticle(eff.partPDG[jParticle])){
        pdgDB->AddParticle(eff.partTitle[jParticle].data(),eff.partTitle[jParticle].data(), eff.partMass[jParticle], kTRUE,
                           0, eff.partCharge[jParticle]*3,"Ion",eff.partPDG[jParticle]);
    }
  }
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

//  delete run;

  cout << " Test passed" << endl;
	cout << " All ok " << endl;
}

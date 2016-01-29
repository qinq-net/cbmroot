
void run_reco_geotest(Int_t nEvents = 10000)
{
   TTree::SetMaxTreeSize(90000000000);
   TString script = TString(gSystem->Getenv("SCRIPT"));
   gRandom->SetSeed(10);

   TString outDir = "/Users/slebedev/Development/cbm/data/simulations/rich/geotest/";
   TString mcFile = outDir + "mc.0005.root";
   TString parFile = outDir + "param.0005.root";
   TString recoFile = outDir + "reco.0005.root";
   std::string resultDir = "results_geotest/";

   if (script == "yes") {
      mcFile = TString(gSystem->Getenv("MC_FILE"));
      recoFile = TString(gSystem->Getenv("RECO_FILE"));
      parFile = TString(gSystem->Getenv("PAR_FILE"));
      resultDir = TString(gSystem->Getenv("RESULT_DIR"));
   }
    
    remove(recoFile.Data());

   gDebug = 0;
   TStopwatch timer;
   timer.Start();

   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();

   FairRunAna *run= new FairRunAna();
   run->SetInputFile(mcFile);
   run->SetOutputFile(recoFile);

   CbmKF *kalman = new CbmKF();
   run->AddTask(kalman);
    
    // ----- MC Data Manager   ------------------------------------------------
    CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
    mcManager->AddFile(mcFile);
    run->AddTask(mcManager);

    CbmRichDigitizer* richDigitizer = new CbmRichDigitizer();
    run->AddTask(richDigitizer);
    
	CbmRichHitProducer* richHitProd  = new CbmRichHitProducer();
	run->AddTask(richHitProd);

	CbmRichReconstruction* richReco = new CbmRichReconstruction();
	richReco->SetRunExtrapolation(false);
	richReco->SetRunProjection(false);
	richReco->SetRunTrackAssign(false);
	richReco->SetFinderName("ideal");
	run->AddTask(richReco);

    CbmMatchRecoToMC* matchRecoToMc = new CbmMatchRecoToMC();
	run->AddTask(matchRecoToMc);

   CbmRichGeoTest* geoTest = new CbmRichGeoTest();
   geoTest->SetOutputDir(resultDir);
   run->AddTask(geoTest);

    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    FairParRootFileIo* parIo1 = new FairParRootFileIo();
    FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
    parIo1->open(parFile.Data());
  //  parIo2->open(parFileList, "in");
    rtdb->setFirstInput(parIo1);
    rtdb->setSecondInput(parIo2);
    rtdb->setOutput(parIo1);
    rtdb->saveOutput();
    
    run->Init();
    cout << "Starting run" << endl;
    run->Run(0,nEvents);

   // -----   Finish   -------------------------------------------------------
   timer.Stop();
   Double_t rtime = timer.RealTime();
   Double_t ctime = timer.CpuTime();
   cout << endl << endl;
   cout << "Macro finished successfully." << endl;
   cout << "Output file is "    << recoFile << endl;
   cout << "Parameter file is " << parFile << endl;
   cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
   cout << endl;

   cout << " Test passed" << endl;
   cout << " All ok " << endl;
}

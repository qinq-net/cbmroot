
void run_reco_urqmdtest(Int_t nEvents = 10)
{
    TTree::SetMaxTreeSize(90000000000);

	TString script = TString(gSystem->Getenv("SCRIPT"));
	TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");

	TString inFile = "/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root";
	TString outDir = "/Users/slebedev/Development/cbm/data/simulations/rich/urqmdtest/";
	TString parFile =  outDir + "25gev.centr.param.root";
	TString mcFile = outDir + "25gev.centr.mc.root";
	TString recoFile = outDir + "25gev.centr.reco.root";

	TString stsMatBudgetFileName = parDir + "/sts/sts_matbudget_v16x.root"; // Material budget file for L1 STS tracking
	std::string resultDir = "results_urqmd_25gev_centr_al_1/";

	if (script == "yes") {
		mcFile = TString(gSystem->Getenv("MC_FILE"));
		parFile = TString(gSystem->Getenv("PAR_FILE"));
		recoFile = TString(gSystem->Getenv("RECO_FILE"));
		stsMatBudgetFileName =  parDir + TString(gSystem->Getenv("STS_MATERIAL_BUDGET_FILE_NAME"));
		resultDir = std::string(gSystem->Getenv("RESULT_DIR"));
	}
    remove(recoFile.Data());

	gRandom->SetSeed(10);

   TList *parFileList = new TList();
   //parFileList->Add(&stsDigiFile);
   //parFileList->Add(&trdDigiFile);

   gDebug = 0;
   TStopwatch timer;
   timer.Start();

   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();

   FairRunAna *run= new FairRunAna();
   run->SetInputFile(mcFile);
   run->SetOutputFile(recoFile);
    
    CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
    mcManager->AddFile(mcFile);
    run->AddTask(mcManager);

    // =========================================================================
    // ===                      STS local reconstruction                     ===
    // =========================================================================
    Double_t dynRange       =   40960.;  // Dynamic range [e]
    Double_t threshold      =    4000.;  // Digitisation threshold [e]
    Int_t nAdc              =    4096;   // Number of ADC channels (12 bit)
    Double_t timeResolution =       5.;  // time resolution [ns]
    Double_t deadTime       = 9999999.;  // infinite dead time (integrate entire event)
    Double_t noise          =       0.;  // ENC [e]
    Int_t digiModel         =       1;   // User sensor type DSSD
    
    // The following settings correspond to a validated implementation.
    // Changing them is on your own risk.
    Int_t  eLossModel       = 1;         // Energy loss model: uniform
    Bool_t useLorentzShift  = kFALSE;    // Deactivate Lorentz shift
    Bool_t useDiffusion     = kFALSE;    // Deactivate diffusion
    Bool_t useCrossTalk     = kFALSE;    // Deactivate cross talk
    
    CbmStsDigitize* stsDigi = new CbmStsDigitize(digiModel);
    stsDigi->SetProcesses(eLossModel, useLorentzShift, useDiffusion, useCrossTalk);
    stsDigi->SetParameters(dynRange, threshold, nAdc, timeResolution, deadTime, noise);
    run->AddTask(stsDigi);
    
    
    FairTask* stsClusterFinder = new CbmStsFindClusters();
    run->AddTask(stsClusterFinder);
    
    FairTask* stsFindHits = new CbmStsFindHits();
    run->AddTask(stsFindHits);
    
    CbmKF* kalman = new CbmKF();
    run->AddTask(kalman);
    CbmL1* l1 = new CbmL1();
    l1->SetStsMaterialBudgetFileName(stsMatBudgetFileName.Data());
   // if (mvdMatBudgetFileName != "") l1->SetMvdMaterialBudgetFileName(mvdMatBudgetFileName.Data());
    run->AddTask(l1);
    
    CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
    FairTask* stsFindTracks = new CbmStsFindTracks(1, stsTrackFinder);
    run->AddTask(stsFindTracks);

    
    CbmLitFindGlobalTracks* finder = new CbmLitFindGlobalTracks();
    finder->SetTrackingType(std::string("branch"));
    finder->SetMergerType("nearest_hit");
    run->AddTask(finder);


    CbmRichDigitizer* richDigi  = new CbmRichDigitizer();
    richDigi->SetNofNoiseHits(0); // We do not need noise hits for UrqmdTest
    run->AddTask(richDigi);
    
   CbmRichHitProducer* richHitProd  = new CbmRichHitProducer();
   run->AddTask(richHitProd);

   CbmRichReconstruction* richReco = new CbmRichReconstruction();
   richReco->SetRunExtrapolation(true);
   richReco->SetRunProjection(true);
   richReco->SetRunTrackAssign(false);
   richReco->SetFinderName("ideal");
  // richReco->SetExtrapolationName("ideal");
   run->AddTask(richReco);

   CbmMatchRecoToMC* match = new CbmMatchRecoToMC();
   run->AddTask(match);

   CbmRichUrqmdTest* urqmdTest = new CbmRichUrqmdTest();
   urqmdTest->SetOutputDir(resultDir);
   run->AddTask(urqmdTest);

   // -----  Parameter database   --------------------------------------------
   FairRuntimeDb* rtdb = run->GetRuntimeDb();
   FairParRootFileIo* parIo1 = new FairParRootFileIo();
   FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
   parIo1->open(parFile.Data());
  // parIo2->open(parFileList, "in");
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

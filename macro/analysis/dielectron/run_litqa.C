void run_litqa(Int_t nEvents = 1000)
{
   TTree::SetMaxTreeSize(90000000000);

	TString script = TString(gSystem->Getenv("SCRIPT"));
	TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");

	//gRandom->SetSeed(10);

   /* TString dir = "/hera/cbm/users/slebedev/mc/dielectron/sep13/25gev/trd/1.0field/nomvd/rho0/";
	TString mcFile = dir + "mc.auau.25gev.centr.00001.root";
	TString parFile = dir + "/params.auau.25gev.centr.00001.root";
	TString recoFile = dir + "/test.reco.test.auau.25gev.centr.00001.root";
	TString qaFile = dir + "/test.litqa.test.auau.25gev.centr.00001.root";*/

	TString parFile = "/Users/slebedev/Development/cbm/data/simulations/lmvm/test.param.root";
	TString mcFile = "/Users/slebedev/Development/cbm/data/simulations/lmvm/test.mc.root";
	TString recoFile = "/Users/slebedev/Development/cbm/data/simulations/lmvm/test.reco.root";
	TString qaFile = "/Users/slebedev/Development/cbm/data/simulations/lmvm/test.litqa.root";

   TString resultDir = "results_litqa/";

   TString geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/analysis/dielectron/geosetup/geo_setup_lmvm.C";

	if (script == "yes") {
		mcFile = TString(gSystem->Getenv("MC_FILE"));
		parFile = TString(gSystem->Getenv("PAR_FILE"));
		recoFile = TString(gSystem->Getenv("RECO_FILE"));
		qaFile = TString(gSystem->Getenv("LITQA_FILE"));
		resultDir = TString(gSystem->Getenv("RESULT_DIR"));

		geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/analysis/dielectron/geosetup/" + TString(gSystem->Getenv("GEO_SETUP_FILE"));
	}

        resultDir = "";

         remove(qaFile.Data());

	//setup all geometries from macro
	cout << "geoSetupName:" << geoSetupFile << endl;
	gROOT->LoadMacro(geoSetupFile);
	init_geo_setup();

	// digi parameters
	TList *parFileList = new TList();
       // TObjString stsDigiFile = parDir + "/" + stsDigi;
	TObjString trdDigiFile = parDir + "/" + trdDigi;
	TObjString tofDigiFile = parDir + "/" + tofDigi;
      //  parFileList->Add(&stsDigiFile);
	if (trdDigiFile.GetString() != "") parFileList->Add(&trdDigiFile);
	parFileList->Add(&tofDigiFile);

	// material budget for STS and MVD
	TString mvdMatBudgetFileName = "";
	TString stsMatBudgetFileName = parDir + "/" + stsMatBudget;

   TStopwatch timer;
   timer.Start();

	// ----  Load libraries   -------------------------------------------------
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
	loadlibs();
	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/determine_setup.C");

	// -----   Reconstruction run   -------------------------------------------
	FairRunAna *run= new FairRunAna();
	if (mcFile != "") run->SetInputFile(mcFile);
	if (recoFile != "") run->AddFriend(recoFile);
	if (qaFile != "") run->SetOutputFile(qaFile);

	CbmKF* kalman = new CbmKF();
	run->AddTask(kalman);
	CbmL1* l1 = new CbmL1();
	l1->SetStsMaterialBudgetFileName(stsMatBudgetFileName.Data());
	if (mvdMatBudgetFileName != "") l1->SetMvdMaterialBudgetFileName(mvdMatBudgetFileName.Data());
	run->AddTask(l1);

   // Reconstruction Qa
   CbmLitTrackingQa* trackingQa = new CbmLitTrackingQa();
   trackingQa->SetMinNofPointsSts(4);
   trackingQa->SetUseConsecutivePointsInSts(true);
   trackingQa->SetMinNofPointsTrd(litQaMinNofPointsTrd);
   trackingQa->SetMinNofPointsMuch(10);
   trackingQa->SetMinNofPointsTof(1);
   trackingQa->SetQuota(0.7);
   trackingQa->SetMinNofHitsTrd(litQaMinNofPointsTrd);
   trackingQa->SetMinNofHitsMuch(10);
   trackingQa->SetVerbose(0);
   trackingQa->SetMinNofHitsRich(7);
   trackingQa->SetQuotaRich(0.6);
   trackingQa->SetPRange(40, 0., 4.);
   trackingQa->SetOutputDir(std::string(resultDir));
   std::vector<std::string> trackCat, richCat;
   trackCat.push_back("All");
   trackCat.push_back("Electron");
   richCat.push_back("All");
   richCat.push_back("Electron");
   richCat.push_back("ElectronReference");
   trackingQa->SetTrackCategories(trackCat);
   trackingQa->SetRingCategories(richCat);
   trackingQa->SetTrdAnnCut(trdAnnCut);
   run->AddTask(trackingQa);

   CbmLitFitQa* fitQa = new CbmLitFitQa();
   fitQa->SetMvdMinNofHits(0);
   fitQa->SetStsMinNofHits(4);
   fitQa->SetMuchMinNofHits(10);
   fitQa->SetTrdMinNofHits(litQaMinNofPointsTrd);
   fitQa->SetPRange(30, 0., 3.);
   fitQa->SetOutputDir(std::string(resultDir));
   run->AddTask(fitQa);

   CbmLitClusteringQa* clusteringQa = new CbmLitClusteringQa();
  // clusteringQa->SetMuchDigiFileName(muchDigiFile.Data());
   clusteringQa->SetOutputDir(std::string(resultDir));
   run->AddTask(clusteringQa);

   CbmLitTofQa* tofQa = new CbmLitTofQa();
   tofQa->SetOutputDir(std::string(resultDir));
  // run->AddTask(tofQa);


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

    run->Init();
    run->Run(0, nEvents);

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    std::cout << "Macro finished successfully." << std::endl;
    std::cout << "Output file is "    << recoFile << std::endl;
    std::cout << "Parameter file is " << parFile << std::endl;
    std::cout << "Real time " << timer.RealTime() << " s, CPU time " << timer.CpuTime() << " s" << std::endl;
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
}

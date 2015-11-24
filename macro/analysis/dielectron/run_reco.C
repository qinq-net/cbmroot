void run_reco(Int_t nEvents = 100)
{
   TTree::SetMaxTreeSize(90000000000);

	TString script = TString(gSystem->Getenv("SCRIPT"));
	TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");

	//gRandom->SetSeed(10);

  // TString parFile = "/Users/slebedev/Development/cbm/data/simulations/lmvm/test.param.root";
 //  TString mcFile = "/Users/slebedev/Development/cbm/data/simulations/lmvm/test.mc.root";
 //  TString recoFile = "/Users/slebedev/Development/cbm/data/simulations/lmvm/test.reco.root";

	TString dir = "/hera/cbm/users/slebedev/data/lmvm/sep15/25gev/geosetup_v1509_25gev/rho0/";
	TString mcFile = dir + "/mc.auau.25gev.centr.00134.root";
	TString parFile = dir + "/params.auau.25gev.centr.00134.root";
	TString recoFile = dir + "/reco.auau.25gev.centr.00134.root";
	//  TString analysisFile = dir + "/test.analysis.test.auau.25gev.centr.00001.root";

    TString geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/analysis/dielectron/geosetup/geosetup_v1509_25gev.C";

	TString delta = "no"; // if "yes" Delta electrons will be embedded
	TString deltaFile = "";

	if (script == "yes") {
		mcFile = TString(gSystem->Getenv("MC_FILE"));
		parFile = TString(gSystem->Getenv("PAR_FILE"));
		recoFile = TString(gSystem->Getenv("RECO_FILE"));
		delta = TString(gSystem->Getenv("DELTA"));
		deltaFile = TString(gSystem->Getenv("DELTA_FILE"));

		geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/analysis/dielectron/geosetup/" + TString(gSystem->Getenv("GEO_SETUP_FILE"));
	}

        remove(recoFile.Data());

       
	//setup all geometries from macro
	cout << "geoSetupName:" << geoSetupFile << endl;
	gROOT->LoadMacro(geoSetupFile);
	init_geo_setup();

	// digi parameters
	TList *parFileList = new TList();
	TObjString stsDigiFile = parDir + "/" + stsDigi;
	TObjString trdDigiFile = parDir + "/" + trdDigi;
	TObjString tofDigiFile = parDir + "/" + tofDigi;
	parFileList->Add(&stsDigiFile);
	if (trdDigiFile.GetString() != "") parFileList->Add(&trdDigiFile);
	parFileList->Add(&tofDigiFile);

	// material budget for STS and MVD
	TString mvdMatBudgetFileName = "";
	TString stsMatBudgetFileName = parDir + "/" + stsMatBudget;

	gDebug = 0;
	TStopwatch timer;
	timer.Start();

	// ----  Load libraries   -------------------------------------------------
	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
	loadlibs();
	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/determine_setup.C");

	// -----   Reconstruction run   -------------------------------------------
	FairRunAna *run= new FairRunAna();
        run->SetGenerateRunInfo(kTRUE);
	if (mcFile != "") run->SetInputFile(mcFile);
	if (recoFile != "") run->SetOutputFile(recoFile);

	CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
	mcManager->AddFile(mcFile);
	run->AddTask(mcManager);

    // =========================================================================
    // ===                     MVD local reconstruction                      ===
	// =========================================================================
	Bool_t useMvdInTracking = kFALSE;
	if (IsMvd(parFile)) {
		  CbmMvdDigitizer* mvdDigitise = new CbmMvdDigitizer("CbmMvdDigitizer", 0, 0);
		  run->AddTask(mvdDigitise);

		  CbmMvdClusterfinder* mvdCluster = new CbmMvdClusterfinder("CbmMvdClusterfinder", 0, 0);
		  run->AddTask(mvdCluster);

		  CbmMvdHitfinder* mvdHitfinder = new CbmMvdHitfinder("CbmMvdHitfinder", 0, 0);
		  mvdHitfinder->UseClusterfinder(kTRUE);
		  run->AddTask(mvdHitfinder);

		  useMvdInTracking = kTRUE;
		  mvdMatBudgetFileName = parDir + "/" + mvdMatBudget;
	}

	// =========================================================================
	// ===                      STS local reconstruction                     ===
	// =========================================================================
	Double_t dynRange = 40960.;  // Dynamic range [e]
	Double_t threshold = 4000.;  // Digitisation threshold [e]
	Int_t nAdc = 4096;   // Number of ADC channels (12 bit)
	Double_t timeResolution = 5.;  // time resolution [ns]
	Double_t deadTime = 9999999.;  // infinite dead time (integrate entire event)
	Double_t noise = 0.;  // ENC [e]
	Int_t digiModel = 1;   // Model: 1 = uniform charge distribution along track

	CbmStsDigitize* stsDigi = new CbmStsDigitize(digiModel);
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
	if (mvdMatBudgetFileName != "") l1->SetMvdMaterialBudgetFileName(mvdMatBudgetFileName.Data());
	run->AddTask(l1);

	CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
	FairTask* stsFindTracks = new CbmStsFindTracks(1, stsTrackFinder);
	run->AddTask(stsFindTracks);


	// =========================================================================
	// ===                     TRD local reconstruction                      ===
	// =========================================================================
	if (IsTrd(parFile)) {
		Bool_t simpleTR = kTRUE; // use fast and simple version for TR production
		CbmTrdRadiator *radiator = new CbmTrdRadiator(simpleTR , "K++");

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
			Bool_t triangularPads = false;// Bucharest triangular pad-plane layout
			//Double_t triggerThreshold = 0.5e-6;//SIS100
			Double_t triggerThreshold = 1.0e-6;//SIS300
			Double_t trdNoiseSigma_keV = 0.1; //default best matching to test beam PRF

			CbmTrdDigitizerPRF* trdDigiPrf = new CbmTrdDigitizerPRF(radiator);
			trdDigiPrf->SetTriangularPads(triangularPads);
			trdDigiPrf->SetNoiseLevel(trdNoiseSigma_keV);
			run->AddTask(trdDigiPrf);

			CbmTrdClusterFinderFast* trdCluster = new CbmTrdClusterFinderFast();
			trdCluster->SetNeighbourTrigger(true);
			trdCluster->SetTriggerThreshold(triggerThreshold);
			trdCluster->SetNeighbourRowTrigger(false);
			trdCluster->SetPrimaryClusterRowMerger(true);
			trdCluster->SetTriangularPads(triangularPads);
		        run->AddTask(trdCluster);

			CbmTrdHitProducerCluster* trdHit = new CbmTrdHitProducerCluster();
			trdHit->SetTriangularPads(triangularPads);
		        run->AddTask(trdHit);
		}
	}// isTRD

	// =========================================================================
	// ===                     TOF local reconstruction                      ===
	// =========================================================================
	if (IsTof(parFile)) {
		CbmTofHitProducerNew* tofHitProd = new CbmTofHitProducerNew("CbmTofHitProducer", 1);
		tofHitProd->SetInitFromAscii(kFALSE);
	        run->AddTask(tofHitProd);
	} //isTof

	// =========================================================================
	// ===                        Global tracking                            ===
	// =========================================================================

	CbmLitFindGlobalTracks* finder = new CbmLitFindGlobalTracks();
	finder->SetTrackingType("branch");
	finder->SetMergerType("nearest_hit");
        run->AddTask(finder);

	// -----   Primary vertex finding   ---------------------------------------
	CbmPrimaryVertexFinder* pvFinder = new CbmPVFinderKF();
	CbmFindPrimaryVertex* findVertex = new CbmFindPrimaryVertex(pvFinder);
        run->AddTask(findVertex);
	// ------------------------------------------------------------------------

	if (IsTrd(parFile)) {
		CbmTrdSetTracksPidANN* trdSetTracksPidAnnTask = new CbmTrdSetTracksPidANN("CbmTrdSetTracksPidANN","CbmTrdSetTracksPidANN");
		trdSetTracksPidAnnTask->SetTRDGeometryType("h++");
	        run->AddTask(trdSetTracksPidAnnTask);
	}//isTrd

    // =========================================================================
    // ===                        RICH reconstruction                        ===
    // =========================================================================
	if (IsRich(parFile)){
		CbmRichHitProducer* richHitProd  = new CbmRichHitProducer();
		richHitProd->SetDetectorType(4);
		richHitProd->SetNofNoiseHits(220);
		richHitProd->SetCollectionEfficiency(1.0);
		richHitProd->SetSigmaMirror(0.06);
	        run->AddTask(richHitProd);

		CbmRichReconstruction* richReco = new CbmRichReconstruction();
	        run->AddTask(richReco);

		// ------------------- RICH Ring matching  ---------------------------------
		CbmRichMatchRings* matchRings = new CbmRichMatchRings();
	        run->AddTask(matchRings);

	}//isRich

	CbmMatchRecoToMC* match = new CbmMatchRecoToMC();
        run->AddTask(match);

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

void run_reco(Int_t nEvents = 2)
{
   TTree::SetMaxTreeSize(90000000000);

	Int_t iVerbose = 0;

	TString script = TString(gSystem->Getenv("SCRIPT"));
	TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");

	gRandom->SetSeed(10);

	TString mcFile = "/Users/slebedev/Development/cbm/data/simulations/rich/richreco/mc.00090.root";
	TString parFile = "/Users/slebedev/Development/cbm/data/simulations/rich/richreco/param.00090.root";
	TString recoFile ="/Users/slebedev/Development/cbm/data/simulations/rich/richreco/reco.00090.root";

	TString geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/rich/run/geosetup/geosetup_25gev.C";

	std::string resultDir = "recqa_0001/";

	if (script == "yes") {
		mcFile = TString(gSystem->Getenv("MC_FILE"));
		recoFile = TString(gSystem->Getenv("RECO_FILE"));
		parFile = TString(gSystem->Getenv("PAR_FILE"));
		resultDir = TString(gSystem->Getenv("LIT_RESULT_DIR"));
		geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/rich/run/geosetup/" + TString(gSystem->Getenv("GEO_SETUP_FILE"));
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
	if (mcFile != "") run->SetInputFile(mcFile);
	if (recoFile != "") run->SetOutputFile(recoFile);

	// ----- MC Data Manager   ------------------------------------------------
	CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
	mcManager->AddFile(mcFile);
	run->AddTask(mcManager);

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
	finder->SetTrackingType(std::string("branch"));
	finder->SetMergerType("nearest_hit");
	run->AddTask(finder);

	CbmPrimaryVertexFinder* pvFinder = new CbmPVFinderKF();
	CbmFindPrimaryVertex* findVertex = new CbmFindPrimaryVertex(pvFinder);
	run->AddTask(findVertex);


	if (IsTrd(parFile)) {
		CbmTrdSetTracksPidANN* trdSetTracksPidAnnTask = new CbmTrdSetTracksPidANN("CbmTrdSetTracksPidANN","CbmTrdSetTracksPidANN");
		trdSetTracksPidAnnTask->SetTRDGeometryType("h++");
		run->AddTask(trdSetTracksPidAnnTask);
	}//isTrd

    // =========================================================================
    // ===                        RICH reconstruction                        ===
    // =========================================================================
	if (IsRich(parFile)){
		CbmRichHitProducer* richHitProd	= new CbmRichHitProducer();
		richHitProd->SetDetectorType(4);
		richHitProd->SetNofNoiseHits(nofNoiseHitsInRich);
		richHitProd->SetCollectionEfficiency(collectionEff);
		richHitProd->SetSigmaMirror(sigmaErrorRich);
		richHitProd->SetCrossTalkHitProb(crosstalkRich);
		run->AddTask(richHitProd);

		CbmRichReconstruction* richReco = new CbmRichReconstruction();
		run->AddTask(richReco);

		CbmRichMatchRings* matchRings = new CbmRichMatchRings();
		run->AddTask(matchRings);
	}//isRich


	CbmMatchRecoToMC* matchRecoToMc = new CbmMatchRecoToMC();
	run->AddTask(matchRecoToMc);

	// Reconstruction Qa
	CbmLitTrackingQa* trackingQa = new CbmLitTrackingQa();
	trackingQa->SetMinNofPointsSts(4);
	trackingQa->SetUseConsecutivePointsInSts(true);
	trackingQa->SetMinNofPointsTrd(minNofPointsTrd);
	trackingQa->SetMinNofPointsMuch(10);
	trackingQa->SetMinNofPointsTof(1);
	trackingQa->SetQuota(0.7);
	trackingQa->SetMinNofHitsTrd(minNofPointsTrd);
	trackingQa->SetMinNofHitsMuch(10);
	trackingQa->SetVerbose(0);
	trackingQa->SetMinNofHitsRich(7);
	trackingQa->SetQuotaRich(0.6);
	trackingQa->SetOutputDir(resultDir);
	trackingQa->SetPRange(20, 0., 10.);
	trackingQa->SetTrdAnnCut(trdAnnCut);
	std::vector<std::string> trackCat, richCat;
	trackCat.push_back("All");
	trackCat.push_back("Electron");
	richCat.push_back("Electron");
	richCat.push_back("ElectronReference");
	trackingQa->SetTrackCategories(trackCat);
	trackingQa->SetRingCategories(richCat);
	run->AddTask(trackingQa);

	CbmLitFitQa* fitQa = new CbmLitFitQa();
	fitQa->SetMvdMinNofHits(0);
	fitQa->SetStsMinNofHits(4);
	fitQa->SetMuchMinNofHits(10);
	fitQa->SetTrdMinNofHits(minNofPointsTrd);
	fitQa->SetOutputDir(resultDir);
	// run->AddTask(fitQa);

	CbmLitClusteringQa* clusteringQa = new CbmLitClusteringQa();
	clusteringQa->SetOutputDir(resultDir);
	run->AddTask(clusteringQa);


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

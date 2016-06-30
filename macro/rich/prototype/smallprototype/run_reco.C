void run_reco(Int_t nEvents = 100)
{
   TTree::SetMaxTreeSize(90000000000);


	string resultDir = "results_smallprototype/";

	Int_t iVerbose = 0;

	TString script = TString(gSystem->Getenv("SCRIPT"));
	TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters/");

	gRandom->SetSeed(10);

	//TString mcFile = "/data/cbm/Gregor/mc.00100.root";
	//TString parFile = "/data/cbm/Gregor/param.00100.root";
	//TString recoFile ="/data/cbm/Gregor/reco.00100.root";
    
    TString parFile = "/Users/slebedev/Development/cbm/data/simulations/richprototype/param.00001.root";
    TString recoFile = "/Users/slebedev/Development/cbm/data/simulations/richprototype/reco.00001.root";
    TString mcFile = "/Users/slebedev/Development/cbm/data/simulations/richprototype/mc.00001.root";

	remove(recoFile.Data());

	gDebug = 0;

    TStopwatch timer;
    timer.Start();

	// ----  Load libraries   -------------------------------------------------
	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
	loadlibs();

	// -----   Reconstruction run   -------------------------------------------
	FairRunAna *run= new FairRunAna();
	if (mcFile != "") run->SetInputFile(mcFile);
	if (recoFile != "") run->SetOutputFile(recoFile);

	//CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
    //mcManager->AddFile(mcFile);
    //run->AddTask(mcManager);

    CbmRichDigitizer* richDigitizer = new CbmRichDigitizer();
    richDigitizer->SetNofNoiseHits(0);
    run->AddTask(richDigitizer);

    CbmRichHitProducer* richHitProd	= new CbmRichHitProducer();
    richHitProd->SetRotationNeeded(false);
    run->AddTask(richHitProd);
    
	CbmRichReconstruction* richReco = new CbmRichReconstruction();
	richReco->SetRunExtrapolation(false);
	richReco->SetRunProjection(false);
	richReco->SetRunTrackAssign(false);
	richReco->SetFinderName("ideal");
	run->AddTask(richReco);

	//CbmMatchRecoToMC* matchRecoToMc = new CbmMatchRecoToMC();
	//run->AddTask(matchRecoToMc);
	
    CbmRichSmallPrototypeQa* richQa = new CbmRichSmallPrototypeQa();
    richQa->SetOutputDir(std::string(resultDir));
    run->AddTask(richQa);

	CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
    mcManager->AddFile(mcFile);
    run->AddTask(mcManager);

	
	

	// -----  Parameter database   --------------------------------------------
	FairRuntimeDb* rtdb = run->GetRuntimeDb();
	FairParRootFileIo* parIo1 = new FairParRootFileIo();
	FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
	parIo1->open(parFile.Data());
	//parIo2->open(parFileList, "in");
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

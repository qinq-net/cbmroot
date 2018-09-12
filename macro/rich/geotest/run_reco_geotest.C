
void run_reco_geotest(Int_t nEvents = 100)
{
   TTree::SetMaxTreeSize(90000000000);
   TString script = TString(gSystem->Getenv("SCRIPT"));

   TString myName = "run_reco_geotest";
   TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory

   TString geoSetupFile = srcDir + "/macro/rich/geosetup/rich_setup_sis100.C";

   TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/geotest/";
   TString mcFile = outDir + "mc.00000.root";
   TString parFile = outDir + "param.00000.root";
   TString digiFile = outDir + "digi.00000.root";
   TString recoFile = outDir + "reco.00000.root";
   std::string resultDir = "results_geotest_v18b_1e/";

   if (script == "yes") {
      mcFile = TString(gSystem->Getenv("MC_FILE"));
      recoFile = TString(gSystem->Getenv("RECO_FILE"));
      parFile = TString(gSystem->Getenv("PAR_FILE"));
      digiFile = TString(gSystem->Getenv("DIGI_FILE"));
      resultDir = TString(gSystem->Getenv("RESULT_DIR"));
      geoSetupFile = srcDir + TString(gSystem->Getenv("GEO_SETUP_FILE"));
   }

    remove(recoFile.Data());

    TString setupFunct = "do_setup()";
    std::cout << "-I- " << myName << ": Loading macro " << geoSetupFile << std::endl;
    gROOT->LoadMacro(geoSetupFile);
    gROOT->ProcessLine(setupFunct);

	std::cout << std::endl<< "-I- " << myName << ": Defining parameter files " << std::endl;
	TList *parFileList = new TList();

	TStopwatch timer;
	timer.Start();
	gDebug = 0;

	FairRunAna *run = new FairRunAna();
	FairFileSource* inputSource = new FairFileSource(digiFile);
	inputSource->AddFriend(mcFile);
	run->SetSource(inputSource);
	run->SetOutputFile(recoFile);
	run->SetGenerateRunInfo(kTRUE);

	FairLogger::GetLogger()->SetLogScreenLevel("INFO");
	FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

	CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
	mcManager->AddFile(mcFile);
	run->AddTask(mcManager);

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

	std::cout << std::endl << std::endl << "-I- " << myName << ": Set runtime DB" << std::endl;
	FairRuntimeDb* rtdb = run->GetRuntimeDb();
	FairParRootFileIo* parIo1 = new FairParRootFileIo();
	FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
	parIo1->open(parFile.Data(),"UPDATE");
	rtdb->setFirstInput(parIo1);
	if ( ! parFileList->IsEmpty() ) {
		parIo2->open(parFileList, "in");
		rtdb->setSecondInput(parIo2);
	}


	std::cout << std::endl << "-I- " << myName << ": Initialise run" << std::endl;
	run->Init();


	rtdb->setOutput(parIo1);
	rtdb->saveOutput();
	rtdb->print();


	std::cout << std::endl << std::endl;
	std::cout << "-I- " << myName << ": Starting run" << std::endl;
	run->Run(0, nEvents);


	timer.Stop();
	Double_t rtime = timer.RealTime();
	Double_t ctime = timer.CpuTime();
	std::cout << std::endl << std::endl;
	std::cout << "Macro finished succesfully." << std::endl;
	std::cout << "Output file is " << mcFile << std::endl;
	std::cout << "Parameter file is " << parFile << std::endl;
	std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl;
	std::cout << " Test passed" << std::endl;
	std::cout << " All ok " << std::endl;

}

void run_reco_urqmdtest(Int_t nEvents = 5)
{
    TTree::SetMaxTreeSize(90000000000);
    TString script = TString(gSystem->Getenv("SCRIPT"));

    TString myName = "run_reco_urqmdtest";
    TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory

    TString geoSetupFile = srcDir + "/macro/rich/geosetup/rich_setup_sis100.C";

    TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/urqmdtest/";
    TString mcFile = outDir + "mc.00000.root";
    TString parFile = outDir + "param.00000.root";
    TString digiFile = outDir + "digi.00000.root";
    TString recoFile = outDir + "reco.00000.root";
    std::string resultDir = "results_urqmdtest/";

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

    CbmSetup* setup = CbmSetup::Instance();

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


    CbmStsFindClusters* stsCluster = new CbmStsFindClusters();
    stsCluster->UseEventMode();
    run->AddTask(stsCluster);

    FairTask* stsHit = new CbmStsFindHitsEvents();
    run->AddTask(stsHit);

    CbmKF* kalman = new CbmKF();
    run->AddTask(kalman);
    CbmL1* l1 = new CbmL1("L1", 0);
    TString stsGeoTag;
    if ( setup->GetGeoTag(kSts, stsGeoTag) ) {
        TString parFile = gSystem->Getenv("VMCWORKDIR");
        parFile = parFile + "/parameters/sts/sts_matbudget_" + stsGeoTag + ".root";
        std::cout << "Using material budget file " << parFile << std::endl;
        l1->SetStsMaterialBudgetFileName(parFile.Data());
    }
    run->AddTask(l1);

    CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
    FairTask* stsFindTracks = new CbmStsFindTracksEvents(stsTrackFinder, false);
    run->AddTask(stsFindTracks);


    CbmLitFindGlobalTracks* finder = new CbmLitFindGlobalTracks();
    finder->SetTrackingType("branch");
    finder->SetMergerType("nearest_hit");
    run->AddTask(finder);


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
    std::cout << "Reco file is " << recoFile << std::endl;
    std::cout << "Parameter file is " << parFile << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl;
    std::cout << std::endl;
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
}

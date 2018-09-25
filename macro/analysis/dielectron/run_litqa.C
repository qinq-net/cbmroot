void run_litqa(Int_t nEvents = 2)
{
    TTree::SetMaxTreeSize(90000000000);
    TString script = TString(gSystem->Getenv("SCRIPT"));

    TString myName = "run_litqa";
    TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory

//    TString geoSetupFile = srcDir + "/macro/analysis/dielectron/geosetup/diel_setup_sis100.C";

    TString geoSetupFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/trunk/geometry/setup/setup_sis100_electron.C";  

    TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/reco/";
    TString mcFile = outDir + "mc.00000.root";
    TString parFile = outDir + "param.00000.root";
    TString recoFile = outDir + "reco.00000.root";
    TString litqaFile = outDir + "litqa.00000.root";


/*    
    TString mcFile =  "/lustre/nyx/cbm/prod/mc/r13109/sis100_electron.00001.tra.root";
    TString parFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/testUhlig/sis100_electron.00001.par.root";
    TString recoFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/testUhlig/sis100_electron.00001.reco.root";
    TString litqaFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/testUhlig/sis100_electron.00001.litqa.root";
*/
    TString energy = "4.5gev";
    TString plutoParticle = "omegaepem";


    std::string resultDir = "results_recoqa/";

    if (script == "yes") {
        mcFile = TString(gSystem->Getenv("MC_FILE"));
        recoFile = TString(gSystem->Getenv("RECO_FILE"));
	parFile = TString(gSystem->Getenv("PAR_FILE"));
        litqaFile = TString(gSystem->Getenv("LITQA_FILE"));
        geoSetupFile = TString(gSystem->Getenv("GEO_SETUP_FILE"));
        energy = TString(gSystem->Getenv("ENERGY"));
        plutoParticle = TString(gSystem->Getenv("PLUTO_PARTICLE"));
        resultDir = TString(gSystem->Getenv("LIT_RESULT_DIR"));
    }

    remove(litqaFile.Data());

    TString setupFunct = "do_setup()";

    if(script=="yes"){
        setupFunct = TString(gSystem->Getenv("SETUP_FUNCT"));
    }

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
    FairFileSource* inputSource = new FairFileSource(mcFile);
    inputSource->AddFriend(recoFile);
    run->SetSource(inputSource);
    run->SetOutputFile(litqaFile);
    run->SetGenerateRunInfo(kTRUE);

    FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

    CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
    mcManager->AddFile(mcFile);
    run->AddTask(mcManager);

    CbmKF* kalman = new CbmKF();
    run->AddTask(kalman);
    CbmL1* l1 = new CbmL1();
    run->AddTask(l1);


    // RICH reco QA
    CbmRichRecoQa* richRecoQa = new CbmRichRecoQa();
    richRecoQa->SetOutputDir(resultDir);
    //run->AddTask(richRecoQa);

    //    // Reconstruction Qa
    CbmLitTrackingQa* trackingQa = new CbmLitTrackingQa();
    trackingQa->SetMinNofPointsSts(4);
    trackingQa->SetUseConsecutivePointsInSts(true);
    // trackingQa->SetMinNofPointsTrd(minNofPointsTrd);
    trackingQa->SetMinNofPointsMuch(10);
    trackingQa->SetMinNofPointsTof(1);
    trackingQa->SetQuota(0.7);
    // trackingQa->SetMinNofHitsTrd(minNofPointsTrd);
    trackingQa->SetMinNofHitsMuch(10);
    trackingQa->SetVerbose(0);
    trackingQa->SetMinNofHitsRich(7);
    trackingQa->SetQuotaRich(0.6);
    trackingQa->SetOutputDir(resultDir);
    trackingQa->SetPRange(12, 0., 6.);

//auau    
    trackingQa->SetTrdAnnCut(0.85);
    trackingQa->SetRichAnnCut(-0.4);
//agag
    //trackingQa->SetTrdAnnCut(0.85);
    //trackingQa->SetRichAnnCut(-0.4);

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
    fitQa->SetStsMinNofHits(6);
    fitQa->SetMuchMinNofHits(10);
    // fitQa->SetTrdMinNofHits(minNofPointsTrd);
    fitQa->SetOutputDir(resultDir);
    //run->AddTask(fitQa);

    CbmLitClusteringQa* clusteringQa = new CbmLitClusteringQa();
    clusteringQa->SetOutputDir(resultDir);
    //run->AddTask(clusteringQa);

    CbmLitTofQa* tofQa = new CbmLitTofQa();
    tofQa->SetOutputDir(std::string(resultDir));
    //run->AddTask(tofQa);



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

    std::cout << "-I- " << myName << ": Starting run" << std::endl;
    run->Run(0,nEvents);


    // Finish
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    std::cout << std::endl << std::endl;
    std::cout << "Macro finished succesfully." << std::endl;
    std::cout << "Qa file is " << litqaFile << std::endl;
    std::cout << "Parameter file is " << parFile << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl;
    std::cout << std::endl;
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
}

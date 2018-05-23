void run_analysis(Int_t nEvents = 2)
{
    TTree::SetMaxTreeSize(90000000000);
    TString script = TString(gSystem->Getenv("SCRIPT"));

    TString myName = "run_reco";
 //   TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
    TString srcDir = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/";

    TString geoSetupFile = srcDir + "trunk/macro/analysis/dielectron/geosetup/diel_setup_sis100.C";

    //    TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/reco/";

    TString outDir = srcDir + "results/results_dielectron/";

    TString mcFile = outDir + "mc.00001.root";
    TString parFile = outDir + "param.00001.root";
    TString recoFile = outDir + "reco.00001.root";
    TString analysisFile = outDir + "analysis.00001.root";

    TString energy = "10gev";
    TString plutoParticle = "rho0";

    if (script == "yes") {
        mcFile = TString(gSystem->Getenv("MC_FILE"));
        recoFile = TString(gSystem->Getenv("RECO_FILE"));
        parFile = TString(gSystem->Getenv("PAR_FILE"));
        geoSetupFile = srcDir + TString(gSystem->Getenv("GEO_SETUP_FILE"));
        energy = TString(gSystem->Getenv("ENERGY"));
        analysisFile = TString(gSystem->Getenv("ANALYSIS_FILE"));
	plutoParticle = TString(gSystem->Getenv("PLUTO_PARTICLE"));
    }

    remove(analysisFile.Data());

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
    FairFileSource* inputSource = new FairFileSource(mcFile);
    inputSource->AddFriend(recoFile);
    run->SetSource(inputSource);
    run->SetOutputFile(analysisFile);
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

    CbmAnaDielectronTask *task = new CbmAnaDielectronTask();
    task->SetEnergyAndPlutoParticle(std::string(energy.Data()), std::string(plutoParticle.Data()));
    task->SetUseMvd(false);
    task->SetUseRich(true);
    task->SetUseTrd(true);
    task->SetUseTof(true);
   // task->SetPionMisidLevel(pionMisidLevel);
  //  task->SetTrdAnnCut(trdAnnCut);
    run->AddTask(task);


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
    std::cout << "Output file is " << recoFile << std::endl;
    std::cout << "Parameter file is " << parFile << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl;
    std::cout << std::endl;
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
}

void run_analysis(Int_t nEvents = 100)
{
    TTree::SetMaxTreeSize(90000000000);
    TString script = TString(gSystem->Getenv("SCRIPT"));

    TString myName = "run_analysis";
//    TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
    TString srcDir = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/trunk/";

//    TString geoSetupFile = srcDir + "/macro/analysis/dielectron/geosetup/diel_setup_sis100.C";
    TString geoSetupFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/trunk/macro/analysis/dielectron/geosetup/diel_setup_sis100.C";
//    TString geoSetupFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/trunk/geometry/setup/setup_sis100_electron.C";

   // TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/reco/";
    TString outDir = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/july18_100k_field50/8gev/omegaepem/";
    TString mcFile = outDir + "mc.auau.8gev.centr.00001.root";//outDir + "mc.00000.root";
    TString parFile = outDir + "params.auau.8gev.centr.00001.root";//outDir + "param.00000.root";
    TString recoFile = outDir + "reco.auau.8gev.centr.00001.root";
    TString analysisFile = outDir + "analysis.auau.8gev.centr.00001.root";


//    TString mcFile =  "/lustre/nyx/cbm/prod/mc/r13109/sis100_electron.00001.tra.root";
//    TString parFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/testUhlig/sis100_electron.00001.par.root";
//    TString recoFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/testUhlig/sis100_electron.00001.reco.root";
//    TString analysisFile = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/testUhlig/sis100_electron.00001.analysis.root";

    TString energy = "8gev";
    TString plutoParticle = "omegaepem";

    if (script == "yes") {
        mcFile = TString(gSystem->Getenv("MC_FILE"));
        recoFile = TString(gSystem->Getenv("RECO_FILE"));
        parFile = TString(gSystem->Getenv("PAR_FILE"));
        geoSetupFile = TString(gSystem->Getenv("GEO_SETUP_FILE"));
        energy = TString(gSystem->Getenv("ENERGY"));
        analysisFile = TString(gSystem->Getenv("ANALYSIS_FILE"));
	plutoParticle = TString(gSystem->Getenv("PLUTO_PARTICLE"));
    }

    remove(analysisFile.Data());

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
   // task->SetTrdAnnCut(0.85);
   // task->SetRichAnnCut(-0.4);
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
    std::cout << "Analysis file is " << analysisFile << std::endl;
    std::cout << "Parameter file is " << parFile << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl;
    std::cout << std::endl;
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
}

void run_reco(Int_t nEvents = 2)
{
    TTree::SetMaxTreeSize(90000000000);
    TString script = TString(gSystem->Getenv("SCRIPT"));

    TString myName = "run_reco";
    TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory

    TString geoSetupFile = srcDir + "/macro/analysis/dielectron/geosetup/diel_setup_sis100.C";

    TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/reco/";
//    TString outDir = "/lustre/nyx/cbm/users/gpitsch/CbmRoot/results/results_dielectron/";
    TString mcFile = outDir + "mc.00000.root";
    TString parFile = outDir + "param.00000.root";
    TString recoFile = outDir + "reco.00000.root";


    if (script == "yes") {
        mcFile = TString(gSystem->Getenv("MC_FILE"));
        recoFile = TString(gSystem->Getenv("RECO_FILE"));
        parFile = TString(gSystem->Getenv("PAR_FILE"));
        geoSetupFile = TString(gSystem->Getenv("GEO_SETUP_FILE"));
    }

    remove(recoFile.Data());

    TString setupFunct = "do_setup()";
    std::cout << "-I- " << myName << ": Loading macro " << geoSetupFile << std::endl;
    gROOT->LoadMacro(geoSetupFile);
    gROOT->ProcessLine(setupFunct);

    CbmSetup* setup = CbmSetup::Instance();

    std::cout << std::endl<< "-I- " << myName << ": Defining parameter files " << std::endl;
    TList *parFileList = new TList();
    TString geoTag;

    // - TRD digitisation parameters
    if ( CbmSetup::Instance()->GetGeoTag(kTrd, geoTag) ) {
        TObjString* trdFile = new TObjString(srcDir + "/parameters/trd/trd_" + geoTag + ".digi.par");
        parFileList->Add(trdFile);
        std::cout << "-I- " << myName << ": Using parameter file " << trdFile->GetString() << std::endl;
    }
    // - TOF digitisation parameters
    if ( CbmSetup::Instance()->GetGeoTag(kTof, geoTag) ) {
        TObjString* tofFile = new TObjString(srcDir + "/parameters/tof/tof_" + geoTag + ".digi.par");
        parFileList->Add(tofFile);
        std::cout << "-I- " << myName << ": Using parameter file " << tofFile->GetString() << std::endl;
        TObjString* tofBdfFile = new TObjString(srcDir + "/parameters/tof/tof_" + geoTag + ".digibdf.par");
        parFileList->Add(tofBdfFile);
        std::cout << "-I- " << myName << ": Using parameter file " << tofBdfFile->GetString() << std::endl;
    }

    TStopwatch timer;
    timer.Start();
    gDebug = 0;


    FairRunAna *run = new FairRunAna();
    FairFileSource* inputSource = new FairFileSource(mcFile);
    run->SetSource(inputSource);
    run->SetOutputFile(recoFile);
    run->SetGenerateRunInfo(kTRUE);

    FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");


    CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
    mcManager->AddFile(mcFile);
    run->AddTask(mcManager);


    // Digitisers
    TString macroName = srcDir + "/macro/run/modules/digitize.C";
    std::cout << "Loading macro " << macroName << std::endl;
    gROOT->LoadMacro(macroName);
    gROOT->ProcessLine("digitize()");


    // Reconstruction tasks
    macroName = srcDir + "/macro/run/modules/reconstruct.C";
    std::cout << "Loading macro " << macroName << std::endl;
    gROOT->LoadMacro(macroName);
    Bool_t recoSuccess = gROOT->ProcessLine("reconstruct()");
    if ( ! recoSuccess ) {
        std::cerr << "-E-" << myName << ": error in executing " << macroName << std::endl;
        return;
    }
    std::cout << "-I-" << myName << ": " << macroName << " excuted successfully" << std::endl;


    CbmMatchRecoToMC* matchRecoToMc = new CbmMatchRecoToMC();
    run->AddTask(matchRecoToMc);

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
    std::cout << "Reco file is " << recoFile << std::endl;
    std::cout << "Parameter file is " << parFile << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl;
    std::cout << std::endl;
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
}

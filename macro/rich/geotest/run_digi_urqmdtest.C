void run_digi_urqmdtest(Int_t nEvents = 5)
{


    TTree::SetMaxTreeSize(90000000000);
    TString script = TString(gSystem->Getenv("SCRIPT"));

    TString myName = "run_reco_urqmdtest";
    TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory

    TString geoSetupFile = srcDir + "/macro/rich/geosetup/rich_setup_sis100.C";
    Double_t eventRate = 1.e7;
    Double_t timeSliceLength = 1.e4;
    Bool_t eventMode = true;

    TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/urqmdtest/";
    TString mcFile = outDir + "mc.00000.root";
    TString parFile = outDir + "param.00000.root";
    TString digiFile = outDir + "digi.00000.root";

    if (script == "yes") {
        mcFile = TString(gSystem->Getenv("MC_FILE"));
        digiFile = TString(gSystem->Getenv("DIGI_FILE"));
        parFile = TString(gSystem->Getenv("PAR_FILE"));
        geoSetupFile = srcDir + TString(gSystem->Getenv("GEO_SETUP_FILE"));
    }

    remove(digiFile.Data());

    FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");
    Bool_t overwrite = kTRUE;

    TStopwatch timer;
    timer.Start();

    CbmDigitization run;
    run.AddInput(mcFile, eventRate);
    run.SetOutputFile(digiFile, overwrite);
    run.SetParameterRootFile(parFile);
    run.SetTimeSliceLength(timeSliceLength);
    run.SetEventMode(eventMode);

    run.Run(nEvents);
    // ------------------------------------------------------------------------

    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    std::cout << std::endl;
    std::cout << "Macro finished successfully." << std::endl;
    std::cout << "Digi file is " << digiFile << std::endl;
    std::cout << "Parameter file is " << parFile << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl << std::endl;
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
}

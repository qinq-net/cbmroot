/* Reconstruct rings, from simulated data. */

void old_run_reco(Int_t nEvents = 100, int geom_nb = 0)
{

    TTree::SetMaxTreeSize(90000000000);
    gRandom->SetSeed(10);

    char PATH1[256];
    char PATH2[256];
    char PATH3[256];

    TString dir = "/home/jordan/Desktop/misalignment_correction/";
    TString runTitle = "Align";
    sprintf(PATH1, "Parameters_RichGeo_jordan_%d.root", geom_nb);
    sprintf(PATH2, "Sim_RichGeo_jordan_%d.root", geom_nb);
    sprintf(PATH3, "Rec_RichGeo_jordan_%d.root", geom_nb);

    TString ParFile = dir + PATH1; // Load Parameter File
    TString SimFile = dir + PATH2; // Load Simulation File
    TString RecFile = dir + PATH3; // Set Output File
    TString stsMatBudgetFileName = "/data/Cbm_Root/Cbm_Root/trunk/parameters/sts/sts_matbudget_v13d.root"; // Material budget file for L1 STS tracking

    // ------------------------------------------------------------------- //
    TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
    TList *parFileList = new TList();
    TObjString stsDigiFile = parDir + "/sts/sts_v13d_std.digi.par"; // STS digi file

    parFileList->Add(&stsDigiFile);
    gDebug = 0;
    TStopwatch timer;
    timer.Start();

    gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
    loadlibs();

    FairRunAna *run= new FairRunAna();
    run->SetInputFile(SimFile);
    run->SetOutputFile(RecFile);


    // =========================================================================
    // ====                     STS local reconstruction                    ====
    // =========================================================================

    Double_t dynRange       =   40960.;  // Dynamic range [e]
    Double_t threshold      =    4000.;  // Digitisation threshold [e]
    Int_t nAdc              =    4096;  // Number of ADC channels (12 bit)
    Double_t timeResolution =       5.;  // time resolution [ns]
    Double_t deadTime       = 9999999.;  // infinite dead time (integrate entire event)
    Double_t noise          =       0.;  // ENC [e]
    Int_t digiModel         = 1;  // Model: 1 = uniform charge distribution along track
    Int_t iVerbose          = 0;

    CbmStsDigitize* stsDigi = new CbmStsDigitize(digiModel);
    stsDigi->SetParameters(dynRange, threshold, nAdc, timeResolution, deadTime, noise);
    run->AddTask(stsDigi);

    FairTask* stsCluster = new CbmStsFindClusters();
    run->AddTask(stsCluster);

    FairTask* stsHit = new CbmStsFindHits();
    run->AddTask(stsHit);

    CbmKF *kalman = new CbmKF();
    run->AddTask(kalman);

    CbmL1* l1 = new CbmL1();
    l1->SetMaterialBudgetFileName(stsMatBudgetFileName);
    run->AddTask(l1);

    CbmStsTrackFinder* stsTrackFinder = new CbmL1StsTrackFinder();
    Bool_t useMvd = kTRUE;
    FairTask* stsFindTracks = new CbmStsFindTracks(iVerbose, stsTrackFinder, useMvd);
    run->AddTask(stsFindTracks);


    CbmRichHitProducer* richHitProd  = new CbmRichHitProducer();	//Set PMT
    richHitProd->SetDetectorType(6);
    richHitProd->SetNofNoiseHits(220);
    richHitProd->SetCollectionEfficiency(1.0);
    richHitProd->SetSigmaMirror(0.06);	//Smearing
    richHitProd->SetCrossTalkHitProb(0.0);
    run->AddTask(richHitProd);

    CbmRichReconstruction* richReco = new CbmRichReconstruction();
    richReco->SetRunExtrapolation(true);
    richReco->SetRunProjection(true);
    richReco->SetRunTrackAssign(true);
    richReco->SetFinderName("ideal");
    run->AddTask(richReco);

    CbmRichMatchRings* matchRings = new CbmRichMatchRings();
    run->AddTask(matchRings);

    CbmRichAlignment* richAlign = new CbmRichAlignment();
    richAlign->SetOutputDir(dir);
    richAlign->SetRunTitle(runTitle);
    richAlign->SetDrawHist(true);
    richAlign->SetIsSimulationAna(true);
    run->AddTask(richAlign);

    // -----  Parameter database   --------------------------------------------
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    FairParRootFileIo* parIo1 = new FairParRootFileIo();
    FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
    parIo1->open(ParFile.Data());
    parIo2->open(parFileList, "in");
    rtdb->setFirstInput(parIo1);
    rtdb->setSecondInput(parIo2);
    rtdb->setOutput(parIo1);
    rtdb->saveOutput();
    run->Init();
    run->Run(0,nEvents);

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished successfully." << endl;
    cout << "Output file is "    << RecFile << endl;
    cout << "Parameter file is " << ParFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
    cout << endl;

    cout << " Test passed" << endl;
    cout << " All ok " << endl;

}

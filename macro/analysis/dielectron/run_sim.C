void run_sim(Int_t nEvents = 2)
{

    TTree::SetMaxTreeSize(90000000000);

    TString script = TString(gSystem->Getenv("SCRIPT"));

    TString myName = "run_sim";  // this macro's name for screen output
    TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory

    TString geoSetupFile = srcDir + "/macro/analysis/dielectron/geosetup/diel_setup_sis100.C";

    TString urqmdFile = "/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root";
    TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/reco/";
    TString parFile =  outDir + "param.00000.root";
    TString mcFile = outDir + "mc.00000.root";
    TString geoFile = outDir + "geosim.00000.root";

    TString electrons = "yes"; // If "yes" than primary electrons will be generated
    Int_t NELECTRONS = 5; // number of e- to be generated
    Int_t NPOSITRONS = 5; // number of e+ to be generated
    TString urqmd = "yes"; // If "yes" than UrQMD will be used as background
    TString pluto = "no"; // If "yes" PLUTO particles will be embedded
    TString plutoFile = "";
    TString plutoParticle = "";


    if (script == "yes") {
        urqmdFile = TString(gSystem->Getenv("URQMD_FILE"));
        mcFile = TString(gSystem->Getenv("MC_FILE"));
        parFile = TString(gSystem->Getenv("PAR_FILE"));
        geoFile = TString(gSystem->Getenv("GEOSIM_FILE"));
        geoSetupFile = srcDir + TString(gSystem->Getenv("GEO_SETUP_FILE"));

        NELECTRONS = TString(gSystem->Getenv("NELECTRONS")).Atoi();
        NPOSITRONS = TString(gSystem->Getenv("NPOSITRONS")).Atoi();
        electrons = TString(gSystem->Getenv("ELECTRONS"));
        urqmd = TString(gSystem->Getenv("URQMD"));
        pluto = TString(gSystem->Getenv("PLUTO"));
        plutoFile = TString(gSystem->Getenv("PLUTO_FILE"));
        plutoParticle = TString(gSystem->Getenv("PLUTO_PARTICLE"));
    }
    remove(parFile.Data());
    remove(mcFile.Data());
    remove(geoFile.Data());

    // Target geometry
    TString  targetElement   = "Gold";
    Double_t targetThickness = 0.025;  // full thickness in cm // for lmvm - 0.0025; // 25 mum
    Double_t targetDiameter  = 2.5;    // diameter in cm
    Double_t targetPosX      = 0.;     // target x position in global c.s. [cm]
    Double_t targetPosY      = 0.;     // target y position in global c.s. [cm]
    Double_t targetPosZ      = 0.;     // target z position in global c.s. [cm]
    Double_t targetRotY      = 0.;     // target rotation angle around the y axis [deg]


    // primary vertex
    Bool_t smearVertexXY = kTRUE;
    Bool_t smearVertexZ  = kTRUE;
    Double_t beamWidthX   = 0.1;  // Gaussian sigma of the beam profile in x [cm]
    Double_t beamWidthY   = 0.1;  // Gaussian sigma of the beam profile in y [cm]

    TStopwatch timer;
    timer.Start();
    gDebug = 0;

    FairRunSim* run = new FairRunSim();
    run->SetName("TGeant3");
    run->SetOutputFile(mcFile);
    run->SetGenerateRunInfo(kTRUE);

    FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

    TString setupFunct = "do_setup()";
    std::cout << "-I- " << myName << ": Loading macro " << geoSetupFile << std::endl;
    gROOT->LoadMacro(geoSetupFile);
    gROOT->ProcessLine(setupFunct);

    std::cout << std:: endl << "-I- " << myName << ": Setting media file" << std::endl;
    run->SetMaterials("media.geo"); // Materials

    TString macroName = gSystem->Getenv("VMCWORKDIR");
    macroName += "/macro/run/modules/registerSetup.C";
    std::cout << std::endl << "Loading macro " << macroName << std::endl;
    gROOT->LoadMacro(macroName);
    gROOT->ProcessLine("registerSetup()");


    std::cout << std::endl << "-I- " << myName << ": Registering target" << std::endl;
    CbmTarget* target = new CbmTarget(targetElement.Data(), targetThickness, targetDiameter);
    target->SetPosition(targetPosX, targetPosY, targetPosZ);
    target->SetRotation(targetRotY);
    target->Print();
    run->AddModule(target);

    // Magnetic field
    std::cout << std::endl << "-I- " << myName << ": Registering magnetic field" << std::endl;
    CbmFieldMap* magField = CbmSetup::Instance()->CreateFieldMap();
    if ( !magField ) {
        std::cout << "-ERROR- : No valid field!";
        return;
    }
    run->SetField(magField);

    //  PrimaryGenerator
    std::cout << std::endl << "-I- " << myName << ": Registering event generators" << std::endl;
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
    // --- Uniform distribution of event plane angle
    primGen->SetEventPlane(0., 2. * TMath::Pi());
    // --- Get target parameters
    Double_t tX = 0.;
    Double_t tY = 0.;
    Double_t tZ = 0.;
    Double_t tDz = 0.;
    if ( target ) {
        target->GetPosition(tX, tY, tZ);
        tDz = target->GetThickness();
    }
    primGen->SetTarget(tZ, tDz);
    primGen->SetBeam(0., 0., beamWidthX, beamWidthY);
    primGen->SmearGausVertexXY(smearVertexXY);
    primGen->SmearVertexZ(smearVertexZ);



    if (urqmd == "yes"){
        CbmUnigenGenerator*  urqmdGen = new CbmUnigenGenerator(urqmdFile);
        urqmdGen->SetEventPlane(0. , 360.);
        urqmdGen->SetEventPlane(0. , 360.);
        primGen->AddGenerator(urqmdGen);
    }

    //add electrons
    if (electrons == "yes"){
        FairBoxGenerator* boxGen1 = new FairBoxGenerator(11, NPOSITRONS);
        boxGen1->SetPtRange(0.,3.);
        boxGen1->SetPhiRange(0.,360.);
        boxGen1->SetThetaRange(2.5,25.);
        boxGen1->SetCosTheta();
        boxGen1->Init();
        primGen->AddGenerator(boxGen1);

        FairBoxGenerator* boxGen2 = new FairBoxGenerator(-11, NELECTRONS);
        boxGen2->SetPtRange(0.,3.);
        boxGen2->SetPhiRange(0.,360.);
        boxGen2->SetThetaRange(2.5,25.);
        boxGen2->SetCosTheta();
        boxGen2->Init();
        primGen->AddGenerator(boxGen2);
    }

    if (pluto == "yes") {
        CbmPlutoGenerator *plutoGen= new CbmPlutoGenerator(plutoFile);
        primGen->AddGenerator(plutoGen);
    }

    run->SetGenerator(primGen);

    std::cout << std::endl << "-I- " << myName << ": Initialise run" << std::endl;
    run->Init();



    if (pluto == "yes" && urqmd == "yes") {
        Float_t bratioEta[6];
        Int_t modeEta[6];

        TGeant3* gMC3 = (TGeant3*) gMC;

        for (Int_t kz = 0; kz < 6; ++kz) {
            bratioEta[kz] = 0.;
            modeEta[kz]   = 0;
        }

        Int_t ipa    = 17;
        bratioEta[0] = 39.38;  //2gamma
        bratioEta[1] = 32.20;  //3pi0
        bratioEta[2] = 22.70;  //pi+pi-pi0
        bratioEta[3] = 4.69;   //pi+pi-gamma
        bratioEta[4] = 0.60;   //e+e-gamma
        bratioEta[5] = 4.4e-2; //pi02gamma

        modeEta[0] = 101;    //2gamma
        modeEta[1] = 70707;  //3pi0
        modeEta[2] = 80907;  //pi+pi-pi0
        modeEta[3] = 80901;  //pi+pi-gamma
        modeEta[4] = 30201;  //e+e-gamma
        modeEta[5] = 10107;  //pi02gamma
        gMC3->Gsdk(ipa, bratioEta, modeEta);

        Float_t bratioPi0[6];
        Int_t modePi0[6];

        for (Int_t kz = 0; kz < 6; ++kz) {
            bratioPi0[kz] = 0.;
            modePi0[kz] = 0;
        }

        ipa = 7;
        bratioPi0[0] = 98.798;
        bratioPi0[1] = 1.198;

        modePi0[0] = 101;
        modePi0[1] = 30201;

        gMC3->Gsdk(ipa, bratioPi0, modePi0);

        Int_t t = time(NULL);
        TRandom *rnd = new TRandom(t);
        gMC->SetRandom(rnd);
    }



    // -----   Runtime database   ---------------------------------------------
    std::cout << std::endl << std::endl;
    std::cout << "-I- " << myName << ": Set runtime DB" << std::endl;
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    CbmFieldPar* fieldPar = (CbmFieldPar*) rtdb->getContainer("CbmFieldPar");
    fieldPar->SetParameters(magField);
    fieldPar->setChanged();
    fieldPar->setInputVersion(run->GetRunId(),1);
    Bool_t kParameterMerged = kTRUE;
    FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(parFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();
    rtdb->print();

    std::cout << std::endl << std::endl << "-I- " << myName << ": Starting run" << std::endl;
    run->Run(nEvents);

   // run->CreateGeometryFile(geoFile);

    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    std::cout << std::endl << std::endl;
    std::cout << "Macro finished successfully." << std::endl;
    std::cout << "Output file is "    << mcFile << std::endl;
    std::cout << "Parameter file is " << parFile << std::endl;
    std::cout << "Geometry file is "  << geoFile << std::endl;
    std::cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << std::endl << std::endl;
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
}


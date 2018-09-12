void run_sim_urqmdtest(Int_t nEvents = 5)
{
    TTree::SetMaxTreeSize(90000000000);

    TString script = TString(gSystem->Getenv("SCRIPT"));

    TString myName = "run_sim_urqmdtest";  // this macro's name for screen output
    TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory

    TString geoSetupFile = srcDir + "/macro/rich/geosetup/rich_setup_sis100.C";

    TString inFile = "/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root";
    TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/urqmdtest/";
    TString parFile =  outDir + "param.00000.root";
    TString mcFile = outDir + "mc.00000.root";
    TString geoFile = outDir + "geosim.00000.root";

    if (script == "yes") {
        inFile = TString(gSystem->Getenv("IN_FILE"));
        mcFile = TString(gSystem->Getenv("MC_FILE"));
        parFile = TString(gSystem->Getenv("PAR_FILE"));
        geoFile = TString(gSystem->Getenv("GEOSIM_FILE"));
        geoSetupFile = srcDir + TString(gSystem->Getenv("GEO_SETUP_FILE"));
    }
    remove(parFile.Data());
    remove(mcFile.Data());
    remove(geoFile.Data());

    // Target geometry
    TString  targetElement   = "Gold";
    Double_t targetThickness = 0.025;  // full thickness in cm
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
    // You can modify the pre-defined setup by using
    // CbmSetup::Instance()->RemoveModule(ESystemId) or
    // CbmSetup::Instance()->SetModule(ESystemId, const char*, Bool_t) or
    // CbmSetup::Instance()->SetActive(ESystemId, Bool_t)
    // See the class documentation of CbmSetup.


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
    primGen->SetEventPlane(0., 2. * TMath::Pi());
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

    CbmUnigenGenerator*  urqmdGen = new CbmUnigenGenerator(inFile);
    urqmdGen->SetEventPlane(0. , 360.);
    primGen->AddGenerator(urqmdGen);

    run->SetGenerator(primGen);
    
    std::cout << std::endl << "-I- " << myName << ": Initialise run" << std::endl;
    run->Init();

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


     run->CreateGeometryFile(geoFile);

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

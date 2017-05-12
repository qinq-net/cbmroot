static TString  fieldMap;
static Double_t fieldZ;
static Double_t fieldScale;


void run_sim(Int_t nEvents = 100, TString outDir = "")
{
    TTree::SetMaxTreeSize(90000000000);
    Int_t iVerbose = 0;

    TString script = TString(gSystem->Getenv("SCRIPT"));
	cout << "script: " << script << endl;

    // -----   In- and output file names   ------------------------------------
    TString setupName = "";
//    setupName = "setup_misalign_5mrad_01y_15x";
//    setupName = "setup_misalign_test";
    setupName = "setup_misalign_v17";

//    TString outDir = "";
    if (script == "yes") {
	outDir = TString(gSystem->Getenv("OUT_DIR"));
    }
    else {
//	outDir = "/data/Sim_Outputs/Correction_test/old_code/";
//	outDir = "/data/Sim_Outputs/Correction_test/new_code/";
	outDir = "/data/Sim_Outputs/Event_Display/";
    }
    TString parFile = outDir + setupName + "_param.root";
    TString mcFile = outDir + setupName + "_mc.root";
    TString geoFile = outDir + setupName + "_geofilefull.root";
    TString outFile = outDir + setupName + "_out.root";

    TString geoSetupFile = "";
    geoSetupFile = "/data/Cbmroot/Src_Cbm/macro/rich/alignment/misalignment_correction/test/geosetup/" + setupName + ".C";

    TString electrons = "yes"; // If "yes" then primary electrons will be generated
    Int_t NELECTRONS = 1; // number of e- to be generated
    Int_t NPOSITRONS = 1; // number of e+ to be generated
    TString urqmd = "no"; // If "yes" then UrQMD will be used as background
    TString urqmdFile = "/data/Cbmroot/Src_Cbm/input/urqmd.auau.10gev.centr.root";
    TString pluto = "no"; // If "yes" PLUTO particles will be embedded
    TString plutoFile = "";
    TString plutoParticle = "";
    // ------------------------------------------------------------------------


    // -----   Script initialization   ----------------------------------------
    if (script == "yes") {
        urqmdFile = TString(gSystem->Getenv("URQMD_FILE"));
        mcFile = TString(gSystem->Getenv("MC_FILE"));
        parFile = TString(gSystem->Getenv("PAR_FILE"));
	cout << "mcFile: " << TString(gSystem->Getenv("MC_FILE")) << endl << "parFile: " << TString(gSystem->Getenv("PAR_FILE")) << endl << "urqmdFile: " << TString(gSystem->Getenv("URQMD_FILE")) << endl;

	geoSetupFile = TString(gSystem->Getenv("VMCWORKDIR")) + "/macro/rich/matching/geosetup/" + TString(gSystem->Getenv("GEO_SETUP_FILE"));
	setupName = TString(gSystem->Getenv("SETUP_NAME"));

        NELECTRONS = TString(gSystem->Getenv("NELECTRONS")).Atoi();
        NPOSITRONS = TString(gSystem->Getenv("NPOSITRONS")).Atoi();
        electrons = TString(gSystem->Getenv("ELECTRONS"));
        urqmd = TString(gSystem->Getenv("URQMD"));
//        pluto = TString(gSystem->Getenv("PLUTO"));
//        plutoFile = TString(gSystem->Getenv("PLUTO_FILE"));
//        plutoParticle = TString(gSystem->Getenv("PLUTO_PARTICLE"));
    }

    std::cout << "-I- using geoSetupFile: " << geoSetupFile << " and setupName: "
	<< setupName << std::endl;

    remove(parFile.Data());
    remove(mcFile.Data());

    gDebug = 0;
    TStopwatch timer;
    timer.Start();


    // -----   Create simulation run   ----------------------------------------
    FairRunSim* fRun = new FairRunSim();
    fRun->SetName("TGeant3");                     // Transport engine
    fRun->SetOutputFile(mcFile);                  // Output file
    fRun->SetGenerateRunInfo(kTRUE);              // Create FairRunInfo file
    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    // ------------------------------------------------------------------------


    // -----   Logger settings   ----------------------------------------------
    //Logger settings
    TString logLevel = "INFO";   // "DEBUG";
    TString logVerbosity = "LOW";
    // ------------------------------------------------------------------------


    // -----   Load the geometry setup   --------------------------------------
    const char* setupName2 = setupName;
    TString setupFunct = "";
    setupFunct = setupFunct + setupName2 + "()";
    std::cout << "-I- geoSetupName: " << geoSetupFile << std::endl
	<< "-I- setupFunct: " << setupFunct << std::endl;
    gROOT->LoadMacro(geoSetupFile);
    gROOT->ProcessLine(setupFunct);
    std::cout << "Geometry initialized!" << std::endl;
    // ------------------------------------------------------------------------


    // creation of the primary vertex
    Bool_t smearVertexXY = kTRUE;
    Bool_t smearVertexZ  = kTRUE;
    Double_t beamWidthX   = 1.;  // Gaussian sigma of the beam profile in x [cm]
    Double_t beamWidthY   = 1.;  // Gaussian sigma of the beam profile in y [cm]
    // ------------------------------------------------------------------------


    // -----   Create media   -------------------------------------------------
    fRun->SetMaterials("media.geo"); // Materials
    // ------------------------------------------------------------------------

    // -----   Create and register modules   ----------------------------------
    std::cout << std::endl;
    //TString macroName = gSystem->Getenv("VMCWORKDIR");
    //macroName += "/macro/run/modules/registerSetup.C";
    TString macroName = "/data/Cbmroot/Src_Cbm/macro/run/modules/registerSetup.C";
    std::cout << "Loading macro " << macroName << std::endl;
    gROOT->LoadMacro(macroName);
    gROOT->ProcessLine("registerSetup()");
    // ------------------------------------------------------------------------


    // --- Define the target geometry -----------------------------------------
    //
    // The target is not part of the setup, since one and the same setup can
    // and will be used with different targets.
    // The target is constructed as a tube in z direction with the specified
    // diameter (in x and y) and thickness (in z). It will be placed at the
    // specified position as daughter volume of the volume present there. It is
    // in the responsibility of the user that no overlaps or extrusions are
    // created by the placement of the target.
    //
    TString  targetElement   = "Gold";
    Double_t targetThickness = 0.025; // 250 mum, full thickness in cm
    Double_t targetDiameter  = 2.5;    // diameter in cm
    Double_t targetPosX      = 0.;     // target x position in global c.s. [cm]
    Double_t targetPosY      = 0.;     // target y position in global c.s. [cm]
    Double_t targetPosZ      = 0.;     // target z position in global c.s. [cm]
    Double_t targetRotY      = 0.;     // target rotation angle around the y axis [deg]

    // -----   Create and register the target   -------------------------------
    CbmTarget* target = new CbmTarget(targetElement.Data(), targetThickness, targetDiameter);
    target->SetPosition(targetPosX, targetPosY, targetPosZ);
    target->SetRotation(targetRotY);
    target->Print();
    fRun->AddModule(target);
    // ------------------------------------------------------------------------


    // -----   Create magnetic field   ----------------------------------------
    Double_t fieldZ       = 40.;            // field centre z position
    Double_t fieldScale   =  1.;            // field scaling factor
    CbmFieldMap* magField = CbmSetup::Instance()->CreateFieldMap();
    if ( ! magField ) {
	std::cout << "-E- run_sim_new: No valid field!" << std::endl;
  	return;
    }
    magField->SetPosition(0., 0., fieldZ);
    magField->SetScale(fieldScale);
    fRun->SetField(magField);
    // ------------------------------------------------------------------------


    // -----   Create PrimaryGenerator   --------------------------------------
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
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

    if (urqmd == "yes") {
        CbmUnigenGenerator*  uniGen = new CbmUnigenGenerator(urqmdFile);
        uniGen->SetEventPlane(0. , 360.);
        primGen->AddGenerator(uniGen);
    }

    if (electrons == "yes") {
	Double_t phi0 = 120., theta0 = 10.1;		// Tile 1_4
	Double_t phi1 = 74., theta1 = 10.;		// Tile 1_5
//	Double_t phi2 = 171.51, theta2 = 24.25;		// Tile 0_1
	Double_t phi3 = 9.5, theta3 = 22.5;		// Tile 0_8


	Double_t Xi = -64.310094 + 20.9690586, Yi = 61.256446 - 0.1092312, Zi = 338.476947 + 4.3663151; // 5mrad Study.
	Double_t omega = TMath::ATan(-1*Xi/Yi);
	Double_t phi4 = 90 + omega*TMath::RadToDeg();
	Double_t oc = TMath::Sqrt(Xi*Xi + Yi*Yi);
	Double_t theta4 = TMath::ATan(oc/Zi)*TMath::RadToDeg();
//	cout << "theta4 & phi4: " << theta4 << ", " <<  phi4 << endl;
//	Double_t phi4 = , theta4 = ;

        FairBoxGenerator* boxGen1 = new FairBoxGenerator(-11, NELECTRONS);
        boxGen1->SetPRange(9., 9.5);
        //boxGen1->SetPtRange(0., 3.);
        boxGen1->SetPhiRange(phi0, phi0);
        boxGen1->SetThetaRange(theta0, theta0);
//        boxGen1->SetPhiRange(phi0 - 5, phi0 + 5);
//        boxGen1->SetThetaRange(theta0 - 2, theta0 + 2);
//        boxGen1->SetPhiRange(0.5, 179.5);
//        boxGen1->SetThetaRange(2.5, 25);
        boxGen1->SetCosTheta();
        boxGen1->Init();
        primGen->AddGenerator(boxGen1);

        FairBoxGenerator* boxGen1 = new FairBoxGenerator(-11, NPOSITRONS);
        boxGen1->SetPRange(9., 9.5);
        //boxGen1->SetPtRange(0., 3.);
        boxGen1->SetPhiRange(phi1, phi1);
        boxGen1->SetThetaRange(theta1, theta1);
//        boxGen1->SetPhiRange(phi1 - 3, phi1 + 3);
//        boxGen1->SetThetaRange(theta1 - 1, theta1 + 1);
//        boxGen1->SetPhiRange(0.5, 179.5);
//        boxGen1->SetThetaRange(2.5, 25);
        boxGen1->SetCosTheta();
        boxGen1->Init();
        primGen->AddGenerator(boxGen1);

/*        FairBoxGenerator* boxGen2 = new FairBoxGenerator(11, NPOSITRONS);
        boxGen2->SetPRange(9., 9.5);
        //boxGen2->SetPtRange(0., 3.);
        boxGen2->SetPhiRange(phi4, phi4);
        boxGen2->SetThetaRange(theta4, theta4);
//        boxGen2->SetPhiRange(phi4 - 1.5, phi4 + 1.5);
//        boxGen2->SetThetaRange(theta4 - 0.5, theta4 + 0.5);
//        boxGen2->SetPhiRange(0.5, 179.5);
//        boxGen2->SetThetaRange(2.5, 25);
        boxGen2->SetCosTheta();
        boxGen2->Init();
        primGen->AddGenerator(boxGen2);
*/
        FairBoxGenerator* boxGen1 = new FairBoxGenerator(-11, NELECTRONS);
        boxGen1->SetPRange(9., 9.5);
        //boxGen1->SetPtRange(0., 3.);
        boxGen1->SetPhiRange(phi3, phi3);
        boxGen1->SetThetaRange(theta3, theta3);
//        boxGen1->SetPhiRange(phi3 - 6, phi3 + 6);
//        boxGen1->SetThetaRange(theta3, theta3 + 1);
//        boxGen1->SetPhiRange(0.5, 179.5);
//        boxGen1->SetThetaRange(2.5, 25);
        boxGen1->SetCosTheta();
        boxGen1->Init();
        primGen->AddGenerator(boxGen1);

    }

        //      CbmLitPolarizedGenerator *polGen;
        //      polGen = new CbmLitPolarizedGenerator(443, NELECTRONS);
        //      polGen->SetDistributionPt(0.176);        // 25 GeV
        //      polGen->SetDistributionY(1.9875,0.228);  // 25 GeV
        //      polGen->SetRangePt(0.,3.);
        //      polGen->SetRangeY(1.,3.);
        //      polGen->SetBox(0);
        //      polGen->SetRefFrame(CbmLitPolarizedGenerator::kHelicity);
        //      polGen->SetDecayMode(CbmLitPolarizedGenerator::kDiElectron);
        //      polGen->SetAlpha(0);
        //      polGen->Init();
        //      primGen->AddGenerator(polGen);

    /*
    if (pluto == "yes") {
        CbmPlutoGenerator *plutoGen= new CbmPlutoGenerator(plutoFile);
        primGen->AddGenerator(plutoGen);
    }
    */
    // ------------------------------------------------------------------------


    // -----   Run initialisation   -------------------------------------------
    fRun->SetGenerator(primGen);
    fRun->SetStoreTraj(kTRUE);
    fRun->Init();
    // ------------------------------------------------------------------------


    // -----   Runtime database   ---------------------------------------------
    CbmFieldPar* fieldPar = (CbmFieldPar*) rtdb->getContainer("CbmFieldPar");
    fieldPar->SetParameters(magField);
    fieldPar->setChanged();
    fieldPar->setInputVersion(fRun->GetRunId(),1);
    Bool_t kParameterMerged = kTRUE;
    FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(parFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();
    rtdb->print();
    // ------------------------------------------------------------------------


    // -----   Start run   ----------------------------------------------------
    fRun->Run(nEvents);
    // ------------------------------------------------------------------------


    // -----   Finish   -------------------------------------------------------
    fRun->CreateGeometryFile(geoFile);
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is "    << mcFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;

    cout << " Test passed" << endl;
    cout << " All ok " << endl;
}

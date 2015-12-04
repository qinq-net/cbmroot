// --------------------------------------------------------------------------
//
// Macro for standard transport simulation using UrQMD input and GEANT3
// Standard CBM setup with MVD, STS, RICH, TRD, TOF and ECAL
//
// V. Friese   22/02/2007
//
// 2014-06-30 - DE - available setups from geometry/setup:
// 2014-06-30 - DE - sis100_hadron
// 2014-06-30 - DE - sis100_electron
// 2014-06-30 - DE - sis100_muon
// 2014-06-30 - DE - sis300_electron
// 2014-06-30 - DE - sis300_muon
//
// --------------------------------------------------------------------------


TString caveGeom="";
TString pipeGeom="";
TString magnetGeom="";
TString mvdGeom="";
TString stsGeom="";
TString richGeom="";
TString muchGeom="";
TString shieldGeom="";
TString trdGeom="";
TString tofGeom="";
TString ecalGeom="";
TString platformGeom="";
TString psdGeom="";
Double_t psdZpos=0.;
Double_t psdXpos=0.;

TString mvdTag="";
TString stsTag="";
TString trdTag="";
TString tofTag="";  

TString stsDigi="";
TString trdDigi="";
TString tofDigi="";

TString mvdMatBudget="";
TString stsMatBudget="";

TString  fieldMap="";
Double_t fieldZ=0.;
Double_t fieldScale=0.;
Int_t    fieldSymType=0;

TString defaultInputFile="";


void run_sim(Int_t nEvents = 2, Int_t mode = 0, Int_t file_nr = 1, const char* inputsetup = "", TString output = "")
{

  // ========================================================================
  //          Adjust this part according to your requirements

  // ----- Paths and file names  --------------------------------------------
  TString inDir   = gSystem->Getenv("VMCWORKDIR");
  

	Char_t filenr[5];
	sprintf(filenr,"%05d",file_nr);
	printf("Filenr: %s\n", filenr);
	TString temp = filenr;
	
  
	const char* setup;
	if(inputsetup == "") {
		setup = "sis100_electron";
	}
	else {
		setup = inputsetup;
	}
	
	inDir = "/common/cbma/users/reinecke/software/simu_apr14/cbmroot_jul14";
  TString setupFile = inDir + "/geometry/setup/" + setup + "_setup.C";
  TString setupFunct = setup;
  setupFunct += "_setup()";
  
  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);
  
  
	//TString inFile = "/common/cbmb/cbm/simulations/input/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr." + temp + ".root";
	TString inFile = "/common/cbmb/cbm/simulations/input/urqmd/auau/10gev/centr/urqmd.auau.10gev.centr." + temp + ".root";
	//TString inFile = "/hera/cbm/prod/gen/urqmd/auau/10gev/centr/urqmd.auau.10gev.centr." + temp + ".root";
	TString plutoFile = "/common/cbma/simulations/gen/pluto/cktA/25gev/omega/epem/pluto.auau.25gev.omega.epem.0001.root";
	TString parFile = "";
	TString outFile = "";
	TString geoFile = "";
	TString outDir  = "";

  
	// settings for simulation
	TString electrons = "no"; 		// If "yes" than primary electrons will be generated
	Int_t NELECTRONS = 5; 			// number of e- to be generated
	Int_t NPOSITRONS = 5; 			// number of e+ to be generated
	TString urqmd = "no"; 			// If "yes" then UrQMD will be used as background
	TString urqmd_addpi0 = "no";	// If "yes" then pi0 will be embeddid in UrQMD events
	Int_t NPI0 = 200;				// number of pi0 to be added to UrQMD sample
	TString pluto = "no"; 			// If "yes" PLUTO particles will be embedded
	TString tomography = "no"; 		// If "yes" gammas will be embedded with boxgenerator (for tomography)
	TString pi0only = "no";			// sample with only pi0
	TString plutoParticle = "";
	Bool_t smearTarget = kTRUE;


	TString outName;
	if(output == "") {
		outName = "sis300electron_test";
	}
	else {
		outName = output;
	}
	TString dirPrefix = "/common/cbmb/Users/reinecke/simulations/outputs/";
	if(mode == 1) {	// tomography
		TString dir = dirPrefix + "tomography/" + outName;
		outDir = dir;
		TString creator = "mkdir " + dir;
		cout << "Create new folder: " << dir << endl;
		status = system(creator.Data());
		cout << "Response of creating folder: " << status << endl;
		parFile = dir + "/tomography." + outName + ".param.root";
		outFile = dir + "/tomography." + outName + ".mc.root";
		geoFile = dir + "/tomography." + outName + ".geofile.root";
		urqmd = "no";
		tomography = "yes";
	}
	if(mode == 2) {	// urqmd
		int status;
	//	TString dir = "/common/home/reinecke/CBM-Simulationen/outputs/urqmd/" + outName;
		TString dir = dirPrefix + "urqmd/" + outName;
		outDir = dir;
		TString creator = "mkdir " + dir;
		cout << "Create new folder: " << dir << endl;
		status = system(creator.Data());
		cout << "Response of creating folder: " << status << endl;
		parFile = dir + "/urqmd." + outName + ".param." + temp + ".root";
		outFile = dir + "/urqmd." + outName + ".mc." + temp + ".root";
		geoFile = dir + "/urqmd." + outName + ".geofile." + temp + ".root";
		urqmd = "yes";
		urqmd_addpi0 = "yes";
		tomography = "no";
	}
	if(mode == 3) {	// pluto
		parFile = dirPrefix + "pluto." + outName + ".param.0001.root";
		outFile = dirPrefix + "pluto." + outName + ".mc.0001.root";
		urqmd = "no";
		tomography = "no";
		pluto = "yes";
	}
	if(mode == 4) {	// only pi0 for acceptance calculations
		int status;
	//	TString dir = "/common/home/reinecke/CBM-Simulationen/outputs/urqmd/" + outName;
		TString dir = dirPrefix + "pi0only/" + outName;
		outDir = dir;
		TString creator = "mkdir " + dir;
		cout << "Create new folder: " << dir << endl;
		status = system(creator.Data());
		cout << "Response of creating folder: " << status << endl;
		parFile = dir + "/pi0only." + outName + ".param." + temp + ".root";
		outFile = dir + "/pi0only." + outName + ".mc." + temp + ".root";
		geoFile = dir + "/pi0only." + outName + ".geofile." + temp + ".root";
		urqmd = "no";
		urqmd_addpi0 = "no";
		tomography = "no";
		pi0only = "yes";
	}

	if(urqmd == "no" && tomography == "no" && pluto == "no" && pi0only == "no") {
		cout << "No input type selected! Exiting..." << endl;
		exit();
	}
  
  cout << "run_sim.C script: folder created!" << endl;
  
  // Function needed for CTest runtime dependency
  //TString depFile = Remove_CTest_Dependency_File(outDir, "run_sim" , setup);
  

  // --- Logger settings ----------------------------------------------------
  TString logLevel = "INFO";   // "DEBUG";
  TString logVerbosity = "LOW";
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
  Double_t targetThickness = 0.025;  // full thickness in cm
  Double_t targetDiameter  = 2.5;    // diameter in cm
  Double_t targetPosX      = 0.;     // target x position in global c.s. [cm]
  Double_t targetPosY      = 0.;     // target y position in global c.s. [cm]
  Double_t targetPosZ      = 0.;     // target z position in global c.s. [cm]
  Double_t targetRotY      = 0.;     // target rotation angle around the y axis [deg]
  // ------------------------------------------------------------------------


  // --- Define the creation of the primary vertex   ------------------------
  //
  // By default, the primary vertex point is sampled from a Gaussian
  // distribution in both x and y with the specified beam profile width,
  // and from a flat distribution in z over the extension of the target.
  // By setting the respective flags to kFALSE, the primary vertex will always
  // at the (0., 0.) in x and y and in the z centre of the target, respectively.
  //
  Bool_t smearVertexXY = kTRUE;
  Bool_t smearVertexZ  = kTRUE;
  Double_t beamWidthX   = 1.;  // Gaussian sigma of the beam profile in x [cm]
  Double_t beamWidthY   = 1.;  // Gaussian sigma of the beam profile in y [cm]
  // ------------------------------------------------------------------------
  

  // In general, the following parts need not be touched
  // ========================================================================

  cout << "[INFO   ] Setup: " << setup << endl;

  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------



  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  // -----   Create simulation run   ----------------------------------------
  FairRunSim* fRun = new FairRunSim();
  fRun->SetName("TGeant3");              // Transport engine
  fRun->SetOutputFile(outFile);          // Output file
  fRun->SetGenerateRunInfo(kTRUE);       // Create FairRunInfo file
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  // ------------------------------------------------------------------------


  // -----   Logger settings   ----------------------------------------------
  gLogger->SetLogScreenLevel(logLevel.Data());
  gLogger->SetLogVerbosityLevel(logVerbosity.Data());
  // ------------------------------------------------------------------------




  // -----   Create media   -------------------------------------------------
  fRun->SetMaterials("media.geo");       // Materials
  // ------------------------------------------------------------------------


  // -----   Create detectors and passive volumes   -------------------------
  if ( caveGeom != "" ) {
    FairModule* cave = new CbmCave("CAVE");
    cave->SetGeometryFileName(caveGeom);
    fRun->AddModule(cave);
  }

  if ( pipeGeom != "" ) {
    FairModule* pipe = new CbmPipe("PIPE");
    pipe->SetGeometryFileName(pipeGeom);
    fRun->AddModule(pipe);
  }
  
  // --- Target
  CbmTarget* target = new CbmTarget(targetElement.Data(),
  		                              targetThickness,
  		                              targetDiameter);
  target->SetPosition(targetPosX, targetPosY, targetPosZ);
  target->SetRotation(targetRotY);
  fRun->AddModule(target);

  if ( magnetGeom != "" ) {
    FairModule* magnet = new CbmMagnet("MAGNET");
    magnet->SetGeometryFileName(magnetGeom);
    fRun->AddModule(magnet);
  }
  
  if ( platformGeom != "" ) {
    FairModule* platform = new CbmPlatform("PLATFORM");
    platform->SetGeometryFileName(platformGeom);
    fRun->AddModule(platform);
  }

  if ( mvdGeom != "" ) {
    FairDetector* mvd = new CbmMvd("MVD", kTRUE);
    mvd->SetGeometryFileName(mvdGeom);
    mvd->SetMotherVolume("pipevac1");
    fRun->AddModule(mvd);
  }

  if ( stsGeom != "" ) {
    FairDetector* sts = new CbmStsMC(kTRUE);
    sts->SetGeometryFileName(stsGeom);
    fRun->AddModule(sts);
  }

  if ( richGeom != "" ) {
    FairDetector* rich = new CbmRich("RICH", kTRUE);
    rich->SetGeometryFileName(richGeom);
    fRun->AddModule(rich);
  }
  
  if ( muchGeom != "" ) {
    FairDetector* much = new CbmMuch("MUCH", kTRUE);
    much->SetGeometryFileName(muchGeom);
    fRun->AddModule(much);
  }
  
  if ( shieldGeom != "" ) {
  	FairModule* shield = new CbmShield("SHIELD");
  	shield->SetGeometryFileName(shieldGeom);
  	fRun->AddModule(shield);
  }

  if ( trdGeom != "" ) {
    FairDetector* trd = new CbmTrd("TRD",kTRUE );
    trd->SetGeometryFileName(trdGeom);
    fRun->AddModule(trd);
  }

  if ( tofGeom != "" ) {
    FairDetector* tof = new CbmTof("TOF", kTRUE);
    tof->SetGeometryFileName(tofGeom);
    fRun->AddModule(tof);
  }
  
  if ( ecalGeom != "" ) {
    FairDetector* ecal = new CbmEcal("ECAL", kTRUE, ecalGeom.Data()); 
    fRun->AddModule(ecal);
  }
  
  if ( psdGeom != "" ) {
  	cout << "Constructing PSD" << endl;
  	CbmPsdv1* psd= new CbmPsdv1("PSD", kTRUE);  
 	psd->SetZposition(psdZpos); // in cm
 	psd->SetXshift(psdXpos);  // in cm    
 	psd->SetGeoFile(psdGeom);  
	fRun->AddModule(psd);
  }
  
  // ------------------------------------------------------------------------

  // -----   Create magnetic field   ----------------------------------------
  if ( 2 == fieldSymType ) {
    CbmFieldMap* magField = new CbmFieldMapSym2(fieldMap);
  }  else if ( 3 == fieldSymType ) {
    CbmFieldMap* magField = new CbmFieldMapSym3(fieldMap);
  } 
  magField->SetPosition(0., 0., fieldZ);
  magField->SetScale(fieldScale);
  fRun->SetField(magField);
  // ------------------------------------------------------------------------

  // Use the experiment specific MC Event header instead of the default one
  // This one stores additional information about the reaction plane
  CbmMCEventHeader* mcHeader = new CbmMCEventHeader();
  fRun->SetMCEventHeader(mcHeader);

  // -----   Create PrimaryGenerator   --------------------------------------
  FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
  // Use the CbmUnigenGenrator which calculates a reaction plane and
  // rotate all particles accordingly

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
		//CbmUrqmdGenerator*  urqmdGen = new CbmUrqmdGenerator(inFile);
		CbmUnigenGenerator*  urqmdGen = new CbmUnigenGenerator(inFile);
		urqmdGen->SetEventPlane(0. , 360.);
		primGen->AddGenerator(urqmdGen);
	}

	if (urqmd == "yes" && urqmd_addpi0 == "yes"){
		FairBoxGenerator* boxGen1 = new FairBoxGenerator(111, NPI0);
		boxGen1->SetPtRange(0.,3.);
		boxGen1->SetPhiRange(0.,360.);
		boxGen1->SetThetaRange(0.,40.);
		boxGen1->SetCosTheta();
		boxGen1->Init();
		primGen->AddGenerator(boxGen1);
	}

	// generation of pure pi0 sample
	if (pi0only == "yes"){
		FairBoxGenerator* boxGen1 = new FairBoxGenerator(111, NPI0);
		boxGen1->SetPtRange(0.,6.);
		boxGen1->SetPhiRange(0.,360.);
		//boxGen1->SetThetaRange(0.,45.);		// set theta range (not at the same time as SetYRange() )
		boxGen1->SetYRange(0., 6.);				// set Y range (not at the same time as SetThetaRange() )
		//boxGen1->SetCosTheta();				// set distribution of theta values (standard = flat)
		boxGen1->Init();
		primGen->AddGenerator(boxGen1);
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
   
	if (tomography == "yes") {
		FairBoxGenerator* boxGen1 = new FairBoxGenerator(22, 300); //22 = gammas, nof = 300
		boxGen1->SetPtRange(0.,3.);
		boxGen1->SetPhiRange(0.,360.);
	//	boxGen1->SetThetaRange(2.5,25.);	// normal acceptance
		boxGen1->SetThetaRange(0.,45.);	// increased acceptance to "illuminate" the whole detector
		boxGen1->SetCosTheta();
		boxGen1->Init();
		primGen->AddGenerator(boxGen1);	
	}

	if (pluto == "yes") {
		FairPlutoGenerator *plutoGen= new FairPlutoGenerator(plutoFile);
		primGen->AddGenerator(plutoGen);
	}
	
	
	fRun->SetGenerator(primGen);       
  // ------------------------------------------------------------------------

 
  // Trajectories Visualization (TGeoManager Only)
  // Switch this on if you want to visualize tracks in the
  // eventdisplay.
  // This is normally switch off, because of the huge files created
  // when it is switched on. 
  Bool_t storeTraj = kFALSE;
  fRun->SetStoreTraj(storeTraj);

  // -----   Run initialisation   -------------------------------------------
  fRun->Init();
  // ------------------------------------------------------------------------
  
  // Set cuts for storing the trajectories.
  // Switch this on only if trajectories are stored.
  // Choose this cuts according to your needs, but be aware
  // that the file size of the output file depends on these cuts

	if(storeTraj) {
   FairTrajFilter* trajFilter = FairTrajFilter::Instance();
   trajFilter->SetStepSizeCut(0.01); // 1 cm
   trajFilter->SetVertexCut(-2000., -2000., 4., 2000., 2000., 100.);
   trajFilter->SetMomentumCutP(10e-3); // p_lab > 10 MeV
   trajFilter->SetEnergyCut(0., 1.02); // 0 < Etot < 1.04 GeV
   trajFilter->SetStorePrimaries(kTRUE);
   trajFilter->SetStoreSecondaries(kTRUE);
}

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
  fRun->CreateGeometryFile(geoFile);

  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is "    << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime 
       << "s" << endl << endl;
  // ------------------------------------------------------------------------

  cout << " Test passed" << endl;
  cout << " All ok " << endl;

  // Function needed for CTest runtime dependency
  //Generate_CTest_Dependency_File(depFile);
  
  
	Int_t analyseMode = mode;
	ofstream outputfile("log.txt", std::ofstream::app);
	if(!outputfile) {
		cout << "Error!" << endl;
	}
	else {
		TTimeStamp testtime;
		outputfile << "########## run_sim.C ##########" << endl;
		outputfile << "Date (of end): " << testtime.GetDate() << "\t Time (of end): " << testtime.GetTime() << " +2" << endl;
		outputfile << "Output file is "    << outFile << endl;
		outputfile << "Parameter file is " << parFile << endl;
		outputfile << "Number of events: " << nEvents << "\t mode: " << analyseMode << endl;
		outputfile << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
		outputfile.close();
	}
	
	
	// copy geometry setup file
	cout << "Copying setup file..." << endl;
	ifstream inputGeometry(setupFile, std::ifstream::in);
	ifstream outputGeometry(dir + setup + "_setup.C", mode = ios_base::out);
	if (!inputGeometry)
		cout << "Eingabe-Datei kann nicht geöffnet werden\n";
	else {
		if(!outputGeometry.good()) {
		outputGeometry.close();
		ofstream outputGeometry2(dir + "/" + setup + "_setup.C", mode = ios_base::out);
		if (!outputGeometry2)
			cout << "Ausgabe-Datei kann nicht geöffnet werden\n"; 
		else {
			cout << "Writing copy of setup file..." << endl;
			char c;
			while (inputGeometry.get(c)) {
				outputGeometry2.put(c);
			}
		}
		}
	}
}


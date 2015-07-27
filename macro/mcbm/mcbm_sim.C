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

void mcbm_sim(Int_t nEvents = 1, const char* setup = "sis18_mcbm")
{

  // ========================================================================
  //          Adjust this part according to your requirements

  // ----- Paths and file names  --------------------------------------------
  TString inDir   = gSystem->Getenv("VMCWORKDIR");

  TString outDir  = "data/";
  TString outFile = outDir + setup + "_test.mc.root";
  TString parFile = outDir + setup + "_params.root";
  TString geoFile = outDir + setup + "_geofile_full.root";

  TString setupFile = inDir + "/geometry/setup/" + setup + "_setup.C";
  TString setupFunct = setup;
  setupFunct += "_setup()";
  
  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);

  TString inFile  = inDir + defaultInputFile;

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
  
  CbmTarget* target = new CbmTarget("Gold", 0.25, 2.5);
  target->SetRotation(90.); // 30.);   // set target at 30 degrees rotation angle around y
  if ( target ) fRun->AddModule(target);

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
    FairDetector* much = new CbmMuchMcbm("MUCH", kTRUE);
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
//  if ( 2 == fieldSymType ) {
//    CbmFieldMap* magField = new CbmFieldMapSym2(fieldMap);
//  }  else if ( 3 == fieldSymType ) {
//    CbmFieldMap* magField = new CbmFieldMapSym3(fieldMap);
//  } 
//  magField->SetPosition(0., 0., fieldZ);
//  magField->SetScale(fieldScale);
  FairConstField* magField = new FairConstField();
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
  CbmUnigenGenerator*  uniGen = new CbmUnigenGenerator(inFile);
  uniGen->SetEventPlane(0. , 360.);
  primGen->AddGenerator(uniGen);

  primGen->SetBeamAngle(30*3.141/180.,0,0,0);  // set direction of beam to 30 degrees

  fRun->SetGenerator(primGen);       
  // ------------------------------------------------------------------------

//  // -----   Create Electron gun as alternative -----------------------------
//  FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
//  // Use the FairBoxGenerator which generates a soingle electron
//  FairBoxGenerator *eminus = new FairBoxGenerator();
//  eminus->SetPDGType(11);
//  eminus->SetMultiplicity(1000);
//  //  eminus->SetBoxXYZ(32.,-32.,32.,-32.,0.);  // shoot at corner of diagonal modules
//  //  eminus->SetBoxXYZ(0., 0., 0., 0., 0.);  // shoot at corner of diagonal modules
//  //  eminus->SetBoxXYZ(57.,-57., 0., 0.,0.);  // shoot at corner of diagonal modules
//  //  eminus->SetBoxXYZ(-57.,-57., 57., 57.,0.);  // shoot at corner of diagonal modules
//  eminus->SetBoxXYZ(-180.,-15.,-150.,15.,0.);  // shoot at corner of diagonal modules
//  eminus->SetPRange(2.,2.);
//  eminus->SetPhiRange(0.,360.);
//  eminus->SetThetaRange(0.,0.);
//  primGen->AddGenerator(eminus);
//
//  //  primGen->SetBeamAngle(30*3.141/180.,0,0,0);  // set direction of beam to 30 degrees
//
//  fRun->SetGenerator(primGen);
//  // ------------------------------------------------------------------------

 
  // Trajectories Visualization (TGeoManager Only)
  // Switch this on if you want to visualize tracks in the
  // eventdisplay.
  // This is normally switch off, because of the huge files created
  // when it is switched on. 
  fRun->SetStoreTraj(kTRUE);

  // -----   Run initialisation   -------------------------------------------
  fRun->Init();
  // ------------------------------------------------------------------------
  
  // Set cuts for storing the trajectories.
  // Switch this on only if trajectories are stored.
  // Choose this cuts according to your needs, but be aware
  // that the file size of the output file depends on these cuts

   FairTrajFilter* trajFilter = FairTrajFilter::Instance();
   trajFilter->SetStepSizeCut(0.01); // 1 cm
   trajFilter->SetVertexCut(-2000., -2000., 4., 2000., 2000., 100.);
   trajFilter->SetMomentumCutP(10e-3); // p_lab > 10 MeV
   trajFilter->SetEnergyCut(0., 1.02); // 0 < Etot < 1.04 GeV
   trajFilter->SetStorePrimaries(kTRUE);
   trajFilter->SetStoreSecondaries(kTRUE);

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
}


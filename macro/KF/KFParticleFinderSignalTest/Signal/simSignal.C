//---------------------------------------------------------------------------------
// @author  M. Zyzak
// @version 1.0
// @since   15.08.14
// 
// macro to simulate signal events for KFParticleFinder
//_________________________________________________________________________________

void simSignal(Int_t iParticle = 0, Int_t nEvents = 10000, bool useMvd = 0)
{
  // ========================================================================
  //          Adjust this part according to your requirements
  
  //For charm particles MVD should be on
  if(iParticle >= 50 && iParticle<=67)
    useMvd = 1;
  
  const bool sameZ = 0;
  const bool usePipe = 1;
  const int  nElectrons = 0;
  
  // Output name
  TString outName = "signal";

  // Output file name
  TString outFile = outName + ".mc.root";

  // Parameter file name
  TString parFile = outName +".params.root";

    // -----  Geometries  -----------------------------------------------------
  TString caveGeom   = "cave.geo";
  TString pipeGeom   = "pipe/pipe_v14l.geo.root"; //===>>>
  TString magnetGeom = "magnet/magnet_v12b.geo.root";
  TString mvdGeom    = "mvd/mvd_v14b.geo.root";

  TString stsGeom;
  if (sameZ) stsGeom    = "sts_same_z.geo";
  else stsGeom    = "sts/sts_v15c.geo.root";

  if (!useMvd) mvdGeom = "";
  if (!usePipe) pipeGeom = "";
  TString richGeom = "";
  TString trdGeom  = "";
  TString tofGeom = "tof/tof_v16a_1h.geo.root";
  TString ecalGeom = "";

  // -----   Magnetic field   -----------------------------------------------
  TString fieldMap    = "field_v12b";   // name of field map
  Double_t fieldZ     = 40.;             // field centre z position
  Double_t fieldScale =  1.;             // field scaling factor
  
  // -----   Input file name   ----------------------------------------------  
  TString inFile = "Signal.txt";

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

  CbmTarget* target = new CbmTarget("Gold", 0.025);
  fRun->AddModule(target);
  
  if ( magnetGeom != "" ) {
    FairModule* magnet = new CbmMagnet("MAGNET");
    magnet->SetGeometryFileName(magnetGeom);
    fRun->AddModule(magnet);
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

  // -----   Create magnetic field   ----------------------------------------
  CbmFieldMap* magField = new CbmFieldMapSym3(fieldMap);
  magField->SetPosition(0., 0., fieldZ);
  magField->SetScale(fieldScale);
  fRun->SetField(magField);
  // ------------------------------------------------------------------------

  // -----   Create PrimaryGenerator   --------------------------------------
  FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
  FairAsciiGenerator*  asciiGen = new FairAsciiGenerator(inFile);
  primGen->AddGenerator(asciiGen);

  fRun->SetGenerator(primGen);
  
  KFPartEfficiencies eff;
  for(int jParticle=eff.fFirstStableParticleIndex+10; jParticle<=eff.fLastStableParticleIndex; jParticle++)
  {
    TDatabasePDG* pdgDB = TDatabasePDG::Instance();

    if(!pdgDB->GetParticle(eff.partPDG[jParticle])){
        pdgDB->AddParticle(eff.partTitle[jParticle].data(),eff.partTitle[jParticle].data(), eff.partMass[jParticle], kTRUE,
                           0, eff.partCharge[jParticle]*3,"Ion",eff.partPDG[jParticle]);
    }
  }
    
  Double_t lifetime = eff.partLifeTime[iParticle]; // lifetime
  Double_t mass = eff.partMass[iParticle];
  Int_t PDG = eff.partPDG[iParticle];
  Double_t charge = eff.partCharge[iParticle];
  
  if(iParticle == 52 || iParticle == 53)
  {
    for(int iPall=52; iPall<54; iPall++)
    {
      Double_t lifetime = eff.partLifeTime[iPall]; // lifetime
      Double_t mass = eff.partMass[iPall];
      Int_t PDG = eff.partPDG[iPall];
      Double_t charge = eff.partCharge[iPall];
    
      FairParticle* newParticle = new FairParticle(PDG, eff.partTitle[iPall].data(), kPTHadron, mass, charge,
              lifetime, "hadron", 0.0, 1, 1, 0, 1, 1, 0, 0, 1, kFALSE);
      fRun->AddNewParticle(newParticle);
    }
    TString pythia6Config = "/u/mzyzak/cbmtrunk/macro/KF/KFParticleFinderSignalTest/Signal/DecayConfig.C()";
    fRun->SetPythiaDecayer(pythia6Config);
  }

  fRun->Init();
  
  if(!(iParticle == 52 || iParticle == 53))
  {
    gMC->DefineParticle(PDG, eff.partTitle[iParticle].data(), kPTHadron, mass, charge,
            lifetime, "hadron", 0.0, 1, 1, 0, 1, 1, 0, 0, 1, kFALSE);
    
    Int_t mode[6][3];
    Float_t bratio[6];

    for (Int_t kz = 0; kz < 6; kz++) {
      bratio[kz] = 0.;
      mode[kz][0] = 0;
      mode[kz][1] = 0;
      mode[kz][2] = 0;
    }
    bratio[0] = 100.;
    for(int iD=0; iD<eff.GetNDaughters(iParticle); iD++)
    {
      if(iD>2)
        continue;
      mode[0][iD] = eff.GetDaughterPDG(iParticle, iD); //pi+
    }
    
    gMC->SetDecayMode(PDG,bratio,mode);
  }
  for(int iP=eff.fFirstHypernucleusIndex; iP<=eff.fLastHypernucleusIndex; iP++)
  {
    Double_t lifetime = eff.partLifeTime[iP]; // lifetime
    Double_t mass = eff.partMass[iP];
    Int_t PDG = eff.partPDG[iP];
    Double_t charge = eff.partCharge[iP];
   
    gMC->DefineParticle(PDG, eff.partTitle[iP].data(), kPTHadron, mass, charge,
            lifetime, "hadron", 0.0, 1, 1, 0, 1, 1, 0, 0, 1, kFALSE);
    
    Int_t mode[6][3];
    Float_t bratio[6];

    for (Int_t kz = 0; kz < 6; kz++) {
      bratio[kz] = 0.;
      mode[kz][0] = 0;
      mode[kz][1] = 0;
      mode[kz][2] = 0;
    }
    bratio[0] = 100.;
    for(int iD=0; iD<eff.GetNDaughters(iP); iD++)
    {
      if(iD>2)
        continue;
      mode[0][iD] = eff.GetDaughterPDG(iP, iD); //pi+
    }
    
    gMC->SetDecayMode(PDG,bratio,mode);
  }
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
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is "    << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime 
       << "s" << endl << endl;
}


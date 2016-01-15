/* Macro used to simulate the passage of particles inside a given detector geometry. */

void old_run_sim(int nEvents = 100, int geom_nb = 0)
{
    
  // int nElectrons=1;
  // bool TakePipe=0, TakeTarget=0, TakeMagnet=1, TakeSTS=0;
  TTree::SetMaxTreeSize(90000000000);

  TString pipeGeom   = "pipe/pipe_standard.geo";
  //pipeGeom   = "pipe/pipe_standard_Shift20cm.geo"; //new geometry always shifted.
  char PATH1[256];
  char PATH2[256];
  char PATH3[256];
  char PATH4[256];

  //sprintf(PATH1, "/home/jordan/Downloads/RICH_Grid.gdml");
  //sprintf(PATH1, "rich/mirror_alignment/rich_v14b_misaligned_%d.gdml", geom_nb);
  //sprintf(PATH1, "rich/rich_v14b_misaligned_%d.gdml", geom_nb);
  sprintf(PATH1, "rich/misalignment_correction/rich_v14b_misaligned_TwoMirr.root");
  TString richGeom = PATH1;

  cout << endl << "Geometry File:" << endl << endl;
  cout << richGeom << endl << endl;
  cout << "++++++++++++++++++++++++++++++++++++++++++++" << endl << endl;

  TString outdir = "/home/jordan/Desktop/misalignment_correction/";
  sprintf(PATH2, "Parameters_RichGeo_jordan_%d.root", geom_nb);
  sprintf(PATH3, "Sim_RichGeo_jordan_%d.root", geom_nb);
  sprintf(PATH4, "OutPutGeo_RichGeo_jordan_%d.root", geom_nb);

  TString ParFile = outdir + PATH2;
  TString SimFile = outdir + PATH3;
  TString OutPutGeoFile = outdir + PATH4;

  cout << "Output Directories:" << endl << endl;
  cout << ParFile << endl;
  cout << SimFile << endl;
  cout << endl << "++++++++++++++++++++++++++++++++++++++++++++" << endl << endl;

  TString caveGeom = "cave.geo";
  TString targetGeom = "target/target_au_250mu.geo";
  //TString magnetGeom = "magnet/magnet_v12a.geo";
  TString magnetGeom = "magnet/magnet_v12b.geo.root";
  TString stsGeom = "sts/sts_v13d.geo.root";
  TString fieldMap = "field_v12a";
  TString richDetectorType = "standard"; // "standard" or "prototype"
  Double_t fieldZ = 50.; // field center z position
  Double_t fieldScale =  1.0; // field scaling factor

  TStopwatch timer;
  timer.Start();
  gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
  loadlibs();

  // Initialize simulation object fRun

  FairRunSim* fRun = new FairRunSim();
  fRun->SetName("TGeant3"); // Transport engine
  fRun->SetOutputFile(SimFile);
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();

  fRun->SetMaterials("media.geo"); // Set Materials from file

  // Add modules to simulation object

  FairModule* cave = new CbmCave("CAVE");
  cave->SetGeometryFileName(caveGeom);
  fRun->AddModule(cave);

  FairModule* pipe = new CbmPipe("PIPE");
  pipe->SetGeometryFileName(pipeGeom);
  fRun->AddModule(pipe);

  FairModule* magnet = new CbmMagnet("MAGNET");
  magnet->SetGeometryFileName(magnetGeom);
  fRun->AddModule(magnet);

  FairDetector* rich = NULL;
  rich = new CbmRich("RICH", kTRUE);
  rich->SetGeometryFileName(richGeom);
  fRun->AddModule(rich);

  FairDetector* sts = NULL;
  sts = new CbmStsMC(kTRUE);
  sts->SetGeometryFileName(stsGeom);
  fRun->AddModule(sts);

  CbmFieldMap* magField = NULL;

  magField = new CbmFieldMapSym2(fieldMap);
  magField->SetPosition(0., 0., fieldZ);
  magField->SetScale(fieldScale);
  fRun->SetField(magField);

  // Initialize particles generator (here generation of e-/e+ pair)

  FairPrimaryGenerator* primGen = new FairPrimaryGenerator();

  //float StartPhi=0., EndPhi=180.;		//StartPhi=90.1
  //float StartTheta=2.5, EndTheta=25.;

  float StartPhi=129.9, EndPhi=139.5;         // StartPhi=141.98, EndPhi=142.;      Phi = azimuthal angle
  float StartTheta=13.5, EndTheta=15.25;       // StartTheta=12.72, EndTheta=12.74;  Theta = polar angle

  // e- Generator
  FairBoxGenerator* boxGen1 = new FairBoxGenerator(11, 1);	//Pdg code and number of particles produced.
  boxGen1->SetPtRange(1.,10.);			//Energy range between 3 and 10 GeV.
  boxGen1->SetPhiRange(StartPhi,EndPhi);	//(0.,360.);
  boxGen1->SetThetaRange(StartTheta,EndTheta);	//(2.5,25.);
  boxGen1->SetCosTheta();
  boxGen1->Init();
  primGen->AddGenerator(boxGen1);

  // e+ Generator
  /*FairBoxGenerator* boxGen2 = new FairBoxGenerator(-11, 1);
  boxGen2->SetPtRange(3.,10.);
  boxGen2->SetPhiRange(StartPhi,EndPhi);	//(0.,360.);
  boxGen2->SetThetaRange(StartTheta,EndTheta);	//(2.5,25.);
  boxGen2->SetCosTheta();
  boxGen2->Init();
  primGen->AddGenerator(boxGen2);*/

  // Generate pi+
  /*     FairBoxGenerator* boxGen1 = new FairBoxGenerator(211, 1);
	 boxGen1->SetPtRange(0.,3.);
	 boxGen1->SetPhiRange(0.,360.);
	 boxGen1->SetThetaRange(2.5,25.);
	 boxGen1->SetCosTheta();
	 boxGen1->Init();
	 primGen->AddGenerator(boxGen1);
  // Generate pi-
	 FairBoxGenerator* boxGen2 = new FairBoxGenerator(-211, 1);
	 boxGen2->SetPtRange(0.,3.);
	 boxGen2->SetPhiRange(0.,360.);
	 boxGen2->SetThetaRange(2.5,25.);
	 boxGen2->SetCosTheta();
	 boxGen2->Init();
	 primGen->AddGenerator(boxGen2);
  */

  fRun->SetGenerator(primGen);
  //fRun->SetStoreTraj(kTRUE);
  fRun->Init();
    
  // FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  // trajFilter->SetStepSizeCut(0.01); // 1 cm
  // trajFilter->SetVertexCut(-2000., -2000., -2000., 2000., 2000., 2000.);
  // trajFilter->SetMomentumCutP(0.); // p_lab > 0
  // trajFilter->SetEnergyCut(0., 10.); // 0 < Etot < 10 GeV
  // trajFilter->SetStorePrimaries(kTRUE);//kFALSE);//kTRUE);
  // trajFilter->SetStoreSecondaries(kTRUE);//kFALSE);
 

  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
  parOut->open(ParFile.Data());
  rtdb->setOutput(parOut);
  rtdb->saveOutput();
  rtdb->print();
  
  fRun->Run(nEvents);
  fRun->CreateGeometryFile(OutPutGeoFile);

  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is "    << SimFile << endl;
  cout << "Parameter file is " << ParFile << endl;
  cout << "Real time = " << rtime << " s, CPU time = " << ctime << "s" << endl << endl;
  cout << "run_sim of "<< nEvents <<" events finished successfully." << endl;
  cout << "All ok." << endl;
}

void run_sim(Int_t nEvents = 500)
{
    TTree::SetMaxTreeSize(90000000000);
    TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
    
	TString script = TString(gSystem->Getenv("SCRIPT"));
    //gRandom->SetSeed(10);

	//TString asciiInput = "/data/cbm/cbmroot/macro/rich/prototype/smallprototype/beamdistribution.ascii.dat";
   //	TString parFile = "/data/cbm/Gregor/param.01001.root";
   //	TString geoFile = "/data/cbm/Gregor/geofilefull.01001.root";
   // TString mcFile = "/data/cbm/Gregor/mc.01001.root";
   
    TString asciiInput = "/Users/slebedev/Development/cbm/trunk/cbmroot/macro/rich/prototype/smallprototype/beamdistribution.ascii.dat";
    TString parFile = "/Users/slebedev/Development/cbm/data/simulations/richprototype/param.00001.root";
    TString geoFile = "/Users/slebedev/Development/cbm/data/simulations/richprototype/geofilefull.00001.root";
    TString mcFile = "/Users/slebedev/Development/cbm/data/simulations/richprototype/mc.00001.root";
    
	if (script == "yes") {
      asciiInput = TString(gSystem->Getenv("IN_ASCII_FILE"));
      outFile = TString(gSystem->Getenv("MC_FILE"));
      parFile = TString(gSystem->Getenv("PAR_FILE"));
      caveGeom = TString(gSystem->Getenv("CAVE_GEOM"));
      richGeom = TString(gSystem->Getenv("RICH_GEOM"));
   }
    

	TString caveGeom = "cave.geo";
	TString richGeom = "rich/prototype/Testbox.geo.root";
    
    remove(parFile.Data());
    remove(mcFile.Data());
    
    gDebug = 0;
    TStopwatch timer;
    timer.Start();
    
    gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
    loadlibs();
   
    
    FairRunSim* fRun = new FairRunSim();
    fRun->SetName("TGeant3");
    fRun->SetOutputFile(mcFile);
    fRun->SetGenerateRunInfo(kTRUE);
    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    
   // fRun->SetMaterials("media.geo"); // Materials
    fRun->SetMaterials("media_rich_prototype.geo"); // Materials
    
    if ( caveGeom != "" ) {
        FairModule* cave = new CbmCave("CAVE");
        cave->SetGeometryFileName(caveGeom);
        fRun->AddModule(cave);
    }
    
    if ( richGeom != "" ) {
        FairDetector* rich = new CbmRich("RICH", kTRUE);
        rich->SetGeometryFileName(richGeom);
        fRun->AddModule(rich);
    }
   
    
    // -----   Create PrimaryGenerator   --------------------------------------
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
    Double_t tX = 0.;
    Double_t tY = 0.;
    Double_t tZ = 0.;
    Double_t tDz = 0.;
   // if ( target ) {
     //   target->GetPosition(tX, tY, tZ);
       // tDz = target->GetThickness();
    //}
    //primGen->SetTarget(tZ, tDz);
    //primGen->SetBeam(0., 0., beamWidthX, beamWidthY);
    //primGen->SmearGausVertexXY(smearVertexXY);
    //primGen->SmearVertexZ(smearVertexZ);
    
   	     FairBoxGenerator* boxGen1 = new FairBoxGenerator(2212, 1);
        boxGen1->SetPRange(2.,2.);
        boxGen1->SetPhiRange(0.,360.);
        boxGen1->SetThetaRange(0., 0.);
		boxGen1->SetXYZ(0.,0.,-30.);
	//	boxGen1->SetBoxXYZ(-1., -1., 1., 1., -30.);
        boxGen1->Init();
		primGen->AddGenerator(boxGen1);
	
	/*	FairAsciiGenerator* asciiGen = new FairAsciiGenerator(asciiInput);	
   		primGen->AddGenerator(asciiGen);
     */   
    
    fRun->SetGenerator(primGen);
    fRun->SetStoreTraj(true);
    
    fRun->Init();
    
    // -----   Runtime database   ---------------------------------------------
    CbmFieldPar* fieldPar = (CbmFieldPar*) rtdb->getContainer("CbmFieldPar");
   /// fieldPar->SetParameters(magField);
    fieldPar->setChanged();
    fieldPar->setInputVersion(fRun->GetRunId(),1);
    Bool_t kParameterMerged = kTRUE;
    FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(parFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();
    rtdb->print();
    
    fRun->Run(nEvents);
    
    fRun->CreateGeometryFile(geoFile);
    
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is "    << mcFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime
    << "s" << endl << endl;
    
    cout << " Test passed" << endl;
    cout << " All ok " << endl;
}


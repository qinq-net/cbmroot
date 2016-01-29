void run_sim_urqmdtest(Int_t nEvents = 20)
{
    TTree::SetMaxTreeSize(90000000000);
    
    
    TString script = TString(gSystem->Getenv("SCRIPT"));
    TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
    
    TString inFile = "/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root";
    TString outDir = "/Users/slebedev/Development/cbm/data/simulations/rich/urqmdtest/";
    TString parFile =  outDir + "25gev.centr.param.root";
    TString outFile = outDir + "25gev.centr.mc.root";
    
    TString caveGeom = "cave.geo";
    TString pipeGeom   = "pipe/pipe_v14n.root";
    TString magnetGeom = "magnet/magnet_v15a.geo.root";
    TString stsGeom = "sts/sts_v15c.geo.root";
    TString richGeom= "rich/Rich_jan2016.gdml";//"rich/Rich_dec_2015_ver2.gdml";
    TString fieldMap = "field_v12b";
    Double_t fieldZ = 40.; // field center z position
    Double_t fieldScale =  1.0; // field scaling factor
    Int_t fieldSymType =  3;
    
    if (script == "yes") {
        inFile = TString(gSystem->Getenv("IN_FILE"));
        outFile = TString(gSystem->Getenv("MC_FILE"));
        parFile = TString(gSystem->Getenv("PAR_FILE"));
        caveGeom = TString(gSystem->Getenv("CAVE_GEOM"));
        pipeGeom = TString(gSystem->Getenv("PIPE_GEOM"));
        stsGeom = TString(gSystem->Getenv("STS_GEOM"));
        richGeom = TString(gSystem->Getenv("RICH_GEOM"));
        fieldMap = TString(gSystem->Getenv("FIELD_MAP"));
        magnetGeom = TString(gSystem->Getenv("MAGNET_GEOM"));
        fieldScale = TString(gSystem->Getenv("FIELD_MAP_SCALE")).Atof();
    }
    remove(parFile.Data());
    remove(outFile.Data());
    
    gDebug = 0;
    TStopwatch timer;
    timer.Start();
    
    gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
    loadlibs();
    
    FairRunSim* fRun = new FairRunSim();
    fRun->SetName("TGeant3"); // Transport engine
    fRun->SetOutputFile(outFile);
    FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
    
    fRun->SetMaterials("media.geo"); // Materials
    
    //Target geometry
    TString  targetElement   = "Gold";
    Double_t targetThickness = 0.025; // 250 mum, full thickness in cm
    Double_t targetDiameter  = 2.5;    // diameter in cm
    Double_t targetPosX      = 0.;     // target x position in global c.s. [cm]
    Double_t targetPosY      = 0.;     // target y position in global c.s. [cm]
    Double_t targetPosZ      = 0.;     // target z position in global c.s. [cm]
    Double_t targetRotY      = 0.;     // target rotation angle around the y axis [deg]
    
    // creation of the primary vertex
    Bool_t smearVertexXY = kTRUE;
    Bool_t smearVertexZ  = kTRUE;
    Double_t beamWidthX   = 1.;  // Gaussian sigma of the beam profile in x [cm]
    Double_t beamWidthY   = 1.;  // Gaussian sigma of the beam profile in y [cm]
    // ------------------------------------------------------------------------
    
    
    if ( caveGeom != "" ) {
        FairModule* cave = new CbmCave("CAVE");
        cave->SetGeometryFileName(caveGeom);
        fRun->AddModule(cave);
    }
    
    if ( pipeGeom != "") {
        FairModule* pipe = new CbmPipe("PIPE");
        pipe->SetGeometryFileName(pipeGeom);
        fRun->AddModule(pipe);
    }
    
    CbmTarget* target = new CbmTarget(targetElement.Data(), targetThickness, targetDiameter);
    target->SetPosition(targetPosX, targetPosY, targetPosZ);
    target->SetRotation(targetRotY);
    fRun->AddModule(target);
    
    if ( magnetGeom != "") {
        FairModule* magnet = new CbmMagnet("MAGNET");
        magnet->SetGeometryFileName(magnetGeom);
        fRun->AddModule(magnet);
    }
    
    if ( stsGeom != "") {
        FairDetector* sts = new CbmStsMC(kTRUE);
        sts->SetGeometryFileName(stsGeom);
        fRun->AddModule(sts);
    }
    
    if ( richGeom != "") {
        FairDetector* rich = new CbmRich("RICH", kTRUE);
        rich->SetGeometryFileName(richGeom);
        fRun->AddModule(rich);
    }
    
    /*if ( trdGeom != "" ) {
     FairDetector* trd = new CbmTrd("TRD",kTRUE );
     trd->SetGeometryFileName(trdGeom);
     fRun->AddModule(trd);
     }*/
    
    // Create magnetic field
    cout <<"fieldSymType=" << fieldSymType << endl;
    CbmFieldMap* magField = NULL;
    if ( 2 == fieldSymType ) {
        CbmFieldMap* magField = new CbmFieldMapSym2(fieldMap);
    }  else if ( 3 == fieldSymType ) {
        CbmFieldMap* magField = new CbmFieldMapSym3(fieldMap);
    }
    magField->SetPosition(0., 0., fieldZ);
    magField->SetScale(fieldScale);
    fRun->SetField(magField);
    
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
    
    CbmUnigenGenerator*  urqmdGen = new CbmUnigenGenerator(inFile);
    urqmdGen->SetEventPlane(0. , 360.);
    //FairUrqmdGenerator* urqmdGen = new FairUrqmdGenerator(inFile);
    primGen->AddGenerator(urqmdGen);
    
    fRun->SetGenerator(primGen);
    fRun->Init();
    
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
    
    fRun->Run(nEvents);
    
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    cout << endl << endl;
    cout << "Macro finished succesfully." << endl;
    cout << "Output file is "    << outFile << endl;
    cout << "Parameter file is " << parFile << endl;
    cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
    cout << " Test passed" << endl;
    cout << " All ok " << endl;
}

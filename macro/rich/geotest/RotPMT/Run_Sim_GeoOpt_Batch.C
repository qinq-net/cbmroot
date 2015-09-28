void Run_Sim_GeoOpt_Batch(Int_t nEvents = 1,  float PMTrotX=5, float PMTrotY=5, int RotMir=-10)
{

  int GeoCase=2;
  int PtNotP=1;  float MomMin=0.; float MomMax=4.;
  //int PtNotP=0;  float MomMin=3.95; float MomMax=4.;
  float StartTheta=2.5; float  EndTheta=25.;
  int PMTtransY=0, PMTtransZ=0;
  int DefaultDims=0;
  int DefaultDims_LargePMT=0;
   bool StoreTraj=0;

  float StartPhi=90., EndPhi=180.;
  TString script = TString(gSystem->Getenv("SCRIPT"));
  if (script == "yes"){
    cout<<" ----------------- running with script --------------------"<<endl;
    nEvents=TString(gSystem->Getenv("N_EVS")).Atof();
    RotMir=TString(gSystem->Getenv("ROTMIR")).Atof();
    PMTrotX=TString(gSystem->Getenv("PMT_ROTX")).Atof();
    PMTrotY=TString(gSystem->Getenv("PMT_ROTY")).Atof();
    PMTtransY=TString(gSystem->Getenv("PMT_TRAY")).Atof();
    PMTtransZ=TString(gSystem->Getenv("PMT_TRAZ")).Atof();
    
    GeoCase=TString(gSystem->Getenv("GEO_CASE")).Atof();
    
    PtNotP=TString(gSystem->Getenv("PT_NOT_P")).Atof();
    MomMin=TString(gSystem->Getenv("MOM_MIN")).Atof();
    MomMax=TString(gSystem->Getenv("MOM_MAX")).Atof();
    EndTheta=TString(gSystem->Getenv("THETA")).Atof();
    StartPhi=TString(gSystem->Getenv("STARTPHI")).Atof();
    EndPhi=TString(gSystem->Getenv("ENDPHI")).Atof();
    DefaultDims=TString(gSystem->Getenv("DEFAULDIMS")).Atof();
    DefaultDims_LargePMT=TString(gSystem->Getenv("DEFAULDIMSLPMT")).Atof();
  }  
  //cout<<RotMir<<", "<<PMTrotX<<", "<<PMTrotY<<", "<<GeoCase<<endl;
  // float  EndTheta=35.;
    
  TTree::SetMaxTreeSize(90000000000);
  //******************************
  TString RotMirText=GetMirText(RotMir);
  TString PMTRotText=GetPMTRotText(PMTrotX, PMTrotY);
  TString PMTTransText=GetPMTTransText(PMTtransY, PMTtransZ);
  TString pipeGeom=GetPipe_GeoFile( GeoCase);
  pipeGeom="";
  //******************************

  cout<<"RotMirText = "<<RotMirText<<endl;
  cout<<"PMTRotText = "<<PMTRotText<<endl;

  TString outDir=GetOutDir(GeoCase);//="/data/GeoOpt/RotPMT/NewGeo/";
  TString GeoText=GetGeoText(GeoCase);
  TString MomText=GetMomText(PtNotP,MomMin,MomMax);
  TString PhiText=GetPhiText(StartPhi, EndPhi);
  TString ThetaText=GetThetaText(EndTheta);
  TString ExtraText=".";//
  if(DefaultDims ==1){
    ExtraText="_DefaultRichDims.";
    if(DefaultDims_LargePMT ==1){ExtraText="_DefaultRichDims_LargePMT.";}
  }

  TString richGeom=GetRICH_GeoFile( RotMirText, PMTRotText, PMTTransText, GeoCase, ExtraText);
  cout<<"rich geo = "<<richGeom<<endl;

  TString NamingText=GeoText+"_"+RotMirText+"_"+PMTRotText+"_"+PMTTransText+"_"+MomText+"_"+ThetaText+ExtraText+"root";
  //  TString NamingText=GeoText+"_"+RotMirText+"_"+PMTRotText+"_"+PMTTransText+"_"+MomText+"_"+ThetaText+PhiText+ExtraText+"root";
  TString ParFile = outDir + "Parameters_"+NamingText;//
  TString SimFile = outDir + "Sim_"+NamingText;
  TString OutPutGeoFile = outDir + "OutPutGeo_"+NamingText;
  gSystem->Exec( ("rm "+ParFile).Data() );
  gSystem->Exec( ("rm "+SimFile).Data() );
  gSystem->Exec( ("rm "+OutPutGeoFile).Data() );

  cout<<"par: "<<ParFile<<endl;
  cout<<"sim: "<<SimFile<<endl;
  cout<<"++++++++++++++++++++++++++++++++++++++++++++"<<endl; 
  //return;
  TString caveGeom = "cave.geo";
  TString magnetGeom = "magnet/magnet_v12b.geo.root";
  TString magnetGeom = "";
  TString stsGeom = "";
  TString fieldMap = "field_v12a";
  Double_t fieldZ = 50.; // field center z position
  Double_t fieldScale =  1.0; // field scaling factor
  
  ///////////////////////////////////////////////////
  gDebug = 0;
  TStopwatch timer;
  timer.Start();
  cout<<" going to load libraries"<<endl;
  
  gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
  loadlibs();
  cout<<" got libraries"<<endl;
  
  FairRunSim* fRun = new FairRunSim();
  fRun->SetName("TGeant3"); // Transport engine
  fRun->SetOutputFile(SimFile);
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  
  fRun->SetMaterials("media.geo"); // Materials
  
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
    //FairDetector* rich = new CbmRich("RICH", kTRUE);
    CbmRich* rich = new CbmRich("RICH", kTRUE);
    rich->SetGeometryFileName(richGeom);
    rich->SetRegisterPhotonsOnSensitivePlane(kTRUE); // Cerenkov photons are also registered in the sim tree
    fRun->AddModule(rich);
  }
  
  CbmFieldMap* magField = new CbmFieldMapSym2(fieldMap);
  magField->SetPosition(0., 0., fieldZ);
  magField->SetScale(fieldScale);
  fRun->SetField(magField);
  
  FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
  
  // e+/-
  FairBoxGenerator* boxGen1 = new FairBoxGenerator(11, 1);
  if(PtNotP==1){boxGen1->SetPtRange(MomMin,MomMax); }
  else{boxGen1->SetPRange(MomMin,MomMax); }
  // boxGen1->SetPRange(0.,10.);
  //boxGen1->SetPtRange(0.,4.);
  boxGen1->SetPhiRange(StartPhi,EndPhi);//0.,360.);
  boxGen1->SetThetaRange(StartTheta,EndTheta);//2.5,25.);
  boxGen1->SetCosTheta();
  boxGen1->Init();
  primGen->AddGenerator(boxGen1);
  
  FairBoxGenerator* boxGen2 = new FairBoxGenerator(-11, 1);
  if(PtNotP==1){boxGen2->SetPtRange(MomMin,MomMax); }
  else{boxGen2->SetPRange(MomMin,MomMax); }
  // boxGen2->SetPtRange(0.,4.);
  // boxGen2->SetPRange(0.,10.);
  boxGen2->SetPhiRange(StartPhi,EndPhi);//0.,360.);
  boxGen2->SetThetaRange(StartTheta,EndTheta);//2.5,25.);
  boxGen2->SetCosTheta();
  boxGen2->Init();
  primGen->AddGenerator(boxGen2);
  
  
  fRun->SetGenerator(primGen);
  if(StoreTraj){fRun->SetStoreTraj(kTRUE);}
  fRun->Init();
     if(StoreTraj){
    FairTrajFilter* trajFilter = FairTrajFilter::Instance();
    trajFilter->SetStepSizeCut(0.01); // 1 cm
    trajFilter->SetVertexCut(-2000., -2000., -2000., 2000., 2000., 2000.);
    trajFilter->SetMomentumCutP(0.); // p_lab > 0
    trajFilter->SetEnergyCut(0., 10.); // 0 < Etot < 10 GeV
    trajFilter->SetStorePrimaries(kTRUE);//kFALSE);//kTRUE);
  }
  CbmFieldPar* fieldPar = (CbmFieldPar*) rtdb->getContainer("CbmFieldPar");
  fieldPar->SetParameters(magField);
  fieldPar->setChanged();
  fieldPar->setInputVersion(fRun->GetRunId(),1);
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
  cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << endl << endl;
  cout << " Test passed" << endl;
  cout << " All ok " << endl;
  cout << "====================================================" << endl;
  cout << "=================== Test Overlaps ==================" << endl;
  gGeoManager->CheckOverlaps(0.001);
  gGeoManager->PrintOverlaps();
  TObjArray *overlapArray = gGeoManager->GetListOfOverlaps();
  Int_t numOverlaps = overlapArray->GetEntries();
  if ( numOverlaps != 0 ) {
    cout << "=================== Test failed =============="<< endl;
    cout << " We have in total "<<numOverlaps<<" overlaps."<<endl;
  }

  cout << "=================== Overlaps Tested ================" << endl;
  cout << "====================================================" << endl;

}
////////////////////////////////////////////
TString GetMomText(int PtNotP, float MomMin, float MomMax){
  TString Pstring="P"; if(PtNotP==1){Pstring="Pt";}
  if( (MomMax-MomMin)<0.1){}
  char Ptxt[256];
  
  if( (MomMax-MomMin)<0.1){sprintf(Ptxt,"%sFixed%d",Pstring.Data(),MomMax);}
  else{sprintf(Ptxt,"%s%dto%d",Pstring.Data(),MomMin,MomMax);}
  stringstream ss; 
  ss<<Ptxt;
  return ss.str();
}

////////////////////////////////////////////
TString GetGeoText(int GeoCase){
  //GeoCase=-2 ==> old geometry with rich_v08a.geo (RICH starts at 1600, Mirror tilt -1)
  //GeoCase=-1 ==> old geometry with rich_v14a.gdml (RICH starts at 1800, Mirror tilt -1)
  //GeoCase=0 ==> old geometry with *.geo (own creation)(RICH starts at 1600, Mirror tilt -1)
  //GeoCase=1 ==> gdml-geo: RICH starts at 1800, Mirror tilt -1 or 10, 
  //                        mirror does NOT cover full acceptance)
  //GeoCase=2 ==> gdml-geo: RICH starts at 1800, Mirror tilt -1 or 10, 
  //                        mirror does cover full acceptance)

 if(GeoCase==-2){return "RichGeo_v08a";}
  if(GeoCase==-1){return "RichGeo_v14a";}
  if(GeoCase==0){return "RichGeo_ascii";}
  if(GeoCase==1){return "RichGeo_OldGdml";}
  if(GeoCase==2){return "RichGeo_NewGdml";}
}
////////////////////////////////////////////
TString GetOutDir(int GeoCase){
  //return "/data/GeoOpt/";

  return "/hera/cbm/users/tariq/GeoOptRootFiles/";
  // if(GeoCase<=0){return "/data/GeoOpt/RotPMT/OlderGeo/";}
  // if(GeoCase==1){return "/data/GeoOpt/RotPMT/OldGeo/";}
  // if(GeoCase==2){return "/data/GeoOpt/RotPMT/NewGeo/";}
}
////////////////////////////////////////////
TString GetMirText(int RotMir){
  char RotMir_txt[256];
  if(RotMir<0){sprintf( RotMir_txt,"RotMir_m%d",RotMir*-1);}
  else{sprintf(RotMir_txt,"RotMir_p%d",RotMir);}

  stringstream ss; 
  ss<<RotMir_txt;
  return ss.str();
}
////////////////////////////////////////////////////////
TString  GetPMTRotText(float PMTrotX, float PMTrotY){
  int ShiftXmod10=(int(PMTrotX*10)) % 10;
  int ShiftYmod10=(int(PMTrotY*10)) % 10;
  float IntegerXValue=PMTrotX-(float (ShiftXmod10))/10.;
  float IntegerYValue=PMTrotY-(float (ShiftYmod10))/10.;
  
  char ShiftXTxt[256]; char ShiftYTxt[256];  
  sprintf(ShiftXTxt,"Xpos%dpoint%d",IntegerXValue,ShiftXmod10);
  sprintf(ShiftYTxt,"Ypos%dpoint%d",IntegerYValue,ShiftYmod10);
  if(PMTrotY<0){sprintf(ShiftYTxt,"Yneg%dpoint%d",-1.*IntegerYValue,-1.*ShiftYmod10);}
  
  stringstream ss; 
  ss<<"RotPMT_"<<ShiftXTxt<<"_"<<ShiftYTxt;
  return ss.str();
}
///////////////////////////////////////////////
TString  GetPMTTransText(int PMTTransY, int PMTTransZ){
  char ZTransText[256]; char YTransText[256];  
  if(PMTTransY < 0) {sprintf( YTransText,"Y_m%d",-1*PMTTransY);}
  else{sprintf( YTransText,"Y_p%d",PMTTransY);}
  if(PMTTransZ < 0) {sprintf( ZTransText,"Z_m%d",-1*PMTTransZ);}
  else{sprintf( ZTransText,"Z_p%d",PMTTransZ);}
  stringstream ss; 
  ss<<"TransPMT_"<<YTransText<<"_"<<ZTransText;
  return ss.str();
}

////////////////////////////////////////////////////////
TString GetRICH_GeoFile( char *RotMirText, TString PMTRotText, TString PMTTransText, int GeoCase, TString ExtraText){
  //GeoCase=-2 ==> old geometry with rich_v08a.geo (RICH starts at 1600, Mirror tilt -1)
  //GeoCase=-1 ==> old geometry with rich_v14a.gdml (RICH starts at 1800, Mirror tilt -1)
  if(GeoCase==-2){return "rich/rich_v08a.geo";}
  if(GeoCase==-1){return "rich/rich_v14a.root";}
  //GeoCase=0 ==> old geometry with *.geo (own creation)(RICH starts at 1600, Mirror tilt -1)
  //GeoCase=1 ==> gdml-geo: RICH starts at 1800, Mirror tilt -1 or 10, 
  //                        mirror does NOT cover full acceptance)
  //GeoCase=2 ==> gdml-geo: RICH starts at 1800, Mirror tilt -1 or 10, 
  //                        mirror does cover full acceptance)

  //  return "/hera/cbm/users/tariq/cbmroot/geometry/rich/rich_v14a.root";
// GeoOpt/2015_minus10deg_.gdml";
  // return "/data/cbmroot/macro/rich/geotest/RotPMT/CreateGeo/RichGeo_NominalDimensions_minus10deg_07122014.gdml";
  // return "rich/GeoOpt/rich_geo_RotMir_m10_RotPMT_Xpos5point0_Ypos5point0_TransPMT_Y_p0_Z_p0_New.root";
  //TString Dir="rich/GeoOpt/RotPMT/";
  TString Dir="rich/GeoOpt";

  TString Dir2="/NewGeo/";
  TString Endung="root";
  if(GeoCase==0){Dir2="/OldGeo/"; Endung=".geo";}
  if(GeoCase==1){Dir2="/OldGeo/";}
  if(GeoCase==2){Dir2="/";}
  stringstream ss; 
  //ss<<Dir<<Dir2<<"rich_geo_"<<RotMirText<<"_"<<PMTRotText<<Endung;
  ss<<Dir<<Dir2<<"rich_geo_"<<RotMirText<<"_"<<PMTRotText<<"_"<<PMTTransText<<ExtraText<<Endung;

  return ss.str();
}
////////////////////////////////////////////////////////
TString GetThetaText( float theta=25.){
  char THtxt[256];
  sprintf(THtxt,"Theta_%d",theta);
  stringstream ss; 
  ss<<THtxt;
  return ss.str();
}
////////////////////////////////////////////////////////
TString GetPhiText(float StartPhi, float EndPhi){
  char PHtxt[256];
  sprintf(PHtxt,"Phi_%d_to_%d",StartPhi, EndPhi);
  stringstream ss; 
  ss<<PHtxt;
  return ss.str();
}
////////////////////////////////////////////////////////
TString GetPipe_GeoFile(int GeoCase){
  if(GeoCase == -2 || GeoCase == 0){return "pipe/pipe_standard.geo";}
  else{return "pipe/pipe_v14h.root";}
}

/*
  with old geo files (rotx=5, and roty=5) Notice that the actual rotx is 6.2 !!
  
PMT position in (x,y,z) [cm]: 51  172.525  225.928
Tilted PMT position in (x,y,z) [cm]: 31.115  196.327  209.516
PMT size in x and y [cm]: 50  30
PMT was tilted around x by 6.20664 degrees
PMT was tilted around y by -5 degrees
Refractive index for lowest photon energies: 1.00045, (n-1)*10000: 4.46242
Mirror center (x,y,z): 0 140.394 54.0827
Mirror radius: 300
Mirror tilting angle: -10 degrees
*/

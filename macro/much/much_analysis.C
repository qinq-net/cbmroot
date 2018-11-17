/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Analysis macro for MUCH									       //
//									                               //
//      Authors: Partha Pratim Bhaduri (partha.bhaduri@vecc.gov.in), Omveer Singh (omveer.rs@amu.ac.in)//
//      and Ekata Nandy (ekata@vecc.gov.in)			                                       //
//												       //
//												       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void much_analysis(int nEvents=100, Int_t nSig=1){
  TString dir;
  
  dir= "data/";
  TString mcFile   =dir + "mc.root";
  TString parFile =  dir + "params.root"; 
  TString recFile  = dir + "reco.root";
  TString outFile  = "data/much_analysis.root";

  TString parDir = TString(gSystem->Getenv("VMCWORKDIR")) + TString("/parameters");
   TString digiFile = parDir + "/much/much_v17b_digi_sector.root";// much digi file

  TTree::SetMaxTreeSize(90000000000);
  TString script = TString(gSystem->Getenv("LIT_SCRIPT"));

  
  TString stsDigiFile = parDir + "/sts/sts_v15a_std.digi.par"; // STS digi file
  TString stsMatBudgetFile = parDir + "/sts/sts_matbudget_v15a.root";


  gSystem->Load("libboost_regex");
  gSystem->Load("libCbmFlibReader");
  gSystem->Load("libGeoBase");
  gSystem->Load("libParBase");
  gSystem->Load("libCbmMuchAnalysis");
  gSystem->Load("libCbmDilept");
  gSystem->Load("libCbmBase");
  gSystem->Load("libCbmData");
  gSystem->Load("libCbmGenerators");
  gSystem->Load("libCbmField");
  gSystem->Load("libGen");
  gSystem->Load("libCbmPassive");
  gSystem->Load("libTrkBase");
  gSystem->Load("libGeane");
  gSystem->Load("libfles_ipc");
  gSystem->Load("libCbmSts");
  gSystem->Load("libCbmTrd");
  gSystem->Load("libCbmEcal");
  gSystem->Load("libPythia6");
  gSystem->Load("libKF");
  gSystem->Load("libL1");
  gSystem->Load("libCbmMvd");

  gSystem->Load("libFairTools");
 
  gSystem->Load("libboost_regex");
  gSystem->Load("libGen");


  gSystem->Load("libLittrackparallel");
  gSystem->Load("libLittrack");

  gSystem->Load("libCbmRich");
  gSystem->Load("libCbmMuch");
  gSystem->Load("libCbmTof");
  gSystem->Load("libCbmGlobal");

  gSystem->Load("libMinuit2");

 
  FairRunAna *fRun= new FairRunAna();
  fRun->SetInputFile(mcFile);
  fRun->AddFriend(recFile);
  fRun->SetOutputFile(outFile);

  TString muchDigiFile = gSystem->Getenv("VMCWORKDIR");
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo*  parIo1 = new FairParRootFileIo();
  parIo1->open(parFile);
  rtdb->setFirstInput(parIo1);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();

  // ------------------------------------------------------------------------

  CbmKF* kf = new CbmKF();
  CbmL1* L1 = new CbmL1();
  
  L1->SetStsMaterialBudgetFileName(stsMatBudgetFile.Data()); 


    CbmAnaDimuonAnalysis* ana = new CbmAnaDimuonAnalysis(digiFile,nSig);
    //CbmAnaDimuonAnalysis* ana = new CbmAnaDimuonAnalysis(); 
    //ana->SetChi2MuChCut(100);
    //ana->SetChi2VertexCut(3.5);
    //ana->SetNofMuchCut(0);
    //ana->SetNofStsCut(7);


  ana->SetVerbose(0);
  ana->SetStsPointsAccQuota(4);
  ana->SetStsTrueHitQuota(0.7);

  fRun->AddTask(kf);
  fRun->AddTask(L1);
  fRun->AddTask(ana);
  fRun->Init();
  fRun->Run(0,nEvents);

  cout<<"tested "<<endl;
  cout<<"all ok"<<endl;
}



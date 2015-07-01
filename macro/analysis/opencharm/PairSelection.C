void PairSelection(Int_t index = 0)
{
  // -------------------------------------------------------------------------
    // Convert index to the string
    char strIndex[4];
    sprintf(strIndex, "%4d", index);
    for(Int_t i = 0; i < 4; i++) {
	if(' ' == strIndex[i]) strIndex[i] = '0';
    }
  // -------------------------------------------------------------------------

// -------------------------------------------------------------------------
    TStopwatch timer;
    timer.Start();

    gDebug=0;

    gROOT->LoadMacro("$VMCWORKDIR/gconfig/basiclibs.C");
    basiclibs();
// -------------------------------------------------------------------------
    // Input files
 TString input = "auau.25gev"; 
 TString system = "centr"; 
 TString signal = "d0";
 Int_t  nEvents = 1000;
 Int_t  iVerbose = 0; 
 const char* setup = "sis300_electron"; 
 bool PileUp = false;
 bool littrack = false;
 Bool_t useMC = kTRUE;
   
  // Input file (MC events)
  TString mcFile = "data/opencharm.mc.unigen." + input + "." + system +"." + signal + ".root";
  // Output file
  TString rcSystem = "data/opencharm.reco.unigen." + input + "." + system + "." + signal;
 
  if(!PileUp)
    {
    if(littrack)
       TString rcFile = rcSystem + ".littrack.root";
    else 
       TString rcFile = rcSystem + ".l1.root";
    }
    else if(littrack)
       TString rcFile = rcSystem + ".PileUp.littrack.root";
    else 
       TString rcFile = rcSystem + ".PileUp.l1.root";
  
    
      // Output file
  TString trackSystem = "data/opencharm.tracks.unigen." + input + "." + system +"."+ signal;
 if(!PileUp)
    {
    if(littrack)
       TString trackFile = trackSystem + ".littrack.root";
    else 
       TString trackFile = trackSystem + ".l1.root";
    }
    else if(littrack)
       TString trackFile = trackSystem + ".PileUp.littrack.root";
    else 
       TString trackFile = trackSystem + ".PileUp.l1.root";
    
    
 TString outFile = "data/opencharm.pairs.unigen." + input + "." + system +"." + signal;
if(useMC) outFile = outFile + ".mcMode";
          outFile = outFile + ".root";
   
    cout << endl
	<< "  mcFile  :   " << mcFile  << endl
	<< "  rcFile  :   " << rcFile  << endl
        << "  outFile    :   " << outFile << endl;
	
	
  // Parameter file
  TString parFile = "data/paramsunigen.";
  parFile = parFile + input;
  parFile = parFile + ".";
  parFile = parFile + system;
  parFile = parFile + ".";
  parFile = parFile + signal;
  parFile = parFile + ".root";
// -------------------------------------------------------------------------	
//  Digitisation files.
  // Add TObjectString containing the different file names to
  // a TList which is passed as input to the FairParAsciiFileIo.
  // The FairParAsciiFileIo will take care to create on the fly 
  // a concatenated input parameter file which is then used during
  // the reconstruction.
  TList *parFileList = new TList();

  TString inDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = inDir + "/parameters/";

  TString setupFile = inDir + "/geometry/setup/" + setup + "_setup.C";
  TString setupFunct = setup;
  setupFunct += "_setup()";

  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);

  TObjString stsDigiFile = paramDir + stsDigi;
  parFileList->Add(&stsDigiFile);
  cout << "macro/run/run_reco.C using: " << stsDigi << endl;
// -------------------------------------------------------------------------

// ------------------------------------------------------------------------

    FairRunAna *fRun = new FairRunAna();
    fRun->SetInputFile(mcFile);
    fRun->AddFriend(rcFile);
    fRun->AddFriend(trackFile);
    fRun->SetOutputFile(outFile);
// -------------------------------------------------------------------------

    CbmKF* KF = new CbmKF();
    fRun->AddTask(KF);

    CbmL1* l1 = new CbmL1();
    fRun->AddTask(l1);  
    
    CbmD0CandidateSelection* D0selection  = new CbmD0CandidateSelection("d0selection", 1, 0.08,  0.01 );
    D0selection->SetTestMode(useMC);

    //CbmD0Candidates* D0cand  = new CbmD0Candidates("d0cand", 1, 10.0,  -1.0 );//open cuts
    //CbmD0CandidatesSE* D0cand  = new CbmD0CandidatesSE("d0candSE", 1,kFALSE, 10.0,  -1.0, 500);//open cuts

    //char* name, Int_t iVerbose, Bool_t SuperEventMode,  cutIPD0max,  cutSVZmin,  cutSVZmax
    //CbmD0CandidatesSE* D0cand  = new CbmD0CandidatesSE("d0cand", 1, kFALSE,  0.1, -0.02, 1 );
    //D0cand->SetNegativeFileName(NegativeD0File);
    //D0cand->SetKFParticle(kTRUE);
    
    fRun->AddTask(D0selection);
    
  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data());
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------
    
// -------------------------------------------------------------------------
    fRun->Init();
    fRun->Run(0,nEvents);
// -------------------------------------------------------------------------

    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    printf("\nRealTime=%f seconds, CpuTime=%f seconds\n",rtime,ctime);
    cout << "output file is : " << outFile << endl;

}

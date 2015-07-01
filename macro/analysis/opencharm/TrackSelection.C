//void TrackSelection(Int_t index) {
void TrackSelection() 
{
    Int_t index;
    
    TString input = "auau.25gev";
    TString system = "centr"; 
    TString signal = "d0";
    Int_t  nEvents = 1000;
    Int_t  iVerbose = 0;
    const char* setup = "sis300_electron"; 
    bool PileUp = false; 
    bool littrack = false;
    
    // Convert index to the string
    char strIndex[4];
    sprintf(strIndex, "%4d", index);
    for(Int_t i = 0; i < 4; i++) {
	if(' ' == strIndex[i]) strIndex[i] = '0';
    }


    TStopwatch timer;
    timer.Start();

    gDebug=0;

    // ------------------------------------------------------------
    // Monte Carlo file
 TString mcFile = "data/opencharm.mc.unigen." + input + "." + system +"." + signal + ".root";

  // Parameter file
  TString parFile = "data/paramsunigen.";
  parFile = parFile + input;
  parFile = parFile + ".";
  parFile = parFile + system;
  parFile = parFile + ".";
  parFile = parFile + signal;
  parFile = parFile + ".root";

    // Reco file
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
  TString outSystem = "data/opencharm.tracks.unigen." + input + "." + system + "." + signal;
  if(!PileUp)
    {
    if(littrack)
       TString outFile = outSystem + ".littrack.root";
    else 
       TString outFile = outSystem + ".l1.root";
    }
    else if(littrack)
       TString outFile = outSystem + ".PileUp.littrack.root";
    else 
       TString outFile = outSystem + ".PileUp.l1.root";

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

  // ========================================================================
  FairRunAna *fRun = new FairRunAna();
  fRun->SetInputFile(mcFile);
  fRun->AddFriend(rcFile);
  fRun->SetOutputFile(outFile);
  // ========================================================================

  // ----- MC Data Manager   ------------------------------------------------
  //CbmMCDataManager* mcManager=new CbmMCDataManager("MCManager", 1);
  //mcManager->AddFile(inFile);
  //run->AddTask(mcManager);
  // ------------------------------------------------------------------------

  CbmKF* KF = new CbmKF();                              // name, Verbose,  cutP,  cutPt, cutPV, cutIP )
  fRun->AddTask(KF);
   
  CbmD0TrackSelection* dSelect = new CbmD0TrackSelection("Selection", 1,  0.0,   0.0,    4.5,  10000.000);
  //dSelect->SetNHitsOfLongTracks(2); //suppress short tracks
  //dSelect->ShowDebugHistos();
  fRun->AddTask(dSelect);

  //OPEN CUTS
  //CbmD0TrackSelection* dSelect = new CbmD0TrackSelection("Selection", 1,  0.0,   0.0,    0.0,  10000.000);
  //dSelect->SetNHitsOfLongTracks(5); //suppress short tracks

  //CbmD0TrackSelection* dSelect = new CbmD0TrackSelection("Selection", 1,  0.0,   0.0,    -10.0, 0.0 );
  //dSelect->ShowDebugHistos();

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

  fRun->Init();
  fRun->Run(0,nEvents);

  timer.Stop();  

  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n",rtime,ctime);
  cout << "output file is: " << outFile << endl;
  //gFile->Close();
  //exit(0);
}

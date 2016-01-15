/* Reconstruct rings, from simulated data. */

void run_reco_jordan(Int_t nEvents = 10, int geom_nb = 0)
{

  TTree::SetMaxTreeSize(90000000000);
  gRandom->SetSeed(10);

  char PATH1[256];
  char PATH2[256];
  char PATH3[256];

  TString dir = "/home/jordan/Documents/CbmRoot/Mirr_Align_Sim/mirror_alignment/";
  sprintf(PATH1, "Parameters_RichGeo_jordan_%d.root", geom_nb);
  sprintf(PATH2, "Sim_RichGeo_jordan_%d.root", geom_nb);
  sprintf(PATH3, "Rec_RichGeo_jordan_%d.root", geom_nb);

  TString ParFile = dir + PATH1; // Load Parameter File
  TString SimFile = dir + PATH2; // Load Simulation File
  TString RecFile = dir + PATH3; // Set Output File

// ------------------------------------------------------------------- //

  gDebug = 0;
  TStopwatch timer;
  timer.Start();

  gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
  loadlibs();

  FairRunAna *run= new FairRunAna();
  run->SetInputFile(SimFile);
  run->SetOutputFile(RecFile);

  CbmKF *kalman = new CbmKF();
  run->AddTask(kalman);

  CbmRichHitProducer* richHitProd  = new CbmRichHitProducer();	//Set PMT
  richHitProd->SetDetectorType(6);
  richHitProd->SetNofNoiseHits(220);
  richHitProd->SetCollectionEfficiency(1.0);
  richHitProd->SetSigmaMirror(0.06);	//Smearing
  richHitProd->SetCrossTalkHitProb(0.0);
  run->AddTask(richHitProd);

  CbmRichReconstruction* richReco = new CbmRichReconstruction();
  richReco->SetRunExtrapolation(false);
  richReco->SetRunProjection(false);
  richReco->SetRunTrackAssign(false);
  richReco->SetFinderName("ideal");
  run->AddTask(richReco);


  CbmRichMatchRings* matchRings = new CbmRichMatchRings();
  run->AddTask(matchRings);
      
  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open(ParFile.Data());
  rtdb->setFirstInput(parIo1);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();

  run->Init();
  run->Run(0,nEvents);

  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished successfully." << endl;
  cout << "Output file is "    << RecFile << endl;
  cout << "Parameter file is " << ParFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;

  cout << " Test passed" << endl;
  cout << " All ok " << endl;

}

// --------------------------------------------------------------------------
//
// Macro for local MVD reconstruction from MC data
//
// Tasks:  CbmMvdDigitiser
//         CbmMvdClusterfinder
//         CbmMvdHitfinder
// 
// P. Sitzmann Juli 2014
// --------------------------------------------------------------------------


void mvd_CbmUniGen_reco_cluster(TString input = "auau.25gev", TString system = "centr", Int_t  nEvents = 100,
              Int_t  iVerbose = 0, const char* setup = "sis300_electron")
{

  // ========================================================================
  //          Adjust this part according to your requirements

   
  // Input file (MC events)
 // TString inFile = "data/mvd.mc.unigen.root";
  TString inFile = "data/mvd.mc.unigen." + input + "." + system + ".root";
  TString deltaFile = "data/mvd.mc.delta.root";
  TString bgFile = "data/mvd.mc.unigen." + input + "." + system + ".root";
  // Output file
  TString outFile = "data/mvd.reco.unigen." + input + "." + system + ".root";

  // Parameter file
  TString parFile = "data/paramsunigen.";
  parFile = parFile + input;
  parFile = parFile + ".";
  parFile = parFile + system;
  parFile = parFile + ".root";

  // In general, the following parts need not be touched
  // ========================================================================

  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------


  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *run= new FairRunAna();

  run->SetInputFile(inFile);
  run->SetOutputFile(outFile);
  // ------------------------------------------------------------------------
 
  // -----   MVD Digitiser   ----------------------------------------------
  CbmMvdDigitizer* mvdDigitise = new CbmMvdDigitizer("MVD Digitiser", 0, iVerbose);
  run->AddTask(mvdDigitise);
  // ----------------------------------------------------------------------

  // -----   MVD Clusterfinder   --------------------------------------------
  CbmMvdClusterfinder* mvdCluster = new CbmMvdClusterfinder("MVD Clusterfinder", 0, iVerbose); 
  run->AddTask(mvdCluster);
  // ----------------------------------------------------------------------

  // -----   MVD Hit Finder   ---------------------------------------------
  CbmMvdHitfinder* mvdHitfinder = new CbmMvdHitfinder("MVD Hit Finder", 0, iVerbose);
  mvdHitfinder->UseClusterfinder(kTRUE);
  run->AddTask(mvdHitfinder);
  // ----------------------------------------------------------------------

  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open(parFile);
  rtdb->setFirstInput(parIo1);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------
  // -----   Run initialisation   -------------------------------------------
  run->Init();
  // ------------------------------------------------------------------------

  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------
     
  // -----   Start run   ----------------------------------------------------
  run->Run(0,nEvents);
  // ------------------------------------------------------------------------

  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is "    << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------


}

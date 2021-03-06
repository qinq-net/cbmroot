// --------------------------------------------------------------------------
//
// Macro to create digi parameters for the TOF
//
// The way how the pad layout looks like has to be
// implemented in a task called in this macro.
// CbmTofCreateDigiPar is the implementation used
// here.
//
// F.Uhlig 07.05.2010
//
// --------------------------------------------------------------------------


void create_tof_digipar(TString fileName="tof_v16a_1e", Int_t nEvents = 0)
{

  cout<<"fileName: "<<fileName<<endl;


  // ========================================================================
  //          Adjust this part according to your requirements

  // Verbosity level (0=quiet, 1=event level, 2=track level, 3=debug)
  Int_t iVerbose = 0;

  // Input file (MC events)
  TString inFile = "auaumbias." + fileName + ".mc.root";

  // Geometry File
  TString geoFile = fileName + "_geo.root";

  // Output file
  TString outFile = "test.esd." + fileName + ".root";

  // Digi Parameter Output File
//  TString digiFile = fileName + ".digi.par.long";
    TString digiFile = fileName + ".digi.par";

  // In general, the following parts need not be touched
  // ========================================================================



  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------



  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------


  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *run= new FairRunAna();
//  run->SetInputFile(inFile);
  run->SetOutputFile(outFile);
  run->SetGeomFile(geoFile);
  FairLogger::GetLogger()->SetLogScreenLevel("DEBUG2");
  // ------------------------------------------------------------------------

  FairRuntimeDb* rtdb = run->GetRuntimeDb();

  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo2->open(digiFile,"out");
  rtdb->setOutput(parIo2);

  CbmTofCreateDigiPar* tofDigiProducer = new CbmTofCreateDigiPar("TOF Digi Producer",
                                                        "TOF task");
  run->AddTask(tofDigiProducer);

  // -------------------------------------------------------------------------

  rtdb->saveOutput();

  // -----   Intialise and run   --------------------------------------------
  //  run->LoadGeometry();
  run->Init();

  rtdb->print();


  CbmTofDigiPar* DigiPar = (CbmTofDigiPar*)
                           rtdb->getContainer("CbmTofDigiPar");

  DigiPar->setChanged();
  DigiPar->setInputVersion(run->GetRunId(),1);
  rtdb->print();
  rtdb->saveOutput();


  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is "    << outFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------

  cout << " Test passed" << endl;
  cout << " All ok " << endl;
  exit(0);
}

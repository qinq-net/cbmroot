// --------------------------------------------------------------------------
//
// Macro for time-based digitisation
//
// Includes (currently) only the STS digitiser

// !!! Be sure to have only STS in simulation. Otherwise, the MC Buffer
// !!! will overflow, since MCPOint other than STS wil not be processed.
// (will be catched later)
//
// V. Friese   01/02/2013
// Version     01/02/2013 (V. Friese)
//
// --------------------------------------------------------------------------

TString caveGeom="";
TString pipeGeom="";
TString magnetGeom="";
TString mvdGeom="";
TString stsGeom="";
TString richGeom="";
TString muchGeom="";
TString shieldGeom="";
TString trdGeom="";
TString tofGeom="";
TString ecalGeom="";
TString platformGeom="";
TString psdGeom="";
Double_t psdZpos=0.;
Double_t psdXpos=0.;

TString mvdTag="";
TString stsTag="";
TString trdTag="";
TString tofTag="";

TString stsDigi="";
TString muchDigi="";
TString trdDigi="";
TString tofDigi="";

TString mvdMatBudget="";
TString stsMatBudget="";

TString  fieldMap="";
Double_t fieldZ=0.;
Double_t fieldScale=0.;
Int_t    fieldSymType=0;

TString defaultInputFile="";

void run_digi_tb(Int_t nEvents = 2, const char* setupName = "sis100_electron")
{
  // ========================================================================
  //          Adjust this part according to your requirements

  TString outDir  = "data/";
  TString inFile  = outDir + setupName + "_test.mc.root";   // Input file (MC events)
  TString parFile = outDir + setupName + "_params.root";    // Parameter file
  TString outFile = outDir + setupName + "_test.raw.root";  // Output file
  
  // Specify interaction rate in 1/s
  Double_t eventRate = 1.e6;
  
  // Specify duration of time slices in output [ns]
  Double_t timeSliceSize = 1000.;
  
  // Specify log level (INFO, DEBUG, DEBUG1, ...)
  TString logLevel = "INFO";

  TString inDir = gSystem->Getenv("VMCWORKDIR");
  TString paramDir = inDir + "/parameters/";

  TString setupFile = inDir + "/geometry/setup/setup_" + setupName + ".C";
  TString setupFunct = TString("setup_") + setupName + "()";

  gROOT->LoadMacro(setupFile);
  gInterpreter->ProcessLine(setupFunct);

  // In general, the following parts need not be touched
  // ========================================================================

  TList *parFileList = new TList();
  TString geoTag;
  CbmSetup* setup = CbmSetup::Instance();

  // - TOF digitisation parameters
  if ( setup->GetGeoTag(kTof, geoTag) ) {
  	TObjString* tofFile = new TObjString(paramDir + "tof/tof_" + geoTag + ".digi.par");
  	TObjString* tofBdfFile = new TObjString(paramDir + "tof/tof_" + geoTag + ".digibdf.par");
  	parFileList->Add(tofFile);
  	parFileList->Add(tofBdfFile);
  }

  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------


  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  Bool_t hasFairMonitor = Has_Fair_Monitor();
  if (hasFairMonitor) {
    FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  }
  // ------------------------------------------------------------------------

  // -----   Reconstruction run   -------------------------------------------
  CbmRunAna *run = new CbmRunAna();
  run->SetAsync();                         // asynchroneous mode
  run->SetInputFile(inFile);
  run->SetOutputFile(outFile);
  run->SetEventMeanTime(1.e9 / eventRate);
  FairRootManager::Instance()->SetUseFairLinks(kTRUE);
  // ------------------------------------------------------------------------


  // ---- Set the log level 	
  FairLogger::GetLogger()->SetLogScreenLevel(logLevel.Data());


  // ----- STS digitiser
  Double_t dynRange       =   40960.;  // Dynamic range [e]
  Double_t threshold      =    4000.;  // Digitisation threshold [e]
  Int_t nAdc              =    4096;   // Number of ADC channels (12 bit)
  Double_t timeResolution =       0.;  // time resolution [ns]
  Double_t deadTime       =     100.;  // infinite dead time (integrate entire event)
  Int_t digiModel         =       1;   // Model: 1 = uniform charge distribution along track
  Double_t noise          =    1000.;  // Noise [e]
	
  // The following settings correspond to a validated implementation. 
  // Changing them is on your own risk.
  Int_t  eLossModel       = 1;         // Energy loss model: uniform 
  Bool_t useLorentzShift  = kFALSE;    // Deactivate Lorentz shift
  Bool_t useDiffusion     = kFALSE;    // Deactivate diffusion
  Bool_t useCrossTalk     = kFALSE;    // Deactivate cross talk
	
  CbmStsDigitize* stsDigi = new CbmStsDigitize(digiModel);
  stsDigi->SetProcesses(eLossModel, useLorentzShift, useDiffusion, useCrossTalk);
  stsDigi->SetParameters(dynRange, threshold, nAdc, timeResolution, deadTime, noise);
  run->AddTask(stsDigi);
  
  CbmTofDigitizerBDF* tofDigi = new CbmTofDigitizerBDF("TOF Digitizer BDF");
  tofDigi->SetInputFileName( paramDir + "tof/test_bdf_input.root");
//  tofDigi->SetHistoFileName( outDir + "tof.digi.hst.root" ); // Uncomment to save control histograms
  tofDigi->SetMonitorHistograms(kFALSE);
  run->AddTask(tofDigi);
 
  // ----- DAQ
  FairTask* daq = new CbmDaq(timeSliceSize);
  run->AddTask(daq);
  
 
  


  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data());
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------


  // -----   Intialise and run   --------------------------------------------
  run->Init();
  cout << "Starting run" << endl;
  run->Run(0, nEvents);
  // ------------------------------------------------------------------------


  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is " << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << endl;
  cout << endl;
  // ------------------------------------------------------------------------

  if (hasFairMonitor) {
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo sysInfo;
    Float_t maxMemory=sysInfo.GetMaxMemory();
    cout << "<DartMeasurement name=\"MaxMemory\" type=\"numeric/double\">";
    cout << maxMemory;
    cout << "</DartMeasurement>" << endl;

    Float_t cpuUsage=ctime/rtime;
    cout << "<DartMeasurement name=\"CpuLoad\" type=\"numeric/double\">";
    cout << cpuUsage;
    cout << "</DartMeasurement>" << endl;

    FairMonitor* tempMon = FairMonitor::GetMonitor();
    tempMon->Print();
  }
  //  delete run;

  cout << " Test passed" << endl;
  cout << " All ok " << endl;
}

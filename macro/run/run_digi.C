/** Macro for detector response simulation (digitisation)
 **
 ** The input is a file with MC data, produced by a transport simulation
 ** (e.g., with run_sim.C). The output is a file with raw data (digis).
 **
 ** The user has to assure that this macro is called with the same CbmSetup
 ** object as used for the transport, e.g. by using the predefined setups.
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 10 February 2016
 **/


void run_digi(Int_t nEvents = 2,
		          const char* setupName = "sis100_electron",
		          const char* inputFile = "")
{


  // ========================================================================
  //          Adjust this part according to your requirements


  // -----   Environment   --------------------------------------------------
  TString myName = "run_digi";  // this macro's name for screen output
  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
  // ------------------------------------------------------------------------


  // -----   In- and output file names   ------------------------------------
  TString inFile = ""; // give here or as argument; otherwise default is taken
  TString outDir  = "data/";
  TString outFile = outDir + setupName + "_test.raw.root";
  TString parFile = outDir + setupName + "_params.root";
  // ------------------------------------------------------------------------


  // --- Logger settings ----------------------------------------------------
  TString logLevel     = "INFO";
  TString logVerbosity = "LOW";
  // ------------------------------------------------------------------------


  // -----   Remove old CTest runtime dependency file  ----------------------
  TString depFile = Remove_CTest_Dependency_File(outDir, "run_digi" , setupName);
  // ------------------------------------------------------------------------


  // -----   Load the geometry setup   -------------------------------------
  std::cout << std::endl;
  TString setupFile = srcDir + "/geometry/setup/setup_" + setupName + ".C";
  TString setupFunct = "setup_";
  setupFunct = setupFunct + setupName + "()";
  std::cout << "-I- " << myName << ": Loading macro " << setupFile << std::endl;
  gROOT->LoadMacro(setupFile);
  gROOT->ProcessLine(setupFunct);
  // ------------------------------------------------------------------------


  // -----   Parameter files as input to the runtime database   -------------
  std::cout << std::endl;
  std::cout << "-I- " << myName << ": Defining parameter files " << std::endl;
  TList *parFileList = new TList();
  TString geoTag;
  CbmSetup* setup = CbmSetup::Instance();

  // - TRD digitisation parameters
  if ( setup->GetGeoTag(kTrd, geoTag) ) {
  	TObjString trdFile(srcDir + "/parameters/trd/trd_" + geoTag + ".digi.par");
  	parFileList->Add(&trdFile);
    std::cout << "-I- " << myName << ": Using parameter file "
    		      << trdFile.GetString() << std::endl;
  }

  // - TOF digitisation parameters
  if ( setup->GetGeoTag(kTof, geoTag) ) {
  	TObjString tofFile(srcDir + "/parameters/tof/tof_" + geoTag + ".digi.par");
  	parFileList->Add(&tofFile);
    std::cout << "-I- " << myName << ": Using parameter file "
    		      << tofFile.GetString() << std::endl;
  	TObjString tofBdfFile(srcDir + "/parameters/tof/tof_" + geoTag + ".digibdf.par");
  	parFileList->Add(&tofBdfFile);
    std::cout << "-I- " << myName << ": Using parameter file "
    		      << tofBdfFile.GetString() << std::endl;
  }
  // ------------------------------------------------------------------------

  
  // In general, the following parts need not be touched
  // ========================================================================



  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------


  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------


  // -----   Input file   ---------------------------------------------------
  std::cout << std::endl;
  TString defaultInputFile = "data/";
  TString defaultInputFile = defaultInputFile + setupName + "_test.mc.root";
  if ( inFile.IsNull() ) {  // Not defined in the macro explicitly
  	if ( strcmp(inputFile, "") == 0 ) {  // not given as argument to the macro
  		inFile = defaultInputFile;
  	}
  	else inFile = inputFile;
  }
  std::cout << "-I- " << myName << ": Using input file " << inFile << std::endl;
  // ------------------------------------------------------------------------


  // -----   FairRunAna   ---------------------------------------------------
  FairRunAna* run = new FairRunAna();
  run->SetInputFile(inFile);
  run->SetOutputFile(outFile);
  run->SetGenerateRunInfo(kTRUE);
  Bool_t hasFairMonitor = Has_Fair_Monitor();
  if (hasFairMonitor) FairMonitor::GetMonitor()->EnableMonitor(kTRUE);
  // ------------------------------------------------------------------------


  // -----   Digitisers   ---------------------------------------------------
  std::cout << std::endl;
  TString macroName = gSystem->Getenv("VMCWORKDIR");
  macroName += "/macro/run/modules/digitize.C";
  std::cout << "Loading macro " << macroName << std::endl;
  gROOT->LoadMacro(macroName);
  gROOT->ProcessLine("digitize()");
  // ------------------------------------------------------------------------


  // -----  Parameter database   --------------------------------------------
  std::cout << std::endl << std::endl;
  std::cout << "-I- " << myName << ": Set runtime DB" << std::endl;
  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
  parIo1->open(parFile.Data());
  parIo2->open(parFileList, "in");
  rtdb->setFirstInput(parIo1);
  rtdb->setSecondInput(parIo2);
  rtdb->setOutput(parIo1);
  rtdb->saveOutput();
  rtdb->print();
  // ------------------------------------------------------------------------


  // -----   Run initialisation   -------------------------------------------
  std::cout << std::endl;
  std::cout << "-I- " << myName << ": Initialise run" << std::endl;
  run->Init();
  // ------------------------------------------------------------------------




  // -----   Start run   ----------------------------------------------------
  std::cout << std::endl << std::endl;
  std::cout << "-I- " << myName << ": Starting run" << std::endl;
  run->Run(0, nEvents);
  // ------------------------------------------------------------------------


  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  std::cout << std::endl << std::endl;
  std::cout << "Macro finished successfully." << std::endl;
  std::cout << "Output file is " << outFile << std::endl;
  std::cout << "Parameter file is " << parFile << std::endl;
  std::cout << "Real time " << rtime << " s, CPU time " << ctime
            << "s" << std::endl << std::endl;
  // ------------------------------------------------------------------------


  // -----   Resource monitoring   ------------------------------------------
  if (hasFairMonitor) {
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo sysInfo;
    Float_t maxMemory=sysInfo.GetMaxMemory();
    std::cout << "<DartMeasurement name=\"MaxMemory\" type=\"numeric/double\">";
    std::cout << maxMemory;
    std::cout << "</DartMeasurement>" << std::endl;

    Float_t cpuUsage=ctime/rtime;
    std::cout << "<DartMeasurement name=\"CpuLoad\" type=\"numeric/double\">";
    std::cout << cpuUsage;
    std::cout << "</DartMeasurement>" << std::endl;

    FairMonitor* tempMon = FairMonitor::GetMonitor();
    tempMon->Print();
  }

  std::cout << " Test passed" << std::endl;
  std::cout << " All ok " << std::endl;

  // Function needed for CTest runtime dependency
  Generate_CTest_Dependency_File(depFile);
  // ------------------------------------------------------------------------

} // End of macro

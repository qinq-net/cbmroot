void test(Int_t nEvents = 2,
          const char* setupName = "sis100_electron",
          const char* output = "test",
          const char* inputFile = "") {
  


  // ---   Logger settings   ------------------------------------------------
  FairLogger::GetLogger()->SetLogScreenLevel("INFO");
  FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");
  // ------------------------------------------------------------------------
  
  
  // -----   In- and output file names   ------------------------------------
  TString dataset(output);
  TString outFile = dataset + ".tra.root";
  TString parFile = dataset + ".par.root";
  TString geoFile = dataset + ".geo.root";
  std::cout << std::endl;
  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
  TString defaultInputFile = srcDir + "/input/urqmd.auau.10gev.centr.root";
  TString inFile;
  if ( strcmp(inputFile, "") == 0 ) inFile = defaultInputFile;
  else inFile = inputFile;
  std::cout << "-I- Using input file " << inFile << std::endl;
  // ------------------------------------------------------------------------
  
  
  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------
 
  
  // --- Transport run   ----------------------------------------------------
  CbmRunTransport run;
  run.AddInput(inFile);
  run.SetOutFileName(outFile);
  run.SetParFileName(parFile);
  run.LoadSetup(setupName);
  run.SetTarget("Gold", 0.025, 2.5);
  run.Run(nEvents);
  // ------------------------------------------------------------------------
 
 
  // -----   Resource monitoring   ------------------------------------------
  timer.Stop();
  if ( Has_Fair_Monitor() ) {      // FairRoot Version >= 15.11
    // Extract the maximal used memory an add is as Dart measurement
    // This line is filtered by CTest and the value send to CDash
    FairSystemInfo sysInfo;
    Float_t maxMemory=sysInfo.GetMaxMemory();
    std::cout << "<DartMeasurement name=\"MaxMemory\" type=\"numeric/double\">";
    std::cout << maxMemory;
    std::cout << "</DartMeasurement>" << std::endl;

    Float_t cpuUsage = timer.CpuTime() / timer.RealTime();
    std::cout << "<DartMeasurement name=\"CpuLoad\" type=\"numeric/double\">";
    std::cout << cpuUsage;
    std::cout << "</DartMeasurement>" << std::endl;
  }

  std::cout << " Test passed" << std::endl;
  std::cout << " All ok " << std::endl;
  // ------------------------------------------------------------------------
  

}

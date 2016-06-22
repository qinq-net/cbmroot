/** @file FHodoLabSetup
 ** @author Florian Uhlig <f.uhlig@gsi.de>
 ** @date 20.06.2016
 **
 ** ROOT macro to read tsa files which have been produced with the new data transport
 ** Convert data into cbmroot format.
 ** Uses CbmFlibTestSource as source task.
 */


void FHodoLabSetup(TString inFile = "hodoTop_source_1000ts_20160422.tsa")
{

  TString srcDir = gSystem->Getenv("VMCWORKDIR");
  TString inDir  = srcDir + "/input/";
  inFile = inDir + inFile;

  // --- Specify number of events to be produced.
  // --- -1 means run until the end of the input file.
  Int_t nEvents = -1;

  // --- Specify output file name (this is just an example)
  TString outFile = "data/test.root";

  // --- Set log output levels
//  FairLogger::GetLogger()->SetLogScreenLevel("DEBUG");
  FairLogger::GetLogger()->SetLogScreenLevel("INFO");
  FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

  // --- Set debug level
  gDebug = 0;
  
  std::cout << std::endl;
  std::cout << ">>> FHodoLabSetup: output file is " << outFile << std::endl;

  // ========================================================================
  // ========================================================================

  std::cout << std::endl;
  std::cout << ">>> FHodoLabSetup: Initialising..." << std::endl;

  // NXyter Unpacker
  CbmTSUnpackTest* test_unpacker = new CbmTSUnpackTest();

  // --- Source task
  CbmFlibTestSource* source = new CbmFlibTestSource();
  source->SetFileName(inFile);
  source->AddUnpacker(test_unpacker, 0xF0, 10);//HODO 1 + 2

  // --- Event header
  //  FairEventHeader* event = new CbmTbEvent();
  //  event->SetRunId(260);

  // --- Run
  FairRunOnline *run = new FairRunOnline(source);
  run->SetOutputFile(outFile);
  //  run->SetEventHeader(event);

  run->Init();

  // --- Start run
  TStopwatch timer;
  timer.Start();
  std::cout << ">>> FHodoLabSetup: Starting run..." << std::endl;
  run->Run(nEvents, 0); // run until end of input file
  timer.Stop();
  
  // --- End-of-run info
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  std::cout << std::endl << std::endl;
  std::cout << ">>> FHodoLabSetup: Macro finished successfully." << std::endl;
  std::cout << ">>> FHodoLabSetup: Output file is " << outFile << std::endl;
  std::cout << ">>> FHodoLabSetup: Real time " << rtime << " s, CPU time "
	    << ctime << " s" << std::endl;
  std::cout << std::endl;

  /// --- Screen output for automatic tests
  std::cout << " Test passed" << std::endl;
  std::cout << " All ok " << std::endl;
}

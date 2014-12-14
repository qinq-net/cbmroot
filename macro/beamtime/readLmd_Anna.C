/** @file readLmd.C
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since December 2013
 ** @date 25.02.2014
 **
 ** @author Anna Senger <a.senger@gsi.de>
 ** @date 09.12.2014 
 ** ROOT macro to read lmd files from beamtime DEC2013 and convert the data
 ** into cbmroot format.
 ** Uses CbmSourceLmd as source task.
 */

void readLmd_Anna(Int_t runID, Int_t fileID)
{
  
	TString runN = TString::Itoa(runID,10);
	TString fileN = TString::Itoa(fileID,10);

	// --- Specify input file name (this is just an example)
	
	TString inDir = "/hera/cbm/prod/beamtime/2013/12/cosy/run" + runN + "/";
	TString inFile = inDir + "run" + runN + "_00" + fileN + "?.lmd";
	
	// --- Specify number of events to be produced.
	// --- -1 means run until the end of the input file.
	Int_t nEvents = -1;

	// --- Specify output file name (this is just an example)
	TString outDir = "data/";
	TString outFile = outDir + "calib.run" + runN + "_" + fileN + ".root";

	// --- Set log output levels
	FairLogger::GetLogger()->SetLogScreenLevel("INFO");
	FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

	// --- Set debug level
  gDebug = 0;
  
  std::cout << std::endl;
  std::cout << ">>> readLmd:  input file is " << inFile  << std::endl;
  std::cout << ">>> readLmd: output file is " << outFile << std::endl;

  // ========================================================================
  // ========================================================================

  std::cout << std::endl;
  std::cout << ">>> readLmd: Initialising..." << std::endl;
  
  // --- Sets
  Bool_t trigger = kTRUE;   // kFALSE - self-trigger; kTRUE - external trigger
  Int_t triggered_station = 2;
  
  // --- Source task
  CbmSourceLmd* source = new CbmSourceLmd();
  source->AddFile(inFile);
  source->SetTriggeredMode(trigger);

  // --- Event header
  FairEventHeader* event = new CbmTbEvent();
  event->SetRunId(runID);

  // --- Run
  FairRunOnline *run = new FairRunOnline(source);
  run->SetOutputFile(outFile);
  run->SetEventHeader(event);
  
  // --- Calibration  
  CbmStsCosyBL* hist= new CbmStsCosyBL();
  hist->SetTriggeredMode(trigger);
  hist->SetTriggeredStation(triggered_station);
  run->AddTask(hist);
  
  run->Init();
  
  // --- Start run
  TStopwatch timer;
  timer.Start();
  std::cout << ">>> readLmd: Starting run..." << std::endl;
  run->Run(nEvents, 0); // run until end of input file
  timer.Stop();
  
  // --- End-of-run info
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  std::cout << std::endl << std::endl;
  std::cout << ">>> readLmd: Macro finished successfully." << std::endl;
  std::cout << ">>> readLmd: Output file is " << outFile << std::endl;
  std::cout << ">>> readLmd: Real time " << rtime << " s, CPU time "
  					<< ctime << " s" << std::endl;
  std::cout << std::endl;

  /// --- Screen output for automatic tests
  std::cout << " Test passed" << std::endl;
  std::cout << " All ok " << std::endl;
}

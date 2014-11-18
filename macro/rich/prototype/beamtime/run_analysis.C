void run_analysis()
{
   TStopwatch timer;
   timer.Start();
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();

   //string hldFileName = "/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/secondtest_pulser16ch+sync.hld";
   string hldFileDir = "";// "/home/pusan/nov2014data/";
   string hldFileName = "te14322160114.hld";// "Laser_100_0.hld";
   Bool_t isAnaPulserEvents = false; // Set to true if you want to analyze pulser events

   // --- Specify number of events to be produced.
   // --- -1 means run until the end of the input file.
   Int_t nEvents = -1;

   // --- Specify output file name (this is just an example)
   //TString outFile = "/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/output_test_file.root";
   TString outFile = hldFileName + ".root";

   // --- Set log output levels
   FairLogger::GetLogger()->SetLogScreenLevel("INFO");
   FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

   // --- Set debug level
   gDebug = 0;

   CbmRichTrbUnpack* source = new CbmRichTrbUnpack(hldFileDir + hldFileName);
   source->SetAnaPulserEvents(isAnaPulserEvents);

   CbmTrbCalibrator* fgCalibrator = CbmTrbCalibrator::Instance();

   // either import calibration tables from the existing file
   fgCalibrator->Import("calibration.root");
   fgCalibrator->Draw();
   // or calibration tables from first events
   // one can explicitly set the number of entries for channel to start its calibration
//   fgCalibrator->EnableCalibration();
//   fgCalibrator->SetCalibrationPeriod(10000);

   // --- Event header
   //FairEventHeader* event = new CbmTbEvent();
   //event->SetRunId(260);

   // --- Run
   FairRunOnline *run = new FairRunOnline(source);
   run->SetOutputFile(outFile);
   //run->SetEventHeader(event);

   if (!isAnaPulserEvents) {
	   CbmRichReconstruction* richReco = new CbmRichReconstruction();
	   richReco->SetFinderName("hough_prototype");
	   richReco->SetRunTrackAssign(false);
	   richReco->SetRunExtrapolation(false);
	   richReco->SetRunProjection(false);
	   richReco->SetRunFitter(false);
	   run->AddTask(richReco);

	   CbmRichTrbRecoQa* qaRaw = new CbmRichTrbRecoQa();
	   run->AddTask(qaRaw);
   } else {
	   CbmRichTrbPulserQa* qaPulser = new CbmRichTrbPulserQa();
	   run->AddTask(qaPulser);
   }

   run->Init();

   // --- Start run
   TStopwatch timer;
   timer.Start();
   run->Run(nEvents, 0); // run until end of input file
   timer.Stop();

   // --- export calibration tables
//   fgCalibrator->Export("calibration.root");

   // --- End-of-run info
   Double_t rtime = timer.RealTime();
   Double_t ctime = timer.CpuTime();
   std::cout << std::endl << std::endl;
   std::cout << "Macro finished successfully." << std::endl;
   std::cout << "Output file is " << outFile << std::endl;
   std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl;
   std::cout << " Test passed" << std::endl;
   std::cout << " All ok " << std::endl;
}


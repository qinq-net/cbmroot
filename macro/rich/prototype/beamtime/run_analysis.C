enum enu_calibMode {
   etn_IMPORT,  // import calibration tables from the file and use them
   etn_ONLINE,  // use first data to calibrate; the channel has to get at least fCalibrationPeriod messages to get calibrated
   etn_NOCALIB, // use linear function going from origin to (512, n) which means that the fine time is not calibrated
   etn_IDEAL   // use almost linear function - close to real calibration but idealized
};

void run_analysis()
{
   TStopwatch timer;
   timer.Start();
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();

   //string hldFileName = "/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/secondtest_pulser16ch+sync.hld";
   string hldFileDir = "";// "/home/pusan/nov2014data/";
   string hldFileName = hldFileDir + "te14322160114.hld";// "Laser_100_0.hld";
   TString outRootFileName = hldFileName + ".root";// "Laser_100_0.hld";
   Bool_t isAnaPulserEvents = false; // Set to true if you want to analyze pulser events

   	TString script = TString(gSystem->Getenv("SCRIPT"));

   	if (script == "yes") {
   		hldFileName = string(gSystem->Getenv("INPUT_HLD_FILE"));
   		outRootFileName = TString(gSystem->Getenv("OUTPUT_ROOT_FILE"));
   	}

   // --- Specify number of events to be produced.
   // --- -1 means run until the end of the input file.
   Int_t nEvents = -1;

   // --- Set log output levels
   FairLogger::GetLogger()->SetLogScreenLevel("INFO");
   FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

   // --- Set debug level
   gDebug = 0;

   CbmRichTrbUnpack* source = new CbmRichTrbUnpack(hldFileName);
   source->SetAnaPulserEvents(isAnaPulserEvents);

   CbmTrbCalibrator* fgCalibrator = CbmTrbCalibrator::Instance();
   fgCalibrator->SetInputFilename("calibration.root");            // does not actually import data - only defines
                                                                  // the file that will be used if you specidy mode etn_IMPORT
   fgCalibrator->SetMode(etn_IMPORT);
                                                                  // Also note the (un)commented line in the end of the macro with export func

   // --- Event header
   //FairEventHeader* event = new CbmTbEvent();
   //event->SetRunId(260);

   // --- Run
   FairRunOnline *run = new FairRunOnline(source);
   run->SetOutputFile(outRootFileName);
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
   //fgCalibrator->Export("calibration.root");

   // --- End-of-run info
   Double_t rtime = timer.RealTime();
   Double_t ctime = timer.CpuTime();
   std::cout << std::endl << std::endl;
   std::cout << "Macro finished successfully." << std::endl;
   std::cout << "Output file is " << outRootFileName << std::endl;
   std::cout << "Real time " << rtime << " s, CPU time " << ctime << " s" << std::endl;
   std::cout << " Test passed" << std::endl;
   std::cout << " All ok " << std::endl;
}


void run_analysis()
{
   TStopwatch timer;
   timer.Start();
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();

   string hldFileName = "Laser_100_0.hld";

   // --- Specify number of events to be produced.
   // --- -1 means run until the end of the input file.
   Int_t nEvents = -1;

   // --- Specify output file name (this is just an example)
   //TString outFile = "/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/output_test_file.root";
   TString outFile = "te14320215205.root";

   // --- Set log output levels
   FairLogger::GetLogger()->SetLogScreenLevel("INFO");
   FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

   // --- Set debug level
   gDebug = 0;


   CbmRichTrbUnpack* source = new CbmRichTrbUnpack(hldFileName);
   source->SetAnaPulserEvents(true);

   //CbmTrbCalibrator* fgCalibrator = CbmTrbCalibrator::Instance();
   //fgCalibrator->Import("calib_te14320215205.root");
   //fgCalibrator->Export("calibration.root");

   // --- Event header
 //  FairEventHeader* event = new CbmTbEvent();
 //  event->SetRunId(260);

   // --- Run
   FairRunOnline *run = new FairRunOnline(source);
   run->SetOutputFile(outFile);
 //  run->SetEventHeader(event);

   CbmRichReconstruction* richReco = new CbmRichReconstruction();
   //richReco->SetZTrackExtrapolation(50.);
   //richReco->SetMinNofStsHits(0);
   richReco->SetFinderName("hough_prototype");
   richReco->SetRunTrackAssign(false);
   richReco->SetRunExtrapolation(false);
   richReco->SetRunProjection(false);
   richReco->SetRunFitter(false);
  // run->AddTask(richReco);

//   CbmRichTrbRecoQa* qaRaw = new CbmRichTrbRecoQa();
//   run->AddTask(qaRaw);

//   CbmRichTrbPulserQa* qaPulser = new CbmRichTrbPulserQa();
//   run->AddTask(qaPulser);


   run->Init();


   // --- Start run
   TStopwatch timer;
   timer.Start();
   run->Run(nEvents, 0); // run until end of input file
   timer.Stop();

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


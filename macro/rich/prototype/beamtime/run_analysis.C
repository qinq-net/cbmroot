void run_analysis()
{
   TStopwatch timer;
   timer.Start();
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();

   //string hldFileName = "/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/secondtest_pulser16ch+sync.hld";
   string hldFileName = "../../../fles/secondtest_pulser16ch+sync.hld";

   // --- Specify number of events to be produced.
   // --- -1 means run until the end of the input file.
   Int_t nEvents = -1;

   // --- Specify output file name (this is just an example)
   //TString outFile = "/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/output_test_file.root";
   TString outFile = "output_test_file.root";

   // --- Set log output levels
   FairLogger::GetLogger()->SetLogScreenLevel("INFO");
   FairLogger::GetLogger()->SetLogVerbosityLevel("LOW");

   // --- Set debug level
   gDebug = 0;


   CbmRichTrbUnpack* source = new CbmRichTrbUnpack(hldFileName);

   // --- Event header
 //  FairEventHeader* event = new CbmTbEvent();
 //  event->SetRunId(260);

   // --- Run
   FairRunOnline *run = new FairRunOnline(source);
   run->SetOutputFile(outFile);
 //  run->SetEventHeader(event);

   CbmRichTrbRecoQa* qaRaw = new CbmRichTrbRecoQa();
   run->AddTask(qaRaw);



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


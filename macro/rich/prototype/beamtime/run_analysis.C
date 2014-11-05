void run_analysis()
{
   TStopwatch timer;
   timer.Start();
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();

   string hldFileName = "/Users/slebedev/Development/cbm/trunk/cbmroot/macro/fles/secondtest_pulser16ch+sync.hld";

   CbmRichTrbUnpack* richUnpack = new CbmRichTrbUnpack();
   richUnpack->Run(hldFileName);
}


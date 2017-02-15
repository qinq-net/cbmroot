/** run_rich_event_display.C
 * @author Semen Lebedev <s.lebedev@gsi.de>
 * @since 2012
 * @version 1.0
 **/

void run_rich_event_display()
{
   gROOT->LoadMacro("$VMCWORKDIR/gconfig/basiclibs.C");
   basiclibs();

   gROOT->LoadMacro("$VMCWORKDIR/macro/rich/cbmlibs.C");
   cbmlibs();
    
    TString outDir = "/Users/slebedev/Development/cbm/data/sim/rich/urqmdtest/";
    TString paramFile =  outDir + "param.3.root";
    TString mcFile = outDir + "mc.3.root";
    TString recoFile = outDir + "reco.3.root";
    TString outFile = outDir + "ed.3.root";
    
    //TString mcFile = "/Users/slebedev/Development/cbm/data/simulations/rich/richreco/mc.00000.root";
    //TString paramFile = "/Users/slebedev/Development/cbm/data/simulations/rich/richreco/param.00000.root";
    //TString recoFile ="/Users/slebedev/Development/cbm/data/simulations/rich/richreco/reco.00000.root";
    //TString outFile ="/Users/slebedev/Development/cbm/data/simulations/rich/richreco/ed.00000.root";
   //TString mcFile = "/Users/slebedev/Development/cbm/data/simulations/richurqmdtest/8gev.mc.0000.root";
   //TString recoFile = "/Users/slebedev/Development/cbm/data/simulations/richurqmdtest/8gev.reco.0000.root";
   //TString paramFile = "/Users/slebedev/Development/cbm/data/simulations/richurqmdtest/8gev.param.0000.root";
   //TString outFile = "/Users/slebedev/Development/cbm/data/simulations/richurqmdtest/8gev.test.ed.root";

   FairRunAna *run= new FairRunAna();
   run->SetInputFile(mcFile);
   run->AddFriend(recoFile);
   run->SetOutputFile(outFile);


   CbmRichEventDisplay *ed = new CbmRichEventDisplay();
   ed->SetDrawRings(false);
   ed->SetDrawHits(true);
   ed->SetDrawPoints(false);
   ed->SetDrawProjections(true);
   run->AddTask(ed);


   FairRuntimeDb* rtdb = run->GetRuntimeDb();
   FairParRootFileIo* parIo1 = new FairParRootFileIo();
   FairParAsciiFileIo* parIo2 = new FairParAsciiFileIo();
   parIo1->open(paramFile.Data());
   rtdb->setFirstInput(parIo1);
   rtdb->setOutput(parIo1);
   rtdb->saveOutput();

   run->Init();
   cout << "Starting run" << endl;
   run->Run(0, 10);
}

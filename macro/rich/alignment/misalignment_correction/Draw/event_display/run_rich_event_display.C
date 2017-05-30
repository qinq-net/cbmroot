/** run_rich_event_display.C
 * @author Semen Lebedev <s.lebedev@gsi.de>
 * @since 2012
 * @version 1.0
 **/

void run_rich_event_display()
{
//  gROOT->LoadMacro("$VMCWORKDIR/gconfig/basiclibs.C");
  gROOT->LoadMacro("/u/jbendar/cbmroot5/CBMSRC/gconfig/basiclibs.C");
  basiclibs();

//  gROOT->LoadMacro("$VMCWORKDIR/macro/rich/cbmlibs.C");
  gROOT->LoadMacro("/u/jbendar/cbmroot5/CBMSRC/macro/rich/cbmlibs.C");
  cbmlibs();

  TString outDir1 = "/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/";
  TString outDir2 = "Batch_3/";
  TString outDir3 = "Aligned/";

  TString name = "setup_align_";
  TString jobID = "16516585";		// Aligned
  TString taskID = "0012";
  TString nb = jobID + "_" + taskID;

  TString outDir = outDir1 + outDir2 + outDir3;

  TString script = TString(gSystem->Getenv("SCRIPT"));
  if ( script == "yes" ) {
	outDir = TString(gSystem->Getenv("OUT_DIR"));
	name = TString(gSystem->Getenv("NAME"));
	nb = TString(gSystem->Getenv("NB_JOB_TASK"));

	std::cout << "outDir: " << outDir << ", name: " << name << ", nb: " << nb << std::endl;
  }

  TString paramFile =  outDir + name + "params." + nb  + ".root";
  TString mcFile = outDir + name + "mc." + nb + ".root";
  TString recoFile = outDir + name + "reco." + nb + ".root";
  TString outFile = outDir + "ed." + nb + ".root";

  FairRunAna *run= new FairRunAna();
  run->SetInputFile(mcFile);
  run->AddFriend(recoFile);
  run->SetOutputFile(outFile);

  CbmRichEventDisplay *ed = new CbmRichEventDisplay();
  ed->SetDrawRings(true);
  ed->SetDrawHits(true);
  ed->SetDrawPoints(false); //
  ed->SetDrawProjections(true);
  string dirout = outDir + nb + "/";
  ed->SetOutputDir(dirout);
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
  run->Run(20, 120);
}


void draw_analysis()
{
	TStopwatch timer;
	timer.Start();
	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
	loadlibs();

	TString outputDir = "recoA/";
	TString runTitle = "run_title_a";
	TString histFileName = "te14324200946.hld.root";

	CbmRichTrbRecoQa* qaReco = new CbmRichTrbRecoQa();
	qaReco->SetOutputDir(outputDir);
	qaReco->SetRunTitle(runTitle);
	qaReco->SetDrawHist(true);
	qaReco->DrawHistFromFile(histFileName);
}


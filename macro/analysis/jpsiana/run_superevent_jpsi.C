
void run_superevent_jpsi()
{
	TStopwatch timer;
	timer.Start();
	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
	loadlibs();

	CbmAnaJpsiSuperEvent* se = new CbmAnaJpsiSuperEvent();
	se->AddFile("/Users/slebedev/Development/cbm/data/jpsi/analysis.0001.root");
	//se->AddFile("/Users/slebedev/Development/cbm/data/jpsi/analysis.0001.root");
	//se->AddFile("/Users/slebedev/Development/cbm/data/jpsi/analysis.0001.root");
	//se->AddFile("/Users/slebedev/Development/cbm/data/jpsi/analysis.0001.root");

	se->Run();
}


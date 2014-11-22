
void draw_study_report()
{
	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
	loadlibs();


	std::vector<std::string> fileNames, studyNames;
	std::string outputDir;

	fileNames.push_back("wls_off_ring_a1.root");
	fileNames.push_back("wls_off_ring_b1.root");
	fileNames.push_back("wls_off_ring_b2.root");
	fileNames.push_back("wls_off_ring_b3.root");
	fileNames.push_back("wls_off_ring_d1.root");
	fileNames.push_back("wls_off_ring_dg1.root");
	fileNames.push_back("wls_off_ring_g1.root");
	fileNames.push_back("wls_off_ring_gh1.root");
	fileNames.push_back("wls_off_ring_h1.root");
	fileNames.push_back("wls_off_ring_h2.root");

	studyNames.push_back("wls_off_ring_a1");
	studyNames.push_back("wls_off_ring_b1");
	studyNames.push_back("wls_off_ring_b2");
	studyNames.push_back("wls_off_ring_b3");
	studyNames.push_back("wls_off_ring_d1");
	studyNames.push_back("wls_off_ring_dg1");
	studyNames.push_back("wls_off_ring_g1");
	studyNames.push_back("wls_off_ring_gh1");
	studyNames.push_back("wls_off_ring_h1");
	studyNames.push_back("wls_off_ring_h2");

	outputDir = "report_wls_off/";

	CbmStudyReport* reportQa = new CbmRichTrbRecoQaStudyReport();
	reportQa->Create(fileNames, studyNames, outputDir);
}

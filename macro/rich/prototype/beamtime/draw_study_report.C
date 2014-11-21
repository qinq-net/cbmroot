
void draw_study_report()
{
	gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
	loadlibs();


	std::vector<std::string> fileNames, studyNames;
	std::string outputDir;

	fileNames.push_back("te14324152650.hld.root");
	fileNames.push_back("te14324200946.hld.root");

	studyNames.push_back("run 1222");
	studyNames.push_back("run 568");

	outputDir = "report/";

	CbmStudyReport* reportQa = new CbmRichTrbRecoQaStudyReport();
	reportQa->Create(fileNames, studyNames, outputDir);
}

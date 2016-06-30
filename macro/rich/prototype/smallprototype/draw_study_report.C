using namespace std;
void draw_study_report()
{
    gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
    loadlibs();
    
    
    std::vector<std::string> fileNames, studyNames;
    std::string outputDir = "results_reports/";
    
    fileNames.push_back("file_hist1.root");
    fileNames.push_back("file_hist2.root");
    
    studyNames.push_back("sim1");
    studyNames.push_back("sim2");
    
    if (outputDir != ""){
        gSystem->mkdir(outputDir.c_str(), true); // create directory if it does not exist
    }
    
    CbmRichSmallPrototypeStudyReport* reportQa = new CbmRichSmallPrototypeStudyReport();
    //reportQa->Create(fileNames, studyNames, outputDir);
}

void draw_litqa()
{
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();


   std::string dir = "/Users/slebedev/Development/cbm/data/lmvm/dec14/8gev/stsv13d/richv14a/trd4/tofv13/0.7field/nomvd/rho0/";
   std::string outputDir = dir + "results_litqa/";
   std::string fileName = dir + "litqa.auau.8gev.centr.all.root";

   gSystem->mkdir(outputDir.c_str(), true);

   CbmSimulationReport* trackingQaReport = new CbmLitTrackingQaReport();
   trackingQaReport->Create(fileName, outputDir);

//   CbmSimulationReport* fitQaReport = new CbmLitFitQaReport();
//   fitQaReport->Create(fileName, outputDir);

//   CbmSimulationReport* clusteringQaReport = new CbmLitClusteringQaReport();
//   clusteringQaReport->Create(fileName, outputDir);

//   CbmLitRadLengthQaReport* radLengthQaReport = new CbmLitRadLengthQaReport();
//   radLengthQaReport->Create(fileName, outputDir);

//   CbmSimulationReport* tofQaReport = new CbmLitTofQaReport();
//   tofQaReport->Create(fileName, outputDir);
}

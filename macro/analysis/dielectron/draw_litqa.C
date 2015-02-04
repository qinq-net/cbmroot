void draw_litqa()
{
   gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
   loadlibs();

   std::string dir = "/Users/slebedev/Development/cbm/data/lmvm/dec14/8gev/stsv13d/richv14a/trd4/tofv13/0.7field/nomvd/rho0/";
   std::string fileName = dir + "litqa.auau.8gev.centr.all.root";

   std::string script = std::string(gSystem->Getenv("SCRIPT"));
   if (script == "yes"){
      dir = std::string(gSystem->Getenv("LMVM_MAIN_DIR"));
      fileName = dir + std::string(gSystem->Getenv("LMVM_LITQA_FILE_NAME"));
   }

   std::string outputDirTracking = dir + "results_litqa_tracking/";
   std::string outputDirClustering = dir + "results_litqa_clustering/";
   gSystem->mkdir(outputDirTracking.c_str(), true);
   gSystem->mkdir(outputDirClustering.c_str(), true);

   CbmSimulationReport* trackingQaReport = new CbmLitTrackingQaReport();
   trackingQaReport->Create(fileName, outputDirTracking);

   CbmSimulationReport* clusteringQaReport = new CbmLitClusteringQaReport();
   clusteringQaReport->Create(fileName, outputDirClustering);

//   CbmSimulationReport* fitQaReport = new CbmLitFitQaReport();
//   fitQaReport->Create(fileName, outputDir);

//   CbmLitRadLengthQaReport* radLengthQaReport = new CbmLitRadLengthQaReport();
//   radLengthQaReport->Create(fileName, outputDir);

//   CbmSimulationReport* tofQaReport = new CbmLitTofQaReport();
//   tofQaReport->Create(fileName, outputDir);
}

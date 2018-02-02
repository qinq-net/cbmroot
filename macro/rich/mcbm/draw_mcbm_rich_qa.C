void draw_mcbm_rich_qa()
{
   std::string outputDir = "results_mcbm_rich_qa/";
   std::string fileName = "/home/aghoehne/Documents/CbmRoot/Gregor/output/reco_results.root";

   CbmRichMCbmQa* richMCbmQa = new CbmRichMCbmQa();
   richMCbmQa->DrawFromFile(fileName, "");
}

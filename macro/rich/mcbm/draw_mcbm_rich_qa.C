void draw_mcbm_rich_qa()
{
   std::string outputDir = "results_mcbm_rich_qa/";
   std::string fileName = "/Users/slebedev/Development/cbm/data/sim/rich/mcbm/all.hists.root";

   CbmRichMCbmQa* richMCbmQa = new CbmRichMCbmQa();
   richMCbmQa->DrawFromFile(fileName, "");
}

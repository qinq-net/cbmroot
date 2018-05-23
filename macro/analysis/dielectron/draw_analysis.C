/** draw_analysis.C
 * @author Elena Lebedeva <e.lebedeva@gsi.de>
 * @since 2010
 * @version 2.0
 **/

void draw_analysis() {

    Bool_t useMvd = false;
    Bool_t drawSignificance = true;
    std::string dir = "/Users/slebedev/Development/cbm/data/lmvm/test6/8gev/omegaepem/";
    std::string fileName = dir + "analysis.auau.8gev.centr.all.root";

    std::string script = std::string(TString(gSystem->Getenv("SCRIPT")).Data());
    if (script == "yes"){
       dir = std::string(TString(gSystem->Getenv("LMVM_MAIN_DIR")).Data());
       fileName = dir + std::string(TString(gSystem->Getenv("LMVM_ANA_FILE_NAME")).Data());
    }

    std::string outputDir = dir + "lmvm_results/";

    CbmAnaDielectronTaskDraw *draw = new CbmAnaDielectronTaskDraw();
    draw->DrawHistFromFile(fileName, outputDir, useMvd);//, drawSignificance);
}

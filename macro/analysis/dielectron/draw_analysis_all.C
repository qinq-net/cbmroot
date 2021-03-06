/** draw_analysis.C
 * @author Elena Lebedeva <e.lebedeva@gsi.de>
 * @since 2011
 * @version 3.0
 **/

void draw_analysis_all() {

    Bool_t useMvd = false;
    std::string dir = "/Users/slebedev/Development/cbm/data/lmvm/may18_100k/8gev/";
    std::string fileName = "analysis.auau.8gev.centr.all.root";

    std::string script = std::string(TString(gSystem->Getenv("SCRIPT")).Data());
    if (script == "yes"){
       dir = std::string(TString(gSystem->Getenv("LMVM_MAIN_DIR")).Data());
       fileName = std::string(TString(gSystem->Getenv("LMVM_ANA_FILE_NAME")).Data());
    }

    std::string outputDir = dir + "lmvm_results_all/";
    std::string fnInmed = dir + "inmed/" + fileName;
    std::string fnQgp = dir + "qgp/" + fileName;
    std::string fnOmega = dir +"omegaepem/" + fileName;
    std::string fnPhi = dir + "phi/" + fileName;
    std::string fnOmegaD = dir +"omegadalitz/" + fileName;

    CbmAnaDielectronTaskDrawAll *draw = new CbmAnaDielectronTaskDrawAll();
    draw->DrawHistosFromFile(fnInmed, fnQgp, fnOmega, fnPhi, fnOmegaD, outputDir, useMvd);
}

using namespace std;

void hadd() {
    gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
    loadlibs();
    gSystem->Load("libAnalysis");

    string dir = "/hera/cbm/users/slebedev/data/jpsi/jun15_25gev/jpsi_urqmd/";
    string fileArray = ".auau.25gev.centr.";
    int nofFiles = 200;

    TString script = TString(gSystem->Getenv("SCRIPT"));
    if (script == "yes") {
    	fileArray = string(gSystem->Getenv("LMVM_FILE_ARRAY"));
    	dir = string(gSystem->Getenv("LMVM_MAIN_DIR"));
        nofFiles = TString(gSystem->Getenv("LMVM_NOF_FILES")).Atof();
    }

    CbmHaddBase::AddFilesInDir(dir, fileArray, "analysis", nofFiles);
    CbmHaddBase::AddFilesInDir(dir, fileArray, "litqa", nofFiles);

}

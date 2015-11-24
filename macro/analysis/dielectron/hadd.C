using namespace std;

void hadd() {
    gROOT->LoadMacro("$VMCWORKDIR/macro/littrack/loadlibs.C");
    loadlibs();
    gSystem->Load("libAnalysis");

    string dir = "/hera/cbm/users/slebedev/mc/dielectron/dec14/8gev/stsv14_4cm/richv14a/trd4/tofv13/1.0field/nomvd/";
    string fileArray = ".auau.8gev.centr.";
    int nofFiles = 250;
    int fileSizeLimit = 50000;
    int nofEvents = 500;

    TString script = TString(gSystem->Getenv("SCRIPT"));
    if (script == "yes") {
    	fileArray = string(gSystem->Getenv("LMVM_FILE_ARRAY"));
    	dir = string(gSystem->Getenv("LMVM_MAIN_DIR"));
        nofFiles = TString(gSystem->Getenv("LMVM_NOF_FILES")).Atof();
    }

    CbmHaddBase::AddFilesInDir(dir + "/omegaepem/", fileArray, "analysis", nofFiles, fileSizeLimit, nofEvents);
    CbmHaddBase::AddFilesInDir(dir + "/phi/", fileArray, "analysis", nofFiles, fileSizeLimit, nofEvents);
    CbmHaddBase::AddFilesInDir(dir + "/omegadalitz/", fileArray, "analysis", nofFiles, fileSizeLimit, nofEvents);
    CbmHaddBase::AddFilesInDir(dir + "/rho0/", fileArray, "analysis", nofFiles, fileSizeLimit, nofEvents);

    CbmHaddBase::AddFilesInDir(dir + "/rho0/", fileArray, "litqa", nofFiles, fileSizeLimit, nofEvents);
    //CbmHaddBase::AddFilesInDir(dir + "/phi/", fileArray, "litqa", nofFiles);
    //CbmHaddBase::AddFilesInDir(dir + "/omegadalitz/", fileArray, "litqa", nofFiles);
    //CbmHaddBase::AddFilesInDir(dir + "/omegaepem/", fileArray, "litqa", nofFiles);
}

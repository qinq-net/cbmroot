{
    gSystem->Load("libcbmnetcntlclientroot.so");
    gInterpreter->AddIncludePath("$VMCWORKDIR");
    gROOT->ProcessLine(".L config_get4v1x.cxx");
}

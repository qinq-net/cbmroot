{
    gSystem->Load("libCbmFlibReader.so");
    gSystem->Load("libcbmnetcntlclientroot.so");
    gInterpreter->AddIncludePath("$VMCWORKDIR");
    gROOT->ProcessLine(".L config_get4v1x.cxx");
//    gROOT->ProcessLine(".L config_tofcern15.cxx");
    gROOT->ProcessLine(".L config_toflab15.cxx");
    gROOT->ProcessLine(".L config_rocgen.cxx");
    cout<<"GET4 control loading done!"<<endl;
}

{
    gSystem->Load("libcbmnetcntlclientroot.so");
    gSystem->Load("libCbmFlibReader.so");
    gInterpreter->AddIncludePath("$VMCWORKDIR");
    gROOT->ProcessLine(".L config_get4v1x.cxx");
//    gROOT->ProcessLine(".L config_tofcern15.cxx");
    gROOT->ProcessLine(".L config_toflab15.cxx");
    cout<<"GET4 control loading done!"<<endl;
}

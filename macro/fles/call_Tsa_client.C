void call_Tsa_client(Int_t nEvents = 100){
  FairRunOnline* run = new FairRunOnline();
  gROOT->ProcessLine(".x readTsa_server.C");
  run->Run(nEvents);
  //printf("to read additional n envents call\n  run->Run(n);\n");
}

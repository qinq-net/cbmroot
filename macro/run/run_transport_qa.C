// forward declaration
TClonesArray* ConnectBranchIfExist(TFile*, TTree*, TString, TString);
void PrintResult(std::vector<std::string>&, std::vector<Int_t>&);

void run_transport_qa(const char* setupName = "sis100_electron",
                      const char* output = "test")
{
 // -----   Environment   --------------------------------------------------
  TString myName = "run_transport_qa";  // this macro's name for screen output
  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
  // ------------------------------------------------------------------------

  // -----   In- and output file names   ------------------------------------
  TString dataset(output);
  TString inFile = dataset + ".tra.30.root";
  std::cout << std::endl;

  std::cout << "-I- " << myName << ": Using input file " << inFile << std::endl;
  // ------------------------------------------------------------------------

  TFile* f = new TFile(inFile);
  TTree* t = static_cast<TTree*>(f->Get("cbmsim"));

  std::vector<std::string> detectors{"Mvd", "Sts", "Rich", "Much", 
                                     "Trd", "Tof", "Psd"};

  std::vector<bool> useDetector{false, false, false, false,
                                       false, false, false, false};
  
  std::vector<TClonesArray*> tcl {ConnectBranchIfExist(f, t, "MvdPoint", "CbmMvdPoint"),
         ConnectBranchIfExist(f, t, "StsPoint", "CbmStsPoint"),
         ConnectBranchIfExist(f, t, "RichPoint", "CbmRichPoint"),
         ConnectBranchIfExist(f, t, "MuchPoint", "CbmMuchPoint"),
         ConnectBranchIfExist(f, t, "TrdPoint", "CbmTrdPoint"),
         ConnectBranchIfExist(f, t, "TofPoint", "CbmTofPoint"),
         ConnectBranchIfExist(f, t, "PsdPoint", "CbmPsdPoint")};

  for (int i=0; i < tcl.size(); ++i) {
    if (tcl[i]) useDetector[i] = true;
  } 

  Int_t events=t->GetEntriesFast();
  std::cout << "Number of events: " << events << std::endl;

  std::vector<Int_t> numPoints{0, 0, 0, 0, 0, 0, 0};
  for (Int_t j=0; j< events; j++) {
    t->GetEntry(j);
    for (int i=0; i < tcl.size(); ++i) {
      if (useDetector[i]) numPoints[i]+=tcl[i]->GetEntriesFast();
    }
  }

  std::for_each(numPoints.begin(), numPoints.end(), [&](int &n){ n/=events; });

  //Todo: Compare Results with benchmark results

  PrintResult(detectors, numPoints);

}

void PrintResult(std::vector<std::string>& detectors, 
                 std::vector<Int_t>& meanNrPoints)
{
  for (Int_t i=0; i<detectors.size(); ++i) {
    std::cout << "Mean number of " << detectors[i] << " points: " 
              << meanNrPoints[i] << std::endl;
  }
}

TClonesArray* ConnectBranchIfExist(TFile* f, TTree* t, TString branch, TString dataClass)
{
  // Find out wich output branches are available using 
  // the BranchList
  TClonesArray* bla{nullptr};
  TList* list= dynamic_cast <TList*> (f->Get("BranchList"));
  if(list) {
    for(Int_t i =0; i< list->GetEntries(); i++) {
      TObjString* Obj=dynamic_cast <TObjString*> (list->At(i));
      if(Obj){
        TString ObjName = Obj->GetString();
        if (ObjName.EqualTo(branch)) {
          bla = new TClonesArray(dataClass);
          t->SetBranchAddress(branch,&bla);
          break;
        }
      }
    }
  }
  return bla;
}

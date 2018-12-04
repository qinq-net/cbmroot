// Test macro which compares the mean number of MC points for the various
// detectors with benchmark values which were calculated using 100 central 
// UrQMD events. If the number of events is smaller than 10 the maximum deviation
// between the expected mean values and the actual mean values must be below
// 10%. If more than 10 events are used the maximum deviation must be bewlow
// 2% for all detectors except the RICH detector. Here the maximum deviation
// stays at 10%.

// forward declarations
TClonesArray* ConnectBranchIfExist(TFile*, TTree*, TString, TString);
void PrintResult(std::vector<std::string>&, std::vector<Int_t>&,
                 std::vector<Int_t>&, std::vector<Int_t>&);

void CompareResult(Int_t, TString, std::vector<std::string>&, std::vector<Int_t>&);

void WriteBenchmarkValues(Int_t, TString, std::vector<std::string>&, 
                     std::vector<Int_t>&, std::vector<Int_t>&,
                     std::vector<Int_t>& max);


void run_transport_qa(const char* setupName = "sis100_electron",
                      const char* output = "test")
{

 // -----   Environment   --------------------------------------------------
  TString myName = "run_transport_qa";  // this macro's name for screen output
  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory

  // -----   In- and output file names   ------------------------------------
  TString dataset(output);
  TString inFile = dataset + ".tra.root";
  std::cout << std::endl;

  std::cout << "-I- " << myName << ": Using input file " << inFile << std::endl;
  // ------------------------------------------------------------------------

  TFile* f = new TFile(inFile);
  TTree* t = static_cast<TTree*>(f->Get("cbmsim"));
  
  std::vector<std::string> detectors{"Mvd", "Sts", "Rich", "Much", 
                                     "Trd", "Tof", "Psd"};

  int imin = std::numeric_limits<int>::min();
  int imax = std::numeric_limits<int>::max();  

  std::vector<Int_t> numPoints{0, 0, 0, 0, 0, 0, 0};
  std::vector<Int_t> max{imin, imin, imin, imin, imin, imin, imin};
  std::vector<Int_t> min{imax, imax, imax, imax, imax, imax, imax};
  
  std::vector<TClonesArray*> tcl {
         ConnectBranchIfExist(f, t, "MvdPoint", "CbmMvdPoint"),
         ConnectBranchIfExist(f, t, "StsPoint", "CbmStsPoint"),
         ConnectBranchIfExist(f, t, "RichPoint", "CbmRichPoint"),
         ConnectBranchIfExist(f, t, "MuchPoint", "CbmMuchPoint"),
         ConnectBranchIfExist(f, t, "TrdPoint", "CbmTrdPoint"),
         ConnectBranchIfExist(f, t, "TofPoint", "CbmTofPoint"),
         ConnectBranchIfExist(f, t, "PsdPoint", "CbmPsdPoint")
         };

  std::vector<bool> useDetector{false, false, false, false,
                                       false, false, false, false};

  for (int i=0; i < tcl.size(); ++i) {
    if (tcl[i]) useDetector[i] = true;
  } 

  Int_t events=t->GetEntriesFast();
  std::cout << "Number of events: " << events << std::endl;

  for (Int_t j=0; j< events; j++) {
    t->GetEntry(j);
    for (int i=0; i < tcl.size(); ++i) {
      if (useDetector[i]) {
        Int_t entries = tcl[i]->GetEntriesFast();
        numPoints[i]+=entries;
        if (entries>max[i]) max[i] = entries;
        if (entries<min[i]) min[i] = entries;
      }
    }
  }

  delete t;
  delete f;

  std::for_each(numPoints.begin(), numPoints.end(), [&](int &n){ n/=events; });

//  PrintResult(detectors, numPoints, min, max);
  
  CompareResult(events, setupName, detectors, numPoints);

// Write new values to the file with the benchmark values 
//  WriteBenchmarkValues(events, setupName, detectors, numPoints, min, max);
}

void WriteBenchmarkValues(Int_t events, TString setupName, 
                     std::vector<std::string>& detectors, 
                     std::vector<Int_t>& meanPoints,
                     std::vector<Int_t>& min,
                     std::vector<Int_t>& max)
{
  Float_t mean, deviation1, deviation2;
  TNtuple *ntuple = new TNtuple(setupName,setupName,"mean:deviation1:deviation2");

  for (Int_t i=0; i<detectors.size(); ++i) {
    TString det{detectors[i]};
    if (det.EqualTo("Rich") || 0 == meanPoints[i]) {
      ntuple->Fill(meanPoints[i], 10, 10);
    } else {
      ntuple->Fill(meanPoints[i], 10, 2);
    }
  }

  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
  srcDir += "/input/qa/";
  TString benchmarkFile = srcDir + "QA_run_transport.root";

  TFile* bFile = new TFile(benchmarkFile, "UPDATE"); 

  ntuple->Write();
  bFile->Close();
  delete ntuple;
  delete bFile;
}

void PrintResult(std::vector<std::string>& detectors, 
                 std::vector<Int_t>& meanNrPoints,
                 std::vector<Int_t>& min,
                 std::vector<Int_t>& max)
{
  for (Int_t i=0; i<detectors.size(); ++i) {
    std::cout << "Number of entries(min/mean/max) for " << detectors[i] << " ("
              << min[i] << "/" << meanNrPoints[i] << "/" 
              << max[i] << ")" << std::endl;
    std::cout << "Number of entries(min/mean/max) for " << detectors[i] << " ("
              << static_cast<Float_t>(min[i])/static_cast<Float_t>(meanNrPoints[i]) << "/" 
              << static_cast<Float_t>(max[i])/static_cast<Float_t>(meanNrPoints[i]) 
              << ")" << std::endl;
  }
}


std::vector<std::tuple<Int_t,Int_t, Int_t>> ReadBenchmarkValues(TString setupName)
{
  TString srcDir = gSystem->Getenv("VMCWORKDIR");  // top source directory
  srcDir += "/input/qa/";
  TString benchmarkFile = srcDir + "QA_run_transport.root";

  TFile* bFile = new TFile(benchmarkFile); 
 
  TNtuple* ntuple = static_cast<TNtuple*>(bFile->Get(setupName));

  Float_t* values;

  std::vector<std::tuple<Int_t,Int_t, Int_t>> expected;
  for (int i=0; i < ntuple->GetEntriesFast(); ++i) {
    ntuple->GetEntry(i);
    values = ntuple->GetArgs();
    expected.emplace_back(make_tuple(values[0],values[1],values[2]));
  }
  delete ntuple;
  delete bFile;

  return expected;
}

void CompareResult(Int_t entries, TString setupName,
                   std::vector<std::string>& detectors, 
                   std::vector<Int_t>& meanPoints)
{

  std::vector<std::tuple<Int_t,Int_t, Int_t>> expected =
     ReadBenchmarkValues(setupName);

  Bool_t allOk{kTRUE};

  stringstream ss;

  for (Int_t i=0; i<detectors.size(); ++i) {
    Int_t expectedMean = get<0>(expected[i]);
    Int_t maxDeviation{0};
    if (entries < 10) {
      maxDeviation = get<1>(expected[i]);
    } else {
      maxDeviation = get<2>(expected[i]);
    }
    Float_t deviation{0.};
    if (meanPoints[i] > 0) {
      deviation = static_cast<Float_t>(meanPoints[i])/static_cast<Float_t>(expectedMean); 
      deviation = TMath::Abs(1. - deviation)*100;
    }

    std::cout << "<DartMeasurement name=" << detectors[i] << " type=\"numeric/double\">";
    std::cout << deviation;
    std::cout << "</DartMeasurement>" << std::endl;

    if (deviation < maxDeviation) {
      ss << "Deviation for detector " << detectors[i] << " with "
         << deviation << "% is less than " << maxDeviation 
         << "%" << std::endl;
    } else {
      allOk=kFALSE;
      ss << "Deviation for detector " << detectors[i] << " with "
         << deviation << "% is larger than " << maxDeviation 
         << "%" << std::endl;
    }
  }
  if (allOk) {
    std::cout << " Test passed" << std::endl;
    std::cout << " All ok " << std::endl;
  } else {
    std::cout << " Test failed" << std::endl;
    std::cout << " ***** " << std::endl;
    std::cout << ss.str() << std::endl;
  }
}


TClonesArray* ConnectBranchIfExist(TFile* f, TTree* t, TString branch, TString dataClass)
{
  // Find out wich output branches are available using 
  // the BranchList
  TClonesArray* tcl{nullptr};
  TList* list= dynamic_cast <TList*> (f->Get("BranchList"));
  if(list) {
    for(Int_t i =0; i< list->GetEntries(); i++) {
      TObjString* Obj=dynamic_cast <TObjString*> (list->At(i));
      if(Obj){
        TString ObjName = Obj->GetString();
        if (ObjName.EqualTo(branch)) {
          tcl = new TClonesArray(dataClass);
          t->SetBranchAddress(branch,&tcl);
          break;
        }
      }
    }
  }
  return tcl;
}

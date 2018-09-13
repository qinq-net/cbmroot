#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <vector> 

#include "TTree.h"
#include "TFile.h"
#include "TClonesArray.h"

// include data class
#include "CbmTofDigiExp.h"

using namespace std;

int main()
{

  // serialize vector
  {
    std::vector<CbmTofDigiExp*> v;

    TFile *f = TFile::Open("/tmp/hvector.root","RECREATE");

    // Create a TTree
    TTree *t = new TTree("tvec","Tree with vectors");
    t->Branch("CbmTofDigiExp",&v);

    v.emplace_back(new CbmTofDigiExp{1, 1., 1.});
    v.emplace_back(new CbmTofDigiExp{2, 2., 2.});
    v.emplace_back(new CbmTofDigiExp{3, 3., 3.});

    t->Fill();

    f->Write();
  }


// Unfortunately does not work
//
/*
  {
    TFile *f = new TFile("/tmp/hvector.root");
    TTree *T = (TTree*)f->Get("tvec");

    TClonesArray *arr = new TClonesArray("CbmTofDigiExp");
    T->GetBranch("CbmTofDigiExp")->SetAutoDelete(kFALSE);
    T->SetBranchAddress("CbmTofDigiExp",&arr);
    Long64_t nentries = T->GetEntries();
    std::cout << "Number of entries" << nentries << std::endl;
    for (Long64_t ev=0;ev<nentries;ev++) {
      arr->Clear();
      T->GetEntry(ev);
      Int_t nDigis = arr->GetEntriesFast();
      for (Int_t i=0;i<nDigis;i++) {
         CbmTofDigiExp* digi = static_cast<CbmTofDigiExp*>(arr->At(i));
         digi->Print();
      }
    }
  }
*/

  {
    TFile *f = TFile::Open("/tmp/hvector.root","READ");

    std::cout << "Size of CbmTofDigiExp: " << sizeof(CbmTofDigiExp) << std::endl;  

    if (!f) { return 1; }

    TTree* t{nullptr}; 
    f->GetObject("tvec",t);

    if (!t) { return 1; }

    std::vector<CbmTofDigiExp*>* v2 = nullptr;

    TBranch* bv2 = nullptr;
    t->SetBranchAddress("CbmTofDigiExp",&v2,&bv2);

    Long64_t nentries = t->GetEntries();
    std::cout << "Number of entries" << nentries << std::endl;

    for (Long64_t ev=0;ev<nentries;ev++) {
      Long64_t tentry = t->LoadTree(ev);
      bv2->GetEntry(tentry);

      for (UInt_t j = 0; j < v2->size(); ++j) {
         CbmTofDigiExp* digi = v2->at(j);
//         std::cout << digi->Print() << std::endl;
         std::cout << digi->ToString() << std::endl;
      }
    }
  }

  return 0;
}
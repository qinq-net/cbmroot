// -------------------------------------------------------------------------
// -----                CbmPsdHitProducer source file             -----
// -----                  Created 15/05/12  by     Alla & SELIM               -----
// -------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <map>

#include "TClonesArray.h"
#include "TMath.h"

#include "FairRootManager.h"

#include "CbmPsdDigi.h"
#include "CbmPsdHitProducer.h"
#include "CbmPsdHit.h"

#include "FairLogger.h"

using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmPsdHitProducer::CbmPsdHitProducer() :
  FairTask("Ideal Psd Hit Producer",1),
  fNHits(0),
  fHitArray(NULL),
  fDigiArray(NULL),
  fXi(),
  fYi(),
  fhModXNewEn(NULL)
{ 
  //  Reset();
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmPsdHitProducer::~CbmPsdHitProducer() 
{
  if ( fDigiArray ) {
    fDigiArray->Delete();
    delete fDigiArray;
  }
  if ( fHitArray ) {
    fHitArray->Delete();
    delete fHitArray;
  }
}
// -------------------------------------------------------------------------



// -----   Public method Init   --------------------------------------------
InitStatus CbmPsdHitProducer::Init() {

  //ifstream fxypos("psd_geo_xy.txt");
  //for (Int_t ii=1; ii<50; ii++) { //SELIM: 49 modules, including central & corner modules (rejected in analysis/flow/eventPlane.cxx)
  //    fxypos>>fXi[ii]>>fYi[ii];
  //    cout<<ii<<" "<<fXi[ii]<<" "<<fYi[ii]<<endl;
  //}
  //fxypos.close();

  fhModXNewEn = new TH1F("hModXNewEn","X distr, En",300,-150.,150.);
  fhModXNewEn->Print();

  // Get RootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman )
  {
    LOG(FATAL) << "-W- CbmPsdHitProducer::Init: RootManager not instantised!" << FairLogger::endl;    //FLORIAN & SELIM
    return kFATAL;
  }

  // Get input array
  fDigiArray = (TClonesArray*) ioman->GetObject("PsdDigi");
  if ( ! fDigiArray )
  {
    LOG(FATAL) << "-E- CbmPsdHitProducer::Init: No PSD digits array!" << FairLogger::endl;    //FLORIAN & SELIM
    return kFATAL;
  }

  // Create and register output array
  fHitArray = new TClonesArray("CbmPsdHit", 1000);
  ioman->Register("PsdHit", "PSD", fHitArray, IsOutputBranchPersistent("PsdHit"));

  fHitArray->Dump();
  cout << "-I- CbmPsdHitProducer: Intialisation successfull " << kSUCCESS<< endl;
  return kSUCCESS;
}


// -------------------------------------------------------------------------



// -----   Public method Exec   --------------------------------------------
void CbmPsdHitProducer::Exec(Option_t* /*opt*/) {

  cout<<" CbmPsdHitProducer::Exec(Option_t* /*opt*/) "<<endl;
  fhModXNewEn->Print();

  // Reset output array
  if ( ! fDigiArray ) Fatal("Exec", "No PsdDigi array");
  Reset();

  // Declare some variables
  CbmPsdDigi* dig = NULL;

  Double_t edep[NPsdMod];//marina
  for (Int_t imod=0; imod<NPsdMod; imod++)  { edep[imod]=0.; }//marina

  std::map<int, Double_t> edepmap;

  // Loop over PsdDigits
  Int_t nDigi = fDigiArray->GetEntriesFast();
  cout<<" nDigits "<<nDigi<<endl;

  for (Int_t idig=0; idig<nDigi; idig++)
  {
    dig = (CbmPsdDigi*) fDigiArray->At(idig);
    if ( ! dig) continue;
    Int_t mod = dig->GetModuleID();
    Int_t sec = dig->GetSectionID();
    Double_t eDep = (Double_t) dig->GetEdep();
    //edep[mod-1] += (Double_t) dig->GetEdep();                     //DEBUG: SELIM

    auto insert_result = edepmap.insert(std::make_pair(mod, eDep));
    if (!insert_result.second) { // entry was here before
      (*insert_result.first).second += eDep;
    }

  }// Loop over MCPoints

  fNHits = 0;
  for (auto edep_entry : edepmap) {
    int modID = edep_entry.first;
    Double_t eDep = edep_entry.second;
    new ((*fHitArray)[fNHits]) CbmPsdHit(modID, eDep);
    fNHits++;
  }

  /*
  for (Int_t imod=0; imod<NPsdMod; imod++) //marina
  {              
    if (edep[imod]>0.)
    {
      new ((*fHitArray)[fNHits]) CbmPsdHit(imod+1, edep[imod]);
      fNHits++;
      //cout<<"MARINA CbmPsdHitProducer " <<fNHits <<" " <<imod+1 <<" " <<edep[imod] <<endl;
      //fhModXNewEn->Fill(fXi[imod],TMath::Sqrt(edep[imod]) );
      //cout<<"CbmPsdHitProducer "<<fNHits<<" "<<imod<<" "<<edep[imod]<<endl;
    }
  }
  */

  // Event summary
  cout << "-I- CbmPsdHitProducer: " <<fNHits<< " CbmPsdHits created." << endl;

}
// -------------------------------------------------------------------------
void CbmPsdHitProducer::Finish()
{
  cout<<" CbmPsdHitProducer::Finish() "<<endl;
  TFile * outfile = new TFile("EdepHistos.root","RECREATE");
  outfile->cd();
  fhModXNewEn->Write();
  //outfile->Close();                     //SELIM
}

// -----   Private method Reset   ------------------------------------------
void CbmPsdHitProducer::Reset() {
  fNHits = 0;
  if ( fHitArray ) fHitArray->Delete();

}


ClassImp(CbmPsdHitProducer)

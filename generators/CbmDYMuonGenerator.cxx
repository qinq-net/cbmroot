// -------------------------------------------------------------------------
// -----                CbmDYMuonGenerator source file                  -----
// -----Created by Dr. Partha Pratim Bhaduri <partha.bhaduri@vecc.gov.in>          -----
// -------------------------------------------------------------------------
#include "CbmDYMuonGenerator.h"
#include "FairPrimaryGenerator.h"

#include "TClonesArray.h"
#include "TDatabasePDG.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TBranch.h"
#include "TVector3.h"
#include "PParticle.h"

#include <iostream>

// -----   Default constructor   ------------------------------------------
CbmDYMuonGenerator::CbmDYMuonGenerator()
  :FairGenerator(),
   iEvent(0),
   fFileName(NULL),
   fInputFile(NULL),
   fInputTree(NULL),

   b_Run(NULL),
   b_Event(NULL),
   b_mass(NULL),
   b_trackNo(NULL),
   b_Px(NULL),
   b_Py(NULL),
   b_Pz(NULL),
   b_En(NULL)
  
{
  /*
  iEvent     = 0;
  fInputFile = NULL;
  fInputTree = NULL;
  */
}
// ------------------------------------------------------------------------



// -----   Standard constructor   -----------------------------------------
CbmDYMuonGenerator::CbmDYMuonGenerator(const Char_t* fileName)
  :FairGenerator(),
   iEvent(0),
   fFileName(fileName),
   fInputFile(new TFile(fileName)),
   fInputTree(NULL),

   b_Run(NULL),
   b_Event(NULL),
   b_mass(NULL),
   b_trackNo(NULL),
   b_Px(NULL),
   b_Py(NULL),
   b_Pz(NULL),
   b_En(NULL)
   
{
  
  fInputTree = (TTree*) fInputFile->Get("Tree");

  fInputTree->SetBranchAddress("Run", &Run, &b_Run);
  fInputTree->SetBranchAddress("Event", &Event, &b_Event);
  fInputTree->SetBranchAddress("trackNo", &trackNo, &b_trackNo);
  fInputTree->SetBranchAddress("MDY", &MDY, &b_mass);
  fInputTree->SetBranchAddress("Px", Px, &b_Px);
  fInputTree->SetBranchAddress("Py", Py, &b_Py);
  fInputTree->SetBranchAddress("Pz", Pz, &b_Pz);
  fInputTree->SetBranchAddress("En", En, &b_En);


}
// ------------------------------------------------------------------------



// -----   Destructor   ---------------------------------------------------
CbmDYMuonGenerator::~CbmDYMuonGenerator()
{
  CloseInput();
}
// ------------------------------------------------------------------------



// -----   Public method ReadEvent   --------------------------------------
Bool_t CbmDYMuonGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{

  // Check for input file
  if ( ! fInputFile ) {
    cout << "-E CbmDYMuonGenerator: Input file nor open!" << endl;
    return kFALSE;
  }

  // Check for number of events in input file
  if ( iEvent > fInputTree->GetEntries() ) {
    cout << "-E CbmDYMuonGenerator: No more events in input file!" << endl;
    CloseInput();
    return kFALSE;
  }
  TFile*  g=gFile;
  fInputFile->cd();
  fInputTree->GetEntry(iEvent);
  g->cd();

  Int_t nParts = trackNo;
    Double_t mass=MDY;
  // Loop over particles in TClonesArray
  for (Int_t iPart=0; iPart < nParts; iPart++) {
   
   Double_t px=Px[iPart],py=Py[iPart],pz=Pz[iPart],ee=En[iPart];
   
   // Track vertices are all set to zero 
    Double_t vx=0.0,vy=0.0,vz=0.0;
    Int_t pdgType=0;

    if (iPart==0) pdgType =-13;
    if (iPart==1) pdgType=13;

   

    // Add track to PrimaryGenerator
    primGen->AddTrack(pdgType, px, py, pz, vx, vy, vz);

  }        //  Loop over particle in event

   iEvent++;

 return kTRUE;

}
// ------------------------------------------------------------------------




// -----   Private method CloseInput   ------------------------------------
void CbmDYMuonGenerator::CloseInput()
{
  if ( fInputFile ) {
    cout << "-I CbmDYMuonGenerator: Closing input file " << fFileName
         << endl;
    fInputFile->Close();
    delete fInputFile;
  }
  fInputFile = NULL;
}
// ------------------------------------------------------------------------



ClassImp(CbmDYMuonGenerator)

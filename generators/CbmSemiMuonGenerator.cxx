// -------------------------------------------------------------------------
// -----                CbmSemiMuonGenerator source file                  -----
// -----          Created Partha Pratim Bhaduri          -----
// -------------------------------------------------------------------------
#include "CbmSemiMuonGenerator.h"

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
CbmSemiMuonGenerator::CbmSemiMuonGenerator()
  :FairGenerator(),
   iEvent(0),
   fFileName(NULL),
   fInputFile(NULL),
   fInputTree(NULL),

   b_Run(NULL),
   b_Event(NULL),
   b_trackNo(NULL),
   b_Px(NULL),
   b_Py(NULL),
   b_Pz(NULL),
   b_En(NULL),
   
   Run(0),
   Event(0),
   trackNo(0)

{
}
  
// ------------------------------------------------------------------------



// -----   Standard constructor   -----------------------------------------
CbmSemiMuonGenerator::CbmSemiMuonGenerator(const Char_t* fileName)
  :FairGenerator(),
   iEvent(0),
   fFileName(fileName),
   fInputFile(new TFile(fileName)),
   fInputTree(NULL),

   b_Run(NULL),
   b_Event(NULL),
   b_trackNo(NULL),
   b_Px(NULL),
   b_Py(NULL),
   b_Pz(NULL),
   b_En(NULL),
   
   Run(0),
   Event(0),
   trackNo(0)
{
 
  fInputTree = (TTree*) fInputFile->Get("Tree");
  
  fInputTree->SetBranchAddress("Run", &Run, &b_Run);
  fInputTree->SetBranchAddress("Event", &Event, &b_Event);
  fInputTree->SetBranchAddress("trackNo", &trackNo, &b_trackNo);
  fInputTree->SetBranchAddress("Px", Px, &b_Px);
  fInputTree->SetBranchAddress("Py", Py, &b_Py);
  fInputTree->SetBranchAddress("Pz", Pz, &b_Pz);
  fInputTree->SetBranchAddress("En", En, &b_En);


}
// ------------------------------------------------------------------------



// -----   Destructor   ---------------------------------------------------
CbmSemiMuonGenerator::~CbmSemiMuonGenerator()
{
  CloseInput();
}
// ------------------------------------------------------------------------



// -----   Public method ReadEvent   --------------------------------------
Bool_t CbmSemiMuonGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{

  // Check for input file
  if ( ! fInputFile ) {
    cout << "-E CbmSemiMuonGenerator: Input file nor open!" << endl;
    return kFALSE;
  }

  // Check for number of events in input file
  if ( iEvent > fInputTree->GetEntries() ) {
    cout << "-E CbmSemiMuonGenerator: No more events in input file!" << endl;
    CloseInput();
    return kFALSE;
  }
  TFile*  g=gFile;
  fInputFile->cd();
  fInputTree->GetEntry(iEvent++);
  g->cd();

  

  Int_t nParts = trackNo;


  // Loop over particles in TClonesArray
  for (Int_t iPart=0; iPart < nParts; iPart++) {
   

    Double_t px=Px[iPart],py=Py[iPart],pz=Pz[iPart],ee=En[iPart];
    
    Double_t vx=0.0,vy=0.0,vz=0.0;
    Int_t pdgType=0;

    if (iPart==0) pdgType =-13; // change to -11 for e-
    if (iPart==1) pdgType=13;  // change to 11 for e+

  
    // Give track to PrimaryGenerator
    primGen->AddTrack(pdgType, px, py, pz, vx, vy, vz);

  }       //  Loop over particle in event

  return kTRUE;

}
// ------------------------------------------------------------------------




// -----   Private method CloseInput   ------------------------------------
void CbmSemiMuonGenerator::CloseInput()
{
  if ( fInputFile ) {
    cout << "-I CbmSemiMuonGenerator: Closing input file " << fFileName
         << endl;
    fInputFile->Close();
    delete fInputFile;
  }
  fInputFile = NULL;
}
// ------------------------------------------------------------------------



ClassImp(CbmSemiMuonGenerator)

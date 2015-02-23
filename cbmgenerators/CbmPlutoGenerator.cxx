// -------------------------------------------------------------------------
// -----                CbmPlutoGenerator source file                  -----
// -----          Created 13/07/04  by V. Friese / D.Bertini           -----
// -------------------------------------------------------------------------
#include "CbmPlutoGenerator.h"

#include "FairPrimaryGenerator.h"       // for FairPrimaryGenerator

#include "PParticle.h"                  // for PParticle

#include "Riosfwd.h"                    // for ostream
#include "TClonesArray.h"               // for TClonesArray
#include "TDatabasePDG.h"               // for TDatabasePDG
#include "TFile.h"                      // for TFile, gFile
#include "TLorentzVector.h"             // for TLorentzVector
#include "TTree.h"                      // for TTree
#include "TVector3.h"                   // for TVector3

#include <stddef.h>                     // for NULL
#include <iostream>                     // for operator<<, basic_ostream, etc

// -----   Default constructor   ------------------------------------------
CbmPlutoGenerator::CbmPlutoGenerator()
  :FairGenerator(),
   iEvent(0),
   fFileName(NULL),
   fInputFile(NULL),
   fInputTree(NULL),
   fParticles(NULL)
{
  /*
  iEvent     = 0;
  fInputFile = NULL;
  fInputTree = NULL;
  */
}
// ------------------------------------------------------------------------



// -----   Standard constructor   -----------------------------------------
CbmPlutoGenerator::CbmPlutoGenerator(const Char_t* fileName)
  :FairGenerator(),
   iEvent(0),
   fFileName(fileName),
   fInputFile(new TFile(fileName)),
   fInputTree(NULL),
   fParticles(new TClonesArray("PParticle",100))
{
  /*
  iEvent     = 0;
  fFileName  = fileName;
  fInputFile = new TFile(fFileName);
  */
  fInputTree = (TTree*) fInputFile->Get("data");
  //  fParticles = new TClonesArray("PParticle",100);
  fInputTree->SetBranchAddress("Particles", &fParticles);
}
// ------------------------------------------------------------------------



// -----   Destructor   ---------------------------------------------------
CbmPlutoGenerator::~CbmPlutoGenerator()
{
  CloseInput();
}
// ------------------------------------------------------------------------



// -----   Public method ReadEvent   --------------------------------------
Bool_t CbmPlutoGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{

  // Check for input file
  if ( ! fInputFile ) {
    cout << "-E CbmPlutoGenerator: Input file nor open!" << endl;
    return kFALSE;
  }

  // Check for number of events in input file
  if ( iEvent > fInputTree->GetEntries() ) {
    cout << "-E CbmPlutoGenerator: No more events in input file!" << endl;
    CloseInput();
    return kFALSE;
  }
  TFile*  g=gFile;
  fInputFile->cd();
  fInputTree->GetEntry(iEvent++);
  g->cd();

  // Get PDG database
  TDatabasePDG* dataBase = TDatabasePDG::Instance();

  // Get number of particle in TClonesrray
  Int_t nParts = fParticles->GetEntriesFast();

  // Loop over particles in TClonesArray
  for (Int_t iPart=0; iPart < nParts; iPart++) {
    PParticle* part = (PParticle*) fParticles->At(iPart);
    Int_t pdgType = dataBase->ConvertGeant3ToPdg( part->ID() );

    // Check if particle type is known to database
    if ( ! pdgType ) {
      cout << "-W CbmPlutoGenerator: Unknown type " << part->ID()
           << ", skipping particle." << endl;
      continue;
    }

    TLorentzVector mom = part->Vect4();
    Double_t px = mom.Px();
    Double_t py = mom.Py();
    Double_t pz = mom.Pz();

    TVector3 vertex = part->getVertex();
    Double_t vx = vertex.x();
    Double_t vy = vertex.y();
    Double_t vz = vertex.z();

    // Give track to PrimaryGenerator
    primGen->AddTrack(pdgType, px, py, pz, vx, vy, vz);

  }        //  Loop over particle in event

  return kTRUE;

}
// ------------------------------------------------------------------------




// -----   Private method CloseInput   ------------------------------------
void CbmPlutoGenerator::CloseInput()
{
  if ( fInputFile ) {
    cout << "-I CbmPlutoGenerator: Closing input file " << fFileName
         << endl;
    fInputFile->Close();
    delete fInputFile;
  }
  fInputFile = NULL;
}
// ------------------------------------------------------------------------



ClassImp(CbmPlutoGenerator)

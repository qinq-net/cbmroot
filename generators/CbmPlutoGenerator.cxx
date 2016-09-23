// -------------------------------------------------------------------------
// -----                CbmPlutoGenerator source file                  -----
// -----          Created 13/07/04  by V. Friese / D.Bertini           -----
// -------------------------------------------------------------------------
#include "CbmPlutoGenerator.h"

#include "FairPrimaryGenerator.h"       // for FairPrimaryGenerator
#include "FairLogger.h"

#include "PStaticData.h"                // for PStaticData
#include "PDataBase.h"                  // for PDataBase
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
   fdata(makeStaticData()),
   fbase(makeDataBase()),
   iEvent(0),
   fFileName(NULL),
   fInputFile(NULL),
   fInputTree(NULL),
   fParticles(NULL),
   fPDGmanual(0)
{
}
// ------------------------------------------------------------------------



// -----   Standard constructor   -----------------------------------------
CbmPlutoGenerator::CbmPlutoGenerator(const Char_t* fileName)
  :FairGenerator(),
   fdata(makeStaticData()),
   fbase(makeDataBase()),
   iEvent(0),
   fFileName(fileName),
   fInputFile(new TFile(fileName)),
   fInputTree(NULL),
   fParticles(new TClonesArray("PParticle",100)),
   fPDGmanual(0)
{
  fInputTree = (TTree*) fInputFile->Get("data");
  fInputTree->SetBranchAddress("Particles", &fParticles);
}
// ------------------------------------------------------------------------



// -----   Destructor   ---------------------------------------------------
CbmPlutoGenerator::~CbmPlutoGenerator()
{
  // remove Pluto database
  delete fdata;
  delete fbase;
  CloseInput();
}
// ------------------------------------------------------------------------



// -----   Public method ReadEvent   --------------------------------------
Bool_t CbmPlutoGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{

  // Check for input file
  if ( ! fInputFile ) {
    LOG(ERROR) << "CbmPlutoGenerator: Input file nor open!" << FairLogger::endl;
    return kFALSE;
  }

  // Check for number of events in input file
  if ( iEvent > fInputTree->GetEntries() ) {
    LOG(ERROR) << "CbmPlutoGenerator: No more events in input file!" << FairLogger::endl;
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

    Int_t *pdgType = 0x0;
    Bool_t found = fbase->GetParamInt("pid",part->ID(),"pythiakf",&pdgType);
    // TODO: replace by fdata->GetParticleKF(part->ID()); as soon as FairSoft uses pluto version 5.43 or higher and remove fbase

    // Check if particle type is known to database
    if ( ! found ) {
      LOG(WARNING) << "CbmPlutoGenerator: Unknown type " << part->ID() 
                   << ", skipping particle." << FairLogger::endl;
      continue;
    }
    LOG(INFO) << iPart << " Particle (geant" << part->ID() << " PDG " 
              << *pdgType << " -> " << dataBase->GetParticle(*pdgType)->GetName()  
              << FairLogger::endl;
    //Info("ReadEvent"," %d Particle (geant%d) PDG %d -> %s", iPart,part->ID(),*pdgType, dataBase->GetParticle(*pdgType)->GetName());

    // set PDG by hand for pluto dilepton pairs and other not defined codes in pluto
    Int_t dielectron=99009911;
    Int_t dimuon    =99009913;
    if(fPDGmanual && *pdgType==0) {
      pdgType=&fPDGmanual;
      LOG(WARNING) << "\t PDG code changed by user defintion to " << *pdgType
                   << FairLogger::endl; 
      //Warning("ReadEvent","\t PDG code changed by user defintion to %d",*pdgType);
      //      Printf(" \t PDG changed to %d -> %s",*pdgType,dataBase->GetParticle(*pdgType)->GetName());
    }
    else if(part->ID()==51) pdgType=&dielectron;
    else if(part->ID()==50) pdgType=&dimuon;

    // get the mother
    Int_t parIdx = part->GetParentIndex();
    // get daughter
    Int_t idx = part->GetDaughterIndex();

    TLorentzVector mom = part->Vect4();
    Double_t px = mom.Px();
    Double_t py = mom.Py();
    Double_t pz = mom.Pz();
    Double_t ee = mom.E();

    TVector3 vertex = part->getVertex();
    Double_t vx = vertex.x();
    Double_t vy = vertex.y();
    Double_t vz = vertex.z();

    Bool_t wanttracking = kTRUE;
    if(idx>-1) wanttracking=kFALSE; // only tracking for decay products
    Int_t parent = parIdx;
    LOG(INFO) << "\t Add particle with parent at index " << parIdx 
              << " and do tracking " << wanttracking << FairLogger::endl;
    //Info("ReadEvent","\t Add particle with parent at index %d and do tracking %d",parIdx,wanttracking);
    //    part->Print();

    // Give track to PrimaryGenerator
    primGen->AddTrack(*pdgType, px, py, pz, vx, vy, vz, parent, wanttracking, ee);

  }        //  Loop over particle in event

  return kTRUE;

}
// ------------------------------------------------------------------------




// -----   Private method CloseInput   ------------------------------------
void CbmPlutoGenerator::CloseInput()
{
  if ( fInputFile ) {
    LOG(INFO) << "CbmPlutoGenerator: Closing input file " << fFileName
              << FairLogger::endl;
    fInputFile->Close();
    delete fInputFile;
  }
  fInputFile = NULL;
}
// ------------------------------------------------------------------------



ClassImp(CbmPlutoGenerator)

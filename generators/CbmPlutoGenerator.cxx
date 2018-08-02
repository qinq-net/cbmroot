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

#include <iosfwd>                       // for ostream
#include "TClonesArray.h"               // for TClonesArray
#include "TDatabasePDG.h"               // for TDatabasePDG
#include "TFile.h"                      // for TFile
#include "TChain.h"                     // for TChain
#include "TLorentzVector.h"             // for TLorentzVector
#include "TTree.h"                      // for TTree
#include "TVector3.h"                   // for TVector3

#include <stddef.h>                     // for NULL
#include <iostream>                     // for operator<<, basic_ostream, etc
#include <sys/stat.h>

// -----   Default constructor   ------------------------------------------
CbmPlutoGenerator::CbmPlutoGenerator()
  :FairGenerator(),
   fdata(makeStaticData()),
   fbase(makeDataBase()),
   iEvent(0),
   fFileName(NULL),
   fInputChain(NULL),
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
   fInputChain(NULL),
   fParticles(new TClonesArray("PParticle",100)),
   fPDGmanual(0)
{
  fInputChain = new TChain("data");
  CheckFileExist(fileName);

  fInputChain->Add(fileName); 
  fInputChain->SetBranchAddress("Particles", &fParticles); 
}
// ------------------------------------------------------------------------

// -----  Constructor with file list   -----------------------------------------
CbmPlutoGenerator::CbmPlutoGenerator(std::vector<std::string> fileNames)
  :FairGenerator(),
   fdata(makeStaticData()),
   fbase(makeDataBase()),
   iEvent(0),
   fFileName(),
   fInputChain(NULL),
   fParticles(new TClonesArray("PParticle",100)),
   fPDGmanual(0)
{
  fInputChain = new TChain("data");
  for(auto & name: fileNames) {
    CheckFileExist(name);
    fInputChain->Add(name.c_str()); 
  }
  fInputChain->SetBranchAddress("Particles", &fParticles); 
}


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
  if ( ! fInputChain ) {
    LOG(ERROR) << "CbmPlutoGenerator: Input file not open!" << FairLogger::endl;
    return kFALSE;
  }

  // Check for number of events in input file
  if ( iEvent > fInputChain->GetEntries() ) {
    LOG(ERROR) << "CbmPlutoGenerator: No more events in input file!" << FairLogger::endl;
    CloseInput();
    return kFALSE;
  }
  fInputChain->GetEntry(iEvent++);

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
    LOG(INFO) << iPart << " Particle (geant " << part->ID() << " PDG " 
              << *pdgType << " -> " << dataBase->GetParticle(*pdgType)->GetName()  
              << FairLogger::endl;

    // set PDG by hand for pluto dilepton pairs and other not defined codes in pluto
    Int_t dielectron=99009911;
    Int_t dimuon    =99009913;
    if(fPDGmanual && *pdgType==0) {
      pdgType=&fPDGmanual;
      LOG(WARNING) << "PDG code changed by user defintion to " << *pdgType
                   << FairLogger::endl; 
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
    LOG(INFO) << "Add particle with parent at index " << parIdx 
              << " and do tracking " << wanttracking << FairLogger::endl;

    // Give track to PrimaryGenerator
    primGen->AddTrack(*pdgType, px, py, pz, vx, vy, vz, parent, wanttracking, ee);

  }        //  Loop over particle in event

  return kTRUE;

}
// ------------------------------------------------------------------------




// -----   Private method CloseInput   ------------------------------------
void CbmPlutoGenerator::CloseInput()
{
  if ( fInputChain ) {
    LOG(INFO) << "CbmPlutoGenerator: Closing input file " << fFileName
              << FairLogger::endl;
    delete fInputChain;
  }
  fInputChain = NULL;
}
// ------------------------------------------------------------------------

void CbmPlutoGenerator::CheckFileExist(std::string filename)
{
  struct stat buffer;   
  if(stat (filename.c_str(), &buffer) == 0) { 
    LOG(INFO) << "CbmPlutoGenerator: Add file " << filename << " to input chain" << FairLogger::endl;
  } else {
    LOG(FATAL) << "Input File " << filename << " not found" << FairLogger::endl;
  }
}

ClassImp(CbmPlutoGenerator)

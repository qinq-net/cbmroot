// -------------------------------------------------------------------------
// -----                     CbmUnigenGenerator                        -----
// -----              Created 2006/Jul/04  by D. Kresan                -----
// -----              Switch to FairLogger 2013/Oct/31 by DE           -----
// -------------------------------------------------------------------------

#include "CbmUnigenGenerator.h"

#include "FairMCEventHeader.h"

#include "FairLogger.h"
#include "FairPrimaryGenerator.h"
#include "FairRunSim.h"

#include "UEvent.h"
#include "UParticle.h"
#include "URun.h"

#include "TFile.h"
#include "TRandom.h"
#include "TTree.h"

#include <iostream>

using namespace std;

// ------------------------------------------------------------------------
CbmUnigenGenerator::CbmUnigenGenerator()
    : FairGenerator(), fEvents(0), fInputFile(NULL), fFileName(""),
      fInTree(NULL), fEvent(NULL), fCM(kFALSE), fBetaCM(0.), fGammaCM(1.),
      fPhiMin(0.), fPhiMax(0.), fEventPlaneSet(kFALSE), fIonMap() {}
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
CbmUnigenGenerator::CbmUnigenGenerator(TString fileName)
    : FairGenerator(), fEvents(0), fInputFile(NULL), fFileName(fileName),
      fInTree(NULL), fEvent(NULL), fCM(kFALSE), fBetaCM(0.), fGammaCM(0.),
      fPhiMin(0.), fPhiMax(0.), fEventPlaneSet(kFALSE), fIonMap() {
  LOG(INFO) << "CbmUnigenGenerator: Opening input file " << fFileName.Data()
            << FairLogger::endl;
  fInputFile = new TFile(fFileName);
  if (NULL == fInputFile) {
    LOG(FATAL) << "CbmUnigenGenerator: Cannot open input file."
               << FairLogger::endl;
  }

  // Get run description
  URun *run = (URun *)fInputFile->Get("run");
  if (NULL == run) {
    LOG(FATAL) << "CbmUnigenGenerator: No run description in input file."
               << FairLogger::endl;
  }
  fCM = kFALSE;
  LOG(DEBUG) << "Target Momentum: " << run->GetPTarg() << FairLogger::endl;
  if (TMath::Abs(run->GetPTarg()) > 0.001) {
    fCM = kTRUE;
  }
  fBetaCM = 0.;
  fGammaCM = 1.;
  if (fCM) {
    LOG(INFO) << "Input data is in CM frame" << FairLogger::endl;
    Double_t elab = (TMath::Power(run->GetNNSqrtS(), 2) -
                     2 * TMath::Power(0.938271998, 2)) /
                    (2 * 0.938271998);
    Double_t plab = TMath::Sqrt(elab * elab - TMath::Power(0.938271998, 2));
    LOG(INFO) << "CbmUnigenGenerator: Plab = " << plab << " AGeV"
              << FairLogger::endl;
    fBetaCM = plab / (elab + 0.938271998);
    fGammaCM = 1. / TMath::Sqrt(1. - fBetaCM * fBetaCM);
    LOG(INFO) << "CbmUnigenGenerator: boost parameters: "
              << "betaCM = " << fBetaCM << ", gammaCM = " << fGammaCM
              << FairLogger::endl;
    // store the beam momentum in FairRunSim, it will be transfered to
    // FairBaseParSet in FairRunSim::Init
    FairRunSim::Instance()->SetBeamMom(plab);
  } else {
    LOG(INFO) << "Input data is in LAB frame" << FairLogger::endl;
  }

  delete run;
  fInTree = (TTree *)fInputFile->Get("events");
  if (NULL == fInTree) {
    LOG(FATAL) << "CbmUnigenGenerator: No event tree in input file."
               << FairLogger::endl;
  }
  fEvent = new UEvent();
  fInTree->SetBranchAddress("event", &fEvent);
  Int_t nIons = RegisterIons();
  cout << "-I- CbmUnigenGenerator: " << nIons << " ions registered." << endl;
}
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
CbmUnigenGenerator::~CbmUnigenGenerator() { CloseInput(); }
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
Bool_t CbmUnigenGenerator::ReadEvent(FairPrimaryGenerator *primGen) {
  // Check for input file
  if (NULL == fInputFile || NULL == fInTree) {
    LOG(ERROR) << "CbmUnigenGenerator: Input file is not opened!"
               << FairLogger::endl;
    return kFALSE;
  }

  // If end of input file is reached : close it and abort run
  if (fEvents >= fInTree->GetEntries()) {
    LOG(INFO) << "CbmUnigenGenerator: End of input file reached"
              << FairLogger::endl;
    CloseInput();
    return kFALSE;
  }

  // Read entry
  fInTree->GetEntry(fEvents);

  LOG(INFO) << "CbmUnigenGenerator: Event " << fEvent->GetEventNr()
            << ", multiplicity " << fEvent->GetNpa() << FairLogger::endl;

  UParticle *particle;
  Double_t px;
  Double_t py;
  Double_t pz;
  Double_t pt;
  Double_t azim;
  Double_t pz1;
  Double_t phi = fEvent->GetPhi();
  Int_t pdgCode;
  Int_t iLambda;
  Int_t iMass;
  Int_t iCharge;

  // ---> Generate rotation angle  D
  if (fEventPlaneSet) {
    phi = gRandom->Uniform(fPhiMin, fPhiMax);
  }

  // Set event id and impact parameter in MCEvent if not yet done
  FairMCEventHeader *event = primGen->GetEvent();
  if (event && (!event->IsSet())) {
    event->SetEventID(fEvent->GetEventNr());
    event->SetB(fEvent->GetB());
    event->SetRotZ(phi);
    event->SetNPrim(fEvent->GetNpa());
    event->MarkSet(kTRUE);
  }

  // Loop over tracks in the current event
  for (Int_t itrack = 0; itrack < fEvent->GetNpa(); itrack++) {
    // Get particle
    particle = fEvent->GetParticle(itrack);
    // Boost
    pz = particle->Pz();
    if (fCM) {
      pz1 = fGammaCM * (pz + fBetaCM * particle->E());
    } else {
      pz1 = pz;
    }
    px = particle->Px();
    py = particle->Py();
    pdgCode = particle->GetPdg();
    LOG(DEBUG2) << "Px before: " << px << FairLogger::endl;
    LOG(DEBUG2) << "Py before: " << py << FairLogger::endl;
    // Rotate momenta by event plane angle
    if (fEventPlaneSet) {
      pt = TMath::Sqrt(px * px + py * py);
      azim = TMath::ATan2(py, px);
      azim += phi;
      px = pt * TMath::Cos(azim);
      py = pt * TMath::Sin(azim);
      LOG(DEBUG2) << "Px after: " << px << FairLogger::endl;
      LOG(DEBUG2) << "Py after: " << py << FairLogger::endl;
    }

    if (pdgCode > 1e9) // fragment
    {
      iLambda = (pdgCode % 100000000) / 10000000;
      iCharge = (pdgCode % 10000000) / 10000;
      iMass = (pdgCode % 10000) / 10;

      if (iLambda != 0) // replace a hypernucleus with non-strange analogue
      {
        LOG(WARNING)
            << "CbmUnigenGenerator: Replacling hypernucleus with pdg = "
            << pdgCode
            << " with non-strange analogue (hypernuclei are not supported by "
               "FairIon)\n";
        pdgCode -= iLambda * 1e7;
        LOG(WARNING) << "CbmUnigenGenerator: New pdg = " << pdgCode << endl
                     << endl;
      }

      if (iCharge == 0) // split the fragment into nucleons if it is neutral
      {
        LOG(WARNING)
            << "CbmUnigenGenerator: Splitting neutral fragment with pdg = "
            << pdgCode
            << " into neutrons (neutral fragments are not supported by "
               "TG4ParticlesManager)\n\n";
        for (int i = 0; i < iMass; i++) {
          primGen->AddTrack(2112, // neutron
                            px / iMass, py / iMass, pz1 / iMass, 0., 0., 0.);
        }
        continue;
      }
    }
    // Give track to PrimaryGenerator
    primGen->AddTrack(pdgCode, px, py, pz1, 0., 0., 0.);
  }

  fEvents += 1;

  return kTRUE;
}

// -----   Public method SetEventPlane   ----------------------------------
void CbmUnigenGenerator::SetEventPlane(Double_t phiMin, Double_t phiMax) {
  fPhiMin = phiMin;
  fPhiMax = phiMax;
  fEventPlaneSet = kTRUE;
}

// ------------------------------------------------------------------------
void CbmUnigenGenerator::CloseInput() {
  if (NULL != fInputFile) {
    LOG(INFO) << "CbmUnigenGenerator: Closing input file " << fFileName.Data()
              << FairLogger::endl;
    fInputFile->Close();
    fInputFile = NULL;
  }
}

// ------------------------------------------------------------------------
Int_t CbmUnigenGenerator::RegisterIons() {

  cout << " CbmUnigenGenerator::RegisterIons() start " << endl;
  UParticle *particle{nullptr};

  Int_t nIons{0};
  fIonMap.clear();

  for (Int_t iEvent = 0; iEvent < fInTree->GetEntries(); ++iEvent) {
    fInTree->GetEntry(iEvent);
    for (Int_t iParticle = 0; iParticle < fEvent->GetNpa(); ++iParticle) {
      // Get particle
      particle = fEvent->GetParticle(iParticle);
      long int pdgCode = particle->GetPdg();

      // for ions pdg code is +-10LZZZAAAI
      // where A = n_Lambda + n_proton + n_neutron
      //       Z = n_proton
      //       L = n_Lambda
      //       I = 0 - ground state, I>0 excitations

      if (pdgCode > 1e9) // ion
      {
        const int iLambda = (pdgCode % 100000000) / 10000000;
        const int iMass = (pdgCode % 10000) / 10;
        const int iCharge = (pdgCode % 10000000) / 10000;

        std::cout << pdgCode << " " << iMass << " " << iCharge << " " << iLambda
                  << std::endl;

        if (iCharge == 0)
          continue; // not present in G4IonTable

        if (iLambda != 0) // replace a hypernucleus with non-strange analogue
        {
          pdgCode -= iLambda * 1e7;
        }

        TString ionName = Form("Ion_%d_%d_%d", iMass, iCharge, iLambda);
        if (fIonMap.find(ionName) == fIonMap.end()) // new ion
        {
          FairIon *ion = new FairIon(
              ionName, iCharge, iMass,
              iCharge); // NOTE no strangeless in FairIon constructor
          fIonMap[ionName] = ion;
          nIons++;
        } // new ion
      }   // ion
    }     // particles
  }       // events

  FairRunSim *run = FairRunSim::Instance();
  for (const auto &mapIt : fIonMap) {
    FairIon *ion = mapIt.second;
    run->AddNewIon(ion);
  }
  return nIons;
}
// ------------------------------------------------------------------------

ClassImp(CbmUnigenGenerator);

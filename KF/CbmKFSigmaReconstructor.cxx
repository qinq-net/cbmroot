//-----------------------------------------------------------
//-----------------------------------------------------------

// Cbm Headers ----------------------
#include "CbmKFSigmaReconstructor.h"
#include "CbmKFParticleInterface.h"
#include "CbmKFParticleFinderPID.h"
#include "CbmStsTrack.h"
#include "FairRunAna.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"
#include "CbmKFVertex.h"

//KF Particle headers
#include "KFPTrackVector.h"
#include "KFParticle.h"

//ROOT headers
#include "TClonesArray.h" //to get arrays from the FairRootManager
#include "TStopwatch.h" //to measure the time
#include "TMath.h" //to calculate Prob function

#include "TH1F.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TMath.h"

//c++ and std headers
#include <iostream>
#include <cmath>
#include <vector>
using std::vector;

CbmKFSigmaReconstructor::CbmKFSigmaReconstructor(const char* name, Int_t iVerbose):
  FairTask(name, iVerbose), fStsTrackBranchName("StsTrack"), fTrackArray(0), fPrimVtx(0), fPVFindMode(0), fPID(0),
  fOutFileName("CbmKFParticleSigma.root"), fOutFile(0), fDirectory(0)
{
  TFile* curFile = gFile;
  TDirectory* curDirectory = gDirectory;
  
  if(!(fOutFileName == ""))
    fOutFile = new TFile(fOutFileName.Data(),"RECREATE");
  else
    fOutFile = gFile;
  
  fOutFile->cd();
  gDirectory->mkdir("SigmaReconstructor");
  gDirectory->cd("SigmaReconstructor");
  fDirectory = gDirectory;
  
  gDirectory->mkdir("Sigma-_->n_pi-");
  gDirectory->cd("Sigma-_->n_pi-");
  {
    gDirectory->mkdir("neutron");
    gDirectory->cd("neutron");
    {
      fHistos[0] = new TH1F("M", "M", 1000, 0, 1.);
      fHistos[1] = new TH1F("Chi2NDF", "Chi2NDF", 100, 0., 20.);
      fHistos[2] = new TH1F("Prob", "Prob", 100., 0., 1.);
    }
    gDirectory->cd("..");

    gDirectory->mkdir("Sigma-");
    gDirectory->cd("Sigma-");
    {
      fHistos[3] = new TH1F("M", "M", 1000, 0.8, 1.8);
      fHistos[4] = new TH1F("Chi2NDF", "Chi2NDF", 100, 0., 20.);
      fHistos[5] = new TH1F("Prob", "Prob", 100., 0., 1.);
    }
    gDirectory->cd("..");
  }
  gDirectory->cd("..");
  
  gDirectory->mkdir("Sigma+_->n_pi+");
  gDirectory->cd("Sigma+_->n_pi+");
  {
    gDirectory->mkdir("neutron");
    gDirectory->cd("neutron");
    {
      fHistos[6] = new TH1F("M", "M", 1000, 0, 1.);
      fHistos[7] = new TH1F("Chi2NDF", "Chi2NDF", 100, 0., 20.);
      fHistos[8] = new TH1F("Prob", "Prob", 100., 0., 1.);
    }
    gDirectory->cd("..");

    gDirectory->mkdir("Sigma+");
    gDirectory->cd("Sigma+");
    {
      fHistos[ 9] = new TH1F("M", "M", 1000, 0.8, 1.8);
      fHistos[10] = new TH1F("Chi2NDF", "Chi2NDF", 100, 0., 20.);
      fHistos[11] = new TH1F("Prob", "Prob", 100., 0., 1.);
    }
    gDirectory->cd("..");
  }
  gDirectory->cd("..");
  
  gDirectory->mkdir("Sigma+_->p_Pi0");
  gDirectory->cd("Sigma+_->p_Pi0");
  {
    gDirectory->mkdir("Pi0");
    gDirectory->cd("Pi0");
    {
      fHistos[12] = new TH1F("M", "M", 3000, -2., 1.);
      fHistos[13] = new TH1F("Chi2NDF", "Chi2NDF", 100, 0., 20.);
      fHistos[14] = new TH1F("Prob", "Prob", 100., 0., 1.);
    }
    gDirectory->cd("..");

    gDirectory->mkdir("Sigma+");
    gDirectory->cd("Sigma+");
    {
      fHistos[15] = new TH1F("M", "M", 1000, 0.8, 1.8);
      fHistos[16] = new TH1F("Chi2NDF", "Chi2NDF", 100, 0., 20.);
      fHistos[17] = new TH1F("Prob", "Prob", 100., 0., 1.);
    }
    gDirectory->cd("..");
  }
  gDirectory->cd("..");
  
  gFile = curFile;
  gDirectory = curDirectory;
}

CbmKFSigmaReconstructor::~CbmKFSigmaReconstructor()
{
}

InitStatus CbmKFSigmaReconstructor::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
  {
    Error("CbmKFSigmaReconstructor::Init","RootManager not instantiated!");
    return kERROR;
  }
  
  // Get input collection
  fTrackArray=(TClonesArray*) ioman->GetObject(fStsTrackBranchName);
  if(fTrackArray==0)
  {
    Error("CbmKFSigmaReconstructor::Init","track-array not found!");
    return kERROR;
  }
  
  fPrimVtx = (CbmVertex*) ioman->GetObject("PrimaryVertex");

  return kSUCCESS;
}

void CbmKFSigmaReconstructor::Exec(Option_t* opt)
{
  Int_t ntracks=0;//fTrackArray->GetEntriesFast();

  vector<CbmStsTrack> vRTracks(fTrackArray->GetEntriesFast());
  vector<int> pdg(fTrackArray->GetEntriesFast(), -1);
  vector<int> trackId(fTrackArray->GetEntriesFast(), -1);

  //select fitted tracks
  for(int iTr=0; iTr<fTrackArray->GetEntriesFast(); iTr++)
  {
    CbmStsTrack* stsTrack = ( (CbmStsTrack*) fTrackArray->At(iTr));
    const FairTrackParam* parameters = stsTrack->GetParamFirst();
    
    Double_t V[15] = {0.f}; 
    for (Int_t i=0,iCov=0; i<5; i++) 
      for (Int_t j=0; j<=i; j++,iCov++)
        V[iCov] = parameters->GetCovariance(i,j);
  
    bool ok = 1;
    ok = ok && finite(parameters->GetX());
    ok = ok && finite(parameters->GetY());
    ok = ok && finite(parameters->GetZ());
    ok = ok && finite(parameters->GetTx());
    ok = ok && finite(parameters->GetTy());
    ok = ok && finite(parameters->GetQp());
    
    for(unsigned short iC=0; iC<15; iC++)
      ok = ok && finite(V[iC]);
    ok = ok && (V[0] < 1. && V[0] > 0.)
            && (V[2] < 1. && V[2] > 0.)
            && (V[5] < 1. && V[5] > 0.)
            && (V[9] < 1. && V[9] > 0.)
            && (V[14] < 1. && V[14] > 0.);
    ok = ok && stsTrack->GetChiSq() < 10*stsTrack->GetNDF();
    if(!ok) continue;

//     if(fPID)
//     {
//       if(fPID->GetPID()[iTr] == -2) continue; 
//       pdg[ntracks] = fPID->GetPID()[iTr];
//     }
    
    vRTracks[ntracks] = *stsTrack;
    trackId[ntracks] = iTr;

    ntracks++;
  }
  
  vRTracks.resize(ntracks);
  pdg.resize(ntracks);
  trackId.resize(ntracks);

  //refit tracks with correct mass taken from PID 
  CbmL1PFFitter fitter;
  vector<float> vChiToPrimVtx;
  CbmKFVertex kfVertex;
  if(fPrimVtx)
    kfVertex = CbmKFVertex(*fPrimVtx);
  
  vector<L1FieldRegion> vField;
  fitter.Fit(vRTracks, pdg);
  fitter.GetChiToVertex(vRTracks, vField, vChiToPrimVtx, kfVertex, -3);
  
  //reconstruct Sigmas
  TStopwatch timer;
  timer.Start();
  
  vector<KFParticle> vSigmaMinus;
  vector<KFParticle> vPiMinusSecondary;
  
  vector<KFParticle> vSigmaPlus;
  vector<KFParticle> vPiPlusSecondary;
  vector<KFParticle> vProtonSecondary;

  
  for(int iTrack=0; iTrack<ntracks; iTrack++)
  {
    KFParticle tmpParticle;

    if(vChiToPrimVtx[iTrack] < 3 )
    {
      CbmKFParticleInterface::SetKFParticleFromStsTrack(&vRTracks[iTrack], &tmpParticle, 3112, 0);
      if(tmpParticle.Q() < 0)
	vSigmaMinus.push_back(tmpParticle);
    }
    else
    {
      CbmKFParticleInterface::SetKFParticleFromStsTrack(&vRTracks[iTrack], &tmpParticle, -211, 1);
      if(tmpParticle.Q() < 0)
	vPiMinusSecondary.push_back(tmpParticle);
    }
  }
  
  for(int iTrack=0; iTrack<ntracks; iTrack++)
  {
    KFParticle tmpParticle;

    if(vChiToPrimVtx[iTrack] < 3 )
    {
      CbmKFParticleInterface::SetKFParticleFromStsTrack(&vRTracks[iTrack], &tmpParticle, 3222, 0);
      if(tmpParticle.Q() > 0)
	vSigmaPlus.push_back(tmpParticle);
    }
    else
    {
      CbmKFParticleInterface::SetKFParticleFromStsTrack(&vRTracks[iTrack], &tmpParticle, 211, 1);
      if(tmpParticle.Q() > 0)
	vPiPlusSecondary.push_back(tmpParticle);
      
      CbmKFParticleInterface::SetKFParticleFromStsTrack(&vRTracks[iTrack], &tmpParticle, 2212, 1);
      if(tmpParticle.Q() > 0)
	vProtonSecondary.push_back(tmpParticle);
    }
  }
  
  for(unsigned int iSigma=0; iSigma<vSigmaMinus.size(); iSigma++)
  {
   //hypothesis Sigma+ -> n Pi-
    for(unsigned int iPi=0; iPi<vPiMinusSecondary.size(); iPi++)
    {
      float zSigma = vSigmaMinus[iSigma].Z();
      float zPi = vPiMinusSecondary[iPi].Z();
      
      if(zPi<zSigma) continue;
      //if(zPi-zSigma > 12.) continue;
      
      KFParticle neutron = vSigmaMinus[iSigma];
      neutron.NDF() = -1;
      neutron.Chi2() = 0;
      neutron.SubtractDaughter(vPiMinusSecondary[iPi]);

      float neutronMassPDG=0.939565378;
      
      if(neutron.NDF() < 0) continue;
      if(neutron.Chi2()/neutron.NDF() > 3.) continue;
      
      float mass, massError;
      neutron.GetMass(mass, massError);
      fHistos[0]->Fill(mass);
      fHistos[1]->Fill(neutron.Chi2()/neutron.NDF());
      fHistos[2]->Fill(TMath::Prob(neutron.Chi2(), neutron.NDF()));
      
      neutron.SetNonlinearMassConstraint(neutronMassPDG);
      
      const KFParticle * sigmaDaughters[2] = {&neutron, &vPiMinusSecondary[iPi]};
      KFParticle sigma;
      sigma.Construct(sigmaDaughters, 2);
      
      if(sigma.NDF() < 0) continue;
      if(sigma.Chi2()/sigma.NDF() > 3.) continue;
      
      sigma.GetMass(mass, massError);
      fHistos[3]->Fill(mass);
      fHistos[4]->Fill(sigma.Chi2()/sigma.NDF());
      fHistos[5]->Fill(TMath::Prob(sigma.Chi2(), sigma.NDF()));
    }
  }
  
  for(unsigned int iSigma=0; iSigma<vSigmaPlus.size(); iSigma++)
   {
  //hypothesis Sigma+ -> n Pi+
    for(unsigned int iPi=0; iPi<vPiPlusSecondary.size(); iPi++)
     {
      float zSigma = vSigmaPlus[iSigma].Z();
      float zPi = vPiPlusSecondary[iPi].Z();
      
      if(zPi<zSigma) continue;
      //if(zPi-zSigma > 12.) continue;
      
      KFParticle neutron = vSigmaPlus[iSigma];
      neutron.NDF() = -1;
      neutron.Chi2() = 0;
      neutron.SubtractDaughter(vPiPlusSecondary[iPi]);

      float neutronMassPDG=0.939565378;
      
      if(neutron.NDF() < 0) continue;
      if(neutron.Chi2()/neutron.NDF() > 3.) continue;
      
      float mass, massError;
      neutron.GetMass(mass, massError);
      fHistos[6]->Fill(mass);
      fHistos[7]->Fill(neutron.Chi2()/neutron.NDF());
      fHistos[8]->Fill(TMath::Prob(neutron.Chi2(), neutron.NDF()));
      
      neutron.SetNonlinearMassConstraint(neutronMassPDG);
      
      const KFParticle * sigmaDaughters[2] = {&neutron, &vPiPlusSecondary[iPi]};
      KFParticle sigma;
      sigma.Construct(sigmaDaughters, 2);
      
      if(sigma.NDF() < 0) continue;
      if(sigma.Chi2()/sigma.NDF() > 3.) continue;
      
      sigma.GetMass(mass, massError);
      fHistos[9]->Fill(mass);
      fHistos[10]->Fill(sigma.Chi2()/sigma.NDF());
      fHistos[11]->Fill(TMath::Prob(sigma.Chi2(), sigma.NDF()));
    }
 
 //hypothesis Sigma+ -> p Pi0
 for(unsigned int iPi=0; iPi<vProtonSecondary.size(); iPi++)
     {
      float zSigma = vSigmaPlus[iSigma].Z();
      float zProton = vProtonSecondary[iPi].Z();
      
      if(zProton<zSigma) continue;
      
      KFParticle Pi0 = vSigmaPlus[iSigma];
      Pi0.NDF() = -1;
      Pi0.Chi2() = 0;
      Pi0.SubtractDaughter(vProtonSecondary[iPi]);

      float Pi0_MassPDG=0.1349766;
      
      if(Pi0.NDF() < 0) continue;
      if(Pi0.Chi2()/Pi0.NDF() > 3.) continue;
      
      float mass, massError;
      Pi0.GetMass(mass, massError);
      fHistos[12]->Fill(mass);
      fHistos[13]->Fill(Pi0.Chi2()/Pi0.NDF());
      fHistos[14]->Fill(TMath::Prob(Pi0.Chi2(), Pi0.NDF()));
      
      Pi0.SetNonlinearMassConstraint(Pi0_MassPDG);
      
      const KFParticle * sigmaDaughters[2] = {&Pi0, &vProtonSecondary[iPi]};
      KFParticle sigma;
      sigma.Construct(sigmaDaughters, 2);
      
      if(sigma.NDF() < 0) continue;
      if(sigma.Chi2()/sigma.NDF() > 3.) continue;
      
      sigma.GetMass(mass, massError);
      fHistos[15]->Fill(mass);
      fHistos[16]->Fill(sigma.Chi2()/sigma.NDF());
      fHistos[17]->Fill(TMath::Prob(sigma.Chi2(), sigma.NDF()));
    }
    
  }
  
  timer.Stop();
}

void CbmKFSigmaReconstructor::Finish()
{
  TDirectory *curr = gDirectory;
  TFile *currentFile = gFile;
  // Open output file and write histograms

  fOutFile->cd();
  WriteHistosCurFile(fDirectory);
  if(!(fOutFileName == ""))
  {   
    fOutFile->Close();
    fOutFile->Delete();
  }
  
  gFile = currentFile;
  gDirectory = curr;
}

void CbmKFSigmaReconstructor::WriteHistosCurFile( TObject *obj )
{
  if( !obj->IsFolder() ) obj->Write();
  else{
    TDirectory *cur = gDirectory;
    TFile *currentFile = gFile;

    TDirectory *sub = cur->GetDirectory(obj->GetName());
    sub->cd();
    TList *listSub = (static_cast<TDirectory*>(obj))->GetList();
    TIter it(listSub);
    while( TObject *obj1=it() ) WriteHistosCurFile(obj1);
    cur->cd();
    gFile = currentFile;
    gDirectory = cur;
  }
}

ClassImp(CbmKFSigmaReconstructor);

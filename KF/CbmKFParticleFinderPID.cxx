//-----------------------------------------------------------
//-----------------------------------------------------------

// Cbm Headers ----------------------
#include "CbmKFParticleFinderPID.h"
#include "CbmStsTrack.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"
#include "CbmTofHit.h"
#include "CbmGlobalTrack.h"

#include "FairRunAna.h"

//ROOT headers
#include "TClonesArray.h"

//c++ and std headers
#include <iostream>

CbmKFParticleFinderPID::CbmKFParticleFinderPID(const char* name, Int_t iVerbose):
  FairTask(name, iVerbose), fStsTrackBranchName("StsTrack"), fGlobalTrackBranchName("GlobalTrack"), 
  fTofBranchName("TofHit"), fMCTracksBranchName("MCTrack"), fTrackMatchBranchName("StsTrackMatch"),
  fTrackArray(0), fGlobalTrackArray(0), fTofHitArray(0), fMCTrackArray(0), fTrackMatchArray(0), 
  fPIDMode(0), fSisMode(0), fPID(0)
{
}

CbmKFParticleFinderPID::~CbmKFParticleFinderPID()
{
}

InitStatus CbmKFParticleFinderPID::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
  {
    Error("CbmKFParticleFinderPID::Init","RootManager not instantiated!");
    return kERROR;
  }
  
  // Get sts tracks
  fTrackArray=(TClonesArray*) ioman->GetObject(fStsTrackBranchName);
  if(fTrackArray==0)
  {
    Error("CbmKFParticleFinderPID::Init","track-array not found!");
    return kERROR;
  }
  
  if(fPIDMode==2)
  {
    // Get global tracks
    fGlobalTrackArray=(TClonesArray*) ioman->GetObject(fGlobalTrackBranchName);
    if(fGlobalTrackArray==0)
    {
      Error("CbmKFParticleFinderPID::Init","global track array not found!");
      return kERROR;
    }
  
    // Get ToF hits
    fTofHitArray=(TClonesArray*) ioman->GetObject(fTofBranchName);
    if(fTofHitArray==0)
    {
      Error("CbmKFParticleFinderPID::Init","track-array not found!");
      return kERROR;
    }
  }
  
  if(fPIDMode==1)
  {
    //MC Tracks
    fMCTrackArray=(TClonesArray*) ioman->GetObject(fMCTracksBranchName);
    if(fMCTrackArray==0)
    {
      Error("CbmKFParticleFinderPID::Init","mc track array not found!");
      return kERROR;
    }
  
    //Track match
    fTrackMatchArray=(TClonesArray*) ioman->GetObject(fTrackMatchBranchName);
    if(fTrackMatchArray==0)
    {
      Error("CbmKFParticleFinderPID::Init","track match array not found!");
      return kERROR;
    }
  }
  
  return kSUCCESS;
}

void CbmKFParticleFinderPID::Exec(Option_t* opt)
{ 
  fPID.clear();
  
  Int_t nTracks=fTrackArray->GetEntriesFast();
  fPID.resize(nTracks, -1);

  if(fPIDMode==1)
    SetMCPID();
  if(fPIDMode==2)
    SetRecoPID(); 
}

void CbmKFParticleFinderPID::Finish()
{
}

void CbmKFParticleFinderPID::SetMCPID()
{
  Int_t nMCTracks = fMCTrackArray->GetEntriesFast();
  Int_t ntrackMatches = fTrackMatchArray->GetEntriesFast();
  for(int iTr=0; iTr<ntrackMatches; iTr++)
  {
    fPID[iTr] = -2;
    CbmTrackMatchNew* stsTrackMatch = (CbmTrackMatchNew*) fTrackMatchArray->At(iTr);
    if(stsTrackMatch -> GetNofLinks() == 0) continue;
    Float_t bestWeight = 0.f;
    Float_t totalWeight = 0.f;
    Int_t mcTrackId = -1;
    for(int iLink=0; iLink<stsTrackMatch -> GetNofLinks(); iLink++)
    {
      totalWeight += stsTrackMatch->GetLink(iLink).GetWeight();
      if( stsTrackMatch->GetLink(iLink).GetWeight() > bestWeight)
      {
        bestWeight = stsTrackMatch->GetLink(iLink).GetWeight();
        mcTrackId = stsTrackMatch->GetLink(iLink).GetIndex();
      }
    }
    if(bestWeight/totalWeight < 0.7) continue;
    if(mcTrackId >= nMCTracks || mcTrackId < 0)
    {
      std::cout << "Sts Matching is wrong!    StsTackId = " << mcTrackId << " N mc tracks = " << nMCTracks << std::endl;
      continue;
    }
    CbmMCTrack *cbmMCTrack = (CbmMCTrack*)fMCTrackArray->At(mcTrackId);
    if(!(TMath::Abs(cbmMCTrack->GetPdgCode()) == 11 ||
         TMath::Abs(cbmMCTrack->GetPdgCode()) == 13 ||
         TMath::Abs(cbmMCTrack->GetPdgCode()) == 211 ||
         TMath::Abs(cbmMCTrack->GetPdgCode()) == 321 ||
         TMath::Abs(cbmMCTrack->GetPdgCode()) == 2212) )
      continue;
    fPID[iTr] = cbmMCTrack->GetPdgCode();
  }
}

void CbmKFParticleFinderPID::SetRecoPID()
{
  if (NULL == fGlobalTrackArray) { Fatal("KF Particle Finder", "No GlobalTrack array!"); }
  if (NULL == fTofHitArray) { Fatal("KF Particle Finder", "No TOF hits array!"); }

  const Double_t m2P  = 0.885;
  const Double_t m2K  = 0.245;
  const Double_t m2Pi = 0.019479835;
  
  Double_t sP[3][5];
  if(fSisMode == 0) //SIS-100
  {
    Double_t sPLocal[3][5] = { {0.056908,-0.0470572,0.0216465,-0.0021016,8.50396e-05},
                               {0.00943075,-0.00635429,0.00998695,-0.00111527,7.77811e-05},
                               {0.00176298,0.00367263,0.00308013,0.000844013,-0.00010423} }; 
    for(Int_t iSp=0; iSp<3; iSp++)
      for(Int_t jSp=0; jSp<5; jSp++)
        sP[iSp][jSp] = sPLocal[iSp][jSp];
  }
  
  if(fSisMode == 1) //SIS-300
  {
    Double_t sPLocal[3][5] = { {0.0337428,-0.013939,0.00567602,-0.000202229,4.07531e-06},
                               {0.00717827,-0.00257353, 0.00389851,-9.83097e-05, 1.33011e-06},
                               {0.001348,0.00220126,0.0023619,7.35395e-05,-4.06706e-06} };
    for(Int_t iSp=0; iSp<3; iSp++)
      for(Int_t jSp=0; jSp<5; jSp++)
        sP[iSp][jSp] = sPLocal[iSp][jSp];
  }

  const Int_t PdgHypo[4] = {2212, 321, 211, -11};

  for (Int_t igt = 0; igt < fGlobalTrackArray->GetEntriesFast(); igt++) {
    const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTrackArray->At(igt));

    Int_t stsTrackIndex = globalTrack->GetStsTrackIndex();
    if( stsTrackIndex<0 ) continue;

//       Bool_t isElectronTRD = 0;
//       Bool_t isElectronRICH = 0;
//       Bool_t isElectron = 0;

    CbmStsTrack* cbmStsTrack = (CbmStsTrack*) fTrackArray->At(stsTrackIndex);
    const FairTrackParam *stsPar = cbmStsTrack->GetParamFirst();
    TVector3 mom;
    stsPar->Momentum(mom);

    Double_t p = mom.Mag();
    Int_t q = stsPar->GetQp() > 0 ? 1 : -1;

//       if(flistRichRings)
//       {
//         Int_t richIndex = globalTrack->GetRichRingIndex();
//         if (richIndex > -1)
//         {
//           CbmRichRing* richRing = (CbmRichRing*)flistRichRings->At(richIndex);
//           if (richRing)
//             if(fElIdAnn->DoSelect(richRing, p) > -0.5) isElectronRICH = 1;
//         }
//       }
// 
//       if(flistTrdTracks)
//       {
//         Int_t trdIndex = globalTrack->GetTrdTrackIndex();
//         if (trdIndex > -1)
//         {
//           CbmTrdTrack* trdTrack = (CbmTrdTrack*)flistTrdTracks->At(trdIndex);
//           if (trdTrack)
//             if (trdTrack->GetPidANN() > 0.979) isElectronTRD = 1;
//         }
//       }

    Double_t l = globalTrack->GetLength();// l is calculated by global tracking
    if(fSisMode==0) //SIS-100
      if( !((l>400.) && (l<800.)) ) continue;
    if(fSisMode==1) //SIS 300
      if( !((l>700.) && (l<1500.)) ) continue;
      
    Double_t time;
    Int_t tofHitIndex = globalTrack->GetTofHitIndex();
    if (tofHitIndex >= 0) {
      const CbmTofHit* tofHit = static_cast<const CbmTofHit*>(fTofHitArray->At(tofHitIndex));
      if(!tofHit) continue;
      time = tofHit->GetTime();
    }
    else
      continue;

    if(fSisMode==0) //SIS-100
      if( !((time>16.) && (time<42.)) ) continue;
    if(fSisMode==1) //SIS 300
      if( !((time>26.) && (time<52.)) ) continue;

    Double_t m2 = p*p*(1./((l/time/29.9792458)*(l/time/29.9792458))-1.);

    Double_t sigma[3];
    sigma[0] = sP[0][0] + sP[0][1]*p + sP[0][2]*p*p + sP[0][3]*p*p*p + sP[0][4]*p*p*p*p;
    sigma[1] = sP[1][0] + sP[1][1]*p + sP[1][2]*p*p + sP[1][3]*p*p*p + sP[1][4]*p*p*p*p;
    sigma[2] = sP[2][0] + sP[2][1]*p + sP[2][2]*p*p + sP[2][3]*p*p*p + sP[2][4]*p*p*p*p;

    Double_t dm2[3];
    dm2[0] = fabs(m2 - m2P)/sigma[0];
    dm2[1] = fabs(m2 - m2K)/sigma[1];
    dm2[2] = fabs(m2 - m2Pi)/sigma[2];

    int iPdg=2;
    Double_t dm2min = dm2[2];

//       if(isElectronRICH && isElectronTRD)
//       {
//         if (p >= 1.) {
//           if (m2 < (0.01 + (p - 1.) * 0.09))
//             isElectron = 1;
//         }
//         else {
//           if (m2 < 0.0)
//             isElectron = 1;
//         }
//       }
// 
//       if(!isElectron)
    {
      if(p>12.) continue;
      if(q>0)
      {
        if(dm2[1] < dm2min) { iPdg = 1; dm2min = dm2[1]; }
        if(dm2[0] < dm2min) { iPdg = 0; dm2min = dm2[0]; }

        if(dm2min > 2) iPdg=-1;
      }
      else
      {
        if(dm2[1] < dm2min) { iPdg = 1; dm2min = dm2[1]; }
        if((dm2min>3) && (dm2[0] < dm2min)) { iPdg = 0; dm2min = dm2[0]; }

        if(dm2min > 2) iPdg=-1;
      }

      if(iPdg > -1)
        fPID[stsTrackIndex] = q*PdgHypo[iPdg];
    }
//       else
//         vTrackPDG[stsTrackIndex] = q*PdgHypo[3];
  }
}

ClassImp(CbmKFParticleFinderPID);

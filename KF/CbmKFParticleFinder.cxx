//-----------------------------------------------------------
//-----------------------------------------------------------

// Cbm Headers ----------------------
#include "CbmKFParticleFinderPID.h"
#include "CbmKFParticleFinder.h"
#include "FairRunAna.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"
#include "CbmKFVertex.h"
#include "CbmEvent.h"

//KF Particle headers
#include "KFParticleTopoReconstructor.h"
#include "KFPTrackVector.h"

//ROOT headers
#include "TClonesArray.h" //to get arrays from the FairRootManager
#include "TStopwatch.h" //to measure the time
#include "TMath.h" //to calculate Prob function

//c++ and std headers
#include <iostream>
#include <cmath>
#include <vector>
using std::vector;

CbmKFParticleFinder::CbmKFParticleFinder(const char* name, Int_t iVerbose):
  FairTask(name, iVerbose), fStsTrackBranchName("StsTrack"), fTrackArray(0), fEvents(0), fPrimVtx(0), fTopoReconstructor(0), fPVFindMode(2), fPID(0),
  fSuperEventAnalysis(0), fSETracks(0), fSEField(0), fSEpdg(0), fSETrackId(0), fSEChiPrim(0), fTimeSliceMode(0)
{
  fTopoReconstructor = new KFParticleTopoReconstructor;
  
  // set default cuts
  SetPrimaryProbCut(0.0001); // 0.01% to consider primary track as a secondary;
}

CbmKFParticleFinder::~CbmKFParticleFinder()
{
  if(fTopoReconstructor) delete fTopoReconstructor;
}

InitStatus CbmKFParticleFinder::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
  {
    Error("CbmKFParticleFinder::Init","RootManager not instantiated!");
    return kERROR;
  }
  
  //check the mode
  fTimeSliceMode = 0;
  if( ioman->CheckBranch("Event") )
  {
    fTimeSliceMode = 1;    
    std::cout << GetName() << ": Running in the timeslice mode." << FairLogger::endl;
  }
  else
    std::cout << GetName() << ": Running in the event by event mode." << FairLogger::endl;
  
  // Get reconstructed events
  if(fTimeSliceMode)
  {
    fEvents = (TClonesArray*) ioman->GetObject("Event");
    if(fEvents == 0)
      Fatal("CbmKFParticleFinder::Init","No events available. Running in the event-by-event mode.");
  }
    
  // Get input collection
  fTrackArray=(TClonesArray*) ioman->GetObject(fStsTrackBranchName);
  if(fTrackArray==0)
  {
    Error("CbmKFParticleFinder::Init","track-array not found!");
    return kERROR;
  }

  // Get pointer to PrimaryVertex object from IOManager if it exists
  // The old name for the object is "PrimaryVertex" the new one
  // "PrimaryVertex." Check first for the new name
  fPrimVtx = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex."));
  if (nullptr == fPrimVtx) {
    fPrimVtx = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex"));
  }

  return kSUCCESS;
}

void CbmKFParticleFinder::Exec(Option_t* /*opt*/)
{
  fTopoReconstructor->Clear();
  
  int nEvents = 1;
  if(fTimeSliceMode)
    nEvents = fEvents->GetEntriesFast();
  
  vector<KFParticleTopoReconstructor> eventTopoReconstructor;
  eventTopoReconstructor.resize(nEvents);
    
  for(int iEvent=0; iEvent<nEvents; iEvent++)
  {
    CbmEvent* event = 0;
    if(fTimeSliceMode)
      event = (CbmEvent*) fEvents->At(iEvent);
    eventTopoReconstructor[iEvent].SetChi2PrimaryCut( InversedChi2Prob(0.0001, 2) );
    
    int nTracksEvent = 0;
    if(fTimeSliceMode)
      nTracksEvent = event->GetNofStsTracks();
    else
      nTracksEvent = fTrackArray->GetEntriesFast();
      
    Int_t ntracks=0;//fTrackArray->GetEntriesFast();

    //calculate number of d-He4 candidates
    int nCandidatesDHe4 = 0;
    if(fPID)
      for(int iTr=0; iTr<nTracksEvent; iTr++)
        if(TMath::Abs(fPID->GetPID()[iTr]) == 1000010029)
          nCandidatesDHe4++;
    
    vector<CbmStsTrack> vRTracks(nTracksEvent + nCandidatesDHe4);
    vector<int> pdg(nTracksEvent + nCandidatesDHe4, -1);
    vector<int> trackId(nTracksEvent + nCandidatesDHe4, -1);
      
    for(int iTr=0; iTr<nTracksEvent; iTr++)
    {
      int stsTrackIndex = 0;
      if(fTimeSliceMode)
        stsTrackIndex = event->GetStsTrackIndex(iTr);
      else
        stsTrackIndex = iTr;
      
      CbmStsTrack* stsTrack = ( (CbmStsTrack*) fTrackArray->At(stsTrackIndex));
      const FairTrackParam* parameters = stsTrack->GetParamFirst();
      
      Double_t V[15] = {0.f}; 
      for (Int_t i=0,iCov=0; i<5; i++) 
        for (Int_t j=0; j<=i; j++,iCov++)
          V[iCov] = parameters->GetCovariance(i,j);
    
      if(stsTrack->GetNofHits() < 3) continue;

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
      
      if(fPID)
      {
        if(fPID->GetPID()[stsTrackIndex] == -2) continue; 
        
        //not clear separation between d and He4
        if(TMath::Abs(fPID->GetPID()[stsTrackIndex]) == 1000010029)
        {
          int sgn = fPID->GetPID()[stsTrackIndex]/TMath::Abs(fPID->GetPID()[stsTrackIndex]);
          pdg[ntracks] = sgn * 1000010020;
          vRTracks[ntracks] = *stsTrack;
          trackId[ntracks] = stsTrackIndex;
          ntracks++;
          
          pdg[ntracks] = sgn * 1000020040;
        }
        else
          pdg[ntracks] = fPID->GetPID()[stsTrackIndex];
      }
      else
        pdg[ntracks] = -1;
      
      vRTracks[ntracks] = *stsTrack;
      trackId[ntracks] = stsTrackIndex;
      
      ntracks++;
    }
    
    vRTracks.resize(ntracks);
    pdg.resize(ntracks);
    trackId.resize(ntracks);

    CbmL1PFFitter fitter;
    vector<float> vChiToPrimVtx;
    CbmKFVertex kfVertex;
    if(fPrimVtx)
      kfVertex = CbmKFVertex(*fPrimVtx);
    
    vector<L1FieldRegion> vField, vFieldAtLastPoint;
    fitter.Fit(vRTracks, pdg);
    fitter.GetChiToVertex(vRTracks, vField, vChiToPrimVtx, kfVertex, 3);
    fitter.CalculateFieldRegionAtLastPoint(vRTracks, vFieldAtLastPoint);
    vector<KFFieldVector> vFieldVector(ntracks), vFieldVectorAtLastPoint(ntracks);
    for(Int_t iTr=0; iTr<ntracks; iTr++)
    {
      int entrSIMD = iTr % fvecLen;
      int entrVec  = iTr / fvecLen;
      vFieldVector[iTr].fField[0] = vField[entrVec].cx0[entrSIMD];
      vFieldVector[iTr].fField[1] = vField[entrVec].cx1[entrSIMD];
      vFieldVector[iTr].fField[2] = vField[entrVec].cx2[entrSIMD];
      vFieldVector[iTr].fField[3] = vField[entrVec].cy0[entrSIMD];
      vFieldVector[iTr].fField[4] = vField[entrVec].cy1[entrSIMD];
      vFieldVector[iTr].fField[5] = vField[entrVec].cy2[entrSIMD];
      vFieldVector[iTr].fField[6] = vField[entrVec].cz0[entrSIMD];
      vFieldVector[iTr].fField[7] = vField[entrVec].cz1[entrSIMD];
      vFieldVector[iTr].fField[8] = vField[entrVec].cz2[entrSIMD];
      vFieldVector[iTr].fField[9] = vField[entrVec].z0[entrSIMD];
    }
    for(Int_t iTr=0; iTr<ntracks; iTr++)
    {
      int entrSIMD = iTr % fvecLen;
      int entrVec  = iTr / fvecLen;
      vFieldVectorAtLastPoint[iTr].fField[0] = vFieldAtLastPoint[entrVec].cx0[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[1] = vFieldAtLastPoint[entrVec].cx1[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[2] = vFieldAtLastPoint[entrVec].cx2[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[3] = vFieldAtLastPoint[entrVec].cy0[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[4] = vFieldAtLastPoint[entrVec].cy1[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[5] = vFieldAtLastPoint[entrVec].cy2[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[6] = vFieldAtLastPoint[entrVec].cz0[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[7] = vFieldAtLastPoint[entrVec].cz1[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[8] = vFieldAtLastPoint[entrVec].cz2[entrSIMD];
      vFieldVectorAtLastPoint[iTr].fField[9] = vFieldAtLastPoint[entrVec].z0[entrSIMD];
    }
    
    if(!fSuperEventAnalysis)
    {
      KFPTrackVector tracks;
      FillKFPTrackVector(&tracks, vRTracks, vFieldVector, pdg, trackId, vChiToPrimVtx);
      KFPTrackVector tracksAtLastPoint;
      FillKFPTrackVector(&tracksAtLastPoint, vRTracks, vFieldVectorAtLastPoint, pdg, trackId, vChiToPrimVtx, 0);
      
      TStopwatch timer;
      timer.Start();
            
      eventTopoReconstructor[iEvent].Init(tracks, tracksAtLastPoint);
      if(fPVFindMode == 0)
      {
        KFPVertex primVtx_tmp;
        primVtx_tmp.SetXYZ(0,0,0);
        primVtx_tmp.SetCovarianceMatrix( 0,0,0,0,0,0 );
        primVtx_tmp.SetNContributors( 0 );
        primVtx_tmp.SetChi2( -100 );

        vector<int> pvTrackIds;
        KFVertex pv(primVtx_tmp);
        eventTopoReconstructor[iEvent].AddPV(pv, pvTrackIds);
      }
      else if(fPVFindMode == 1)
        eventTopoReconstructor[iEvent].ReconstructPrimVertex();
      else if(fPVFindMode == 2)
        eventTopoReconstructor[iEvent].ReconstructPrimVertex(0);
      

      eventTopoReconstructor[iEvent].SortTracks();
      eventTopoReconstructor[iEvent].ReconstructParticles();
      
      timer.Stop();
      eventTopoReconstructor[iEvent].SetTime(timer.RealTime());
    }
    else
    {
      for(int iTr=0; iTr<ntracks; iTr++)
      {
        const FairTrackParam* parameters = vRTracks[iTr].GetParamFirst();
        float a = parameters->GetTx(), b = parameters->GetTy(), qp = parameters->GetQp();
        Int_t q = 0;
        if(qp>0.f)
          q = 1;
        if(qp<0.f)
          q=-1;
        float c2 = 1.f/(1.f + a*a + b*b);
        float pq = 1.f/qp * TMath::Abs(q);
        float p2 = pq*pq;
        float pz = sqrt(p2*c2);
        float px = a*pz;
        float py = b*pz;
        float pt = sqrt(px*px + py*py);
        
        bool save=0;
        
        if(vChiToPrimVtx[iTr] < 3)
        {
          if( (fabs(pdg[iTr]) == 11 && pt > 0.2f) || (fabs(pdg[iTr]) == 13) || (fabs(pdg[iTr]) == 19) )
            save=1;
        }
        
        if(vChiToPrimVtx[iTr] > 3)
        {
          if( ( fabs(pdg[iTr]) == 211 || fabs(pdg[iTr]) == 321 || fabs(pdg[iTr]) == 2212 || pdg[iTr] == -1) && pt > 0.2f )
            save = 1;
        }
        
        if(save)
        {
          fSETracks.push_back(vRTracks[iTr]);
          fSEField.push_back(vFieldVector[iTr]);
          fSEpdg.push_back(pdg[iTr]);
          fSETrackId.push_back(fSETrackId.size());
          fSEChiPrim.push_back(vChiToPrimVtx[iTr]);
        }
      }
    }
  }
  
  
  vector<int> PVTrackIndexArray;
  int indexAdd = 0;
    
  for(int iEvent=0; iEvent<nEvents; iEvent++)
  {
    const KFParticleTopoReconstructor* eventTR = &eventTopoReconstructor[iEvent];
    
    for( int iPV=0;iPV<eventTR->NPrimaryVertices();iPV++)
    {
      PVTrackIndexArray = eventTR->GetPVTrackIndexArray(iPV);
      for(unsigned int iTr=0; iTr<PVTrackIndexArray.size(); iTr++)
        PVTrackIndexArray[iTr]=PVTrackIndexArray[iTr] + indexAdd;
            
      fTopoReconstructor->AddPV(eventTR->GetPrimKFVertex(iPV),PVTrackIndexArray);
      PVTrackIndexArray.clear();
    }
    for(unsigned int iP=0;iP<eventTR->GetParticles().size();iP++)
    {
      KFParticle particle;
      const KFParticle& particleEvent =eventTR->GetParticles()[iP];
      particle = particleEvent;
      particle.CleanDaughtersId();
      int idP = particleEvent.Id()+indexAdd;
      int idDaughter=0;
      for(int nD=0;nD<particleEvent.NDaughters();nD++)
      {
        if(particleEvent.NDaughters()==1) idDaughter = particleEvent.DaughterIds()[nD];
        if(particleEvent.NDaughters()>1) idDaughter = particleEvent.DaughterIds()[nD]+indexAdd;
        particle.AddDaughterId(idDaughter);
      }
      particle.SetId(idP);
      fTopoReconstructor->AddParticle(particle);
    }
    indexAdd +=eventTR->GetParticles().size();
  } 
}

void CbmKFParticleFinder::Finish()
{
  if(fSuperEventAnalysis)
  {
    KFPTrackVector tracks;
    FillKFPTrackVector(&tracks, fSETracks, fSEField, fSEpdg, fSETrackId, fSEChiPrim);
    KFPTrackVector tracksAtLastPoint;
    
    std::cout << "CbmKFParticleFinder: Start SE analysis" << std::endl;
    TStopwatch timer;
    timer.Start();

    fTopoReconstructor->Init(tracks, tracksAtLastPoint);

    KFPVertex primVtx_tmp;
    primVtx_tmp.SetXYZ(0,0,0);
    primVtx_tmp.SetCovarianceMatrix( 0,0,0,0,0,0 );
    primVtx_tmp.SetNContributors( 0 );
    primVtx_tmp.SetChi2( -100 );
    vector<int> pvTrackIds;
    KFVertex pv(primVtx_tmp);
    fTopoReconstructor->AddPV(pv, pvTrackIds);

    fTopoReconstructor->SortTracks();
    fTopoReconstructor->ReconstructParticles();

    timer.Stop();
    fTopoReconstructor->SetTime(timer.RealTime());
    std::cout << "CbmKFParticleFinder: Finish SE analysis" << timer.RealTime() << std::endl;
  }
}

void CbmKFParticleFinder::FillKFPTrackVector(KFPTrackVector* tracks, const vector<CbmStsTrack>& vRTracks, const vector<KFFieldVector>& vField, 
                                             const vector<int>& pdg, const vector<int>& trackId, const vector<float>& vChiToPrimVtx, bool atFirstPoint) const
{
  int ntracks = vRTracks.size();
  tracks->Resize(ntracks);
  //fill vector with tracks
  for(Int_t iTr=0; iTr<ntracks; iTr++)
  {
    const FairTrackParam* parameters;
    if(atFirstPoint)
      parameters = vRTracks[iTr].GetParamFirst();
    else
      parameters = vRTracks[iTr].GetParamLast();
    
    double par[6] = {0.f};
    
    double tx = parameters->GetTx(), ty = parameters->GetTy(), qp = parameters->GetQp();
  
    Int_t q = 0;
    if(qp>0.f)
      q = 1;
    if(qp<0.f)
      q=-1;
    if( TMath::Abs(pdg[iTr]) == 1000020030 || TMath::Abs(pdg[iTr]) == 1000020040 ) q *= 2;
      
    
    double c2 = 1.f/(1.f + tx*tx + ty*ty);
    double pq = 1.f/qp * TMath::Abs(q);
    double p2 = pq*pq;
    double pz = sqrt(p2*c2);
    double px = tx*pz;
    double py = ty*pz;
      
    par[0] = parameters->GetX();
    par[1] = parameters->GetY();
    par[2] = parameters->GetZ();
    par[3] = px;
    par[4] = py;
    par[5] = pz;

    //calculate covariance matrix
    double t = sqrt(1.f + tx*tx + ty*ty);
    double t3 = t*t*t;
    double dpxdtx = q/qp*(1.f+ty*ty)/t3;
    double dpxdty = -q/qp*tx*ty/t3;
    double dpxdqp = -q/(qp*qp)*tx/t;
    double dpydtx = -q/qp*tx*ty/t3;
    double dpydty = q/qp*(1.f+tx*tx)/t3;
    double dpydqp = -q/(qp*qp)*ty/t;
    double dpzdtx = -q/qp*tx/t3;
    double dpzdty = -q/qp*ty/t3;
    double dpzdqp = -q/(qp*qp*t3);
    
    double F[6][5] = { {1.f, 0.f, 0.f,    0.f,    0.f},
                       {0.f, 1.f, 0.f,    0.f,    0.f},
                       {0.f, 0.f, 0.f,    0.f,    0.f},
                       {0.f, 0.f, dpxdtx, dpxdty, dpxdqp},
                       {0.f, 0.f, dpydtx, dpydty, dpydqp},
                       {0.f, 0.f, dpzdtx, dpzdty, dpzdqp} };
    
    double VFT[5][6];
    for(int i=0; i<5; i++)
      for(int j=0; j<6; j++)
      {
        VFT[i][j] = 0;
        for(int k=0; k<5; k++)
        {
          VFT[i][j] +=  parameters->GetCovariance(i,k) * F[j][k];
        }
      }
    
    double cov[21];
    for(int i=0, l=0; i<6; i++)
      for(int j=0; j<=i; j++, l++)
      {
        cov[l] = 0;
        for(int k=0; k<5; k++)
        {
          cov[l] += F[i][k] * VFT[k][j];
        }
      }
    
    for(Int_t iP=0; iP<6; iP++)
      tracks->SetParameter(par[iP], iP, iTr);
    for(Int_t iC=0; iC<21; iC++)
      tracks->SetCovariance(cov[iC], iC, iTr);
    for(Int_t iF=0; iF<10; iF++)
      tracks->SetFieldCoefficient(vField[iTr].fField[iF], iF, iTr);
    tracks->SetId(trackId[iTr], iTr);
    tracks->SetPDG(pdg[iTr], iTr);
    tracks->SetQ(q, iTr);

    if(fPVFindMode == 0)
    {    
      if(vChiToPrimVtx[iTr] < 3)
        tracks->SetPVIndex(0, iTr);
      else
        tracks->SetPVIndex(-1, iTr);
    }
    else
      tracks->SetPVIndex(-1, iTr);
  }
}

double CbmKFParticleFinder::InversedChi2Prob(double p, int ndf) const
{
  double epsilon = 1.e-14;
  double chi2Left = 0.f;
  double chi2Right = 10000.f;
  
  double probLeft = p - TMath::Prob(chi2Left, ndf);
  
  double chi2Centr = (chi2Left+chi2Right)/2.f;
  double probCentr = p - TMath::Prob( chi2Centr, ndf);
  
  while( TMath::Abs(chi2Right-chi2Centr)/chi2Centr > epsilon )
  {
    if(probCentr * probLeft > 0.f)
    {
      chi2Left = chi2Centr;
      probLeft = probCentr;
    }
    else
    {
      chi2Right = chi2Centr;
    }
    
    chi2Centr = (chi2Left+chi2Right)/2.f;
    probCentr = p - TMath::Prob( chi2Centr, ndf);
  }
  
  return chi2Centr;
}

void CbmKFParticleFinder::SetPrimaryProbCut(float prob)
{ 
  fTopoReconstructor->SetChi2PrimaryCut( InversedChi2Prob(prob, 2) );
}

void CbmKFParticleFinder::SetSuperEventAnalysis()
{ 
  fSuperEventAnalysis=1; 
  fPVFindMode = 0;
  fTopoReconstructor->SetMixedEventAnalysis();
}

KFParticleFinder*  CbmKFParticleFinder::GetKFParticleFinder()          { return fTopoReconstructor->GetKFParticleFinderPointer(); }
void CbmKFParticleFinder::SetMaxDistanceBetweenParticlesCut(float cut) { GetKFParticleFinder()->SetMaxDistanceBetweenParticlesCut(cut); }
void CbmKFParticleFinder::SetLCut(float cut)                           { GetKFParticleFinder()->SetLCut(cut); }
void CbmKFParticleFinder::SetChiPrimaryCut2D(float cut)                { GetKFParticleFinder()->SetChiPrimaryCut2D(cut); }
void CbmKFParticleFinder::SetChi2Cut2D(float cut)                      { GetKFParticleFinder()->SetChi2Cut2D(cut); }
void CbmKFParticleFinder::SetLdLCut2D(float cut)                       { GetKFParticleFinder()->SetLdLCut2D(cut); }
void CbmKFParticleFinder::SetLdLCutXiOmega(float cut)                  { GetKFParticleFinder()->SetLdLCutXiOmega(cut); }
void CbmKFParticleFinder::SetChi2TopoCutXiOmega(float cut)             { GetKFParticleFinder()->SetChi2TopoCutXiOmega(cut); }
void CbmKFParticleFinder::SetChi2CutXiOmega(float cut)                 { GetKFParticleFinder()->SetChi2CutXiOmega(cut); }
void CbmKFParticleFinder::SetChi2TopoCutResonances(float cut)          { GetKFParticleFinder()->SetChi2TopoCutResonances(cut); }
void CbmKFParticleFinder::SetChi2CutResonances(float cut)              { GetKFParticleFinder()->SetChi2CutResonances(cut); }
void CbmKFParticleFinder::SetPtCutLMVM(float cut)                      { GetKFParticleFinder()->SetPtCutLMVM(cut); }
void CbmKFParticleFinder::SetPCutLMVM(float cut)                       { GetKFParticleFinder()->SetPCutLMVM(cut); }
void CbmKFParticleFinder::SetPtCutJPsi(float cut)                      { GetKFParticleFinder()->SetPtCutJPsi(cut); }
void CbmKFParticleFinder::SetPtCutCharm(float cut)                     { GetKFParticleFinder()->SetPtCutCharm(cut); }
void CbmKFParticleFinder::SetChiPrimaryCutCharm(float cut)             { GetKFParticleFinder()->SetChiPrimaryCutCharm(cut); }
void CbmKFParticleFinder::SetLdLCutCharmManybodyDecays(float cut)      { GetKFParticleFinder()->SetLdLCutCharmManybodyDecays(cut); }
void CbmKFParticleFinder::SetChi2TopoCutCharmManybodyDecays(float cut) { GetKFParticleFinder()->SetChi2TopoCutCharmManybodyDecays(cut); }
void CbmKFParticleFinder::SetChi2CutCharmManybodyDecays(float cut)     { GetKFParticleFinder()->SetChi2CutCharmManybodyDecays(cut); }
void CbmKFParticleFinder::SetLdLCutCharm2D(float cut)                  { GetKFParticleFinder()->SetLdLCutCharm2D(cut); }
void CbmKFParticleFinder::SetChi2TopoCutCharm2D(float cut)             { GetKFParticleFinder()->SetChi2TopoCutCharm2D(cut); }
void CbmKFParticleFinder::SetChi2CutCharm2D(float cut)                 { GetKFParticleFinder()->SetChi2CutCharm2D(cut); }

ClassImp(CbmKFParticleFinder);

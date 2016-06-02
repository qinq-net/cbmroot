///////////////////////////////////////////////////////////////////////////
//
//
// Authors:
//   Julian Book   <Julian.Book@cern.ch>
/*

  Analysis track that keep references to all tracklets of sub detectors and 
  provides easy access to them via e.g. GetTrack(DetectorId det).

  Two TLorentzVector hold information on the momentum components and
  position. Further the SetMassHypo is calculated according to the 
  setting of PairAnalysis::SetLegPdg(pdgLeg1, pdgLeg2) and the actual charge.
  In addition a track can be refitted using this mass assumption if enabled
  using PairAnalysis::SetRefitWithMassAssump(kTRUE)

  TObject bits are used to flag the matching between detector tracklets and MC tracks.
  Bits used are >14 and correspond to CbmDetectorList.h -> DetectorId
*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//#include <TObjArray.h>
#include <vector>

#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TParticle.h>
#include <TParticlePDG.h>

#include "FairTrackParam.h"
#include "CbmDetectorList.h"

#include "CbmKFVertex.h"
#include "CbmTrack.h"
#include "CbmGlobalTrack.h"
#include "CbmTrackParam.h"
#include "CbmMvdDetector.h"
#include "CbmMvdStationPar.h"
#include "CbmStsTrack.h"
#include "CbmMuchTrack.h"
#include "CbmTrdTrack.h"
#include "CbmRichRing.h"
#include "CbmTofHit.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"

#include "L1Field.h"
#include "CbmL1PFFitter.h"

#include "CbmLitToolFactory.h"
#include "CbmLitTrackParam.h"
#include "CbmLitPtrTypes.h"
#include "CbmLitConverterFairTrackParam.h"

#include "PairAnalysisTrack.h"

ClassImp(PairAnalysisTrack)

PairAnalysisTrack::PairAnalysisTrack() :
  TNamed(),
  fMomentum(),
  fPosition()
{
  //
  // Default Constructor
  //

}

//______________________________________________
PairAnalysisTrack::PairAnalysisTrack(const char* name, const char* title) :
  TNamed(name, title),
  fMomentum(),
  fPosition()
{
  //
  // Named Constructor
  //

}

//______________________________________________
PairAnalysisTrack::PairAnalysisTrack(TParticle *fastTrk,
				     CbmMCTrack *mctrk ) :
  TNamed(),
  fMCTrack(mctrk),
  fMomentum(),
  fPosition(),
  fPdgCode(fastTrk->GetPdgCode()),
  fLabel(fastTrk->GetFirstMother())
{
  //
  // Constructor
  //
  fastTrk->Momentum(fMomentum);
  fastTrk->ProductionVertex(fPosition);

  TParticlePDG *mcPart = fastTrk->GetPDG(0);
  if(mcPart) fCharge  = mcPart->Charge()/3;
}

//______________________________________________
PairAnalysisTrack::PairAnalysisTrack(CbmKFVertex *vtx,
				     CbmGlobalTrack *gtrk,
				     CbmStsTrack *ststrk,
				     CbmMuchTrack *muchtrk,
				     CbmTrdTrack *trdtrk,
				     CbmRichRing *richring,
				     CbmTofHit *tofhit,
				     CbmMCTrack *mctrk,
				     CbmTrackMatchNew *stsmatch,
				     CbmTrackMatchNew *muchmatch,
				     CbmTrackMatchNew *trdmatch,
				     CbmTrackMatchNew *richmatch,
				     FairTrackParam *richproj
				     ) :
  TNamed(),
  fPrimVertex(vtx),
  fGlblTrack(gtrk),
  fStsTrack(ststrk),
  fMuchTrack(muchtrk),
  fTrdTrack(trdtrk),
  fRichRing(richring),
  fTofHit(tofhit),
  fMCTrack(mctrk),
  fStsTrackMatch(stsmatch),
  fMuchTrackMatch(muchmatch),
  fTrdTrackMatch(trdmatch),
  fRichRingMatch(richmatch),
  fRichProj(richproj),
  fMvdEntrance(new FairTrackParam()),
  fMomentum(),
  fPosition()
{
  //
  // Constructor
  //
  Double_t m2=TMath::Power(TDatabasePDG::Instance()->GetParticle(11)->Mass(), 2);

  /// check mvd entrance if mvd is in setup
  CbmMvdStationPar* mvdpar = CbmMvdDetector::Instance()->GetParameterFile();
  if(mvdpar->GetStationCount()) {
    Double_t zMvd = mvdpar->GetZPosition(0); // z-position of the first mvd station
    TrackExtrapolatorPtr fExtrapolator = CbmLitToolFactory::CreateTrackExtrapolator("rk4");
    CbmLitTrackParam litParamIn;
    CbmLitConverterFairTrackParam::FairTrackParamToCbmLitTrackParam( ststrk->GetParamFirst(), &litParamIn);
    CbmLitTrackParam litParamOut;
    fExtrapolator->Extrapolate(&litParamIn, &litParamOut, zMvd, NULL);
    CbmLitConverterFairTrackParam::CbmLitTrackParamToFairTrackParam(&litParamOut, fMvdEntrance);
  }

  /// get parameters at primary vertex
  const CbmTrackParam *ppar = fGlblTrack->GetParamVertex();
  if(ppar) {
    fMomentum.SetPxPyPzE( ppar->GetPx(), ppar->GetPy(), ppar->GetPz(), 0. );
    fMomentum.SetE( TMath::Sqrt(fMomentum.Vect().Mag2()+m2) );
    fPosition.SetXYZM(    ppar->GetX(),  ppar->GetY(),  ppar->GetZ(),  TMath::Sqrt(m2) );
    fCharge = (ppar->GetQp()>0. ? +1. : -1. );
    CalculateChi2Vtx();
  }
  else {
    Refit(211);
  }

  if(mctrk) fPdgCode = mctrk->GetPdgCode();

}

//______________________________________________
PairAnalysisTrack::PairAnalysisTrack(const PairAnalysisTrack& track) :
  TNamed(track.GetName(), track.GetTitle()),
  fPrimVertex(track.fPrimVertex),
  fGlblTrack(track.GetGlobalTrack()),
  fStsTrack(track.fStsTrack),
  fMuchTrack(track.fMuchTrack),
  fTrdTrack(track.fTrdTrack),
  fRichRing(track.GetRichRing()),
  fTofHit(track.GetTofHit()),
  fMCTrack(track.GetMCTrack()),
  fStsTrackMatch(track.GetTrackMatch(kSTS)),
  fMuchTrackMatch(track.GetTrackMatch(kMUCH)),
  fTrdTrackMatch(track.GetTrackMatch(kTRD)),
  fRichRingMatch(track.GetTrackMatch(kRICH)),
  fRichProj(track.GetRichProj()),
  fMvdEntrance(track.GetMvdEntrance()),
  fMomentum(track.fMomentum),
  fPosition(track.fPosition),
  fChi2Vtx(track.ChiToVertex()),
  fCharge(track.Charge()),
  fPdgCode(track.PdgCode()),
  fLabel(track.GetLabel()),
  fWeight(track.GetWeight())
{
  //
  // Copy Constructor
  //

  this->SetBit(BIT(14+kTOF),  track.TestBit(BIT(14+kTOF) ));
  this->SetBit(BIT(14+kRICH), track.TestBit(BIT(14+kRICH)));
  this->SetBit(BIT(14+kTRD),  track.TestBit(BIT(14+kTRD) ));
  this->SetBit(BIT(14+kSTS),  track.TestBit(BIT(14+kSTS) ));
  this->SetBit(BIT(14+kMUCH), track.TestBit(BIT(14+kMUCH)));

}

//______________________________________________
PairAnalysisTrack::~PairAnalysisTrack()
{
  //
  // Default Destructor
  //
  //  if(fPrimVertex) delete fPrimVertex;
}

//______________________________________________
CbmTrackMatchNew* PairAnalysisTrack::GetTrackMatch(DetectorId det) const
{
  //
  // get track match depending on detector id
  //
  switch(det) {
  case kMVD:  return fStsTrackMatch; // there is no mvd track, hit are associtaed to sts track
  case kSTS:  return fStsTrackMatch;
  case kTRD:  return fTrdTrackMatch;
  case kMUCH: return fMuchTrackMatch;
  case kRICH: return fRichRingMatch;
  default:   return 0x0;
  }

}

//______________________________________________
CbmTrack* PairAnalysisTrack::GetTrack(DetectorId det) const
{
  //
  // get track depending on detector id
  //
  switch(det) {
  case kMVD:  return fStsTrack; // there is no mvd track, hit are associtaed to sts track
  case kSTS:  return fStsTrack;
  case kTRD:  return fTrdTrack;
  case kMUCH: return fMuchTrack;
  case kRICH: return 0x0;
  default:   return 0x0;
  }

}

//______________________________________________
void PairAnalysisTrack::SetMassHypo(Int_t pdg1, Int_t pdg2, Bool_t refitMassAssump)
{
  //
  // use charge, time and track length information to assign
  // the best guessed mass hypothesis
  //
  const Double_t mpdg1 = TDatabasePDG::Instance()->GetParticle(pdg1)->Mass();
  const Double_t mpdg2 = TDatabasePDG::Instance()->GetParticle(pdg2)->Mass();
  const Double_t cpdg1 = TDatabasePDG::Instance()->GetParticle(pdg1)->Charge()*3;
  const Double_t cpdg2 = TDatabasePDG::Instance()->GetParticle(pdg2)->Charge()*3;

  Double_t m2 = 0.;
  Int_t ppdg  = 0;   // prefered pdg
  // match STS charge of track to pid and set mass accordingly
  if(fCharge*cpdg1 < 0)      { m2=mpdg2*mpdg2; ppdg=pdg2; }
  else if(fCharge*cpdg2 < 0) { m2=mpdg1*mpdg1; ppdg=pdg1; }
  else                       Error("SetMassHypo","via STS charge went wrong!");

  // use TOF time(ns) and track length(cm) if available
  if(fTofHit && kFALSE) { //TODO: switched OFF!!
    m2 = fMomentum.Mag2() * (TMath::Power( (fTofHit->GetTime()*1e-9*TMath::C()) /
					   fGlblTrack->GetLength()/100,           2)  - 1);
  }

  // refit (under pdg assumption if activated)
  if(!refitMassAssump) {
    /// get back parameters at primary vertex
    const CbmTrackParam *ppar = fGlblTrack->GetParamVertex();
    if(ppar) {
      fMomentum.SetPxPyPzE( ppar->GetPx(), ppar->GetPy(), ppar->GetPz(), 0. );
      fMomentum.SetE( TMath::Sqrt(fMomentum.Vect().Mag2()+m2) );
      fPosition.SetXYZM(    ppar->GetX(),  ppar->GetY(),  ppar->GetZ(),  TMath::Sqrt(m2) );
      fCharge  = (ppar->GetQp()>0. ? +1. : -1. );
      CalculateChi2Vtx();
    }
    else
      Refit(211);
  }
  else {
    Refit(ppdg);
    //    fMomentum.Print();
    /// set mass hypo
    fMomentum.SetE( TMath::Sqrt(fMomentum.Vect().Mag2() + m2) );
    //    fMomentum.Print();
  }

}

//______________________________________________
void PairAnalysisTrack::Refit(Int_t pidHypo)
{
  //
  // refit the track under certain mass assumption using CbmL1PFFitter
  // to the primary vertex
  //

  vector<CbmStsTrack> stsTracks;
  stsTracks.resize(1);
  stsTracks[0] = *fStsTrack;
  vector<L1FieldRegion> vField;
  vector<float> chiPrim;
  vector<int> pidHypos;
  pidHypos.push_back(pidHypo);

  // printf("stst track: %p \t prim vertex: %p\n",fStsTrack,fPrimVertex);
  // printf("  fit track with mass assumption, pdg: %d (default is: %d)\n",pidHypo,fStsTrack->GetPidHypo());

  CbmL1PFFitter fPFFitter;
  if(pidHypo) fPFFitter.Fit(stsTracks, pidHypos); // fit with mass hypo
  //  printf("  fit done for mass hypo (%p)\n",&stsTracks[0]);

  // NOTE: as an alternative to fPFFitter.Fit one can use fStsTrack->SetPidHypo(pidHypo);
  fPFFitter.GetChiToVertex(stsTracks, vField, chiPrim, *fPrimVertex, 3.e6);
  fChi2Vtx = chiPrim[0];
  //  printf("  track refitted with chi2/ndf: %.3f , param %p \n",fChi2Vtx,stsTracks[0].GetParamFirst());

  const FairTrackParam* vtxTrack = stsTracks[0].GetParamFirst();
  if(!vtxTrack) Error("Refit","No track param found!");

  // update position and momentum vectors
  TVector3 mom;
  vtxTrack->Momentum(mom);
  fMomentum.SetVect(mom);

  TVector3 pos;
  vtxTrack->Position(pos);
  fPosition.SetVect(pos);

  // set charge based on fit
  fCharge  = (vtxTrack->GetQp()>0. ? +1. : -1. );

}

//______________________________________________
void PairAnalysisTrack::CalculateChi2Vtx()
{
  //
  // calculation according to CbmL1PFFitter::GetChiToVertex
  //

  // primary vertex
  Double_t Cv[3] = { fPrimVertex->GetCovMatrix()[0], fPrimVertex->GetCovMatrix()[1], fPrimVertex->GetCovMatrix()[2] };

  // track params at prim vertex
  const CbmTrackParam *ppar = fGlblTrack->GetParamVertex();

  // impact param
  Double_t dx = ppar->GetX() - fPrimVertex->GetRefX();
  Double_t dy = ppar->GetY()- fPrimVertex->GetRefY();


  Double_t c[3] = { ppar->GetCovariance(0,0), ppar->GetCovariance(1,0), ppar->GetCovariance(1,1) };
  c[0]+= Cv[0];  c[1]+= Cv[1];  c[2]+= Cv[2];

  Double_t d = c[0]*c[2] - c[1]*c[1] ;
  Double_t chi = TMath::Sqrt( TMath::Abs( 0.5*(dx*dx*c[0]-2*dx*dy*c[1]+dy*dy*c[2])/d ) );
  Bool_t isNull = (TMath::Abs(d)<1.e-20);

  if(isNull) fChi2Vtx = -1.;
  else       fChi2Vtx = chi;

}

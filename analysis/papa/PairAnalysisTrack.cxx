//
// TObject bits are used to flag the MC matching between detector tracklets
// bits used are >14 and used according to CbmDetectorList.h -> DetectorId
//
//

//#include <TObjArray.h>

#include <TDatabasePDG.h>
#include <TLorentzVector.h>

#include "FairTrackParam.h"
#include "CbmDetectorList.h"

#include "CbmTrack.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"
#include "CbmMuchTrack.h"
#include "CbmTrdTrack.h"
#include "CbmRichRing.h"
#include "CbmTofHit.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"

#include "PairAnalysisTrack.h"

ClassImp(PairAnalysisTrack)

PairAnalysisTrack::PairAnalysisTrack() :
  TNamed(),
  fGlblTrack(0x0),
  fStsTrack(0x0),
  fMuchTrack(0x0),
  fTrdTrack(0x0),
  fRichRing(0x0),
  fTofHit(0x0),
  fMCTrack(0x0),
  fStsTrackMatch(0x0),
  fMuchTrackMatch(0x0),
  fTrdTrackMatch(0x0),
  fRichRingMatch(0x0),
  fRichProj(0x0),
  fMomentum(),
  fPosition(),
  fCharge(0),
  fPdgCode(0),
  fLabel(-1),
  fWeight(1.),
  fMultiMatch(0)
{
  //
  // Default Constructor
  //

}

//______________________________________________
PairAnalysisTrack::PairAnalysisTrack(const char* name, const char* title) :
  TNamed(name, title),
  fGlblTrack(0x0),
  fStsTrack(0x0),
  fMuchTrack(0x0),
  fTrdTrack(0x0),
  fRichRing(0x0),
  fTofHit(0x0),
  fMCTrack(0x0),
  fStsTrackMatch(0x0),
  fMuchTrackMatch(0x0),
  fTrdTrackMatch(0x0),
  fRichRingMatch(0x0),
  fRichProj(0x0),
  fMomentum(),
  fPosition(),
  fCharge(0),
  fPdgCode(0),
  fLabel(-1),
  fWeight(1.),
  fMultiMatch(0)
{
  //
  // Named Constructor
  //

}

//______________________________________________
PairAnalysisTrack::PairAnalysisTrack(CbmGlobalTrack *gtrk,
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
  fMomentum(),
  fPosition(),
  fCharge(0),
  fPdgCode(0),
  fLabel(-1),
  fWeight(1.),
  fMultiMatch(0)
{
  //
  // Constructor
  //
  //TODO: check which track components should go into it, do fitting?
  TVector3 mom;
  ststrk->GetParamFirst()->Momentum(mom);
  fMomentum.SetVect(mom);
  Double_t m2=TMath::Power(TDatabasePDG::Instance()->GetParticle(11)->Mass(), 2);
  fMomentum.SetE( TMath::Sqrt(mom.Mag2()+m2) );
  TVector3 pos;
  ststrk->GetParamFirst()->Position(pos);
  fPosition.SetVect(pos);

  fCharge  = (ststrk->GetParamFirst()->GetQp()>0. ? +1. : -1. );
  if(mctrk) fPdgCode = mctrk->GetPdgCode(); 
}

//______________________________________________
PairAnalysisTrack::PairAnalysisTrack(const PairAnalysisTrack& track) :
  TNamed(track.GetName(), track.GetTitle()),
  fGlblTrack(0),
  fStsTrack(0),
  fMuchTrack(0),
  fTrdTrack(0),
  fRichRing(0),
  fTofHit(0),
  fMCTrack(0),
  fStsTrackMatch(0),
  fMuchTrackMatch(0),
  fTrdTrackMatch(0),
  fRichRingMatch(0),
  fRichProj(0),
  fMomentum(track.fMomentum),
  fPosition(track.fPosition),
  fCharge(track.Charge()),
  fPdgCode(track.PdgCode()),
  fLabel(track.GetLabel()),
  fWeight(track.GetWeight()),
  fMultiMatch(0)
{
  //
  // Copy Constructor
  //

  this->SetBit(BIT(14+kTOF),  track.TestBit(BIT(14+kTOF) ));
  this->SetBit(BIT(14+kRICH), track.TestBit(BIT(14+kRICH)));
  this->SetBit(BIT(14+kTRD),  track.TestBit(BIT(14+kTRD) ));
  this->SetBit(BIT(14+kSTS),  track.TestBit(BIT(14+kSTS) ));
  this->SetBit(BIT(14+kMUCH), track.TestBit(BIT(14+kMUCH)));
  // TVector3 mom;
  // ststrk->GetParamFirst()->Momentum(mom);
  // fMomentum.SetVect(mom);
  // Double_t m2=TMath::Power(TDatabasePDG::Instance()->GetParticle(11)->Mass(), 2);
  // fMomentum.SetE( TMath::Sqrt(mom.Mag2()+m2) );
  // TVector3 pos;
  // ststrk->GetParamFirst()->Position(pos);
  // fPosition.SetVect(pos);

  // fCharge  = (ststrk->GetParamFirst()->GetQp()>0. ? +1. : -1. );
  // if(mctrk) fPdgCode = mctrk->GetPdgCode(); 
}

//______________________________________________
PairAnalysisTrack::~PairAnalysisTrack()
{
  //
  // Default Destructor
  //

}

//______________________________________________
CbmTrackMatchNew* PairAnalysisTrack::GetTrackMatch(DetectorId det) const
{
  //
  // get track match depending on detector id
  //
  switch(det) {
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
  case kSTS:  return fStsTrack;
  case kTRD:  return fTrdTrack;
  case kMUCH: return fMuchTrack;
  case kRICH: return 0x0;
  default:   return 0x0;
  }

}

//______________________________________________
void PairAnalysisTrack::SetMassHypo(Int_t pdg1, Int_t pdg2)
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
  // match STS charge of track to pid and set mass accordingly
  if(fCharge*cpdg1 < 0)      m2=mpdg2*mpdg2;
  else if(fCharge*cpdg2 < 0) m2=mpdg1*mpdg1;
  else                       Printf("PairAnalysisTrack::SetMassHypo via STS charge went wrong!");

  // use TOF time(ns) and track length(cm) if available
  if(fTofHit && 0) { //TODO: switched OFF!!
    m2 = fMomentum.Mag2() * (TMath::Power( (fTofHit->GetTime()*1e-9*TMath::C()) /
					   fGlblTrack->GetLength()/100,           2)  - 1);
  }

  // set mass hypo
  fMomentum.SetE( TMath::Sqrt(fMomentum.Mag2() + m2) );

}

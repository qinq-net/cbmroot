///////////////////////////////////////////////////////////////////////////
//                PairAnalysisEvent                                        //
//                                                                       //
//                                                                       //
/*
Add Detailed description


*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////
#include <Rtypes.h>
#include <TObjArray.h>
#include <TArrayS.h>


#include "FairRootManager.h"
#include "FairMCPoint.h"
#include "FairTrackParam.h"

#include "CbmDetectorList.h"
#include "CbmMCEventHeader.h"

#include "CbmVertex.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"
#include "CbmMuchTrack.h"
#include "CbmTrdTrack.h"
#include "CbmRichRing.h"
#include "CbmTofHit.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"

#include "PairAnalysisTrack.h"
#include "PairAnalysisEvent.h"

ClassImp(PairAnalysisEvent)

PairAnalysisEvent::PairAnalysisEvent() :
  TNamed(),
  fMCHeader(0x0),       //mc header
  fMCTracks(0x0),       //mc tracks
  fStsMatches(0x0),     //STS matches
  fMuchMatches(0x0),     //MUCH matches
  fTrdMatches(0x0),     //TRD matches
  fRichMatches(0x0),     //RICH matches
  fMvdPoints(0x0),      //MVD points
  fStsPoints(0x0),      //STS points
  fMuchPoints(0x0),      //MUCH points
  fRichPoints(0x0),      //RICH points
  fTrdPoints(0x0),      //TRD points
  fTofPoints(0x0),     //TOF points
  fGlobalTracks(0x0),   //global tracks
  fTrdTracks(0x0),      //TRD tracks
  fStsTracks(0x0),      //STS tracks
  fMuchTracks(0x0),      //MUCH tracks
  fRichRings(0x0),      //RICH rings
  fMvdHits(0x0),      //MVD hits
  fStsHits(0x0),      //STS hits
  fMuchHits(0x0),      //MUCH hits
  fMuchHitsStraw(0x0),   //MUCH hits
  fTrdHits(0x0),      //TRD hits
  fRichHits(0x0),      //RICH hits
  fTofHits(0x0),      //TOF hits
  fRichProjection(0x0),
  fMvdHitMatches(0x0),      //MVD hits
  fStsHitMatches(0x0),      //STS hits
  fRichHitMatches(0x0),      //RICH hits
  fMuchHitMatches(0x0),      //MUCH hits
  fTrdHitMatches(0x0),      //TRD hits
  fTofHitMatches(0x0),      //TOF hits
  fPrimVertex(0x0),     //primary vertices
  fTracks(new TObjArray(1)), // array of papa tracks
  fMultiMatch(0)
{
  //
  // Default Constructor
  //
  fTracks->SetOwner(kTRUE);

}

//______________________________________________
PairAnalysisEvent::PairAnalysisEvent(const char* name, const char* title) :
  TNamed(name, title),
  fMCHeader(0x0),       //mc header
  fMCTracks(0x0),       //mc tracks
  fStsMatches(0x0),     //STS matches
  fMuchMatches(0x0),     //MUCH matches
  fTrdMatches(0x0),     //TRD matches
  fRichMatches(0x0),     //RICH matches
  fMvdPoints(0x0),      //MVD points
  fStsPoints(0x0),      //STS points
  fMuchPoints(0x0),      //MUCH points
  fRichPoints(0x0),      //RICH points
  fTrdPoints(0x0),      //TRD points
  fTofPoints(0x0),     //TOF points
  fGlobalTracks(0x0),   //global tracks
  fTrdTracks(0x0),      //TRD tracks
  fStsTracks(0x0),      //STS tracks
  fMuchTracks(0x0),      //MUCH tracks
  fRichRings(0x0),      //RICH rings
  fMvdHits(0x0),      //MVD hits
  fStsHits(0x0),      //STS hits
  fMuchHits(0x0),      //MUCH hits
  fMuchHitsStraw(0x0),   //MUCH hits
  fTrdHits(0x0),      //TRD hits
  fRichHits(0x0),      //RICH hits
  fTofHits(0x0),      //TOF hits
  fRichProjection(0x0),
  fMvdHitMatches(0x0),      //MVD hits
  fStsHitMatches(0x0),      //STS hits
  fRichHitMatches(0x0),      //RICH hits
  fMuchHitMatches(0x0),      //MUCH hits
  fTrdHitMatches(0x0),      //TRD hits
  fTofHitMatches(0x0),      //TOF hits
  fPrimVertex(0x0),     //primary vertices
  fTracks(new TObjArray(1)), // array of papa tracks
  fMultiMatch(0)
{
  //
  // Named Constructor
  //
  fTracks->SetOwner(kTRUE);
}

//______________________________________________
PairAnalysisEvent::~PairAnalysisEvent()
{
  //
  // Default Destructor
  //
  delete fPrimVertex;

  fTracks->Clear("C");
  fGlobalTracks->Delete();   //global tracks
  fMCHeader->Delete();       //mc tracks
  fMCTracks->Delete();       //mc tracks

  fTrdTracks->Delete();      //TRD tracks
  fStsTracks->Delete();      //STS tracks
  fMuchTracks->Delete();      //MUCH tracks
  fRichRings->Delete();      //RICH rings

  fStsMatches->Delete();     //STS matches
  fMuchMatches->Delete();     //MUCH matches
  fTrdMatches->Delete();     //TRD matches
  fRichMatches->Delete();     //RICH matches

  fMvdPoints->Delete();      //MVD hits
  fStsPoints->Delete();      //STS hits
  fMuchPoints->Delete();      //MUCH hits
  fRichPoints->Delete();      //RICH hits
  fTrdPoints->Delete();      //TRD hits
  fTofPoints->Delete();     //TOF matches

  fMvdHits->Delete();      //MVD hits
  fStsHits->Delete();      //STS hits
  fMuchHits->Delete();      //MUCH hits
  fMuchHitsStraw->Delete();      //MUCH hits
  fTrdHits->Delete();      //TRD hits
  fRichHits->Delete();      //RICH hits
  fTofHits->Delete();      //TOF hits

  fRichProjection->Delete();
  fMvdHitMatches->Delete();      //MVD hits
  fStsHitMatches->Delete();      //STS hits
  fMuchHitMatches->Delete();      //MUCH hits
  fRichHitMatches->Delete();      //RICH hits
  fTrdHitMatches->Delete();      //TRD hits
  fTofHitMatches->Delete();      //TOF hits
}

//______________________________________________
void PairAnalysisEvent::SetInput(FairRootManager *man)
{
  //
  // setup the track/hit branches
  //
  fGlobalTracks = (TClonesArray*) man->GetObject("GlobalTrack");
  fTrdTracks    = (TClonesArray*) man->GetObject("TrdTrack");
  fStsTracks    = (TClonesArray*) man->GetObject("StsTrack");
  fMuchTracks   = (TClonesArray*) man->GetObject("MuchTrack");
  fRichRings    = (TClonesArray*) man->GetObject("RichRing");
  fPrimVertex   = (CbmVertex*)    man->GetObject("PrimaryVertex");
  // MC matches and tracks
  fMCHeader     = (CbmMCEventHeader*) man->GetObject("MCEventHeader.");
  fMCTracks     = (TClonesArray*) man->GetObject("MCTrack");
  fStsMatches   = (TClonesArray*) man->GetObject("StsTrackMatch");
  fMuchMatches  = (TClonesArray*) man->GetObject("MuchTrackMatch");
  fTrdMatches   = (TClonesArray*) man->GetObject("TrdTrackMatch");
  fRichMatches  = (TClonesArray*) man->GetObject("RichRingMatch");
  // hits
  fMvdHits      = (TClonesArray*) man->GetObject("MvdHit");
  fStsHits      = (TClonesArray*) man->GetObject("StsHit");
  fMuchHits     = (TClonesArray*) man->GetObject("MuchPixelHit");
  fMuchHitsStraw= (TClonesArray*) man->GetObject("MuchStrawHit");
  fTrdHits      = (TClonesArray*) man->GetObject("TrdHit");
  fRichHits     = (TClonesArray*) man->GetObject("RichHit");
  fTofHits      = (TClonesArray*) man->GetObject("TofHit");
  // hit matches
  fMvdHitMatches = (TClonesArray*) man->GetObject("MvdHitMatch");
  fStsHitMatches = (TClonesArray*) man->GetObject("StsHitMatch");
  fRichHitMatches = 0x0;//(TClonesArray*) man->GetObject("RichHitMatch");
  fMuchHitMatches = (TClonesArray*) man->GetObject("MuchHitMatch");
  fTrdHitMatches = (TClonesArray*) man->GetObject("TrdHitMatch");
  fTofHitMatches = 0x0;//(TClonesArray*) man->GetObject("TofHitMatch");
  // mc points
  fMvdPoints    = (TClonesArray*) man->GetObject("MvdPoint");
  fStsPoints    = (TClonesArray*) man->GetObject("StsPoint");
  fRichPoints   = 0x0;//(TClonesArray*) man->GetObject("RichPoint");
  fMuchPoints   = (TClonesArray*) man->GetObject("MuchPoint");
  fTrdPoints    = (TClonesArray*) man->GetObject("TrdPoint");
  fTofPoints    = (TClonesArray*) man->GetObject("TofPoint");

  fRichProjection = (TClonesArray*) man->GetObject("RichProjection");
  //  if(fMCTracks)   printf("PairAnalysisEvent::SetInput: size of mc array: %04d \n",fMCTracks->GetSize());
}

//______________________________________________
void PairAnalysisEvent::Init()
{
  //
  // initialization of track arrays
  //
  fTracks->Clear("C");
  if(!fGlobalTracks) return;

  TArrayS matches;
  if(fMCTracks) matches.Set(fMCTracks->GetEntriesFast());

  // loop over all glbl tracks
  for (Int_t i=0; i<fGlobalTracks->GetEntriesFast(); i++) {
    // global track
    CbmGlobalTrack *gtrk=static_cast<CbmGlobalTrack*>(fGlobalTracks->UncheckedAt(i));
    if(!gtrk) continue;

    Int_t itrd  = gtrk->GetTrdTrackIndex();
    Int_t ists  = gtrk->GetStsTrackIndex();
    Int_t irich = gtrk->GetRichRingIndex();
    Int_t itof  = gtrk->GetTofHitIndex();
    Int_t imuch = gtrk->GetMuchTrackIndex();

    // reconstructed tracks
    CbmTrdTrack *trdTrack=0x0;
    if(fTrdTracks && itrd>=0) trdTrack=static_cast<CbmTrdTrack*>(fTrdTracks->At(itrd));
    CbmStsTrack *stsTrack=0x0;
    if(fStsTracks && ists>=0) stsTrack=static_cast<CbmStsTrack*>(fStsTracks->At(ists));
    CbmRichRing *richRing=0x0;
    if(fRichRings && irich>=0) richRing=static_cast<CbmRichRing*>(fRichRings->At(irich));
    CbmTofHit *tofHit=0x0;
    if(fTofHits && itof>=0) tofHit=static_cast<CbmTofHit*>(fTofHits->At(itof));
    CbmMuchTrack *muchTrack=0x0;
    if(fMuchTracks && imuch>=0) muchTrack=static_cast<CbmMuchTrack*>(fMuchTracks->At(imuch));

    // track matches
    CbmTrackMatchNew *stsMatch = 0x0;
    if(stsTrack) stsMatch = static_cast<CbmTrackMatchNew*>( fStsMatches->At(ists) );
    Int_t istsMC = (stsMatch && stsMatch->GetNofHits()>0 ? stsMatch->GetMatchedLink().GetIndex() : -1 );
    CbmTrackMatchNew *muchMatch = 0x0;
    if(muchTrack) muchMatch = static_cast<CbmTrackMatchNew*>( fMuchMatches->At(imuch) );
    Int_t imuchMC = (muchMatch && muchMatch->GetNofHits()>0 ? muchMatch->GetMatchedLink().GetIndex() : -1 );
    CbmTrackMatchNew *trdMatch = 0x0;
    if(trdTrack) trdMatch = static_cast<CbmTrackMatchNew*>( fTrdMatches->At(itrd) );
    Int_t itrdMC = (trdMatch ? trdMatch->GetMatchedLink().GetIndex() : -1 );
    CbmTrackMatchNew *richMatch = 0x0;
    if(richRing) richMatch = static_cast<CbmTrackMatchNew*>( fRichMatches->At(irich) );
    Int_t irichMC = (richMatch && richMatch->GetNofHits()>0 ? richMatch->GetMatchedLink().GetIndex() : -1 );
    FairMCPoint *tofPoint = 0x0;
    if(tofHit && tofHit->GetRefId()>=0) tofPoint = static_cast<FairMCPoint*>( fTofPoints->At(tofHit->GetRefId()) );
    Int_t itofMC = (tofPoint ? tofPoint->GetTrackID() : -1 );

    // rich projection
    FairTrackParam *richProj = 0x0;
    if(fRichProjection) richProj = static_cast<FairTrackParam*>(fRichProjection->At(i));

    // monte carlo track based on the STS match!!!
    Int_t iMC = istsMC;
    CbmMCTrack *mcTrack=0x0;
    if(fMCTracks && iMC>=0) mcTrack=static_cast<CbmMCTrack*>(fMCTracks->At(iMC));
    // increment position in matching array
    if(mcTrack && fMCTracks) matches[istsMC]++;
    // build papa track
    fTracks->AddAtAndExpand(new PairAnalysisTrack(gtrk, stsTrack,muchTrack,trdTrack,richRing,tofHit,
						  mcTrack, stsMatch,muchMatch,trdMatch,richMatch,
						  richProj),
			    i);

    // set MC label and matching bits
    PairAnalysisTrack *tr = static_cast<PairAnalysisTrack*>(fTracks->UncheckedAt(i));
    if(iMC<0) iMC=-999; // STS tracks w/o MC matching
    tr->SetLabel(iMC);
    //      tr->SetBit(BIT(14+kMVD),  (iMC==imvdMC)  );
    tr->SetBit(BIT(14+kSTS),  (iMC==istsMC)  );
    tr->SetBit(BIT(14+kRICH), (iMC==irichMC) );
    tr->SetBit(BIT(14+kTRD),  (iMC==itrdMC)  );
    tr->SetBit(BIT(14+kTOF),  (iMC==itofMC) );
    tr->SetBit(BIT(14+kMUCH), (iMC==imuchMC)  );

  }

  // number of multiple matched tracks
  for(Int_t i=0; i<matches.GetSize(); i++)
    fMultiMatch += (matches[i] > 1 ? matches[i] : 0);

}


PairAnalysisTrack *PairAnalysisEvent::GetTrack(UInt_t pos)
{
  //
  // intialize the papa track and return it
  //

  // check intitialisation
  if(fTracks->GetSize()<=pos || !fTracks->At(pos))
    Fatal("PairAnalysisEvent::GetTrack","Event initialisation failed somehow !!!");

  return static_cast<PairAnalysisTrack*>(fTracks->At(pos));

}

//______________________________________________
Int_t PairAnalysisEvent::GetNumberOfMatches(DetectorId det) const
{
  //
  // number of track matches
  //
  switch(det) {
  case kSTS:  return (fStsMatches  ? fStsMatches->GetEntriesFast()  : 0);
  case kMUCH: return (fMuchMatches ? fMuchMatches->GetEntriesFast() : 0);
  case kTRD:  return (fTrdMatches  ? fTrdMatches->GetEntriesFast()  : 0);
  case kRICH: return (fRichMatches ? fRichMatches->GetEntriesFast() : 0);
  default:   return 0;
  }

}

//______________________________________________
Int_t PairAnalysisEvent::GetNumberOfHits(DetectorId det) const
{
  //
  // number of reconstructed hits
  //
  if(!GetHits(det)) { return 0; }
  else {              return (GetHits(det)->GetEntriesFast()); }
}

//______________________________________________
Int_t PairAnalysisEvent::GetNumberOfPoints(DetectorId det) const
{
  //
  // number of reconstructed hits
  //
  if(!GetPoints(det)) { return 0; }
  else {                return (GetPoints(det)->GetEntriesFast()); }
}

//______________________________________________
TClonesArray *PairAnalysisEvent::GetHits(DetectorId det) const {
  //
  // get hits array for certain detector
  //
  //TODO: add much straw hits
  switch(det) {
  case kMVD: return fMvdHits;
  case kSTS: return fStsHits;
  case kMUCH:return fMuchHits; //pixel
  case kTRD: return fTrdHits;
  case kRICH:return fRichHits;
  case kTOF: return fTofHits;
  default:   return 0x0;
  }

}

//______________________________________________
TClonesArray *PairAnalysisEvent::GetHitMatches(DetectorId det) const {
  //
  // get hit matches array for certain detector
  //
  //TODO: add much straw hits
  switch(det) {
  case kMVD: return fMvdHitMatches;
  case kSTS: return fStsHitMatches;
  case kMUCH:return fMuchHitMatches; //pixel
  case kTRD: return fTrdHitMatches;
  case kRICH:return fRichHitMatches;
  case kTOF: return fTofHitMatches;
  default:   return 0x0;
  }

}

//______________________________________________
TClonesArray *PairAnalysisEvent::GetPoints(DetectorId det) const {
  //
  // get mc points array for certain detector
  //
  switch(det) {
  case kMVD: return fMvdPoints;
  case kSTS: return fStsPoints;
  case kMUCH:return fMuchPoints;
  case kTRD: return fTrdPoints;
  case kRICH:return fRichPoints;
  case kTOF: return fTofPoints;
  default:   return 0x0;
  }

}

//______________________________________________
void PairAnalysisEvent::Clear(Option_t *opt)
{
  //
  // clear arrays
  //
  fTracks->Clear("C");
  // fGlobalTracks->Delete();   //global tracks
  // fTrdTracks->Delete();      //TRD tracks
  // fStsTracks->Delete();      //STS tracks
  // fMCTracks->Delete();       //mc tracks
  // fStsMatches->Delete();     //STS matches

}

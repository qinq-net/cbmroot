///////////////////////////////////////////////////////////////////////////
//                PairAnalysis Mixed Event                               //
//                                                                       //
//                                                                       //
/*
Detailed description


*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include <TObjArray.h>
#include <TExMap.h>
#include <TProcessID.h>

#include "CbmVertex.h"
#include "PairAnalysisTrack.h"

#include "PairAnalysisMixedEvent.h"

ClassImp(PairAnalysisMixedEvent)

PairAnalysisMixedEvent::PairAnalysisMixedEvent() :
  TNamed(),
  fArrTrackP(),
  fArrTrackN(),
  fArrVertex("CbmVertex",1),
  fArrPairs("PairAnalysisPair",0),
  fNTracksP(0),
  fNTracksN(0),
  fEventData(),
  fPID(0x0),
  fPIDIndex(0)
{
  //
  // Default Constructor
  //

}

//______________________________________________
PairAnalysisMixedEvent::PairAnalysisMixedEvent(const char* name, const char* title) :
  TNamed(name, title),
  fArrTrackP(),
  fArrTrackN(),
  fArrVertex("CbmVertex",1),
  fArrPairs("PairAnalysisPair",0),
  fNTracksP(0),
  fNTracksN(0),
  fEventData(),
  fPID(0x0),
  fPIDIndex(0)
{
  //
  // Named Constructor
  //
}

//______________________________________________
PairAnalysisMixedEvent::~PairAnalysisMixedEvent()
{
  //
  // Default Destructor
  //
  fArrTrackP.Delete();
  fArrTrackN.Delete();
  fArrVertex.Delete();
  fArrPairs.Delete();
}

//______________________________________________
void PairAnalysisMixedEvent::SetTracks(const TObjArray &arrP, const TObjArray &arrN, const TObjArray &/*arrPairs*/)
{
  //
  // Setup PairAnalysisPairs
  // assumes that the objects in arrP and arrN are
  //

  //Clear out old entries before filling new ones
  Clear();
  // we keep the tracks buffered to minimise new / delete operations
  fNTracksN=0;
  fNTracksP=0;

  //check size of the arrays
  if (fArrTrackP.GetSize()<arrP.GetSize()) {
    fArrTrackP.Expand(arrP.GetSize());
  }
  if (fArrTrackN.GetSize()<arrN.GetSize()) {
    fArrTrackN.Expand(arrN.GetSize());
  }

  TExMap mapStoredVertices;
  fPIDIndex=TProcessID::GetPIDs()->IndexOf(fPID);
  // fill particles
  Int_t tracks=0;
  for (Int_t itrack=0; itrack<arrP.GetEntriesFast(); ++itrack){
    PairAnalysisTrack *track=dynamic_cast<PairAnalysisTrack*>(arrP.At(itrack));
    if (!track) continue;

    // buffer track
    PairAnalysisTrack   *ctrack = new (fArrTrackP[tracks]) PairAnalysisTrack(*track);

    // buffer vertex, don't duplicate
    // most particles will be assiciated to the primary vertex ...
    /*
      CbmVertex *vtx=track->GetProdVertex();
      CbmVertex *cvertex = 0x0;
      if (vtx){
        cvertex = reinterpret_cast<CbmVertex*>(mapStoredVertices.GetValue(reinterpret_cast<ULong64_t>(vtx)));
        if (!cvertex) {
          if (mapStoredVertices.Capacity()<=mapStoredVertices.GetSize()) mapStoredVertices.Expand(2*mapStoredVertices.GetSize());
          if (fArrVertex.GetSize()<=fArrVertex.GetEntriesFast()) fArrVertex.Expand(2*fArrVertex.GetSize());
          cvertex = new (fArrVertex[fArrVertex.GetEntriesFast()]) CbmVertex(*vtx);
          AssignID(cvertex);
          mapStoredVertices.Add(reinterpret_cast<ULong64_t>(vtx),reinterpret_cast<ULong64_t>(cvertex));
        }
      }
      ctrack->SetProdVertex(cvertex);
    */
    ++tracks;
    }
  fNTracksP=tracks;

  tracks=0;
  for (Int_t itrack=0; itrack<arrN.GetEntriesFast(); ++itrack){
    PairAnalysisTrack *track=dynamic_cast<PairAnalysisTrack*>(arrN.At(itrack));
    if (!track) continue;

    //buffer track
    PairAnalysisTrack   *ctrack = new (fArrTrackN[tracks]) PairAnalysisTrack(*track);

    // buffer vertex, don't duplicate
    // most particles will be assiciated to the primary vertex ...
    /*
    CbmVertex *vtx=track->GetProdVertex();
    CbmVertex *cvertex = 0x0;
      if (vtx){
        cvertex = reinterpret_cast<CbmVertex*>(mapStoredVertices.GetValue(reinterpret_cast<ULong64_t>(vtx)));
        if (!cvertex) {
          if (mapStoredVertices.Capacity()<=mapStoredVertices.GetSize()) mapStoredVertices.Expand(2*mapStoredVertices.GetSize());
          if (fArrVertex.GetSize()<=fArrVertex.GetEntriesFast()) fArrVertex.Expand(2*fArrVertex.GetSize());
          cvertex = new (fArrVertex[fArrVertex.GetEntriesFast()]) CbmVertex(*vtx);
          AssignID(cvertex);
          mapStoredVertices.Add(reinterpret_cast<ULong64_t>(vtx),reinterpret_cast<ULong64_t>(cvertex));
        }
      }
      ctrack->SetProdVertex(cvertex);
    */
    ++tracks;
  }
  fNTracksN=tracks;

  //TODO: pair arrays
}

//______________________________________________
void PairAnalysisMixedEvent::Clear(Option_t *opt)
{
  //
  // clear arrays
  //
  //   fArrTrackP.Clear(opt);
  //   fArrTrackN.Clear(opt);

  for (Int_t i=fArrTrackP.GetEntriesFast()-1; i>=0; --i){
    delete fArrTrackP.RemoveAt(i);
  }

  for (Int_t i=fArrTrackN.GetEntriesFast()-1; i>=0; --i){
    delete fArrTrackN.RemoveAt(i);
  }

  for (Int_t i=0; i<fArrVertex.GetEntriesFast(); ++i){
    delete fArrVertex.RemoveAt(i);
  }

  fArrTrackP.ExpandCreateFast(1);
  fArrTrackN.ExpandCreateFast(1);
  fArrVertex.ExpandCreateFast(1);

  fArrPairs.Clear(opt);
}

//______________________________________________
void PairAnalysisMixedEvent::Set(Int_t /*size*/)
{
  //
  // set size of array
  //
  // fArrTrackP.SetClass("PairAnalysisTrack",size);
  // fArrTrackN.SetClass("PairAnalysisTrack",size);
  fArrTrackP.SetClass("PairAnalysisTrack",1000);
  fArrTrackN.SetClass("PairAnalysisTrack",1000);
}

//______________________________________________
void PairAnalysisMixedEvent::SetEventData(const Double_t data[PairAnalysisVarManager::kNMaxValuesMC])
{
  //
  // copy only evnet variables
  //
  for (Int_t i=PairAnalysisVarManager::kPairMax; i<PairAnalysisVarManager::kNMaxValuesMC;++i) fEventData[i]=data[i];
}

//______________________________________________
void PairAnalysisMixedEvent::AssignID(TObject *obj)
{
  //
  // Custom function to assign a uid to an object with an own process id
  // to avoid problems buffering the vertices
  //
  UInt_t uid=1;
  if (fPID->GetObjects()) uid=fPID->GetObjects()->GetEntriesFast();
  uid+=(fPIDIndex<<24);
  obj->SetBit(kIsReferenced);
  obj->SetUniqueID(uid);
  fPID->PutObjectWithID(obj);
}

#ifndef PAIRANALYSISEVENT_H
#define PAIRANALYSISEVENT_H

//#############################################################
//#                                                           #
//#      Class PairAnalysisEvent                                #
//#      Class for event structure and references to tracks   #
//#                                                           #
//#  Authors:                                                 #
//#   Julian    Book,     Uni Ffm / Julian.Book@cern.ch       #
//#                                                           #
//#############################################################
#include <TNamed.h>
#include <TClonesArray.h>
#include <TObjArray.h>

#include "CbmDetectorList.h"
#include "CbmVertex.h"

class FairRootManager;
class PairAnalysisTrack;

class PairAnalysisEvent : public TNamed {
public:
  PairAnalysisEvent();
  PairAnalysisEvent(const char*name, const char* title);

  virtual ~PairAnalysisEvent();

  void SetInput(FairRootManager *man);
  void Init();


  // getters
  CbmVertex *GetPrimaryVertex()       const { return fPrimVertex; }
  Int_t      GetNumberOfTracks()      const { return (fGlobalTracks ? fGlobalTracks->GetEntriesFast() : -1); }
  Int_t      GetNumberOfMCTracks()    const { return (fMCTracks ? fMCTracks->GetEntriesFast() : -1); }
  Int_t      GetNumberOfVageMatches() const { return fMultiMatch; }
  Int_t      GetNumberOfMatches(DetectorId det) const;

  TClonesArray *GetHits(DetectorId det) const;
  TClonesArray *GetPoints(DetectorId det) const;

  TClonesArray *GetTrdHits()          const { return fTrdHits; }
  TClonesArray *GetTrdHitMatches()    const { return fTrdHitMatches; }
  TClonesArray *GetTrdPoints()        const { return fTrdPoints; }

  TObjArray    *GetTracks()           const { return fTracks; }
  PairAnalysisTrack *GetTrack(UInt_t pos);

  virtual void Clear(Option_t *opt="C");


private:
 TClonesArray *fMCTracks;       //mc tracks
 TClonesArray *fStsMatches;     //STS matches
 TClonesArray *fMuchMatches;     //MUCH matches
 TClonesArray *fTrdMatches;     //TRD matches
 TClonesArray *fRichMatches;    //RICH matches

 TClonesArray *fStsPoints;        //STS points
 TClonesArray *fMuchPoints;      //MUCH points
 TClonesArray *fRichPoints;      //RICH points
 TClonesArray *fTrdPoints;        //TRD points
 TClonesArray *fTofPoints;      //TOF points

 TClonesArray *fGlobalTracks;   //global tracks
 TClonesArray *fTrdTracks;      //TRD tracks
 TClonesArray *fStsTracks;      //STS tracks
 TClonesArray *fMuchTracks;      //MUCH tracks
 TClonesArray *fRichRings;     //RICH rings

 TClonesArray *fStsHits;        //STS hits
 TClonesArray *fMuchHits;       //MUCH pixel hits
 TClonesArray *fMuchHitsStraw;  //MUCH straw hits
 TClonesArray *fTrdHits;        //TRD hits
 TClonesArray *fRichHits;       //RICH hits
 TClonesArray *fTofHits ;       //TOF hits

 TClonesArray *fRichProjection;       //RICH projection

 TClonesArray *fTrdHitMatches;        //TRD hits

 CbmVertex    *fPrimVertex;     //primary vertex
 TObjArray    *fTracks;         //papa tracks

 Int_t         fMultiMatch;     // number of mutiple matched tracks 

 PairAnalysisEvent(const PairAnalysisEvent& event);
 PairAnalysisEvent &operator=(const PairAnalysisEvent &c);

 ClassDef(PairAnalysisEvent,3)         // PairAnalysis Event
};



#endif

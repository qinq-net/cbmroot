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
class CbmMCEventHeader;
class CbmStsTrack;
class PairAnalysisTrack;

class PairAnalysisEvent : public TNamed {
public:
  PairAnalysisEvent();
  PairAnalysisEvent(const char*name, const char* title);

  virtual ~PairAnalysisEvent();

  void SetInput(FairRootManager *man);
  void Init();


  // getters
  CbmMCEventHeader *GetMCHeader()     const { return fMCHeader; }
  CbmVertex *GetPrimaryVertex()       const { return fPrimVertex; }
  Int_t      GetNumberOfTracks()      const { return (fGlobalTracks ? fGlobalTracks->GetEntriesFast() : -1); }
  Int_t      GetNumberOfMCTracks()    const { return (fMCTracks ? fMCTracks->GetEntriesFast() : -1); }
  Int_t      GetNumberOfVageMatches() const { return fMultiMatch; }
  Int_t      GetNumberOfMatches(DetectorId det) const;
  Int_t      GetNumberOfHits(   DetectorId det) const;
  Int_t      GetNumberOfPoints( DetectorId det) const;

  TClonesArray *GetHits(      DetectorId det) const;
  TClonesArray *GetHitMatches(DetectorId det) const;
  TClonesArray *GetPoints(    DetectorId det) const;

  TClonesArray *GetTrdHits()          const { return fTrdHits; }
  TClonesArray *GetTrdHitMatches()    const { return fTrdHitMatches; }
  TClonesArray *GetTrdPoints()        const { return fTrdPoints; }

  TObjArray    *GetTracks()           const { return fTracks; }
  PairAnalysisTrack *GetTrack(UInt_t pos);

  virtual void Clear(Option_t *opt="C");
protected:

  Int_t GetMvdMatchingIndex(CbmStsTrack *track) const;

private:
 CbmMCEventHeader *fMCHeader;       //mc header
 TClonesArray *fMCTracks;       //mc tracks
 TClonesArray *fStsMatches;     //STS matches
 TClonesArray *fMuchMatches;     //MUCH matches
 TClonesArray *fTrdMatches;     //TRD matches
 TClonesArray *fRichMatches;    //RICH matches

 TClonesArray *fMvdPoints;        //MVD points
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

 TClonesArray *fMvdHits;        //MVD hits
 TClonesArray *fStsHits;        //STS hits
 TClonesArray *fMuchHits;       //MUCH pixel hits
 TClonesArray *fMuchHitsStraw;  //MUCH straw hits
 TClonesArray *fTrdHits;        //TRD hits
 TClonesArray *fRichHits;       //RICH hits
 TClonesArray *fTofHits ;       //TOF hits

 TClonesArray *fRichProjection;       //RICH projection
 
 TClonesArray *fMvdHitMatches;    // hit matches
 TClonesArray *fStsHitMatches;    // hit matches
 TClonesArray *fRichHitMatches;    // hit matches
 TClonesArray *fMuchHitMatches;    // hit matches
 TClonesArray *fTrdHitMatches;    // hit matches
 TClonesArray *fTofHitMatches;    // hit matches

 CbmVertex    *fPrimVertex;     //primary vertex
 TObjArray    *fTracks;         //papa tracks

 Int_t         fMultiMatch;     // number of mutiple matched tracks 

 PairAnalysisEvent(const PairAnalysisEvent& event);
 PairAnalysisEvent &operator=(const PairAnalysisEvent &c);

 ClassDef(PairAnalysisEvent,5)         // PairAnalysis Event
};



#endif

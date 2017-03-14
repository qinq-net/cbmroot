/** @file CbmBuildEventsFromTracksReal.h
 ** @author Valentina Akishina <v.akishina@gsi.de>, Maksym Zyzak <m.zyzak@gsi.de>
 ** @date 14.03.2017
 **/
#ifndef CBMBUILDEVENTSFROMTRACKSREAL_H_
#define CBMBUILDEVENTSFROMTRACKSREAL_H 1

#include <FairTask.h>
#include "CbmStsTrack.h"
#include "CbmMCDataArray.h"

class TClonesArray;
class CbmMCEventList;

class CbmBuildEventsFromTracksReal: public FairTask 
{
 public:

  /** Constructor **/
  CbmBuildEventsFromTracksReal();

  /** Destructor **/
  virtual ~CbmBuildEventsFromTracksReal();

  /** Task execution **/
  virtual void Exec(Option_t* opt);

  void QA();

 private:
   
  struct SortTracks {
  CbmStsTrack Track;
  int index;
  bool used;
  };

  static bool CompareTrackTime(const SortTracks& a, const SortTracks& b) { return (a.Track.GetTime() < b.Track.GetTime()); }

  TClonesArray* fStsDigis;   ///< Input array (class CbmStsDigi)
  TClonesArray* fStsTracks;   ///< Input array (class CbmStsDigi)
  CbmMCDataArray* fMCTracks;   ///< Input array (class CbmStsDigi)
  TClonesArray* fStsHits;   ///< Input array (class CbmStsDigi)
  TClonesArray* fMcPoints;   ///< Input array (class CbmStsDigi)
  TClonesArray* fEvents;     ///< Output array (class CbmEvent)
  TClonesArray* fStsTrackMatchArray;

  CbmMCEventList* fEventList;
  


  /** Task initialisation **/
  virtual InitStatus Init();

  CbmBuildEventsFromTracksReal(const CbmBuildEventsFromTracksReal&);
  CbmBuildEventsFromTracksReal& operator=(const CbmBuildEventsFromTracksReal&);
 
  ClassDef(CbmBuildEventsFromTracksReal, 1);

};

#endif /* CBMBUILDEVENTSFROMTRACKS_H */

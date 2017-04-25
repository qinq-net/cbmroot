/** @file CbmBuildEventsFromTracksReal.h
 ** @author Valentina Akishina <v.akishina@gsi.de>, Maksym Zyzak <m.zyzak@gsi.de>
 ** @date 14.03.2017
 **/
#ifndef CBMBUILDEVENTSFROMTRACKSREAL_H_
#define CBMBUILDEVENTSFROMTRACKSREAL_H 1

#include <FairTask.h>
#include "CbmStsTrack.h"

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

 private:
   
  struct SortTracks 
  {
    CbmStsTrack Track;
    int index;
    bool used;
  };

  static bool CompareTrackTime(const SortTracks& a, const SortTracks& b) { return (a.Track.GetTime() < b.Track.GetTime()); }

  TClonesArray* fStsTracks;   ///< Input array (class CbmStsDigi)
  TClonesArray* fEvents;     ///< Output array (class CbmEvent)

  /** Task initialisation **/
  virtual InitStatus Init();

  CbmBuildEventsFromTracksReal(const CbmBuildEventsFromTracksReal&);
  CbmBuildEventsFromTracksReal& operator=(const CbmBuildEventsFromTracksReal&);
 
  ClassDef(CbmBuildEventsFromTracksReal, 1);
};

#endif /* CBMBUILDEVENTSFROMTRACKS_H */

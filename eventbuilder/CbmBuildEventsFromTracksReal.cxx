/** @file CbmBuildEventsFromTracksReal.cxx
 ** @author Valentina Akishina <v.akishina@gsi.de>, Maksym Zyzak <m.zyzak@gsi.de>
 ** @date 14.03.2017
 **/

#include "CbmBuildEventsFromTracksReal.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "FairLogger.h"
#include "FairRootManager.h"
#include "CbmDetectorList.h"
#include "CbmEvent.h"
#include "CbmStsHit.h"
#include "CbmStsDigi.h"

#include "CbmMCDataArray.h"
#include "CbmMCDataManager.h"
#include "CbmMCTrack.h"
#include "CbmMCEventList.h"
#include "CbmTrackMatchNew.h"

using namespace std;


// =====   Constructor   =====================================================
CbmBuildEventsFromTracksReal::CbmBuildEventsFromTracksReal() :
	FairTask("CbmBuildEventsFromTracksReal"),
	fStsDigis(NULL),
	fMCTracks(NULL),
	fStsTracks(NULL),
	fEvents(NULL)
	//fNofEntries(0)
{
}
// ===========================================================================



// =====   Destructor   ======================================================
CbmBuildEventsFromTracksReal::~CbmBuildEventsFromTracksReal() {
//   fEvents->Delete();
}
// ===========================================================================

// =====   Task initialisation   =============================================
InitStatus CbmBuildEventsFromTracksReal::Init() {


        // --- Get FairRootManager instance
  FairRootManager* ioman = FairRootManager::Instance();
  assert ( ioman );

  CbmMCDataManager* mcManager = (CbmMCDataManager*) ioman->GetObject("MCDataManager");
  if( mcManager == NULL )
    LOG(FATAL) << GetName() << ": No CbmMCDataManager!" << FairLogger::endl;
  
  fMCTracks = (CbmMCDataArray*) mcManager->InitBranch("MCTrack");
  if ( fMCTracks == NULL )
    LOG(FATAL) << GetName() << ": No MCTrack data!" << FairLogger::endl;
  
  fEventList = (CbmMCEventList*) ioman->GetObject("MCEventList.");
  if(fEventList==0)
  {
    Error("CbmKFParticleFinderQA::Init","MC Event List not found!");
    return kERROR;
  }

  fStsTracks=(TClonesArray*) ioman->GetObject("StsTrack");
  assert ( fStsTracks );

  fStsTrackMatchArray = (TClonesArray*) ioman->GetObject("StsTrackMatch");
  if(fStsTrackMatchArray==0)
  {
    Error("CbmKFParticleFinderQA::Init","track match array not found!");
    return kERROR;
  }
  

  // Register output array (CbmStsDigi)
  fEvents = new TClonesArray("CbmEvent",100);
  ioman->Register("Event", "CbmEvent", fEvents,
                                        IsOutputBranchPersistent("Event"));

  return kSUCCESS;
}
// ===========================================================================

// =====   Task execution   ==================================================
void CbmBuildEventsFromTracksReal::Exec(Option_t*) {

	TStopwatch timer;
	timer.Start();
	std::map<Int_t, CbmEvent*> eventMap;

	// Clear output array
	fEvents->Delete();    

	UInt_t nTracks = fStsTracks->GetEntriesFast();

  vector<CbmStsTrack> vRTracks_new;
  vector <int> Index;
   


  
  vector <SortTracks> Tracks;
  
  vector< vector<int> > timeClusters;
  int nUsedTracks = 0;
  
  
  
  
   for(int iTrack=0; iTrack<nTracks; iTrack++)
  {
    CbmStsTrack* track = (CbmStsTrack*) fStsTracks->At(iTrack);
    
    Double_t time = track->GetTime();
    Double_t timeEr = track->GetTimeError();

    SortTracks structure;
    
    structure.Track = *track;
    structure.index = iTrack;
    structure.used = 0;
    
    if (timeEr > 0 && timeEr == timeEr && time == time) Tracks.push_back(structure);
  }
  
  // std::sort(Tracks.begin(), Tracks.end(), CompareTrackTime);
  

   nUsedTracks = 0;
  
   vector< vector<UInt_t> > clusters;

  
   while(nUsedTracks < Tracks.size())
   {

     vector<UInt_t> cluster;  
     
    //find first unused track and create a new cluster
    for(int iTrack=0; iTrack<Tracks.size(); iTrack++)
      if(!Tracks[iTrack].used)
      {
        cluster.push_back(Tracks[iTrack].index);
        Tracks[iTrack].used = true;
        nUsedTracks++;
        break;
      }
    
    //form a cluster around this track
    for(int iTrack=0; iTrack<Tracks.size(); iTrack++)
    {
      if(Tracks[iTrack].used) continue;

      float time1  = Tracks[iTrack].Track.GetTime();
      float timeC1 = Tracks[iTrack].Track.GetTimeError();

      for(int iClusterTrack=0; iClusterTrack<cluster.size(); iClusterTrack++)
      {
        CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(cluster[iClusterTrack]);
        
        float time2  = stsTrack->GetTime();
        float timeC2 = stsTrack->GetTimeError();
        
        float dt = fabs(time1 - time2);
        float error = sqrt(timeC1+ timeC2); ///&&????????????????
        
        if((dt<4*error)&&(dt<8.5) )
        {
          cluster.push_back(Tracks[iTrack].index);
          Tracks[iTrack].used = true;
          nUsedTracks++;
          break;
        }
      }
    }

    
    float time_min = 100000000000;
    float time_max =0;
    
    for(int iClusterTrack=0; iClusterTrack<cluster.size(); iClusterTrack++)
      {
        CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(cluster[iClusterTrack]);
        
        if (time_min>stsTrack->GetTime()) time_min=stsTrack->GetTime();
        
        if (time_max<stsTrack->GetTime()) time_max=stsTrack->GetTime();
      }
    
    if (cluster.size()>1) clusters.push_back(cluster);
  }

        
  for(int iEvent=0; iEvent<clusters.size(); iEvent++)
  {
     CbmEvent* event = NULL;
     Int_t nEvents = fEvents->GetEntriesFast();

   //  if (clusters[iEvent].size()>1) 
     { 
            
       event = new ( (*fEvents)[nEvents] ) CbmEvent(iEvent);
       event->SetStsTracks(clusters[iEvent]);
            
     }
  }        

  timer.Stop();
}



struct CbmBuildEventMCTrack
{
  CbmBuildEventMCTrack():
    fMCFileId(-1), fMCEventId(-1), fMCTrackId(-1), fRecoTrackId(), fRecoEventId()
  {
  }
  
  int fMCFileId;
  int fMCEventId;
  int fMCTrackId;
  
  vector<int> fRecoTrackId;
  vector<int> fRecoEventId;
};

ClassImp(CbmBuildEventsFromTracksReal)


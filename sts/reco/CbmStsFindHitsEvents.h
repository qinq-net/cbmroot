/** @file CbmStsFindHitsEvents.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 17.06.2014
 ** @date 21.10.2014
 **/


#ifndef CBMSTSFINDHITSEVENTS_H
#define CBMSTSFINDHITSEVENTS_H 1

#include <set>
#include "TStopwatch.h"
#include "FairTask.h"
#include "CbmStsModule.h"
#include "CbmStsSensorTypeDssd.h"

class TClonesArray;
class CbmEvent;
class CbmStsSetup;


/** @class CbmStsFindHitsEvents
 ** @brief Task class for finding STS hits in event-by-event mode
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 17.06.2014
 ** @date 21.10.2016
 ** @version 1.0
 **
 ** This task constructs hits (3-d points) from clusters. In each module,
 ** the intersection points from each pair of front and back side
 ** clusters are calculated and stored as hit. Obviously, the cluster
 ** finding task has to be run before this one.
 **
 ** This version follows the new event data model, where one tree entry
 ** corresponds to one time slice. It assumes that events are already
 ** defined based on digis.
 **
 ** The origin of the class is the "old" CbmStsFindHits task, which operates
 ** directly on the tree entry without using the event class.
 **/
class CbmStsFindHitsEvents : public FairTask
{

	public:

    /** Constructor **/
    CbmStsFindHitsEvents();


    /** Destructor  **/
    virtual ~CbmStsFindHitsEvents();


    /** Task execution **/
    virtual void Exec(Option_t* opt);


    /** End-of-run action **/
    virtual void Finish();


    /** End-of-event action  **/
    virtual void FinishEvent();


    /** Initialisation **/
    virtual InitStatus Init();


    /** Set dead time for time-based hit finding **/
    virtual void SetDTime(Double_t dTime) { fDTime = dTime; }


	private:

    TClonesArray* fEvents;               ///< Input array of CbmEvent
    TClonesArray* fClusters;             ///< Input array of CbmStsCluster
    TClonesArray* fHits;                 ///< Output array of CbmStsHits
    CbmStsSetup*  fSetup;                ///< Instance of STS setup
    TStopwatch    fTimer;                ///< ROOT timer
    Double_t fDTime;					           ///< Dead time for time-based hit finding

    // --- Run counters
    Int_t    fNofEvents;       ///< Total number of events processed
    Double_t fNofClustersTot;  ///< Total number of clusters processed
    Double_t fNofHitsTot;      ///< Total number of hits produced
    Double_t fTimeTot;         ///< Total execution time


    /** Process one event
     ** @param event  Pointer to event object
     ** @value Number of hits produced in this event
     **/
    Int_t ProcessEvent(CbmEvent* event);


    /** Sort clusters into modules
     ** @param  event  Pointer to event object
     ** @return Number of clusters sorted
     **/
    Int_t SortClusters(CbmEvent* event);


    /** Prevent usage of copy constructor and assignment operator **/
    CbmStsFindHitsEvents(const CbmStsFindHitsEvents&);
    CbmStsFindHitsEvents operator=(const CbmStsFindHitsEvents&);


    ClassDef(CbmStsFindHitsEvents, 1);
};

#endif

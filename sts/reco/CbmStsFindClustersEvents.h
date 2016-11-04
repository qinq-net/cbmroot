/** @file CbmFindClustersEvents.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 ** @date 19.10.2016
 **/


#ifndef CBMSTSFINDCLUSTERSEVENTS_H
#define CBMSTSFINDCLUSTERSEVENTS_H 1

#include <map>
#include <set>
#include <string>
#include <vector>
#include "TStopwatch.h"
#include "FairTask.h"
#include "CbmStsDigi.h"
#include "reco/CbmStsClusterAnalysis.h"
#include "setup/CbmStsModule.h"
#include "CbmTimeSlice.h"

class TClonesArray;
class CbmEvent;
class CbmStsSetup;
class CbmStsClusterFinder;

/** @class CbmStsFindClustersEvents
 ** @brief Task class for finding STS clusters in events
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 ** @data 19.10.2016
 ** @version 1.0
 **
 ** This task groups StsDigis into clusters. Digis are first sorted w.r.t.
 ** the module they are registered by; the cluster finding is then performed
 ** in each module.
 **
 ** The actual cluster finding algorithm is defined in an engine class
 ** derived from CbmStsClusterFinder.
 **
 ** This version follows the new event data model, where one tree entry
 ** corresponds to one time slice. It assumes that events are already
 ** defined based on digis.
 **/
class CbmStsFindClustersEvents : public FairTask
{

	public:
    /** Constructor 
     ** @param finderModel  Cluster finder model.
     **			    0 = ideal: using MC information
     **			    1 = real : neighbouring strips
     **			    2 = gap  : allow gaps where dead channels are
     **/
    CbmStsFindClustersEvents(Int_t finderModel = 1);


    /** Destructor  **/
    virtual ~CbmStsFindClustersEvents();


    /** Task execution **/
    virtual void Exec(Option_t* opt);


    /** End-of-run action **/
    virtual void Finish();


    /** End-of-event action **/
    virtual void FinishEvent();


    /** Get array with CbmStsClusters
     ** @value Pointer to cluster TClonesArray
     **/
    TClonesArray* GetClusters() { return fClusters; }


    /** Initialisation **/
    virtual InitStatus Init();


    /** Set the parameters for all modules in the CbmSetup.
     ** Currently, all modules have the same parameters, which are defined
     ** in the constructor (default) or by the SetParameters() method.
     **/
    void SetModuleParameters();


    /** Set parameters for all modules if digitiser and cluster finder
     ** are used in different macro.
     ** TODO: This has to be solved in a better way without hardcoding.
     **/
    void SetParameters(Double_t dynRange = 75000., Double_t threshold = 3000., Int_t nAdc = 32,
      		           Double_t timeResolution = 10., Double_t deadTime = 800.,
      		           Double_t noise = 1000.) {
         fDynRange       = dynRange;
         fThreshold      = threshold;
         fNofAdcChannels = nAdc;
         fTimeResolution = timeResolution;
         fDeadTime       = deadTime;
         fNoise          = noise;
    }



    private:

    TClonesArray* fEvents;            ///< Input array of events
    TClonesArray* fDigis;             ///< Input array of CbmStsDigi
    TClonesArray* fClusters;          ///< Output array of CbmStsCluster
    CbmStsSetup*  fSetup;             ///< Instance of STS setup
    CbmStsClusterFinder* fFinder;     ///< Cluster finder engine
    CbmStsClusterAnalysis* fAna;      ///< Cluster analysis engine
    TStopwatch    fTimer;             ///< ROOT timer
    Int_t         fFinderModel;       ///< Cluster finder model
    
    // --- Run counters
    Int_t     fNofEvents;       ///< Total number of events processed
    Double_t  fNofDigisTot;     ///< Total number of digis processed
    Double_t  fNofClustersTot;  ///< Total number of clusters produced
    Double_t  fTimeTot;         ///< Total execution time

    // --- Module parameters
    Double_t fDynRange;            ///< Dynamic range [e]
    Double_t fThreshold;           ///< Threshold [e]
    Int_t    fNofAdcChannels;      ///< Number of ADC channels
    Double_t fTimeResolution;      ///< Time resolution (sigma) [ns]
    Double_t fNoise;               ///< equivalent noise charge (sigma) [ns]
    Double_t fDeadTime;            ///< Channel dead time

    /** Set of active modules in the current event **/
    // TODO: This set sorts according to the (random) memory address, which is
    // very inconvenient for debugging. Maybe a list or a vextor would be better.
    std::set<CbmStsModule*> fActiveModules;


    /** Process one event
     ** @param event Pointer to event object
     **/
    void ProcessEvent(CbmEvent* event);

    /** Sort digis into module digi maps
     ** @param  event  Pointer to event object
     ** @return Number of digis sorted
     **/
    Int_t SortDigis(CbmEvent* event);


    /** Prevent usage of copy constructor and assignment operator **/
    CbmStsFindClustersEvents(const CbmStsFindClustersEvents&);
    CbmStsFindClustersEvents operator=(const CbmStsFindClustersEvents&);


    ClassDef(CbmStsFindClustersEvents, 1);
};

#endif

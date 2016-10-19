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
     **			    1 = simple: only neighboring strips
     **			    2 = with gap: take into account dead channels
     ** @param algorithm  Cluster finder algorithm.
     **			    0 = center-of-gravity
     **			    1 = advanced(for 2-strip and bigger clusters)
     **/
    CbmStsFindClustersEvents(Int_t finderModel = 1, Int_t algorithm = 1);


    /** Destructor  **/
    virtual ~CbmStsFindClustersEvents();


    /** Task execution
     ** Inherited from FairTask.
     **/
    virtual void Exec(Option_t* opt);


    /** End-of-run action
     ** Inherited from FairTask.
     **/
    virtual void Finish();


    /** End-of-event action
     ** Inherited from FairTaks
     **/
    virtual void FinishEvent();


    /** Initialisation
     ** Inherited from FairTask.
     **/
    virtual InitStatus Init();
    //void ExecMQ();
    //bool InitMQ(const std::string& geo_file);
    //InitStatus SetTimeSlice(CbmTimeSlice* ts);
    TClonesArray* GetClusters() {return fClusters;}


    /** Set energy loss model in order to take it into account in position error **/
    void SetELossModel(Int_t eLossModel = 1) {fELossModel = eLossModel;}

    /** Set parameters for all modules if digitizer and cluster finder
     ** are used in different macro. Need for time-based case.
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

    void SetModuleParameters();


    private:

    TClonesArray* fEvents;            ///< Input array of events
    TClonesArray* fDigis;             ///< Input array of CbmStsDigi
    TClonesArray* fClusters;          ///< Output array of CbmStsCluster
    CbmStsSetup*  fSetup;             ///< Instance of STS setup
    CbmStsClusterFinder* fFinder;     ///< Cluster finder
    TStopwatch    fTimer;             ///< ROOT timer
    Int_t         fFinderModel;       ///< Cluster finder model
    Int_t         fAlgorithm;         ///< Cluster finder algorithm
    Int_t         fELossModel;        ///< Energy loss model, to take propely into position error
    
    Bool_t fUseFinderTb;              ///< Using of time based cluster finder
    Double_t fDeadTime;               ///< Dead time for time-based cluster finder

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

    /** Set of active modules in the current event **/
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

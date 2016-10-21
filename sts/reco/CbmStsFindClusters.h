/** @file CbmFindClusters.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 16.06.2014
 **/


#ifndef CBMSTSFINDCLUSTERS_H
#define CBMSTSFINDCLUSTERS_H 1

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
class CbmStsSetup;
class CbmStsClusterFinderSimple;

/** @class CbmStsFindClusters
 ** @brief Task class for finding STS clusters
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 ** @version 1.0
 **
 ** This task groups StsDigis into clusters. Digis are first sorted w.r.t.
 ** the module they are registered by; the cluster finding is then performed
 ** in each module.
 **
 ** The actual cluster finding algorithm is defined in an engine class
 ** CbmStsClusterFinderSimple. Other, alternative algorithms can be
 ** plugged in.
 **/
class CbmStsFindClusters : public FairTask
{

	public:
    /** Constructor 
     ** @param finderModel  Cluster finder model.
     **			    0 = ideal: using MC information
     **			    1 = real:  neighbouring strips
     **			    2 = gap:   allow gaps whenre inactive channels are
     **	@param algorithm   Cluster fit algorithm
     **			    0 = center-of-gravity
     **			    1 = advanced(for 2-strip and bigger clusters)
     **/
    CbmStsFindClusters(Int_t finderModel = 1, Int_t algorithm = 1);


    /** Destructor  **/
    virtual ~CbmStsFindClusters();


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
    void ExecMQ();
    bool InitMQ(const std::string& geo_file);
    InitStatus SetTimeSlice(CbmTimeSlice* ts);
    TClonesArray* GetClusters() {return fClusters;}

    /** Set the DAQ flag to use time slices as an input **/
    void UseDaq()					{ fDaq 			= kTRUE;
    								  fUseFinderTb 	= kTRUE; }

    /** Set the FinderTb flag to use time based cluster finder **/
    void UseTbClusterFinder(Double_t dTime = 20.)  { fDeadTime = dTime;
                                                     fUseFinderTb    = kTRUE; }

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

    TClonesArray* fDigis;             ///< Input array of CbmStsDigi
    TClonesArray* fClusters;          ///< Output array of CbmStsCluster
    CbmStsSetup*  fSetup;             ///< Instance of STS setup
    CbmStsClusterFinderSimple* fFinder;  ///< Cluster finder
    TStopwatch    fTimer;             ///< ROOT timer
    Int_t         fFinderModel;       ///< Cluster finder model
    Int_t         fAlgorithm;         ///< Cluster finder algorithm
    Int_t         fELossModel;        ///< Energy loss model, to take propely into position error
    
    CbmTimeSlice* fTimeSlice;         ///< Time slice object in the DAQ approach
    std::vector<CbmStsDigi> fDigiData;     ///< Vector of digis for the time slices
    Bool_t fDaq;                      ///< Using DAQ
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
//    Double_t fDeadTime;            ///< Single-channel dead time [ns]
    Double_t fNoise;               ///< equivalent noise charge (sigma) [ns]

    /** Set of active modules in the current event **/
    std::set<CbmStsModule*> fActiveModules;

    /** Map from module address to map of digis in channels **/
//    std::map<CbmStsModule*, std::map<Int_t, CbmStsDigi*> > fDigiMap;

    /** Sort digis into module digi maps
     ** @return Number of digis sorted
     **/
    Int_t SortDigis();


    /** Prevent usage of copy constructor and assignment operator **/
    CbmStsFindClusters(const CbmStsFindClusters&);
    CbmStsFindClusters operator=(const CbmStsFindClusters&);


    ClassDef(CbmStsFindClusters, 1);
};

#endif

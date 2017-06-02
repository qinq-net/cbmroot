/** @file CbmFindClusters.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 05.04.2017
 **/


#ifndef CBMSTSFINDCLUSTERS_H
#define CBMSTSFINDCLUSTERS_H 1

#include "TStopwatch.h"
#include "FairTask.h"

class TClonesArray;
class CbmStsClusterAnalysis;
class CbmStsClusterFinderModule;
class CbmStsDigitizeSettings;
class CbmEvent;
class CbmStsSetup;


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
 ** The task works in time-based mode, i.e. it can operate both on
 ** time-slice and event input. Set UseEventMode()
 ** to choose event-by-event operation.
 **
 ** The actual cluster finding algorithm is defined in the class
 ** CbmStsClusterFinderModule.
 **/
class CbmStsFindClusters : public FairTask
{

	public:
    /** Constructor **/
    CbmStsFindClusters();


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


    /** Initialisation
     ** Inherited from FairTask.
     **/
    virtual InitStatus Init();


    TClonesArray* GetClusters() {return fClusters;}

    void SetModuleParameters();

    void UseEventMode(Bool_t choice = kTRUE) { fEventMode = choice; }


    private:

    TClonesArray* fEvents;            //! Input array of events
    TClonesArray* fDigis;             //! Input array of CbmStsDigi
    TClonesArray* fClusters;          //! Output array of CbmStsCluster
    CbmStsSetup*  fSetup;             //! Instance of STS setup
    CbmStsDigitizeSettings* fSettings; //! Instance of digi settings
    CbmStsClusterAnalysis* fAna;      //! Instance of Cluster Analysis tool
    TStopwatch    fTimer;             //! ROOT timer
    Bool_t fEventMode;                /// Run event-by-event if kTRUE
    Bool_t fLegacy;                   /// Legacy mode for event-by-event

    // --- Run counters
    Int_t     fNofEntries;      ///< Number of tree entries processed
    Int_t     fNofUnits;        ///< Number of time slices or events processed
    Double_t  fNofDigis;        ///< Total number of digis processed
    Double_t  fNofClusters;     ///< Total number of clusters produced
    Double_t  fTimeTot;         ///< Total execution time

    // --- Module parameters
    /*
    Double_t fDynRange;            ///< Dynamic range [e]
    Double_t fThreshold;           ///< Threshold [e]
    Int_t    fNofAdcChannels;      ///< Number of ADC channels
    Double_t fTimeResolution;      ///< Time resolution (sigma) [ns]
    Double_t fDeadTime;            ///< Single-channel dead time [ns]
    Double_t fNoise;               ///< equivalent noise charge (sigma) [ns]
    */

    // --- Map from module address to cluster finding module
    std::map<UInt_t, CbmStsClusterFinderModule*> fModules;  //!

    // --- Map from time to index of digi
    // --- Use for legacy mode when digis do not come time-ordered
    std::multimap<Double_t, Int_t> fDigiMap;   //!


    /** @brief Instantiate cluster finding modules
     ** @value Number of modules created
     **/
    Int_t CreateModules();


   /** @brief Initialise the digitisation settings
     **
     ** This method read the digi settings object from file,
     ** sets it to the setup and updates the module parameters.
     */
    void InitSettings();


    /** @brief Process one time slice or event
     ** @param event  Pointer to CbmEvent object
     **
     ** If a NULL event pointer is given, the entire input branch is processed.
     **/
    void ProcessData(CbmEvent* event = NULL);


    /** @brief Process one STS digi
     ** @param index  Index of STS digi in its TClonesArray
     **/
    void ProcessDigi(Int_t index);


    /** @brief Process one legacy event
     **
     ** Calls time-sorting before processing.
     **/
    void ProcessLegacyEvent();


    /** @brief Read digis from a legacy event
     ** @value  Number of STS digis in event
     **
     ** The legacy event (event-based STS digitiser) does not deliver
     ** a time-ordered sequence of STS digis, which is required by the cluster
     ** finder. So, digis are time-ordered before processing in this method.
     **/
    Int_t ReadLegacyEvent();


    /** Prevent usage of copy constructor and assignment operator **/
    CbmStsFindClusters(const CbmStsFindClusters&) = delete;
    CbmStsFindClusters operator=(const CbmStsFindClusters&) = delete;


    ClassDef(CbmStsFindClusters, 1);
};

#endif

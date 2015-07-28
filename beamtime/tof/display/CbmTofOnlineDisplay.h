#ifndef CBMTOFONLINEDISPLAY_H
#define CBMTOFONLINEDISPLAY_H

#include "FairTask.h"

class TClonesArray;
class TCanvas;
class TH2;

class CbmTofOnlineDisplay : public FairTask
{
  public:

    /** Default constructor **/
    CbmTofOnlineDisplay();

    /** Constructor with parameters (Optional) **/
    //  CbmTofOnlineDisplay(Int_t verbose);


    /** Destructor **/
    ~CbmTofOnlineDisplay();


    /** Initiliazation of task at the beginning of a run **/
    virtual InitStatus Init();

    /** ReInitiliazation of task when the runID changes **/
    virtual InitStatus ReInit();


    /** Executed for each event. **/
    virtual void Exec(Option_t* opt);

    /** Load the parameter container from the runtime database **/
    virtual void SetParContainers();

    /** Finish task called at the end of the run **/
    virtual void Finish();

    void SetNumberOfTDC(Int_t val) { fNumberOfTDC = val; }
    void SetUpdateInterval(Int_t val) { fUpdateInterval = val; }

    void SetResMonitorEna( Bool_t bInEna = kTRUE ){ fbMonitorRes = bInEna; };

    void MonitorCts( Bool_t bMonitor = kTRUE){ fbMonitorCts = bMonitor; };
    void MonitorSebStatus( Bool_t bMonitor = kTRUE){ fbMonitorSebStatus = bMonitor; };
    void MonitorTdcStatus( Bool_t bMonitor = kTRUE){ fbMonitorTdcStatus = bMonitor; };
    void MonitorFSMockup( Bool_t bMonitor = kTRUE){ fbMonitorFSMockup = bMonitor; };

    void SetNumberOfSEB(Int_t val) { fNumberOfSEB = val; }

  private:

    /** Input array from previous already existing data level **/
    //  TClonesArray* <InputDataLevel>;

    /** Output array to  new data level**/
    //  TClonesArray* <OutputDataLevel>;

    TCanvas* fTdcChannelOccupancy; //!

    Int_t fNumberOfTDC;
    Int_t fNumberOfSEB;
    Int_t fUpdateInterval;
    Int_t fEventCounter;

    Bool_t   fbMonitorCts;
    TCanvas* fCtsTriggerMonitor; //!

    Bool_t   fbMonitorSebStatus;
    TCanvas* fSebSizeMonitor; //!
    TCanvas* fSebStatusMonitor; //!

    Bool_t   fbMonitorTdcStatus;
    TCanvas* fTdcSizeMonitor; //!
    TCanvas* fTdcStatusMonitor; //!
    
    Bool_t   fbMonitorFSMockup;
    TCanvas* fFSMockupMonitor; //!

    // Monitoring of the TDC resolution stability using reference channel
    Bool_t   fbMonitorRes;
    TCanvas* fOverviewRes; //!
    TH2*     fhResolutionSummary;

    Bool_t   fbMonitorDigiStatus;
    TCanvas* fDigiSizeMonitor; //!
    TCanvas* fDigiStatusMonitor; //!

    CbmTofOnlineDisplay(const CbmTofOnlineDisplay&);
    CbmTofOnlineDisplay operator=(const CbmTofOnlineDisplay&);

    ClassDef(CbmTofOnlineDisplay,1);
};

#endif

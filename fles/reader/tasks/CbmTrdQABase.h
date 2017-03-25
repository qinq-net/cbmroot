#ifndef CBMTRDQABASE_H
#define CBMTRDQABASE_H

#include "FairTask.h"
#include <CbmHistManager.h>
#include "CbmSpadicRawMessage.h"
#include "TClonesArray.h"
#include <algorithm>

class CbmTrdQABase : public FairTask
{
    protected:
    /** Constructor with parameters (Optional) **/
    CbmTrdQABase(TString ClassName);
    public:

    /** Default constructor **/
    CbmTrdQABase():CbmTrdQABase("CbmTrdQABase"){};

    /** Destructor **/
    ~CbmTrdQABase();


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

    /*To ie implemented by inheriting Class*/
    virtual void CreateHistograms();
/*
    * Executed after each event. *
    virtual void FinishEvent();

    * Action at end of run. For this task and all of the subtasks. *
    virtual void FinishTask();
*/
//    inline Int_t GetSpadicID(Int_t sourceA);
    //Functions to analyze Spadic Messages
    //TODO: Implement indirectly in terms of Beamtime tools
    virtual TString GetSpadicName(Int_t RobID,Int_t SpadicID,TString,Bool_t);
    virtual Int_t GetRobID(CbmSpadicRawMessage* raw);
    virtual Int_t GetSpadicID(CbmSpadicRawMessage* raw);
    inline TString GetTriggerName(Int_t Triggertype){
      TString triggerTypes[4] = { "Global_ trigger",
				  "Self_triggered",
				  "Neighbor_triggered",
				  "Self_and_neighbor_triggered"};
      if(Triggertype<-1||Triggertype>3){
	LOG(FATAL)<< this->GetName() << ": Unknown Triggertype "<< Triggertype<<FairLogger::endl;
      }
      if(Triggertype==-1)
	return TString("No_Hit");
      return triggerTypes[Triggertype];
    }
    inline TString GetStopName(Int_t Stoptype){    
      TString stopTypes[6] = {"Normal_end_of_message", 
			  "Channel_buffer_full", 
			  "Ordering_FIFO_full", 
			  "Multi_hit", 
			  "Multi_hit_and_channel_buffer_full", 
			  "Multi_hit_and_ordering_FIFO_full"};
    if(Stoptype<0||Stoptype>5){
	LOG(FATAL)<< this->GetName() << ": Unknown Stoptype "<< Stoptype<<FairLogger::endl;
      }
      return stopTypes[Stoptype];
    }

    /*    virtual Int_t GetModuleID(CbmSpadicRawMessage* raw);
    virtual Int_t GetLayerID(CbmSpadicRawMessage* raw);
    virtual Int_t GetSectorID(CbmSpadicRawMessage* raw);
    virtual Int_t GetRowID(CbmSpadicRawMessage* raw);
    virtual Int_t GetColumnID(CbmSpadicRawMessage* raw);
    virtual Int_t GetCombiID(CbmSpadicRawMessage* raw);
    virtual Int_t GetChannelOnPadPlane(Int_t SpadicChannel);
    virtual Bool_t FragmentedPulseTest(CbmSpadicRawMessage* raw);
    virtual Int_t GetMaximumAdc(CbmSpadicRawMessage* raw);
    virtual Float_t GetIntegratedCharge(CbmSpadicRawMessage* raw);
    */

  protected:

    /** Input array from previous already existing data level **/
    TClonesArray* fRaw;
    TClonesArray* fInput;
    
    CbmHistManager* fHm;


  public:

    CbmTrdQABase(const CbmTrdQABase&);
    CbmTrdQABase operator=(const CbmTrdQABase&);

    ClassDef(CbmTrdQABase,1);
};

#endif

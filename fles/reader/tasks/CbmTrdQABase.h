#ifndef CBMTRDQABASE_H
#define CBMTRDQABASE_H

#include "FairTask.h"
#include "CbmTrdTestBeamTools.h"
#include <CbmHistManager.h>
#include "CbmSpadicRawMessage.h"
#include "TClonesArray.h"
#include <algorithm>

class CbmTrdQABase : public FairTask
{
    protected:

    /** Input array from previous already existing data level **/
    TClonesArray* fRaw;
    TClonesArray* fInput;
    CbmTrdTestBeamTools* fBT;
    CbmHistManager* fHm;


 public:
  /** Constructor with parameters (Optional) **/
     CbmTrdQABase(TString ClassName):CbmTrdQABase(nullptr,ClassName){};

    CbmTrdQABase(CbmTrdTestBeamTools*,TString);

    /** Default constructor **/
 CbmTrdQABase():CbmTrdQABase(nullptr,TString("CbmTrdQABase")){};

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
    TString GetSpadicName(Int_t RobID,Int_t SpadicID,TString RobName="SysCore",Bool_t FullSpadic=true){
      return fBT->GetSpadicName(RobID,SpadicID,RobName,FullSpadic);  
    };
    Int_t GetRobID(CbmSpadicRawMessage* raw){
      return fBT->GetRobID(raw);
    };
    Int_t GetSpadicID(CbmSpadicRawMessage* raw){
      return fBT->GetSpadicID(raw);
    };
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
    Int_t GetModuleID(CbmSpadicRawMessage* raw){
      return fBT->GetModuleID(raw);}
    ;
    Int_t GetLayerID(CbmSpadicRawMessage* raw){
      return fBT->GetLayerID(raw);
    };
    Int_t GetRowID(CbmSpadicRawMessage* raw){
      return fBT->GetRowID(raw);
    };
    Int_t GetColumnID(CbmSpadicRawMessage* raw){
      return fBT->GetColumnID(raw);
    };
    Int_t GetChannelOnPadPlane(CbmSpadicRawMessage* raw){
      return fBT->GetChannelOnPadPlane(raw);
    };
    Int_t GetMaximumAdc(CbmSpadicRawMessage* raw){
      return fBT->GetMaximumAdc(raw);
    };
    //    virtual Float_t GetIntegratedCharge(CbmSpadicRawMessage* raw);
    


  public:

    CbmTrdQABase(const CbmTrdQABase&);
    CbmTrdQABase operator=(const CbmTrdQABase&);

    ClassDef(CbmTrdQABase,1);
};

#endif

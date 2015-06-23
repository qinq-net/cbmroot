#ifndef CBMTRDRAWBEAMPROFILE_H
#define CBMTRDRAWBEAMPROFILE_H

#include "FairTask.h"
#include "CbmHistManager.h"
#include "CbmSpadicRawMessage.h"
#include "TClonesArray.h"

class CbmTrdRawBeamProfile : public FairTask
{
  public:

    /** Default constructor **/
    CbmTrdRawBeamProfile();

    /** Constructor with parameters (Optional) **/
    //  CbmTrdRawBeamProfile(Int_t verbose);


    /** Destructor **/
    ~CbmTrdRawBeamProfile();


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

    virtual Int_t GetModuleID(CbmSpadicRawMessage* raw);
    virtual Int_t GetLayerID(CbmSpadicRawMessage* raw);
    virtual Int_t GetSectorID(CbmSpadicRawMessage* raw);
    virtual Int_t GetRowID(CbmSpadicRawMessage* raw);
    virtual Int_t GetColumnID(CbmSpadicRawMessage* raw);
    virtual Int_t GetChannelOnPadPlane(Int_t SpadicChannel);

  private:

    /** Input array from previous already existing data level **/
    TClonesArray* fRawSpadic;
   /** Output array **/
    TClonesArray* fDigis; //! TRD digis
    TClonesArray* fClusters;
    Int_t fiDigi;
    Int_t fiCluster;
    CbmHistManager* fHM;

    Int_t fMessageCounter;
    Int_t fContainerCounter;

    std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > > fTimeBuffer;// <ASIC ID "Syscore%d_Spadic%d"<Time, <CombiId, SpadicMessage> >

    /** Output array to  new data level**/
    //  TClonesArray* <OutputDataLevel>;

    void CreateHistograms();

    void Clusterizer();

    CbmTrdRawBeamProfile(const CbmTrdRawBeamProfile&);
    CbmTrdRawBeamProfile operator=(const CbmTrdRawBeamProfile&);

    ClassDef(CbmTrdRawBeamProfile,1);
};

#endif

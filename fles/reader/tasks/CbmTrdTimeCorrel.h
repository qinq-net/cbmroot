#ifndef CBMTRDTIMECORREL_H
#define CBMTRDTIMECORREL_H

#include "FairTask.h"

#include "CbmHistManager.h"

#include "TClonesArray.h"
#include "CbmSpadicRawMessage.h"
#include <deque>

typedef std::map<Int_t, std::map<ULong_t, CbmSpadicRawMessage* > > EpochMap;
typedef std::map<Int_t, std::map<Int_t,std::map<ULong_t, Long_t> > > OffsetMap;


class CbmTrdTimeCorrel : public FairTask
{
 public:

  CbmTrdTimeCorrel();
  ~CbmTrdTimeCorrel();

  virtual InitStatus Init();

  virtual InitStatus ReInit();

  virtual void Exec(Option_t* opt);

  virtual void SetParContainers();

  virtual void Finish();

  virtual void FinishEvent();

  void SetRun(Int_t run)               { fRun = run;}
  void SetRewriteSpadicName(Bool_t rewrite)  { fRewriteSpadicName = rewrite; }

  void ClusterizerSpace();

 void ClusterizerTime();

  void CleanUpBuffers();

 private:
  std::map<TString, std::map<ULong_t, std::map<Int_t, CbmSpadicRawMessage*> > > fMessageBuffer;
  std::deque<CbmSpadicRawMessage*> fLinearHitBuffer;
  Int_t   fSpadics;
  Bool_t  fRewriteSpadicName;

  const Bool_t fActivateClusterizer = true;
  const Bool_t fDebugMode = false;
  const Bool_t fDrawSignalShapes = false;
  const Bool_t fDrawPadResponse = true;
  const Bool_t fCalculateBaseline = false;
  const Int_t  fSignalShapeThreshold = -255	;
  Int_t fBaseline[2];
  const Bool_t fActivateDeltaTAnalysis = false;
  const Bool_t fActivateOffsetAnalysis = false;
  const Bool_t fDrawClustertypes = true;


  Int_t   GetSpadicID(Int_t sourceA);

  TString GetSpadicName(Int_t eqID,Int_t sourceA);
  TString RewriteSpadicName(TString spadicName);

  TString GetStopName(Int_t stopType);

  TClonesArray* fRawSpadic;//input per timeSlice
  TClonesArray* fRawMessages;//output without MS overlaps including all message types without isStrange, isEpoch, isEpochOutOfSynch
  TClonesArray* fDigis;
  TClonesArray* fClusters;

  Int_t fiRawMessage;//counter for fRawMessages
  Int_t fiDigi;//counter for fDigis
  Int_t fiCluster;//counter for fClusters

  CbmHistManager* fHM;

  Int_t fNrTimeSlices;
  
  Int_t fRun;

  ULong_t fLastMessageTime[3][6][32];
  /* Store the last message time.
     Intended usage is for the first (outer) argument to be the SysCoreID 
     and the second (inner) to be the SPADICID of a full SPADIC.
     Third argument is the ChannelID.
     Timestamps are stored for a full spadic.
  */

  Int_t fEpochMarkerArray[3][6];
  Bool_t fFirstEpochMarker[3][6];

  // Variables for book keeping after a regress of the Epoch counter  
  Bool_t EpochRegressTriggered[3][6];
  Int_t EpochRegressOffset[3][6];
  Int_t EpochRegressCounter[3][6];
  void FillBaselineHistogram(CbmSpadicRawMessage*);
  void FillSignalShape(CbmSpadicRawMessage&);
  void FillSignalShape(CbmSpadicRawMessage* message){
	  FillSignalShape(*message);
  }

  
  void CreateHistograms();

  void ReLabelAxis(TAxis* axis, TString type, Bool_t underflow, Bool_t overflow);

  Int_t GetMessageType(CbmSpadicRawMessage* raw);

  Int_t GetChannelOnPadPlane(Int_t SpadicChannel);

  // The followinfg block is used to generate TrdDigis
  Int_t GetSectorID(CbmSpadicRawMessage* raw);
  Int_t GetRowID(CbmSpadicRawMessage* raw);
  Int_t GetLayerID(CbmSpadicRawMessage* raw);
  Int_t GetColumnID(CbmSpadicRawMessage* raw);
  Int_t GetModuleID(CbmSpadicRawMessage* raw);
  Int_t GetCharge(CbmSpadicRawMessage&);


  OffsetMap timestampOffsets;
  OffsetMap CalculateTimestampOffsets(const EpochMap &epochBuffer);

  Bool_t fGraph;
  Bool_t fOutputCloneArrays;
  
  friend class Cluster;
 public: class Cluster : public TObject
  {
  public:
    Cluster():Cluster(0){};
    Cluster(Int_t Windowsize):Cluster(Windowsize,-255,-255,true){};
    Cluster(Int_t,Int_t,Int_t,Bool_t);
    ~Cluster();
    Size_t size();
    Int_t Type();
    Int_t Windowsize();
    ULong_t GetFulltime();
    Float_t GetHorizontalPosition();
    Int_t GetSpadic();
    Int_t GetRow();
    std::pair<Int_t,Float_t> GetPosition();
    Int_t GetTotalCharge();
    std::pair<std::vector<CbmSpadicRawMessage>::const_iterator,std::vector<CbmSpadicRawMessage>::const_iterator> GetEntries();
    Bool_t AddEntry (CbmSpadicRawMessage);
    Bool_t FillChargeDistribution(TH2*);
  private:
    std::vector<CbmSpadicRawMessage> fEntries;
    Bool_t fParametersCalculated;
    Bool_t fPreCalculatedBaseline;
    Int_t fSpadic, fRow, fType, fTotalCharge, fWindowsize;
    Int_t fBaseline[2];
    Float_t fHorizontalPosition;
    void CalculateParameters();
    Int_t GetHorizontalMessagePosition(CbmSpadicRawMessage&);
    Int_t GetSpadicID(Int_t);
    Int_t GetChannelOnPadPlane(Int_t);
    Int_t GetCharge(CbmSpadicRawMessage&);
  };
 private:
  std::deque<Cluster> fClusterBuffer;


  CbmTrdTimeCorrel(const CbmTrdTimeCorrel&);
  CbmTrdTimeCorrel operator=(const CbmTrdTimeCorrel&);

  ClassDef(CbmTrdTimeCorrel,1);
};




#endif



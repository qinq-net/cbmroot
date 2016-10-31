// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackTestTof                         -----
// -----               Created 27.10.2016 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTSUnpackTestTof_H
#define CbmTSUnpackTestTof_H

#ifndef __CINT__
  #include "Timeslice.hpp"
  #include "rocMess_wGet4v1.h"
#endif

#include "CbmTSUnpack.h"
#include "CbmHistManager.h"
#include "CbmNxyterRawMessage.h"
#include "CbmFiberHodoDigi.h"
#include "CbmTbDaqBuffer.h"

#include "TClonesArray.h"

#include <vector>
#include <map>

class CbmDigi;

class CbmTSUnpackTestTof : public CbmTSUnpack
{
public:
  
  CbmTSUnpackTestTof( UInt_t uNbGdpb = 1 );
  virtual ~CbmTSUnpackTestTof();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  virtual void Finish();

  void FillOutput(CbmDigi* digi);
  
  void SetMsLimitLevel( size_t uAcceptBoundaryPct = 100 ) { fuMsAcceptsPercent = uAcceptBoundaryPct; }
  size_t GetMsLimitLevel( ) { return fuMsAcceptsPercent; }

private:

  size_t fuMsAcceptsPercent; /** Reject Ms with index inside TS above this, assumes 100 MS per TS **/
  UInt_t fuMinNbGdpb;
  UInt_t fuCurrNbGdpb;

  std::vector<int> fMsgCounter;
  std::map<UInt_t, UInt_t> fGdpbIdIndexMap;
/*
  std::map<int,int> fHodoStationMap;
*/
  CbmHistManager* fHM;  ///< Histogram manager

  /** Current epoch marker for each ROC and GET4 (first epoch in the stream initialises the map item) **/
  std::map<Int_t, std::map<Int_t, UInt_t> > fCurrentEpoch; 
  Int_t fNofEpochs; /** Current epoch marker for each ROC **/
  ULong_t fCurrentEpochTime;     /** Time stamp of current epoch **/

  Int_t fEquipmentId;

//  TClonesArray* fFiberHodoRaw;
//  TClonesArray* fFiberHodoDigi;
//  CbmNxyterRawMessage* fRawMessage;  
//  CbmFiberHodoDigi* fDigi;  

  CbmTbDaqBuffer* fBuffer;

//  void InitializeTofMapping();

  void CreateHistograms();

#ifndef __CINT__
  void FillHitInfo(ngdpb::Message);
  void FillEpochInfo(ngdpb::Message);
  void PrintSlcInfo(ngdpb::Message);
  void PrintSysInfo(ngdpb::Message);
#endif

  CbmTSUnpackTestTof(const CbmTSUnpackTestTof&);
  CbmTSUnpackTestTof operator=(const CbmTSUnpackTestTof&);

  ClassDef(CbmTSUnpackTestTof, 1)
};

#endif

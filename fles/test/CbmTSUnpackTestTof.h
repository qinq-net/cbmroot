// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTSUnpackTestTof                            -----
// -----                 Created 20.06.2016 by F. Uhlig                    -----
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
  
  CbmTSUnpackTestTof();
  virtual ~CbmTSUnpackTestTof();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  virtual void Finish();

  void FillOutput(CbmDigi* digi);

private:

  std::vector<int> fMsgCounter;

  std::map<int,int> fHodoStationMap;
  
  Int_t fHodoFiber[128];  /** Mapping from fiber hodoscope feb channel to fiber number **/
  Int_t fHodoPlane[128];  /** Mapping from fiber hodoscope feb channel to plane number 1=X, 2=Y **/
  Int_t fHodoPixel[128];  /** Mapping from fiber hodoscope feb channel to pixel number **/

  CbmHistManager* fHM;  ///< Histogram manager

  std::map<Int_t, UInt_t> fCurrentEpoch; // Current epoch (first epoch in the stream initialises the map item)
  Int_t fNofEpochs; /** Current epoch marker for each ROC **/
  ULong_t fCurrentEpochTime;     /** Time stamp of current epoch **/

  Int_t fEquipmentId;

  TClonesArray* fFiberHodoRaw;
  TClonesArray* fFiberHodoDigi;
  CbmNxyterRawMessage* fRawMessage;  
  CbmFiberHodoDigi* fDigi;  

  CbmTbDaqBuffer* fBuffer;

  void InitializeFiberHodoMapping();

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

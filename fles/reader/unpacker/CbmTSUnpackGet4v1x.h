// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                      CbmTSUnpackGet4v1x                           -----
// -----                    Created 15.12.2014 by                          -----
// -----                        P.-A. Loizeau                              -----
// -----                  Based on CbmTSUnpackNxyter                       -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKGET4V1X_H
#define CBMTSUNPACKGET4V1X_H

#ifndef __CINT__
  #include "Timeslice.hpp"
#endif

#include "CbmTSUnpack.h"

#include "TClonesArray.h"

#include <vector>

class TH1;
class TH2;

class CbmTSUnpackGet4v1x : public CbmTSUnpack
{
public:
  
  CbmTSUnpackGet4v1x();
  virtual ~CbmTSUnpackGet4v1x();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  virtual void Finish();

  void SetGet4Nb( Int_t iNbChips);

  inline void SetVerbose( Bool_t inVerb = kTRUE ) { fbVerbose = inVerb; }

  inline void SetMode( Int_t inMode = 2 ) { fiMode = inMode; }

  // protected:
  //  virtual void Register();

private:
  // Behavior control variables
  Bool_t fbVerbose;
  Int_t  fiMode;

  // Hardware related variables => MUST be set on startup by user!
  UInt_t fuNbRocs;
  UInt_t fuNbGet4;

  // Epochs book-keeping variables
  Int_t fiCurrEpoch; // Current epoch (first epoch in the stream initialises this variable)
  std::vector< Int_t > fviCurrEpoch2; // Current epoch2 (one per GET4 chip)

  // Monitoring related variables
  TH1 * fhCountsPerChannel;
  TH2 * fhGet4EpochSyncDist;
  TH2 * fhGet4ChanDllStatus;

  // Verbose functions (redundant with Print from GET4 Hack/Tools?)
  void Print6bytesMessage(const uint8_t* msContent_shifted);

  // Monitoring functions
  void InitMonitorHistograms();
  void MonitorMessage_epoch(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void MonitorMessage_sync(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void MonitorMessage_aux(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void MonitorMessage_sys(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void MonitorMessage_epoch2(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void MonitorMessage_get4(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void MonitorMessage_Get4v1(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void FillMonitorHistograms();
  void WriteMonitorHistograms();
  void DeleteMonitorHistograms();

  // Unpacking functions
  // EqID = Equipment identifier from ums, RocID = ROC identifier from ums
  void ProcessMessage_epoch(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_sync(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_aux(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_sys(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_epoch2(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_get4(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_Get4v1(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);

  // Output array of raw GET4 32b hits
//  TClonesArray* fGet4Raw;
  
  // Output array of raw sync messages
//  TClonesArray* fGet4RawSync;
  
  ClassDef(CbmTSUnpackGet4v1x, 1)
};

#endif // CBMTSUNPACKGET4V1X_H

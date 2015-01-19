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

// Specific headers
#include "CbmGet4v1xHackDef.h"
#include "CbmTSUnpack.h"

// ROOT headers
#include "TClonesArray.h"

// C++ std headers
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

  inline void SetRocNb(  UInt_t uNbRocsIn ) { fuNbRocs = uNbRocsIn; }
  inline void SetGet4Nb( UInt_t uNbChipsIn) { fuNbGet4 = uNbChipsIn; }

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
  TH2 * fhMessageTypePerRoc;
  TH2 * fhRocSyncTypePerRoc;
  TH2 * fhRocAuxTypePerRoc;
  TH2 * fhSysMessTypePerRoc;
  TH2 * fhGet4EpochFlags;
  TH2 * fhGet4EpochSyncDist;
  TH1 * fhGet4ChanDataCount;
  TH2 * fhGet4ChanDllStatus;
  TH2 * fhGet4ChanTotMap;
  TH2 * fhGet4ChanErrors;
  TH2 * fhGet4ChanSlowContM;

  // Verbose functions (redundant with Print from GET4 Hack/Tools?)
  void Print6bytesMessage(const uint8_t* msContent_shifted);

  // Monitoring functions
  void InitMonitorHistograms();
  void MonitorMessage_epoch(  get4v1x::Message mess, uint16_t EqID);
  void MonitorMessage_sync(   get4v1x::Message mess, uint16_t EqID);
  void MonitorMessage_aux(    get4v1x::Message mess, uint16_t EqID);
  void MonitorMessage_sys(    get4v1x::Message mess, uint16_t EqID);
  void MonitorMessage_epoch2( get4v1x::Message mess, uint16_t EqID);
  void MonitorMessage_get4(   get4v1x::Message mess, uint16_t EqID);
  void MonitorMessage_Get4v1( get4v1x::Message mess, uint16_t EqID);
  void FillMonitorHistograms();
  void WriteMonitorHistograms();
  void DeleteMonitorHistograms();

  // Unpacking functions
  // EqID = Equipment identifier from ums
  void ProcessMessage_epoch(  get4v1x::Message mess, uint16_t EqID);
  void ProcessMessage_sync(   get4v1x::Message mess, uint16_t EqID);
  void ProcessMessage_aux(    get4v1x::Message mess, uint16_t EqID);
  void ProcessMessage_sys(    get4v1x::Message mess, uint16_t EqID);
  void ProcessMessage_epoch2( get4v1x::Message mess, uint16_t EqID);
  void ProcessMessage_get4(   get4v1x::Message mess, uint16_t EqID);
  void ProcessMessage_Get4v1( get4v1x::Message mess, uint16_t EqID);

  // Output array of raw GET4 32b hits
//  TClonesArray* fGet4Raw;
  
  // Output array of raw sync messages
//  TClonesArray* fGet4RawSync;
  
  ClassDef(CbmTSUnpackGet4v1x, 1)
};

#endif // CBMTSUNPACKGET4V1X_H

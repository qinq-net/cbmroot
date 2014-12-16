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

  virtual void Finish() {;}

  void SetGet4Nb( Int_t iNbChips);

  inline void SetVerbose( Bool_t inVerb = kTRUE ) { bVerbose = inVerb; }

  // protected:
  //  virtual void Register();

private:

  Bool_t bVerbose;

  Int_t fCurrEpoch; // Current epoch (first epoch in the stream initialises this variable)
  std::vector< Int_t > fCurrEpoch2; // Current epoch2 (one per GET4 chip)

  void Print6bytesMessage(const uint8_t* msContent_shifted);

  // EqID = Equipment identifier from ums, RocID = ROC identifier from ums
  void ProcessMessage_hit(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_epoch(const uint8_t* msContent_shifted);
  void ProcessMessage_sync(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_aux(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_sys(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_epoch2(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_get4(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_Get4v1(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);

  // Output array of raw hit messages
//  TClonesArray* fNxyterRaw;
  
  // Output array of raw sync messages
//  TClonesArray* fNxyterRawSync;
  
  ClassDef(CbmTSUnpackGet4v1x, 1)
};

#endif // CBMTSUNPACKGET4V1X_H

// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                      CbmTSUnpackNxyter                            -----
// -----                    Created 07.11.2014 by                          -----
// -----                   E. Ovcharenko, F. Uhlig                         -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CBMTSUNPACKNXYTER_H
#define CBMTSUNPACKNXYTER_H

#ifndef __CINT__
  #include "Timeslice.hpp"
#endif

#include "CbmTSUnpack.h"

#include "TClonesArray.h"

enum MessageTypes {
   MSG_NOP    = 0,
   MSG_HIT    = 1,
   MSG_EPOCH  = 2,
   MSG_SYNC   = 3,
   MSG_AUX    = 4,
   MSG_SYS    = 7
};

class CbmTSUnpackNxyter : public CbmTSUnpack
{
public:
  
  CbmTSUnpackNxyter();
  virtual ~CbmTSUnpackNxyter();
    
  virtual Bool_t Init();
#ifndef __CINT__
  virtual Bool_t DoUnpack(const fles::Timeslice& ts, size_t component);
#endif
  virtual void Reset();

  virtual void Finish() {;}

  // protected:
  //  virtual void Register();

private:

  Int_t fCurrEpoch; // Current epoch (first epoch in the stream initialises this variable)

  void Print6bytesMessage(const uint8_t* msContent_shifted);

  void ProcessMessage_hit(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_epoch(const uint8_t* msContent_shifted);
  void ProcessMessage_sync(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_aux(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);
  void ProcessMessage_sys(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID);

  // Output array of raw hit messages
  TClonesArray* fNxyterRaw;
  
  // Output array of raw sync messages
  TClonesArray* fNxyterRawSync;
  
  ClassDef(CbmTSUnpackNxyter, 1)
};

#endif

#ifndef CBMTRBRAWMESSAGE_H
#define CBMTRBRAWMESSAGE_H 1


#include "CbmRawMessage.h"

class CbmTrbRawMessage : public CbmRawMessage
{

 public:

  /** Default constructor  **/
  CbmTrbRawMessage();

  /** Constructor  **/
  CbmTrbRawMessage(Int_t FlibLink, Int_t FebId, Int_t ChannelId,
                   Int_t EpochMarker, Int_t CoarseTime,
                   Int_t FineTime, Bool_t Edge, Double_t Correction=0.);

  /** Destructor  **/
  virtual ~CbmTrbRawMessage() { };

  void Copy(CbmTrbRawMessage* source);

  Int_t GetTDCfine() { return fTDCfine;}
  Int_t GetTDCcoarse() { return fTime;}
  Int_t GetEdge() { return fEdge;}
  Double_t GetCorr() { return fCorrection; }

  ULong_t GetFullTime();

 private:

  /** Data members **/
  Int_t fTDCfine;
  Bool_t fEdge;
  Double_t fCorrection;

  ClassDef(CbmTrbRawMessage,2);

};

#endif // CBMTRBRAWMESSAGE_H

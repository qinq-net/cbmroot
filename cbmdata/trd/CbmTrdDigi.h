/**
 * \file CbmTrdDigi.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 03.05.2013
 **/

#ifndef CBMTRDDIGI_H
#define CBMTRDDIGI_H 1

#include "CbmDigi.h"
#include "CbmTrdAddress.h"

#include <string>
#include <list>
using std::string;
using std::list;

class CbmTrdDigi : public CbmDigi
{
 public:
  /**
   * \brief Default constructor.
   */
  CbmTrdDigi();

  /**
   * \brief Constructor with assignment.
   * \param[in] address Unique channel address.
   * \param[in] charge Charge.
   * \param[in] time Absolute time [ns].
   */
  CbmTrdDigi(Int_t address, Double_t charge, Double_t time);
  CbmTrdDigi(Int_t address, Double_t charge, Double_t time, Int_t triggerType, Int_t infoType, Int_t stopType);
  CbmTrdDigi(Int_t address, Double_t fullTime, Int_t triggerType, Int_t infoType, Int_t stopType, Int_t nrSamples, Float_t* samples);
  /**
   * \brief Destructor.
   */
  ~CbmTrdDigi();

  /**
   * \brief Inherited from CbmDigi.
   */
  Int_t GetAddress() const { return fAddress; };

  /**
   * \brief Inherited from CbmDigi.
   */
  Double_t GetCharge() const { return fCharge; }

  /**
   * \brief Inherited from CbmDigi.
   */
  Int_t GetSystemId() const { return CbmTrdAddress::GetSystemId(fAddress); }

  /**
   * \brief Inherited from CbmDigi.
   */
  Double_t GetTime() const { return fTime; }

  // Accessors for CbmSpadicRawMessage Testbeam input
  Int_t GetNrSamples() { return fNrSamples; }
  Float_t* GetSamples() { return fSamples; }
  Int_t GetTriggerType() { return fTriggerType; }
  Int_t GetStopType() { return fStopType; }
  Int_t GetInfoType() { return fInfoType; }
  //===============================================

  /** Accessors **/
  void SetAddress(Int_t address) { fAddress = address; }
  void SetCharge(Double_t charge) { fCharge = charge; }
  void SetTime(Double_t time) { fTime = time; }
  void SetTriggerType(Int_t triggerType) {fTriggerType = triggerType; }
  void SetInfoType(Int_t infoType) {fInfoType = infoType; }
  void SetStopType(Int_t stopType) {fStopType = stopType; }
  void SetPulseShape(Float_t pulse[45]);
  /** Modifiers **/
  void AddCharge(Double_t charge) { fCharge += charge; }

  string ToString() const;
  
 private:
  Int_t fn_FNR_Triggers;
  Int_t fAddress; ///< Unique channel address
  Double_t fCharge; ///< Charge
  Double_t fTime; ///< Absolute time [ns]
  Int_t fTriggerType;
  Int_t fInfoType;
  Int_t fStopType;
  Int_t fBufferOverflowCount; 
  Int_t fNrSamples;
  Float_t fSamples[45];

  ClassDef(CbmTrdDigi, 6);
};

#endif

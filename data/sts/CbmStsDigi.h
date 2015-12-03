/** @file CbmStsDigi.h
 ** @author V.Friese <v.friese@gsi.de>
 ** @since 28.08.2006
 ** @version 4.0
 **/


#ifndef CBMSTSDIGI_H
#define CBMSTSDIGI_H 1

#include "CbmDigi.h"

#include "CbmDetectorList.h"

#ifndef __CINT__
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#endif //__CINT__

/** @class CbmStsDigi
 ** @brief Data class for a single-channel message in the STS
 **
 ** The CbmStsDigi is the ROOT representation of the smallest information
 ** unit delivered by the CBM-STS by a single readout channel. It carries
 ** the channel address, the measurement time and the digitised charge
 ** as information.
 **/
class CbmStsDigi : public CbmDigi
{

 public:

  /** Default constructor **/
  CbmStsDigi() : CbmDigi(), fAddress(0), fTime(0), fCharge(0) { }


  /** Standard constructor
   ** @param  address  Unique element address
   ** @param  time     Measurement time [ns]
   ** @param  charge   Charge [ADC units]
   **/
  CbmStsDigi(UInt_t address, ULong64_t time, UShort_t charge)
      : CbmDigi(), fAddress(address), fTime(time), fCharge(charge) { }


  /** Destructor **/
  virtual ~CbmStsDigi() { };


  /** Unique detector element address  (see CbmStsAddress)
   ** @value Unique address of readout channel
   **/
  virtual Int_t GetAddress() const { return fAddress; }


  /** Charge
   ** @value Charge [ADC units]
   **/
  virtual Double_t GetCharge() const { return Double_t(fCharge); }


 /** System ID
  ** @value System identifier (enum DetectorId)
  **/
  virtual Int_t GetSystemId() const { return kSTS; }


  /** Time of measurement
   ** @value Time [ns]
   **/
  virtual Double_t GetTime() const { return Double_t(fTime); }

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar& fAddress;
    ar& fTime;
    ar& fCharge;
  }

 private:

#ifndef __CINT__ // for BOOST serialization
  friend class boost::serialization::access;
#endif // for BOOST serialization

  UInt_t    fAddress;     ///< Unique detector address
  ULong64_t fTime;        ///< Time [ns]
  UShort_t  fCharge;      ///< Charge [ADC units]


  ClassDef(CbmStsDigi,4);

};


#endif

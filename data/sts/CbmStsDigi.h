/** @file CbmStsDigi.h
 ** @author V.Friese <v.friese@gsi.de>
 ** @since 28.08.2006
 ** @version 6
 **/


#ifndef CBMSTSDIGI_H
#define CBMSTSDIGI_H 1

#include "CbmDefs.h"
#include "CbmDigi.h"

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
  CbmStsDigi() :
    CbmDigi(),
    fAddress(0),
    fChannel(0),
    fTime(0),
    fCharge(0)
 { }


  /** Standard constructor
   ** @param  address  Unique element address
   ** @param  time     Measurement time [ns]
   ** @param  charge   Charge [ADC units]
   **/
  CbmStsDigi(Int_t address, Int_t channel, ULong64_t time, UShort_t charge) :
      CbmDigi(),
      fAddress(address),
      fChannel(channel),
      fTime(time),
      fCharge(charge)
  { }


  /** Destructor **/
  virtual ~CbmStsDigi() { };


  /** Unique detector element address  (see CbmStsAddress)
   ** @value Unique address of readout channel
   **/
  virtual Int_t GetAddress() const { return fAddress; }


  /** @brief Channel number in module
   ** @value Channel number
   **/
  UShort_t GetChannel() const { return fChannel; }


  /** Charge
   ** @value Charge [ADC units]
   **/
  virtual Double_t GetCharge() const { return Double_t(fCharge); }


 /** System ID
  ** @value System identifier (enum DetectorId)
  **/
  virtual Int_t GetSystemId() const { return kSts; }


  /** Time of measurement
   ** @value Time [ns]
   **/
  virtual Double_t GetTime() const { return Double_t(fTime); }

  template <class Archive>
  void serialize(Archive& ar, const unsigned int /*version*/)
  {
    ar& fAddress;
    ar& fTime;
    ar& fCharge;
  }


  /** String output **/
  virtual std::string ToString() const;


 private:

#ifndef __CINT__ // for BOOST serialization
  friend class boost::serialization::access;
#endif // for BOOST serialization

  Int_t     fAddress;     ///< Unique element address
  UShort_t  fChannel;     ///< Channel number
  Long64_t  fTime;        ///< Time [ns]
  UShort_t  fCharge;      ///< Charge [ADC units]


  ClassDef(CbmStsDigi, 6);

};


#endif

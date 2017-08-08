/** CbmMuchDigi.h
 **@author M.Ryzhinskiy <m.ryzhinskiy@gsi.de>
 **@since 19.03.07
 **@version 1.0
 **
 **@author Vikas Singhal <vikas@vecc.gov.in>
 **@since 17/05/16
 **@version 2.0
 ** Data class for digital MUCH information
 ** Data level: RAW
 **
 **
 **/


#ifndef CBMMUCHDIGI_H
#define CBMMUCHDIGI_H 1

#include "CbmDigi.h"
#include "CbmDefs.h"
#include "CbmMuchDigiMatch.h"
#include "CbmMuchAddress.h"


#ifndef __CINT__
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#endif //__CINT__


class CbmMuchDigi : public CbmDigi{
 public:
  	/** Default Constructor */
	CbmMuchDigi(): CbmDigi(), fAddress(0), fTime(0), fCharge(0) {}
	
	/** Standard constructor
	** @param  address  Unique element address
	** @param  time     Measurement time [ns]
	** @param  charge   Charge [ADC units]
 	**/
	CbmMuchDigi(UInt_t address, UShort_t charge=0, ULong64_t time=0)
		: CbmDigi(), fAddress(address), fTime(time), fCharge(charge) {}
			//fMatch will be created in the CbmMuchSignal and should be deleted by that class destructor only.
			//fMatch = new CbmMatch();}

	CbmMuchDigi(CbmMuchDigi* digi);
	CbmMuchDigi(CbmMuchDigi* digi,CbmMuchDigiMatch* match);
	CbmMuchDigi(const CbmMuchDigi&);
	CbmMuchDigi& operator=(const CbmMuchDigi&);

  	/** Destructor **/
	virtual ~CbmMuchDigi(){}

	//Getters
	Int_t     GetSystemId() const { return kMuch; }
	UShort_t  GetAdc()      const { return fCharge; }

 	virtual Int_t     GetAddress()  const { return static_cast<Int_t>(fAddress); }
	virtual Double_t  GetTime()     const { return static_cast<Double_t>(fTime); }
  
	// Setters	
	void SetAdc(Int_t adc);
	void SetTime(ULong64_t time);
	void SetSaturation(Bool_t saturate) {fSaturationFlag = saturate;}
	void SetAddress(Int_t address) {fAddress = address;}
	 
	// Specially for littrack
	// TODO remove after littrack fix
	Int_t GetDetectorId() const { return CbmMuchAddress::GetElementAddress(GetAddress(),kMuchModule); }
	Int_t GetChannelId()  const { return GetAddress(); }
	Int_t GetADCCharge()  const { return GetAdc(); }
	Int_t GetDTime() const { return 0; }

	template <class Archive>
  	void serialize(Archive& ar, const unsigned int /*version*/)
  	{
      		ar& fAddress;
      		ar& fTime;
      		ar& fCharge;
      		ar& fSaturationFlag;
  	}
	

 	private:

	#ifndef __CINT__ // for BOOST serialization
  		friend class boost::serialization::access;
	#endif

	UInt_t		fAddress; // Unique detector address
 	ULong64_t	fTime;	  // Absolute Time Stamp[ns]
	UShort_t	fCharge;  // Charge [ADC Units]

	//Below flag has to be set during the CbmMuchDigi Creation only.
	Bool_t 		fSaturationFlag = 0; //If adc value crosses the Maximum Adc value of actual electronics then SaturationFlag will be set.  

	ClassDef(CbmMuchDigi,3);
};
#endif

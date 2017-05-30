/** CbmMuchDigi.cxx
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
#include "CbmMuchDigi.h"
#include "CbmMuchAddress.h"
#include "CbmMuchDigiMatch.h"

#include <memory>

CbmMuchDigi::CbmMuchDigi(CbmMuchDigi* digi)
	: CbmDigi(*digi),
	fAddress(digi->fAddress),
 	fTime(digi->fTime),
	fCharge(digi->fCharge)
{ 
}

CbmMuchDigi::CbmMuchDigi(CbmMuchDigi* digi,CbmMuchDigiMatch* match)
  : CbmDigi(*digi),
	fAddress(digi->fAddress),
	fTime(digi->fTime),
	fCharge(digi->fCharge)
{  
	SetMatch(match);
}


CbmMuchDigi::CbmMuchDigi(const CbmMuchDigi& rhs)
	: CbmDigi(rhs),
	fAddress(rhs.fAddress),
	fTime(rhs.fTime),
	fCharge(rhs.fCharge)
{
}

CbmMuchDigi& CbmMuchDigi::operator=(const CbmMuchDigi& rhs)
{

  if (this != &rhs) {
    CbmDigi::operator=(rhs);
    fAddress = rhs.fAddress;
    fTime = rhs.fTime;
    fCharge = rhs.fCharge;
  }
  return *this;
}

// -----   Set new charge   ------------------------------------------------
void CbmMuchDigi::SetAdc(Int_t adc) {
	//ADC value should not be more than saturation
	fCharge=adc;	
	// if Saturation
	Int_t saturation = (1<<12); //2 ^ 12 - 1;	
	if(fCharge >= saturation){
		fCharge=saturation-1;
		fSaturationFlag=1;
	}
	if(fCharge < 0) fCharge=0;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMuchDigi::SetTime(ULong64_t time) { 
  	fTime = time;
}
// -------------------------------------------------------------------------

ClassImp(CbmMuchDigi)

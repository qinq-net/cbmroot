/**CbmMuchSignal.cxx
 * @class CbmMuchSignal
 **@author Vikas Singhal <vikas@vecc.gov.in>
 **@since 17/05/16
 **@version 1.0
 **@brief Data class for an analog signal in the MUCH
 ** Simple data class used in the digitisation process of the MUCH. It describes
 ** an analog charge distribution produced in the MUCH GEM Detector and arriving at the
 ** readout. It contains time and charge information corresponding to the MUCH Pad.
 ** In the most general case, a signal can be produced by more than one
 ** MCPoint; that is why the MC reference is of type CbmMatch and not CbmLink.
 **
 **
 **
 **/
#include "CbmMuchDigi.h"
#include "CbmMuchAddress.h"
#include "CbmMuchDigiMatch.h"
#include "CbmMuchSignal.h"
#include "TMath.h"
#include "TRandom.h"

#include <memory>

using namespace std;

/*
CbmMuchSignal::CbmMuchSignal(CbmMuchSignal* signal)
	: TObject(),
	fAddress(signal->fAddress),
	fTimeStart(signal->fTimeStart),
	fTimeStop(signal->fTimeStop),
	fSignalShape(signal->fSignalShape)
{
}
*/
CbmMuchSignal::CbmMuchSignal(const CbmMuchSignal& rhs)
	: TObject(),
	fAddress(rhs.fAddress),
	fTimeStart(rhs.fTimeStart),
	fTimeStop(rhs.fTimeStop),
	fSignalShape(rhs.fSignalShape),
	fMatch(rhs.fMatch)
{
}
/*
CbmMuchSignal& CbmMuchSignal::operator=(const CbmMuchSignal& rhs)
{

	if (this != &rhs) {
	TObject::operator=(rhs);
	fAddress = rhs.fAddress;
	fTimeStop = rhs.fTimeStop;
	fTimeStart = rhs.fTimeStart;
	fSignalShape = rhs.fSignalShape;
	fMatch	= rhs.fMatch;
  }
  return *this;
}
*/


void CbmMuchSignal::MakeSignalShape(UInt_t charge, TArrayD shape) {
	Int_t bin0 = 0;
			//Int_t((fT0)/gkResponseBin);
	Int_t nbins = bin0+shape.GetSize();
	if (fSignalShape.GetSize()<nbins) fSignalShape.Set(nbins);
	for (Int_t j=0;j<shape.GetSize();j++)  fSignalShape[bin0 + j]+=charge*shape[j];
}
// -------------------------------------------------------------------------
//
//
// -------------------------------------------------------------------------
//Below function will add the Signal shapes of 2 signal

void CbmMuchSignal::MergeSignal(CbmMuchSignal* signal){
	
	Int_t StartDiff = signal->GetTimeStart()-fTimeStart;
	Int_t StopDiff = signal->GetTimeStop()-fTimeStop;
	//cout << " Stop Difference "<< StopDiff << endl; 
	if(StopDiff<0) StopDiff = 0; //Shows Second signal Stop time is within first signal stop time, no need to increase the SignalShape size.
	
	if(StartDiff<0){
	//	std::cout<<"First Signal start time " <<fTimeStart<<" Second Signal start time is " <<signal->GetTimeStart() << endl;
		StartDiff = (-1)*StartDiff; 
	}

	TArrayD SecondSignalShape = signal->GetSignalShape();
	fSignalShape.Set(fSignalShape.GetSize()+StopDiff);
	//std::cout<<"MergeSignal called and size of fSignalShape "<<fSignalShape.GetSize()<<endl;
	for (Int_t j=0;j<fSignalShape.GetSize()&&j<SecondSignalShape.GetSize();j++)
		  
		fSignalShape[j+StartDiff]=fSignalShape[j+StartDiff]+SecondSignalShape[j];
		//SignalShape will be added in the first signal at location from where second signal start.
}

void CbmMuchSignal::AddNoise(UInt_t meanNoise){
	for (Int_t i=0;i<fSignalShape.GetSize();i++){
		fSignalShape[i]+=TMath::Abs(meanNoise*gRandom->Gaus());
  	}
}


Int_t CbmMuchSignal::GetMaxCharge(){
	Int_t max_charge = -1;
	for (Int_t i=0;i<fSignalShape.GetSize();i++){
		Int_t charge = fSignalShape[i];
		if (charge>max_charge) max_charge = charge;
	}
  return max_charge;
}


Int_t CbmMuchSignal::GetTimeStamp(Int_t threshold){
  //Int_t threshold = 10000;
  Int_t bin1 = -1;
  for (Int_t i=0;i<fSignalShape.GetSize();i++){
    if (bin1<0 && fSignalShape[i]>threshold) {
      bin1 = i;
      return fTimeStart+bin1*gkResponseBin;
    }
  }
  return -1;
}
// -------------------------------------------------------------------------



/* Below functions will not be used as ADC or Charge value will be calculated on the basis of fSignalShape of the Analog Signal.
// -----   Add charge   ----------------------------------------------------
void CbmMuchSignal::AddAdc(Int_t adc) {
  Int_t newAdc = GetAdc() + adc;
  SetAdc(newAdc);
}
// -------------------------------------------------------------------------


// -----   Set new charge   ------------------------------------------------
void CbmMuchSignal::SetAdc(Int_t adc) {
	//Charge value should not be more than saturation
	
	UShort_t saturation = 4095; //2 ^ 12 - 1;	
	if(adc>saturation){
		fCharge=saturation;
		fSaturationFlag=1;
	}
	else if (adc < 0) fCharge=0;
	else	fCharge = (UShort_t)adc;
}
// -------------------------------------------------------------------------
*/

ClassImp(CbmMuchSignal)

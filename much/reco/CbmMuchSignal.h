/**CbmMuchSignal.h
 * @author Vikas Singhal <vikas@vecc.gov.in>
 **@since 10/08/16
 **@version 1.0
 ** Data class for analog MUCH information
 ** Data level: buffered in the CbmMuchReadoutBuffer
 **
 **
 **/


#ifndef CBMMUCHSIGNAL_H
#define CBMMUCHSIGNAL_H 1


#include "TObject.h"
#include "TArrayI.h"
#include "TArrayD.h"
#include "CbmMatch.h"
class TClonesArray;

static const Int_t gkResponseBin	= 1;
static const Double_t fDeadTime		= 400;  // Deadtime

class CbmMuchSignal : public TObject{
 public:
  	/** Default Constructor */
	CbmMuchSignal(): TObject(), fAddress(0), fTimeStart(0), fTimeStop(0), fSignalShape(0){}
	
	/** Standard constructor
	** @param  fTimeStart     Start time [ns]
	** @param  fTimeStop      Stop time [ns] = Start Time + Dead Time
	** @param  charge   Charge [ADC units]
 	**/
	CbmMuchSignal(UInt_t address, ULong64_t starttime=0)
		: TObject(), fAddress(address), fTimeStart(starttime), fSignalShape(0)

		{
			fMatch = new CbmMatch();
			fTimeStop = fTimeStart + fDeadTime;
		}

	CbmMuchSignal(CbmMuchSignal* signal);
	CbmMuchSignal(const CbmMuchSignal&);
	CbmMuchSignal& operator=(const CbmMuchSignal&);

  	/** Destructor **/
	virtual ~CbmMuchSignal(){
		if (fMatch) delete fMatch;
		}


	// Added below functions as CbmMuchSignal will be buffered in the CbmMuchReadoutBuffer.
	// Absolute start time for the CbmMuchSignal. If earlier MCPoint hits the same pad which was hit by later MCPoint and it is within deadtime then earlier MCPoint Time should be start time of this Signal
	Double_t GetTimeStart() const 	{ return fTimeStart; }

	// Absolute stop time for this CbmMuchSignal. If later MCPoint hits on the same pad and it is within deadtime then later MCPoint Time + deadtime should be stop time of this Signal. It shows tha for how much time this pad is active.
	Double_t GetTimeStop() const	{return fTimeStop; }
	TArrayD GetSignalShape() const	{return fSignalShape; }
	CbmMatch* GetMatch() const	{return fMatch;}
	Int_t GetAddress()  const	{return fAddress; }

	//Setters
	void SetTimeStart(Double_t starttime) {fTimeStart = starttime; }
	void SetTimeStop(Double_t stoptime) {fTimeStop = stoptime; }
	void SetSignalShape(TArrayD signalshape) {fSignalShape = signalshape; }
	
	void SetMatch(CbmMatch* match) {fMatch = match;}

	void MakeSignalShape(UInt_t,TArrayD);
	//MergeSignal Function which will implement Signal PileUp
	void MergeSignal(CbmMuchSignal*);
	void AddNoise(UInt_t);

	Int_t GetMaxCharge();
	Int_t GetCharge(){return GetMaxCharge();} //Will return respective ADC value
	Int_t GetTimeStamp(Int_t);
	
 private:
	UInt_t		fAddress;	//Unique detector address
 	ULong64_t	fTimeStart;	// Absolute start Time[ns]
	ULong64_t	fTimeStop; 	//Absolut Time till then this digi is active
	TArrayD 	fSignalShape;	// Array of time bins
					//Making Actual Electronics Signal Shape
	CbmMatch* 	fMatch;

	ClassDef(CbmMuchSignal,1);
};
#endif

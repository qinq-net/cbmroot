#ifndef CBMTRBRAWHIT_H
#define CBMTRBRAWHIT_H

#include <iostream>

using namespace std;

class CbmTrbRawHit
{
public:
	CbmTrbRawHit () :
		fTdcId (0),
		fLeadingChannel (0),
		fLeadingEpoch(0),
		fLeadingCoarseTime(0),
		fLeadingFineTime(0),
		fTrailingChannel(0),
		fTrailingEpoch(0),
		fTrailingCoarseTime(0),
		fTrailingFineTime(0),
		fLeadingFineTimeCalibCorr(0.),
		fTrailingFineTimeCalibCorr(0.)
    {

	}

	CbmTrbRawHit (UShort_t tdc,
		UShort_t leadingChannel, UInt_t leadingEpoch, UShort_t leadingCoarseTime, UShort_t leadingFineTime,
	    UShort_t trailingChannel, UInt_t trailingEpoch, UShort_t trailingCoarseTime, UShort_t trailingFineTime) :
	         fTdcId (tdc),
	         fLeadingChannel(leadingChannel),
	         fLeadingEpoch(leadingEpoch),
	         fLeadingCoarseTime(leadingCoarseTime),
	         fLeadingFineTime(leadingFineTime),
	         fTrailingChannel(trailingChannel),
	         fTrailingEpoch(trailingEpoch),
	         fTrailingCoarseTime(trailingCoarseTime),
	         fTrailingFineTime(trailingFineTime),
	         fLeadingFineTimeCalibCorr(0.),
	         fTrailingFineTimeCalibCorr(0.)
	{

	}

   UShort_t GetTdc() const {return this->fTdcId;}
   UShort_t GetLChannel() const {return this->fLeadingChannel;}
   UInt_t GetLEpoch() const {return this->fLeadingEpoch;}
   UShort_t GetLCTime() const {return this->fLeadingCoarseTime;}
   UShort_t GetLFTime() const {return this->fLeadingFineTime;}
   UShort_t GetTChannel() const {return this->fTrailingChannel;}
   UInt_t GetTEpoch() const {return this->fTrailingEpoch;}
   UShort_t GetTCTime() const {return this->fTrailingCoarseTime;}
   UShort_t GetTFTime () const {return this->fTrailingFineTime;}

   // calibrated fine times
   Double_t GetLeadingTimeCorr() const {return this->fLeadingFineTimeCalibCorr;}
   Double_t GetTrailingTimeCorr () const {return this->fTrailingFineTimeCalibCorr;}
   void SetLeadingTimeCorr (Double_t LFTimeCalibrCorr) {this->fLeadingFineTimeCalibCorr = LFTimeCalibrCorr;}
   void SetTrailingTimeCorr (Double_t TFTimeCalibrCorr){this->fTrailingFineTimeCalibCorr = TFTimeCalibrCorr;}

private:
   UShort_t fTdcId;

   UShort_t fLeadingChannel;
   UInt_t fLeadingEpoch;            // 28 bit
   UShort_t fLeadingCoarseTime;     // 11 bit
   UShort_t fLeadingFineTime;       // 10 bit
   UShort_t fTrailingChannel;
   UInt_t fTrailingEpoch;           // 28 bit
   UShort_t fTrailingCoarseTime;    // 11 bit
   UShort_t fTrailingFineTime;      // 10 bit

   // Corrected times
   Double_t fLeadingFineTimeCalibCorr;
   Double_t fTrailingFineTimeCalibCorr;
};

#endif

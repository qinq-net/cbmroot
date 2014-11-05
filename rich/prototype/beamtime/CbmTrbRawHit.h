#ifndef CBMTRBRAWHIT_H
#define CBMTRBRAWHIT_H

#include <iostream>

using namespace std;

class CbmTrbRawHit
{
public:

	CbmTrbRawHit (UShort_t tdc, UShort_t channel, UChar_t edgeType,
         UInt_t epoch, UShort_t coarseTime, UShort_t fineTime) :
             fTdcId (tdc), fChannel (channel), fEdgeType (edgeType)
	{
      switch (edgeType)
      {
      case 0:  // trailing
         this->fTrailingEpoch = epoch;
         this->fTrailingCoarseTime = coarseTime;
         this->fTrailingFineTime = fineTime;
         break;
      case 1:  // leading
         this->fLeadingEpoch = epoch;
         this->fLeadingCoarseTime = coarseTime;
         this->fLeadingFineTime = fineTime;
         break;
      case 2:
         cout << "CbmTrbRawHit: For initializing both edges' times call another constructor." << endl;
         break;
      default:
         cout << "CbmTrbRawHit: Incorrect EdgeType: 0-Leading, 1-Trailing, 2-Both" <<endl;;
         break;
      }

      fLeadingFineTimeCalibCorr = 0.;
      fTrailingFineTimeCalibCorr = 0.;
   }

   UShort_t GetTdc() const {return this->fTdcId;}
   UShort_t GetChannel() const {return this->fChannel;}
   UChar_t GetEdgeType() const {return this->fEdgeType;}
   UInt_t GetLEpoch() const {return this->fLeadingEpoch;}
   UShort_t GetLCTime() const {return this->fLeadingCoarseTime;}
   UShort_t GetLFTime() const {return this->fLeadingFineTime;}
   UInt_t GetTEpoch() const {return this->fTrailingEpoch;}
   UShort_t GetTCTime() const {return this->fTrailingCoarseTime;}
   UShort_t GetTFTime () const {return this->fTrailingFineTime;}

   // calibrated fine times
   Double_t GetLeadingTimeCorr() const {return this->fLeadingFineTimeCalibCorr;}
   Double_t GetTrailingTimeCorr () const {return this->fTrailingFineTimeCalibCorr;}
   void SetLeadingTimeCorr (Double_t LFTimeCalibrCorr) {this->fLeadingFineTimeCalibCorr = LFTimeCalibrCorr;}
   void SetTrailingTimeCorr (Double_t TFTimeCalibrCorr){this->fTrailingFineTimeCalibCorr = TFTimeCalibrCorr;}

   // return true if leading edge exists
   inline bool IsLeading() const { return (fEdgeType == 1) || (fEdgeType == 2); }
   // return true if trailing edge exists
   inline bool IsTrailing() const { return (fEdgeType == 0) || (fEdgeType == 2); }

private:
   UShort_t fTdcId;
   UShort_t fChannel;

   // Flag:
   // 3 - not initialized
   // 2 - both leading and trailing
   // 1 - only leading
   // 0 - only trailing
   // Done so unusual to be the same 0/1 as 0/1 of 'isrising' in TTrbProc::ProcessTDCV3
   UChar_t fEdgeType;

   // Data
   UInt_t fLeadingEpoch;            // 28 bit
   UShort_t fLeadingCoarseTime;     // 11 bit
   UShort_t fLeadingFineTime;       // 10 bit
   UInt_t fTrailingEpoch;           // 28 bit
   UShort_t fTrailingCoarseTime;    // 11 bit
   UShort_t fTrailingFineTime;      // 10 bit

   // Corrected times
   Double_t fLeadingFineTimeCalibCorr;
   Double_t fTrailingFineTimeCalibCorr;
};

#endif

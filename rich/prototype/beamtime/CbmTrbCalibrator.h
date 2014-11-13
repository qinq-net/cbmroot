#ifndef CBMTRBCALIBRATOR_H
#define CBMTRBCALIBRATOR_H

#include "TObject.h"
#include "TH1.h"
#include "TROOT.h"

#include "CbmRichTrbDefines.h"

class CbmTrbCalibrator : public TObject
{
public: // methods
   /*
    * Destructor
    */
   ~CbmTrbCalibrator();

   /*
    * Singleton class object returner
    */
   static CbmTrbCalibrator* Instance();

   /*
    * Add raw fine time received from TDC and unpacked by the CbmRichTrbUnpack
    */
   void AddFineTime(UShort_t inTRBid, UShort_t inTDCid, UShort_t inCHid, UShort_t leadingFT, UShort_t trailingFT);
   
   /*
    * Return time in ns.
    * Fine time counter is 10 bits => UShort_t is enough.
    */
   Double_t GetFineTimeCalibrated(UShort_t TRB, UShort_t TDC, UShort_t CH, UShort_t fineCnt);
   
   /*
    * Monkey code here. Call this method at each event in the Unpacker after
    * (but don't forget to call AddFineTime for each fine time to be taken into account).
    * This counts the number of events to determine when to start DoCalibrate.
    */
   void NextRawEvent();
   
   /*
    * Per se calibration of all the channels of all TDCs of all TRBs.
    */
   void DoCalibrate();

   void GenHistos();

private: // methods

   /*
    * Constructor - private because the class is singleton
    */
   CbmTrbCalibrator();

   /*
    * Pseudo-calibration - used during debug.
    * Return time in ns.
    * Fine time counter is 10 bits => UShort_t is enough.
    */
   Double_t GetLinearFineCalibration(UShort_t fineCnt);

   /*
    * Return really calibrated fine time. DoCalibrate() should have been called before.
    * If NextRawEvent() had been called for each event and calibrationPeriod is correctly defined
    * then the object of this class automatically handles calibration.
    * Return time in ns.
    * Fine time counter is 10 bits => UShort_t is enough.
    */
   Double_t GetRealFineCalibration(UShort_t TRB, UShort_t TDC, UShort_t CH, UShort_t fineCnt);

private: // data members

   /*
    * If true then calibration will be executed after having
    * calibrationPeriod events in the buffer.
    */
   Bool_t toDoCalibration;
   
   /*
    * Minimum number of events to start calibration.
    */
   UInt_t calibrationPeriod;

   /*
    * Flushed after calibration.
    */
   TH1I* hLeadingFine[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
   
   /*
    * Flushed after calibration.
    */
   TH1I* hTrailingFine[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
   
   /*
    * Accumulates across all the received data.
    */
   TH1I* hLeadingFineBuffer[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
   
   /*
    * Accumulates across all the received data.
    */
   TH1I* hTrailingFineBuffer[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];

   /*
    * Renewes at calibration.
    * Initialized with `identity` (linear =1 function) before the first calibration.
    */
	TH1D* hCalcBinWidth[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
	
	/*
    * Renewes at calibration.
    * Initialized with `identity` (linear y=k*x function) before the first calibration.
    */
	TH1D* hCalBinTime[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];

   /*
    * Counter of events taken into account for calibration.
    * Used to determine when to start DoCalibrate.
    */
   static UInt_t fEventCounter;

   /*
    * Pointer to the singleton class object.
    */
   static CbmTrbCalibrator* fInstance;


   ClassDef(CbmTrbCalibrator,1)
};

#endif // CBMTRBCALIBRATOR_H

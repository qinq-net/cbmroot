#ifndef CBMTRBCALIBRATOR_H
#define CBMTRBCALIBRATOR_H

#include "TObject.h"
#include "TFolder.h"
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

   void GenHistos();

   /*
    * Singleton class object returner.
    */
   static CbmTrbCalibrator* Instance();

   /*
    * Set the period of calibration.
    */
   void SetCalibrationPeriod(UInt_t period) { fCalibrationPeriod = period; }

   /*
    * Get the period of calibration.
    */
   //UInt_t GetCalibrationPeriod() { return fCalibrationPeriod; }

   /*
    * Add raw fine time received from TDC and unpacked by the CbmRichTrbUnpack.
    */
   void AddFineTime(UInt_t inTRBid, UShort_t inTDCid, UShort_t inCHid, UShort_t fineTime);
   
   /*
    * Return time in ns.
    * Fine time counter is 10 bits => UShort_t is enough.
    */
   Double_t GetFineTimeCalibrated(UShort_t TRB, UShort_t TDC, UShort_t CH, UShort_t fineCnt);
   
   /*
    * Per se calibration of a certain channel of a certain TDC of a certain TRB.
    */
   void DoCalibrate(UShort_t TRB, UShort_t TDC, UShort_t CH);

   /*
    * Export the calibration information into the root file.
    */
   void Export(const char* filename = "calibration.root");

   /*
    * Import the calibration information from the root file.
    */
   void Import(const char* filename = "calibration.root");

   /*
    * Draw a canvas with the flags indicating whilch channels have been calibrated.
    */
   void Draw();

private: // methods

   /*
    * Constructor - private because the class is singleton.
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
    * If true then calibration of channel CH will be executed after having
    * calibrationPeriod entries in the buffer for channel CH.
    */
   Bool_t fToDoCalibration;
   
   /*
    * Minimum number of fine time counters taken into account to start calibration.
    */
   UInt_t fCalibrationPeriod;

   /*
    * Root folder for all the calibration data.
    */
   TFolder* fTRBroot;

   /*
    * Calibration has already been done - use calibation table,
    * Otherwise - use linear claibration.
    */
   TH1C* fCalibrationDoneHisto[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC];

   /*
    * Flushed after calibration.
    */
   TH1D* fhLeadingFineBuffer[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
   
   /*
    * Accumulates across all the received data.
    */
   TH1D* fhLeadingFine[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];

   /*
    * Renewes at calibration.
    * Initialized with `identity` (linear =1 function) before the first calibration.
    */
	TH1D* fhCalcBinWidth[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
	
	/*
    * Renewes at calibration.
    * Initialized with `identity` (linear y=k*x function) before the first calibration.
    */
	TH1D* fhCalBinTime[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
   
   /*
    * For each channel - counter of fine time counters taken into accout for calibration.
    */
   UInt_t fFTcounter[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];

   /*
    * Pointer to the singleton class object.
    */
   static CbmTrbCalibrator* fInstance;

   ClassDef(CbmTrbCalibrator,1)
};

#endif // CBMTRBCALIBRATOR_H

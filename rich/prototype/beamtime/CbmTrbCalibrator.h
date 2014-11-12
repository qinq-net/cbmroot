#ifndef CBMTRBCALIBRATOR_H
#define CBMTRBCALIBRATOR_H

#include "TObject.h"
#include "TH1I.h"
#include "CbmRichTrbDefines.h"

#include <stdint.h>

class CbmTrbCalibrator : public TObject
{
public:
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
   void AddFineTime(uint32_t inTRBid, uint32_t inTDCid, uint32_t inCHid, uint32_t leadingFT, uint32_t trailingFT);
   
   /*
    * Return time in ns.
    * Fine time counter is 10 bits => UShort_t is enough.
    */
   Double_t GetFineTimeCalibrated(UShort_t fineCnt);

   void GenHistos();
   
private:

   /*
    * Constructor - private because the class is singleton
    */
   CbmTrbCalibrator();

   Bool_t toDoGeneralCalibration;
   Bool_t toDoContinuousCalibration;
   UInt_t calibrationPeriod;

   TH1I* hLeadingFine[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
   TH1I* hLeadingFineBuffer[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
   TH1I* hTrailingFine[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];
   TH1I* hTrailingFineBuffer[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC][TRB_TDC3_CHANNELS];

   /*
    * Return time in ns.
    * Fine time counter is 10 bits => UShort_t is enough.
    */
   Double_t GetLinearFineCalibration(UShort_t fineCnt);

   static CbmTrbCalibrator* fInstance;

   ClassDef(CbmTrbCalibrator,1)
};

#endif // CBMTRBCALIBRATOR_H

#ifndef CBMTRBCALIBRATOR_H
#define CBMTRBCALIBRATOR_H

#include "TObject.h"

class CbmRawSubEvent;

class CbmTrbCalibrator : public TObject
{
private:
   static CbmTrbCalibrator* fInstance;

public:
   CbmTrbCalibrator();
   ~CbmTrbCalibrator();

   static CbmTrbCalibrator* Instance();

   void AddRawSubEvent(CbmRawSubEvent* rawsubEv);

   Double_t GetFineTimeCalibrated(UShort_t fineTimeCounter);

   ClassDef(CbmTrbCalibrator, 1)
};

#endif // CBMTRBCALIBRATOR_H

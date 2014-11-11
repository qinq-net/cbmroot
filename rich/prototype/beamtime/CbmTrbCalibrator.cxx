#include "CbmTrbCalibrator.h"

#include "CbmRawSubEvent.h"

CbmTrbCalibrator* CbmTrbCalibrator::fInstance = 0;

CbmTrbCalibrator::CbmTrbCalibrator()
{
}

CbmTrbCalibrator::~CbmTrbCalibrator()
{
}

CbmTrbCalibrator* CbmTrbCalibrator::Instance()
{
   if (!fInstance) fInstance = new CbmTrbCalibrator();
   return fInstance;
}

//TOD implement!!!
void CbmTrbCalibrator::AddRawSubEvent(CbmRawSubEvent* rawsubEv)
{
}

//TODO implement!!!
Double_t GetFineTimeCalibrated(UShort_t fineTimeCounter)
{
   return 0;
}

ClassImp(CbmTrbCalibrator)

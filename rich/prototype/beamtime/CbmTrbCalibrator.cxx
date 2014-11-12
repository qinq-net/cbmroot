#include "CbmTrbCalibrator.h"

#include <iostream>

CbmTrbCalibrator* CbmTrbCalibrator::fInstance = 0;

CbmTrbCalibrator::CbmTrbCalibrator():
   toDoGeneralCalibration(true),
   toDoContinuousCalibration(false),
   calibrationPeriod(10000)
{
    GenHistos();
}

CbmTrbCalibrator::~CbmTrbCalibrator()
{
}

CbmTrbCalibrator* CbmTrbCalibrator::Instance()
{
   if (!fInstance) fInstance = new CbmTrbCalibrator();
   return fInstance;
}

void CbmTrbCalibrator::AddFineTime(uint32_t inTRBid, uint32_t inTDCid, uint32_t inCHid, uint32_t leadingFT, uint32_t trailingFT)
{

   uint32_t trb_index = (inTRBid >> 4) & 0x00FF;
   uint32_t tdc_index = (inTDCid & 0x000F);


   std::cout << trb_index << " " << tdc_index << " "  << inCHid << std::endl;
   

   hLeadingFine[trb_index][tdc_index][inCHid]->Fill(leadingFT);
   hTrailingFine[trb_index][tdc_index][inCHid]->Fill(trailingFT);


}

Double_t CbmTrbCalibrator::GetFineTimeCalibrated(UShort_t fineCnt)
{
   return GetLinearFineCalibration(fineCnt);
}

Double_t CbmTrbCalibrator::GetLinearFineCalibration(UShort_t fineCnt)
{
   return (fineCnt<20) ? 0. : ((fineCnt>500) ? 5. : (fineCnt-20)/480.*5.);
}

// Currently two basic types of hostigrams here needed for fine time calibaration -
// Leading and trailing fine time distributions.
void CbmTrbCalibrator::GenHistos()
{
   TString obname;
   TString obtitle;
   TString dirname;

   UInt_t tbins = TRB_TDC3_FINEBINS;
   Int_t trange = TRB_TDC3_FINEBINS;

   for (Int_t b=0; b<TRB_TDC3_NUMBOARDS; ++b)
   {
      for (Int_t t=0; t<TRB_TDC3_NUMTDC; ++t)
      {
         for (Int_t i=0; i<TRB_TDC3_CHANNELS; ++i)
         {
            dirname.Form("TRB/TRB%02d_TDC%02d", b, t);


            obname.Form("%s/Chan%02d/LeadingFineTime_%02d_%02d_%02d", dirname.Data(), i, b, t, i);
            obtitle.Form("Fine time Leading edge TRB %02d TDC %02d Channel %02d", b, t, i);
            hLeadingFine[b][t][i] = new TH1I(obname.Data(), obtitle.Data(), tbins, 0, trange);

            obname.Form("%s/Chan%02d/LeadingFineTimeBuffer_%02d_%02d_%02d", dirname.Data(), i, b, t, i);
            obtitle.Form("Fine time Leading edge TRB %02d TDC %02d Channel %02d (calibration buffer)", b, t, i);
            hLeadingFineBuffer[b][t][i] = new TH1I(obname.Data(), obtitle.Data(), tbins, 0, trange);


            obname.Form("%s/Chan%02d/TrailingFineTime_%02d_%02d_%02d", dirname.Data(), i, b, t, i);
            obtitle.Form("Fine time Trailing edge TRB %02d TDC %02d Channel %02d", b, t, i);
            hTrailingFine[b][t][i] = new TH1I(obname.Data(), obtitle.Data(), tbins, 0, trange);

            obname.Form("%s/Chan%02d/TrailingFineTimeBuffer_%02d_%02d_%02d", dirname.Data(), i, b, t, i);
            obtitle.Form("Fine time Trailing edge TRB %02d TDC %02d Channel %02d (calibration buffer)", b, t, i);
            hTrailingFineBuffer[b][t][i] = new TH1I(obname.Data(), obtitle.Data(), tbins, 0, trange);

         }
      }
   }

}

ClassImp(CbmTrbCalibrator)

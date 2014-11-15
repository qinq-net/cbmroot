#include "CbmTrbCalibrator.h"

#include "TFolder.h"
#include "FairLogger.h"

CbmTrbCalibrator* CbmTrbCalibrator::fInstance = 0;
UInt_t CbmTrbCalibrator::fEventCounter = 0;

CbmTrbCalibrator::CbmTrbCalibrator():
   toDoCalibration(true),
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

void CbmTrbCalibrator::AddFineTime(UShort_t inTRBid, UShort_t inTDCid, UShort_t inCHid, UShort_t leadingFT, UShort_t trailingFT)
{
   UShort_t trb_index = (inTRBid >> 4) & 0x00FF;
   UShort_t tdc_index = (inTDCid & 0x000F);
   LOG(DEBUG2) << trb_index << " " << tdc_index << " "  << inCHid << FairLogger::endl;

   hLeadingFine[trb_index-1][tdc_index][inCHid]->Fill(leadingFT);
   hTrailingFine[trb_index-1][tdc_index][inCHid]->Fill(trailingFT);
   hLeadingFineBuffer[trb_index-1][tdc_index][inCHid]->Fill(leadingFT);
   hTrailingFineBuffer[trb_index-1][tdc_index][inCHid]->Fill(trailingFT);
}

void CbmTrbCalibrator::NextRawEvent()
{
   fEventCounter++;
   
   if (fEventCounter >= calibrationPeriod && toDoCalibration) {
      DoCalibrate();
      fEventCounter = 0;
   }
}

void CbmTrbCalibrator::DoCalibrate()
{
   LOG(DEBUG) << "CALIBRATING" << FairLogger::endl;

   TH1* hfinecal = 0;
   UInt_t sum;
   Double_t binwidth;
   Double_t binhits;

   for (Int_t board=0; board<TRB_TDC3_NUMBOARDS; ++board) {
      for (Int_t tdc=0; tdc<TRB_TDC3_NUMTDC; ++tdc) {
         for (Int_t ch=0; ch<TRB_TDC3_CHANNELS; ++ch) {
         
            hfinecal = hLeadingFine[board][tdc][ch];
            sum = hfinecal->GetEntries();
            
            for (Int_t fb=0; fb<TRB_TDC3_FINEBINS; ++fb)
            {
               binwidth = 1.;
               binhits = hfinecal->GetBinContent(fb + 1);
               if (sum)
                  binwidth = (TRB_TDC3_COARSEUNIT * binhits) / (Double_t)sum;
               hCalcBinWidth[board][tdc][ch]->SetBinContent(fb + 1, binwidth);
               
               Double_t calbintime = 1.;
               if (fb == 0)
                  calbintime = hCalcBinWidth[board][tdc][ch]->GetBinContent(fb + 1) / 2;
               else
                  calbintime = hCalBinTime[board][tdc][ch]->GetBinContent(fb) +
                              (hCalcBinWidth[board][tdc][ch]->GetBinContent(fb) + hCalcBinWidth[board][tdc][ch]->GetBinContent(fb + 1)) / 2;

               hCalBinTime[board][tdc][ch]->SetBinContent(fb + 1, calbintime);
            }

            hLeadingFine[board][tdc][ch]->Reset();
            hTrailingFine[board][tdc][ch]->Reset();
         }
      }
   }
   
   toDoCalibration = false;
   
}

Double_t CbmTrbCalibrator::GetFineTimeCalibrated(UShort_t TRB, UShort_t TDC, UShort_t CH, UShort_t fineCnt)
{
  // return GetRealFineCalibration(TRB, TDC, CH, fineCnt);
   return GetLinearFineCalibration(fineCnt);
}

Double_t CbmTrbCalibrator::GetLinearFineCalibration(UShort_t fineCnt)
{
   return (fineCnt<20) ? 0. : ((fineCnt>500) ? 5. : (fineCnt-20)/480.*5.);
}

Double_t CbmTrbCalibrator::GetRealFineCalibration(UShort_t TRB, UShort_t TDC, UShort_t CH, UShort_t fineCnt)
{
   return hCalBinTime[TRB-1][TDC][CH]->GetBinContent(fineCnt+1); //TODO check
}

void CbmTrbCalibrator::GenHistos()
{
   TString obname;
   TString obtitle;
   TString dirname;

   UInt_t tbins = TRB_TDC3_FINEBINS;
   Int_t trange = TRB_TDC3_FINEBINS;

   TFolder* TRBroot = gROOT->GetRootFolder()->AddFolder("TRB", "TRB root folder");
   gROOT->GetListOfBrowsables()->Add(TRBroot, "TRB");

   TFolder* TRBfolders[TRB_TDC3_NUMBOARDS];
   TFolder* TDCfolders[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC];

   // TRBs are numerated startgin from 1, but the arrays of histos are from 0 (as in any C++ code)
   // TDCs are numerated from 0. One TRB has TDCs {0,1,2,3}.
   // Channels are numerated from 0 and there are total 33 channels for each TDC. 0-th channel is the reference channel.

   for (Int_t b=0; b<TRB_TDC3_NUMBOARDS; ++b) {
   
      obname.Form("TRB%02d", b+1);
      obtitle.Form("TRB%02d", b+1);
      TRBfolders[b] = TRBroot->AddFolder(obname, obtitle);
      
      for (Int_t t=0; t<TRB_TDC3_NUMTDC; ++t) {
      
         obname.Form("TDC%d", t);
         obtitle.Form("TDC%d", t);
         TDCfolders[b][t] = TRBfolders[b]->AddFolder(obname, obtitle);
      
         for (Int_t i=0; i<TRB_TDC3_CHANNELS; ++i) {

            obname.Form("LeadingFineTime_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Fine time Leading edge TRB %02d TDC %02d Channel %02d", b+1, t, i);
            hLeadingFine[b][t][i] = new TH1I(obname.Data(), obtitle.Data(), tbins, 0, trange);

            obname.Form("TrailingFineTime_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Fine time Trailing edge TRB %02d TDC %d Channel %02d", b+1, t, i);
            hTrailingFine[b][t][i] = new TH1I(obname.Data(), obtitle.Data(), tbins, 0, trange);

            obname.Form("LeadingFineTimeBuffer_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Fine time Leading edge TRB %d TDC %d Channel %02d (calibration buffer)", b+1, t, i);
            hLeadingFineBuffer[b][t][i] = new TH1I(obname.Data(), obtitle.Data(), tbins, 0, trange);

            obname.Form("TrailingFineTimeBuffer_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Fine time Trailing edge TRB %02d TDC %d Channel %02d (calibration buffer)", b+1, t, i);
            hTrailingFineBuffer[b][t][i] = new TH1I(obname.Data(), obtitle.Data(), tbins, 0, trange);

            TDCfolders[b][t]->Add(hLeadingFine[b][t][i]);
            TDCfolders[b][t]->Add(hTrailingFine[b][t][i]);
            TDCfolders[b][t]->Add(hLeadingFineBuffer[b][t][i]);
            TDCfolders[b][t]->Add(hTrailingFineBuffer[b][t][i]);

            obname.Form("BinWidth_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Binwidth Histogram of TRB %02d TDC %d Channel %02d", b+1, t, i);
            hCalcBinWidth[b][t][i] = new TH1D(obname.Data(), obtitle.Data(), TRB_TDC3_FINEBINS, 0, TRB_TDC3_FINEBINS);

            //! Reset time bin correction arrays
            for (int fb=0; fb<TRB_TDC3_FINEBINS; ++fb)
               hCalcBinWidth[b][t][i]->SetBinContent(fb + 1, 1.0);

            obname.Form("CalTableBinTime_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Calibration Table of TRB %02d TDC %d Channel %02d", b+1, t, i);
            hCalBinTime[b][t][i] = new TH1D(obname.Data(), obtitle.Data(), TRB_TDC3_FINEBINS, 0, TRB_TDC3_FINEBINS);
            
            //! Reset time bin correction arrays
            for (int fb=0; fb<TRB_TDC3_FINEBINS; ++fb)
               hCalBinTime[b][t][i]->SetBinContent(fb + 1, (Double_t)(TRB_TDC3_COARSEUNIT * fb) / (Double_t)TRB_TDC3_FINEBINS);

            TDCfolders[b][t]->Add(hCalcBinWidth[b][t][i]);
            TDCfolders[b][t]->Add(hCalBinTime[b][t][i]);

         } // Channels
      } // TDCs
   } // TRBs

} // end of method

ClassImp(CbmTrbCalibrator)

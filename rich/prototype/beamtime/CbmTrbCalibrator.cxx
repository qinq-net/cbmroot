#include "CbmTrbCalibrator.h"

#include "FairLogger.h"
#include "TFile.h"

CbmTrbCalibrator* CbmTrbCalibrator::fInstance = 0;
//UInt_t CbmTrbCalibrator::fEventCounter = 0;

CbmTrbCalibrator::CbmTrbCalibrator():
   fToDoCalibration(true),
   fCalibrationPeriod(10000)
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

// inTRBid - raw id, looks like 0x8025    TRB - from 0 to 16 incl.
// inTDCid - raw id, looks like 0x****    TDC - from 0 to 3 incl.
// inCHid - raw id of the channel - the same as CH - number of the channel from 0 to 32 incl.
void CbmTrbCalibrator::AddFineTime(UShort_t inTRBid, UShort_t inTDCid, UShort_t inCHid, UShort_t fineTime)
{
   UShort_t TRB = (inTRBid >> 4) & 0x00FF - 1;
   UShort_t TDC = (inTDCid & 0x000F);
   UShort_t CH = inCHid;
   LOG(DEBUG3) << "AddFineTime: TRB" << TRB << " TDC" << TDC << " CH" << CH << FairLogger::endl;

   fhLeadingFineBuffer[TRB][TDC][CH]->Fill(fineTime);
   fhLeadingFine[TRB][TDC][CH]->Fill(fineTime);
/*
   fhTrailingFineBuffer[TRB-1][TDC][CH]->Fill(trailingFT);
   fhTrailingFine[TRB-1][TDC][CH]->Fill(trailingFT);
*/

   fFTcounter[TRB][TDC][CH]++;
   
   if (fFTcounter[TRB][TDC][CH] >= fCalibrationPeriod
      && fToDoCalibration
      && fCalibrationDoneHisto[TRB][TDC]->GetBinContent(CH+1) == 0)
   {
      DoCalibrate(TRB, TDC, CH);
      fFTcounter[TRB][TDC][CH] = 0;
   }
}

/*
void CbmTrbCalibrator::NextRawEvent()
{
   fEventCounter++;
   
   if (fEventCounter >= fCalibrationPeriod && fToDoCalibration) {
      DoCalibrate();
      fEventCounter = 0;
   }
}
*/

void CbmTrbCalibrator::Save(const char* filename)
{
   TDirectory* current = gDirectory;
   TFile* old = gFile;
   TFile* file = new TFile(filename, "RECREATE");
   
   if (file == NULL || !(file->IsOpen()))
   {
      // error
   } else {
      file->cd();
      fTRBroot->Write();
      file->Close();
      current->cd();   
   }
   
   delete file;
   
   gFile = old;
   gDirectory = current;
}

void CbmTrbCalibrator::DoCalibrate(UShort_t TRB, UShort_t TDC, UShort_t CH)
{
   LOG(DEBUG) << "DoCalibrate: TRB"  << TRB << " TDC" << TDC << " CH" << CH << FairLogger::endl;

   TH1* hfinecal = 0;
   UInt_t sum;
   Double_t binwidth;
   Double_t binhits;
         
   hfinecal = fhLeadingFineBuffer[TRB][TDC][CH];
   sum = hfinecal->GetEntries();
   
   for (Int_t fb=0; fb<TRB_TDC3_FINEBINS; ++fb)
   {
      binwidth = 1.;
      binhits = hfinecal->GetBinContent(fb + 1);
      if (sum)
         binwidth = (TRB_TDC3_COARSEUNIT * binhits) / (Double_t)sum;
      fhCalcBinWidth[TRB][TDC][CH]->SetBinContent(fb + 1, binwidth);
      
      Double_t calbintime = 1.;
      if (fb == 0)
         calbintime = fhCalcBinWidth[TRB][TDC][CH]->GetBinContent(fb + 1) / 2;
      else
         calbintime = fhCalBinTime[TRB][TDC][CH]->GetBinContent(fb) +
                     (fhCalcBinWidth[TRB][TDC][CH]->GetBinContent(fb) + fhCalcBinWidth[TRB][TDC][CH]->GetBinContent(fb + 1)) / 2;

      fhCalBinTime[TRB][TDC][CH]->SetBinContent(fb + 1, calbintime);
   }

   fhLeadingFineBuffer[TRB][TDC][CH]->Reset();
   //fhTrailingFineBuffer[TRB][TDC][CH]->Reset();

   fCalibrationDoneHisto[TRB][TDC]->SetBinContent(CH+1, 1);

}

Double_t CbmTrbCalibrator::GetFineTimeCalibrated(UShort_t TRB, UShort_t TDC, UShort_t CH, UShort_t fineCnt)
{
   if (fCalibrationDoneHisto[TRB][TDC]->GetBinContent(CH+1) == 1)
      return GetRealFineCalibration(TRB, TDC, CH, fineCnt);
   else
      return GetLinearFineCalibration(fineCnt);
}

Double_t CbmTrbCalibrator::GetLinearFineCalibration(UShort_t fineCnt)
{
   return (fineCnt<20) ? 0. : ((fineCnt>500) ? 5. : (fineCnt-20)/480.*5.);
}

Double_t CbmTrbCalibrator::GetRealFineCalibration(UShort_t TRB, UShort_t TDC, UShort_t CH, UShort_t fineCnt)
{
   return fhCalBinTime[TRB][TDC][CH]->GetBinContent(fineCnt+1); //TODO check
}

void CbmTrbCalibrator::GenHistos()
{
   TString obname;
   TString obtitle;
   TString dirname;

   UInt_t tbins = TRB_TDC3_FINEBINS;
   Int_t trange = TRB_TDC3_FINEBINS;

   fTRBroot = gROOT->GetRootFolder()->AddFolder("TRB", "TRB root folder");
   gROOT->GetListOfBrowsables()->Add(fTRBroot, "TRB");

   TFolder* TRBfolders[TRB_TDC3_NUMBOARDS];
   TFolder* TDCfolders[TRB_TDC3_NUMBOARDS][TRB_TDC3_NUMTDC];

   // TRBs are numerated startgin from 1, but the arrays of histos are from 0 (as in any C++ code)
   // TDCs are numerated from 0. One TRB has TDCs {0,1,2,3}.
   // Channels are numerated from 0 and there are total 33 channels for each TDC. 0-th channel is the reference channel.

   for (Int_t b=0; b<TRB_TDC3_NUMBOARDS; ++b) {
   
      obname.Form("TRB%02d", b+1);
      obtitle.Form("TRB%02d", b+1);
      TRBfolders[b] = fTRBroot->AddFolder(obname, obtitle);
      
      for (Int_t t=0; t<TRB_TDC3_NUMTDC; ++t) {
      
         obname.Form("TDC%d", t);
         obtitle.Form("TDC%d", t);
         TDCfolders[b][t] = TRBfolders[b]->AddFolder(obname, obtitle);
      
         for (Int_t i=0; i<TRB_TDC3_CHANNELS; ++i) {

            obname.Form("FineTimeBuffer_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Fine time Leading edge TRB %d TDC %d Channel %02d (calibration buffer)", b+1, t, i);
            fhLeadingFineBuffer[b][t][i] = new TH1D(obname.Data(), obtitle.Data(), tbins, 0, trange);

            obname.Form("FineTime_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Fine time Leading edge TRB %02d TDC %02d Channel %02d", b+1, t, i);
            fhLeadingFine[b][t][i] = new TH1D(obname.Data(), obtitle.Data(), tbins, 0, trange);

            TDCfolders[b][t]->Add(fhLeadingFineBuffer[b][t][i]);
            TDCfolders[b][t]->Add(fhLeadingFine[b][t][i]);
/*
            obname.Form("TrailingFineTimeBuffer_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Fine time Trailing edge TRB %02d TDC %d Channel %02d (calibration buffer)", b+1, t, i);
            hTrailingFineBuffer[b][t][i] = new TH1D(obname.Data(), obtitle.Data(), tbins, 0, trange);

            obname.Form("TrailingFineTime_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Fine time Trailing edge TRB %02d TDC %d Channel %02d", b+1, t, i);
            hTrailingFine[b][t][i] = new TH1D(obname.Data(), obtitle.Data(), tbins, 0, trange);

            TDCfolders[b][t]->Add(hTrailingFineBuffer[b][t][i]);
            TDCfolders[b][t]->Add(hTrailingFine[b][t][i]);
*/
            obname.Form("BinWidth_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Binwidth Histogram of TRB %02d TDC %d Channel %02d", b+1, t, i);
            fhCalcBinWidth[b][t][i] = new TH1D(obname.Data(), obtitle.Data(), TRB_TDC3_FINEBINS, 0, TRB_TDC3_FINEBINS);

            //! Reset time bin correction arrays
            for (int fb=0; fb<TRB_TDC3_FINEBINS; ++fb)
               fhCalcBinWidth[b][t][i]->SetBinContent(fb + 1, 1.0);

            obname.Form("CalTableBinTime_%02d_%d_%02d", b+1, t, i);
            obtitle.Form("Calibration Table of TRB %02d TDC %d Channel %02d", b+1, t, i);
            fhCalBinTime[b][t][i] = new TH1D(obname.Data(), obtitle.Data(), TRB_TDC3_FINEBINS, 0, TRB_TDC3_FINEBINS);
            
            //! Reset time bin correction arrays
            for (int fb=0; fb<TRB_TDC3_FINEBINS; ++fb)
               fhCalBinTime[b][t][i]->SetBinContent(fb + 1, (Double_t)(TRB_TDC3_COARSEUNIT * fb) / (Double_t)TRB_TDC3_FINEBINS);

            TDCfolders[b][t]->Add(fhCalcBinWidth[b][t][i]);
            TDCfolders[b][t]->Add(fhCalBinTime[b][t][i]);

         } // Channels
         
         obname.Form("CalibrationDone_%02d_%d", b+1, t);
         obtitle.Form("Flag indicating that calibration is done for TRB %02d TDC %d", b+1, t);
         fCalibrationDoneHisto[b][t] = new TH1C(obname.Data(), obtitle.Data(), TRB_TDC3_CHANNELS, 0, TRB_TDC3_CHANNELS);
         
         TDCfolders[b][t]->Add(fCalibrationDoneHisto[b][t]);
         
      } // TDCs
   } // TRBs

} // end of method

ClassImp(CbmTrbCalibrator)

/*
 * CbmTrdDigiAnalysis.cxx
 *
 *  Created on: Apr 22, 2017
 *      Author: philipp
 */

#include <CbmTrdDigiAnalysis.h>


CbmTrdDigiAnalysis::CbmTrdDigiAnalysis ()
    : CbmTrdQABase ("CbmTrdDigiAnalysis")
{
  // TODO Auto-generated constructor stub

}

CbmTrdDigiAnalysis::~CbmTrdDigiAnalysis ()
{
  // TODO Auto-generated destructor stub
}

void
CbmTrdDigiAnalysis::CreateHistograms ()
{
  for (Int_t Layer = 0; Layer <fBT->GetNrLayers (); ++Layer)
    {
      for (Int_t Row = 0; Row < fBT->GetNrRows(Layer) ; ++Row)
        {
          for (Int_t Col = 0; Col < fBT->GetNrColumns(Layer); Col++)
            {
              TString HistName = "Signalshape_Layer" + std::to_string(Layer) + "Row"+std::to_string(Row)+"Collumn"
                  + std::to_string (Col);
              fHm->Create2<TH2I> (HistName.Data (), HistName.Data (), 32, -0.5,
                                  31.5, 512 + 256, -255.5, 511.5);
              fHm->H2 (HistName.Data ())->GetYaxis ()->SetTitle (
                  "0-suppressed ADC-Value");
              fHm->H2 (HistName.Data ())->GetXaxis ()->SetTitle ("Timebin");
            }
        }
    }
}

void
CbmTrdDigiAnalysis::Exec (Option_t*)
{
  const Int_t NrDigis = fDigis->GetEntriesFast ();
  LOG(INFO) << this->GetName () << ": Analyzing Signalshape of " << NrDigis
               << " Digis " << FairLogger::endl;
  for (UInt_t Index = 0; Index < NrDigis; Index++)
    {
      CbmTrdDigi* Digi = static_cast<CbmTrdDigi*> (fDigis->At (Index));
      Int_t Address=Digi->GetAddress();
      Int_t Layer=CbmTrdAddress::GetLayerId(Address);
      Int_t Row=CbmTrdAddress::GetRowId(Address);
      Int_t Col=CbmTrdAddress::GetColumnId(Address);
      TString HistName = "Signalshape_Layer" + std::to_string(Layer) + "Row"+std::to_string(Row)+"Collumn"
          + std::to_string (Col);

      TH2* Hist=fHm->H2(HistName.Data());
      for(Int_t Sample=0;Sample<32;Sample++)
        Hist->Fill(Sample,Digi->GetSamples()[Sample]);
    }
  LOG(DEBUG) << this->GetName () << ": Done Analyzing Signalshape of " << NrDigis
               << " Digis " << FairLogger::endl;

}

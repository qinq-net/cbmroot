#include <numeric>
#include "CbmTrdSimpleDigitizer.h"
#include "CbmTrdDigi.h"
#include "CbmTrdAddress.h"

CbmTrdSimpleDigitizer::CbmTrdSimpleDigitizer ()
    : CbmTrdQABase ("CbmTrdSimpleDigitizer"), fDigis(nullptr)
{
}
;

InitStatus
CbmTrdSimpleDigitizer::Init ()
{
  LOG(DEBUG) << "Initilization of " << this->GetName () << FairLogger::endl;

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance ();

  // Get a pointer to the previous already existing data level
  fRaw = static_cast<TClonesArray*> (ioman->GetObject ("SpadicRawMessage"));
  if (!fRaw)
    {
      LOG(FATAL) << "No InputDataLevelName SpadicRawMessage array!\n"
                    << this->GetName () << " will be inactive"
                    << FairLogger::endl;
      return kERROR;
    }

  fDigis = new TClonesArray ("CbmTrdDigi", 100);
  ioman->Register ("TrdDigi", "TRD Digis", fDigis,
                   IsOutputBranchPersistent ("TrdDigi"));

  // Do whatever else is needed at the initilization stage
  // Create histograms to be filled
  // initialize variables
  CbmTrdSimpleDigitizer::CreateHistograms ();

  return kSUCCESS;

}

void
CbmTrdSimpleDigitizer::CreateHistograms ()
{
  for (Int_t layer = 0; layer < fBT->GetNrLayers(); layer++)
    {
      TString HistName = "Baselinemap_Layer_" + std::to_string (layer);
      fHm->Add (
          HistName.Data (),
          new TProfile2D (HistName.Data (), HistName.Data (), 256, -0.5, 255.5,
                          6, -0.5, 5.5));
      fHm->P2 (HistName.Data ())->GetXaxis ()->SetTitle ("Column");
      fHm->P2 (HistName.Data ())->GetYaxis ()->SetTitle ("Row");
    }
  TString GraphName = "Digis_Created";
  fHm->Add (GraphName.Data (), new TGraph);
  fHm->G1 (GraphName.Data ())->SetNameTitle (GraphName.Data (),
                                             GraphName.Data ());
  fHm->G1 (GraphName.Data ())->GetXaxis ()->SetTitle ("TimeSlice");
  fHm->G1 (GraphName.Data ())->GetYaxis ()->SetTitle ("Nr of created Digis");
}
;

void
CbmTrdSimpleDigitizer::Exec (Option_t*)
{
  static Int_t NrTimeslice = 0;
  LOG(INFO) << this->GetName () << ": Clearing  Digis" << FairLogger::endl;
  fDigis->Clear ();
  UInt_t nSpadicMessages = fRaw->GetEntriesFast (); //SPADIC messages per TimeSlice
  TString Statefullness = "stateless";
  LOG(INFO) << this->GetName () << ": Estimating Baseline " << Statefullness
               << " Digitizer" << FairLogger::endl;
  std::vector<TProfile2D*> Hitmaps;
  for (Int_t layer = 0; layer < fBT->GetNrLayers(); layer++)
    {
      TString HistName = "Baselinemap_Layer_" + std::to_string (layer);
      Hitmaps.push_back (fHm->P2 (HistName.Data ()));
    }
  for (UInt_t iSpadicMessage = 0; iSpadicMessage < nSpadicMessages;
      ++iSpadicMessage)
    {
      CbmSpadicRawMessage *raw = static_cast<CbmSpadicRawMessage*> (fRaw->At (
          iSpadicMessage));
      if (!(raw->GetHit ()))
        {
          continue;
        }
      Int_t Baseline = raw->GetSamples ()[0];
      if (Baseline < -150)
        Hitmaps.at (GetLayerID (raw))->Fill (GetColumnID (raw), GetRowID (raw),Baseline);
    }
  std::map<UInt_t, boost::circular_buffer<int16_t> > BaselineMap;
  for (UInt_t iSpadicMessage = 0; iSpadicMessage < nSpadicMessages;
      ++iSpadicMessage)
    {
      CbmSpadicRawMessage *raw = static_cast<CbmSpadicRawMessage*> (fRaw->At (
          iSpadicMessage));
      if (!(raw->GetHit ()))
        {
          continue;
        }
      auto CurrentBuffer = BaselineMap.find (fBT->GetAddress (raw));
      if (CurrentBuffer == BaselineMap.end ())
        {
          auto& TempBuffer = BaselineMap[fBT->GetAddress (raw)];
          TempBuffer.set_capacity (30);
          CurrentBuffer = BaselineMap.find (fBT->GetAddress (raw));
        }
      if (CurrentBuffer->second.full ())
        continue;
      int16_t BaselineMeasure = fBT->GetBaseline (raw);
      //std::cout << " "<< BaselineMeasure << std::endl;
      if (BaselineMeasure < -170)
        CurrentBuffer->second.push_back (BaselineMeasure);
    }
  TString GraphName = "Digis_Created";
  Int_t NrDigis = 0;
  LOG(INFO) << this->GetName () << ": Creating Digis with " << Statefullness
               << " Digitizer" << FairLogger::endl;
  Float_t*Samples = new Float_t[32];
  for (UInt_t iSpadicMessage = 0; iSpadicMessage < nSpadicMessages;
      ++iSpadicMessage)
    {
      CbmSpadicRawMessage *raw = static_cast<CbmSpadicRawMessage*> (fRaw->At (
          iSpadicMessage));
      if (!(raw->GetHit () || raw->GetHitAborted ()))
        {
          continue;
        }
      Float_t BaselineEstimate =
        { 0 };
      if (!raw->GetHitAborted ())
        {
          auto CurrentBuffer = BaselineMap.find (fBT->GetAddress (raw));
          int16_t BaselineMeasure = fBT->GetBaseline (raw);
          if (BaselineMeasure < -170)
            {
              CurrentBuffer->second.push_back (BaselineMeasure);
              BaselineEstimate = static_cast<Float_t> (BaselineMeasure);
            }
          else
            {
              BaselineEstimate = std::accumulate (
                  CurrentBuffer->second.begin (), CurrentBuffer->second.end (),
                  0L);
              //Div-by-Zero Check, should NEVER occur.
              if (!CurrentBuffer->second.empty ())
                BaselineEstimate /= CurrentBuffer->second.size ();
              //std::cout << BaselineEstimate << std::endl;
            }
        }
      else
        {
          auto CurrentBuffer = BaselineMap.find (fBT->GetAddress (raw));
          if(CurrentBuffer==BaselineMap.end())BaselineEstimate = fBT->GetBaseline(raw);
          else
            if (!CurrentBuffer->second.empty ()){
        	BaselineEstimate = std::accumulate(CurrentBuffer->second.begin(),CurrentBuffer->second.end(),0);
        	BaselineEstimate /= CurrentBuffer->second.size ();
            }
          //for (auto x : CurrentBuffer->second)
            //BaselineEstimate+=x;
          //Div-by-Zero Check, should NEVER occur.
        }
      //BaselineEstimate=(fBT->GetBaseline(raw)<BaselineEstimate)?fBT->GetBaseline(raw):BaselineEstimate;
      for (UInt_t i = 0; i < 32; i++)
        {
          if (i < raw->GetNrSamples ())
            Samples[i] = static_cast<Float_t> (raw->GetSamples ()[i])
                - BaselineEstimate;
          else
            Samples[i] = 0.0;
        }
      new ((*fDigis)[NrDigis]) CbmTrdDigi (
          static_cast<Int_t> (fBT->GetAddress (raw)),
          raw->GetFullTime () * 1E3 / 16, //65 ns per timestamp
          raw->GetTriggerType (), raw->GetInfoType (), raw->GetStopType (),
          raw->GetNrSamples (), Samples/*&Samples[32]*/);
      static_cast<CbmTrdDigi*> (fDigis->At (NrDigis++))->SetCharge (
          fBT->GetMaximumAdc(
              raw,
              BaselineEstimate)-BaselineEstimate);
    }
  delete Samples;
  fHm->G1 (GraphName.Data ())->SetPoint (fHm->G1 (GraphName.Data ())->GetN (),
                                         NrTimeslice++, NrDigis);
  fDigis->Sort ();
  LOG(INFO) << this->GetName () << ": Finished creating Digis with "
               << Statefullness << " Digitizer" << FairLogger::endl;
}

ClassImp(CbmTrdSimpleDigitizer)

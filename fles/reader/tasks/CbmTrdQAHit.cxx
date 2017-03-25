#include "CbmTrdQAHit.h"
#include "CbmBeamDefaults.h"
#include "FairLogger.h"
#include "CbmHistManager.h"
#include "TString.h"
#include "TStyle.h"

#include <cmath>
#include <map>
#include <vector>
#include <cmath>
#include <map>
#include <vector>

ClassImp(CbmTrdQABase)
const  Int_t NrRobs=4;
const  Int_t NrSpadics=3;

CbmTrdQAHit::CbmTrdQAHit(TString ClassName) :
  CbmTrdQABase("CbmTrdQAHit"){
}

void CbmTrdQAHit::CreateHistograms(){
  for(Int_t syscore = 0; syscore < NrOfActiveSyscores; ++syscore) {
    for(Int_t spadic = 0; spadic < NrOfActiveSpadics; ++spadic) {
      TString spadicName = GetSpadicName(syscore, spadic, "SysCore",true);
      TString HistName="TriggerType_vs_StopType_"+spadicName;
      fHm->Add(HistName.Data(), new TH2I(HistName.Data(),HistName.Data(),5,-1.5,3.5,7,-1.5,5.5));
      fHm->H2(HistName.Data())->GetYaxis()->SetTitle("StopType");
      fHm->H2(HistName.Data())->GetXaxis()->SetTitle("TriggerType");
      //Triggertype vs Nr of Samples:
      HistName="NrOfSamples_vs_TriggerType_"+spadicName;
      fHm->Add(HistName.Data(), new TH2I(HistName.Data(),HistName.Data(),33,-0.5,32.5,5,-1.5,3.5));
      fHm->H2(HistName.Data())->GetXaxis()->SetTitle("NrOfSamples");
      fHm->H2(HistName.Data())->GetYaxis()->SetTitle("TriggerType");
      HistName="NrOfSamples_vs_StopType_"+spadicName;
      fHm->Add(HistName.Data(), new TH2I(HistName.Data(),HistName.Data(),33,-0.5,32.5,7,-1.5,5.5));
      fHm->H2(HistName.Data())->GetXaxis()->SetTitle("NrOfSamples");
      fHm->H2(HistName.Data())->GetYaxis()->SetTitle("StopType");
    }
  }
  for(Int_t syscore = 0; syscore < NrOfActiveSyscores; ++syscore) {
    for(Int_t spadic = 0; spadic < NrOfActiveSpadics; ++spadic) {
      TString spadicName = GetSpadicName(syscore, spadic, "SysCore",true);
      TString HistName="Hitfrequency_vs_Time_"+spadicName;
      fHm->Add(HistName.Data(), new TGraph());
      fHm->G1(HistName.Data())->SetNameTitle(HistName.Data(),HistName.Data());
      fHm->G1(HistName.Data())->GetYaxis()->SetTitle("HitFrequency");
      fHm->G1(HistName.Data())->GetXaxis()->SetTitle("Fulltime/s");
    }
  }  
}

void CbmTrdQAHit::Exec(Option_t*)
{
  static Int_t NrTimeSlice=-1;
  std::vector<Double_t> LastHit(NrOfActiveSyscores*NrOfActiveSpadics*32);
  /*  LOG(INFO) << this->GetName() <<": Number of current TimeSlice:"
	       << ++NrTimeSlice << FairLogger::endl;
  LOG(INFO) << this->GetName() <<": rawMessages in TS:          "
	       << fRaw->GetEntriesFast () << FairLogger::endl;
  */
  Int_t nSpadicMessages = fRaw->GetEntriesFast (); //SPADIC messages per TimeSlice
  std::vector<TH2*> HistogramArray;
  std::vector<TGraph*> HitfrequencyGraphs;
  for(Int_t RobID=0;RobID<NrOfActiveSyscores;RobID++){
    for(Int_t SpadicID=0;SpadicID<NrOfActiveSpadics;SpadicID++){
      TString spadicName = GetSpadicName(RobID, SpadicID, "SysCore",true);
      TString HistName="TriggerType_vs_StopType_"+spadicName;
      HistogramArray.push_back(fHm->H2(HistName.Data()));
      HistName="Hitfrequency_vs_Time_"+spadicName;
      HitfrequencyGraphs.push_back(fHm->G1(HistName.Data()));
    }
  }
  //first Loop for TriggerType_vs_StopType:
  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage){
    CbmSpadicRawMessage *raw= static_cast<CbmSpadicRawMessage*>(fRaw->At(iSpadicMessage));
    if(raw->GetHit()||raw->GetHitAborted())
      HistogramArray.at(GetRobID(raw)*NrOfActiveSpadics+GetSpadicID(raw)%2)->Fill(raw->GetTriggerType(),raw->GetStopType());
  }
  LOG(INFO)<<this->GetName()<<": Loop for TriggerType_vs_StopType done"<<FairLogger::endl;
  //Loop for NrOfSamples_vs_TriggerType:
  HistogramArray.clear();
  for(Int_t RobID=0;RobID<NrOfActiveSyscores;RobID++){
    for(Int_t SpadicID=0;SpadicID<NrOfActiveSpadics;SpadicID++){
      TString spadicName = GetSpadicName(RobID, SpadicID, "SysCore",true);
      TString HistName="NrOfSamples_vs_TriggerType_"+spadicName;
      HistogramArray.push_back(fHm->H2(HistName.Data()));
    }
  }
  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage){
    CbmSpadicRawMessage *raw= static_cast<CbmSpadicRawMessage*>(fRaw->At(iSpadicMessage));
    if(raw->GetHit()||raw->GetHitAborted())
      HistogramArray.at(GetRobID(raw)*NrOfActiveSpadics+GetSpadicID(raw)%2)->Fill(raw->GetNrSamples(),raw->GetTriggerType());
  }
  LOG(INFO)<<this->GetName()<<": Loop for NrOfSamples_vs_TriggerType done"<<FairLogger::endl;
  //Loop for NrOfSamples_vs_StopType:
  HistogramArray.clear();
  for(Int_t RobID=0;RobID<NrOfActiveSyscores;RobID++){
    for(Int_t SpadicID=0;SpadicID<NrOfActiveSpadics;SpadicID++){
      TString spadicName = GetSpadicName(RobID, SpadicID, "SysCore",true);
      TString HistName="NrOfSamples_vs_StopType_"+spadicName;
      HistogramArray.push_back(fHm->H2(HistName.Data()));
    }
  }
  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage){
    CbmSpadicRawMessage *raw= static_cast<CbmSpadicRawMessage*>(fRaw->At(iSpadicMessage));
    if(raw->GetHit()||raw->GetHitAborted())
      HistogramArray.at(GetRobID(raw)*NrOfActiveSpadics+GetSpadicID(raw)%2)->Fill(raw->GetNrSamples(),raw->GetStopType());
  }
  LOG(INFO)<<this->GetName()<<": Loop for NrOfSamples_vs_StopType done"<<FairLogger::endl;

  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage){
    CbmSpadicRawMessage *raw= static_cast<CbmSpadicRawMessage*>(fRaw->At(iSpadicMessage));  
    if(!(raw->GetHit()||raw->GetTriggerType()==1))continue;
    TGraph* CurrentGraph=HitfrequencyGraphs.at(GetRobID(raw)*NrOfActiveSpadics+GetSpadicID(raw)%2);
    Long64_t CurrentTime=raw->GetFullTime();
    UInt_t Index=GetRobID(raw)*NrOfActiveSpadics*32+GetSpadicID(raw)*16+raw->GetChannelID();
    Double_t CurrentFrequency=16*1E6/static_cast<Double_t>(CurrentTime-LastHit.at(Index));
    LastHit.at(Index)=CurrentTime;
    if(std::isinf(CurrentFrequency)||std::isnan(CurrentFrequency)||CurrentFrequency<10.0)
      continue;
    CurrentGraph->SetPoint(CurrentGraph->GetN(),CurrentTime/(16*1E6),CurrentFrequency);
  }
    LOG(INFO)<<this->GetName()<<": Loop for Hitfrequency_vs_Time done"<<FairLogger::endl;
}


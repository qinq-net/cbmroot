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
      TString HistName="TriggerType_vs_StopType"+spadicName;
      fHm->Add(HistName.Data(), new TH2I(HistName.Data(),HistName.Data(),5,-1.5,3.5,7,-1.5,5.5));
      fHm->H2(HistName.Data())->GetYaxis()->SetTitle("StopType");
      fHm->H2(HistName.Data())->GetXaxis()->SetTitle("TriggerType");
    }
  }
}

void CbmTrdQAHit::Exec(Option_t*)
{
  static Int_t NrTimeSlice=-1;
  LOG(INFO) << this->GetName() <<": Number of current TimeSlice:"
	       << ++NrTimeSlice << FairLogger::endl;
  LOG(INFO) << this->GetName() <<": rawMessages in TS:          "
	       << fRaw->GetEntriesFast () << FairLogger::endl;
  Int_t nSpadicMessages = fRaw->GetEntriesFast (); //SPADIC messages per TimeSlice
  std::vector<TH2*> HistogramArray;
  for(Int_t RobID=0;RobID<NrOfActiveSyscores;RobID++){
    for(Int_t SpadicID=0;SpadicID<NrOfActiveSpadics;SpadicID++){
      TString spadicName = GetSpadicName(RobID, SpadicID, "SysCore",true);
      TString HistName="TriggerType_vs_StopType"+spadicName;
      HistogramArray.push_back(fHm->H2(HistName.Data()));
    }
  }
  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage){
    CbmSpadicRawMessage *raw= static_cast<CbmSpadicRawMessage*>(fRaw->At(iSpadicMessage));
    HistogramArray.at(GetRobID(raw)*NrOfActiveSpadics+GetSpadicID(raw)%2)->Fill(raw->GetTriggerType(),raw->GetStopType());
  }
}


/**
 *Minimal example of an CBM TRD QA analysis class.
 *Any daughter class only needs to implement the CreateHistograms() and the Exec() functions.
 *A future version will be parametriseable for different in-beam tests.
 */
#include "CbmTrdQABase.h"
#include "CbmSpadicRawMessage.h"
#include "CbmTrdTestBeamTools.h"
#include "CbmBeamDefaults.h"

#include "FairLogger.h"

#include "CbmHistManager.h"

#include "TString.h"
#include "TStyle.h"

#include <cmath>
#include <map>
#include <vector>

ClassImp(CbmTrdQABase)



// ---- Default constructor -------------------------------------------
CbmTrdQABase::CbmTrdQABase (CbmTrdTestBeamTools*ptr,TString ClassName) :
  FairTask (ClassName.Data()), fRaw(nullptr),fBT(CbmTrdTestBeamTools::Instance(ptr)), fHm(new CbmHistManager)
{
  LOG(INFO) << TString("Default Constructor of ")+TString(this->GetName())
		<< FairLogger::endl;
}

// ---- Destructor ----------------------------------------------------
CbmTrdQABase::~CbmTrdQABase ()
{
  LOG(DEBUG) << TString("Destructor of ")+TString(this->GetName()) << FairLogger::endl;
}

// ----  Initialisation  ----------------------------------------------
void CbmTrdQABase::SetParContainers ()
{
  LOG(DEBUG) << TString("SetParContainers of ")+TString(this->GetName()) << FairLogger::endl;
  // Load all necessary parameter containers from the runtime data base
  /*
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();

   <CbmTrdQABaseDataMember> = (<ClassPointer>*)
   (rtdb->getContainer("<ContainerName>"));
   */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmTrdQABase::Init ()
{
  LOG(DEBUG) << TString("Initilization of ")+TString(this->GetName()) << FairLogger::endl;

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance ();

  // Get a pointer to the previous already existing data level
  fRaw = static_cast<TClonesArray*> (ioman->GetObject ("SpadicRawMessage"));
  if (!fRaw)
    {
      LOG(FATAL)
	<< "No InputDataLevelName array!\n "<<this->GetName() << " will be inactive"  << FairLogger::endl;
      return kERROR;
    }
  TClonesArray* tempArray=static_cast<TClonesArray*> (ioman->GetObject ("TrdDigi"));
  if (tempArray){
    LOG(INFO) << "Digi Input Available"<< FairLogger::endl;
    fDigis=tempArray;
  }
  tempArray=static_cast<TClonesArray*> (ioman->GetObject ("TrdCluster"));
  if (tempArray){
    LOG(INFO) << "Switched to Cluster Input"<< FairLogger::endl;
    fClusters=tempArray;
    }
  CreateHistograms();
  return kSUCCESS;
}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmTrdQABase::ReInit ()
{
  LOG(DEBUG) << "Initilization of "<<this->GetName() << FairLogger::endl;
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmTrdQABase::Exec (Option_t*)
{
  static Int_t NrTimeSlice=-1;
  LOG(INFO) << this->GetName() <<": Number of current TimeSlice:"
	       << ++NrTimeSlice << FairLogger::endl;
  LOG(INFO) << this->GetName() <<": rawMessages in TS:          "
	       << fRaw->GetEntriesFast () << FairLogger::endl;
  LOG(DEBUG) <<this->GetName() <<": Begin sorting CbmSpadicMessage*"
		<< FairLogger::endl;
  Int_t nSpadicMessages = fRaw->GetEntriesFast (); //SPADIC messages per TimeSlice
  TGraph* EinzelHistogram=fHm->G1("TSCounter");
  std::vector<TGraph*> HistogramArray;
  for(Int_t RobID=0;RobID<fBT->GetNrRobs();RobID++){
    for(Int_t SpadicID=0;SpadicID<fBT->GetNrSpadics()*2;SpadicID++){
      HistogramArray.push_back(fHm->G1(TString("TSCounter"+GetSpadicName(RobID,SpadicID,"Syscore",false)).Data()));
    }
  }
  EinzelHistogram->SetPoint(NrTimeSlice,NrTimeSlice,nSpadicMessages);
  std::vector<Int_t> MessageCounters(fBT->GetNrRobs()*fBT->GetNrSpadics()*2,0);
  for (Int_t iSpadicMessage=0; iSpadicMessage < nSpadicMessages; ++iSpadicMessage){
    CbmSpadicRawMessage *raw= static_cast<CbmSpadicRawMessage*>(fRaw->At(iSpadicMessage));
    MessageCounters.at(GetRobID(raw)*fBT->GetNrSpadics()*2+GetSpadicID(raw))++;
  }
  for(Int_t RobID=0;RobID<fBT->GetNrRobs();RobID++){
    for(Int_t SpadicID=0;SpadicID<fBT->GetNrSpadics()*2;SpadicID++){
      HistogramArray.at(RobID*fBT->GetNrSpadics()*2+SpadicID)->SetPoint(NrTimeSlice,NrTimeSlice,MessageCounters.at(RobID*fBT->GetNrSpadics()*2+SpadicID));
    }
  }
}
// ---- Create Histograms----------------------------------------------
void CbmTrdQABase::CreateHistograms()
{
  //TODO: Implement with CbmTrdTestBeamTools
  fHm->Add("TSCounter",new TGraph);
  fHm->G1("TSCounter")->SetNameTitle("TSCounter","TSCounter");
  fHm->G1("TSCounter")->GetXaxis()->SetTitle("TS Number");
  fHm->G1("TSCounter")->GetYaxis()->SetTitle("Message Counter");
  for(Int_t RobID=0;RobID<fBT->GetNrRobs();RobID++){
    for(Int_t SpadicID=0;SpadicID<fBT->GetNrSpadics()*2;SpadicID++){
      fHm->Add(TString("TSCounter"+GetSpadicName(RobID,SpadicID,"Syscore",false)).Data(),new TGraph);
      TString GraphName="TSCounter"+GetSpadicName(RobID,SpadicID,"Syscore",false);
      fHm->G1(GraphName.Data())->SetNameTitle(GraphName.Data(),GraphName.Data());
      fHm->G1(GraphName.Data())->GetXaxis()->SetTitle("TS Number");
      fHm->G1(GraphName.Data())->GetYaxis()->SetTitle("Message Counter");
    }
  }
}



  

// ---- FinishEvent----------------------------------------------------
/*void CbmTrdQABase::FinishEvent ()
{
  LOG(DEBUG) << "FinishEvent of CbmTrdQABase" << FairLogger::endl;
  fProcSpadic->Clear ();
}*/

// ---- Finish --------------------------------------------------------
void CbmTrdQABase::Finish ()
{
  LOG(DEBUG) << "Finish of "<< this->GetName() << FairLogger::endl;
  FairRootManager::Instance()->GetOutFile()->cd();
  //Save Histograms
  LOG(INFO) << "Saving Histograms of "<< this->GetName() << FairLogger::endl;
  gDirectory->mkdir(this->GetName());
  gDirectory->Cd(this->GetName());
  fHm->WriteToFile();
  FairRootManager::Instance()->GetOutFile()->cd();
  LOG(DEBUG) << "Finish of "<< this->GetName() <<" done." << FairLogger::endl;
}/*
void CbmTrdQABase::FinishTask ()
{
  LOG(DEBUG) << "Finish of CbmTrdQABase" << FairLogger::endl;
  // Update Histos and Canvases
}*/

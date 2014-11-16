#include "CbmTrdRawBeamProfile.h"

#include "CbmSpadicRawMessage.h"
#include "CbmHistManager.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"

#include <string>

// ---- Default constructor -------------------------------------------
CbmTrdRawBeamProfile::CbmTrdRawBeamProfile()
  : FairTask("CbmTrdRawBeamProfile"),
    fRawSpadic(NULL),
   fHM(new CbmHistManager())
{
  LOG(DEBUG) << "Default Constructor of CbmTrdRawBeamProfile" << FairLogger::endl;
}

// ---- Destructor ----------------------------------------------------
CbmTrdRawBeamProfile::~CbmTrdRawBeamProfile()
{
  LOG(DEBUG) << "Destructor of CbmTrdRawBeamProfile" << FairLogger::endl;
}

// ----  Initialisation  ----------------------------------------------
void CbmTrdRawBeamProfile::SetParContainers()
{
  LOG(DEBUG) << "SetParContainers of CbmTrdRawBeamProfile" << FairLogger::endl;
  // Load all necessary parameter containers from the runtime data base
  /*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  <CbmTrdRawBeamProfileDataMember> = (<ClassPointer>*)
    (rtdb->getContainer("<ContainerName>"));
  */
}

// ---- Init ----------------------------------------------------------
InitStatus CbmTrdRawBeamProfile::Init()
{
  LOG(DEBUG) << "Initilization of CbmTrdRawBeamProfile" << FairLogger::endl;

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance();

  // Get a pointer to the previous already existing data level
  fRawSpadic = (TClonesArray*) ioman->GetObject("SpadicRawMessage");
  if ( ! fRawSpadic ) {
    LOG(FATAL) << "No InputDataLevelName array!\n CbmTrdRawBeamProfile will be inactive" << FairLogger::endl;
    return kERROR;
  }

  // Do whatever else is needed at the initilization stage
  // Create histograms to be filled
  // initialize variables
  CreateHistograms();

  return kSUCCESS;

}

// ---- ReInit  -------------------------------------------------------
InitStatus CbmTrdRawBeamProfile::ReInit()
{
  LOG(DEBUG) << "Initilization of CbmTrdRawBeamProfile" << FairLogger::endl;
  return kSUCCESS;
}

// ---- Exec ----------------------------------------------------------
void CbmTrdRawBeamProfile::Exec(Option_t* option)
{
  LOG(DEBUG) << "Exec of CbmTrdRawBeamProfile" << FairLogger::endl;

  Int_t entries = fRawSpadic->GetEntriesFast();
  LOG(INFO) << "******" << FairLogger::endl;
  LOG(INFO) << "Entries: " << entries << FairLogger::endl;

  for (Int_t i=0; i < entries; ++i) {
    CbmSpadicRawMessage* raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(i));
    Int_t eqID = raw->GetEquipmentID();
    Int_t sourceA = raw->GetSourceAddress();
    Int_t chID = raw->GetChannelID();
    /*
      LOG(INFO) << "******" << FairLogger::endl;
      LOG(INFO) << "Equipment ID: " << eqID << Fairlogger::endl;
      LOG(INFO) << "Source Address: " << sourceA << FairLogger::endl;
      LOG(INFO) << "Channel ID: " << chID << FairLogger::endl;
    */

    string syscore;
    switch (eqID) {
    case 57345:  // Münster
      syscore="SysCore1_";
      break;
    case 57346: // Frankfurt
      syscore="SysCore2_";
      break;
    case 57347: // Bucarest
      syscore="SysCore3_";
      break;
    }     

    string spadic;
    switch (sourceA) {
    case 0:  // first spadic
      spadic="Spadic1";
    case 1:  // first spadic
      spadic="Spadic1";
      break;
    case 2:  // second spadic
      spadic="Spadic2";
    case 3:  // second spadic
      spadic="Spadic1";
      break;
    case 4:  // third spadic
      spadic="Spadic2";
    case 5:  // third spadic
      spadic="Spadic2";
      break;
    }     

    string histName = "CountRate_" + syscore + spadic;
    fHM->H1(histName)->Fill(chID);
    histName = "BaseLine_" + syscore + spadic;
    fHM->H2(histName)->Fill(chID,raw->GetSamples()[0]);
    histName = "Integrated_ADC_Spectrum_" + syscore + spadic;
    for (Int_t bin = 1; bin < 32; bin++)
      fHM->H2(histName)->Fill(chID,raw->GetSamples()[bin] - raw->GetSamples()[0]);
  } 
}

// ---- Finish --------------------------------------------------------
void CbmTrdRawBeamProfile::Finish()
{
  LOG(DEBUG) << "Finish of CbmTrdRawBeamProfile" << FairLogger::endl;
}

void CbmTrdRawBeamProfile::CreateHistograms()
{

  // Create histograms for 3 Syscores with maximum 3 Spadics

  string syscoreName[] = { "SysCore1", "SysCore2", "SysCore3" };
  string spadicName[] = { "Spadic1", "Spadic2", "Spadic3" };

  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      string histName = "CountRate_" + syscoreName[syscore] + "_" + spadicName[spadic];
      fHM->Add(histName, new TH1F(histName.c_str(), string(histName + ";Channel;Counts").c_str(), 16, 0, 15));

      histName = "BaseLine_" + syscoreName[syscore] + "_" + spadicName[spadic];
      fHM->Add(histName, new TH2F(histName.c_str(), string(histName + ";Channel;ADC value in Bin 0").c_str(), 16, 0, 15, 511, -256, 255));

      histName = "Integrated_ADC_Spectrum_" + syscoreName[syscore] + "_" + spadicName[spadic];
      fHM->Add(histName, new TH2F(histName.c_str(), string(histName + ";Channel;Integr. ADC values in Bin [1,31]").c_str(), 16, 0, 15, 511*32, 0, 511*32));
    }
  } 


}


ClassImp(CbmTrdRawBeamProfile)

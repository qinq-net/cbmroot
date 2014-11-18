#include "CbmTrdRawBeamProfile.h"

#include "CbmSpadicRawMessage.h"
#include "CbmHistManager.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"

#include "TString.h"

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
  fRawSpadic = static_cast<TClonesArray*>(ioman->GetObject("SpadicRawMessage"));
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

  Int_t channelMapping[32] = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,
			      0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30};
  //  LOG(DEBUG) << "Exec of CbmTrdRawBeamProfile" << FairLogger::endl;

  Int_t entries = fRawSpadic->GetEntriesFast();
  //  LOG(INFO) << "******" << FairLogger::endl;
  LOG(INFO) << "Entries: " << entries << FairLogger::endl;

  for (Int_t i=0; i < entries; ++i) {
    CbmSpadicRawMessage* raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(i));
    Int_t eqID = raw->GetEquipmentID();
    Int_t sourceA = raw->GetSourceAddress();
    Int_t chID = raw->GetChannelID();
    std::cout << "  eqID " << eqID << "  sourceA " << sourceA << "  chID " << chID << std::endl;
    Int_t nrSamples=raw->GetNrSamples();
   
    if ( 32 == nrSamples) {
      TString syscore="";
      switch (eqID) {
      case 57345:  // Münster
	syscore="SysCore1_";
	break;
      case 57346: // Frankfurt
	syscore="SysCore1_";
	break;
      case 57347: // Bucarest
	syscore="SysCore1_";
	break;
      default:
        LOG(FATAL) << "EquipmentID " << eqID << "not known." << FairLogger::endl;
	break;
      }     
      
      TString spadic="";
      switch (sourceA) {
      case 0:  // first spadic
	spadic="Spadic1";
	break;
      case 1:  // first spadic
	spadic="Spadic1";
	chID += 16;
	break;
      case 2:  // second spadic
	spadic="Spadic2";
	break;
      case 3:  // second spadic
	spadic="Spadic1";
	chID += 16;
	break;
      case 4:  // third spadic
	spadic="Spadic2";
	break;
      case 5:  // third spadic
	spadic="Spadic2";
	chID += 16;
	break;
      default:
        LOG(FATAL) << "Source Address " << sourceA << "not known." << FairLogger::endl;
	break;
      }     

      TString channelId;
      channelId.Form("_Ch%02d", chID);

      TString histName = "CountRate_" + syscore + spadic;
      fHM->H1(histName.Data())->Fill(chID);

      histName = "BaseLine_" + syscore + spadic;
      fHM->H2(histName.Data())->Fill(chID,raw->GetSamples()[0]);

      for (Int_t bin = 1; bin < 32; bin++) {
	histName = "Integrated_ADC_Spectrum_" + syscore + spadic;
	fHM->H2(histName.Data())->Fill(chID,
				       raw->GetSamples()[bin] - raw->GetSamples()[0]);
      }

      for (Int_t bin = 0; bin < 32; bin++) {
	histName = "Signal_Shape_" + syscore + spadic + channelId;
	fHM->H2(histName.Data())->Fill(bin,raw->GetSamples()[bin]);

	histName = "Pulse_" + syscore + spadic + channelId;
	fHM->H1(histName.Data())->Fill(bin,raw->GetSamples()[bin]);
      }

      histName = "Trigger_Heatmap_" + syscore + spadic;
      if (chID%2 == 0) {
        fHM->H2(histName.Data())->Fill(chID/2,0);
      } else {
        fHM->H2(histName.Data())->Fill((chID-1)/2,1);
      }

      //histName = "Trigger_Correlation_" + syscore + spadic;// needs time information to correlated events in different chambers
      //fHM->H2(histName.Data())->Fill(chID,chID);
    } else {
      // corrupt message?
    }
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

  TString syscoreName[] = { "SysCore1", "SysCore2", "SysCore3" };
  TString spadicName[]  = { "Spadic1", "Spadic2", "Spadic3" };
  TString channelName[] = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", 
			   "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
			   "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", 
			   "30", "31"};
  for(Int_t syscore = 0; syscore < 3; ++syscore) {
    for(Int_t spadic = 0; spadic < 3; ++spadic) {
      TString histName = "CountRate_" + syscoreName[syscore] + "_" + spadicName[spadic];
      TString title = histName + ";Channel;Counts";
      fHM->Add(histName.Data(), new TH1F(histName, title, 33, 0, 33));

      histName = "BaseLine_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Channel;ADC value in Bin 0";
      fHM->Add(histName.Data(), 
	       new TH2F(histName, title, 33, 0, 33, 511, -256, 255));

      histName = "Integrated_ADC_Spectrum_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Channel;Integr. ADC values in Bin [1,31]";
      fHM->Add(histName.Data(),
	       new TH2F(histName, title, 33, 0, 33, 2000, 0, 2000));

      histName = "Trigger_Heatmap_" + syscoreName[syscore] + "_" + spadicName[spadic];
      title = histName + ";Channel;Trigger Counter";
      fHM->Add(histName.Data(),
	       new TH2F(histName, title, 16, 0, 16, 2, 0, 2));

      for(Int_t  channel = 0; channel < 32; channel++) {
	histName = "Signal_Shape_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	title = histName + ";Channel;ADC value";
	fHM->Add(histName.Data(), 
		 new TH2F(histName, title, 32, 0, 32, 511, -256, 255));
      }
      for(Int_t  channel = 0; channel < 32; channel++) {
	histName = "Pulse_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	title = histName + ";Channel;ADC value";
	fHM->Add(histName.Data(), 
		 new TH1F(histName, title, 32, 0, 32));
      }

      /*
	histName = "Trigger_Correlation_" + syscoreName[syscore] + "_" + spadicName[spadic];
	fHM->Add(histName, new TH2F(histName.c_str(), string(histName + ";Channel;Trigger Counter").c_str(), 32, 0, 32, 32, 0, 32));
      */
    }
  } 


}


ClassImp(CbmTrdRawBeamProfile)

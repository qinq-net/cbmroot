#include "CbmTrdRawBeamProfile.h"

#include "CbmSpadicRawMessage.h"
#include "CbmHistManager.h"
#include "CbmBeamDefaults.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"

#include "TString.h"

#include <cmath>

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

  //Int_t channelMapping[32] = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,
  //			      0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30};
 
  Int_t entries = fRawSpadic->GetEntriesFast();
  Int_t sumTrigger = 0;
  //  LOG(INFO) << "******" << FairLogger::endl;
  if (entries > 0)
    LOG(INFO) << "Entries: " << entries << FairLogger::endl;
  TString triggerTypes[4] = {"Global trigger","Self triggered","Neighbor triggered","Self and neighbor triggered"};
  TString stopTypes[6] = {"Normal end of message", "Channel buffer full", "Ordering FIFO full", "Multi hit", "Multi hit and channel buffer full", "Multi hit and ordering FIFO full"};
  TString infoTypes[8] = {"Channel disabled during message building", "Next grant timeout", "Next request timeout", "New grant but channel empty", "Corruption in message builder", "Empty word", "Epoch out of sync", "infoType out of array"};
  //if (entries > 500) entries = 500; // for fast data visualization
  ULong_t lastSpadicTime[3][6] = {{0}}; //[sys][spa]

  for (Int_t i=0; i < entries; ++i) {

    CbmSpadicRawMessage* raw = static_cast<CbmSpadicRawMessage*>(fRawSpadic->At(i));
    Int_t eqID = raw->GetEquipmentID();
    Int_t sourceA = raw->GetSourceAddress();
    Int_t chID = raw->GetChannelID();
    Int_t SysId(-1), SpaId(-1);
    Int_t nrSamples=raw->GetNrSamples();
    Int_t triggerType=raw->GetTriggerType();
    Int_t stopType=raw->GetStopType();
    Int_t infoType=raw->GetInfoType();
    if (infoType > 6) infoType = 7;
    Int_t groupId=raw->GetGroupId();
    ULong_t time = raw->GetFullTime();
    //cbmroot/fles/spadic/message/constants/..
    //  HIT (GLB, 0x0) /**< Global trigger */
    //  HIT (SLF, 0x1) /**< Self triggered */
    //  HIT (NBR, 0x2) /**< Neighbor triggered */
    //  HIT (SAN, 0x3) /**< Self and neighbor triggered */
    //
    //  STOP (END, 0x0) /**< Normal end of message */
    //	STOP (EBF, 0x1) /**< Channel buffer full */
    //	STOP (EFF, 0x2) /**< Ordering FIFO full */
    //	STOP (EDH, 0x3) /**< Multi hit */
    //	STOP (EDB, 0x4) /**< Multi hit and channel buffer full */
    //	STOP (EDO, 0x5) /**< Multi hit and ordering FIFO full */
    //
    //	INFO (DIS, 0x0) /**< Channel disabled during message building */
    //	INFO (NGT, 0x1) /**< Next grant timeout */
    //	INFO (NRT, 0x2) /**< Next request timeout */
    //	INFO (NBE, 0x3) /**< New grant but channel empty */
    //	INFO (MSB, 0x4) /**< Corruption in message builder */
    //	INFO (NOP, 0x5) /**< Empty word */
    //	INFO (SYN, 0x6) /**< Epoch out of sync */

	  TString syscore="";
    switch (eqID) {
    case kMuenster:  // Muenster
      syscore="SysCore0_";
      SysId = 0;
      break;
    case kFrankfurt: // Frankfurt
      syscore="SysCore1_";
      SysId = 1;
      break;
    case kBucarest: // Bucarest
      syscore="SysCore2_";
      SysId = 2;
      break;
    default:
      LOG(FATAL) << "EquipmentID " << eqID << "not known." << FairLogger::endl;
      break;
    }     
      
    TString spadic="";
    switch (sourceA) {
    case (SpadicBaseAddress+0):  // first spadic
      spadic="Spadic0";
      SpaId = 0;
      break;
    case (SpadicBaseAddress+1):  // first spadic
      spadic="Spadic0";
      SpaId = 1;
      chID += 16;
      break;
    case (SpadicBaseAddress+2):  // second spadic
      spadic="Spadic1";
      SpaId = 2;
      break;
    case (SpadicBaseAddress+3):  // second spadic
      spadic="Spadic1";
      SpaId = 3;
      chID += 16;
      break;
    case (SpadicBaseAddress+4):  // third spadic
      spadic="Spadic2";
      SpaId = 4;
      break;
    case (SpadicBaseAddress+5):  // third spadic
      spadic="Spadic2";
      SpaId = 5;
      chID += 16;
      break;
    default:
      LOG(FATAL) << "Source Address " << sourceA << "not known." << FairLogger::endl;
      break;
    }   

    if (time >= lastSpadicTime[SysId][SpaId]){
      // ok
    } else {
      LOG(ERROR) << "SPADIC " << SysId << SpaId << " event time " << time << " < last time " << lastSpadicTime[SysId][SpaId] << FairLogger::endl;
    }
  
    fHM->H1(TString("TriggerTypes_" + syscore + spadic).Data())->Fill(triggerTypes[triggerType],1);
    fHM->H1(TString("StopTypes_" + syscore + spadic).Data())->Fill(stopTypes[stopType],1);
    if (infoType <= 7)
      fHM->H1(TString("InfoTypes_" + syscore + spadic).Data())->Fill(infoTypes[infoType],1);
    fHM->H1(TString("GroupId_" + syscore + spadic).Data())->Fill(groupId);
    //sumTrigger++;
    if (stopType > 0){ //corrupt or multi message
      TString histName = "ErrorCounter_" + syscore + spadic;
      fHM->H1(histName.Data())->Fill(chID);   
    } else  {  //only normal message end
      sumTrigger++;
      TString channelId;
      channelId.Form("_Ch%02d", chID);

      TString histName = "CountRate_" + syscore + spadic;
      fHM->H1(histName.Data())->Fill(chID);

      histName = "BaseLine_" + syscore + spadic;
      fHM->H2(histName.Data())->Fill(chID,raw->GetSamples()[0]);

      Float_t AdcIntegral = 0;
      for (Int_t bin = 1; bin < nrSamples; bin++) {
	AdcIntegral += raw->GetSamples()[bin] + 256;// - raw->GetSamples()[0]);

	histName = "Signal_Shape_" + syscore + spadic + channelId;
	fHM->H2(histName.Data())->Fill(bin,raw->GetSamples()[bin]);
      }

      histName = "Integrated_ADC_Spectrum_" + syscore + spadic;
      fHM->H2(histName.Data())->Fill(chID, AdcIntegral);

      histName = "Trigger_Heatmap_" + syscore + spadic;
      if (chID%2 == 0) {
	fHM->H2(histName.Data())->Fill(chID/2,0);
      } else {
	fHM->H2(histName.Data())->Fill((chID-1)/2,1);
      } 
    } 
    if (i == entries-1){
      for (Int_t sy = 0; sy < 2; sy++){
	for (Int_t sp = 0; sp < 2; sp++){
	  TString histName = "TriggerCounter_SysCore" + std::to_string(sy) + "_Spadic" + std::to_string(sp);
	  for (Int_t timeSlice = 1; timeSlice <= fHM->H1(histName.Data())->GetNbinsX(); timeSlice++){
	    fHM->H1(histName.Data())->SetBinContent(timeSlice,fHM->H1(histName.Data())->GetBinContent(timeSlice+1)); // shift all bin one to the left
	  
	    if (timeSlice == fHM->H1(histName.Data())->GetNbinsX())
	      fHM->H1(histName.Data())->SetBinContent(timeSlice,0); // set all last bins to 0
	  } 
	}
      }
      TString histName = "TriggerCounter_" + syscore + spadic;
      fHM->H1(histName.Data())->SetBinContent(fHM->H1(histName.Data())->GetNbinsX(),sumTrigger);// set only the spa sys combi to new value
      fHM->H1("TriggerSum")->Fill(TString(syscore+spadic),sumTrigger);
      if (sumTrigger == 0) std::cout << syscore + spadic << std::endl;
    } // last entry
    lastSpadicTime[SysId][SpaId] = time;
  } //entries
}


  // ---- Finish --------------------------------------------------------
  void CbmTrdRawBeamProfile::Finish()
  {
    LOG(DEBUG) << "Finish of CbmTrdRawBeamProfile" << FairLogger::endl;
  }

  void CbmTrdRawBeamProfile::CreateHistograms()
  {

    // Create histograms for 3 Syscores with maximum 3 Spadics

    TString syscoreName[3] = { "SysCore0", "SysCore1", "SysCore2" };
    TString spadicName[3]  = { "Spadic0", "Spadic1", "Spadic2" };
    TString channelName[32] = { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", 
				"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", 
				"20", "21", "22", "23", "24", "25", "26", "27", "28", "29", 
				"30", "31"};
    TString triggerTypes[4] = { "Global trigger","Self triggered","Neighbor triggered","Self and neighbor triggered"};
    TString stopTypes[6] = {"Normal end of message", "Channel buffer full", "Ordering FIFO full", "Multi hit", "Multi hit and channel buffer full", "Multi hit and ordering FIFO full"};
    TString infoTypes[8] = {"Channel disabled during message building", "Next grant timeout", "Next request timeout", "New grant but channel empty", "Corruption in message builder", "Empty word", "Epoch out of sync", "infoType out of array"};

    fHM->Add("TriggerSum", new TH1F("TriggerSum", "TriggerSum", 9,0,9));
    //fHM->H1("TriggerSum")->GetYaxis()->SetRangeUser(0,1E6);
    for(Int_t syscore = 0; syscore < 3; ++syscore) {
      for(Int_t spadic = 0; spadic < 3; ++spadic) {
	fHM->H1("TriggerSum")->GetXaxis()->SetBinLabel(3*syscore+spadic+1,TString(syscoreName[syscore]+"_"+spadicName[spadic]));

	TString histName = "CountRate_" + syscoreName[syscore] + "_" + spadicName[spadic];
	TString title = histName + ";Channel;Counts";
	fHM->Add(histName.Data(), new TH1F(histName, title, 32, 0, 32));

	histName = "TriggerTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Counts";
	fHM->Add(histName.Data(), new TH1F(histName, title, 4, 0, 4));
	for (Int_t tType=0; tType < 4; tType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(tType+1,triggerTypes[tType]);

	histName = "StopTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Counts";
	fHM->Add(histName.Data(), new TH1F(histName, title, 6, 0, 6));
	for (Int_t sType=0; sType < 6; sType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(sType+1,stopTypes[sType]);

	histName = "InfoTypes_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Counts";
	fHM->Add(histName.Data(), new TH1F(histName, title, 7, 0, 7));
	for (Int_t iType=0; iType < 8; iType++)
	  fHM->H1(histName.Data())->GetXaxis()->SetBinLabel(iType+1,infoTypes[iType]);

	histName = "GroupId_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + "; ;Counts";
	fHM->Add(histName.Data(), new TH1F(histName, title, 2, 0, 2));

	histName = "TriggerCounter_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";TimeSlice;Trigger / TimeSlice";
	fHM->Add(histName.Data(), new TH1F(histName, title, 5000, 0, 5000));

	histName = "ErrorCounter_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Channel;ADC value in Bin 0";
	fHM->Add(histName.Data(), new TH1F(histName, title, 32, 0, 32));

	histName = "BaseLine_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Channel;ADC value in Bin 0";
	fHM->Add(histName.Data(), new TH2F(histName, title, 32, 0, 32, 511, -256, 255));

	histName = "Integrated_ADC_Spectrum_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Channel;Integr. ADC values in Bin [1,31]";
	fHM->Add(histName.Data(), new TH2F(histName, title, 32, 0, 32, 33*256, 0, 33*256));

	histName = "Trigger_Heatmap_" + syscoreName[syscore] + "_" + spadicName[spadic];
	title = histName + ";Channel;Trigger Counter";
	fHM->Add(histName.Data(), new TH2F(histName, title, 16, 0, 16, 2, 0, 2));

	for(Int_t  channel = 0; channel < 32; channel++) {
	  histName = "Signal_Shape_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	  title = histName + ";Time Bin;ADC value";
	  fHM->Add(histName.Data(), new TH2F(histName, title, 33, 0, 33, 511, -256, 255));
	}
	for(Int_t  channel = 0; channel < 32; channel++) {
	  histName = "Pulse_" + syscoreName[syscore] + "_" + spadicName[spadic] + "_Ch" + channelName[channel];
	  title = histName + ";Time Bin;ADC value";
	  fHM->Add(histName.Data(), new TH1F(histName, title, 33, 0, 33));
	}

	/*
	  histName = "Trigger_Correlation_" + syscoreName[syscore] + "_" + spadicName[spadic];
	  fHM->Add(histName, new TH2F(histName.c_str(), string(histName + ";Channel;Trigger Counter").c_str(), 32, 0, 32, 32, 0, 32));
	*/
      }
    } 


  }


  ClassImp(CbmTrdRawBeamProfile)

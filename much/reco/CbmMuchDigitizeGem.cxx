/** CbmMuchDigitizeGem.cxx
 *@author Vikas Singhal <vikas@vecc.gov.in>
 *@since 01.10.16
 *@version 2.0
 *@author Evgeny Kryshen <e.kryshen@gsi.de>
 *@since 01.05.11
 *@version 2.0
 *@author Mikhail Ryzhinskiy <m.ryzhinskiy@gsi.de>
 *@since 19.03.07
 *@version 1.0
 **
 ** CBM task class for digitizing MUCH for both Event by event and Time based mode.
 ** Task level RECO
 ** Produces objects of type CbmMuchDigi out of CbmMuchPoint.
 **/

// Includes from MUCH
#include "CbmMuchDigitizeGem.h"
#include "CbmMuchPoint.h"
#include "CbmMuchSector.h"
#include "CbmMuchStation.h"
#include "CbmMuchModuleGem.h"
#include "CbmMuchModuleGemRadial.h"
#include "CbmMuchModuleGemRectangular.h"
#include "CbmMuchPad.h"
#include "CbmMuchPadRadial.h"
#include "CbmMuchPadRectangular.h"
#include "CbmMuchSectorRadial.h"
#include "CbmMuchSectorRectangular.h"
#include "CbmMuchDigi.h"
#include "CbmMuchReadoutBuffer.h"

// Includes from base
#include "FairRootManager.h"
#include "FairEventHeader.h"
#include "FairMCEventHeader.h"
#include "FairMCPoint.h"
#include "FairRunAna.h"
#include "FairRunSim.h"
#include "FairLogger.h"

// Includes from Cbm
#include "CbmMCTrack.h"
#include "CbmMCEpoch.h"
//#include "CbmMCBuffer.h"
#include "TObjArray.h"
#include "TDatabasePDG.h"
#include "TFile.h"
#include "TRandom.h"
#include "TChain.h"
#include "TH1D.h"

#include <cassert>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include "../../base/CbmDaqBuffer.h"

using std::fixed;
using std::right;
using std::setprecision;
using std::setw;
using std::string;
using std::map;
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmMuchDigitizeGem::CbmMuchDigitizeGem()
  : CbmDigitize("MuchDigitizeGem"),
    //fgDeltaResponse(),
    fAlgorithm(1),
    fGeoScheme(CbmMuchGeoScheme::Instance()),
    fDigiFile(),
    fFlag(0),
    fPoints(NULL),
    fMCTracks(NULL),
    fDigis(NULL),
    fDigiMatches(NULL),
    fNFailed(0),
    fNOutside(0),
    fNMulti(0),
    fNADCChannels(256),
    fQMax(500000),
    fQThreshold(10000),
    fMeanNoise(1500),
    fSpotRadius(0.05),
    fMeanGasGain(1e4),
    fDTime(3),
    fDeadPadsFrac(0),
    fTimer(),
    fMcChain(NULL),
    //fDeadTime(400),
    fDriftVelocity(100),
    //fPeakingTime(20),
    //fRemainderTime(40),
  fTimeBinWidth(1),
  fNTimeBins(200),
  fTOT(0),
  fTotalDriftTime(0.4/fDriftVelocity*10000), // 40 ns
  fSigma(),
  fMPV(),
  fIsLight(1), // fIsLight = 1 (default) Store Light CbmMuchDigiMatch in output branch, fIsLight = 0 Create Heavy CbmMuchDigiMatch with fSignalShape info.
  fTimePointLast(-1),
  fTimeDigiFirst(-1),
  fTimeDigiLast(-1),
  fNofPoints(0),
  fNofSignals(0),
  fNofDigis(0),
  fNofEvents(0),
  fNofPointsTot(0.),
  fNofSignalsTot(0.),
  fNofDigisTot(0.),
  fTimeTot(),
  fAddressCharge(),
  fGenerateElectronicsNoise(kFALSE),
  fPerPadNoiseRate(10e-9),
  fNoiseCharge(nullptr)
{
  fSigma[0] = new TF1("sigma_e","pol6",-5,10);
  fSigma[0]->SetParameters(sigma_e);

  fSigma[1] = new TF1("sigma_mu","pol6",-5,10);
  fSigma[1]->SetParameters(sigma_mu);

  fSigma[2] = new TF1("sigma_p","pol6",-5,10);
  fSigma[2]->SetParameters(sigma_p);

  fMPV[0]   = new TF1("mpv_e","pol6",-5,10);
  fMPV[0]->SetParameters(mpv_e);

  fMPV[1]   = new TF1("mpv_mu","pol6",-5,10);
  fMPV[1]->SetParameters(mpv_mu);

  fMPV[2]   = new TF1("mpv_p","pol6",-5,10);
  fMPV[2]->SetParameters(mpv_p);
  Reset();
  fNoiseCharge = new TF1("Noise Charge", "TMath::Gaus(x, [0], [1])",
			 fQThreshold, fQMax/10);  //noise function to calculate charge for noise hit. mean=fQThreashold(10000),fQMax=500000

}
// -------------------------------------------------------------------------



// -------------------------------------------------------------------------
CbmMuchDigitizeGem::CbmMuchDigitizeGem(const char* digiFileName, Int_t flag) 
  : CbmDigitize("MuchDigitizeGem"),
    //fgDeltaResponse(),
    fAlgorithm(1),
    fGeoScheme(CbmMuchGeoScheme::Instance()),
    fDigiFile(digiFileName),
    fFlag(flag),
    fPoints(NULL),
    fMCTracks(NULL),
    fDigis(NULL),
    fDigiMatches(NULL),
    fNFailed(0),
    fNOutside(0),
    fNMulti(0),
    fNADCChannels(256),
    fQMax(500000),
    fQThreshold(10000),
    fMeanNoise(1500),
    fSpotRadius(0.05),
    fMeanGasGain(1e4),
    fDTime(3),
    fDeadPadsFrac(0),
    fTimer(),
    fMcChain(NULL),
//fDeadTime(400),
  fDriftVelocity(100),
//fPeakingTime(20),
//fRemainderTime(40),
  fTimeBinWidth(1),
  fNTimeBins(200),
  fTOT(0),
  fTotalDriftTime(0.4/fDriftVelocity*10000), // 40 ns
  fSigma(),
  fMPV(),
  fIsLight(1), // fIsLight = 1 (default) Store Light CbmMuchDigiMatch in output branch, fIsLight = 0 Create Heavy CbmMuchDigiMatch with fSignalShape info.  
  fTimePointLast(-1),
  fTimeDigiFirst(-1),
  fTimeDigiLast(-1),
  fNofPoints(0),
  fNofSignals(0),
  fNofDigis(0),
  fNofEvents(0),
  fNofPointsTot(0.),
  fNofSignalsTot(0.),
  fNofDigisTot(0.),
  fTimeTot(),
  fAddressCharge(),
  fGenerateElectronicsNoise(kFALSE),
  fPerPadNoiseRate(10e-9),
  fNoiseCharge(nullptr)
{
  fSigma[0] = new TF1("sigma_e","pol6",-5,10);
  fSigma[0]->SetParameters(sigma_e);

  fSigma[1] = new TF1("sigma_mu","pol6",-5,10);
  fSigma[1]->SetParameters(sigma_mu);

  fSigma[2] = new TF1("sigma_p","pol6",-5,10);
  fSigma[2]->SetParameters(sigma_p);

  fMPV[0]   = new TF1("mpv_e","pol6",-5,10);
  fMPV[0]->SetParameters(mpv_e);

  fMPV[1]   = new TF1("mpv_mu","pol6",-5,10);
  fMPV[1]->SetParameters(mpv_mu);

  fMPV[2]   = new TF1("mpv_p","pol6",-5,10);
  fMPV[2]->SetParameters(mpv_p);
  Reset();
  fNoiseCharge = new TF1("Noise Charge", "TMath::Gaus(x, [0], [1])",
			 fQThreshold, fQMax/10);  //noise function to calculate charge for noise hit. mean=fQThreashold(10000),fQMax=500000


}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMuchDigitizeGem::~CbmMuchDigitizeGem() {
  if (fDigis) {
    fDigis->Delete();
    delete fDigis;
  }
  if (fDigiMatches) {
    fDigiMatches->Delete();
    delete fDigiMatches;
  }

  delete fSigma[0];
  delete fSigma[1];
  delete fSigma[2];
  delete fMPV[0];
  delete fMPV[1];
  delete fMPV[2];

}
// -------------------------------------------------------------------------



// -----   Private method Reset   -------------------------------------------
void CbmMuchDigitizeGem::Reset() {
  fTimeDigiFirst = fTimeDigiLast = -1;
  fNofPoints = fNofSignals = fNofDigis = 0;
}
// -------------------------------------------------------------------------






// -----   Private method Init   -------------------------------------------
InitStatus CbmMuchDigitizeGem::Init() {

  // Screen output
  std::cout << std::endl;
  LOG(INFO) << "=========================================================="
	    << FairLogger::endl;
  LOG(INFO) << GetName() << ": Initialisation" << FairLogger::endl
	    << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (!ioman)
    Fatal("Init", "No FairRootManager");

  if ( fEventMode ) {
    LOG(INFO) << fName << ": Using event-by-event mode"
	      << FairLogger::endl;
  }


  // Get geometry version tag
  gGeoManager->CdTop();
  TGeoNode* cave = gGeoManager->GetCurrentNode();  // cave
  TString geoTag;
  for (Int_t iNode = 0; iNode < cave->GetNdaughters(); iNode++) {
    TString name = cave->GetDaughter(iNode)->GetVolume()->GetName();
    if ( name.Contains("much", TString::kIgnoreCase) ) {
      geoTag = TString(name(5, name.Length() - 5));
      LOG(INFO) << fName << ": MUCH geometry tag is " << geoTag
		<< FairLogger::endl;
      break;
    } //? node is MUCH
  } //# top level nodes


  // Set the parameter file and the flag, if not done in constructor
  if ( fDigiFile.IsNull() ) {
    if ( geoTag.IsNull() ) LOG(FATAL) << fName
				      << ": no parameter file specified and no MUCH node found in geometry!"
				      << FairLogger::endl;
    fDigiFile = gSystem->Getenv("VMCWORKDIR");
    // TODO: (VF) A better naming convention for the geometry tag and the
    // corresponding parameter file is surely desirable.
    fDigiFile += "/parameters/much/much_" + geoTag(0,4)
      + "_digi_sector.root";
    LOG(INFO) << fName << ": Using parameter file "
	      << fDigiFile << FairLogger::endl;

    fFlag = (geoTag.Contains("mcbm", TString::kIgnoreCase) ? 1 : 0);
    LOG(INFO) << fName << ": Using flag " << fFlag
	      << (fFlag ? " (mcbm) " : "(standard)") << FairLogger::endl;
  }


  // Initialize GeoScheme
  TFile* oldfile=gFile;
  TFile* file=new TFile(fDigiFile);
  if ( ! file->IsOpen() ) LOG(FATAL) << fName << ": parameter file " << fDigiFile
				     << " does not exist!" << FairLogger::endl;
  TObjArray* stations = (TObjArray*) file->Get("stations");
  file->Close();
  file->Delete();
  gFile=oldfile;
  fGeoScheme->Init(stations,fFlag);
  // Determine drift volume width
  Double_t driftVolumeWidth = 0.4; // cm - default
  for (Int_t i=0;i<fGeoScheme->GetNStations();i++){
    CbmMuchStation* station = fGeoScheme->GetStation(i);
    if (station->GetNLayers()<=0) continue;
    CbmMuchLayerSide* side = station->GetLayer(0)->GetSide(0);
    if (side->GetNModules()<=0) continue;
    CbmMuchModule* module = side->GetModule(0);
    if (module->GetDetectorType()!=1 && module->GetDetectorType()!=3) continue;
    driftVolumeWidth = module->GetSize().Z();
    break;
  }
  fTotalDriftTime = driftVolumeWidth/fDriftVelocity*10000; // [ns];
  //Reading MC point as Event by event for time based digi generation also.  
  // Get input array of MuchPoints
  fPoints = (TClonesArray*) ioman->GetObject("MuchPoint");
  assert( fPoints );

  // Get input array of MC tracks
  fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
  assert( fMCTracks );
  
  // Register output arrays
  fDigis = new TClonesArray("CbmMuchDigi", 1000);
  ioman->Register("MuchDigi", "Digital response in MUCH", fDigis, kTRUE);
  fDigiMatches = new TClonesArray("CbmMuchDigiMatch", 1000);
  ioman->Register("MuchDigiMatch", "Digi Match in MUCH", fDigiMatches, kTRUE);

  //fgDeltaResponse is used in the CbmMuchSignal for analysing the Signal Shape,
  //it is generated once in the digitizer and then be used by each CbmMuchSignal.
  //For reducing the time therefore generated once in the CbmMuchDigitize Gem and
  //not generated in the CbmMuchSignal
  // Set response on delta function
  
  // Not using fSignalShape as consuming memory. Commening all the field related.

  //Int_t nShapeTimeBins=Int_t(gkResponsePeriod/gkResponseBin);
  //fgDeltaResponse.Set(nShapeTimeBins);
  //for (Int_t i=0;i<fgDeltaResponse.GetSize();i++){
  //  Double_t time = i*gkResponseBin;
  //  if (time<=fPeakingTime) fgDeltaResponse[i]=time/fPeakingTime;
  //  else fgDeltaResponse[i] = exp(-(time-fPeakingTime)/fRemainderTime); 
  //}

  // --- Enable histogram if want to enalyze Noise spectrum.
  //noise = new TH1D("noise", "Noise Generated per Event NoiseRate 10e-8", 100 , 0 , 200);
  LOG(INFO) << GetName() << ": Initialisation successful"
	    << FairLogger::endl;
  LOG(INFO) << "=========================================================="
	    << FairLogger::endl;
  std::cout << std::endl;

  return kSUCCESS;
}
// -------------------------------------------------------------------------


// -----   Public method Exec   --------------------------------------------
void CbmMuchDigitizeGem::Exec(Option_t*) {
  
  // --- Start timer and reset counters
  fTimer.Start();
  Reset();

  // --- Event number and time
  GetEventInfo();
  LOG(DEBUG) << GetName() << ": Processing event " << fCurrentEvent
	     << " from input " << fCurrentInput << " at t = " << fCurrentEventTime
	     << " ns with " << fPoints->GetEntriesFast() << " MuchPoints "
	     << FairLogger::endl;

  //ReadAndRegister(fCurrentEventTime);
     
  for (Int_t iPoint=0; iPoint<fPoints->GetEntriesFast(); iPoint++) {
    const CbmMuchPoint* point = (const CbmMuchPoint*) fPoints->At(iPoint);
    LOG(DEBUG1) << GetName() << ": Processing MCPoint " << iPoint << FairLogger::endl;
    assert( point );
    ExecPoint(point, iPoint);
    fNofPoints++;
  }  // MuchPoint loop

  if( fEventMode ) ReadAndRegister(-1);
  else ReadAndRegister(fCurrentEventTime);
	
  // --- Event log
  LOG(INFO) << "+ " << setw(15) << GetName() << ": Event " << setw(6)
	    << right << fCurrentEvent << " at " << fixed << setprecision(3)
	    << fCurrentEventTime << " ns, points: " << fNofPoints
	    << ", signals: " << fNofSignals
	    << ", digis: " << fNofDigis << ". Exec time " << setprecision(6)
	    << fTimer.RealTime() << " s." << FairLogger::endl;

  // --------------------NOISE---------------------- //
  if (fGenerateElectronicsNoise) {

    fPreviousEventTime = fNofEvents ? fPreviousEventTime : 0.;
    Int_t nNoise = GenerateNoise(fPreviousEventTime,fCurrentEventTime);
    fNofNoiseTot += nNoise;
     //noise->Fill(nNoise);
    LOG(INFO) << "+ " << setw(20) << GetName() << ": Generated  " << nNoise
	      << " noise signals from t = " << fPreviousEventTime << " ns to "
	      << fCurrentEventTime << " ns" << FairLogger::endl;
    LOG(DEBUG3) << "+ " << setw(20) << GetName() << ": Generated  " << fNofNoiseSignals
		<< " noise signals for this time slice from t = " << fPreviousEventTime << " ns to "  << fCurrentEventTime << "ns" << FairLogger::endl;
  }
  LOG(INFO) << "+ " << setw(20) << GetName() << " : " << fNofNoiseTot << " total noise generated till now." << FairLogger::endl;
  fPreviousEventTime = fCurrentEventTime ;
 
  fTimer.Stop();
  fNofEvents++;
  fNofPointsTot   += fNofPoints;
  fNofSignalsTot  += fNofSignals;
  fNofDigisTot    += fNofDigis;
  fTimeTot        += fTimer.RealTime();

}
Int_t CbmMuchDigitizeGem::GenerateNoise(Double_t t1, Double_t t2){
  LOG(DEBUG) << "+ " << setw(20) << GetName()<< ": Previous Time " << fPreviousEventTime << " Current Time " << fCurrentEventTime << "ns" << FairLogger::endl;
  assert( t2 > t1 );
  Int_t numberofstations=fGeoScheme->GetNStations();
  auto StationNoise = 0;
  for (Int_t i=0;i<numberofstations;i++){
    CbmMuchStation* station = fGeoScheme->GetStation(i);
    auto numberoflayers=station->GetNLayers();
    if (numberoflayers<=0) continue;

    auto LayerNoise = 0;
    for (Int_t j=0;j<numberoflayers;j++){
      CbmMuchLayerSide* side = station->GetLayer(j)->GetSide(0);
      auto numberofmodules=side->GetNModules();
      if (numberofmodules<=0) continue;

      auto FrontModuleNoise = 0;
      for (auto k=0;k<numberofmodules;k++){
	CbmMuchModuleGem* module = (CbmMuchModuleGem*)(side->GetModule(k));
	if (module->GetDetectorType()!=1 && module->GetDetectorType()!=3) continue;
	FrontModuleNoise += GenerateNoisePerModule(module,t1,t2);
      }
      side = station->GetLayer(j)->GetSide(1);
      numberofmodules=side->GetNModules();
      if (numberofmodules<=0) continue;
      auto BackModuleNoise = 0;
      for (auto k=0;k<numberofmodules;k++){
	CbmMuchModuleGem* module = (CbmMuchModuleGem*)side->GetModule(k);
	if (module->GetDetectorType()!=1 && module->GetDetectorType()!=3) continue;
	BackModuleNoise += GenerateNoisePerModule(module,t1,t2);
      }
      LayerNoise+=FrontModuleNoise+BackModuleNoise;
    }
    LOG(DEBUG1) << "+ " << setw(20) << GetName() << ": Generated  " << LayerNoise
	      << " noise signals in station " << i <<" from t = " << fPreviousEventTime << " ns to "
	      << fCurrentEventTime << " ns" << FairLogger::endl;
    StationNoise+=LayerNoise;
  }
  return StationNoise;
}

//================================Generte Noise==============================================//

Int_t CbmMuchDigitizeGem::GenerateNoisePerModule(CbmMuchModuleGem* module, Double_t t1,Double_t t2) {
  auto NumberOfPad = module->GetNPads();
  Double_t nNoiseMean = fPerPadNoiseRate * NumberOfPad * ( t2 - t1 );
  Int_t nNoise = gRandom->Poisson(nNoiseMean);
  LOG(DEBUG) << "+ " << setw(20) << GetName() << ": Number of noise signals : "  << nNoise << " in one module. "<< FairLogger::endl;
  for (Int_t iNoise = 0; iNoise <= nNoise; iNoise++) {
    Int_t padnumber = Int_t(gRandom->Uniform(Double_t(NumberOfPad)));
    CbmMuchPad *pad = module->GetPad(padnumber);
    Double_t NoiseTime = gRandom->Uniform(t1, t2);
    Double_t charge = fNoiseCharge->GetRandom();
    while(charge<0) charge = fNoiseCharge->GetRandom();
    AddNoiseSignal(pad, NoiseTime, charge);
  }
  //noise->Fill(nNoise);
  return nNoise;
}

//=================Add a signal to the buffer=====================//
 
void CbmMuchDigitizeGem::AddNoiseSignal(CbmMuchPad* pad, Double_t time,Double_t charge) {
  assert(pad);
  LOG(DEBUG3) << GetName() << ": Receiving signal " << charge
	      << " in channel " << pad->GetAddress() << " at time "
	      << time << "ns" << FairLogger::endl;
  //  LOG(DEBUG) << GetName() << ": discarding signal in dead channel "
  //  << channel << FairLogger::endl;
  //  return;
  CbmMuchSignal* signal = new CbmMuchSignal(pad->GetAddress(),time);
  //signal->SetTimeStart(time);
  //signal->SetTimeStop(time+fDeadTime);
  signal->SetCharge((UInt_t)charge);
  UInt_t address = pad->GetAddress();
  CbmMuchReadoutBuffer::Instance()->Fill(address, signal);
  fNofNoiseSignals++;
  LOG(DEBUG3)<< "+ " << setw(20) << GetName() <<": Registered a Noise CbmMuchSignal into the CbmMuchReadoutBuffer. Number of Noise Signal generated "<< fNofNoiseSignals <<FairLogger::endl;
}
//====================End of Noise part=================//

// -------------------------------------------------------------------------
//Read all the Signal from CbmMuchReadoutBuffer, convert the analog signal into the digital response  and register Output according to event by event mode and Time based mode.
void CbmMuchDigitizeGem::ReadAndRegister(Long_t eventTime){
  std::vector<CbmMuchSignal*> SignalList;
  //Event Time should be passed with the Call	
  /*Double_t eventTime = -1.;
    if(fDaq){
    eventTime = FairRun::Instance()->GetEventHeader()->GetEventTime();
    }*/
	
  Int_t ReadOutSignal = CbmMuchReadoutBuffer::Instance()->ReadOutData(eventTime,SignalList);
  LOG(DEBUG) << GetName() << ": Number of Signals read out from Buffer "<< ReadOutSignal << " and SignalList contain " << SignalList.size() << " entries."<<FairLogger::endl;

  for (std::vector<CbmMuchSignal*>::iterator LoopOver= SignalList.begin(); LoopOver != SignalList.end(); LoopOver++)
    {
      CbmMuchDigi* digi = ConvertSignalToDigi(*LoopOver);
      //assert(digi);
      if (!digi){
	LOG(DEBUG2)<< GetName() << ": Digi not created as signal is below threshold."<<FairLogger::endl;
      }
      else {
	LOG(DEBUG2)<< GetName() << ": New digi: sector = "<< CbmMuchAddress::GetSectorIndex(digi->GetAddress()) <<" channel= " << CbmMuchAddress::GetChannelIndex(digi->GetAddress())<< FairLogger::endl;

	SendDigi(digi);
	fNofDigis++;
      }
    }

  LOG(DEBUG) << GetName() << ": " << fNofDigis
	     << ( fNofDigis == 1 ? " digi " :  " digis " )
	     << "created and sent to DAQ ";
  if ( fNofDigis ) LOG(DEBUG) << "( from " << fixed
			      << setprecision(3) << fTimeDigiFirst << " ns to "
			      << fTimeDigiLast << " ns )";
  LOG(DEBUG) << FairLogger::endl;

  // After digis are created from signals the signals have to be removed 
  // Otherwise there is a huge memeory leak
  for (auto signal : SignalList) {
    delete (signal);
  }

}//----ReadAndRegister -------

//Convert Signal into the Digi with appropriate methods.

CbmMuchDigi* CbmMuchDigitizeGem::ConvertSignalToDigi(CbmMuchSignal* signal){

  //signal below threshold should be discarded.
  if(signal->GetCharge() < fQThreshold) return (NULL);
  Long64_t TimeStamp = signal->GetTimeStamp();

  //  Int_t TimeStamp = signal->GetTimeStamp(fQThreshold);
  //
  //  if (TimeStamp < 0) return (NULL);//entire signal is below threshold, no digi generation.

		
  CbmMuchDigi *digi = new CbmMuchDigi();
  digi->SetAddress(signal->GetAddress());
  //Charge in number of electrons, need to be converted in ADC value
	
  digi->SetAdc((signal->GetCharge())*fNADCChannels/fQMax);//Charge should be computed as per Electronics Response.
  digi->SetTime(TimeStamp);

  // Create new match object. If one uses the pointer from the CbmMuchSignal
  // it is not possible to remove the CbmMuchSignal
  CbmMatch* digiMatch = new CbmMatch(*signal->GetMatch());
  digi->SetMatch(digiMatch);
  // Update times of first and last digi
  fTimeDigiFirst = fNofDigis ? TMath::Min(fTimeDigiFirst, Double_t(TimeStamp)) : TimeStamp;
  fTimeDigiLast  = TMath::Max(fTimeDigiLast, Double_t(TimeStamp));

  //	digi->SetPileUp();
  //	digi->SetDiffEvent();
  return(digi);
}



// -------------------------------------------------------------------------
void CbmMuchDigitizeGem::Finish(){

  // --- In event-by-event mode, the analogue buffer should be empty.
  if ( fEventMode ) {
    if ( CbmMuchReadoutBuffer::Instance()->GetNData() ) {
      LOG(INFO) << fName << ": "
		<< CbmMuchReadoutBuffer::Instance()->GetNData()
		<< " signals in readout buffer" << FairLogger::endl;
      LOG(FATAL) << fName << ": Readout buffer is not empty at end of run "
		 << "in event-by-event mode!" << FairLogger::endl;
    } //? non-empty buffer
  } //? event-by-event mode

  else {  // time-based mode
    fTimer.Start();
    std::cout << std::endl;
    LOG(INFO) << GetName() << ": Finish run" << FairLogger::endl;
    Reset();
    LOG(INFO) << fName << ": "
	      << CbmMuchReadoutBuffer::Instance()->GetNData()
	      << " signals in readout buffer" << FairLogger::endl;
    ReadAndRegister(-1.); // -1 means process all data
    LOG(INFO) << setw(15) << GetName() << ": Finish, points "
	      << fNofPoints<< ", signals: " << fNofSignals
	      << ", digis: " << fNofDigis << ". Exec time " << setprecision(6)
	      << fTimer.RealTime() << " s." << FairLogger::endl;
    LOG(INFO) << fName << ": "
	      << CbmMuchReadoutBuffer::Instance()->GetNData()
	      << " signals in readout buffer" << FairLogger::endl;
    fTimer.Stop();
    fNofPointsTot  += fNofPoints;
    fNofSignalsTot += fNofSignals;
    fNofDigisTot   += fNofDigis;
    fTimeTot       += fTimer.RealTime();
  } //? time-based mode
	//noise->Draw();
  std::cout << std::endl;
  LOG(INFO) << "=====================================" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
  LOG(INFO) << "Events processed    : " << fNofEvents << FairLogger::endl;
  LOG(INFO) << "MuchPoint / event   : " << setprecision(1)
	    << fNofPointsTot / Double_t(fNofEvents)
	    << FairLogger::endl;
  LOG(INFO) << "MuchSignal / event  : "
	    << fNofSignalsTot / Double_t(fNofEvents)
    //<< " / " << fNofSignalsBTot / Double_t(fNofEvents)
	    << FairLogger::endl;
  LOG(INFO) << "MuchDigi / event    : "
	    << fNofDigisTot  / Double_t(fNofEvents) << FairLogger::endl;
  LOG(INFO) << "Digis per point     : " << setprecision(6)
	    << fNofDigisTot / fNofPointsTot << FairLogger::endl;
  LOG(INFO) << "Digis per signal    : "
	    << fNofDigisTot / fNofSignalsTot
	    << FairLogger::endl;
  LOG(INFO) << "Noise digis / event : " << fNofNoiseTot / Double_t(fNofEvents)
                              << FairLogger::endl;
  LOG(INFO) << "Noise fraction      : " << fNofNoiseTot / fNofDigisTot
      << FairLogger::endl;

  LOG(INFO) << "Real time per event : " << fTimeTot / Double_t(fNofEvents)
	    << " s" << FairLogger::endl;
  LOG(INFO) << "=====================================" << FairLogger::endl;


  //if (fDaq)	ReadAndRegister(-1.);
             
}
// -------------------------------------------------------------------------


// ------- Private method ExecAdvanced -------------------------------------
Bool_t CbmMuchDigitizeGem::ExecPoint(const CbmMuchPoint* point, Int_t iPoint) {
  
  //std::cout<<" start execution "<<iPoint<<std::endl;
  TVector3 v1,v2,dv;
  point->PositionIn(v1);
  point->PositionOut(v2);
  dv = v2-v1;

  Bool_t Status;
  Int_t detectorId = point->GetDetectorID();
  CbmMuchModule* module = fGeoScheme->GetModuleByDetId(detectorId);
  
  if (fAlgorithm==0){    // Simple digitization
    TVector3 v = 0.5*(v1+v2);
    CbmMuchPad* pad = 0;
    if (module->GetDetectorType()==1){
      CbmMuchModuleGemRectangular* module1 = (CbmMuchModuleGemRectangular*) module;
      pad = module1->GetPad(v[0],v[1]);
      if (pad) printf("x0=%f,y0=%f\n",pad->GetX(),pad->GetY());
    } else if (module->GetDetectorType()==3){
      CbmMuchModuleGemRadial* module3 = (CbmMuchModuleGemRadial*) module;
      pad = module3->GetPad(v[0],v[1]);
    }
    if (!pad) return kFALSE;
    AddCharge(pad,fQMax,iPoint,point->GetTime(),0);
    return kTRUE;
  }
  
  // Start of advanced digitization
  Int_t nElectrons = Int_t(GetNPrimaryElectronsPerCm(point)*dv.Mag());
  if (nElectrons<0) return kFALSE;
  
  Double_t time = point->GetTime();
  
  if (module->GetDetectorType()==1) {
    CbmMuchModuleGemRectangular* module1 = (CbmMuchModuleGemRectangular*) module;
    map<CbmMuchSector*,Int_t> firedSectors;
    for (Int_t i=0;i<nElectrons;i++) {
      Double_t aL = gRandom->Rndm();
      Double_t driftTime = (1-aL)*fTotalDriftTime;
      TVector3 ve = v1 + dv*aL;
      UInt_t ne   = GasGain();
      Double_t x  = ve.X();
      Double_t y  = ve.Y();
      Double_t x1 = x-fSpotRadius;
      Double_t x2 = x+fSpotRadius;
      Double_t y1 = y-fSpotRadius;
      Double_t y2 = y+fSpotRadius;
      Double_t s  = 4*fSpotRadius*fSpotRadius;
      firedSectors[module1->GetSector(x1,y1)]=0;
      firedSectors[module1->GetSector(x1,y2)]=0;
      firedSectors[module1->GetSector(x2,y1)]=0;
      firedSectors[module1->GetSector(x2,y2)]=0;
      for (map<CbmMuchSector*, Int_t>::iterator it = firedSectors.begin(); it!= firedSectors.end(); it++) {
        CbmMuchSector* sector  = (*it).first;
        if (!sector) continue;
        for (Int_t iPad=0;iPad<sector->GetNChannels();iPad++){
          CbmMuchPad* pad = sector->GetPadByChannelIndex(iPad);
          Double_t xp0 = pad->GetX();
          Double_t xpd = pad->GetDx()/2.;
          Double_t xp1 = xp0-xpd;
          Double_t xp2 = xp0+xpd;
          if (x1>xp2 || x2<xp1) continue;
          Double_t yp0 = pad->GetY();
          Double_t ypd = pad->GetDy()/2.;
          Double_t yp1 = yp0-ypd;
          Double_t yp2 = yp0+ypd;
          if (y1>yp2 || y2<yp1) continue;
          Double_t lx = x1>xp1 ? (x2<xp2 ? x2-x1 : xp2-x1) : x2-xp1;
          Double_t ly = y1>yp1 ? (y2<yp2 ? y2-y1 : yp2-y1) : y2-yp1;
          AddCharge(pad,UInt_t(ne*lx*ly/s),iPoint,time,driftTime);
        }
      } // loop fired sectors
      firedSectors.clear();
    }
  }
  
  if (module->GetDetectorType()==3) {
    fAddressCharge.clear();
    CbmMuchModuleGemRadial* module3 = (CbmMuchModuleGemRadial*) module;
    if(!module3){
      LOG(DEBUG) << GetName() << ": Not Processing MCPoint " << iPoint <<" because it is not on any GEM module."<< FairLogger::endl;
      return 1;
    }
    CbmMuchSectorRadial* sFirst = (CbmMuchSectorRadial*) module3->GetSectorByIndex(0);  //First sector
    if(!sFirst){
      LOG(DEBUG) << GetName() << ": Not Processing MCPoint " << iPoint <<" because it is on the module " << module3 <<"  but not the first sector. "<< sFirst << FairLogger::endl;
      return 1;
    }
    CbmMuchSectorRadial* sLast  = (CbmMuchSectorRadial*) module3->GetSectorByIndex(module3->GetNSectors()-1); //Last sector

    if(!sLast){
      LOG(DEBUG) << GetName() << ": Not Processing MCPoint " << iPoint <<" because it is not the last sector of module."<< module3 << FairLogger::endl;
      return 1;
    }
    Double_t rMin =sFirst->GetR1(); //Mimimum radius of the Sector//5
    Double_t rMax =sLast->GetR2();  //Maximum radius of the Sector//35

    //cout<<rMin<<"      Yeah      "<<rMax<<endl;
    // std::cout<<"min Rad "<<rMin<<"   max Rad  "<<rMax<<std::endl;
    //Calculating drifttime once for one track or one MCPoint, not for all the Primary Electrons generated during DriftGap.

    Double_t driftTime = -1;
    while(driftTime < 0) {

      Double_t aL   = gRandom->Gaus(0.5,0.133); //Generting random number for calculating Drift Time.

      driftTime = (1-aL)*fTotalDriftTime; 

    }

    for (Int_t i=0;i<nElectrons;i++) { //Looping over all the primary electrons
      Double_t RandomNumberForPrimaryElectronPosition   = gRandom->Rndm();
      TVector3 ve   = v1 + dv*RandomNumberForPrimaryElectronPosition;

      //------------------------Added by O. Singh 11.12.2017 for mCbm-------------------------
      Double_t r=0.0, phi=0.0;
      if(fFlag==1){//mCbm
	TVector3 nVe;
	Double_t XX = ve.X();
	Double_t YY = ve.Y();
	Double_t ZZ = ve.Z();

	Double_t tX=11.8;
	Double_t tY=72.0;
	Double_t rAng = 168.5;

	Double_t nXX = (XX-tX)*cos(rAng*TMath::DegToRad())+(YY-tY)*sin(rAng*TMath::DegToRad());//Transfotamation of MuChpoints to pads position
	Double_t nYY = -(XX-tX)*sin(rAng*TMath::DegToRad())+(YY-tY)*cos(rAng*TMath::DegToRad());
	Double_t nZZ = ZZ;

	nVe.SetX(nXX);
	nVe.SetY(nYY);
	nVe.SetZ(nZZ);
	r    = nVe.Perp(); 
	phi  = nVe.Phi();
      }else {//Cbm

	r    = ve.Perp(); 
	phi  = ve.Phi();
      }
      //--------------------------------------------------------------------------
      UInt_t ne     = GasGain(); //Number of secondary electrons
      Double_t r1   = r-fSpotRadius; 
      Double_t r2   = r+fSpotRadius;
      Double_t phi1 = phi-fSpotRadius/r;
      Double_t phi2 = phi+fSpotRadius/r;

      if (r1<rMin && r2>rMin) {//Adding charge to the pad which is on Lower Boundary 
        Status = AddCharge(sFirst,UInt_t(ne*(r2-rMin)/(r2-r1)),iPoint,time,driftTime,phi1,phi2);
	if(!Status)LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
        continue;
      }  
      if (r1<rMax && r2>rMax) {//Adding charge to the pad which is on Upper Boundary
        Status = AddCharge(sLast,UInt_t(ne*(rMax-r1)/(r2-r1)),iPoint,time,driftTime,phi1,phi2);
	if(!Status)LOG(DEBUG) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
	continue;
      }
      if (r1 <rMin && r2 <rMin) continue;
      if (r1 >rMax && r2 >rMax) continue;
      
      CbmMuchSectorRadial* s1 = module3->GetSectorByRadius(r1);
      CbmMuchSectorRadial* s2 = module3->GetSectorByRadius(r2);


      if (s1==s2) {Status = AddCharge(s1,ne,iPoint,time,driftTime,phi1,phi2); 
	if (!Status) LOG(DEBUG3) << GetName() << ": Processing MCPoint " << iPoint
				 <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
      }
      else {//Adding praportionate charge to both the pad 
	Status = AddCharge(s1,UInt_t(ne*(s1->GetR2()-r1)/(r2-r1)),iPoint,time,driftTime,phi1,phi2);
	if(!Status)LOG(DEBUG3) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
        Status = AddCharge(s2,UInt_t(ne*(r2-s2->GetR1())/(r2-r1)),iPoint,time,driftTime,phi1,phi2);
	if(!Status)LOG(DEBUG3) << GetName() << ": Processing MCPoint " << iPoint <<" in which Primary Electron : "<<i<< " not contributed charge. "<< FairLogger::endl;
      }
    }

    //Generate CbmMuchSignal for each entry of fAddressCharge and store in the CbmMuchReadoutBuffer
    if(!BufferSignals(iPoint,time,driftTime)) 
      LOG(DEBUG3) << GetName() << ": Processing MCPoint " << iPoint <<" nothing is buffered. "<< FairLogger::endl;
    fAddressCharge.clear();
    LOG(DEBUG1) << GetName() << ": fAddressCharge size is " << fAddressCharge.size() <<" Cleared fAddressCharge. "<< FairLogger::endl;
  }
  // std::cout<<" Execution completed for point # "<<iPoint<<std::endl;
  return kTRUE;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Int_t CbmMuchDigitizeGem::GasGain() {
  Double_t gasGain = -fMeanGasGain * TMath::Log(1 - gRandom->Rndm());
  if (gasGain < 0.) gasGain = 1e6;
  return (Int_t) gasGain;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMuchDigitizeGem::Sigma_n_e(Double_t Tkin, Double_t mass) {
  Double_t logT;
  if (mass < 0.1) {
    logT = log(Tkin * 0.511 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_e) logT = min_logT_e;
    return fSigma[0]->Eval(logT);
  } else if (mass >= 0.1 && mass < 0.2) {
    logT = log(Tkin * 105.658 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_mu) logT = min_logT_mu;
    return fSigma[1]->Eval(logT);
  } else {
    logT = log(Tkin * 938.272 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_p) logT = min_logT_p;
    return fSigma[2]->Eval(logT);
  }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t CbmMuchDigitizeGem::MPV_n_e(Double_t Tkin, Double_t mass) {
  Double_t logT;
  if (mass < 0.1) {
    logT = log(Tkin * 0.511 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_e) logT = min_logT_e;
    return fMPV[0]->Eval(logT);
  } else if (mass >= 0.1 && mass < 0.2) {
    logT = log(Tkin * 105.658 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_mu) logT = min_logT_mu;
    return fMPV[1]->Eval(logT);
  } else {
    logT = log(Tkin * 938.272 / mass);
    if (logT > 9.21034)    logT = 9.21034;
    if (logT < min_logT_p) logT = min_logT_p;
    return fMPV[2]->Eval(logT);
  }
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
Double_t CbmMuchDigitizeGem::GetNPrimaryElectronsPerCm(const CbmMuchPoint* point){
  Int_t trackId = point->GetTrackID();
  //  Int_t eventId = point->GetEventID();
  if (trackId < 0) return -1;


  /* Commented out on request of A. Senger from 22.01.2014
     if (fDaq && eventId!=FairRootManager::Instance()->GetInTree()->GetBranch("MCTrack")->GetReadEntry())
     FairRootManager::Instance()->GetInTree()->GetBranch("MCTrack")->GetEntry(eventId);
  */
  CbmMCTrack* mcTrack = (CbmMCTrack*) fMCTracks->At(trackId);
  
  if (!mcTrack) return -1;
  Int_t pdgCode = mcTrack->GetPdgCode();

  TParticlePDG *particle = TDatabasePDG::Instance()->GetParticle(pdgCode);
  // Assign proton hypothesis for unknown particles
  if (!particle) particle = TDatabasePDG::Instance()->GetParticle(2212);
  if (TMath::Abs(particle->Charge()) < 0.1) return -1;

  Double_t m = particle->Mass();
  TLorentzVector p;
  p.SetXYZM(point->GetPx(),point->GetPy(),point->GetPz(),m);
  Double_t Tkin = p.E()-m; // kinetic energy of the particle
  Double_t sigma = CbmMuchDigitizeGem::Sigma_n_e(Tkin,m); // sigma for Landau distribution
  Double_t mpv   = CbmMuchDigitizeGem::MPV_n_e(Tkin,m);   // most probable value for Landau distr.
  Double_t n = gRandom->Landau(mpv, sigma);
  while (n > 5e4) n = gRandom->Landau(mpv, sigma); // restrict Landau tail to increase performance
  return m<0.1 ? n/l_e : n/l_not_e;
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
Bool_t CbmMuchDigitizeGem::AddCharge(CbmMuchSectorRadial* s,UInt_t ne, Int_t /*iPoint*/, Double_t /*time*/, Double_t /*driftTime*/, 
				     Double_t phi1, Double_t phi2){
  CbmMuchPadRadial* pad1 = s->GetPadByPhi(phi1);
  if(!pad1) return kFALSE;
  //assert(pad1); has to check if any pad address is NULL
  CbmMuchPadRadial* pad2 = s->GetPadByPhi(phi2);
  if(!pad2) return kFALSE;
  //assert(pad2); has to check if any pad address is NULL
  if (pad1==pad2){
    UInt_t address = pad1->GetAddress();
    //Finding that if for the same address if already charge stored then add the charge.
    std::map<UInt_t,UInt_t>::iterator it=fAddressCharge.find(address);
    if(it!=fAddressCharge.end()) it->second=it->second+ne;
    else fAddressCharge.insert(std::pair<UInt_t,UInt_t>(address,ne));
    //    AddChargePerMC(pad1,ne,iPoint,time,driftTime);
  }
  else {
    Double_t phi = pad1 ? pad1->GetPhi2() : pad2 ? pad2->GetPhi1() : 0;
    UInt_t pad1_ne = UInt_t(ne*(phi-phi1)/(phi2-phi1));

    UInt_t address = pad1->GetAddress();
    //Finding that if for the same address if already charge stored then add the charge.
    std::map<UInt_t,UInt_t>::iterator it=fAddressCharge.find(address);
    if(it!=fAddressCharge.end()) it->second=it->second+pad1_ne;
    else fAddressCharge.insert (std::pair<UInt_t,UInt_t>(address,pad1_ne));
    //    AddChargePerMC(pad1,pad1_ne   ,iPoint,time,driftTime);
 

    // Getting some segmentation fault a
    address = pad2->GetAddress();
    //Finding that if for the same address if already charge stored then add the charge.
    it=fAddressCharge.find(address);
    if(it!=fAddressCharge.end()) it->second=it->second+ne-pad1_ne;
    else fAddressCharge.insert (std::pair<UInt_t,UInt_t>(address,ne-pad1_ne));
    // AddChargePerMC(pad2,ne-pad1_ne,iPoint,time,driftTime);
  }
  return kTRUE;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//Will remove this AddCharge, only used for simple and Rectangular Geometry.
void CbmMuchDigitizeGem::AddCharge(CbmMuchPad* pad, UInt_t charge, Int_t iPoint, Double_t time, Double_t driftTime){
  if (!pad) return;

  Long_t  AbsTime = fCurrentEventTime + time + driftTime;

  //Creating a new Signal, it will be deleted by CbmReadoutBuffer()
  CbmMuchSignal* signal = new CbmMuchSignal(pad->GetAddress(),AbsTime);
  //signal->SetTimeStart(AbsTime);
  //signal->SetTimeStop(AbsTime+fDeadTime);
  signal->SetCharge(charge);
  //signal->MakeSignalShape(charge,fgDeltaResponse);
  signal->AddNoise(fMeanNoise);
  UInt_t address = pad->GetAddress();
  //match->AddCharge(iPoint,charge,time+driftTime,fgDeltaResponse,time,eventNr,inputNr);
  CbmLink link(charge, iPoint, fCurrentEvent, fCurrentInput);
  //std::cout<<"Before AddLink"<< endl;
  (signal->GetMatch())->AddLink(link);
  //std::cout<<"After AddLink"<< endl;
  //Adding all these temporary signal into the CbmMuchReadoutBuffer
  CbmMuchReadoutBuffer::Instance()->Fill(address, signal);
  //Increasing number of signal by one.
  fNofSignals++;
  LOG(DEBUG4)<<" Registered the CbmMuchSignal into the CbmMuchReadoutBuffer "<<FairLogger::endl;

}//end of AddCharge 



//----------------------------------------------------------
Bool_t CbmMuchDigitizeGem::BufferSignals(Int_t iPoint,Double_t time, Double_t driftTime){

  if(!fAddressCharge.size()) { LOG(DEBUG2) << "Buffering MC Point " << iPoint
					   << " but fAddressCharge size is " << fAddressCharge.size() << "so nothing to Buffer for this MCPoint." << FairLogger::endl;
    return kFALSE;
  }
  UInt_t  AbsTime = fCurrentEventTime + time + driftTime;
  LOG(DEBUG2) << GetName() << ": Processing event " << fCurrentEvent
	      << " from input " << fCurrentInput << " at t = " << fCurrentEventTime
	      << " ns with " << fPoints->GetEntriesFast() << " MuchPoints "
	      << " and Number of pad hit is "<< fAddressCharge.size()<<"." 
	      << FairLogger::endl;
  //Loop on the fAddressCharge to store all the Signals into the CbmReadoutBuffer()
  //Generate one by one CbmMuchSignal from the fAddressCharge and store them into the CbmMuchReadoutBuffer.
  for(auto it=fAddressCharge.begin();it!=fAddressCharge.end();++it){
    UInt_t address = it->first;
    //Creating a new Signal, it will be deleted by CbmReadoutBuffer()
    CbmMuchSignal* signal = new CbmMuchSignal(address,AbsTime);
    //signal->SetTimeStart(AbsTime);
    //signal->SetTimeStop(AbsTime+fDeadTime);
    //signal->MakeSignalShape(it->second,fgDeltaResponse);
    signal->SetCharge(it->second);
    signal->AddNoise(fMeanNoise);
    CbmLink link(signal->GetCharge(), iPoint, fCurrentEvent, fCurrentInput);
    (signal->GetMatch())->AddLink(link);
    //Adding all these temporary signal into the CbmMuchReadoutBuffer
    CbmMuchReadoutBuffer::Instance()->Fill(address, signal);
    //Increasing number of signal by one.
    fNofSignals++;
    LOG(DEBUG3)<<" Registered the CbmMuchSignal into the CbmMuchReadoutBuffer "<<FairLogger::endl;
  }
	
  LOG(DEBUG2) << GetName() << ": For MC Point " << iPoint
	      << " buffered " << fAddressCharge.size() << " CbmMuchSignal into the CbmReadoutBuffer." << FairLogger::endl;
  return kTRUE;
}//end of BufferSignals
// -------------------------------------------------------------------------




// -----   Reset output arrays   -------------------------------------------
void CbmMuchDigitizeGem::ResetArrays() {
  if ( fDigis ) fDigis->Delete();
  if ( fDigiMatches ) fDigiMatches->Delete();
}
// -------------------------------------------------------------------------



// -----   Write to output   -----------------------------------------------
void CbmMuchDigitizeGem::WriteDigi(CbmDigi* digi) {

  CbmMuchDigi* muchDigi = dynamic_cast<CbmMuchDigi*>(digi);
  assert(digi);

  new ((*fDigis)[fDigis->GetEntriesFast()]) CbmMuchDigi(muchDigi);
  new ((*fDigiMatches)[fDigiMatches->GetEntriesFast()])
    CbmMuchDigiMatch((CbmMuchDigiMatch*)digi->GetMatch());

}
// -------------------------------------------------------------------------


ClassImp(CbmMuchDigitizeGem)







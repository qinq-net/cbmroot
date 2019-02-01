// Includes from TRD
#include "CbmTrdModuleSimR.h"
#include "CbmTrdDigitizer.h"
#include "CbmTrdAddress.h"
#include "CbmTrdPoint.h"
#include "CbmTrdDigi.h"
#include "CbmTrdParSetAsic.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParSpadic.h"
#include "CbmTrdRadiator.h"

// Includes from CbmRoot
//#include "CbmDaqBuffer.h"
#include "CbmMatch.h"

// Includes from FairRoot
#include <FairLogger.h>

// Includes from Root
#include <TRandom3.h>
#include <TMath.h>
#include <TVector3.h>
#include <TH1.h>
#include <TH2.h>
#include <TH2D.h>
#include <TH2I.h>
#include <TH1I.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TClonesArray.h>
#include <TGeoManager.h>
#include <TStopwatch.h>
#include <TFile.h>

// Includes from C++
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;
using std::cout;
using std::endl;
using std::vector;
using std::make_pair;
using std::max;
using std::map;
using std::fabs;
using std::pair;
using std::tuple;
using std::get;
using std::make_tuple;

//_________________________________________________________________________________
CbmTrdModuleSimR::CbmTrdModuleSimR(Int_t mod, Int_t ly, Int_t rot)
  : CbmTrdModuleSim(mod, ly, rot),
    fSigma_noise_keV(0.1),
    fRandom(NULL),
    fMinimumChargeTH(.5e-06),
    fCrosstalkLevel(0.01),
    fCurrentTime(-1.),
    fAddress(-1.),
    fPulseSwitch(false),
    fPrintPulse(false), //only for debugging
    fTimeShift(false),    //for pulsed mode
    fAddCrosstalk(false), //for pulsed mode
    fClipping(false),     //for pulsed mode
    fepoints(1),
    fCalibration(35./1.1107/0.8), 
    fTau(120.0),
    fTriggerSlope(7.0),
    fDistributionMode(1),

    fRecoMode(1),
    fEReco(90.623613),        // 2 < i < 8
    fAdcNoise(1),
    fLastEventTime(-1),
    fEventTime(-1),
    fLastTime(-1),
    fCollectTime(2048),    //in pulsed mode
    fnClusterConst(0),
    fnScanRowConst(0),
    fnScanColConst(0),
  
    frecostart(2),
    frecostop(8),
    fLastPoint(0),
    fLastEvent(0),
    nofElectrons(0),
    nofLatticeHits(0),
    nofPointsAboveThreshold(0),
    fAnalogBuffer(),
    fPulseBuffer(),
    fMultiBuffer(),
    fMCBuffer(),
    fTimeBuffer()

{
  if (fSigma_noise_keV > 0.0) fRandom = new TRandom3();
  if (!fPulseSwitch && CbmTrdDigitizer::IsTimeBased()) fCollectTime = 200;
  SetNameTitle(Form("TrdSimR%d", mod), "Simulator for rectangular read-out.");

  if(fPulseSwitch)   SetPulsePars(fRecoMode);
}


//_______________________________________________________________________________
void CbmTrdModuleSimR::AddDigi(Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t trigger)
{
  Double_t weighting = charge;
  if (CbmTrdDigitizer::UseWeightedDist()) {
    TVector3 padPos, padPosErr;
    fDigiPar->GetPadPosition(address, padPos, padPosErr);
    Double_t distance = sqrt(pow(fXYZ[0] - padPos[0],2) + pow(fXYZ[1] - padPos[1],2));
    weighting = 1. / distance;
  }

  
  //  std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it = fDigiMap.find(address);
  //  std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it;
  AddNoise(charge);
    

  Int_t col= CbmTrdAddress::GetColumnId(address);
  Int_t row= CbmTrdAddress::GetRowId(address);
  Int_t sec= CbmTrdAddress::GetSectorId(address);
  Int_t ncols= fDigiPar->GetNofColumns();
  Int_t channel(0);
  for(Int_t isec(0); isec<sec; isec++) channel += ncols*fDigiPar->GetNofRowsInSector(isec);
  channel += ncols * row + col;

  //printf("CbmTrdModuleSimR::AddDigi(%10d)=%3d [%d] col[%3d] row[%d] sec[%d]\n", address, channel, fModAddress, col, row, sec);
  //  if(charge*1e6 > 100.)   cout<<charge<<"   "<<fTimeBuffer[address]/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)<<endl;
  //  cout<<charge*1e6<<"   "<<fTimeBuffer[address]/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)<<endl;

  std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it = fDigiMap.find(address);
  if (it == fDigiMap.end()) { // Pixel not yet in map -> Add new pixel
    CbmMatch* digiMatch = new CbmMatch();
    digiMatch->AddLink(CbmLink(weighting, fPointId, fEventId, fInputId));
    fDigiMap[address] = make_pair(new CbmTrdDigi(channel, charge*1e6, ULong64_t(time/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)), 0, 0), digiMatch);
    //  std::cout<<"  digimap: " << fDigiMap.size()<<std::endl;
    it = fDigiMap.find(address);
    it->second.first->SetAddressModule(fModAddress);//module);
    if(trigger==1)    it->second.first->SetTriggerType(CbmTrdDigi::kSelf);
    if(trigger==2)    it->second.first->SetTriggerType(CbmTrdDigi::kNeighbor);
  }
  else{
    it->second.first->AddCharge(charge*1e6);
    it->second.second->AddLink(CbmLink(weighting, fPointId, fEventId, fInputId));
  }
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::ProcessBuffer(Int_t address){

  std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>           analog=fAnalogBuffer[address];
  std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>::         iterator it;
  CbmTrdDigi *digi = analog.begin()->first;
  CbmMatch *digiMatch = new CbmMatch();
  //printf("CbmTrdModuleSimR::ProcessBuffer(%10d)=%3d\n", address, digi->GetAddressChannel());

  Int_t trigger = 0;
  Float_t digicharge=0;
  Float_t digiTRcharge=0;
  for (it=analog.begin() ; it != analog.end(); it++) {
    digicharge+=it->first->GetCharge();
    digiMatch->AddLink(it->second->GetLink(0));
    //printf("  add charge[%f] trigger[%d]\n", it->first->GetCharge(), it->first->GetTriggerType());
  }
  digi->SetCharge(digicharge);
  digi->SetTriggerType(fAnalogBuffer[address][0].first->GetTriggerType());

  //  std::cout<<digicharge<<std::endl;
  
  //  if(analog.size()>1)  for (it=analog.begin()+1 ; it != analog.end(); it++) if(it->first) delete it->first;
  
  fDigiMap[address] = make_pair(digi, digiMatch);

  fAnalogBuffer.erase(address);
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::ProcessPulseBuffer(Int_t address, Bool_t FNcall, Bool_t MultiCall=false, Bool_t down=true, Bool_t up=true){

  map<Int_t, pair<vector<Int_t>,CbmMatch*>>::iterator iBuff = fPulseBuffer.find(address);
  map<Int_t, Double_t>::iterator                      tBuff = fTimeBuffer.find(address);

  if(iBuff==fPulseBuffer.end() || tBuff==fTimeBuffer.end()) return;
  Int_t trigger=CheckTrigger(fPulseBuffer[address].first);
  if(fPulseBuffer[address].first.size()<32) {return;}
  
  if(trigger==0 && !FNcall)  {return;}
  if(trigger==1 && FNcall)  {FNcall=false;}

  Int_t col= CbmTrdAddress::GetColumnId(address);
  Int_t row= CbmTrdAddress::GetRowId(address);
  Int_t sec= CbmTrdAddress::GetSectorId(address);
  Int_t ncols= fDigiPar->GetNofColumns();
  Int_t channel(0);
  for(Int_t isec(0); isec<sec; isec++) channel += ncols*fDigiPar->GetNofRowsInSector(isec);
  channel += ncols * row + col;


  Float_t digicharge=0;
  Int_t samplesum=0;
  for(Int_t i=0;i<fPulseBuffer[address].first.size();i++){
    if(i>=frecostart && i<=frecostop)   samplesum+=fPulseBuffer[address].first[i];
  }
  digicharge=samplesum/fEReco;

  //  if(digicharge > 1000.) {  fPulseBuffer.erase(address);  fTimeBuffer.erase(address);  return;}
  //  if(digicharge > 100.)   cout<<digicharge<<"   "<<fTimeBuffer[address]/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)<<endl;
  //  cout<<digicharge<<"   "<<fTimeBuffer[address]/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)<<endl;
  CbmTrdDigi* digi= new CbmTrdDigi(channel, digicharge, ULong64_t(fTimeBuffer[address]/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)), 0, 0);

  digi->SetAddressModule(fModAddress);//module);


  if(trigger==1){
    digi->SetTriggerType(CbmTrdDigi::kSelf);
  }
  if(trigger==0 && FNcall) {
    digi->SetTriggerType(CbmTrdDigi::kNeighbor);
  }
  if(trigger==1 && MultiCall){
    //    digi->SetTriggerType(CbmTrdDigi::kMulti);
    digi->SetTriggerType(CbmTrdDigi::kTrg1);
  }
  
  digi->SetMatch(fPulseBuffer[address].second);

  if(!FNcall && fPrintPulse)  cout<<"main call    charge: "<<digi->GetCharge()<<"   col : " << col<<"   lay : " << CbmTrdAddress::GetLayerId(address)<<"   trigger: " << trigger<<"    time: " << digi->GetTime()<<endl;
  if(FNcall && fPrintPulse)   cout<<"FN call     charge: "<<digi->GetCharge()<<"   col : " << col<<"   lay : " << CbmTrdAddress::GetLayerId(address)<< "   trigger: " << trigger<<"    time: " << digi->GetTime()<<endl;

  // if(!FNcall && MultiCall)  cout<<"main call    charge: "<<digi->GetCharge()<<"   col : " << col<<"   lay : " << CbmTrdAddress::GetLayerId(address)<<"   trigger: " << trigger<<"    time: " << digi->GetTime()<<endl;
  // if(FNcall && MultiCall)   cout<<"FN call     charge: "<<digi->GetCharge()<<"   col : " << col<<"   lay : " << CbmTrdAddress::GetLayerId(address)<< "   trigger: " << trigger<<"    time: " << digi->GetTime()<<endl;

  fDigiMap[address] = make_pair(digi, fPulseBuffer[address].second);

  fPulseBuffer.erase(address);  

  if(!FNcall && !MultiCall && trigger==1){
    if(down){
      Int_t FNaddress=0;
      if(col>=1)           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col-1);
      Double_t timediff = TMath::Abs(fTimeBuffer[address]-fTimeBuffer[FNaddress]);
      if(FNaddress!=0 && timediff < CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC))     ProcessPulseBuffer(FNaddress,true,false,true,false);
    }

    if(up){
      Int_t FNaddress=0;
      if(col<(ncols-1))    FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(address), sec, row, col+1);
      Double_t timediff = TMath::Abs(fTimeBuffer[address]-fTimeBuffer[FNaddress]);
      if(FNaddress!=0 && timediff < CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC))     ProcessPulseBuffer(FNaddress,true,false,false,true);
    }
  }

  
  if(!FNcall && MultiCall && trigger==1){
    if(down){
      Int_t FNaddress=0;
      if(col>=1)           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col-1);
      Double_t timediff = TMath::Abs(fTimeBuffer[address]-fTimeBuffer[FNaddress]);
      if(FNaddress!=0)     ProcessPulseBuffer(FNaddress,true,true,true,false);
    }

    if(up){
      Int_t FNaddress=0;
      if(col<(ncols-1))    FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(address), sec, row, col+1);
      Double_t timediff = TMath::Abs(fTimeBuffer[address]-fTimeBuffer[FNaddress]);
      if(FNaddress!=0)     ProcessPulseBuffer(FNaddress,true,true,false,true);
    }
  }

  fTimeBuffer.erase(address);
  
}


//_______________________________________________________________________________
void CbmTrdModuleSimR::AddDigitoBuffer(Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t trigger)
{
  Double_t weighting = charge;
  if (CbmTrdDigitizer::UseWeightedDist()) {
    TVector3 padPos, padPosErr;
    fDigiPar->GetPadPosition(address, padPos, padPosErr);
    Double_t distance = sqrt(pow(fXYZ[0] - padPos[0],2) + pow(fXYZ[1] - padPos[1],2));
    weighting = 1. / distance;
  }

  //compare times of the buffer content with the actual time and process the buffer if collecttime is over
  Bool_t eventtime=false;
  if(time>0.000) eventtime=true;
  if(eventtime)        CheckTime(address);
  
  AddNoise(charge);

  //fill digis in the buffer
  CbmMatch* digiMatch = new CbmMatch();
  digiMatch->AddLink(CbmLink(weighting, fPointId, fEventId, fInputId));

  Int_t col= CbmTrdAddress::GetColumnId(address);
  Int_t row= CbmTrdAddress::GetRowId(address);
  Int_t sec= CbmTrdAddress::GetSectorId(address);
  Int_t ncols= fDigiPar->GetNofColumns();
  Int_t channel(0);
  for(Int_t isec(0); isec<sec; isec++) channel += ncols*fDigiPar->GetNofRowsInSector(isec);
  channel += ncols * row + col;

  //  cout<<charge*1e6<<"   "<<fTimeBuffer[address]/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)<<endl;
  CbmTrdDigi* digi= new CbmTrdDigi(channel, charge*1e6, ULong64_t(time/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)), 0, 0);
  digi->SetAddressModule(fModAddress);//module);
  if(trigger==1)  digi->SetTriggerType(CbmTrdDigi::kSelf);
  if(trigger==2)  digi->SetTriggerType(CbmTrdDigi::kNeighbor);
  digi->SetMatch(digiMatch);
  //  printf("CbmTrdModuleSimR::AddDigitoBuffer(%10d)=%3d [%d] col[%3d] row[%d] sec[%d]\n", address, channel, fModAddress, col, row, sec);

  fAnalogBuffer[address].push_back(make_pair(digi, digiMatch));
  fTimeBuffer[address]=fCurrentTime;
  if(!eventtime)   ProcessBuffer(address);
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::AddDigitoPulseBuffer(Int_t address, Double_t reldrift,Double_t charge, Double_t chargeTR, Double_t time, Int_t trigger,Int_t epoints,Int_t ipoint,Bool_t finish=false)
{

  Double_t weighting = charge;
  if (CbmTrdDigitizer::UseWeightedDist()) {
    TVector3 padPos, padPosErr;
    fDigiPar->GetPadPosition(address, padPos, padPosErr);
    Double_t distance = sqrt(pow(fXYZ[0] - padPos[0],2) + pow(fXYZ[1] - padPos[1],2));
    weighting = 1. / distance;
  }


  vector<Int_t> oldpulse;  
  oldpulse = MakePulse(charge,oldpulse);
  Int_t oldtrigger  = CheckTrigger(oldpulse);
  vector<Int_t> comppulse;
  comppulse = MakePulse(charge,comppulse);
  Int_t comptrigger = CheckTrigger(comppulse);
  
  //  if(!CbmTrdDigitizer::IsTimeBased() && fPointId-fLastPoint!=0) {CheckBuffer(true);CleanUp(true);}

  if(reldrift==0. && CbmTrdDigitizer::IsTimeBased()){
  //  if(fPointId-fLastPoint!=0 && CbmTrdDigitizer::IsTimeBased()){
    Double_t timediff=fCurrentTime-fTimeBuffer[address];
    Double_t dt = TMath::Abs(fTimeBuffer[address] - time);
    CheckMulti(address,fPulseBuffer[address].first);
    fMultiBuffer.erase(address);
  }
  fMCBuffer.erase(address);
  
  if(CbmTrdDigitizer::IsTimeBased() && fPointId-fLastPoint!=0)    CheckTime(address);
  
  fMultiBuffer[address]=make_pair(fMultiBuffer[address].first+charge,0.);

  fMCBuffer[address].push_back(fPointId);
  fMCBuffer[address].push_back(fEventId);
  fMCBuffer[address].push_back(fInputId);
  vector<Int_t> pulse;
  if(fTimeBuffer[address]>0) {
    pulse=fPulseBuffer[address].first;
    if(pulse.size()<32) return;
    AddToPulse(address,charge,reldrift,pulse);
    fPulseBuffer[address].second->AddLink(CbmLink(weighting, fPointId, fEventId, fInputId));
  }
  else    {
    pulse=MakePulse(charge,pulse);
    fPulseBuffer[address].first=pulse;
    CbmMatch* digiMatch = new CbmMatch();
    digiMatch->AddLink(CbmLink(weighting, fPointId, fEventId, fInputId));
    fPulseBuffer[address].second=digiMatch;
    fTimeBuffer[address]=time;
  }

  if(!CbmTrdDigitizer::IsTimeBased() && finish) {CheckBuffer(true);CleanUp(true);}
}



std::vector<Int_t> CbmTrdModuleSimR::MakePulse(Double_t charge,vector<Int_t> pulse){


  Int_t sample[32];
  for(Int_t i=0;i<32;i++)  sample[i]=0;
  Double_t timeshift = fRandom->Uniform(0.,CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC));
  for(Int_t i=0;i<32;i++){
    Int_t noise = AddNoiseADC();
    
    //    Int_t cross = AddCrosstalk(address,i,sec,row,col,ncols);
    if(fTimeShift)     sample[i]=fCalibration*charge*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+timeshift)+noise;
    if(!fTimeShift)    sample[i]=fCalibration*charge*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC))+noise;
    if(sample[i] > 500 && fClipping) sample[i]=500;  //clipping
  }

  for(Int_t i=0;i<32;i++){
    pulse.push_back(sample[i]);
  }

  return pulse;

  
}

void CbmTrdModuleSimR::AddToPulse(Int_t address,Double_t charge,Double_t reldrift,vector<Int_t> pulse){

  Int_t comptrigger= CheckTrigger(pulse);
  Double_t timeshift = fRandom->Uniform(0.,CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC));  
  vector<Int_t> temppulse; 
  for(Int_t i=0;i<32;i++) temppulse.push_back(pulse[i]);
  Double_t dt=fCurrentTime-fTimeBuffer[address];
  Int_t startbin=(dt+reldrift)/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC);
  if(startbin>31 || dt<0.)   return;
  for(Int_t i=startbin;i<32;i++){
    Int_t addtime=i-startbin;
    pulse[i]+=fCalibration*charge*1e6*CalcResponse(addtime*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+timeshift);
    if(pulse[i] > 500 && fClipping) pulse[i]=500;  //clipping
  }

  vector<Int_t> newpulse; 
  for(Int_t i=0;i<32;i++){
    Int_t noise = AddNoiseADC();
    if(fTimeShift){
      Int_t sample = fCalibration*charge*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+timeshift)+noise;
      if(sample > 500 && fClipping) sample=500;  //clipping
      newpulse.push_back(sample);
    }
    if(!fTimeShift){
      Int_t sample = fCalibration*charge*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC))+noise;
      if(sample > 500 && fClipping) sample=500;  //clipping
      newpulse.push_back(sample);
    }
  }

  Int_t trigger = CheckTrigger(pulse);
  Int_t newtrigger = CheckTrigger(newpulse);
  if(comptrigger==0 && trigger==1){
    for(Int_t i=0;i<32;i++) {
      if(fTimeShift){
	if(startbin+i<32)    pulse[i]=temppulse[startbin+i]+fCalibration*charge*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+timeshift);
	else                 pulse[i]=fCalibration*charge*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+timeshift);
	if(pulse[i] > 500 && fClipping) pulse[i]=500;  //clipping
      }
      if(!fTimeShift){
	if(startbin+i<32)    pulse[i]=temppulse[startbin+i]+fCalibration*charge*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC));
	else                 pulse[i]=fCalibration*charge*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC));
	if(pulse[i] > 500 && fClipping) pulse[i]=500;  //clipping
      }
    }
    dt = TMath::Abs(fTimeBuffer[address] - fCurrentTime);

    fTimeBuffer[address]=fCurrentTime;
    
    Double_t weighting = charge;
    if (CbmTrdDigitizer::UseWeightedDist()) {
      TVector3 padPos, padPosErr;
      fDigiPar->GetPadPosition(address, padPos, padPosErr);
      Double_t distance = sqrt(pow(fXYZ[0] - padPos[0],2) + pow(fXYZ[1] - padPos[1],2));
      weighting = 1. / distance;
    }
    //    fPulseBuffer[address].second->AddLink(CbmLink(weighting, fPointId, fEventId, fInputId));
  }    

  if(trigger==2){
    fMultiBuffer[address].second=fCurrentTime;
  }

  for(Int_t i=0;i<32;i++){
  }

  
  fPulseBuffer[address].first=pulse;
  
}

void CbmTrdModuleSimR::CheckMulti(Int_t address,vector<Int_t> pulse){

  Int_t trigger = CheckTrigger(pulse);
  if(trigger==2){
  
    Float_t digicharge=0;
    Int_t samplesum=0;
    Int_t maincol= CbmTrdAddress::GetColumnId(address);
    Int_t row= CbmTrdAddress::GetRowId(address);
    Int_t sec= CbmTrdAddress::GetSectorId(address);
    Int_t shift=GetMultiBin(pulse);
    Int_t ncols= fDigiPar->GetNofColumns();
    Double_t timeshift = fRandom->Uniform(0.,CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC));      
    
    vector<Int_t> temppulse;
    map<Int_t,vector<Int_t>> templow;
    map<Int_t,vector<Int_t>> temphigh;
    
    temppulse=pulse;
    for(Int_t i=0;i<32;i++) {
      if(i >= shift) pulse[i]=0.;
    }
    fPulseBuffer[address].first=pulse;

    //    cout<<"multi time: " << fTimeBuffer[address]<<"   col: "<<maincol<<endl;
    Double_t dt=TMath::Abs(fMultiBuffer[address].second - fTimeBuffer[address]);

    
    Int_t col = maincol;
    Int_t FNaddress=0;
    Double_t FNshift=0;
    vector<Int_t> FNpulse=fPulseBuffer[address].first;
    if(col>=1)           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col-1);
    Int_t FNtrigger=1;
    
    while(FNtrigger==1 && FNaddress!=0){
      if(col>=1)           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col-1);
      else break;
      col--;
      FNpulse=fPulseBuffer[FNaddress].first;
      templow[FNaddress]=FNpulse;
      FNshift = (fTimeBuffer[address] - fTimeBuffer[FNaddress])/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+shift;
      for(Int_t i=0;i<32;i++) {
    	if(i >= FNshift) FNpulse[i]=0.;
      }
      FNtrigger=CheckTrigger(FNpulse);
      fPulseBuffer[FNaddress].first=FNpulse;
      if(col==0) break;

      if(FNtrigger == 1){
	dt=TMath::Abs(fMultiBuffer[FNaddress].second - fTimeBuffer[FNaddress]);
      }

      //      cout<<"col: "<<col<<"  "<< fTimeBuffer[FNaddress]<<"   FNaddress: " << FNaddress<<"  FNtrigger: "<< FNtrigger<<"  samples: " << fPulseBuffer[FNaddress].first.size()<<"   time: " << fTimeBuffer[FNaddress]<<endl;
    }
    
    col = maincol;
    FNaddress=0;
    if(col<(ncols-1))           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col+1);
    FNtrigger=1;
    
    while(FNtrigger==1 && FNaddress!=0){
      if(col<(ncols-1))         FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col+1);
      else break;
      col++;
      FNpulse=fPulseBuffer[FNaddress].first;
      temphigh[FNaddress]=FNpulse;
      FNshift = (fTimeBuffer[address] - fTimeBuffer[FNaddress])/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+shift;
      for(Int_t i=0;i<32;i++) {
    	if(i >= FNshift) FNpulse[i]=0.;
      }
      FNtrigger=CheckTrigger(FNpulse);
      fPulseBuffer[FNaddress].first=FNpulse;
      if(col==ncols-1) break;

      if(FNtrigger == 1){
	dt=TMath::Abs(fMultiBuffer[FNaddress].second - fTimeBuffer[FNaddress]);
      }

    }
    //    cout<<" call multi process"<<endl;
    ProcessPulseBuffer(address,false,true,true,true);
    //    cout<<endl;

    
    for(Int_t i=0;i<32;i++) {
      if(fTimeShift)    pulse[i]=fCalibration*fMultiBuffer[address].first*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+timeshift);
      if(!fTimeShift)   pulse[i]=fCalibration*fMultiBuffer[address].first*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC));
    }
    
    fTimeBuffer[address]=fMultiBuffer[address].second;
    fPulseBuffer[address].first=pulse;
        
    if(col<ncols-1)           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col+1);
    FNtrigger=1;
    
    while(FNtrigger==1  && FNaddress!=0){
      if(col<(ncols-1))           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col+1);
      else break;
      col++;
      FNpulse=fPulseBuffer[FNaddress].first;
      for(Int_t i=0;i<32;i++) {
	if(fTimeShift){
	  if(shift+i<32 &&  temphigh[FNaddress].size()>0)             FNpulse[i]=temphigh[FNaddress][i];
	  else                                                        FNpulse[i]=fCalibration*fMultiBuffer[FNaddress].first*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+timeshift);
	  if(FNpulse[i] > 500 && fClipping) FNpulse[i]=500;  //clipping
	}
	if(!fTimeShift){
	  if(shift+i<32 &&  temphigh[FNaddress].size()>0)             FNpulse[i]=temphigh[FNaddress][i];
	  else                                                        FNpulse[i]=fCalibration*fMultiBuffer[FNaddress].first*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC));
	  if(FNpulse[i] > 500 && fClipping) FNpulse[i]=500;  //clipping
	}
      }
      fPulseBuffer[FNaddress].first=FNpulse;	  
      FNtrigger=CheckTrigger(FNpulse);
      
      fTimeBuffer[FNaddress]=fMultiBuffer[address].second;
      fMultiBuffer.erase(FNaddress);
      if(col==ncols-1)  break;
    }

    if(col>=1)           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col-1);
    FNtrigger=1;
    
    while(FNtrigger==1 && FNaddress!=0){
      if(col>=1)           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col-1);
      else break;
      col--;
      FNpulse=fPulseBuffer[FNaddress].first;
      for(Int_t i=0;i<32;i++) {
	if(fTimeShift){
	  if(shift+i<32 &&  temphigh[FNaddress].size()>0)             FNpulse[i]=temphigh[FNaddress][i];
	  else                                                        FNpulse[i]=fCalibration*fMultiBuffer[FNaddress].first*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)+timeshift);
	  if(FNpulse[i] > 500 && fClipping) FNpulse[i]=500;  //clipping
	}
	if(!fTimeShift){
	  if(shift+i<32 &&  temphigh[FNaddress].size()>0)             FNpulse[i]=temphigh[FNaddress][i];
	  else                                                        FNpulse[i]=fCalibration*fMultiBuffer[FNaddress].first*1e6*CalcResponse(i*CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC));
	  if(FNpulse[i] > 500 && fClipping) FNpulse[i]=500;  //clipping
	}
      }
      fPulseBuffer[FNaddress].first=FNpulse;	  
      FNtrigger=CheckTrigger(FNpulse);
      
      fTimeBuffer[FNaddress]=fMultiBuffer[address].second;
      fMultiBuffer.erase(FNaddress);
      if(col==0) break;
    }

    CbmMatch* digiMatch = new CbmMatch();
    digiMatch->AddLink(CbmLink(fMultiBuffer[address].first, fMCBuffer[address][0], fMCBuffer[address][1],  fMCBuffer[address][2]));
    fPulseBuffer[address].second = digiMatch;
  }
}

Int_t CbmTrdModuleSimR::CheckTrigger(vector<Int_t> pulse){

  Int_t  slope=0;
  Bool_t trigger=false;
  Bool_t falling=false;
  Bool_t multihit=false;
  for(Int_t i=0;i<pulse.size();i++){
    if(i<pulse.size()-1)               slope=pulse[i+1]-pulse[i];
    if(slope>=fTriggerSlope && !trigger)           trigger=true;
    if(slope<0 &&  trigger)            falling=true;
    if(slope>=fTriggerSlope && trigger && falling) multihit=true;
  }

  if(!trigger && !multihit)  return 0;
  if(trigger && !multihit)   return 1;
  if(trigger && multihit)    return 2;

  return 0;
}

Int_t CbmTrdModuleSimR::GetMultiBin(vector<Int_t> pulse){

  Int_t  slope=0;
  Int_t  startbin=0;
  Bool_t trigger=false;
  Bool_t falling=false;
  Bool_t multihit=false;
  for(Int_t i=0;i<32;i++){
    if(i<31) slope=pulse[i+1]-pulse[i];
    if(slope>=fTriggerSlope && !trigger)           trigger=true;
    if(slope<0 &&  trigger)            falling=true;
    if(slope>=fTriggerSlope && trigger && falling) {multihit=true;startbin=i;}
  }

  return startbin;
}



//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::CalcPRF(Double_t x, Double_t W, Double_t h)
{
  Float_t K3 = 0.525; 
  Double_t SqrtK3 = sqrt(K3);

  return fabs(
	      -1. / (2. * atan(SqrtK3)) * (
					   atan(SqrtK3 * tanh(TMath::Pi() * (-2. + SqrtK3 ) * (W + 2.* x) / (8.* h) )) + 
					   atan(SqrtK3 * tanh(TMath::Pi() * (-2. + SqrtK3 ) * (W - 2.* x) / (8.* h) ))
					   )
	      );
}

//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::CalcResponse(Double_t t)
{

  return (t/fTau)*(t/fTau)*TMath::Exp(-(t/fTau));
  
}

//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::DistributeCharge(Double_t pointin[3],Double_t pointout[3], Double_t delta[3],Double_t pos[3],Int_t ipoints)
{
  if(fDistributionMode == 1){
    for (Int_t i = 0; i < 3; i++){
      pos[i] = pointin[i] +  (0.01) * delta[i] + 0.95 * delta[i]/fepoints * ipoints ;
    }
  }


  //in development
  Double_t lastpos[3] ={pointin[0],pointin[1],pointin[2]};
  Double_t dist_gas = TMath::Sqrt(delta[0]*delta[0]+delta[1]*delta[1]+delta[2]*delta[2]);
  if(fDistributionMode == 2){
    if(ipoints > 0)       for (Int_t i = 0; i < 3; i++) lastpos[i] = pos[i];
    //    cout<< "last x: "<<lastpos[0]<< " y: "<<lastpos[1]<< " z: "<<lastpos[2]<<endl;    
    Double_t roll = gRandom->Integer(100);
    Double_t s = GetStep(fRandom->Gaus(4,2),dist_gas,roll)/dist_gas; 
    //    cout<<" dist gas: " << dist_gas<<" roll: "<< roll << "  s: "<<s<<endl;
    if( (lastpos[0] + s * delta[0]) >  pointout[0] || (lastpos[1] + s * delta[1]) >  pointout[1] || (lastpos[2] + s * delta[2]) >  pointout[2]){
      for (Int_t i = 0; i < 3; i++){
	pos[i] = pointin[i] +  (0.95) * delta[i];
      }
      //      cout<< " x: "<<pos[0]<< " y: "<<pos[1]<< " z: "<<pos[2]<<endl;      
    }
    for (Int_t i = 0; i < 3; i++)    pos[i] = lastpos[i] +  s * delta[i];
    
  }

  return 0.;
}

//_______________________________________________________________________________
Bool_t CbmTrdModuleSimR::MakeDigi(CbmTrdPoint *point, Double_t time, Bool_t TR)
{
  if(!CbmTrdDigitizer::IsTimeBased()) fPulseSwitch = false;
  
  // calculate current physical time
  fCurrentTime =time + point->GetTime();//+ AddDrifttime(gRandom->Integer(240))*1000;  //convert to ns;
  fEventTime = time;
  
  const Double_t nClusterPerCm = 1.0;
  Double_t point_in[3] = {
    point->GetXIn(),
    point->GetYIn(),
    point->GetZIn()
  };
  Double_t point_out[3] = {
    point->GetXOut(),
    point->GetYOut(),
    point->GetZOut()
  };
  Double_t local_point_out[3];// exit point coordinates in local module cs
  Double_t local_point_in[3]; // entrace point coordinates in local module cs
  gGeoManager->cd(GetPath());
  gGeoManager->MasterToLocal(point_in,  local_point_in);
  gGeoManager->MasterToLocal(point_out, local_point_out);
  SetPositionMC(local_point_out);  

  
  // General processing on the MC point
  Double_t  ELoss(0.), ELossTR(0.),
    ELossdEdX(point->GetEnergyLoss());
  if (fRadiator && TR){
    nofElectrons++;
    if (fRadiator->LatticeHit(point)){  // electron has passed lattice grid (or frame material) befor reaching the gas volume -> TR-photons have been absorbed by the lattice grid
      nofLatticeHits++;
    } else if (point_out[2] >= point_in[2]){ //electron has passed the radiator
      TVector3 mom;
      point->Momentum(mom);
      ELossTR = fRadiator->GetTR(mom);
    }
  }
  ELoss = ELossTR + ELossdEdX;
  //if (ELoss > fMinimumChargeTH)  nofPointsAboveThreshold++;
  if (ELoss > 1.11e-6)  nofPointsAboveThreshold++;

  
   
  Double_t cluster_pos[3];   // cluster position in local module coordinate system
  Double_t cluster_delta[3]; // vector pointing in MC-track direction with length of one path slice within chamber volume to creat n cluster

  Double_t trackLength = 0;
  
  for (Int_t i = 0; i < 3; i++) {
    cluster_delta[i] = (local_point_out[i] - local_point_in[i]);
    trackLength += cluster_delta[i] * cluster_delta[i];
  }
  trackLength = TMath::Sqrt(trackLength);
  Int_t nCluster = trackLength / nClusterPerCm + 0.9;// Track length threshold of minimum 0.1cm track length in gas volume

  if (fnClusterConst > 0){
    nCluster = fnClusterConst;   // Set number of cluster to constant value
  }

  if (nCluster < 1){
    return kFALSE;
  }
  nCluster=1;
  
  for (Int_t i = 0; i < 3; i++){
    cluster_delta[i] /= Double_t(nCluster);
  }

  Double_t clusterELoss = ELoss / Double_t(nCluster);
  Double_t clusterELossTR = ELossTR / Double_t(nCluster);


  //to change the number of ionization points in the gas
  Int_t epoints=fepoints;

  if(epoints!=1){
    clusterELoss = ELoss / epoints;
    clusterELossTR = ELossTR / epoints;
  }

  if(!fPulseSwitch){
    for (Int_t ipoints = 0; ipoints < epoints; ipoints++){
      DistributeCharge(local_point_in,local_point_out,cluster_delta,cluster_pos,ipoints);

      //      cout<< " x: "<<cluster_pos[0]<< " y: "<<cluster_pos[1]<< " z: "<<cluster_pos[2]<<endl;
      
      if ( fDigiPar->GetSizeX() < fabs(cluster_pos[0]) || fDigiPar->GetSizeY() < fabs(cluster_pos[1])){
	printf("->    nC %i/%i x: %7.3f y: %7.3f \n",ipoints,nCluster-1,cluster_pos[0],cluster_pos[1]);
	for (Int_t i = 0; i < 3; i++)
	  printf("  (%i) | in: %7.3f + delta: %7.3f * cluster: %i/%i = cluster_pos: %7.3f out: %7.3f g_in:%f g_out:%f\n",
		 i,local_point_in[i],cluster_delta[i],ipoints,(Int_t)nCluster,cluster_pos[i],local_point_out[i],point_in[i],point_out[i]);
      }
    

      //    add noise digis between the actual and the last event
      if(CbmTrdDigitizer::AddNoise()){
	Int_t noiserate=fRandom->Uniform(0,3); //still in development
	Double_t simtime=fCurrentTime;
	for(Int_t ndigi=0; ndigi<noiserate; ndigi++){
	  NoiseTime(time);
	  //        ScanPadPlane(cluster_pos, fRandom->Gaus(0, fSigma_noise_keV * 1.E-6), 0,epoints,ipoints);
	}
	fCurrentTime=simtime;
      }

      //      cout<< " x: "<<cluster_pos[0]<< " y: "<<cluster_pos[1]<< " z: "<<cluster_pos[2]<<endl;
      
      //      fDigiPar->ProjectPositionToNextAnodeWire(cluster_pos);
      //      std::cout<<cluster_pos[0]<<std::endl;
      ScanPadPlane(cluster_pos, 0.,clusterELoss, clusterELossTR,epoints,ipoints);

    }
  }
  
  Double_t driftcomp=10000;
  Int_t    start=0;
  Double_t Ionizations[epoints][3];
  if(fPulseSwitch){
    //    cout<<" in x: " << local_point_in[0]<<" y: " << local_point_in[1]<<" z: " << local_point_in[2]<<endl;
    for (Int_t ipoints = 0; ipoints < epoints; ipoints++){
      DistributeCharge(local_point_in,local_point_out,cluster_delta,cluster_pos,ipoints);

      //      if(ipoints == epoints-1 && cluster_pos[2] > local_point_out[2])  cout<< " x: "<<cluster_pos[0]<< " y: "<<cluster_pos[1]<< " z: "<<cluster_pos[2]<<endl;
      
      for (Int_t i = 0; i < 3; i++)  Ionizations[ipoints][i]=cluster_pos[i];
      
      if ( fDigiPar->GetSizeX() < fabs(cluster_pos[0]) || fDigiPar->GetSizeY() < fabs(cluster_pos[1])){
	printf("->    nC %i/%i x: %7.3f y: %7.3f \n",ipoints,nCluster-1,cluster_pos[0],cluster_pos[1]);
	for (Int_t i = 0; i < 3; i++)  printf("  (%i) | in: %7.3f + delta: %7.3f * cluster: %i/%i = cluster_pos: %7.3f out: %7.3f g_in:%f g_out:%f\n",i,local_point_in[i],cluster_delta[i],ipoints,(Int_t)nCluster,cluster_pos[i],local_point_out[i],point_in[i],point_out[i]);
      }
    
      fDigiPar->ProjectPositionToNextAnodeWire(cluster_pos);
      Int_t relz=239-(cluster_pos[2]-local_point_in[2])/0.005;
      if(TMath::Abs(AddDrifttime(relz)) < driftcomp && TMath::Abs(AddDrifttime(relz)) > 0.)     {driftcomp=TMath::Abs(AddDrifttime(relz));start=ipoints;}
      //      cout<< " reldrift: " << driftcomp<< endl;
    }

    //    cout<<" out x: " << local_point_out[0]<<" y: " << local_point_out[1]<<" z: " << local_point_out[2]<<endl<<endl;;
    //    cout<<endl;
    
    //    DistributeCharge(local_point_in,local_point_out,cluster_delta,cluster_pos,start);
    for (Int_t i = 0; i < 3; i++) cluster_pos[i] = Ionizations[start][i];
    
    Int_t relz=239-(cluster_pos[2]-local_point_in[2])/0.005;
    Double_t reldrift=TMath::Abs(AddDrifttime(relz)-driftcomp)*1000;
    //    cout<<" relz: "<<relz<< " drift: " << reldrift<< "   adddrifttime: " << AddDrifttime(relz)<<endl;
    if(reldrift < 250.)  ScanPadPlane(cluster_pos, 0.,clusterELoss, clusterELossTR,epoints,start);
    if(fPrintPulse)  cout<<endl;

    
    for (Int_t ipoints = 0; ipoints < epoints; ipoints++){
      if(ipoints==start) continue;
      for (Int_t i = 0; i < 3; i++) cluster_pos[i] = Ionizations[ipoints][i];
      //      DistributeCharge(local_point_in,local_point_out,cluster_delta,cluster_pos,ipoints);

      relz=239-(cluster_pos[2]-local_point_in[2])/0.005;
      reldrift=TMath::Abs(AddDrifttime(relz)-driftcomp)*1000;
      //      cout<< " drift: " << reldrift<< "   adddrifttime: " << AddDrifttime(relz)<<endl;
      if(reldrift<250.)   ScanPadPlane(cluster_pos, reldrift,clusterELoss, clusterELossTR,epoints,ipoints);
      if(fPrintPulse)  cout<<endl;
    }
  }

  
  fLastEventTime = time;
  fLastPoint=fPointId;
  fLastEvent=fEventId;
  fLastTime=fCurrentTime;
  return true;
}


//_______________________________________________________________________________
void CbmTrdModuleSimR::ScanPadPlane(const Double_t* local_point, Double_t reldrift,Double_t clusterELoss, Double_t clusterELossTR,Int_t epoints,Int_t ipoint)
{
  Int_t sectorId(-1), columnId(-1), rowId(-1);
  fDigiPar->GetPadInfo( local_point, sectorId, columnId, rowId);
  if (sectorId < 0 && columnId < 0 && rowId < 0) {
    return;
  }
  else {
    for (Int_t i = 0; i < sectorId; i++) {
      rowId += fDigiPar->GetNofRowsInSector(i); // local -> global row
    }

    Double_t displacement_x(0), displacement_y(0);//mm
    Double_t h = fDigiPar->GetAnodeWireToPadPlaneDistance();
    Double_t W(fDigiPar->GetPadSizeX(sectorId)), H(fDigiPar->GetPadSizeY(sectorId));
    fDigiPar->TransformToLocalPad(local_point, displacement_x, displacement_y);

    Int_t maxCol(5/W+0.5), maxRow(6);//5/H+3);// 7 and 3 in orig. minimum 5 times 5 cm area has to be scanned
    if (fnScanRowConst > 0)
      maxRow = fnScanRowConst;
    if (fnScanColConst > 0)
      maxCol = fnScanColConst;

    Int_t startCol(columnId-maxCol/2), startRow(rowId-maxRow/2);
    Double_t sum = 0;
    Int_t secRow(-1), targCol(-1), targRow(-1), targSec(-1), address(-1),
      fnRow(fDigiPar->GetNofRows()), fnCol(fDigiPar->GetNofColumns());

    for (Int_t iRow = startRow; iRow <= rowId+maxRow/2; iRow++) {
      Int_t iCol=columnId;
      if (((iCol >= 0) && (iCol <= fnCol-1)) && ((iRow >= 0) && (iRow <= fnRow-1))){// real adress
        targSec = fDigiPar->GetSector(iRow, secRow);
	address = CbmTrdAddress::GetAddress(fLayerId, CbmTrdAddress::GetModuleId(fModAddress), targSec, secRow, iCol);
      }
      else {
	targRow = iRow;
	targCol = iCol;
	if (iCol < 0) {
	  targCol = 0;
	}
	else if (iCol > fnCol-1) {
	  targCol = fnCol-1;
	}
	if (iRow < 0) {
	  targRow = 0;
	}
	else if (iRow > fnRow-1) {
	  targRow = fnRow-1;
	}
    
	targSec = fDigiPar->GetSector(targRow, secRow);
	address = CbmTrdAddress::GetAddress(fLayerId, CbmTrdAddress::GetModuleId(fModAddress), targSec, secRow, targCol);
      }

      Bool_t print=false;
      
      //distribute the mc charge fraction over the channels wit the PRF
      Float_t chargeFraction = 0;
      Float_t ch = 0;
      Float_t tr = 0;

      // cout<<" prf half: " << CalcPRF(0 * W , W, h)<<endl;
      // cout<<" prf half -1 : " << CalcPRF(-1 * W , W, h)<<endl;
      // cout<<" prf half +1: " << CalcPRF(1 * W , W, h)<<endl;
      chargeFraction = CalcPRF((iCol - columnId) * W - displacement_x, W, h) * CalcPRF((iRow - rowId) * H - displacement_y, H, h);
      
      sum += chargeFraction;

      ch=    chargeFraction * clusterELoss;
      tr=    chargeFraction * clusterELossTR;
  
      Bool_t lowerend=false;
      Bool_t upperend=false;
      Int_t collow=1;
      Int_t colhigh=1;


      
      if(ch>=(fMinimumChargeTH/epoints)){
	if(!CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && !print)            AddDigi(address, ch, tr, fCurrentTime, Int_t (1));
	if(!CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && print)            {
	  AddDigi(address, ch, tr, fCurrentTime, Int_t (1));
	  cout<<" time: " << fCurrentTime<< "  col: "  << iCol<< "  row: " << iRow - rowId<< "  secrow: " << secRow<< "   charge: " << ch*1e6<< " 1 " << endl;
	}
	if(CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch)             AddDigitoBuffer(address, ch, tr, fCurrentTime, Int_t (1));
	if(fPulseSwitch)                                                AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (1),epoints,ipoint,false);
	if(fPulseSwitch && print)         {
	  AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (1),epoints,ipoint,false);
	  cout<<" time: " << fCurrentTime<< "  col: "  << iCol<< "  row: " << iRow - rowId<< "  secrow: " << secRow<< "   charge: " << ch*1e6<< " 1 " << endl;  
	}


	while(!lowerend){
	  if ((((iCol-collow) >= 0) && ((iCol-collow) <= fnCol-1)) && ((iRow >= 0) && (iRow <= fnRow-1))){// real adress
	    targSec = fDigiPar->GetSector(iRow, secRow);
	    address = CbmTrdAddress::GetAddress(fLayerId, CbmTrdAddress::GetModuleId(fModAddress), targSec, secRow, iCol-collow);

	  }
	  else {break;}
    
	  chargeFraction = CalcPRF(((iCol-collow) - columnId) * W - displacement_x, W, h) * CalcPRF((iRow - rowId) * H - displacement_y, H, h);
	  sum += chargeFraction;
	  ch=chargeFraction * clusterELoss;
	  tr=chargeFraction * clusterELossTR;
    
    
	  if(ch>=(fMinimumChargeTH/epoints) && !CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && !print)                   {AddDigi( address, ch, tr, fCurrentTime, Int_t (1));collow++;}
	  if(ch<(fMinimumChargeTH/epoints) && !CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && !print)                    {AddDigi( address, ch, tr, fCurrentTime, Int_t (2));lowerend=true;}
	  if(ch>=(fMinimumChargeTH/epoints) && !CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && print)                   {
	    cout<<" time: " << fCurrentTime<< "  col: "  << iCol-collow<< "  row: " << iRow - rowId<< "  secrow: " << secRow<<"   charge: " << ch*1e6<< " 1 " << endl;
	    AddDigi( address, ch, tr, fCurrentTime, Int_t (1));collow++;
	  }
	  if(ch<(fMinimumChargeTH/epoints) && !CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && print)                    {
	    cout<<" time: " << fCurrentTime<< "  col: "  << iCol-collow<< "  row: " << iRow - rowId<< "  secrow: " << secRow<< "   charge: " << ch*1e6<< " 0 " << endl;  
	    AddDigi( address, ch, tr, fCurrentTime, Int_t (2));lowerend=true;
	  }
	  if(ch>=(fMinimumChargeTH/epoints) && CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch)                    {AddDigitoBuffer( address, ch, tr, fCurrentTime, Int_t (1));collow++;}
	  if(ch<(fMinimumChargeTH/epoints) && CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch)                     {AddDigitoBuffer( address, ch, tr, fCurrentTime, Int_t (2));lowerend=true;}
	  if(ch>=(fMinimumChargeTH/epoints) && fPulseSwitch && !print)           {AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (1),epoints,ipoint,false);collow++;} 
	  if(ch<(fMinimumChargeTH/epoints) && fPulseSwitch && !print)            {AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (2),epoints,ipoint,false);lowerend=true;}

	  if(ch>=(fMinimumChargeTH/epoints) && fPulseSwitch && print)    {
	    cout<<" time: " << fCurrentTime<< "  col: "  << iCol-collow<< "  row: " << iRow - rowId<< "  secrow: " << secRow<<"   charge: " << ch*1e6<< " 1 " << endl;  
	    AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (1),epoints,ipoint,false);collow++;
	  }
	  if(ch<(fMinimumChargeTH/epoints) && fPulseSwitch && print)     {
	    cout<<" time: " << fCurrentTime<< "  col: "  << iCol-collow<< "  row: " << iRow - rowId<< "  secrow: " << secRow<< "   charge: " << ch*1e6<< " 0 " << endl;  
	    AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (2),epoints,ipoint,false);lowerend=true;
	  }
	}
        
	while(!upperend){

	  if ((((iCol+colhigh) >= 0) && ((iCol+colhigh) <= fnCol-1)) && ((iRow >= 0) && (iRow <= fnRow-1))){// real adress
	    targSec = fDigiPar->GetSector(iRow, secRow);
	    address = CbmTrdAddress::GetAddress(fLayerId, CbmTrdAddress::GetModuleId(fModAddress), targSec, secRow, iCol+colhigh);
	  }
	  else {break;}

    
	  chargeFraction = CalcPRF(((iCol+colhigh) - columnId) * W - displacement_x, W, h) * CalcPRF((iRow - rowId) * H - displacement_y, H, h);
	  sum += chargeFraction;
	  ch=chargeFraction * clusterELoss;
	  tr=chargeFraction * clusterELossTR;

	  if(ch>=(fMinimumChargeTH/epoints) && !CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && !print)                   {AddDigi( address, ch, tr, fCurrentTime, Int_t (1));colhigh++;}
	  if(ch<(fMinimumChargeTH/epoints) && !CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && !print)                    {AddDigi( address, ch, tr, fCurrentTime, Int_t (2));upperend=true;}
	  if(ch>=(fMinimumChargeTH/epoints) && !CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && print)                   {
	    cout<<" time: " << fCurrentTime<< "  col: "  << iCol+colhigh<< "  row: " << iRow - rowId<< "  secrow: " << secRow<< "   charge: " << ch*1e6<< " 1 " << endl;  
	    AddDigi( address, ch, tr, fCurrentTime, Int_t (1));colhigh++;
	  }
	  if(ch<(fMinimumChargeTH/epoints) && !CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch && print)                    {
	    cout<<" time: " << fCurrentTime<< "  col: "  << iCol+colhigh<< "  row: " << iRow - rowId<< "  secrow: " << secRow<< "   charge: " << ch*1e6<< " 0 " << endl;  
	    AddDigi( address, ch, tr, fCurrentTime, Int_t (2));upperend=true;
	  }
	  if(ch>=(fMinimumChargeTH/epoints) && CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch)                    {AddDigitoBuffer( address, ch, tr, fCurrentTime, Int_t (1));colhigh++;}
	  if(ch<(fMinimumChargeTH/epoints) && CbmTrdDigitizer::IsTimeBased() && !fPulseSwitch)                     {AddDigitoBuffer( address, ch, tr, fCurrentTime, Int_t (2));upperend=true;}
	  if(ch>=(fMinimumChargeTH/epoints) && fPulseSwitch && !print)           {AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (1),epoints,ipoint,false);colhigh++;}
	  if(ch<(fMinimumChargeTH/epoints) && fPulseSwitch && !print)            {
	    if(ipoint == epoints-1 && epoints > 1)    AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (2),epoints,ipoint,true);
	    if(ipoint != epoints-1 && epoints > 1)    AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (2),epoints,ipoint,false);
	    if(epoints == 1)                           AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (2),epoints,ipoint,true);
	    upperend=true;
	  }
	  
 	  if(ch>=(fMinimumChargeTH/epoints) && fPulseSwitch && print)    {
	    cout<<" time: " << fCurrentTime<< "  col: "  << iCol+colhigh<< "  row: " << iRow - rowId<< "  secrow: " << secRow<< "   charge: " << ch*1e6<< " 1 " << endl;  
	    AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (1),epoints,ipoint);colhigh++;
	  }
	  if(ch<(fMinimumChargeTH/epoints) && fPulseSwitch && print)     {
	    cout<<" time: " << fCurrentTime<< "  col: "  << iCol+colhigh<< "  row: " << iRow - rowId<< "  secrow: " << secRow<< "   charge: " << ch*1e6<< " 0 " << endl;  
	    if(ipoint == epoints-1 && epoints > 1)    AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (2),epoints,ipoint,true);
	    if(ipoint != epoints-1 && epoints > 1)    AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (2),epoints,ipoint,false);
	    if(epoints == 1)                           AddDigitoPulseBuffer(address, reldrift,ch, tr, fCurrentTime, Int_t (2),epoints,ipoint,true);
	    upperend=true;
	  }
	}

	if(print) cout<<endl;
	
      }//if charge > trigger
    }//for rows
  }
}


//_______________________________________________________________________________
void CbmTrdModuleSimR::SetAsicPar(CbmTrdParSetAsic *p)
{
/** Build local set of ASICs and perform initialization. Need a proper fDigiPar already defined.
 */  

  if(!fDigiPar){
    LOG(WARNING) << GetName() << "::SetAsicPar : No Digi params for module "<< fModAddress <<". Try calling first CbmTrdModSim::SetDigiPar."<< FairLogger::endl;
   return;
  }

  if(fAsicPar){
    LOG(WARNING) << GetName() << "::SetAsicPar : The list for module "<< fModAddress <<" already initialized."<< FairLogger::endl;
    return;    
  }
  fAsicPar = new CbmTrdParSetAsic();
  CbmTrdParSpadic *asic(NULL);
  
  Int_t iFebGroup = 0; // 1; 2;  // normal, super, ultimate
  Int_t gRow[3] = {  2, 2, 2 };  // re-ordering on the feb -> same mapping for normal and super
  Int_t gCol[3] = { 16, 8, 4 };  // re-ordering on the feb -> same mapping for normal and super
  Double_t xAsic = 0;  // x position of Asic
  Double_t yAsic = 0;  // y position of Asic

  Int_t rowId(0), isecId(0), irowId(0), iAsic(0);
  for (Int_t s = 0; s < fDigiPar->GetNofSectors(); s++) {    
    for (Int_t r = 0; r < fDigiPar->GetNofRowsInSector(s); r++){ 
      for (Int_t c = 0; c < fDigiPar->GetNofColumnsInSector(s); c++){ 
        // ultimate density 6 rows,  5 pads
        // super    density 4 rows,  8 pads
        // normal   density 2 rows, 16 pads
        if ((rowId % gRow[iFebGroup]) == 0){
          if ((c % gCol[iFebGroup]) == 0){
            xAsic =     c + gCol[iFebGroup] / 2.;
            yAsic =     r + gRow[iFebGroup] / 2.;

            Double_t local_point[3];
            Double_t padsizex = fDigiPar->GetPadSizeX(s);
            Double_t padsizey = fDigiPar->GetPadSizeY(s);

            // calculate position in sector coordinate system
            // with the origin in the lower left corner (looking upstream)
            local_point[0] = ((Int_t)(xAsic + 0.5) * padsizex);
            local_point[1] = ((Int_t)(yAsic + 0.5) * padsizey);
            
            // calculate position in module coordinate system
            // with the origin in the lower left corner (looking upstream)
            local_point[0] += fDigiPar->GetSectorBeginX(s);
            local_point[1] += fDigiPar->GetSectorBeginY(s);

            // local_point[i] must be >= 0 at this point      Double_t local_point[3];
            Double_t fDx(GetDx()), fDy(GetDy());
            asic = new CbmTrdParSpadic(iAsic, iFebGroup, local_point[0] - fDx, local_point[1] - fDy);
            fAsicPar->SetAsicPar(iAsic, asic);
            if (local_point[0] > 2*fDx){
              LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: asic position x=" << local_point[0] << " is out of bounds [0," << 2*fDx<< "]!" << FairLogger::endl;
              fDigiPar->Print("all");
            }
            if (local_point[1] > 2*fDy){     
              LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: asic position y=" << local_point[1] << " is out of bounds [0," << 2*fDy<< "]!" << FairLogger::endl;
              fDigiPar->Print("all");
            }
            for (Int_t ir = rowId; ir < rowId + gRow[iFebGroup]; ir++) {
              for (Int_t ic = c; ic < c + gCol[iFebGroup]; ic++) {
                if (ir >= fDigiPar->GetNofRows() )     LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: ir " << ir << " is out of bounds!" << FairLogger::endl;
                if (ic >= fDigiPar->GetNofColumns() )  LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: ic " << ic << " is out of bounds!" << FairLogger::endl;
                isecId = fDigiPar->GetSector((Int_t)ir, irowId);
                asic->SetChannelAddress(
                  CbmTrdAddress::GetAddress(fLayerId, CbmTrdAddress::GetModuleId(fModAddress), isecId, irowId, ic));
          //s, ir, ic));//new
                if (false)
                  printf("               M:%10i(%4i) s: %i  irowId: %4i  ic: %4i r: %4i c: %4i   address:%10i\n",fModAddress,
                    CbmTrdAddress::GetModuleId(fModAddress),
                    isecId, irowId, ic, r, c,
                    CbmTrdAddress::GetAddress(fLayerId, fModAddress, isecId, irowId, ic));
              } 
            } 
            iAsic++;  // next Asic
          }
        }
      }
      rowId++;
    }
  }

  // Self Test 
  for (Int_t s = 0; s < fDigiPar->GetNofSectors(); s++){
    const Int_t nRow = fDigiPar->GetNofRowsInSector(s);
    const Int_t nCol = fDigiPar->GetNofColumnsInSector(s);
    for (Int_t r = 0; r < nRow; r++){
      for (Int_t c = 0; c < nCol; c++){
        Int_t channelAddress = CbmTrdAddress::GetAddress(fLayerId, CbmTrdAddress::GetModuleId(fModAddress),s, r, c);
        if (fAsicPar->GetAsicAddress(channelAddress) == -1)
          LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: Channel address:" << channelAddress << " is not or multible initialized in module " << fModAddress << "(ID:" << CbmTrdAddress::GetModuleId(fModAddress) << ")" << "(s:" << s << ", r:" << r << ", c:" << c << ")" << FairLogger::endl;
      }
    }
  }
}


//_______________________________________________________________________________
void CbmTrdModuleSimR::SetNoiseLevel(Double_t sigma_keV)       
{ 
  fSigma_noise_keV = sigma_keV;
  if(!fRandom && fSigma_noise_keV > 0.) fRandom = new TRandom3();
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::SetDistributionPoints(Int_t points)       
{ 

  fepoints = points;
  
}

 //_______________________________________________________________________________
void CbmTrdModuleSimR::SetSpadicResponse(Double_t calibration, Double_t tau, Double_t calreco)     
{ 

  fCalibration = calibration;
  fTau = tau;
    //    fEReco(147.661038), //32 samples
    //    fEReco(90.623613),        // 2 < i < 8
  fEReco = calreco;        // 2 < i < 6
  
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::SetPulsePars(Int_t mode)     
{ 

  if(mode == 1){
    frecostart = 2;
    frecostop  = 8;
    fEReco     = 90.62361;
  }
    
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::SetPulseMode(Bool_t pulsed = true)     
{ 

  fPulseSwitch = pulsed;
  
}

 
//_______________________________________________________________________________
void CbmTrdModuleSimR::SetPadPlaneScanArea(Int_t column, Int_t row)
{
  if (row%2 == 0) row += 1;
  fnScanRowConst = row;
  if (column%2 == 0) column += 1;
  fnScanColConst = column;
}


//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::AddNoise(Double_t charge){
  
  if (fSigma_noise_keV > 0.0 && CbmTrdDigitizer::AddNoise() && !fPulseSwitch){
    Double_t noise = fRandom->Gaus(0, fSigma_noise_keV * 1.E-6);// keV->GeV // add only once per digi and event noise !!!
    charge += noise; // resulting charge can be < 0 -> possible  problems with position reconstruction
    return charge;
  }
  else return 0.;
}

//_______________________________________________________________________________
Int_t CbmTrdModuleSimR::AddNoiseADC(){
  
  if (fSigma_noise_keV > 0.0 && CbmTrdDigitizer::AddNoise() && fPulseSwitch){
    Int_t noise = fRandom->Gaus(0,fAdcNoise);
    return noise;
    // return 0;
  }
  else return 0;

}

//_______________________________________________________________________________
Int_t CbmTrdModuleSimR::AddCrosstalk(Double_t address,Int_t i, Int_t sec,Int_t row,Int_t col,Int_t ncols){

  Double_t cross=0.;
  if (fAddCrosstalk && fPulseSwitch){
      Int_t FNaddress=0;
      if(col>=1)           FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(fModAddress), sec, row, col-1);
      if(fPulseBuffer[address].first[i] > 0.)   cross += fPulseBuffer[address].first[i] * fCrosstalkLevel;
      
      FNaddress=0;
      if(col<(ncols-1))    FNaddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(address), sec, row, col+1);
      if(fPulseBuffer[address].first[i] > 0.)   cross += fPulseBuffer[address].first[i] * fCrosstalkLevel;
  }
    
  return cross;
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::CheckBuffer(Bool_t EB=false){

  
  std::map<Int_t,Double_t>::                                  iterator timeit;
  std::vector<Int_t> toBeErased;

  Bool_t done = false;

  while(!done){
    done = true;
    for(timeit=fTimeBuffer.begin(); timeit !=fTimeBuffer.end();timeit++){
      Int_t add=timeit->first;
      if(fCurrentTime < fTimeBuffer[add]) continue;
      Double_t dt=fCurrentTime-fTimeBuffer[add];
      if((dt<fCollectTime || dt==fCurrentTime) && !EB) continue;
//      if(!fPulseSwitch)    {ProcessBuffer(add);fTimeBuffer.erase(add);}
      if(!fPulseSwitch)    {ProcessBuffer(add);toBeErased.push_back(add);}
      if(fPulseSwitch)     {
	vector<Int_t> pulse;  
	pulse=fPulseBuffer[add].first;

	if(CheckTrigger(pulse)==1 && EB)          {ProcessPulseBuffer(add,false,false,true,true);break;}


	if(CheckTrigger(pulse)==1 && !EB)   {
	  ProcessPulseBuffer(add,false,false,true,true);
	  done = false;
	  break;
	}

	if(fPrintPulse)   cout<<endl;
      }
    }
  }

  for(auto& address: toBeErased) {
     fTimeBuffer.erase(address);
  }
}

//_______________________________________________________________________________
Int_t CbmTrdModuleSimR::FlushBuffer(ULong64_t time){

  
  if(!CbmTrdDigitizer::IsTimeBased()) return 0;
  if(CbmTrdDigitizer::IsTimeBased() && time == 0){  
    std::map<Int_t,Double_t>::                                  iterator timeit;
    Bool_t done = false;

    while(!done){
      done = true;
      for(timeit=fTimeBuffer.begin(); timeit !=fTimeBuffer.end();timeit++){
	Int_t add=timeit->first;
	if(!fPulseSwitch)    {ProcessBuffer(add);}
	if(fPulseSwitch)     {
	  vector<Int_t> pulse;  
	  pulse=fPulseBuffer[add].first;

	  if(CheckTrigger(pulse)==1)   {
	    ProcessPulseBuffer(add,false,false,true,true);
	    done = false;
	    break;
	  }

	  if(fPrintPulse)   cout<<endl;
	}
      }
    }
    return 0;
  }
  else return 0;
}


//_______________________________________________________________________________
void CbmTrdModuleSimR::CleanUp(Bool_t EB=false){
  
  std::map<Int_t,Double_t>::                                  iterator timeit;
  // clean up
  if(fPulseSwitch){
    for(timeit=fTimeBuffer.begin(); timeit !=fTimeBuffer.end();timeit++){
      Int_t add=timeit->first;
      Double_t dt=fCurrentTime-fTimeBuffer[add];
      if((dt<fCollectTime || dt==fCurrentTime) && !EB) continue;

      fPulseBuffer.erase(add);
      fTimeBuffer.erase(add);

    }
  }
  
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::CheckTime(Int_t address){

  //compare last entry in the actual channel with the current time
  std::map<Int_t,Double_t>::                                  iterator timeit;
  Double_t dt=fCurrentTime-fTimeBuffer[address];
  //  cout<<" dt: " << dt<<endl;
  Bool_t go=false;
  if(fCurrentTime>fTimeBuffer[address] && dt>0.0000000){
    if(dt>fCollectTime && dt!=fCurrentTime && !fPulseSwitch)        {ProcessBuffer(address);fTimeBuffer.erase(address);go=true;}
    //    if(dt>fCollectTime && dt!=fCurrentTime && fPulseSwitch)         {ProcessPulseBuffer(address,false,false);cout<<"    ------   " <<endl;go=true;}
    if(dt>fCollectTime && dt!=fCurrentTime && fPulseSwitch)         {
      //ProcessPulseBuffer(address,false,false,true,true);
      go=true;
      if(fPrintPulse)   cout<<endl;
    }
  }

  if(go && fPulseSwitch)    {CheckBuffer(false);CleanUp(false);}
  if(go && !fPulseSwitch)   CheckBuffer();
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::NoiseTime(ULong64_t eventTime)
{
  fCurrentTime=fRandom->Uniform(fLastEventTime, eventTime);
}

//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::AddDrifttime(Double_t x)
{
  Double_t drifttime[240]={0.11829,0.11689,0.11549,0.11409,0.11268,0.11128,0.10988,0.10847,0.10707,0.10567,
			   0.10427,0.10287,0.10146,0.10006,0.09866,0.09726,0.095859,0.094459,0.09306,0.091661,
			   0.090262,0.088865,0.087467,0.086072,0.084677,0.083283,0.08189,0.080499,0.07911,0.077722,
			   0.076337,0.074954,0.073574,0.072197,0.070824,0.069455,0.06809,0.066731,0.065379,0.064035,
			   0.0627,0.061376,0.060063,0.058764,0.05748,0.056214,0.054967,0.053743,0.052544,0.051374,
			   0.05024,0.049149,0.048106,0.047119,0.046195,0.045345,0.044583,0.043925,0.043403,0.043043,
			   0.042872,0.042932,0.043291,0.044029,0.045101,0.04658,0.048452,0.050507,0.052293,0.053458,
			   0.054021,0.053378,0.052139,0.53458,0.050477,0.048788,0.047383,0.046341,0.045631,0.045178,
			   0.045022,0.045112,0.045395,0.045833,0.046402,0.047084,0.047865,0.048726,0.049651,0.050629,
			   0.051654,0.052718,0.053816,0.054944,0.056098,0.057274,0.058469,0.059682,0.060909,0.062149,
			   0.0634,0.064661,0.06593,0.067207,0.06849,0.069778,0.07107,0.072367,0.073666,0.074968,
			   0.076272,0.077577,0.078883,0.080189,0.081495,0.082801,0.084104,0.085407,0.086707,0.088004,
			   0.089297,0.090585,0.091867,0.093142,0.094408,0.095664,0.096907,0.098134,0.099336,0.10051,
			   0.10164,0.10273,0.10375,0.10468,0.10548,0.10611,0.10649,0.10655,0.10608,0.10566,
			   0.1072,0.10799,0.10875,0.11103,0.11491,0.11819,0.12051,0.12211,0.12339,0.12449,
			   0.12556,0.12663,0.12771,0.12881,0.12995,0.13111,0.13229,0.13348,0.13468,0.13589,
			   0.13711,0.13834,0.13957,0.1408,0.14204,0.14328,0.14452,0.14576,0.14701,0.14825,
			   0.1495,0.15075,0.152,0.15325,0.1545,0.15576,0.15701,0.15826,0.15952,0.16077,
			   0.16203,0.16328,0.16454,0.16579,0.16705,0.1683,0.16956,0.17082,0.17207,0.17333,
			   0.17458,0.17584,0.1771,0.17835,0.17961,0.18087,0.18212,0.18338,0.18464,0.18589,
			   0.18715,0.18841,0.18966,0.19092,0.19218,0.19343,0.19469,0.19595,0.19721,0.19846,
			   0.19972,0.20098,0.20223,0.20349,0.20475,0.20601,0.20726,0.20852,0.20978,0.21103,
			   0.21229,0.21355,0.2148,0.21606,0.21732,0.21857,0.21983,0.22109,0.22234,0.2236,
			   0.22486,0.22612,0.22737,0.22863,0.22989,0.23114,0.2324,0.23366,0.23491,0.23617};

  
  Int_t xindex=0;

  return drifttime[Int_t(x)];
}


//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::GetStep(Double_t gamma,Double_t dist,Int_t roll)
{
  Double_t prob = 0.;
  //  Double_t dist_gas = TMath::Sqrt(delta[0]*delta[0]+delta[1]*delta[1]+delta[2]*delta[2])/10;
  Int_t steps = 1000;
  Double_t fgamma = 0.;

  std::pair<Double_t,Double_t> bethe[12] = {
    make_pair(1.5,1.5),make_pair(2,1.1),make_pair(3,1.025),make_pair(4,1),make_pair(10,1.1),
    make_pair(20,1.2),make_pair(100,1.5),make_pair(200,1.6),make_pair(300,1.65),make_pair(400,1.675),
    make_pair(500,1.7),make_pair(1000,1.725)
  };

  for(Int_t n=0;n<12;n++){
    //    cout<< n <<endl;
    //    continue;
    //    cout<<gamma<<"  n: "<<bethe[n].first<< "   n+1: " << bethe[n+1].first<<endl;
    if(gamma < bethe[0].first)             fgamma = bethe[0].second;
    if(n == 11)                            {fgamma = bethe[11].second;break;}

    if(gamma >= bethe[n].first && gamma <= bethe[n+1].first){
      Double_t dx = bethe[n+1].first - bethe[n].first;
      Double_t dy = bethe[n+1].second - bethe[n].second;
      Double_t slope = dy / dx ;
      //      cout<<"  dx: " << dx  << "  dy: " << dy << "   slope: " << slope<<endl;
      fgamma = (gamma - bethe[n].first) * slope + bethe[n].second;
      break;
    }
  }
  
  //  cout<< " gamma : " << gamma<< "  f: "<<fgamma<<endl;
  Double_t D = 1/(20.5 * fgamma);
  //  cout<< " gamma : " << gamma<< "  f: "<<fgamma<<"  D: " << D<<endl;
  for(Int_t i = 1; i<steps ; i++){
    Double_t s = (dist/steps)*i;
    prob = (1 - TMath::Exp(-s/D)) * 100;
    //    cout<<" s: " << s<<"  exp: " << TMath::Exp(-s/D)<<" prob: " << prob<<"  roll: "<< roll<<endl;
    if(prob >= roll)  return s;
  }
  

}


//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::GetBetheBloch(Double_t gamma)
{
  Int_t index = gamma;
  Double_t BB[10]={1.,1.,1.,1.,1.,1.,1.,1.,1.,1.};

  return BB[index];
}

ClassImp(CbmTrdModuleSimR)


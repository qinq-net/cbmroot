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
#include "CbmDaqBuffer.h"
#include "CbmMatch.h"

// Includes from FairRoot
#include <FairLogger.h>

// Includes from Root
#include <TRandom3.h>
#include <TMath.h>
#include <TVector3.h>
#include <TH2.h>
#include <TAxis.h>
#include <TClonesArray.h>
#include <TGeoManager.h>
#include <TStopwatch.h>

// Includes from C++
#include <iomanip>
#include <iostream>
#include <cmath>

using std::cout;
using std::endl;
using std::make_pair;
using std::max;
using std::fabs;
using std::pair;

//_________________________________________________________________________________
CbmTrdModuleSimR::CbmTrdModuleSimR(Int_t mod, Int_t ly, Int_t rot, 
                  Double_t x, Double_t y, Double_t z, 
                  Double_t dx, Double_t dy, Double_t dz)
  : CbmTrdModuleSim(mod, ly, rot, x, y, z, dx, dy, dz),
   fSigma_noise_keV(0.1),
   fNoise(NULL),
   fMinimumChargeTH(.5e-06),
   fCurrentTime(-1.),
   fAddress(-1.),
   fLastEventTime(-1),
   fCollectTime(250),
   fnClusterConst(0),
   fnScanRowConst(0),
   fnScanColConst(0),
   fdtlow(0),
   fdthigh(0),
   nofElectrons(0),
   nofLatticeHits(0),
   nofPointsAboveThreshold(0),
   fAnalogBuffer(),
   fTimeBuffer()
{
  if (fSigma_noise_keV > 0.0) fNoise = new TRandom3();
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

  std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it = fDigiMap.find(address);
  AddNoise(charge);
    
  CbmMatch* digiMatch = new CbmMatch();
  digiMatch->AddLink(CbmLink(weighting, fPointId, fEventId, fInputId));

  Int_t col= CbmTrdAddress::GetColumnId(address);
  Int_t row= CbmTrdAddress::GetRowId(address);
  Int_t module= CbmTrdAddress::GetModuleId(address);
  Int_t ncols= fDigiPar->GetNofColumns();
  Int_t channel = ncols * row + col;

  fDigiMap[address] = make_pair(new CbmTrdDigi(channel, charge*1e6, ULong64_t(time/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)), 0, 0), digiMatch);
  it = fDigiMap.find(address);

  it->second.first->SetAddressModule(module);
  if(trigger==1)    it->second.first->SetTriggerType(CbmTrdDigi::kSelf);
  if(trigger==2)    it->second.first->SetTriggerType(CbmTrdDigi::kNeighbor);
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
  Int_t module= CbmTrdAddress::GetModuleId(address);
  Int_t ncols= fDigiPar->GetNofColumns();
  Int_t channel = ncols * row + col;

  CbmTrdDigi* digi= new CbmTrdDigi(channel, charge*1e6, ULong64_t(time/CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)), 0, 0);
  digi->SetAddressModule(module);
  if(trigger==1)  digi->SetTriggerType(CbmTrdDigi::kSelf);
  if(trigger==2)  digi->SetTriggerType(CbmTrdDigi::kNeighbor);
  digi->SetMatch(digiMatch);
   fAnalogBuffer[address].push_back(make_pair(digi, digiMatch));
  fTimeBuffer[address]=fCurrentTime;
  if(!eventtime)   ProcessBuffer(address);
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
Bool_t CbmTrdModuleSimR::MakeDigi(CbmTrdPoint *point, Double_t time, Bool_t TR)  
{
  // calculate current physical time
  fCurrentTime =time + point->GetTime()+ AddDrifttime(gRandom->Integer(240))*1000;  //convert to ns;

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
  if (ELoss > fMinimumChargeTH)  nofPointsAboveThreshold++;

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
  Int_t epoints=1;

  if(epoints!=1){
    clusterELoss = ELoss / epoints;
    clusterELossTR = ELossTR / epoints;
  }
  
  for (Int_t ipoints = 0; ipoints < epoints; ipoints++){
    for (Int_t i = 0; i < 3; i++){
      if(epoints==1)                          cluster_pos[i] = local_point_in[i] +  (0.01) * cluster_delta[i];
      //  if(epoints==1)                    cluster_pos[i] = local_point_in[i] + (0.5 + iCluster) * cluster_delta[i];
      if(epoints==2 && ipoints==0)            cluster_pos[i] = local_point_in[i] +  (0.01) * cluster_delta[i];
      if(epoints==2 && ipoints==1)            cluster_pos[i] = local_point_out[i] - (0.01) * cluster_delta[i];
      if(epoints==3 && ipoints==0)            cluster_pos[i] = local_point_in[i] +  (0.1) * cluster_delta[i];
      if(epoints==3 && ipoints==1)            cluster_pos[i] = local_point_out[i] - (0.1) * cluster_delta[i];
      if(epoints==3 && ipoints==2)            cluster_pos[i] = local_point_in[i] +  (0.5) * cluster_delta[i];
      if(epoints>3)                           cluster_pos[i] = local_point_in[i] + (fNoise->Uniform(0.01,0.99)) * cluster_delta[i];

    }

    if ( fDigiPar->GetSizeX() < fabs(cluster_pos[0]) || fDigiPar->GetSizeY() < fabs(cluster_pos[1])){
      printf("->    nC %i/%i x: %7.3f y: %7.3f \n",ipoints,nCluster-1,cluster_pos[0],cluster_pos[1]);
      for (Int_t i = 0; i < 3; i++)
        printf("  (%i) | in: %7.3f + delta: %7.3f * cluster: %i/%i = cluster_pos: %7.3f out: %7.3f g_in:%f g_out:%f\n",
           i,local_point_in[i],cluster_delta[i],ipoints,(Int_t)nCluster,cluster_pos[i],local_point_out[i],point_in[i],point_out[i]);
    }
    

    //add noise digis between the actual and the last event
    if(CbmTrdDigitizer::AddNoise()){
      Int_t noiserate=fNoise->Uniform(0,3); //still in development
      Double_t simtime=fCurrentTime;
      for(Int_t ndigi=0; ndigi<noiserate; ndigi++){
    NoiseTime(time);
    ScanPadPlane(cluster_pos, fNoise->Gaus(0, fSigma_noise_keV * 1.E-6), 0,epoints,ipoints);
      }
      fCurrentTime=simtime;
    }

    
    fDigiPar->ProjectPositionToNextAnodeWire(cluster_pos);
    ScanPadPlane(cluster_pos, clusterELoss, clusterELossTR,epoints,ipoints);
  }
  
  return kTRUE;
}


//_______________________________________________________________________________
void CbmTrdModuleSimR::ScanPadPlane(const Double_t* local_point, Double_t clusterELoss, Double_t clusterELossTR,Int_t epoints,Int_t ipoints)
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
  address = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, iCol);
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
  address = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, targCol);
      }


      //distribute the mc charge fraction over the channels wit the PRF
      Float_t chargeFraction = 0;
      Float_t ch = 0;
      Float_t tr = 0;

      chargeFraction = CalcPRF((iCol - columnId) * W - displacement_x, W, h) * CalcPRF((iRow - rowId) * H - displacement_y, H, h);
      
      sum += chargeFraction;

      ch=    chargeFraction * clusterELoss;
      tr=    chargeFraction * clusterELossTR;
  
      Bool_t lowerend=false;
      Bool_t upperend=false;
      Int_t collow=1;
      Int_t colhigh=1;
      
      if(ch>=fMinimumChargeTH){
  if(!CbmTrdDigitizer::IsTimeBased())       AddDigi(address, ch, tr, fCurrentTime, Int_t (1));
  if(CbmTrdDigitizer::IsTimeBased())        AddDigitoBuffer(address, ch, tr, fCurrentTime, Int_t (1));

  while(!lowerend){
    if ((((iCol-collow) >= 0) && ((iCol-collow) <= fnCol-1)) && ((iRow >= 0) && (iRow <= fnRow-1))){// real adress
      targSec = fDigiPar->GetSector(iRow, secRow);
      address = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, iCol-collow);

    }
    else {break;}
    
    chargeFraction = CalcPRF(((iCol-collow) - columnId) * W - displacement_x, W, h) * CalcPRF((iRow - rowId) * H - displacement_y, H, h);
    sum += chargeFraction;
    ch=chargeFraction * clusterELoss;
    tr=chargeFraction * clusterELossTR;
    
    
    if(ch>=fMinimumChargeTH && !CbmTrdDigitizer::IsTimeBased())       {AddDigi( address, ch, tr, fCurrentTime, Int_t (1));collow++;}
    else if(ch<fMinimumChargeTH && !CbmTrdDigitizer::IsTimeBased())   {AddDigi( address, ch, tr, fCurrentTime, Int_t (2));lowerend=true;}
    else if(ch>=fMinimumChargeTH && CbmTrdDigitizer::IsTimeBased())   {AddDigitoBuffer( address, ch, tr, fCurrentTime, Int_t (1));collow++;}
    else if(ch<fMinimumChargeTH && CbmTrdDigitizer::IsTimeBased())    {AddDigitoBuffer( address, ch, tr, fCurrentTime, Int_t (2));lowerend=true;}
  }
        
  while(!upperend){

    if ((((iCol+colhigh) >= 0) && ((iCol+colhigh) <= fnCol-1)) && ((iRow >= 0) && (iRow <= fnRow-1))){// real adress
      targSec = fDigiPar->GetSector(iRow, secRow);
      address = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, iCol+colhigh);
    }
    else {break;}

    
    chargeFraction = CalcPRF(((iCol+colhigh) - columnId) * W - displacement_x, W, h) * CalcPRF((iRow - rowId) * H - displacement_y, H, h);
    sum += chargeFraction;
    ch=chargeFraction * clusterELoss;
    tr=chargeFraction * clusterELossTR;

          if(ch>=fMinimumChargeTH && !CbmTrdDigitizer::IsTimeBased())  {AddDigi( address, ch, tr, fCurrentTime, Int_t (1));colhigh++;}
    if(ch<fMinimumChargeTH && !CbmTrdDigitizer::IsTimeBased())   {AddDigi( address, ch, tr, fCurrentTime, Int_t (2));upperend=true;}
    if(ch>=fMinimumChargeTH && CbmTrdDigitizer::IsTimeBased())   {AddDigitoBuffer( address, ch, tr, fCurrentTime, Int_t (1));colhigh++;}
    if(ch<fMinimumChargeTH && CbmTrdDigitizer::IsTimeBased())    {AddDigitoBuffer( address, ch, tr, fCurrentTime, Int_t (2));upperend=true;}
  }

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
    LOG(WARNING) << GetName() << "::SetAsicPar : No Digi params for module "<< fModuleId <<". Try calling first CbmTrdModSim::SetDigiPar."<< FairLogger::endl;
    return;
  }

  if(fAsicPar){
    LOG(WARNING) << GetName() << "::SetAsicPar : The list for module "<< fModuleId <<" already initialized."<< FairLogger::endl;
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

            asic = new CbmTrdParSpadic(iAsic, iFebGroup, local_point[0] - fDx, local_point[1] - fDy);
            fAsicPar->SetAsicPar(iAsic, asic);
            if (local_point[0] > 2*fDx)     LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: asic position x=" << local_point[0] << " is out of bounds [0," << 2*fDx<< "]!" << FairLogger::endl;
            if (local_point[1] > 2*fDy)     LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: asic position y=" << local_point[1] << " is out of bounds [0," << 2*fDy<< "]!" << FairLogger::endl;

            for (Int_t ir = rowId; ir < rowId + gRow[iFebGroup]; ir++) {
              for (Int_t ic = c; ic < c + gCol[iFebGroup]; ic++) {
                if (ir >= fDigiPar->GetNofRows() )     LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: ir " << ir << " is out of bounds!" << FairLogger::endl;
                if (ic >= fDigiPar->GetNofColumns() )  LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: ic " << ic << " is out of bounds!" << FairLogger::endl;
                isecId = fDigiPar->GetSector((Int_t)ir, irowId);
                asic->SetChannelAddress(
                  CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(fModuleId),
                                  CbmTrdAddress::GetModuleId(fModuleId),
                                  isecId, irowId, ic));
          //s, ir, ic));//new
                if (false)
                  printf("               M:%10i(%4i) s: %i  irowId: %4i  ic: %4i r: %4i c: %4i   address:%10i\n",fModuleId,
                    CbmTrdAddress::GetModuleId(fModuleId),
                    isecId, irowId, ic, r, c,
                    CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(fModuleId),
                            CbmTrdAddress::GetModuleId(fModuleId),
                            isecId, irowId, ic));
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
        Int_t channelAddress = CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(fModuleId),
                CbmTrdAddress::GetModuleId(fModuleId), 
                s, r, c);
        if (fAsicPar->GetAsicAddress(channelAddress) == -1)
          LOG(ERROR) << "CbmTrdModuleSimR::SetAsicPar: Channel address:" << channelAddress << " is not or multible initialized in module " << fModuleId << "(ID:" << CbmTrdAddress::GetModuleId(fModuleId) << ")" << "(s:" << s << ", r:" << r << ", c:" << c << ")" << FairLogger::endl;
      }
    }
  }
}

//_______________________________________________________________________________
void CbmTrdModuleSimR::SetNoiseLevel(Double_t sigma_keV)       
{ 
  fSigma_noise_keV = sigma_keV;
  if(!fNoise && fSigma_noise_keV > 0.) fNoise = new TRandom3();
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
void CbmTrdModuleSimR::ProcessBuffer(Int_t address){

  Float_t digicharge=0;
  Float_t digiTRcharge=0;

  std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>           analog=fAnalogBuffer[address];
  std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>::         iterator it;

  for (it=analog.begin() ; it != analog.end(); it++) {
    digicharge+=it->first->GetCharge();
  }

  // Copy match object. Will be deleted in the digi destructor.
  // TODO: check if move of object is possible

  Int_t col= CbmTrdAddress::GetColumnId(address);
  Int_t row= CbmTrdAddress::GetRowId(address);
  Int_t module= CbmTrdAddress::GetModuleId(address);
  Int_t ncols= fDigiPar->GetNofColumns();
  Int_t channel = ncols * row + col;

  
  Int_t trigger = fAnalogBuffer[address][0].first->GetTriggerType();
  CbmMatch* digiMatch = new CbmMatch(*fAnalogBuffer[address][0].second);
  CbmTrdDigi* digi= new CbmTrdDigi(channel, digicharge, ULong64_t(fAnalogBuffer[address].back().first->GetTime()),trigger,0);
  digi->SetAddressModule(module);
  digi->SetTime(digi->GetTime());
  digi->SetMatch(digiMatch);
   CbmDaqBuffer::Instance()->InsertData(digi);
  //  fnoDigis++;

  fAnalogBuffer.erase(address);

}

//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::AddNoise(Double_t charge){
  
  if (fSigma_noise_keV > 0.0){
    Double_t noise = fNoise->Gaus(0, fSigma_noise_keV * 1.E-6);// keV->GeV // add only once per digi and event noise !!!
    charge += noise; // resulting charge can be < 0 -> possible  problems with position reconstruction
  }

  return charge;
}

//_______________________________________________________________________________
Double_t CbmTrdModuleSimR::CheckTime(Int_t address){


  //compare last entry in the actual channel with the current time
  std::map<Int_t,Double_t>::                                  iterator timeit;
  Double_t dt=fCurrentTime-fTimeBuffer[address];
  Bool_t go=false;
  if(fCurrentTime>fTimeBuffer[address] && dt>0.0000000){
    if(dt>fCollectTime && dt!=fCurrentTime)        {ProcessBuffer(address);fTimeBuffer.erase(address);}
    if(dt>3*fCollectTime && dt!=fCurrentTime)      go=true;
  }

  //also check other channels if collection time is far over in the actual channel
  if(go){
    for(timeit=fTimeBuffer.begin(); timeit !=fTimeBuffer.end();timeit++){
      Int_t add=timeit->first;
      dt=fCurrentTime-fTimeBuffer[add];
      if(dt>fCollectTime && dt!=fCurrentTime){
        ProcessBuffer(add);
        fTimeBuffer.erase(add);
      }
    }
  }

}

//_______________________________________________________________________________
void CbmTrdModuleSimR::NoiseTime(ULong64_t eventTime)
{
  fCurrentTime=fNoise->Uniform(fLastEventTime, eventTime);
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

ClassImp(CbmTrdModuleSimR)


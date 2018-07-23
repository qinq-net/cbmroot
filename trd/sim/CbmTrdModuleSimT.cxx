#include "CbmTrdModuleSimT.h"

#include "CbmTrdAddress.h"
#include "CbmTrdPoint.h"
#include "CbmTrdDigi.h"
#include "CbmTrdDigitizer.h"
#include "CbmTrdParSetAsic.h"
#include "CbmTrdParFasp.h"
#include "CbmTrdParModGas.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParModGain.h"
#include "CbmTrdRadiator.h"
#include "CbmTrdTrianglePRF.h"

#include "CbmMatch.h"
#include "CbmDaqBuffer.h"

#include <FairLogger.h>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoManager.h>

#include <iomanip>

#define VERBOSE 0

using std::cout;
using std::endl;
using std::make_pair;
using std::max;
using std::fabs;
using std::pair;
using namespace std;

//_________________________________________________________________________________
CbmTrdModuleSimT::CbmTrdModuleSimT(Int_t mod, Int_t ly, Int_t rot, 
                  Double_t x, Double_t y, Double_t z, 
                  Double_t dx, Double_t dy, Double_t dz, Bool_t FASP)
  : CbmTrdModuleSim(mod, ly, rot, x, y, z, dx, dy, dz)
  ,fTriangleBinning(NULL)
{
  SetNameTitle("CbmTrdModuleSimT", Form("Simulator for traingular read-out geometry Module %d", mod));
  SetAsic(FASP);
}

//_________________________________________________________________________________
CbmTrdModuleSimT::~CbmTrdModuleSimT()
{
  if(fTriangleBinning) delete fTriangleBinning;
}

//_________________________________________________________________________________
Bool_t CbmTrdModuleSimT::MakeDigi(CbmTrdPoint *point, Double_t time, Bool_t TR)  
{
/**  
  Steering routine for building digits out of the TRD hit for the triangular pad geometry. 
  1. Scan the amplification cells span by the track\n
  2. Build digits for each cell proportional with the projected energy on the cell\n
    2.1 Continuous distribution for ionisation\n
    2.2 Exponential decay for TR with constant \lambda
*/

  if(VERBOSE){ 
    printf("MakeDigi @ T[ns] = ev[%10.2f]+hit[%5.2f] ...\n", time, point->GetTime());
    point->Print("");
  }
  Double_t  gin[3] = {point->GetXIn(), point->GetYIn(), point->GetZIn()},
            gout[3]= {point->GetXOut(),point->GetYOut(),point->GetZOut()},
            lin[3],     // entrace point coordinates in local module cs
            lout[3],    // exit point coordinates in local module cs
            ain[3],     // entrace anode wire position
            aout[3],    // exit anode wire position
            dd[3];      // vec(lout)-vec(lin)
  gGeoManager->cd(GetPath());
  gGeoManager->MasterToLocal(gin,  lin);
  gGeoManager->MasterToLocal(gout, lout);
  SetPositionMC(lout);
  if(VERBOSE) printf("  pin[%7.4f %7.4f %7.4f] pout[%7.4f %7.4f %7.4f]\n", lin[0], lin[1], lin[2], lout[0], lout[1], lout[2]);
  
  // General processing on the MC point
  Double_t  ELoss(0.), ELossTR(0.),
            ELossdEdX(point->GetEnergyLoss());
  if (fRadiator && TR){
//    nofElectrons++;
    if (fRadiator->LatticeHit(point)){  // electron has passed lattice grid (or frame material) befor reaching the gas volume -> TR-photons have been absorbed by the lattice grid
//      nofLatticeHits++;
    } else if (gout[2] >= gin[2]){ //electron has passed the radiator
      TVector3 mom;
      point->Momentum(mom);
      ELossTR = fRadiator->GetTR(mom);
    }
  }
  ELoss = ELossTR + ELossdEdX;
  //if (ELoss > fMinimumChargeTH)  nofPointsAboveThreshold++;
  
  // compute track length in the gas volume
  Double_t trackLength(0.), txy(0.);
  for (Int_t i = 0; i < 3; i++) {
    dd[i] = (lout[i] - lin[i]);
    if(i==2) txy = trackLength;
    trackLength += dd[i] * dd[i];
  }
  if(trackLength) trackLength = TMath::Sqrt(trackLength);
  else{
    LOG(WARNING) << GetName() << "::MakeDigi: NULL track length for"
    " dEdx("<<std::setprecision(5)<<ELoss*1e6<<") keV " << FairLogger::endl;
    return kFALSE;
  }
  if(txy) txy = TMath::Sqrt(txy);
  else{
    LOG(WARNING) << GetName() << "::MakeDigi: NULL xy track length projection for"
    " dEdx("<<std::setprecision(5)<<ELoss*1e6<<") keV " << FairLogger::endl;
    return kFALSE;
  }
  // compute yz direction
  Double_t dzdy=dd[2]/dd[1];
  if(VERBOSE) printf("=> dzdy[%f]\n", dzdy);
  
  // get anode wire for the entrance point
  memcpy(ain, lin, 3*sizeof(Double_t));
  fDigiPar->ProjectPositionToNextAnodeWire(ain);
  // get anode wire for the exit point
  memcpy(aout, lout, 3*sizeof(Double_t));
  fDigiPar->ProjectPositionToNextAnodeWire(aout);

  // estimate no of anode wires hit by the track
  Double_t dw(fDigiPar->GetAnodeWireSpacing());
  Int_t ncls=TMath::Nint(TMath::Abs(aout[1]-ain[1])/dw+1.);
  if(VERBOSE) {
    printf("  Ncls[%d]\n", ncls);
    printf("  pin[%7.4f %7.4f %7.4f] pout[%7.4f %7.4f %7.4f]\n", lin[0], lin[1], lin[2], lout[0], lout[1], lout[2]);
    printf("  ain[%7.4f %7.4f %7.4f] aout[%7.4f %7.4f %7.4f]\n", ain[0], ain[1], ain[2], aout[0], aout[1], aout[2]);
  }
  
  // calculate track segmentation on the amplification cells distribution
  Int_t sgnx(1), sgny(1); 
  if(lout[0]<lin[0]) sgnx=-1;
  if(lout[1]<lin[1]) sgny=-1;
  Double_t  dy[] = {TMath::Min((ain[1]+0.5*sgny*dw-lin[1])*sgny, (lout[1]-lin[1])*sgny), 
                    TMath::Min((lout[1]-(aout[1]-0.5*sgny*dw))*sgny, (lout[1]-lin[1])*sgny)},
            dxw(TMath::Abs(dd[0]*dw/dd[1])),
            dx[] = {TMath::Abs(dy[0]*dd[0]/dd[1]), TMath::Abs(dy[1]*dd[0]/dd[1])};
  // check partition
  Double_t DX(dx[0]), DY(dy[0]);
  for(Int_t ic(1); ic<ncls-1; ic++){DX+=dxw; DY+=dw;}
  if(ncls>1){DX+=dx[1]; DY+=dy[1];}
  if(VERBOSE) {
    printf("  tdx[%7.4f] dx[%7.4f %7.4f %7.4f] check[%7.4f]\n"
           "  tdy[%7.4f] dy[%7.4f %7.4f %7.4f] check[%7.4f]\n", 
           dd[0], dx[0], dx[1], dxw, sgnx*DX, 
           dd[1], dy[0], dy[1], dw,  sgny*DY);
  }
  
  Double_t pos[3]={ain[0], ain[1], ain[2]},
           ldx(0.), ldy(0.),
           dxy(0.), e(0.), etr(0.),
           tdrift, y0=lin[1]-ain[1], z0=lin[2];
  for(Int_t icl(0); icl<ncls; icl++){
    if(!icl){ldx=dx[0]; ldy=dy[0];}  
    else if(icl==ncls-1){ldx=dx[1]; ldy=dy[1];}  
    else{ldx=dxw; ldy=dw;}
   
    dxy=ldx*ldx+ldy*ldy;
    if(dxy<=0){
      LOG(ERROR) << GetName()<<"::MakeDigi: NULL projected track length in cluster "<<icl
        <<" for track length[cm] ("<<std::setprecision(5)<<ldx<<", "<<std::setprecision(2)<<ldy<<")."
        " dEdx("<<std::setprecision(5)<<ELoss*1e6<<") keV " << FairLogger::endl;
      continue;
    }
    dxy=TMath::Sqrt(dxy);
    if(VERBOSE) printf("    %d ldx[%7.4f] ldy[%7.4f] xy[%7.4f] frac=%7.2f%%\n", icl, ldx, ldy, dxy, 1.e2*dxy/txy);

    Double_t  dEdx(dxy/txy),
              dEdxTR(dxy/txy),
              cELoss(ELoss*dEdx),         // continuos energy deposit 
              cELossTR(ELossTR*dEdxTR);   // TODO decay distribution of energy 
    e+=cELoss; etr+=cELossTR;

    if(VERBOSE) printf("      y0[%7.4f] z0[%7.4f] y1[%7.4f] z1[%7.4f]\n", y0, z0, y0+ldy*sgny, z0+dzdy*ldy*sgny);     
    tdrift = fChmbPar->ScanDriftTime(y0, z0, dzdy, ldy*sgny); y0+=ldy*sgny; z0+=dzdy*ldy*sgny;
    pos[0]+=0.5*ldx*sgnx; 
    if(VERBOSE) printf("      time_hit[ns]=%10.2f time_drift[ns]=%6.2f\n", time+point->GetTime(), tdrift);
    ScanPadPlane(pos, ldx, 1e6*cELoss, 1e6*cELossTR, time+point->GetTime()+tdrift);
    pos[0]+=0.5*ldx*sgnx; 
    pos[1]+=dw*sgny;
  }
  if(TMath::Abs(lout[0]-pos[0])>1.e-3){
    LOG(WARNING) << GetName() <<"::MakeDigi: Along wire coordinate error : x_sim="
      <<std::setprecision(5)<<lout[0]<<" x_calc="<<std::setprecision(5)<<pos[0] << FairLogger::endl;
  }
  if(TMath::Abs(ELoss-e)>1.e-3){
    LOG(WARNING) << GetName() <<"::MakeDigi: dEdx partition to anode wires error : E[keV] = "
      <<std::setprecision(5)<<ELoss*1e6<<" Sum(Ei)[keV]="<<std::setprecision(5)<<e*1e6 << FairLogger::endl;
  }
  if(etr>ELossTR){
    LOG(WARNING) << GetName() <<"::MakeDigi: TR energy partition to anode wires error : Etr[keV] = "
      <<std::setprecision(5)<<ELossTR*1e6<<" Sum(Ei)[keV]="<<std::setprecision(5)<<etr*1e6 << FairLogger::endl;
  }

  return kTRUE;
}

//_________________________________________________________________________________
Bool_t CbmTrdModuleSimT::ScanPadPlane(const Double_t* point, Double_t DX, Double_t ELoss, Double_t ELossTR, Double_t toff) 
{
/**  
  The hit is expressed in local chamber coordinates, localized as follows:
    - Along the wire in the middle of the track projection on the closest wire
    - Across the wire on the closest anode.
    
  The physical uncertainty along wires is given by the projection span (dx) and the energy from ionisation is proportional to the track projection length in the local chamber x-y plane. For the TR energy the proportionality to the total TR is given by the integral over the amplification cell span of a decay law with decay constant ...
  
  The class CbmTrdTrianglePRF is used to navigate the pad plane outward from the hit position until a threshold wrt to center is reached. The pad-row cross clusters are considered. Finally all digits are registered via AddDigi() function. 
*/
  if(VERBOSE) printf("    SPP : xy[%7.4f %7.4f] D[%7.4f] E[keV]=%7.4f time[ns]=%10.2f\n", point[0], point[1], DX, ELoss, toff);

  Int_t sec(-1), col(-1), row(-1);
  fDigiPar->GetPadInfo(point, sec, col, row);
  if (sec < 0 || col < 0 || row < 0) {
    LOG(ERROR) << "CbmTrdModuleSimT::ScanPadPlane: Hit to pad matching failed for ["
      <<std::setprecision(5)<<point[0]<<", "
      <<std::setprecision(5)<<point[1]<<", "
      <<std::setprecision(5)<<point[2]<<"]." << FairLogger::endl;
    return kFALSE;
  }
  for (Int_t is(0); is < sec; is++) row += fDigiPar->GetNofRowsInSector(is);
  if(VERBOSE) printf("      Found pad @ col[%d] row[%d]\n", col, row);
  
  Double_t dx, dy;
  fDigiPar->TransformToLocalPad(point, dx, dy);
  if(VERBOSE) printf("      pad x[%7.4f] y[%7.4f]\n", dx, dy);

  // build binning if called for the first time. Don't care about sector information as Bucharest has only 1 type of pads
  if(!fTriangleBinning) 
    fTriangleBinning=new CbmTrdTrianglePRF(fDigiPar->GetPadSizeX(1), fDigiPar->GetPadSizeY(1));
  if(!fTriangleBinning->SetOrigin(dx, dy)){
    LOG(WARNING) << "CbmTrdModuleSimT::ScanPadPlane: Hit outside integration limits ["
      <<std::setprecision(5)<<dx<<", "
      <<std::setprecision(5)<<dy<<"]." << FairLogger::endl;
    return kFALSE;
  }
  
  // set minimum threshold for all channels [keV]
  // TODO should be stored/computed in CbmTrdModule via triangular/FASP digi param
  Double_t epsilon=1.e-4;

  // local storage for digits on a maximum area of 5x3 columns for up[1]/down[0] pads
  const Int_t nc=2*CbmTrdTrianglePRF::NC+1;
  const Int_t nr=2*CbmTrdTrianglePRF::NR+1;
  Double_t array[nc][nr][2] = {{{0.}}}, prf(0.);
  Int_t colOff, rowOff, up, bx, by;
  
  // look right
  do{
    // check if there is any contribution on this bin column
    if(fTriangleBinning->GetChargeFraction()<=epsilon) break;
    
    // look up
    do{
      prf = fTriangleBinning->GetChargeFraction();
      fTriangleBinning->GetCurrentPad(colOff, rowOff, up);
      if( colOff<0 || colOff>=nc ||
          rowOff<0 || rowOff>=nr){
        printf("CbmTrdModuleSimT::ScanPadPlane: Bin outside mapped array : col[%d] row[%d]\n", colOff, rowOff);
        break;
      }
      //fTriangleBinning->GetCurrentBin(bx, by);
      //printf("      {ru} bin[%2d %2d] c[%d] r[%d] u[%2d] PRF[%f]\n", bx, by, colOff, rowOff, up, prf);
      if(up) array[colOff][rowOff][(up>0?1:0)] += prf;
      else{ 
        array[colOff][rowOff][0] += 0.5*prf;
        array[colOff][rowOff][1] += 0.5*prf;
      }
    } while(fTriangleBinning->NextBinY() && prf>=epsilon);
    fTriangleBinning->GoToOriginY();
      //printf("\n");
    
    // skip bin @ y0 which was calculated before
    if(!fTriangleBinning->PrevBinY()) continue;
    
    // look down
    do{
      prf = fTriangleBinning->GetChargeFraction();
      fTriangleBinning->GetCurrentPad(colOff, rowOff, up);
      if( colOff<0 || colOff>=nc ||
          rowOff<0 || rowOff>=nr){
        printf("CbmTrdModuleSimT::ScanPadPlaneTriangleAB: Bin outside mapped array : col[%d] row[%d]\n", colOff, rowOff);
        break;
      }
      //fTriangleBinning->GetCurrentBin(bx, by);
      //printf("      {rd} bin[%2d %2d] c[%d] r[%d] u[%2d] PRF[%f]\n", bx, by, colOff, rowOff, up, prf);
      if(up) array[colOff][rowOff][(up>0?1:0)] += prf;
      else{ 
        array[colOff][rowOff][0] += 0.5*prf;
        array[colOff][rowOff][1] += 0.5*prf;
      }
    } while(fTriangleBinning->PrevBinY() && prf>=epsilon);
    fTriangleBinning->GoToOriginY();
      //printf("\n");
    
  } while(fTriangleBinning->NextBinX());    
  fTriangleBinning->GoToOriginX();
  

  if(fTriangleBinning->PrevBinX()){ // skip bin @ x0 which was calculated before
    // look left
    do{
      // check if there is any contribution on this bin column
      if(fTriangleBinning->GetChargeFraction()<=epsilon) break;

      // look up
      do{
        prf = fTriangleBinning->GetChargeFraction();
        fTriangleBinning->GetCurrentPad(colOff, rowOff, up);
        if( colOff<0 || colOff>=nc ||
            rowOff<0 || rowOff>=nr){
          printf("CbmTrdModuleSimT::ScanPadPlane: Bin outside mapped array : col[%d] row[%d]\n", colOff, rowOff);
          break;
        }
        //fTriangleBinning->GetCurrentBin(bx, by);
        //printf("      {lu} bin[%2d %2d] c[%d] r[%d] u[%2d] PRF[%f]\n", bx, by, colOff, rowOff, up, prf);
        if(up) array[colOff][rowOff][(up>0?1:0)] += prf;
        else{ 
          array[colOff][rowOff][0] += 0.5*prf;
          array[colOff][rowOff][1] += 0.5*prf;
        }
      } while(fTriangleBinning->NextBinY() && prf>=epsilon);
      fTriangleBinning->GoToOriginY();

      // skip bin @ y0 which was calculated before
      if(!fTriangleBinning->PrevBinY()) continue;
      
      // look down
      do{
        prf = fTriangleBinning->GetChargeFraction();
        fTriangleBinning->GetCurrentPad(colOff, rowOff, up);
        if( colOff<0 || colOff>=nc ||
            rowOff<0 || rowOff>=nr){
          printf("CbmTrdModuleSimT::ScanPadPlane: Bin outside mapped array : col[%d] row[%d]\n", colOff, rowOff);
          break;
        }
        //fTriangleBinning->GetCurrentBin(bx, by);
        //printf("      {ld} bin[%2d %2d] c[%d] r[%d] u[%2d] PRF[%f]\n", bx, by, colOff, rowOff, up, prf);
        if(up) array[colOff][rowOff][(up>0?1:0)] += prf;
        else{ 
          array[colOff][rowOff][0] += 0.5*prf;
          array[colOff][rowOff][1] += 0.5*prf;
        }
      } while(fTriangleBinning->PrevBinY() && prf>=epsilon);
      fTriangleBinning->GoToOriginY();
      //printf("\n");
      
    } while(fTriangleBinning->PrevBinX());    
  }
  fTriangleBinning->GoToOriginX();
      //printf("\n");
  if(VERBOSE) {
    printf("        "); for(Int_t ic(0); ic<5; ic++) printf("%7d[u/d]  ", ic); printf("\n");
    for(Int_t ir(nr); ir--; ){
      printf("      r[%d] ", ir);
      for(Int_t ic(0); ic<nc; ic++) printf("%6.4f/%6.4f ", array[ic][ir][1], array[ic][ir][0]);
      printf("\n");
    }
  }
  
  // pair pads and convert to ADC
  // calibration ADC -> keV based on 55Fe measurements as presented @
  //https://indico.gsi.de/event/4760/session/6/contribution/58/material/slides/0.pdf on slide 14
  // TODO should be stored/computed in CbmTrdParModGain
  const Float_t ECalib[]={-528./380., 1./380.};
  Double_t Emeasure(0.);
  for(Int_t ir(nr); ir--; ){
    for(Int_t ic(0); ic<nc; ic++){
      for(Int_t iup(0); iup<2; iup++){
        if(array[ic][ir][iup]<=epsilon) continue;
        array[ic][ir][iup]*=ELoss/fTriangleBinning->Norm();
        Emeasure+=array[ic][ir][iup];
        array[ic][ir][iup] = (array[ic][ir][iup]-ECalib[0])*380.;
      }
      if(ic>0) array[ic-1][ir][0]+=array[ic][ir][1];  // add top pad to previous tilt pair
      array[ic][ir][1] += array[ic][ir][0];           // add bottom pad to current rect pair
    }
  }
  if(VERBOSE) {
    printf("      E_sim[keV]=%6.4f E_digi[keV]=%6.4f\n", ELoss, Emeasure);
    printf("        "); for(Int_t ic(0); ic<5; ic++) printf("%7d[R/T]  ", ic); printf("\n");
    for(Int_t ir(nr); ir--; ){
      printf("      r[%d] ", ir);
      for(Int_t ic(0); ic<nc; ic++) printf("%6.1f/%6.1f ", array[ic][ir][1], array[ic][ir][0]);
      printf("\n");
    }
  }
  // register digitisation to container
  Int_t address(0);
  for(Int_t ir(0); ir<nr; ir++){
    for(Int_t ic(0); ic<nc; ic++){  
      // check if column is inside pad-plane        
      Int_t wcol(col+ic-CbmTrdTrianglePRF::NC);
      if( wcol<0 || wcol>=fDigiPar->GetNofColumns() ) continue;

      // check if row is inside pad-plane
      Int_t wrow(row+ir-CbmTrdTrianglePRF::NR);
      if( wrow<0 || wrow>=fDigiPar->GetNofRows()) continue;

      // check if there are data available
      Double_t dch[2]={0.}; Bool_t kCOL(kFALSE);
      for(Int_t iup(0); iup<2; iup++){
        if(array[ic][ir][iup]<=epsilon) continue; 
        dch[iup]=array[ic][ir][iup];
        kCOL = kTRUE;
      }
      if(!kCOL) continue;

      // compute column address
      //Int_t srow, isec=fDigiPar->GetSectorRow(wrow, srow);
      //printf("AB : ly(%d) modId(%d) sec[%d] row[%d] col[%d]\n", fLayerId, CbmTrdAddress::GetModuleId(fModuleId), isec, srow, wcol);
      address = GetPadAddress(wrow, wcol);//CbmTrdAddress::GetAddress(fLayerId, CbmTrdAddress::GetModuleId(fModuleId), isec, srow, wcol);
      
      // choose time based simulation or ebye
      if(!kTRUE/*fStream*/){
//         AddDigi(fMCPointId, address,dch[0], dch[0] * ELossTR/ELoss, fEventTime +toff, 0);
//         AddDigi(fMCPointId, address,dch[1], dch[1] * ELossTR/ELoss, fEventTime +toff, 1);
        if(VERBOSE){
          //std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it = fBuffer.find(address);
          //cout<<(it->second.first)->ToString();
        }
      } else {
        AddDigi(address, &dch[0], toff, ELossTR/ELoss);
      }
    }
  }
  return kTRUE;
}

//_______________________________________________________________________________________________
void CbmTrdModuleSimT::AddDigi(Int_t address, Double_t *charge, Double_t time, Double_t fTR)
{
/**
 * Adding triangular digits to time slice buffer
 */
  
  // get ASIC channel calibration
  const CbmTrdParFaspChannel *chFasp[2]={NULL};
  Int_t asicAddress=fAsicPar->GetAsicAddress(address<<1);
  if(asicAddress<0){
    LOG(WARNING) << GetName() << "::AddDigi: FASP Calibration for ro_ch " << address << " in module " << fModuleId <<" missing." << FairLogger::endl;
  } else {
    LOG(DEBUG) << GetName() << "::AddDigi: Found FASP "<< asicAddress <<" for ro_ch " << address << " in module " << fModuleId<< FairLogger::endl;
    CbmTrdParFasp *fasp  = (CbmTrdParFasp*)fAsicPar->GetAsicPar(asicAddress);
    //fasp->Print();
    chFasp[0] = fasp->GetChannel(address, 0);
    chFasp[1] = fasp->GetChannel(address, 1);
  }
  
  Float_t fChargeRef(chFasp[1]?chFasp[1]->GetMinDelaySignal():2586);   // reference value in ADC @ minimum delay fFASPpileUpTime 300ns
  Float_t fdt(chFasp[1]?chFasp[1]->GetMinDelayParam():4.181e-6);      // factor of parabolic dependence dt=fdt*(s-s0)^2 to calculate trigger delay
  Int_t fFASPpileUpTime(chFasp[1]?chFasp[1]->GetPileUpTime():300); // pile-up time 300ns @ fChargeRef
  //chFasp[1]->Print();
  Double_t weighting = charge[0]+charge[1];
  
  if (CbmTrdDigitizer::UseWeightedDist()) {
    TVector3 padPos, padPosErr;
    fDigiPar->GetPadPosition(address, padPos, padPosErr);
    Double_t distance = sqrt(pow(fXYZ[0] - padPos[0],2) + pow(fXYZ[1] - padPos[1],2));
    weighting = 1. / distance;
  }
  
  // make digi
  CbmTrdDigi* digi(NULL), *sdigi(NULL); CbmMatch* digiMatch(NULL);  
  // compute time delay based on CADENCE simulation
  Float_t tDelay=(0.5*(charge[0]+charge[1])-fChargeRef);
  tDelay*=tDelay; tDelay*=fdt;
  //printf("       DigiTime[ns] = %10.2f Form[%3d] delay[%6.2f]\n", time, fFASPpileUpTime, tDelay);
  digi= new CbmTrdDigi(address, charge[0], charge[1], ULong64_t((time+fFASPpileUpTime+tDelay)/CbmTrdDigi::Clk(CbmTrdDigi::kFASP)));
  digi->SetAddressModule(fModuleId); // may not be needed in the future
  //printf("CbmTrdModuleSimT::AddDigi(%d)=%d\n", address, fModuleId);
  digiMatch = new CbmMatch();
  digiMatch->AddLink(CbmLink(weighting, fPointId, fEventId, fInputId));
  digi->SetMatch(digiMatch);

  // get the link to saved digits
  std::map<Int_t,std::vector<pair<CbmTrdDigi*, CbmMatch*>>>::iterator it=fBuffer.find(address);

  // check for saved 
  if(it!=fBuffer.end()){
    Bool_t kINSERT(kFALSE);
    for(std::vector<pair<CbmTrdDigi*, CbmMatch*>>::iterator itv=fBuffer[address].begin(); itv!=fBuffer[address].end(); itv++){
      sdigi = itv->first;
      if(sdigi->GetTime()<=digi->GetTime()) continue; // arrange digits in increasing order of time
      fBuffer[address].insert(itv, make_pair(digi,digiMatch));
      if(VERBOSE) cout<<"         => Save(I) "<<digi->ToString();        
      kINSERT=kTRUE;
      break;
    }
    if(!kINSERT){ 
      fBuffer[address].push_back(make_pair(digi, digiMatch));
      if(VERBOSE) cout<<"         => Save(B) "<<digi->ToString();        
    }
  } else { // add address
    if(VERBOSE) cout<<"         => Add "<<digi->ToString();        
    fBuffer[address].push_back(make_pair(digi, digiMatch));
  }
}                                                

//_______________________________________________________________________________________________
Int_t CbmTrdModuleSimT::FlushBuffer(ULong64_t time)
{ 
/** Flush time sorted digi buffer until requested moment in time. If time limit not specified flush all digits.
 *  Calculate timely interaction between digits which are produced either on different anode wires for the same particle or 
 * are produced by 2 particle close by. Also take into account FASP dead time and mark such digits correspondingly
 */  

  //printf("CbmTrdModuleSimT::FlushBuffer(%lu)\n", time);
  Double_t dt(0.);

  Int_t n(0), n1(0); 
  CbmTrdDigi *digi(NULL), *sdigi(NULL);
  CbmMatch *digiMatch(NULL), *sdigiMatch(NULL);
  CbmTrdParFasp *fasp(NULL); const CbmTrdParFaspChannel *chFasp[2]={NULL};

  // write from saved buffer
  Int_t address(0), localAddress(0), ndigi(0), n2(0);
  for(std::map<Int_t, std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>>::iterator it = fBuffer.begin(); it!=fBuffer.end(); it++){
    localAddress = it->first;
    ndigi   = fBuffer[localAddress].size();
    if(!ndigi){
      //printf("FOUND saved vector empty @ %d\n", localAddress);
      continue;
    }
    // compute CBM address
    Int_t col, row = GetPadRowCol(localAddress, col),
          srow, sec = fDigiPar->GetSector(row, srow);
    address = CbmTrdAddress::GetAddress(fLayerId, fModuleId, sec, srow, col);
    if(VERBOSE) printf("FOUND %d digi @ col[%d] row[%d] srow[%d] sec[%d]\n", ndigi, col, row, srow, sec);

    // get ASIC channel calibration
    Int_t asicAddress=fAsicPar->GetAsicAddress(localAddress<<1);
    if(asicAddress<0){
      LOG(WARNING) << GetName() << "::FlushBuffer: FASP Calibration for ro_ch " << localAddress << " in module " << fModuleId <<" missing." << FairLogger::endl;
    } else {
      LOG(DEBUG) << GetName() << "::FlushBuffer: Found FASP "<< asicAddress <<" for ro_ch " << localAddress << " in module " << fModuleId<< FairLogger::endl;
      fasp  = (CbmTrdParFasp*)fAsicPar->GetAsicPar(asicAddress);
      //fasp->Print();
      chFasp[0] = fasp->GetChannel(localAddress, 0);
      chFasp[1] = fasp->GetChannel(localAddress, 1);
    }

    Int_t fFASPpileUpTime = (chFasp[0]?chFasp[0]->GetPileUpTime():300); // pile-up time 300ns @ max amplitude
    Float_t fFASPdeadTime = fFASPpileUpTime + 
                            CbmTrdDigi::Clk(CbmTrdDigi::kFASP)*(chFasp[0]?chFasp[0]->GetFlatTop():14);  // deadtime 14 clocks @ 80MHz
    
    n2+=ndigi; Int_t idx(0);
    std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>::iterator iv = fBuffer[localAddress].begin();
    while(iv != fBuffer[localAddress].end()){
      digi = iv->first; digiMatch = iv->second;
      //printf("check %d of %d [%p]\n", idx++, ndigi, (void*)digi);
      // skip digits which might further interact
      if(time>0 && digi->GetTime()>time-fFASPdeadTime) break;
      // look to later events
      std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>::iterator jv = iv+1;
      while(jv != fBuffer[localAddress].end()){
        sdigi = jv->first;
        // skip digits which might further interact
        if(time>0 && sdigi->GetTime()>time) break;

        dt=sdigi->GetTime() - digi->GetTime();        
        if(dt<0){
          LOG(ERROR) << GetName() << "::FlushBuffer : Time inversion in buffered digits. Need investigations by expert." << FairLogger::endl;
          break;
          iv++; continue;
        } 
        //printf("check dt[%f] ...\n", dt);  
        sdigiMatch = jv->second;
        if(dt<fFASPpileUpTime) { 
          digi->SetTimeDAQ(sdigi->GetTimeDAQ());
          digi->AddCharge(sdigi, (1.-dt/fFASPpileUpTime));
          // TODO take care of TR charge
          digi->SetPileUp();
          for(Int_t il(0); il<sdigiMatch->GetNofLinks(); il++) digiMatch->AddLink(sdigiMatch->GetLink(il));
          
          // clean vector element
          jv = fBuffer[localAddress].erase(jv);
          if(VERBOSE) cout<<"\tPile-up dt["<< fixed << setprecision(5) <<dt<<"] with "<<sdigi->ToString();
          // clean digits allocation
          delete sdigi; 
        } else if(dt<fFASPdeadTime) {
          // clean vector element
          jv = fBuffer[localAddress].erase(jv);

          sdigi->SetMasked();
          if(VERBOSE) cout<<"\tDeadtime dt["<< setw(5)<<dt<<"]. "<<endl<<"WBM: "<<sdigi->ToString();        
          // digits allocation clearing is taken care by the CbmDaqBuffer
          //CbmDaqBuffer::Instance()->InsertData(sdigi);
          fDigiMap[address] = make_pair(sdigi, sdigiMatch);
        } else break;
      } 
      if(VERBOSE) cout<<"  WB: "<<digi->ToString();
      //daqBuffer->InsertData(digi);
      fDigiMap[address] = make_pair(digi, digiMatch);
      iv=fBuffer[localAddress].erase(iv); // remove from saved buffer
    }
    // clear address if there are no more digits available
    //if(!fBuffer[address].size()) it = fBuffer.erase(it);
  }
//   LOG(INFO) << "TRDdigitizerPRF::FlushLocalBuffer : digits " << n1 << "(local)/"<< n-n1 << "(late) @ "<<time<<"ns Ev["<< fEventNr<<"]. Hold digits "<<fDigiMap.size()<<"(local)/"<<n2<<"(late)."<<FairLogger::endl; 
  if(VERBOSE) DumpBuffer();
  return n;
}

//_______________________________________________________________________________________________
void CbmTrdModuleSimT::DumpBuffer() const
{
  for(std::map<Int_t, std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>>::const_iterator it = fBuffer.begin(); it!=fBuffer.end(); it++){
    if(!it->second.size()) continue;    
    printf("address[%10d] n[%2d]\n", it->first, (Int_t)it->second.size());
    for(std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>::const_iterator iv = it->second.begin(); iv != it->second.end(); iv++)  cout<<"\t"<<iv->first->ToString()<<endl;
  }  
}


//_______________________________________________________________________________
void CbmTrdModuleSimT::SetAsicPar(CbmTrdParSetAsic *p)
{
/** Build local set of ASICs and perform initialization. Need a proper fDigiPar already defined.
 */  
  if(fAsicPar){
    LOG(WARNING) << GetName() << "::SetAsicPar : The list for module "<< fModuleId <<" already initialized."<< FairLogger::endl;
    return;    
  }
  fAsicPar = p;//new CbmTrdParSetAsic();
  //fAsicPar->Print();
  return;
  if(!fDigiPar){
    LOG(WARNING) << GetName() << "::SetAsicPar : No Digi params for module "<< fModuleId <<". Try calling first CbmTrdModSim::SetDigiPar to get FASP position right."<< FairLogger::endl;
    return;
  }

  CbmTrdParAsic *asic(NULL);
  
  Int_t iFebGroup = 0;
  Int_t gRow[3] = {  1, 2, 4 };  // re-ordering on the feb -> same mapping for normal and super
  Int_t gCol[3] = {  8, 8, 4 };  // re-ordering on the feb -> same mapping for normal and super
  Double_t xAsic = 0;  // x position of Asic
  Double_t yAsic = 0;  // y position of Asic

  Int_t rowId(0), isecId(0), irowId(0), iAsic(0);
  for (Int_t s = 0, rg(0); s < fDigiPar->GetNofSectors(); s++) {    
    for (Int_t r = 0; r < fDigiPar->GetNofRowsInSector(s); r++, rg++){ 
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
            if (local_point[0] > 2*fDx)     LOG(ERROR) << GetName() << "::SetAsicPar: asic position x=" << local_point[0] << " is out of bounds [0," << 2*fDx<< "]!" << FairLogger::endl;
            if (local_point[1] > 2*fDy)     LOG(ERROR) << GetName() << "::SetAsicPar: asic position y=" << local_point[1] << " is out of bounds [0," << 2*fDy<< "]!" << FairLogger::endl;

            // local_point[i] must be >= 0 at this point      Double_t local_point[3];
            Int_t address=GetAsicAddress(iAsic);
            if(!(asic = fAsicPar->GetAsicPar(address))){
              LOG(WARNING) << GetName() << "::SetAsicPar : Couldn't find ASIC @ "<<local_point[0] - fDx<<", "<< local_point[1] - fDy<<" address "<<address<< FairLogger::endl;
              asic = new CbmTrdParFasp(address, iFebGroup, local_point[0] - fDx, local_point[1] - fDy);
              fAsicPar->SetAsicPar(address, asic);
            } else {
              //LOG(INFO) << GetName() << "::SetAsicPar : Found ASIC @ address "<<address<< FairLogger::endl;
              asic->SetPosition(local_point[0] - fDx, local_point[1] - fDy);
              asic->SetFebGrouping(iFebGroup);
            }
            
            // read-out channel to FASP channel mapping TODO more realistically
            for (Int_t ir = rowId; ir < rowId + gRow[iFebGroup]; ir++) {
              for (Int_t ic = c; ic < c + gCol[iFebGroup]; ic++) {
                if (ir >= fDigiPar->GetNofRows() )     LOG(ERROR) <<  GetName() << "::SetAsicPar: ir " << ir << " is out of bounds!" << FairLogger::endl;
                if (ic >= fDigiPar->GetNofColumns() )  LOG(ERROR) <<  GetName() << "::SetAsicPar: ic " << ic << " is out of bounds!" << FairLogger::endl;
                //isecId = fDigiPar->GetSector((Int_t)ir, irowId);
                asic->SetChannelAddress(GetPadAddress(rg, ic));
                  //CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(fModuleId), CbmTrdAddress::GetModuleId(fModuleId), isecId, irowId, ic));
                if (false)
                  printf("               M:%10i(%4i) s: %i  irowId: %4i  ic: %4i r: %4i c: %4i   address:%10i\n",fModuleId,
                    CbmTrdAddress::GetModuleId(fModuleId),
                    isecId, irowId, ic, r, c,
                    CbmTrdAddress::GetAddress(fLayerId, fModuleId, isecId, irowId, ic));
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
//   for (Int_t s = 0; s < fDigiPar->GetNofSectors(); s++){
//     const Int_t nRow = fDigiPar->GetNofRowsInSector(s);
//     const Int_t nCol = fDigiPar->GetNofColumnsInSector(s);
  for (Int_t r = 0; r < GetNrows(); r++){
    for (Int_t c = 0; c < GetNcols(); c++){
      Int_t channelAddress = GetPadAddress(r,c);
      //CbmTrdAddress::GetAddress(CbmTrdAddress::GetLayerId(fModuleId),CbmTrdAddress::GetModuleId(fModuleId), s, r, c);
      if (fAsicPar->GetAsicAddress(channelAddress) == -1)
        LOG(ERROR) <<  GetName() << "::SetAsicPar: Channel address:" << channelAddress << " is not or multiple initialized in module " << fModuleId << "(ID:" << CbmTrdAddress::GetModuleId(fModuleId) << ")" << "(r:" << r << ", c:" << c << ")" << FairLogger::endl;
    }
  }
//  }
  //fAsicPar->Print();
}

ClassImp(CbmTrdModuleSimT)


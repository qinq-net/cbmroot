#include "CbmTrdParModGas.h"
#include "CbmTrdGas.h"

#include <FairLogger.h>

#include <TObjArray.h>
#include <TObjString.h>
#include <TH2F.h>
#include <TMath.h>

#define VERBOSE 0  

// binding energy in keV for 'K' 'L' and 'M' shells of Ar and Xe 
Float_t CbmTrdParModGas::fgkBindingEnergy[2][NSHELLS] = {
  {34.5,  5.1, 1.1}, // Xe
  { 3.2, 0.25, 0.1}  // Ar
};
// binding energy in keV for 'K' 'L' and 'M' shells of Ar and Xe 
Float_t CbmTrdParModGas::fgkBR[2][NSHELLS-1] = {
  {0.11, 0.02}, // Xe
  {0.11, 0.02}  // Ar
};
// Bucharest detector gas gain parametrization based on 55Fe measurements
Float_t CbmTrdParModGas::fgkGGainUaPar[2][2]  = {
  {-9.27081, 7.98688},  // Xe
  {-10.1676,  8.3745}   // Ar
};
Float_t CbmTrdParModGas::fgkE0 = 866.1047;// energy offset in ADC ch @ 0 keV

//___________________________________________________________________
CbmTrdParModGas::CbmTrdParModGas(const char* title)
  : CbmTrdParMod("CbmTrdParModGas", title)
  ,fConfig(0)
  ,fUa(0)
  ,fUd(0)
  ,fDw(0.3)
  ,fGasThick(0.6)
  ,fPercentCO2(0.2)
  ,fDriftMap(NULL)
  ,fFileNamePID()
{
  TString s(title);
  TString name; Int_t val;
  TObjArray *so = s.Tokenize("/");
  for(Int_t ie(0); ie<so->GetEntries(); ie+=2){
    name = ((TObjString*)(*so)[ie])->String();
    if(name.EqualTo("Module")) fModuleId = ((TObjString*)(*so)[ie+1])->String().Atoi();
    else if(name.EqualTo("Ua")) fUa = ((TObjString*)(*so)[ie+1])->String().Atoi();
    else if(name.EqualTo("Ud")) fUd = ((TObjString*)(*so)[ie+1])->String().Atoi();
//    else if(name.EqualTo("Gas")) snprintf(fGas, 3, "%s", ((TObjString*)(*so)[ie+1])->String().Data());
    SetNobleGasType();
  }
  so->Delete(); delete so;

  if(VERBOSE) printf("Module[%2d] U[%4d %3d]\n", fModuleId, fUa, fUd);
}

//___________________________________________________________________
CbmTrdParModGas::~CbmTrdParModGas()
{
//  if(fDriftMap) delete fDriftMap;
}  

//_______________________________________________________________________________________________
Float_t CbmTrdParModGas::GetCharge(Float_t ekev) const
{

  Int_t gasId=GetNobleGasType()-1;
  Double_t gain = TMath::Exp(fgkGGainUaPar[gasId][0]+fgkGGainUaPar[gasId][1]*fUa*1.e-3);
  //printf("gasId[%d] ua[%d] gain[%f]\n", gasId, fUa, gain);
  Double_t eadc = fgkE0 + gain * ekev;
  //printf("E = %fkeV %fADC\n", ekev, eadc);  
  // apply FASP gain -> should be done without intermediate ADC conversion TODO 
  Double_t sFASP = eadc/2.; // FASP signal [mV]; FASP amplification 1
  // FASP gaincharacteristic -> should be defined elsewhere
  // data based on CADENCE simulations
  Double_t s0FASP = 10, gFASP = 6;
  return (sFASP-s0FASP)/gFASP;
}

//_______________________________________________________________________________________________
Int_t CbmTrdParModGas::GetShellId(const Char_t shell) const
{
/** Return index of atomic shell. 
 * shell name can be 'K', 'L' and 'M'
 */  
  switch(shell){
    case 'K': return 0;
    case 'L': return 1;
    case 'M': return 2;
    default:
      LOG(WARNING) << GetName() <<"::GetShellId: Atomic shell : "<<shell<<" not defined for gas "<<(GetNobleGasType()==2?"Ar":"Xe") << FairLogger::endl;
      return -1;
  }
}

//_______________________________________________________________________________________________
Float_t CbmTrdParModGas::GetBindingEnergy(const Char_t shell, Bool_t main) const
{
  Int_t gasId=GetNobleGasType()-1;
  Int_t shellId = GetShellId(shell);
  if(shellId<0) return 0;
  
  if(!main) return fgkBindingEnergy[gasId][shellId];
  else{
    if(shellId<NSHELLS-1) return fgkBindingEnergy[gasId][shellId+1];
    else{
      LOG(WARNING) << GetName() <<"::GetBindingEnergy: Request atomic shell : "<<shellId+1<<" not defined for gas "<<(gasId?"Ar":"Xe") << FairLogger::endl;
      return 0;
    }
  }
  return 0;
}

//_______________________________________________________________________________________________
Float_t CbmTrdParModGas::GetNonIonizingBR(const Char_t shell) const
{
  Int_t gasId=GetNobleGasType()-1;
  Int_t shellId = GetShellId(shell);
  if(shellId<0) return 0;

  return fgkBR[gasId][shellId];  
}

//_______________________________________________________________________________________________
Char_t CbmTrdParModGas::GetPEshell(Float_t Ex) const
{
  const Char_t shellName[NSHELLS] = {'K', 'L', 'M'};
  Int_t gasId=GetNobleGasType()-1;
  for(Int_t ishell(0); ishell<NSHELLS; ishell++){
    if(Ex<fgkBindingEnergy[gasId][ishell]) continue;
    return shellName[ishell];
  }  
  LOG(WARNING) << GetName() <<"::GetPEshell: Ex[keV] "<<Ex<<"less than highes atomic shell binding energy : "<<fgkBindingEnergy[gasId][NSHELLS-1]<<" for gas "<<(gasId?"Ar":"Xe") << FairLogger::endl;
  return 0;
}

//_______________________________________________________________________________________________
Double_t CbmTrdParModGas::GetDriftTime(Double_t y0, Double_t z0) const
{
  const TAxis *ay(fDriftMap->GetXaxis()), *az(fDriftMap->GetYaxis());
  Int_t by(ay->FindBin(y0)), 
        bz(az->FindBin(z0));
  Double_t tmin(fDriftMap->GetBinContent(by, bz));      
  if(VERBOSE) printf("GetDriftTime :: Start @ dt=%3d [ns]\n", Int_t(tmin));
  return tmin;
}

//_______________________________________________________________________________________________
Double_t CbmTrdParModGas::ScanDriftTime(Double_t y0, Double_t z0, Double_t dzdy, Double_t dy) const
{
  Double_t y1=y0+dy, z1=z0+dzdy*dy,
           dw(fDw), dwh(0.5*dw), dhh(fGasThick);
  
  if(VERBOSE) printf("ScanDriftTime :: Try : [%7.4f %7.4f] => [%7.4f %7.4f]\n", y0, z0, y1, z1);
  if(y1<-dwh-1e-3) y0+=dw;
  else if(y1>dwh+1.e-3) y0-=dw;
  if(VERBOSE){ 
    y1=y0+dy; z1=z0+dzdy*dy;
    printf("ScanDriftTime :: Do  : y0(%7.4f), z0(%7.4f), dzdy(%7.4f), dy(%7.4f)\n", y0, z0, dzdy, dy);
  }
  TH2F *h = fDriftMap;  
  y1=y0; z1=z0;
  const TAxis *ay(fDriftMap->GetXaxis()), *az(fDriftMap->GetYaxis());
  Int_t by(ay->FindBin(y1)), 
        bz(az->FindBin(z1)),
        nby(ay->GetNbins()),
        nbz(az->GetNbins());
  Float_t dyStep = ay->GetBinWidth(1), 
          tmin(500), tmax(0), tc(0);
  while(by>0 && by<=nby && bz<=nbz){
    bz=az->FindBin(z1);
    tc=h->GetBinContent(by, bz);
    if(tc>1.e-4&&tc<tmin) tmin=tc;
    if(tc>1.e-4&&tc>tmax) tmax=tc;
    z1+=TMath::Abs(dzdy)*dyStep;
    if(dzdy>0){ y1+=dyStep; by++;}
    else{ y1-=dyStep; by--;}
  }
  if(VERBOSE) printf("ScanDriftTime :: Start @ dt=%3d [ns]\n", Int_t(tmin));

  return tmin;
}

//___________________________________________________________________
void CbmTrdParModGas::SetDriftMap(TH2F *hm, TDirectory *d)
{
/**  
 * Load drift map in the module and get ownership
 */

  if(VERBOSE) printf("CbmTrdParModGas::SetDriftMap : Module[%2d] U[%4d %3d]\n", fModuleId, fUa, fUd);
  
  fDriftMap = (TH2F*)hm->Clone(Form("trdDM%02d", fModuleId));
  fDriftMap->SetTitle(GetTitle());
  fDriftMap->SetDirectory(d);
}

ClassImp(CbmTrdParModGas)

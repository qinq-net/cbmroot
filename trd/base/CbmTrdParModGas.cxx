#include "CbmTrdParModGas.h"
#include "CbmTrdGas.h"

#include <TObjArray.h>
#include <TObjString.h>
#include <TH2F.h>

#define VERBOSE 0
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
  
  fDriftMap = (TH2F*)hm->Clone(Form("trdDM%02d", fModuleId));
  fDriftMap->SetTitle(GetTitle());
  fDriftMap->SetDirectory(d);
}

ClassImp(CbmTrdParModGas)

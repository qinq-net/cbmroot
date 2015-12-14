/** @file CbmTofTracklet.cxx
 ** @author nh
 ** @date 17.05.2015
 **
 **/

#include "CbmTofTracklet.h"
#include "CbmTofHit.h"
#include "Rtypes.h"                     // for Double_t, Double32_t, Int_t, etc
#include "TMatrixFSymfwd.h"             // for TMatrixFSym
#include <iostream>
#include "FairLogger.h"

using std::cout;
using std::endl;

CbmTofTracklet::CbmTofTracklet() :
   TObject(),
   fGlbTrack(-1),
   fTrackLength(0.),
   fPidHypo(-1),
   fDistance(0.),
   fTime(0.),
   fTt(0.),
   fT0(0.),
   fChiSq(0.),
   fNDF(0),
   fTrackPar(),
   fParamFirst(),
   fParamLast(),
   fTofHit(0,-1),
   fTofDet(),
   fMatChi(),
   fpHit()
{
}

CbmTofTracklet::CbmTofTracklet( const CbmTofTracklet &t) :
   TObject(),
   fGlbTrack(-1),
   fTrackLength(0.),
   fPidHypo(-1),
   fDistance(0.),
   fTime(0.),
   fTt(0.),
   fT0(0.),
   fChiSq(0.),
   fNDF(0),
   fTrackPar(),
   fParamFirst(),
   fParamLast(),
   fTofHit(0,-1),
   fTofDet(),
   fMatChi(),
   fpHit()
{
  fGlbTrack=t.fGlbTrack;
  fTrackLength=t.fTrackLength;
  fPidHypo=t.fPidHypo;
  fDistance=t.fDistance;
  fTime=t.fTime;
  fTt=t.fTt;
  fT0=t.fT0;
  fChiSq=t.fChiSq;
  fNDF=t.fNDF;
  fTrackPar=t.fTrackPar;
  fParamFirst=t.fParamFirst;
  fParamLast=t.fParamLast;
  fTofHit=t.fTofHit;
  fTofDet=t.fTofDet;
  fMatChi=t.fMatChi;
  fpHit=t.fpHit;
}

CbmTofTracklet::~CbmTofTracklet() {
}

/*
CbmTofTracklet::CbmTofTracklet(const CbmTofTracklet &fSource) :
  TObject(), //CbmTrack(),
   fGlbTrack(-1),
   fTrackLength(0.),
   fPidHypo(-1),
   fDistance(0.),
   fTime(0.),
   fTt(0.),
   fT0(0.),
   fChiSq(0.),
   fNDF(0),
   fTrackPar(),
   fParamFirst(),
   fParamLast(),
   fTofHit(0,-1),
   fTofDet(),
   fMatChi(),
   fpHit()
{
}

CbmTofTracklet& CbmTofTracklet::operator=(const CbmTofTracklet &fSource){
  // do something !
   return *this;
}
*/

void CbmTofTracklet::SetParamLast(const CbmTofTrackletParam* par){
  fParamLast.SetX( par->GetX() );
  fParamLast.SetY( par->GetY() );
  fParamLast.SetZ( par->GetZ() );
  fParamLast.SetTx( par->GetTx() );
  fParamLast.SetTy( par->GetTy() );
  fParamLast.SetQp( par->GetQp() );
  for(int i=0, k=0;i<3;i++) for(int j=0; j<=i; j++,k++)
      fParamLast.SetCovariance(i,j,par->GetCovariance(k));
}

void CbmTofTracklet::GetFairTrackParamLast(){
  fTrackPar.SetX( fParamLast.GetX() );
  fTrackPar.SetY( fParamLast.GetY() );
  fTrackPar.SetZ( fParamLast.GetZ() );
  fTrackPar.SetTx( fParamLast.GetTx() );
  fTrackPar.SetTy( fParamLast.GetTy() );
  fTrackPar.SetQp( fParamLast.GetQp() );
  for(int i=0, k=0;i<3;i++) for(int j=0; j<=i; j++,k++)
      fTrackPar.SetCovariance(k,fParamLast.GetCovariance(i,j));
}

Double_t CbmTofTracklet::GetMatChi2(Int_t iSm){
  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    //cout << Form(" -v- ind %d, sm %d==%d ?, chi %f ",fTofHit[iHit],iSm,fTofDet[iHit],fMatChi[iHit])<<endl;
    //if(0==fTofDet[iHit]) LOG(FATAL) << " CbmTofTracklet::GetMatChi2 Invalid Detector Type! "<<FairLogger::endl;
    if (iSm == fTofDet[iHit]) return fMatChi[iHit];
  }
  return -1.;
}

Int_t CbmTofTracklet::GetFirstInd(Int_t iSm){
  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    //    cout << "     GFI "<< iSm <<", "<<iHit<<", "<< fTofDet[iHit] << endl;
    if (iSm != fTofDet[iHit]) return fTofHit[iHit];
  }
  LOG(FATAL) << " CbmTofTracklet::GetFirstInd, did only find "<< iSm << FairLogger::endl;
  return -1;
}

Double_t CbmTofTracklet::GetZ0x(){
  Double_t dZ0=0.;
  if(fTrackPar.GetTx()!=0.) dZ0=-fTrackPar.GetX()/fTrackPar.GetTx()+fTrackPar.GetZ();
  return dZ0;
}

Double_t CbmTofTracklet::GetZ0y(){
  Double_t dZ0=0.;
  if(fTrackPar.GetTy()!=0.) dZ0=-fTrackPar.GetY()/fTrackPar.GetTy()+fTrackPar.GetZ();
  return dZ0;
}

Double_t CbmTofTracklet::UpdateT0(){ //returns estimated time at R==0
  Double_t dT0=0.;
  Double_t nValidHits=0.;
  Int_t    iHit0=-1;
  if(fTofHit.size()>2) UpdateTt();  // update Tt first
  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    //cout << fpHit[iHit]->ToString()<<endl;
    if( fTofDet[iHit]>0) {                        // exlude faked hits
      dT0 += fpHit[iHit].GetTime() - fTt*fpHit[iHit].GetR();
      nValidHits++;
    } else iHit0=iHit;
  }
  dT0 /= nValidHits;
  fT0=dT0;
  if (iHit0>-1) fpHit[iHit0].SetTime(dT0);

  //cout << Form("   -D- CbmTofTracklet::GetT0: Trkl size %d,  validHits %3.0f, Tt = %6.2f dT0 = %6.2f",fTofHit.size(),nValidHits,fTt,dT0)<<endl;  
  return fT0;
}

Double_t CbmTofTracklet::UpdateTt(){
  Double_t dTt=0.;
  Int_t iNt=0;
  for (UInt_t iHL=0; iHL<fpHit.size()-1; iHL++){
      if( fTofDet[iHL]>0 )                         // exclude faked hits 
      for (UInt_t iHH=iHL+1; iHH<fpHit.size(); iHH++){
      if( fTofDet[iHH]>0) {                        // exclude faked hits 	      
	dTt+=(fpHit[iHH].GetTime()-fpHit[iHL].GetTime())/(fpHit[iHH].GetR()-fpHit[iHL].GetR());
	iNt++;
      }
    }
  }
  if (iNt==0) {
    cout << "-W- CbmTofTracklet::GetTt: No valid hit pair "<<endl;
    return fTt; 
  }
  fTt = dTt/(Double_t)iNt;
  return fTt;
}


Double_t CbmTofTracklet::GetTdif(Int_t iSmType, CbmTofHit* pHit){
  Double_t dTref=0.;
  Double_t Nref=0;
  Double_t dTt=0.;
  Int_t iNt=0;
  for (Int_t iHL=0; iHL<fpHit.size()-1; iHL++){
     if (iSmType == fTofDet[iHL] || 0 == fTofDet[iHL]) continue;           // exclude faked hits 
     for (Int_t iHH=iHL+1; iHH<fpHit.size(); iHH++){
       if (iSmType == fTofDet[iHH] || 0 == fTofDet[iHH]) continue;           // exclude faked hits 
	dTt+=(fpHit[iHH].GetTime()-fpHit[iHL].GetTime())/(fpHit[iHH].GetR()-fpHit[iHL].GetR());
	iNt++;
     }
  }
  
  if (iNt==0) {
    cout << "-E- CbmTofTracklet::GetTdif: No valid hit pair "<<endl;
    return 1.E20; 
  }
  dTt/=(Double_t)iNt;

  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    if (iSmType == fTofDet[iHit] || 0 == fTofDet[iHit]) continue;  
    dTref += fpHit[iHit].GetTime() - dTt*(fpHit[iHit].GetR()-pHit->GetR());
    Nref++;
  }
  if(Nref == 0) {
    cout << "-E- CbmTofTracklet::GetTdif: SmType "<<iSmType<<", Nref "<<Nref
	 <<" sizes "<<fTofHit.size()<<", "<<fpHit.size()<<endl;
    return 1.E20;
  }
  dTref /= Nref;
  Double_t dTdif=pHit->GetTime()-dTref;
  // cout << "-D- CbmTofTracklet::GetTdif: iSt "<< iSt<<" SmType "<<iSmType<<", Nref "<<Nref<<" Tdif "<<dTdif<<endl;
  return dTdif;
}

const Double_t* CbmTofTracklet::GetPoint(Int_t n) {  // interface to event display: CbmTracks
  fP[0]=fpHit[n].GetX();
  fP[1]=fpHit[n].GetY();
  fP[2]=fpHit[n].GetZ();
  fP[3]=fpHit[n].GetTime();
  //  cout <<Form("CbmTofTracklet::GetPoint %d, %6.2f, %6.2f, %6.2f, %6.2f ",n,fP[0],fP[1],fP[2],fP[3]) << endl;
  return fP;
}

const Double_t* CbmTofTracklet::GetFitPoint(Int_t n) {  // interface to event display: CbmTracks
  fP[0]=GetFitX(fpHit[n].GetZ());
  fP[1]=GetFitY(fpHit[n].GetZ());
  fP[2]=fpHit[n].GetZ();
  fP[3]=fpHit[n].GetTime();
  /*
  cout <<Form("CbmTofTracklet::GetFitPoint %d, %6.2f - %6.2f, %6.2f - %6.2f, %6.2f, %6.2f ",
	      n,fP[0],fpHit[n]->GetX(),fP[1],fpHit[n]->GetY(),fP[2],fP[3]) << endl;
  */
  return fP;
}

Double_t CbmTofTracklet::GetFitX(Double_t dZ){
  return fTrackPar.GetX() + fTrackPar.GetTx()*(dZ-fTrackPar.GetZ());
}

Double_t CbmTofTracklet::GetFitY(Double_t dZ){
  return fTrackPar.GetY() + fTrackPar.GetTy()*(dZ-fTrackPar.GetZ());
}

Double_t CbmTofTracklet::GetFitT(Double_t dR){
  return GetT0() + fTt*dR;
}

void CbmTofTracklet::Clear(Option_t* option){

  //  cout << "-D- Clear TofTracklet with option "<<*option<<endl; 
  fTofHit.clear();
  fTofDet.clear();
  fMatChi.clear();
  fpHit.clear();

}

ClassImp(CbmTofTracklet)


/** @file CbmTofTracklet.cxx
 ** @author nh
 ** @date 17.05.2015
 **
 **/

#include "CbmTofTracklet.h"
#include "CbmTofHit.h"

#include "FairLogger.h"

#include "Rtypes.h"                     // for Double_t, Double32_t, Int_t, etc
#include "TMatrixFSymfwd.h"             // for TMatrixFSym
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompSVD.h"

using std::vector;

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

Double_t CbmTofTracklet::GetMatChi2(Int_t iAddr){
  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    //LOG(INFO) << Form(" -v- ind %d, sm %d==%d ?, chi %f ",fTofHit[iHit],iSm,fTofDet[iHit],fMatChi[iHit])<<
    //FairLogger::endl;
    //if(0==fTofDet[iHit]) LOG(FATAL) << " CbmTofTracklet::GetMatChi2 Invalid Detector Type! "<<FairLogger::endl;
    if (iAddr == fTofDet[iHit]) return fMatChi[iHit];
  }
  return -1.;
}

Int_t CbmTofTracklet::GetFirstInd(Int_t iAddr){
  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    //    LOG(INFO) << "     GFI "<< iSm <<", "<<iHit<<", "<< fTofDet[iHit] <<
    // FairLogger::endl;
    if (iAddr != fTofDet[iHit]) return fTofHit[iHit];
  }
  LOG(FATAL) << " CbmTofTracklet::GetFirstInd, did only find "<< iAddr << FairLogger::endl;
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

Double_t CbmTofTracklet::GetTex(CbmTofHit* pHit){
  /*
  Double_t dR2=0.;
  dR2 += TMath::Power(fTrackPar.GetX()-pHit->GetX(),2);
  dR2 += TMath::Power(fTrackPar.GetY()-pHit->GetY(),2);
  dR2 += TMath::Power(fTrackPar.GetZ()-pHit->GetZ(),2);
  Double_t dR = TMath::Sqrt(dR2);
  */
  Double_t dR = pHit->GetR();
  LOG(DEBUG) <<Form(" CbmTofTracklet::GetTex T0 %7.1f dR %7.1f, Tt %7.4f => Tex %7.3f ",
		    fT0,dR,fTt,fT0 + dR*fTt)
	     << FairLogger::endl;
  return   fT0 + dR*fTt;
}

Double_t CbmTofTracklet::UpdateT0(){ //returns estimated time at R=0
//  Double_t dT0=0.;
  Int_t    nValidHits=0.;
  Int_t    iHit0=-1;
  /*
  if(fTofHit.size()>2) UpdateTt();  // update Tt first
  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    //LOG(INFO) << fpHit[iHit]->ToString()<<FairLogger::endl;
    if( fTofDet[iHit]>0) {                        // exlude faked hits
      dT0 += fpHit[iHit].GetTime() - fTt*fpHit[iHit].GetR();
      nValidHits++;
    } else iHit0=iHit;
  }
  dT0 /= nValidHits;
  fT0=dT0;
  */
  //
  // follow tutorial solveLinear.C to solve the linear equation t=t0+tt*R
  //
  Double_t aR[fTofHit.size()];
  Double_t at[fTofHit.size()];
  Double_t ae[fTofHit.size()];
  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    if( fTofDet[iHit]>0) {                        // exlude faked hits
      aR[nValidHits]=fpHit[iHit].GetR();
      at[nValidHits]=fpHit[iHit].GetTime();
      ae[nValidHits]=0.1;                         // const timing error, FIXME
      nValidHits++;
    } else iHit0=iHit;
  }

  TVectorD R; R.Use(nValidHits,aR);
  TVectorD t; t.Use(nValidHits,at);
  TVectorD e; e.Use(nValidHits,ae);

  const Int_t nrVar  = 2;
  TMatrixD A(nValidHits,nrVar);
  TMatrixDColumn(A,0) = 1.0;
  TMatrixDColumn(A,1) = R;

  // first bring the weights in place
  TMatrixD Aw = A;
  TVectorD yw = t;
  for (Int_t irow = 0; irow < A.GetNrows(); irow++) {
    TMatrixDRow(Aw,irow) *= 1/e(irow);
    yw(irow) /= e(irow);
  }

  TDecompSVD svd(Aw);
  Bool_t ok;
  const TVectorD c_svd = svd.Solve(yw,ok);

  // c_svd.Print();

  fT0=c_svd[0];
  fTt=c_svd[1];

  if (iHit0>-1) fpHit[iHit0].SetTime(fT0);
  /*
  LOG(INFO)<< Form("-D- CbmTofTracklet::UpdateT0: Trkl size %u,  validHits %d, Tt = %6.2f T0 = %6.2f",
              (UInt_t)fTofHit.size(),nValidHits,fTt,fT0)<<FairLogger::endl;  
  */
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
    LOG(WARNING) << "No valid hit pair "<<FairLogger::endl;
    return fTt; 
  }
  fTt = dTt/(Double_t)iNt;
  return fTt;
}


Double_t CbmTofTracklet::GetTdif(Int_t iDetId, CbmTofHit* pHit){
  Double_t dTref=0.;
  Double_t Nref=0;
  Double_t dTt=0.;
  Int_t iNt=0;
  if(0){
    for (UInt_t iHL=0; iHL<fpHit.size()-1; iHL++){
     if (iDetId == fTofDet[iHL] || 0 == fTofDet[iHL]) continue;           // exclude faked hits 
     for (UInt_t iHH=iHL+1; iHH<fpHit.size(); iHH++){
       if (iDetId == fTofDet[iHH] || 0 == fTofDet[iHH]) continue;           // exclude faked hits 
	dTt+=(fpHit[iHH].GetTime()-fpHit[iHL].GetTime())/(fpHit[iHH].GetR()-fpHit[iHL].GetR());
	iNt++;
     }
    }
  
    if (iNt==0) {
      LOG(ERROR) << "No valid hit pair "<<FairLogger::endl;
      return 1.E20; 
    }
    dTt/=(Double_t)iNt;
  }else{
    dTt=fTt;
  }
  for (UInt_t iHit=0; iHit<fTofHit.size(); iHit++){
    if (iDetId == fTofDet[iHit] || 0 == fTofDet[iHit]) continue;  
    dTref += fpHit[iHit].GetTime() - dTt*(fpHit[iHit].GetR()-pHit->GetR());
    Nref++;
  }
  if(Nref == 0) {
    LOG(ERROR) << "DetId "<<iDetId<<", Nref "<<Nref
	 <<" sizes "<<fTofHit.size()<<", "<<fpHit.size()<<FairLogger::endl;
    return 1.E20;
  }
  dTref /= Nref;
  Double_t dTdif=pHit->GetTime()-dTref;
  // LOG(DEBUG) << "iSt "<< iSt<<" DetId "<<iDetId<<", Nref "<<Nref<<" Tdif
  // "<<dTdif<<FairLogger::endl;
  return dTdif;
}

const Double_t* CbmTofTracklet::GetPoint(Int_t n) {  // interface to event display: CbmTracks
  fP[0]=fpHit[n].GetX();
  fP[1]=fpHit[n].GetY();
  fP[2]=fpHit[n].GetZ();
  fP[3]=fpHit[n].GetTime();
  //  LOG(INFO) <<Form("CbmTofTracklet::GetPoint %d, %6.2f, %6.2f, %6.2f, %6.2f ",n,fP[0],fP[1],fP[2],fP[3]) <<
  // FairLogger::endl;
  return fP;
}

const Double_t* CbmTofTracklet::GetFitPoint(Int_t n) {  // interface to event display: CbmTracks
  fP[0]=GetFitX(fpHit[n].GetZ());
  fP[1]=GetFitY(fpHit[n].GetZ());
  fP[2]=fpHit[n].GetZ();
  fP[3]=fpHit[n].GetTime();
  /*
  LOG(INFO) <<Form("CbmTofTracklet::GetFitPoint %d, %6.2f - %6.2f, %6.2f - %6.2f, %6.2f, %6.2f ",
	      n,fP[0],fpHit[n]->GetX(),fP[1],fpHit[n]->GetY(),fP[2],fP[3]) <<
              FairLogger::endl;
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
  return fT0 + fTt*dR;
}

void CbmTofTracklet::Clear(Option_t* /*option*/){

  //  LOG(DEBUG) << "Clear TofTracklet with option "<<*option<<FairLogger::endl; 
  fTofHit.clear();
  fTofDet.clear();
  fMatChi.clear();
  fpHit.clear();

}

ClassImp(CbmTofTracklet)


/**
 * \file CbmRichGeoOpt.cxx
 *
 * \author Tariq Mahmoud <t.mahmoud@gsi.de>
 * \date 2014
 **/

#include "CbmRichGeoOpt.h"
#include "TH1D.h"
#include "TH3D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"

#include "CbmMCTrack.h"
#include "FairTrackParam.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmRichRing.h"
#include "CbmRichHit.h"
#include "CbmRichPoint.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>
//++++++++++++++++++++++++++++++++++++++++++
#include "CbmRichHitProducer.h"
#include "CbmRichRingLight.h"

using namespace std;
using boost::assign::list_of;

CbmRichGeoOpt::CbmRichGeoOpt()
  : FairTask("CbmRichGeoOpt"),
    fRichPoints(NULL),
    fMcTracks(NULL),
    fRefPoints(NULL),
    fRichHits(NULL),
    fGP(),
    fRichRings(NULL), 
    fRichRingMatches(NULL),
    fEventNum(0),
    PointsFilled(0),
    fMinNofHits(7),
    nPhotonsNotOnPlane(0),
    nPhotonsNotOnSphere(0),
    nTotalPhorons(0),
    PlanePoints(),
    PMTPlaneCenter(),
    ReadPMTPlaneCenter(),
    ReadPMTPlaneCenterOrig(),
    MirrorCenter(),
    ReadMirrorCenter(),
    RotX(0.),
    RotY(0.),
    r1(),
    r2(),
    n(),
  H_MomPrim(NULL),
  H_PtPrim(NULL),
  H_Hits_XY(NULL),
  H_PointsIn_XY(NULL),
  H_PointsOut_XY(NULL),
  H_NofPhotonsPerHit(NULL),
  H_DiffXhit(NULL),
  H_DiffYhit(NULL),
  H_dFocalPoint_Delta(NULL),
  H_dFocalPoint_Rho(NULL),
  H_Alpha(NULL),
  H_Alpha_UpLeft(NULL),
  H_Alpha_XYposAtDet(NULL),
  H_acc_mom_el(NULL),
  H_acc_pty_el(NULL),
  H_NofHitsAll(NULL),
  H_RingCenterX(NULL),
  H_RingCenterY(NULL),
  H_RingCenter(NULL),
  H_Radius(NULL), 
  H_aAxis(NULL), 
  H_bAxis(NULL), 
  H_boa(NULL), 
  H_dR(NULL),
  H_RingCenter_Aaxis(NULL),  
  H_RingCenter_Baxis(NULL),  
  H_RingCenter_boa(NULL),
  H_RingCenter_dR(NULL)
{
  /*
  fEventNum = 0;
  PointsFilled = 0;
  fMinNofHits = 7;
  nPhotonsNotOnPlane = 0;
  nPhotonsNotOnSphere = 0;
  nTotalPhorons = 0; 
  */
}

CbmRichGeoOpt::~CbmRichGeoOpt()
{

}

InitStatus CbmRichGeoOpt::Init()
{
   cout << "CbmRichGeoOpt::Init"<<endl;
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmRichGeoOpt::Init","RootManager not instantised!"); }

   fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
   if ( NULL == fRichPoints) { Fatal("CbmRichGeoOpt::Init","No RichPoint array!"); }

   fRefPoints = (TClonesArray*) ioman->GetObject("RefPlanePoint");
   if ( NULL == fRefPoints) { Fatal("CbmRichGeoOpt::Init","No fRefPoints array!"); }

   fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( NULL == fMcTracks) { Fatal("CbmRichGeoOpt::Init","No MCTrack array!"); }

   fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
   if ( NULL == fRichHits) { Fatal("CbmRichGeoTest::Init","No RichHit array!"); }

   fGP = CbmRichHitProducer::InitRootGeometry();
   
   ///////////////////////////////////////////
   fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
   if ( NULL == fRichRings) { Fatal("CbmRichGeoTest::Init","No RichRing array!"); }

   fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
   if ( NULL == fRichRingMatches) { Fatal("CbmRichGeoTest::Init","No RichRingMatch array!"); }

   /////////////// need three points on the PMT plane to determine its equation
  PlanePoints.resize(3);
  for(int p=0;p<PlanePoints.size();p++){
    PlanePoints[p].SetX(-1000.);PlanePoints[p].SetY(-1000.);PlanePoints[p].SetZ(-1000.);
  }
  InitHistograms();
  
  return kSUCCESS;
}

void CbmRichGeoOpt::Exec(Option_t* option)
{
  fEventNum++;
  cout << "#################### CbmRichGeoOpt, event No. " <<  fEventNum << endl;
  if(PointsFilled==0){
    for(int p=1;p<PlanePoints.size();p++){
      if( PlanePoints[p].X() == PlanePoints[p-1].X() ){FillPointsAtPMT();PointsFilled=0;}else{PointsFilled=1;}
    }
  }
  
  //cout << "#################### CbmRichGeoOpt, event No. " <<  fEventNum << endl;
  //Fill the coordinates of the three points on the PMT plane 
      
  PMTPlaneCenter.SetX(fGP.fPmtX); PMTPlaneCenter.SetY(fGP.fPmtY); PMTPlaneCenter.SetZ(fGP.fPmtZ);
  if(PointsFilled==1 && fEventNum<10){
    for(int p=0;p<PlanePoints.size();p++){
      cout<<"Point "<<p<< ": ("<<PlanePoints[p].X()<<" , "<< PlanePoints[p].Y()<<" , "<< PlanePoints[p].Z()<<")"<<endl;
    }
  }
  
  /////////////////////////////////////////
  if(PointsFilled==1){
    GetPMTRotAngels();
    GetPlaneCenter(fGP.fMirrorTheta*180./TMath::Pi(), RotX*180./TMath::Pi(), RotY*180./TMath::Pi());

    if(fEventNum<10){
      r1=PlanePoints[1]-PlanePoints[0]; 
      r2=PlanePoints[2]-PlanePoints[0]; 
      n=r1.Cross(r2);
      MirrorCenter.SetX(fGP.fMirrorX);MirrorCenter.SetY(fGP.fMirrorY);MirrorCenter.SetZ(fGP.fMirrorZ);
      cout<<"MirrorCenter=("<<MirrorCenter.X()<<","<<MirrorCenter.Y()<<","<<MirrorCenter.Z()<<")"<<endl;
      cout<<"r1=("<<r1.X()<<","<<r1.Y()<<","<<r1.Z()<<")"<<endl;
      cout<<"r2=("<<r2.X()<<","<<r2.Y()<<","<<r2.Z()<<")"<<endl;
      cout<<"n=("<<n.X()<<","<<n.Y()<<","<<n.Z()<<")"<<endl;
    }
    HitsAndPoints();
    RingParameters();
    FillMcHist();
  }
}

/////// Get data of hits and points
void CbmRichGeoOpt::HitsAndPoints(){
  Int_t nofRefPoints = fRefPoints->GetEntriesFast();
  Int_t nofPoints = fRichPoints->GetEntriesFast();
  if(nofPoints==0 || nofRefPoints==0){return;} if(nofPoints>1500){return;} 
  cout<<"nofPoints:  "<<  nofPoints<<endl;
  //loop over points and get momentum of photons --> calculate angle (to be done later)
  
  for (int i = 0; i < nofRefPoints; i++) {
    TVector3 PosAtRefl; TVector3 PosAtDetIn; TVector3 PosAtDetOut;
    CbmRichPoint* RefPoint = (CbmRichPoint*)fRefPoints->At(i);
    if (RefPoint == NULL) continue;
    int trackId = RefPoint->GetTrackID(); if(trackId==-2) {continue;}
 
    RefPoint->Position(PosAtRefl);
    int Zpos=int(10.*PosAtRefl.Z());//2653 or 2655 -->take 2655 which is the entrance point 
    //of the REFLECTED photon into the sensitive plane   
    //cout<<PosAtRefl.Z()<<"    "<<Zpos<<endl;
    if(Zpos==2653){continue;}
    
    
    CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(trackId);
    if(NULL == point) continue;
    PosAtDetIn.SetX(point->GetX()); PosAtDetIn.SetY(point->GetY()); PosAtDetIn.SetZ(point->GetZ());
        
    bool Checked=CheckPointLiesOnPlane(PosAtDetIn,PlanePoints[0],n);
        
    nTotalPhorons++;//nPhotonsNotOnPlane++;
    if( Checked==0 ){nPhotonsNotOnPlane++; continue; }
    //float DistMCToFocalPoint=GetDistanceMirrorCenterToPMTPoint(point);
    float Delta= GetDistanceMirrorCenterToPMTPoint(PosAtDetIn)-(fGP.fMirrorR/2.);
    float hypot= GetIntersectionPointsLS(MirrorCenter, PosAtRefl, PosAtDetIn,fGP.fMirrorR/2.);
    if(hypot==-1.){nPhotonsNotOnSphere++; continue;}
    float rho=TMath::Sqrt(hypot*hypot-Delta*Delta);

    H_dFocalPoint_Delta->Fill(Delta);
    H_dFocalPoint_Rho->Fill(rho);
    /////////// calculate the vectors on teh PMT plane

    TVector3 LineSensToPMT=PosAtDetIn-PosAtRefl;
    /////////// calculate alpha relative to the "tilted" PMT plane !!
    double Alpha=LineSensToPMT.Angle(n);//*TMath::RadToDeg();
    double AlphaInDeg=Alpha*TMath::RadToDeg();
    if(AlphaInDeg>90.){AlphaInDeg=180.-AlphaInDeg;}
    
    H_PointsIn_XY->Fill(PosAtDetIn.X(),PosAtDetIn.Y());
    //PosAtDetOut
    CbmRichHitProducer::TiltPoint(&PosAtDetIn, &PosAtDetOut, fGP.fPmtPhi, fGP.fPmtTheta, fGP.fPmtZOrig);
    H_PointsOut_XY->Fill(PosAtDetOut.X(),PosAtDetOut.Y());

    H_Alpha->Fill(AlphaInDeg);
    if(PosAtDetIn.X()<0. && PosAtDetIn.Y()>0) {
      H_Alpha_UpLeft->Fill(AlphaInDeg );
      H_Alpha_XYposAtDet->Fill(PosAtDetIn.X(),PosAtDetIn.Y(),AlphaInDeg);
      if(PosAtDetIn.X() <= PMTPlaneX && PosAtDetIn.Y() > PMTPlaneY){H_Alpha_UpLeft_Q1->Fill(AlphaInDeg ); H_Alpha_XYposAtDet_Q1->Fill(PosAtDetIn.X(),PosAtDetIn.Y(),AlphaInDeg);}
      if(PosAtDetIn.X() > PMTPlaneX && PosAtDetIn.Y() > PMTPlaneY){H_Alpha_UpLeft_Q2->Fill(AlphaInDeg ); H_Alpha_XYposAtDet_Q2->Fill(PosAtDetIn.X(),PosAtDetIn.Y(),AlphaInDeg);}
      if(PosAtDetIn.X() <= PMTPlaneX && PosAtDetIn.Y() <= PMTPlaneY){H_Alpha_UpLeft_Q3->Fill(AlphaInDeg ); H_Alpha_XYposAtDet_Q3->Fill(PosAtDetIn.X(),PosAtDetIn.Y(),AlphaInDeg);}
      if(PosAtDetIn.X() > PMTPlaneX && PosAtDetIn.Y() <= PMTPlaneY){H_Alpha_UpLeft_Q4->Fill(AlphaInDeg ); H_Alpha_XYposAtDet_Q4->Fill(PosAtDetIn.X(),PosAtDetIn.Y(),AlphaInDeg);}

      if(PosAtDetIn.X() > PMTPlaneXatThird){H_Alpha_UpLeft_LeftPart->Fill(AlphaInDeg ); H_Alpha_XYposAtDet_LeftPart->Fill(PosAtDetIn.X(),PosAtDetIn.Y(),AlphaInDeg);}
      if(PosAtDetIn.X() <= PMTPlaneXatThird){H_Alpha_UpLeft_RightPart->Fill(AlphaInDeg ); H_Alpha_XYposAtDet_RightPart->Fill(PosAtDetIn.X(),PosAtDetIn.Y(),AlphaInDeg);}
    }
  }
  
  //***********************************************************  
  Int_t nofHits = fRichHits->GetEntriesFast();
  for (Int_t iH = 0; iH < nofHits; iH++){
    CbmRichHit *hit = (CbmRichHit*) fRichHits->At(iH); if ( hit == NULL ) continue;
    Int_t pointInd =  hit->GetRefId(); if (pointInd < 0) continue;
    CbmRichPoint *point = (CbmRichPoint*) fRichPoints->At(pointInd); if ( point == NULL ) continue;
    
    H_NofPhotonsPerHit->Fill(hit->GetNPhotons());
    
    TVector3 inPos(point->GetX(), point->GetY(), point->GetZ());
    TVector3 outPos;
    CbmRichHitProducer::TiltPoint(&inPos, &outPos, fGP.fPmtPhi, fGP.fPmtTheta, fGP.fPmtZOrig);
    H_Hits_XY->Fill(hit->GetX(), hit->GetY());
    H_DiffXhit->Fill(hit->GetX() - outPos.X());
    H_DiffYhit->Fill(hit->GetY() - outPos.Y());
    
  }
}

///////////////////////////////
void CbmRichGeoOpt::RingParameters()
{
  Int_t nofRings = fRichRings->GetEntriesFast();
  for (Int_t iR = 0; iR < nofRings; iR++){
    CbmRichRing *ring = (CbmRichRing*) fRichRings->At(iR);  if (NULL == ring) continue;
    CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR); if (NULL == ringMatch) continue;
    Int_t mcTrackId = ringMatch->GetMatchedLink().GetIndex();  if (mcTrackId < 0) continue;
    CbmMCTrack* mcTrack = (CbmMCTrack*)fMcTracks->At(mcTrackId);  if (!mcTrack) continue;
    
    Int_t motherId = mcTrack->GetMotherId();
    Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
    Double_t momentum = mcTrack->GetP();
    Double_t pt = mcTrack->GetPt();
    Double_t rapidity = mcTrack->GetRapidity();
    
    if (pdg != 11 || motherId != -1) continue; // only primary electrons
        
    if (ring->GetNofHits() >= fMinNofHits){
      H_acc_mom_el->Fill(momentum);
      H_acc_pty_el->Fill(rapidity, pt);
    }
 
   ///////////////////////////////////
    float radius=ring->GetRadius();
    if(radius<=0.){continue;}//with ideal finder --> many rings with radius -1. 
    //Test if radius is a NAN:
    if(! (radius<=1. || radius >1.) ){continue;}
    float aA = ring->GetAaxis(); 
    float bA = ring->GetBaxis(); 

    H_Radius->Fill(radius); H_aAxis->Fill(aA); H_bAxis->Fill(bA); H_boa->Fill(bA/aA);
      
    float CentX=ring->GetCenterX();
    float CentY=ring->GetCenterY();
    
    H_RingCenter->Fill(CentX,CentY); 
    H_RingCenter_Aaxis->Fill(CentX,CentY,aA);
    H_RingCenter_Baxis->Fill(CentX,CentY,bA); 
    
    H_RingCenter_boa->Fill(CentX,CentY,bA/aA);
    if(CentX <= PMTPlaneX && CentY > PMTPlaneY){H_boa_Q1->Fill(bA/aA); H_RingCenter_boa_Q1->Fill(CentX,CentY,bA/aA);}
    if(CentX > PMTPlaneX && CentY > PMTPlaneY){H_boa_Q2->Fill(bA/aA); H_RingCenter_boa_Q2->Fill(CentX,CentY,bA/aA);}
    if(CentX <= PMTPlaneX && CentY <= PMTPlaneY){H_boa_Q3->Fill(bA/aA); H_RingCenter_boa_Q3->Fill(CentX,CentY,bA/aA);}
    if(CentX > PMTPlaneX && CentY <= PMTPlaneY){H_boa_Q4->Fill(bA/aA); H_RingCenter_boa_Q4->Fill(CentX,CentY,bA/aA);}
    //  if(PosAtDetIn.X() > PMTPlaneX && PosAtDetIn.Y() <= PMTPlaneY){
    
    if(CentX <=  PMTPlaneX && CentY >PMTPlaneY){H_boa_Q1->Fill(bA/aA);}
    int nAllHitsInR=ring->GetNofHits();
    H_NofHitsAll->Fill(nAllHitsInR);
    
    for(int iH=0;iH<nAllHitsInR;iH++){
      CbmRichHit* hit = (CbmRichHit*) fRichHits->At(ring->GetHit(iH));
      double xH=hit->GetX();
      double yH=hit->GetY();
      double dR=aA-TMath::Sqrt( (CentX-xH)*(CentX-xH) + (CentY-yH)*(CentY-yH) );
      H_dR->Fill(dR);
      H_RingCenter_dR->Fill(CentX,CentY,dR);
      if(CentX <= PMTPlaneX && CentY > PMTPlaneY){H_dR_Q1->Fill(dR); H_RingCenter_dR_Q1->Fill(CentX,CentY,dR);}
      if(CentX > PMTPlaneX && CentY > PMTPlaneY){H_dR_Q2->Fill(dR); H_RingCenter_dR_Q2->Fill(CentX,CentY,dR);}
      if(CentX <= PMTPlaneX && CentY <= PMTPlaneY){H_dR_Q3->Fill(dR); H_RingCenter_dR_Q3->Fill(CentX,CentY,dR);}
      if(CentX > PMTPlaneX && CentY <= PMTPlaneY){H_dR_Q4->Fill(dR); H_RingCenter_dR_Q4->Fill(CentX,CentY,dR);}
    } 
  }
}

////////////////////////////////
void CbmRichGeoOpt::FillMcHist()
{
  for (Int_t i = 0; i < fMcTracks->GetEntriesFast(); i++){
    CbmMCTrack* mcTrack = (CbmMCTrack*)fMcTracks->At(i);
    if (!mcTrack) continue;
    Int_t motherId = mcTrack->GetMotherId();
    Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
    
    if (pdg == 11 && motherId == -1){
      H_MomPrim->Fill(mcTrack->GetP());
      H_PtPrim->Fill(mcTrack->GetPt());
    }
  }

}

////////////////////////////////
void CbmRichGeoOpt::InitHistograms()
{
  int nBinsX = 28; double xMin = -110.; double xMax = 110.;  
  int nBinsY = 40; double yMin = -200; double yMax = 200.;
  
  H_MomPrim = new TH1D("H_MomPrim", "H_MomPrim;p [GeV]; Yield", 200, 0., 10.);
  H_PtPrim = new TH1D("H_PtPrim", "H_PtPrim;p [GeV]; Yield", 80, 0., 4.);
  
  
  H_Hits_XY = new TH2D("H_Hits_XY", "H_Hits_XY;X [cm];Y [cm];Counter", 150, -100., 50.,341, 0.,340.);//100, 240.,340.);//nBinsX, xMin, xMax, nBinsY, yMin, yMax);
  H_PointsIn_XY = new TH2D("H_PointsIn_XY", "H_PointsIn_XY;X [cm];Y [cm];Counter", 2001, -100., 100.,341, 0.,340.);//1001, 140.,240.);
  H_PointsOut_XY = new TH2D("H_PointsOut_XY", "H_PointsOut_XY;X [cm];Y [cm];Counter", 1101, -100., 10.,341, 0.,340.);//nBinsX, xMin, xMax, nBinsY, yMin, yMax);
  
  H_NofPhotonsPerHit = new TH1D("H_NofPhotonsPerHit", "H_NofPhotonsPerHit;Number of photons per hit;Yield", 10, -0.5, 9.5);
  H_DiffXhit = new TH1D("H_DiffXhit", "H_DiffXhit;Y_{point}-Y_{hit} [cm];Yield", 200, -1., 1.);
  H_DiffYhit = new TH1D("H_DiffYhit", "H_DiffYhit;Y_{point}-Y_{hit} [cm];Yield", 200, -1., 1.);
 
  H_Alpha= new TH1D("H_Alpha","H_Alpha;#alpha_{photon-PMT} [deg];Yield",360,0.,180.);
  H_Alpha_UpLeft= new TH1D("H_Alpha_UpLeft","H_Alpha_UpLeft;#alpha_{photon-PMT} [deg];Yield",180,0.,90.);

  H_Alpha_UpLeft_Q1= new TH1D("H_Alpha_UpLeft_Q1","H_Alpha_UpLeft_Q1;#alpha_{photon-PMT} [deg];Yield",180,0.,90.);
  H_Alpha_UpLeft_Q2= new TH1D("H_Alpha_UpLeft_Q2","H_Alpha_UpLeft_Q2;#alpha_{photon-PMT} [deg];Yield",180,0.,90.);
  H_Alpha_UpLeft_Q3= new TH1D("H_Alpha_UpLeft_Q3","H_Alpha_UpLeft_Q3;#alpha_{photon-PMT} [deg];Yield",180,0.,90.);
  H_Alpha_UpLeft_Q4= new TH1D("H_Alpha_UpLeft_Q4","H_Alpha_UpLeft_Q4;#alpha_{photon-PMT} [deg];Yield",180,0.,90.);
  H_Alpha_UpLeft_LeftPart= new TH1D("H_Alpha_UpLeft_LeftPart","H_Alpha_UpLeft_LeftPart;#alpha_{photon-PMT} [deg];Yield",180,0.,90.);
  H_Alpha_UpLeft_RightPart= new TH1D("H_Alpha_UpLeft_RightPart","H_Alpha_UpLeft_RightPart;#alpha_{photon-PMT} [deg];Yield",180,0.,90.);

  H_Alpha_XYposAtDet= new TH3D("H_Alpha_XYposAtDet","H_Alpha_XYposAtDet; X [cm]; Y [cm];#alpha_{photon-PMT} [deg];Yield",270, -90, 0,  450, 50,200, 180,0.,90.);
  H_Alpha_XYposAtDet_Q1= new TH3D("H_Alpha_XYposAtDet_Q1","H_Alpha_XYposAtDet_Q1; X [cm]; Y [cm];#alpha_{photon-PMT} [deg];Yield",270, -90, 0,  450, 50,200, 180,0.,90.);
  H_Alpha_XYposAtDet_Q2= new TH3D("H_Alpha_XYposAtDet_Q2","H_Alpha_XYposAtDet_Q2; X [cm]; Y [cm];#alpha_{photon-PMT} [deg];Yield",270, -90, 0,  450, 50,200, 180,0.,90.);
  H_Alpha_XYposAtDet_Q3= new TH3D("H_Alpha_XYposAtDet_Q3","H_Alpha_XYposAtDet_Q3; X [cm]; Y [cm];#alpha_{photon-PMT} [deg];Yield",270, -90, 0,  450, 50,200, 180,0.,90.);
  H_Alpha_XYposAtDet_Q4= new TH3D("H_Alpha_XYposAtDet_Q4","H_Alpha_XYposAtDet_Q4; X [cm]; Y [cm];#alpha_{photon-PMT} [deg];Yield",270, -90, 0,  450, 50,200, 180,0.,90.);
  H_Alpha_XYposAtDet_LeftPart= new TH3D("H_Alpha_XYposAtDet_LeftPart","H_Alpha_XYposAtDet_LeftPart; X [cm]; Y [cm];#alpha_{photon-PMT} [deg];Yield",270, -90, 0,  450, 50,200, 180,0.,90.);
  H_Alpha_XYposAtDet_RightPart= new TH3D("H_Alpha_XYposAtDet_RightPart","H_Alpha_XYposAtDet_RightPart; X [cm]; Y [cm];#alpha_{photon-PMT} [deg];Yield",270, -90, 0,  450, 50,200, 180,0.,90.);

  //////////////////////////////////////
  H_dFocalPoint_Delta= new TH1D("H_dFocalPoint_Delta","H_dFocalPoint_Delta;#Delta_{f} [mm];Yield",200,-20.,20.);
  H_dFocalPoint_Rho= new TH1D("H_dFocalPoint_Rho","H_dFocalPoint_Rho;#rho_{f} [mm];Yield",150,50.,200.);


  //////////////////////////////////////
  // Detector acceptance efficiency vs. (pt,y) and p
  H_acc_mom_el = new TH1D("H_acc_mom_el", "H_acc_mom_el;p [GeV/c];Yield", 24, 0., 12.);
  H_acc_pty_el = new TH2D("H_acc_pty_el", "H_acc_pty_el;Rapidity;P_{t} [GeV/c];Yield",25, 0., 4., 20, 0., 3.);
  //////////////////////////////////

  H_NofHitsAll = new TH1D("H_NofHitsAll", "H_NofHitsAll;Nof hits in ring;Yield", 50, 0., 50.);
  
  /////////////////////////////////////////////
  H_Radius= new TH1D("H_Radius","H_Radius",401, 2.,6.);
  H_aAxis= new TH1D("H_aAxis","H_aAxis",801, 2.,10.);
  H_bAxis= new TH1D("H_bAxis","H_bAxis",801, 2.,10.);
  H_boa= new TH1D("H_boa","H_boa",500, 0.5,1.);
  H_boa_Q1= new TH1D("H_boa_Q1","H_boa_Q1",500, 0.5,1.);
  H_boa_Q2= new TH1D("H_boa_Q2","H_boa_Q2",500, 0.5,1.);
  H_boa_Q3= new TH1D("H_boa_Q3","H_boa_Q3",500, 0.5,1.);
  H_boa_Q4= new TH1D("H_boa_Q4","H_boa_Q4",500, 0.5,1.);
  H_boa_LeftPart= new TH1D("H_boa_LeftPart","H_boa_LeftPart",500, 0.5,1.);
  H_boa_RightPart= new TH1D("H_boa_RightPart","H_boa_RightPart",500, 0.5,1.);
  

  H_dR= new TH1D("H_dR","H_dR",101,-5.0,5.0);  
  H_dR_Q1= new TH1D("H_dR_Q1","H_dR_Q1",500, 0.5,1.);
  H_dR_Q2= new TH1D("H_dR_Q2","H_dR_Q2",500, 0.5,1.);
  H_dR_Q3= new TH1D("H_dR_Q3","H_dR_Q3",500, 0.5,1.);
  H_dR_Q4= new TH1D("H_dR_Q4","H_dR_Q4",500, 0.5,1.);
  H_dR_LeftPart= new TH1D("H_dR_LeftPart","H_dR_LeftPart",500, 0.5,1.);
  H_dR_RightPart= new TH1D("H_dR_RightPart","H_dR_RightPart",500, 0.5,1.);


  H_RingCenter= new TH2D("H_RingCenter","H_RingCenter",1001, -100., 0.,2501, 50.,300.);
  
  H_RingCenter_Aaxis= new TH3D("H_RingCenter_Aaxis","H_RingCenter_Aaxis",301, -100, 0,301, 200, 300, 401, 2.,10.);
  H_RingCenter_Baxis= new TH3D("H_RingCenter_Baxis","H_RingCenter_Baxis",301, -100, 0,301, 200, 300, 401, 2.,10.);
  H_RingCenter_boa= new TH3D("H_RingCenter_boa","H_RingCenter_boa",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_boa_Q1= new TH3D("H_RingCenter_boa_Q1","H_RingCenter_boa_Q1",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_boa_Q2= new TH3D("H_RingCenter_boa_Q2","H_RingCenter_boa_Q2",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_boa_Q3= new TH3D("H_RingCenter_boa_Q3","H_RingCenter_boa_Q3",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_boa_Q4= new TH3D("H_RingCenter_boa_Q4","H_RingCenter_boa_Q4",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_boa_LeftPart= new TH3D("H_RingCenter_boa_LeftPart","H_RingCenter_boa_LeftPart",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_boa_RightPart= new TH3D("H_RingCenter_boa_RightPart","H_RingCenter_boa_RightPart",301, -100, 0,301, 200, 300, 25, 0.5,1.);

  H_RingCenter_dR= new TH3D("H_RingCenter_dR","H_RingCenter_dR",301, -100, 0,301, 200, 300, 251, -0.5,0.5);
  H_RingCenter_dR_Q1= new TH3D("H_RingCenter_dR_Q1","H_RingCenter_dR_Q1",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_dR_Q2= new TH3D("H_RingCenter_dR_Q2","H_RingCenter_dR_Q2",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_dR_Q3= new TH3D("H_RingCenter_dR_Q3","H_RingCenter_dR_Q3",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_dR_Q4= new TH3D("H_RingCenter_dR_Q4","H_RingCenter_dR_Q4",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_dR_LeftPart= new TH3D("H_RingCenter_dR_LeftPart","H_RingCenter_dR_LeftPart",301, -100, 0,301, 200, 300, 25, 0.5,1.);
  H_RingCenter_dR_RightPart= new TH3D("H_RingCenter_dR_RightPart","H_RingCenter_dR_RightPart",301, -100, 0,301, 200, 300, 25, 0.5,1.);
 
  
}
//////////////////////////////////////////////////////////
void CbmRichGeoOpt::WriteHistograms(){
 
  H_MomPrim->Write(); 
  H_PtPrim->Write(); 

  H_Hits_XY->Write(); 
  H_PointsIn_XY->Write();
  H_PointsOut_XY->Write();
  H_NofPhotonsPerHit->Write(); 
  H_DiffXhit->Write();
  H_DiffYhit->Write();
 
  H_Alpha->Write();
  H_Alpha_UpLeft->Write();
  H_Alpha_UpLeft_Q1->Write();
  H_Alpha_UpLeft_Q2->Write();
  H_Alpha_UpLeft_Q3->Write();
  H_Alpha_UpLeft_Q4->Write();
  H_Alpha_UpLeft_LeftPart->Write();
  H_Alpha_UpLeft_RightPart->Write();

  H_Alpha_XYposAtDet->Write();
  H_Alpha_XYposAtDet_Q1->Write();
  H_Alpha_XYposAtDet_Q2->Write();
  H_Alpha_XYposAtDet_Q3->Write();
  H_Alpha_XYposAtDet_Q4->Write();
  H_Alpha_XYposAtDet_LeftPart->Write();
  H_Alpha_XYposAtDet_RightPart->Write();


  H_acc_mom_el->Write();
  H_acc_pty_el->Write();
  
  H_dFocalPoint_Delta->Write(); 
  H_dFocalPoint_Rho->Write();
  H_NofHitsAll->Write();
  
  H_Radius->Write();
  H_aAxis->Write();
  H_bAxis->Write();
  
  H_boa->Write();
  H_boa_Q1->Write(); 
  H_boa_Q2->Write(); 
  H_boa_Q3->Write(); 
  H_boa_Q4->Write(); 
  H_boa_LeftPart->Write(); 
  H_boa_RightPart->Write();

  H_dR->Write();
  H_dR_Q1->Write(); 
  H_dR_Q2->Write(); 
  H_dR_Q3->Write(); 
  H_dR_Q4->Write(); 
  H_dR_LeftPart->Write(); 
  H_dR_RightPart->Write();

  H_RingCenter->Write();
  H_RingCenter_Aaxis->Write();
  H_RingCenter_Baxis->Write();
  
  H_RingCenter_boa->Write();
  H_RingCenter_boa_Q1->Write();
  H_RingCenter_boa_Q2->Write();
  H_RingCenter_boa_Q3->Write();
  H_RingCenter_boa_Q4->Write();
  H_RingCenter_boa_LeftPart->Write();
  H_RingCenter_boa_RightPart->Write();
  
  H_RingCenter_dR->Write();
  H_RingCenter_dR->Write();
  H_RingCenter_dR_Q1->Write();
  H_RingCenter_dR_Q2->Write();
  H_RingCenter_dR_Q3->Write();
  H_RingCenter_dR_Q4->Write();
  H_RingCenter_dR_LeftPart->Write();
  H_RingCenter_dR_RightPart->Write();
}
//////////////////////////////////////////////////////////////
///////////////////////////////
void CbmRichGeoOpt::FillPointsAtPMT()
{
  
  for(int p=0;p<PlanePoints.size();p++){
    if(PlanePoints[p].X() != -1000.){
      if(p==0){continue;}
      else{
	int PointFilled=1;
	for(int p2=p-1;p2>-1;p2--){
	  if(TMath::Abs( PlanePoints[p2].X() - PlanePoints[p].X() ) < 1.0){PointFilled=0;}
	}
	if(PointFilled==1){continue;}
      }
    }
    
    //fEventNum++;
    Int_t nofPoints = fRichPoints->GetEntriesFast();
    
    for(Int_t ip = 0; ip < nofPoints-10; ip+=10){
      CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(ip);
      if(NULL == point) continue;
      int trackId = point->GetTrackID(); if(trackId==-2) continue;
      if(point->GetX()>=0 || point->GetY()<=0){continue;}

      PlanePoints[p].SetX(point->GetX());PlanePoints[p].SetY(point->GetY());PlanePoints[p].SetZ(point->GetZ());
      if(PlanePoints[p].X() !=-1000.){break;}
    }
  }
}

//////////////////////////////////////////////////////


//////////////////////////////////////////////////////
float  CbmRichGeoOpt::GetIntersectionPointsLS( TVector3 MirrCenter,  TVector3 G_P1,  TVector3 G_P2, float R){
  float A=(G_P1-MirrCenter)*(G_P1-MirrCenter);
  float B=(G_P2-G_P1)*(G_P2-G_P1);
  float P=2.*( (G_P1-MirrCenter)*(G_P2-G_P1) )/(B);
  float q=(A-R*R)/B;

  float t1=-1.*P/2.-TMath::Sqrt( (P/2.)*(P/2.) -q);
  float t2=-1.*P/2.+TMath::Sqrt( (P/2.)*(P/2.) -q);
  //cout<<"t1="<<t1<<",  t2="<<t2<<endl;
  //Check if nan --> no intersection
  if(! (t1==1. || t1 >1.) ){return -1.;}
  //cout<<"t1="<<t1<<",  t2="<<t2<<endl;

  TVector3 IntersectP1;  TVector3 IntersectP2;
  IntersectP1.SetX( G_P1.X()+t1*(G_P2.X()-G_P1.X()) );
  IntersectP1.SetY( G_P1.Y()+t1*(G_P2.Y()-G_P1.Y()) );
  IntersectP1.SetZ( G_P1.Z()+t1*(G_P2.Z()-G_P1.Z()) );

  IntersectP2.SetX( G_P1.X()+t2*(G_P2.X()-G_P1.X()) );
  IntersectP2.SetY( G_P1.Y()+t2*(G_P2.Y()-G_P1.Y()) );
  IntersectP2.SetZ( G_P1.Z()+t2*(G_P2.Z()-G_P1.Z()) );

  TVector3 Line1=IntersectP1-G_P1;
  float Length1=TMath::Sqrt(Line1.X()*Line1.X() + Line1.Y()*Line1.Y() + Line1.Z()*Line1.Z());
  TVector3 Line2=IntersectP2-G_P1;
  float Length2=TMath::Sqrt(Line2.X()*Line2.X() + Line2.Y()*Line2.Y() + Line2.Z()*Line2.Z());
  
  //return Length1<Length2 ?  Length1 :  Length2;
  if(Length1<Length2){return Length1;}else{return Length2;}
 }
//////////////////////////////////////////////////////
float  CbmRichGeoOpt::GetDistanceMirrorCenterToPMTPoint(TVector3 PMTpoint)
{
  float XTerm= (PMTpoint.X()-MirrorCenter.X())*(PMTpoint.X()-MirrorCenter.X());
  float YTerm= (PMTpoint.Y()-MirrorCenter.Y())*(PMTpoint.Y()-MirrorCenter.Y());
  float ZTerm= (PMTpoint.Z()-MirrorCenter.Z())*(PMTpoint.Z()-MirrorCenter.Z());
  return TMath::Sqrt(XTerm+YTerm+ZTerm);

}
//////////////////////////////////////////////////////
bool  CbmRichGeoOpt::CheckPointLiesOnPlane(TVector3 Point,TVector3 p0,TVector3 norm )
{
  double TolaratedDiff=0.001;
  double ProdP0WithNorm=p0.Dot(norm);// cout<<"ProdP0WithNorm = "<<ProdP0WithNorm;
  double ProdPWithNorm=Point.Dot(norm); //cout<<"  ProdPWithNorm = "<<ProdPWithNorm<<endl;
  return TMath::Abs(ProdP0WithNorm - ProdPWithNorm) <= ( (TMath::Abs(ProdP0WithNorm) < TMath::Abs(ProdPWithNorm) ? TMath::Abs(ProdPWithNorm) : TMath::Abs(ProdP0WithNorm)) * TolaratedDiff);
}
//////////////////////////////////////////////////////
void CbmRichGeoOpt::GetPMTRotAngels()
{
  RotX=fGP.fPmtTheta; RotY=fGP.fPmtPhi;
}

//////////////////////////////////////////////////////
void  CbmRichGeoOpt::GetPlaneCenter(float rotMir, float rotX, float rotY)
{
  float MinX=-110., MaxX=0., MinY=120., MaxY=220.;
  
  if(rotMir < 1.1 && rotMir > 0.9 ){//rotMir==1){
    
    //   if( (rotX < 0.05 && rotX > -0.05 ) && (rotY < 0.05 && rotY > -0.05 ) ){MinX=-110., MaxX=0., MinY=120., MaxY=220.;  }
    if( rotX < 5 && (rotY < 0.05 && rotY > -0.05 ) ){MinX=-110., MaxX=0., MinY=120., MaxY=220.;  }
    else if(rotX == 0 && rotY == 5 ){MinX=-110., MaxX=0., MinY=120., MaxY=220.;  }
    else{MinX=-110., MaxX=0., MinY=120., MaxY=220.;  }
    
  }else if(rotMir < -9.9 && rotMir > -10.1 ){//rotMir==-10){
    
    //   if( (rotX <= 5) && (rotY <= 5 ) ){MinX=-110., MaxX=-1.5, MinY=125., MaxY=196.;  }
    if( (rotY <= 5 ) ){MinX=-110., MaxX=-1.5, MinY=125., MaxY=196.;  }
    else if( (rotY > 5 && rotY <=25 ) ){MinX=-105., MaxX=-5., MinY=125., MaxY=197.;  }
    else if( (rotY > 25 && rotY <=30 ) ){MinX=-101., MaxX=-9., MinY=125., MaxY=199.;  }
    else if( (rotY > 30 && rotY <=35 ) ){MinX=-100., MaxX=-13., MinY=125., MaxY=199.;  }
    else if( (rotY > 35 && rotY <=40 ) ){MinX=-100., MaxX=-16., MinY=125., MaxY=199.;  }
    else if( (rotY > 40 && rotY <=45 ) ){MinX=-99., MaxX=-20., MinY=125., MaxY=200.;  }
    else if( (rotY > 45) ){MinX=-94., MaxX=-25., MinY=125., MaxY=200.;  }
    
  }
  
  PMTPlaneX=MinX+(MaxX-MinX)/2.; PMTPlaneY=MinY+(MaxY-MinY)/2.;
  PMTPlaneXatThird=MinX+(MaxX-MinX)/3.; PMTPlaneYatThird=MinY+(MaxY-MinY)/3.;//; double PMTPlaneYatThird;
}

//////////////////////////////////////////////////////
bool  CbmRichGeoOpt::CheckPointLiesOnSphere(TVector3 Point)
{
  

}

//////////////////////////////////////////////////////
bool  CbmRichGeoOpt::CheckLineIntersectsPlane(TVector3 Point)
{


}
//////////////////////////////////////////////////////
bool  CbmRichGeoOpt::CheckLineIntersectsSphere(TVector3 Point)
{


}
//////////////////////////////////////////////////////
void CbmRichGeoOpt::Finish()
{
  cout<<nPhotonsNotOnPlane<<" out of "<<nTotalPhorons<<" are not on the plane("<<float(nPhotonsNotOnPlane)/float(nTotalPhorons)<<")"<<endl;
  cout<<nPhotonsNotOnSphere<<" out of "<<nTotalPhorons<<" are not on the ideal sphere("<<float(nPhotonsNotOnSphere)/float(nTotalPhorons)<<")"<<endl;
  WriteHistograms();
}



ClassImp(CbmRichGeoOpt)

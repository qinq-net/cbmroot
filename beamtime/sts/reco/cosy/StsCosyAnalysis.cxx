#include <iostream>

#include "TClonesArray.h"

#include "FairRootManager.h"
#include "FairLogger.h"
#include "FairRunAna.h"

#include "StsCosyAnalysis.h"
#include "CbmStsHit.h"
#include "CbmStsAddress.h"

#include "TMath.h"
#include "TF1.h"
#include "TDirectory.h"
#include "TGraphErrors.h"
#include "TTree.h"

#include <map>

using std::cout;
using std::endl;



StsCosyAnalysis::StsCosyAnalysis() :
  FairTask("StsCosyAnalysis",1),
  fHits(NULL),
  fChi2X(1e9),
  fChi2Y(1e9),
  fNofDet(5),
  fTrackSelectType(0),
  fEventSelectType(0) 
{ 
  fChain = new TChain("cbmsim");
  fTimeLimit[0] = 16;
  fTimeLimit[1] = 16;
  fTimeLimit[2] = 16;
  
  fTimeShift[0] = 0;
  fTimeShift[1] = -16;
  fTimeShift[2] = -16;  
  fXbin = 300; // 300 µm
  fYbin = 400; // 400 µm
    
  fCutName = "";
}

StsCosyAnalysis::~StsCosyAnalysis() 
{
  if ( fHits ) {
    fHits->Delete();
    delete fHits;
  }
}


InitStatus StsCosyAnalysis::Init() {

  // Get RootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman )
    {
      LOG(FATAL) << "StsCosyAnalysis::Init: noRootManager !" << FairLogger::endl; 
      return kFATAL;
    }
  // Get input arrays
  
  fHits =(TClonesArray *)  ioman->GetObject("StsHit");
  if ( ! fHits ) {
    LOG(ERROR) << "No StsHits array!" << FairLogger::endl;
    LOG(ERROR) << "Task will be inactive" << FairLogger::endl;
    return kERROR;
  }
  
  for(int i=0;i<fNofDet;i++)
    fMapPts.insert(make_pair(i,vector<CbmStsHit*>()));  
  
   for (Int_t i=0;i<3;i++)
     {
       Char_t inName[25]; 
       sprintf(inName,"ResX%i",i); 
       resX[i] = new TH1F(inName,inName,200, -0.1,0.1);
       
       sprintf(inName,"ResY%i",i); 
       resY[i] = new TH1F(inName,inName,200, -0.1,0.1);   
              
       sprintf(inName,"XY_STS_%i",i);
       if(i==0 || i==2)XY[i] = new TH2F(inName,inName,100, -1, 1,100, -1, 1);
       else XY[i] = new TH2F(inName,inName,100, -1.5, 1.5,100, -1.5, 1.5);
       
       sprintf(inName,"XvsResX_STS_%i",i);
       X_ResX[i] = new TH2F(inName,inName,400, -1, 1,400, -0.1, 0.1);
       
       sprintf(inName,"YvsResY_STS_%i",i);
       Y_ResY[i] = new TH2F(inName,inName,400, -1, 1,400, -0.1, 0.1);
     }

     XX_STS_0_1 = new TH2F("XX_STS_0_1","XX_STS_0_1",100,-1,1,100,-1,1);
     XX_STS_0_2 = (TH2F*)XX_STS_0_1->Clone("XX_STS_0_2");XX_STS_0_2->SetTitle("XX_STS_0_2");
     XX_STS_0_2woSTS1 = (TH2F*)XX_STS_0_1->Clone("XX_STS_0_2woSTS1");XX_STS_0_2woSTS1->SetTitle("XX_STS_0_2woSTS1");     
     XX_STS_2_1 = (TH2F*)XX_STS_0_1->Clone("XX_STS_2_1");XX_STS_2_1->SetTitle("XX_STS_2_1");
     
     YY_STS_0_1 = new TH2F("YY_STS_0_1","YY_STS_0_1",100,-1.5,1.5,100,-1.5,1.5);
     YY_STS_0_2 = (TH2F*)YY_STS_0_1->Clone("YY_STS_0_2");YY_STS_0_2->SetTitle("YY_STS_0_2");
     YY_STS_0_2woSTS1 = (TH2F*)YY_STS_0_1->Clone("YY_STS_0_2woSTS1");YY_STS_0_2woSTS1->SetTitle("YY_STS_0_2woSTS1");
     YY_STS_2_1 = (TH2F*)YY_STS_0_1->Clone("YY_STS_2_1");YY_STS_2_1->SetTitle("YY_STS_2_1");
          
    chi2X = new TH1F("chi2X","chi2X",100, 0, 1000);
    chi2XY = new TH2F("chi2XY","chi2XY",300, -10, 300,300, -10, 300);
    chi2Y = (TH1F*)chi2X->Clone("chi2Y");chi2Y->SetTitle("chi2Y");
    
    XY_woSTS1[0] = new TH2F("XY_STS0_woSTS1","XY_STS0_woSTS1",100, -1, 1,100, -1, 1);
    XY_woSTS1[1] = new TH2F("XY_STS2_woSTS1","XY_STS2_woSTS1",100, -1, 1,100, -1, 1);
    
    Int_t nXbins = 30000/fXbin;
    Int_t nYbins = 30000/fYbin;
    
    XY_eff = new TH2F("XYforEff_STS1","XYforEff_STS1",nXbins, -1.5, 1.5, nYbins, -1.5, 1.5);
    XY_STS1projection = (TH2F*)XY_eff->Clone("XY_STS1projection");XY_STS1projection->SetTitle("XY_STS1projection");
    XY_STS1projection_select = (TH2F*)XY_eff->Clone("XY_STS1projection_select");XY_STS1projection_select->SetTitle("XY_STS1projection_select");

    NofTracks =  new TH1F("NofTracks","NofTracks", 101, -0.5, 100.5);
    
  if(fCutName != "" ){
    LOG(INFO) << "Use inpute file for time cuts " << fCutName << FairLogger::endl;
    TFile *file_cuts = new TFile(fCutName);
   
   
   TTree *timeHit0 = (TTree*)file_cuts ->Get("STS0-STS1");
   timeHit0->SetBranchAddress("TimeLimit",&fTimeLimit[0]);   
   timeHit0->SetBranchAddress("TimeShift",&fTimeShift[0]);   
   timeHit0->GetEntry(0);

   TTree *timeHit1 = (TTree*)file_cuts ->Get("STS0-STS2");
   timeHit1->SetBranchAddress("TimeLimit",&fTimeLimit[1]);   
   timeHit1->SetBranchAddress("TimeShift",&fTimeShift[1]);   
   timeHit1->GetEntry(0);

   TTree *timeHit2 = (TTree*)file_cuts ->Get("STS2-STS1");
   timeHit2->SetBranchAddress("TimeLimit",&fTimeLimit[2]);   
   timeHit2->SetBranchAddress("TimeShift",&fTimeShift[2]);   
   timeHit2->GetEntry(0);
   file_cuts->Close();
  }  
  
  TFile *file = new TFile(fAlignName); 
  if ( !file  ) {
    LOG(ERROR) << "No alignment file!" << FairLogger::endl;
    return kERROR;
  }
   
   TTree *tree = (TTree*)file->Get("Alignment");
   
   tree->SetBranchAddress("dX0",&fX[0]);
   tree->SetBranchAddress("dX1",&fX[1]);
   tree->SetBranchAddress("dX2",&fX[2]);
   
   tree->SetBranchAddress("dY0",&fY[0]);
   tree->SetBranchAddress("dY1",&fY[1]);
   tree->SetBranchAddress("dY2",&fY[2]);

   tree->GetEntry(0);
   file->Close();
   
   fEvent = 0;
   
   cout << "-I- StsCosyAnalysis: Intialisation successfull " << kSUCCESS<< endl;
   return kSUCCESS;

}

// -----   Public method Exec   --------------------------------------------
void StsCosyAnalysis::Exec(Option_t* opt) {
    
  Reset(); 
  CbmStsHit* hit = NULL;
  Int_t nofHits = fHits->GetEntries();
  
  if(fEvent%100000 == 0)cout << "-I- StsCosyAnalysis: ----- " << fEvent << endl; 
  
  // Loop over hits
  for (Int_t iHit=0; iHit<nofHits; iHit++) 
    {
      hit = (CbmStsHit*) fHits->At(iHit);
      
      // Determine sector type and channel numbers
      Int_t detId = CbmStsAddress::GetElementId(hit->GetAddress(),kStsStation);
      vector<CbmStsHit*>& vlist =  fMapPts[detId];
      vlist.push_back((CbmStsHit*) hit); 
    } 
      

  // Find the tracks 
  FindTracks();
  
  fEvent++;
     
}


Bool_t StsCosyAnalysis::FindTracks(){
  
  vector<CbmStsHit*>& v_sts0 =  fMapPts[0];
  vector<CbmStsHit*>& v_sts1 =  fMapPts[1];
  vector<CbmStsHit*>& v_sts2 =  fMapPts[2];
    
  CbmStsHit* pt0=NULL; 
  CbmStsHit* pt1=NULL; 
  CbmStsHit* pt2=NULL; 
  
  TVector3 sts0_pos, sts1_pos, sts2_pos;
  int Npoints = 3;
  
  double x[Npoints];
  double ex[Npoints];
  double z[Npoints];
  double ez[Npoints];
  double z2[Npoints];
  double ez2[Npoints];
  double y[Npoints];
  double ey[Npoints];
  
  Double_t Xnew;
  Double_t Ynew;
  
  for(Int_t i=0; i< v_sts0.size() ; i++) 
    {
      pt0 = (CbmStsHit*) v_sts0[i];
      pt0->Position(sts0_pos); 
      x[0]= sts0_pos.X()-fX[0];
      y[0]= sts0_pos.Y()-fY[0];
      XY_woSTS1[0]->Fill(x[0],y[0]);
    }
  for(Int_t i=0; i< v_sts2.size() ; i++) 
    {
      pt0 = (CbmStsHit*) v_sts2[i];
      pt0->Position(sts2_pos); 
      x[0]= sts2_pos.X()-fX[2];
      y[0]= sts2_pos.Y()-fY[2];
      XY_woSTS1[1]->Fill(x[0],y[0]);
    }
	      
  if((fEventSelectType == 1 && (v_sts0.size()==1 && v_sts2.size()==1)) || fEventSelectType == 0)// = 1 - selection of the 1-hit events    
  for(Int_t i=0; i< v_sts0.size() ; i++) 
    {
      
Int_t tracks = 0;

Double_t Chi2X = 1e9;
Double_t Chi2Y = 1e9;
Double_t ResX[3];
Double_t ResY[3];
Double_t Xarr[3];
Double_t Yarr[3];

Bool_t trigger = kFALSE;


      pt0 = (CbmStsHit*) v_sts0[i];
      pt0->Position(sts0_pos); 
      x[0]= sts0_pos.X()-fX[0];      
      y[0]= sts0_pos.Y()-fY[0];
      z[0]= sts0_pos.Z();
      ex[0]= pt0->GetDx();
      ey[0]= pt0->GetDy();
      ez[0]= 0.5;
      
	  for(Int_t k=0; k< v_sts2.size() ; k++) 
	    {
	      pt2 = (CbmStsHit*) v_sts2[k];
	      
	      if(TMath::Abs(pt0->GetTime()-pt2->GetTime()- fTimeShift[1])>fTimeLimit[1])continue;
	      pt2->Position(sts2_pos); 
	      x[2]= (sts2_pos.X()-fX[2]);
	      y[2]= sts2_pos.Y()-fY[2];	    
	      z[2]= sts2_pos.Z();
	      ex[2]= pt2->GetDx();
	      ey[2]= pt2->GetDy();
	      ez[2]= 0.5;
	      
double xx[2], yy[2], zz[2], dxx[2], dyy[2], dzz[2];

	      xx[0]=x[0];
	      yy[0]=y[0];
	      zz[0]=z[0];

	      xx[1]=x[2];
	      yy[1]=y[2];
	      zz[1]=z[2];

	      dxx[0]=ex[0];
	      dyy[0]=ey[0];
	      dzz[0]=ez[0];

	      dxx[1]=ex[2];
	      dyy[1]=ey[2];
	      dzz[1]=ez[2];	
	      
	      TGraphErrors* graph;
	      TGraphErrors* graph1;   
	      graph = new TGraphErrors(2,zz,xx,dzz,dxx);   
	      graph1 = new TGraphErrors(2,zz,yy,dzz,dyy);
	      TF1 *fit = new TF1("fit", "[0] + [1]*x");	      
	      graph->Fit("fit","QME");
	      TF1 *fit1 = new TF1("fit1", "[0] + [1]*x");
	      graph1->Fit("fit1","QME");
	
	      XX_STS_0_2woSTS1->Fill(x[0],x[2]);
	      YY_STS_0_2woSTS1->Fill(y[0],y[2]);
	
	      Xnew = fit->GetParameter(0)+fit->GetParameter(1)*99.3;
	      Ynew = fit1->GetParameter(0)+fit1->GetParameter(1)*99.3;
	      
	      XY_STS1projection->Fill(Xnew,Ynew);
	      
	      proj_events.push_back(fEvent);
	      X_proj.push_back(Xnew);
	      Y_proj.push_back(Ynew);
	      	      
	      graph1->Delete();
	      graph->Delete();
	      fit->Delete();
 	      fit1->Delete();
	      	      
      if(fEventSelectType == 1 && v_sts1.size()!=1) continue;   // selection of 1-hit events
	
      for(Int_t j=0; j< v_sts1.size() ; j++) 
	{
	  pt1 = (CbmStsHit*) v_sts1[j];
	  if(TMath::Abs(pt0->GetTime()-pt1->GetTime() - fTimeShift[0])>fTimeLimit[0])continue;
	  if(TMath::Abs(pt2->GetTime()-pt1->GetTime()- fTimeShift[2])>fTimeLimit[2])continue;
	  pt1->Position(sts1_pos); 
	  x[1]= sts1_pos.X()-fX[1];
	  y[1]= sts1_pos.Y()-fY[1];	    
	  z[1]= sts1_pos.Z();
	  ex[1]= pt1->GetDx();
	  ey[1]= pt1->GetDy();
	  ez[1]= 0.5;

	      // Fit data numerically:
	      // ---------------------
	      TGraphErrors* gr;
	      TGraphErrors* gr1;
	      gr = new TGraphErrors(Npoints,z,x,ez,ex);
	      
	      TF1 *fitt = new TF1("fitt", "[0] + [1]*x");	      
	      gr->Fit("fitt","QME");
	      
	      double xChi2 = fitt->GetChisquare();
	      double xNdof = fitt->GetNDF();
	      double p0 = fitt->GetParameter(0);
	      double p1 = fitt->GetParameter(1);
	      double Prob = fitt->GetProb();
	      
	      gr1 = new TGraphErrors(Npoints,z,y,ez,ey);

	      TF1 *fitt1 = new TF1("fitt1", "[0] + [1]*x");
	      gr1->Fit("fitt1","QME");
	      
	      double yChi2 = fitt1->GetChisquare();
	      double yNdof = fitt1->GetNDF();
	      double yProb = fitt1->GetProb();
	      double p2 = fitt1->GetParameter(0);
	      double p3 = fitt1->GetParameter(1);
	      
	      gr1->Delete();
	      gr->Delete();
	      fitt->Delete();
 	      fitt1->Delete();
	      	      
	      if(xChi2/xNdof > fChi2X || yChi2/yNdof > fChi2Y)continue; 
	
if(fTrackSelectType == 1) // selection of the best track per event (best chi2/ndf)
{
  if(Chi2X > xChi2/xNdof && Chi2Y > yChi2/yNdof)
  {
    Chi2X = xChi2/xNdof;  Chi2Y = yChi2/yNdof;
    for(int m=0; m<3; m++)
    {
      Xarr[m]=x[m];ResX[m]=x[m]-p0-p1*z[m];
      Yarr[m]=y[m];ResY[m]=y[m]-p2-p3*z[m];
      trigger = kTRUE;          
    }
  }
}
else if(fTrackSelectType == 0) // use all tracks
{	
	for(int m=0; m<3; m++)
	{
	  resX[m]->Fill(x[m]-p0-p1*z[m]);
	  resY[m]->Fill(y[m]-p2-p3*z[m]);
	  
	  X_ResX[m]->Fill(x[m],x[m]-p0-p1*z[m]);
	  Y_ResY[m]->Fill(x[m],y[m]-p2-p3*z[m]);
	  XY[m]->Fill(x[m],y[m]);
	}  
	      XY_events.push_back(fEvent);
	      X_STS1.push_back(x[1]);
	      Y_STS1.push_back(y[1]);

	      chi2Y->Fill(Chi2Y);
	      chi2X->Fill(Chi2X);
	      
	      XX_STS_0_1->Fill(x[0],x[1]);
	      XX_STS_0_2->Fill(x[0],x[2]);
	      XX_STS_2_1->Fill(x[2],x[1]);
	      YY_STS_0_1->Fill(y[0],y[1]);
	      YY_STS_0_2->Fill(y[0],y[2]);
	      YY_STS_2_1->Fill(y[2],y[1]);
}
tracks++;

	    }//!sts2
	}//!sts1
	NofTracks->Fill(tracks);
	if(tracks==0)continue;
	if(!trigger)continue;
	if(fTrackSelectType == 1)
	{
	  for(int m=0; m<3; m++)
	  {
	    resX[m]->Fill(ResX[m]);
	    resY[m]->Fill(ResY[m]);	  
    
	    X_ResX[m]->Fill(Xarr[m],ResX[m]);
	    Y_ResY[m]->Fill(Yarr[m],ResY[m]);
	    XY[m]->Fill(Xarr[m],Yarr[m]);
	  }
	   	
	      XY_events.push_back(fEvent);
	      X_STS1.push_back(Xarr[1]);
	      Y_STS1.push_back(Yarr[1]);

	  chi2Y->Fill(Chi2Y);
	  chi2X->Fill(Chi2X);
	  chi2XY->Fill(Chi2X,Chi2Y);
	
	  XX_STS_0_1->Fill(Xarr[0],Xarr[1]);
	  XX_STS_0_2->Fill(Xarr[0],Xarr[2]);
	  XX_STS_2_1->Fill(Xarr[2],Xarr[1]);
	  YY_STS_0_1->Fill(Yarr[0],Yarr[1]);
	  YY_STS_0_2->Fill(Yarr[0],Yarr[2]);
	  YY_STS_2_1->Fill(Yarr[2],Yarr[1]);
	}
    }//!sts0
    
  
  return kTRUE;
}


void StsCosyAnalysis::Reset() {
  fMapPts.clear();
}


void StsCosyAnalysis::Finish(){
 
  Double_t dX[3], dY[3];
    
  for(Int_t i=0;i<3;i++)
  {
    TF1 *fitX = new TF1("fitX", "gaus");
    resX[i]->Fit("fitX","QME");
    dX[i] = fitX->GetParameter(2);
    
    TF1 *fitY = new TF1("fitY", "gaus");
    resY[i]->Fit("fitY","QME");
    dY[i] = fitY->GetParameter(2);
    
    fitX->Delete();
    fitY->Delete();
  }
    
 TTree * data = new TTree("Resolution",	"Resolution");
	 data->Branch("dX0", &dX[0], "dX0/D");
	 data->Branch("dX1", &dX[1], "dX1/D");
	 data->Branch("dX2", &dX[2], "dX2/D");

	 data->Branch("dY0", &dY[0], "dY0/D");
	 data->Branch("dY1", &dY[1], "dY1/D");
	 data->Branch("dY2", &dY[2], "dY2/D");

	 data->Fill();
	 data->Write();    

  for(Int_t i=0;i<3;i++){resX[i]->Write(); X_ResX[i]->Write(); resY[i]->Write();Y_ResY[i]->Write();} 
  for(Int_t i=0;i<3;i++){XY[i]->Write();}
  
    XY_woSTS1[0]->Write();  
    XY_woSTS1[1]->Write();  

    XX_STS_0_1->Write();
    YY_STS_0_1->Write();
    
    XX_STS_0_2->Write();
    YY_STS_0_2->Write();
    
    XX_STS_2_1->Write();
    YY_STS_2_1->Write();  
    
    XX_STS_0_2woSTS1->Write();
    YY_STS_0_2woSTS1->Write();

    chi2X->Write();
    chi2Y->Write();
    chi2XY->Write();

    NofTracks->Write();

  Double_t NofProjections = 0;
  
  vector <Int_t> select_events;
  vector <Double_t> X_select, Y_select;
  
  for(Int_t i=0; i< proj_events.size(); i++)for(Int_t j=0; j< XY_events.size(); j++)
  {
    if(TMath::Abs(X_STS1[j]-X_proj[i]) > fXbin*1e-4 || TMath::Abs(Y_STS1[j]-Y_proj[i]) > fYbin*1e-4)continue;
    select_events.push_back(proj_events[i]);
    X_select.push_back(X_proj[i]);
    Y_select.push_back(Y_proj[i]);
    XY_STS1projection_select->Fill(X_proj[i],Y_proj[i]);
  }
  for(Int_t j=0; j< XY_events.size(); j++)XY_eff->Fill(X_STS1[j],Y_STS1[j]);
  
  XY_eff->Write();
  XY_STS1projection->Write();
  XY_STS1projection_select->Write();
  
  for(Int_t i=0; i< select_events.size(); i++)for(Int_t j=0; j< XY_events.size(); j++)
  {
    if(select_events[i] != XY_events[i])continue;
    NofProjections++;
  }  
  
  Double_t NofP = select_events.size() + (XY_events.size()-NofProjections);
  
  LOG(INFO) << FairLogger::endl;
  cout << "Acceptance  procedure: event selection" << endl;
  cout << "Limits " << fXbin << " um x " << fYbin << " um" << endl;  
  cout << "----------------- Number of hits in STS3 = " << XY_events.size() << endl;
  cout << "Number of projections in STS3 acceptance = " << NofP << endl;
  cout << "------------------------------ Efficency = " << XY_events.size()/NofP*100. << "%" << endl;
  
  
  LOG(INFO) << FairLogger::endl;
  LOG(INFO) << "=====================================================" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Finish" << FairLogger::endl;
  LOG(INFO) << "=====================================================" << FairLogger::endl;
  LOG(INFO) << FairLogger::endl;
}




ClassImp(StsCosyAnalysis)

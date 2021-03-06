void pl_pull_trk(Int_t NSt=8, Int_t iVar=0, Int_t iFit=0){
  //  TCanvas *can = new TCanvas("can22","can22");
  //  can->Divide(2,2); 
  TCanvas *can = new TCanvas("can","can",50,0,800,800);
  switch(NSt){
  case 6:
  case 5:
  case 4:
    can->Divide(3,3);
    break; 
  case 18:
    can->Divide(4,5);
    break; 
  default:
    can->Divide(4,4);
    ; 
  }
 gPad->SetFillColor(0);
 gStyle->SetPalette(1);
 gStyle->SetOptStat(kTRUE);

 gROOT->cd();
 gROOT->SetDirLevel(1);

 TH1 *h;
 TH1 *h1;
 TH2 *h2;

 const Int_t MSt=30;
 Double_t vSt[MSt];
 Double_t vMean[MSt];
 Double_t vSig[MSt];
 Double_t vRes[MSt];
 Double_t vStErr[MSt];
 Double_t vMeanErr[MSt];
 Double_t vSigErr[MSt];
 Double_t vResErr[MSt];
 // if (h!=NULL) h->Delete();
 Int_t iCan=1;
 TString var;
 Double_t Nall;

 switch(iVar){
 case 0:
   var="X";
   break;
 case 1:
   var="Y";
   break;
 case 2:
   var="Z";
   break;
 case 3:
   var="T";
   break;
 case 4:
   var="TB";
   break;
 }

 for (Int_t iSt=0; iSt<NSt; iSt++) {
 can->cd(iCan++);
 gROOT->cd();
 TString hname=Form("hPull%s_Station_%d",var.Data(),iSt);
  h1=(TH1 *)gROOT->FindObjectAny(hname);
  if (h1!=NULL) {
      h1->Draw("");
      Nall=h1->GetEntries();
      gPad->SetLogy();
      gPad->SetGridx();
      if (iFit>0){
	Double_t dFMean=h1->GetMean();
	Double_t dFLim=3.0*h1->GetRMS();
	TFitResultPtr fRes=h1->Fit("gaus","S","",dFMean-dFLim,dFMean+dFLim);
	//cout << " fRes = "<< fRes <<endl;
	if( -1 == fRes) return;
	vSt[iSt]=iSt;
	vMean[iSt]=fRes->Parameter(1);
	vSig[iSt]=fRes->Parameter(2);
	vStErr[iSt]=0.;
	vMeanErr[iSt]=fRes->ParError(1);
	vSigErr[iSt]=fRes->ParError(2);
	//vSig[iSt]=TMath::Max(20.,vSig[iSt]);
      }
  }else 
   {
     cout << hname << " not found" << endl;
   }
 }
 if(0==iFit) return;
 can->cd(iCan++);
 Double_t dLMargin=0.35;
 Double_t dTitOffset=1.8;
 gPad->SetLeftMargin(dLMargin);
 TGraphErrors *grm = new TGraphErrors(NSt, vSt, vMean, vStErr, vMeanErr);
 grm->SetTitle("Mean");
 grm->GetXaxis()->SetTitle("Station number");
 switch(iVar){
 case 0:
 case 1:
 case 2:
 grm->GetYaxis()->SetTitle("mean deviation (cm)");
   break;
 default:
 grm->GetYaxis()->SetTitle("mean deviation (ns)");
 }
 grm->GetYaxis()->SetTitleOffset(dTitOffset);
 grm->GetXaxis()->SetLimits(-0.5,NSt-0.5);
 grm->SetMarkerStyle(24);
 grm->Draw("APLE");

 can->cd(iCan++);
 gPad->SetLeftMargin(dLMargin);
 TGraphErrors *grs = new TGraphErrors(NSt, vSt, vSig, vStErr, vSigErr);
 grs->SetTitle("Gaussian width");
 grs->GetXaxis()->SetTitle("Station number");
 switch(iVar){
 case 0:
 case 1:
 case 2:
 grs->GetYaxis()->SetTitle("Gaussian sigma (cm)");
   break;
 default:
 grs->GetYaxis()->SetTitle("Gaussian sigma (ns)");
 }
 grs->GetYaxis()->SetTitleOffset(dTitOffset);
 grs->GetXaxis()->SetLimits(-0.5,NSt-0.5);
 grs->SetMarkerStyle(24);
 grs->Draw("APLE");

 can->cd(iCan++);
 gPad->SetLeftMargin(dLMargin);
 Double_t val=(NSt-1)*(NSt-1);
 TMatrixD a(NSt,NSt); 
 for(Int_t i=0; i<NSt; i++) for(Int_t j=0; j<NSt; j++) {
     if (i==j) {
       a[i][j]=1;
     }else{
       a[i][j]=1./val;
     }
   }
 a.Draw("colz");
 a.Print();

 // can->cd(iCan++);
 TMatrixD ainv=a; 
 ainv.Invert();
 ainv.Draw("colz");
 ainv.Print();
 TMatrixD aSig(NSt,1);
 for(Int_t i=0; i<NSt; i++) aSig[i][0]=vSig[i]*vSig[i];

 cout << "Measured gaussian widths: "<<endl; 
 aSig.Print();
 TMatrixD xRes = ainv * aSig;
 cout << "Resolution of counters: "<<endl; 
 xRes.Print();

 //can->cd(iCan++);
 for(Int_t i=0; i<NSt; i++) {
   vRes[i]=TMath::Sqrt(TMath::Abs(xRes[i][0]));
   vResErr[i] = vSigErr[i];
 }
 TGraphErrors *grr = new TGraphErrors(NSt, vSt, vRes, vStErr, vResErr);
 grr->SetTitle("Final resolution");
 grr->GetXaxis()->SetTitle("Station number");
 switch(iVar){
 case 0:
 case 1:
 case 2:
 grr->GetYaxis()->SetTitle("resolution (cm)");
   break;
 default:
 grr->GetYaxis()->SetTitle("resolution (ns)");
 }
 grr->GetYaxis()->SetTitleOffset(dTitOffset);
 grr->GetXaxis()->SetLimits(-0.5,NSt-0.5);
 //grr->GetXaxis()->SetRangeUser(-0.5,NSt-0.5);
 grr->SetMarkerStyle(24);
 grr->Draw("APLE");

 for(Int_t i=0; i<NSt; i++) 
   cout<<Form("GMean %6.3f +/- %6.5f, GSig: %6.3f +/- %6.5f => ResC %d: %6.3f ",vMean[i],vMeanErr[i],vSig[i],vSigErr[i],i,vRes[i])<<endl;

 cout << "Res-summary "<<iVar<<": Nall, sigs = "<<Nall;
 for (Int_t i=0; i<NSt; i++) cout << Form(", %7.4f",vRes[i]);
 cout <<endl; 

 can->SaveAs(Form("pl_pull_trk_%s%02d.pdf",var.Data(),NSt));
}

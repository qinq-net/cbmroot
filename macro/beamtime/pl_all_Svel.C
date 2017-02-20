void pl_all_Svel(Int_t iNSt=6)
{
  //  TCanvas *can = new TCanvas("can22","can22");
  //  can->Divide(2,2); 
  //  TCanvas *can = new TCanvas("can","can",48,55,700,900);
  TCanvas *can = new TCanvas("can","can",48,56,900,900);
  can->Divide(2,3,0.01,0.01); 
  //  can->Divide(2,2,0,0); 
  Float_t lsize=0.06;

 gPad->SetFillColor(0);
 gStyle->SetPalette(1);
 gStyle->SetLabelSize(lsize);

 //gStyle->SetOptStat(kTRUE);
 //gROOT->cd();
 //gROOT->SetDirLevel(2);
 gStyle->SetPadLeftMargin(0.3);
 gStyle->SetTitleOffset(1.4, "y");

 TH1 *h;
 TH2 *h2;
 const Int_t   iType[6]={5,4,6,2,9,8};

 Int_t iCanv=0;
 // if (h!=NULL) h->Delete();

 for(Int_t iSt=0; iSt<iNSt; iSt++){

    can->cd(iCanv+1); iCanv++;
    gROOT->cd();
    TString hname=Form("cl_SmT%01d_Svel",iType[iSt]);
    h=(TH1 *)gROOT->FindObjectAny(hname);
    if (h!=NULL) {
     h->Draw("");
     h->SetMinimum(0.8);
     h->SetMaximum(1.2);
     h->UseCurrentStyle();
     //     gPad->SetLogy();
    }else{cout<<"Histogram "<<hname<<" not existing. "<<endl;}
 }
  can->SaveAs(Form("pl_all_Svel.pdf"));
} 

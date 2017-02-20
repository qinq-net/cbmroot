void pl_XY_trk(Int_t NSt=4){
  //  TCanvas *can = new TCanvas("can22","can22");
  //  can->Divide(2,2); 
  TCanvas *can = new TCanvas("can","can",50,0,800,800);
  can->Divide(3,NSt); 

 gPad->SetFillColor(0);
 gStyle->SetPalette(1);
 gStyle->SetOptStat(kTRUE);

 gROOT->cd();
 gROOT->SetDirLevel(1);

 TH1 *h;
 TH1 *h1;
 TH2 *h2; 
 TH2 *h2m;
 TString hname="";

 // if (h!=NULL) h->Delete();
 Int_t iCan=1;

 for (Int_t iSt=0; iSt<NSt; iSt++) {
 can->cd(iCan++);
 gROOT->cd();
 hname=Form("hXY_AllStations_%d",iSt);

  h2=(TH2 *)gROOT->FindObjectAny(hname);
  if (h2!=NULL) {
      h2->Draw("colz");
      TH2D *h2acc=(TH2D *)h2->Clone(Form("Acc_%d",iSt));
      h2acc->Divide(h2,h2,1.,1.,"B"); 
      Double_t Nall=h2->GetEntries();

      can->cd(iCan++);
      gROOT->cd();
      hname=Form("hXY_MissedStation_%d",iSt);
      h2m=(TH2 *)gROOT->FindObjectAny(hname);
      if (h2m!=NULL) {
      h2m->Draw("colz");
      Double_t Nmis=h2->GetEntries();
      Double_t NmisI=h2->Integral();
      TH2D *h2missed=(TH2D *)h2->Clone(Form("Missed_%d",iSt));
      h2missed->Multiply(h2m,h2acc,1.,1.,"B");
      Double_t NmisaI=h2missed->Integral();
      Double_t eff=1. - NmisaI/(Nall+NmisaI);
      cout << "Effficiency of Station "<<iSt<<": all "<<Nall<<", mis "<<Nmis<<", "<<NmisI<<", "<<NmisaI
	   <<" -> "<<eff<<endl;

      can->cd(iCan++);
      TH2D *h2frac=(TH2D *)h2->Clone(Form("FractionMissed_%d",iSt));
      TH2D *h2tot=(TH2D *)h2m->Clone(Form("Total_%d",iSt));
      h2tot->Add(h2tot,h2,1.,1.);
      h2frac->Divide(h2missed,h2tot,1.,1.,"B");
      h2frac->Draw("colz");
      h2frac->SetTitle(Form("Fraction of hits missing, station %d",iSt));
      h2frac->SetMaximum(0.8);
      
      TPad *newpad=new TPad("newpad","a transparent pad",0,0,1,1);
      newpad->SetFillStyle(4000);
      newpad->Draw();
      newpad->cd();
      TPaveLabel *tit = new TPaveLabel(0.2,0.75,0.45,0.9,Form(" <eff>: %5.2f",eff));
      tit->SetFillColor(0);
      tit->SetTextFont(52);
      tit->Draw();
      }
 }else 
   {
     cout << hname << " not found" << endl;
   }
 }
 can->SaveAs("pl_XY_trk.pdf");

}

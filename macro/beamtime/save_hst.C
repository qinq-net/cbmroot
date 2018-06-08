void save_hst(TString cstr="status.hst.root", Bool_t bROOT = kFALSE){
 cout << "save all histograms to file "<<cstr.Data()<<endl;
 TList* tHistoList(NULL);
 if(bROOT)
 {
  tHistoList = gROOT->GetList();
 }
 else
 {
  tHistoList = gDirectory->GetList();
 }
 TIter next(tHistoList);
 // Write histogramms to the file
 TFile *fHist = new TFile(cstr,"RECREATE");
 {
  TH1 *h;
  TObject* obj;
  while( (obj= (TObject*)next()) ){
      if(obj->InheritsFrom(TH1::Class())){
         h = (TH1*)obj;
	 //         cout << "Write histo " << h->GetTitle() << endl;
         h->Write();
      }
  }
 }
 // fHist->ls();
 fHist->Close();
}

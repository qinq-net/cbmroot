/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Invariant Mass Spectra									       //
//									                               //
//      Authors: Omveer Singh (omveer.rs@amu.ac.in), Partha Pratim Bhaduri (partha.bhaduri@vecc.gov.in)//
//      and Ekata Nandy (ekata@vecc.gov.in)			                                       //
//												       //
//												       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  // Specification of cut values 
  Int_t MUCHhits = 11;
  Int_t STShits = 7;
  Double_t MUCHchi2 = 1.3;
  Double_t STSchi2 = 1.2;
  Double_t Vchi2 = 2;

 
  
  Int_t nSigFiles =1;
 
 gStyle->SetCanvasColor(10);
 gStyle->SetFrameFillColor(10);
 gStyle->SetHistLineWidth(4);
 gStyle->SetPadColor(10);
 gStyle->SetStatColor(10);
 gStyle->SetPalette(1);
 gStyle->SetPaintTextFormat("2.1f");

 TDatabasePDG* dataBase = TDatabasePDG::Instance();

 TClonesArray *Mu, *diMu, *MCTrack;
  
CbmAnaMuonCandidate *mu, *mu1, *mu2;
CbmMCTrack *mct = new CbmMCTrack();
TTree *InputTree, *InputTreeMC;

 char fileout[256];

sprintf(fileout,"invmass_jpsi.root");//Output File Name

 TFile *FFF = new TFile(fileout,"recreate");

TH1D *invM_sgn = new TH1D("invM_sgn","invM_mass for jPsi meson",180,0.2,5.0);
(invM_sgn->GetXaxis())->SetTitle("m_{inv} (GeV/c^{2})");
(invM_sgn->GetYaxis())->SetTitle("counts");
invM_sgn->SetLineColor(kBlack);




 TLorentzVector P1, P2, P3, M;
 TLorentzVector *Mi;
 Int_t NofEvent;
 Int_t NofFile;



//---------------------------------------------- 
 

TTree *Plus2 = new TTree("Plus2","part-");
Plus2->Branch("P0", &P1(0), "Px/D:Py:Pz:E", 10000000);
Plus2->Branch("NofEvent", &NofEvent, "NofEvent/I", 10000000);
Plus2->Branch("NofFile", &NofFile, "NofFile/I", 10000000);

TTree *Minus2 = new TTree("Minus2","part+");
Minus2->Branch("P0", &P1(0), "Px/D:Py:Pz:E", 10000000);
Minus2->Branch("NofEvent", &NofEvent, "NofEvent/I", 10000000);
Minus2->Branch("NofFile", &NofFile, "NofFile/I", 10000000);



 TFile *f;
 TFile *fMC;

 Int_t nEvents = 100;
 
 Int_t nParts;


 Int_t nF = 0;

 Int_t nPart = 0;

 nF = 0;

 Float_t embed_effect = 1.0;

int k;
 for(k=1; k<=nSigFiles; k++)
   {
     if(k%50 == 0)cout << k << endl;

     char name[]="/home/om/CbmRoot/AUG18/macro/much/data/much_analysis.root"; //Much Analysis Output File
     
     f = new TFile(name);
     if (f->IsZombie() || f->GetNkeys() < 1)continue;
     
     InputTree = (TTree*)f->Get("cbmsim"); 
     InputTree->SetBranchAddress("MuCandidates", &Mu); 
     NofFile = k;


cout<< "Entries In Tree =   " <<InputTree->GetEntries() <<endl;
     //----------------------------------------------------------- 
 
     for(int iEvent=0; iEvent<InputTree->GetEntries(); iEvent++)//Event Loop
       {	
	 InputTree->GetEntry(iEvent);
	 nParts = Mu->GetEntriesFast();
	 nF++;
	 NofEvent=iEvent;
	

	 for (int iPart=0; iPart<nParts; iPart++)
	   {
	     mu = (CbmAnaMuonCandidate*)Mu->At(iPart);
	     //if(mu->GetTrueMu() == 0)continue;
	      if(mu->GetNStsHits() < STShits)continue;
	     if(mu->GetNMuchHits() < MUCHhits)continue;
	//     if(mu->GetChiMuch() > MUCHchi2)continue;
	//     if(mu->GetChiToVertex() > Vchi2)continue;
	//    if(mu->GetChiSTS() > STSchi2)continue;
	//     Int_t NMuHits =mu->GetNMuchHits();
	     Mi = mu->GetMomentumRC();
	      P1=*Mi;
	     
	     if(mu->GetSign() < 0)Minus2->Fill();
	     else Plus2->Fill();
	   }
       }
     f->Close();
   }  //k signal loop
 
 Int_t nF1, nF2, nE1, nE2;
 
 Plus2->SetBranchAddress("P0", &P1(0));
 Minus2->SetBranchAddress("P0", &P2(0));

 Plus2->SetBranchAddress("NofEvent", &nE1);
 Minus2->SetBranchAddress("NofEvent", &nE2);
 
 Plus2->SetBranchAddress("NofFile", &nF1);
 Minus2->SetBranchAddress("NofFile", &nF2);

  
 Int_t entriesPlus2  = Plus2->GetEntries();
 Int_t entriesMinus2 = Minus2->GetEntries();

cout<< "No. of Mu Plus=   " << entriesPlus2 << endl;
cout <<"No. of Mu Minus=     " << entriesMinus2 <<endl;

 //==========================signal invm calculation==========================

 for(int jPart=0; jPart<entriesPlus2; jPart++)
   {
  
       Plus2->GetEntry(jPart);
     for(int iPart=0; iPart<entriesMinus2; iPart++) 
       {
	 Minus2->GetEntry(iPart);   		
	 if(nF1 != nF2)continue;
       
	 if(nE1 != nE2)continue;
         M = P1 + P2;
         invM_sgn->Fill(M.M());
       }
   }

TCanvas* cv = new TCanvas; 
//invM_sgn->Scale(1/nF);
invM_sgn->Draw();
invM_sgn->Write();
 

FFF->Write();



 cout<<" Macro complete "<<endl;
}




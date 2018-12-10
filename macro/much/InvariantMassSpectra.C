/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Invariant Mass Spectra									       //
//									                               //
//      Authors: Omveer Singh (omveer.rs@amu.ac.in), Partha Pratim Bhaduri (partha.bhaduri@vecc.gov.in)//
//      and Ekata Nandy (ekata@vecc.gov.in)			                                       //
//												       //
//												       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void InvariantMassSpectra(
			  Int_t nEvents=100,
			  TString dataset = "data/test",
			  TString setup = "sis100_muon_jpsi")
{

  TString inFile = dataset + ".analysis.root";
  TString outFile =dataset+ "."+ setup + ".invariantmass.root";


  // this macro's name for screen output
  TString myName = "InvariantMassSpectra";  
  
  
  // Specification of cut values 
  Int_t MUCHhits = 11;
  Int_t STShits = 7;
  Double_t MUCHchi2 = 1.3;
  Double_t STSchi2 = 1.2;
  Double_t Vchi2 = 2;
  
  
  // setting canvas attributes
  gStyle->SetCanvasColor(10);
  gStyle->SetFrameFillColor(10);
  gStyle->SetHistLineWidth(4);
  gStyle->SetPadColor(10);
  gStyle->SetStatColor(10);
  gStyle->SetPalette(1);
  gStyle->SetPaintTextFormat("2.1f");


  // Define trees for plus and minus muons
  Int_t NofEvent,NofFile;
  TLorentzVector P1;
  TTree *Plus = new TTree("Plus","part-");
  Plus->Branch("P0", &P1(0), "Px/D:Py:Pz:E", 10000000);
  Plus->Branch("NofEvent", &NofEvent, "NofEvent/I", 10000000);
  Plus->Branch("NofFile", &NofFile, "NofFile/I", 10000000);
  
  TTree *Minus = new TTree("Minus","part+");
  Minus->Branch("P0", &P1(0), "Px/D:Py:Pz:E", 10000000);
  Minus->Branch("NofEvent", &NofEvent, "NofEvent/I", 10000000);
  Minus->Branch("NofFile", &NofFile, "NofFile/I", 10000000);
  
  
  //Read Input File 
  TFile *fInputFile = new TFile(inFile,"READ");
  if ( ! fInputFile ) {
    cout << "*** "<<myName<<": Input file " << fInputFile << " not found!\n"
	 << "Be sure to run much_analysis.C before for the respective MUCH geometry!" << endl;
  }
  
  
  // Input Tree and Branch     
  TTree* InputTree = (TTree*) fInputFile->Get("cbmsim") ;
  if (!InputTree) {delete fInputFile; return;} // just a precaution
  TClonesArray *Mu = new TClonesArray("CbmAnaMuonCandidate");
  InputTree->SetBranchAddress("MuCandidates", &Mu); 
  
  
  //Event Loop
  int firstEvent=0;
  cout<<myName<<": macro run for "<<nEvents<<" events"<<endl;
  for (Int_t event = firstEvent; event < (firstEvent + nEvents) && event<InputTree->GetEntriesFast(); event++){	
    InputTree->GetEntry(event);
    if ( 0 == event%1000 )  cout <<endl<< "*** Processing event "<< event << endl;
    
    Int_t nParts = Mu->GetEntriesFast();
    NofEvent=event;
    
    //Muon Candidate Loop
    for (int iPart=0; iPart<nParts; iPart++){
      CbmAnaMuonCandidate *mu = (CbmAnaMuonCandidate*)Mu->At(iPart);
      //if(mu->GetTrueMu() == 0)continue;
      if(mu->GetNStsHits() < STShits)continue;
      if(mu->GetNMuchHits() < MUCHhits)continue;
      //if(mu->GetChiMuch() > MUCHchi2)continue;
      //if(mu->GetChiToVertex() > Vchi2)continue;
      //if(mu->GetChiSTS() > STSchi2)continue;
      //Int_t NMuHits =mu->GetNMuchHits();
      TLorentzVector  *Mi = mu->GetMomentumRC();
      P1=*Mi;
      if(mu->GetSign() < 0)Minus->Fill();
      else Plus->Fill();
    }
  }
  
  fInputFile->Close();// close input file
  
  
  //Set Branch of Tree Plus and Minus
  Int_t  nE1, nE2;
  TLorentzVector Pplus,Pminus;
  Plus->SetBranchAddress("P0", &Pplus(0));
  Minus->SetBranchAddress("P0", &Pminus(0));
  Plus->SetBranchAddress("NofEvent", &nE1);
  Minus->SetBranchAddress("NofEvent", &nE2);
  
  
  cout<<endl<< "No. of Mu Plus:   " << Plus->GetEntries() << endl;
  cout <<"No. of Mu Minus:    " << Minus->GetEntries() <<endl;
  
  
  // define histogram
  TH1D *invM_sgn = new TH1D("invM_sgn","invM_mass for jPsi meson",180,0.2,5.0);
  (invM_sgn->GetXaxis())->SetTitle("m_{inv} (GeV/c^{2})");
  (invM_sgn->GetYaxis())->SetTitle("counts");
  invM_sgn->SetLineColor(kBlack);
  
  
  //Invariant Mass Calculation Loop
  for(int jPart=0; jPart<Plus->GetEntries(); jPart++)
    {
      Plus->GetEntry(jPart);
      for(int iPart=0; iPart<Minus->GetEntries(); iPart++) 
	{
	  Minus->GetEntry(iPart);   		
	  if(nE1 != nE2)continue; //for same event
	  TLorentzVector M = Pplus + Pminus;
	  invM_sgn->Fill(M.M()); //invariant mass
	}
    }
  
  
  // Open output file
  TFile *output = new TFile(outFile,"recreate");
  output->cd();

  
  //Plotting the result
  TCanvas* can = new TCanvas(); 
  invM_sgn->Draw();
  invM_sgn->Write();
  
  
  //close output file
  output->Close(); 
  
  
  cout<<endl<<"Macro Finished Successfully "<<endl;
  cout<<endl<<"Input File: "<<inFile<<endl;
  cout<< "Output File: "    << outFile << endl;
  
}




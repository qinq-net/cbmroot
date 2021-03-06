#if !defined(__CINT__) || defined(__MAKECINT__)
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TH1.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TProfile2D.h"

#include "FairRadLenPoint.h"

#include <iostream>
#include <vector>
using std::cout;
using std::endl;
using std::vector;
#endif

// need to set this root include path:
// include path: -I/opt/cbm/fairsoft_jul15p1/installation/include/root -I/opt/cbm/fairroot_v-15.07-fairsoft_jul15p1/include -I/opt/cbm/fairsoft_jul15p1/installation/include
//.include $SIMPATH/include
//.include $FAIRROOTPATH/include

//Int_t matbudget_ana(Int_t nEvents = 10      , const char* stsGeo = "v16v")
//Int_t matbudget_ana(Int_t nEvents = 1000000 , const char* stsGeo = "v16v")
Int_t matbudget_ana(Int_t nEvents = 10000000, const char* stsGeo = "v16v")
{
	
  // Input file (MC)
  TString stsVersion(stsGeo);
  TString inFile = "data/matbudget." + stsVersion + ".mc.root";
  TFile* input = new TFile(inFile);
	if ( ! input ) {
		cout << "*** matbudget_ana: Input file " << inFile << " not found!\n"
		     << "Be sure to run matbudget_mc.C before for the respective STS geometry!" << endl;
		exit(1);
	}

	// Output file (material maps)
	TString outFile = "sts_matbudget_" + stsVersion + ".root";

	// Input tree and branch
  TTree* tree = (TTree*) input->Get("cbmsim") ;
  TClonesArray* points = new TClonesArray("FairRadLenPoint");
  tree->SetBranchAddress("RadLen", &points);

  // Create output histograms
  TH1D* hisRadLen = new TH1D("hisRadLen","Radiation Length", 1000,0,100);
  const int nStations = 8;       // number of STS stations
  const int nBins     = 1000;    // number of bins in histograms (in both x and y)
  const int rMax      = 55;      // maximal radius for histograms (for both x and y)
  const double zRange = 1.4;
  TProfile2D* hStationRadLen[nStations];
  TProfile2D* hStsRadLen;
  double StsRadThick = 0;
  
  TString stsname = "Material Budget x/X_{0} [%], STS";
    hStsRadLen = new TProfile2D(stsname, stsname, nBins,-rMax, rMax, nBins,-rMax, rMax);

  for ( int i = 0; i < nStations; ++i ) {
    TString hisname = "Radiation Thickness [%],";
    hisname += " Station";
    hisname += i+1;
    TString name = "Material Budget x/X_{0} [%],";
    name += " Station ";
    name += i+1;
    hStationRadLen[i] = new TProfile2D(hisname, name, nBins,-rMax, rMax, nBins,-rMax, rMax);
  }
  
  // Auxiliary variables
  TVector3 vecs,veco;
  std::map<int,int> trackHitMap;


  // Event loop
  int firstEvent = 0;
  for (Int_t event = firstEvent;
  		 event < (firstEvent + nEvents) && event<tree->GetEntriesFast();
  		 event++) {
    tree->GetEntry(event);
    if ( 0 == event%10000 )  cout << "*** Processing event "<< event << endl;

    const int nTracks = 1;
    std::vector<double> RadLengthOnTrack (nTracks,0.0); //trackID, vector with points on track
    std::vector<double> TrackLength (nTracks,0.0); //trackID, vector with points on track

    vector<double> RadThick(nStations,0);
    Double_t x=0;
    Double_t y=0;

    // For this implementation to be correct, there should be only one MCTrack per event.

    // Point loop
    for (Int_t iPoint = 0; iPoint < points->GetEntriesFast(); iPoint++) {
      FairRadLenPoint *point=(FairRadLenPoint*) points->At(iPoint);

      // Get track position at entry and exit of material
      TVector3 posIn, posOut, posDif;
      posIn  = point->GetPosition();
      posOut = point->GetPositionOut();
      posDif = posOut - posIn;

      // Midpoint between in and out
      const TVector3 middle = (posOut + posIn);
      x = middle.X()/2;
      y = middle.Y()/2;
      const double z = middle.Z()/2;
      
      // Material budget per unit length
      const double radThick = posDif.Mag() / point->GetRadLength();
      RadLengthOnTrack[point->GetTrackID()] += radThick;
      TrackLength[point->GetTrackID()] += posDif.Mag();

      // Determine station number
      int iStation    =  posIn.Z()/10  - 3 + 0.5; // suppose equidistant stations at 30-100 cm
      int iStationOut =  posOut.Z()/10 - 3 + 0.5;
      if ( iStationOut != iStation) continue;
      if ( iStation >= nStations || iStation < 0 ) continue;
      RadThick[iStation] += radThick;
    }
    
    StsRadThick = 0;
    // Fill material budget map for each station
    for ( int i = 0; i < nStations; ++i )
    {
      hStationRadLen[i]->Fill( x, y, RadThick[i]*100 );
      StsRadThick += RadThick[i];
    }

    hStsRadLen->Fill( x, y, StsRadThick*100 );
    
    for (int k = 0; k < RadLengthOnTrack.size(); k++)
      if (RadLengthOnTrack[k] > 0)
      	hisRadLen->Fill(RadLengthOnTrack[k]);

  } // event loop



  // Plotting the results
//single  TCanvas* can1 = new TCanvas("c","c",800,800);
  TCanvas* can1 = new TCanvas("c","c",1600,800);
  can1->Divide(nStations/2,2);
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);

  // Open output file
  TFile* output = new TFile(outFile, "RECREATE");
  output->cd();
  
  for ( int iStation = 0; iStation < nStations; iStation++) {
    can1->cd(iStation+1);
//single    int iStation = 7;
    hStationRadLen[iStation]->GetXaxis()->SetTitle("x [cm]");
    hStationRadLen[iStation]->GetYaxis()->SetTitle("y [cm]");
    //hStationRadLen[iStation]->GetZaxis()->SetTitle("x/X_{0} [%]");
    //hStationRadLen[i]->GetZaxis()->SetTitle("radiation thickness [%]");
    hStationRadLen[iStation]->SetAxisRange(0, zRange, "Z");
    hStationRadLen[iStation]->Draw("colz");
    hStationRadLen[iStation]->Write();
  }
  
  // Plot file
  TString plotFile = "sts_" + stsVersion + "_matbudget.png";
  can1->SaveAs(plotFile);

  //================================================================
  
  // Plotting the results
  TCanvas* can2 = new TCanvas("c","c",800,800);
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);

  can2->cd();
  hStsRadLen->GetXaxis()->SetTitle("x [cm]");
  hStsRadLen->GetYaxis()->SetTitle("y [cm]");
  hStsRadLen->SetAxisRange(0, 10, "Z");
  hStsRadLen->Draw("colz");

  // Plot file
  plotFile = "sts_" + stsVersion + "_total_matbudget.png";
  can2->SaveAs(plotFile);

  //================================================================
  
  TString thisStation(0);
  can2->Clear();
  for ( int iStation = 0; iStation < nStations; iStation++) {
    hStationRadLen[iStation]->Draw("colz");
    // Plot file
    thisStation.Form("%d",iStation+1);
    plotFile = "sts_" + stsVersion + "_station_" + thisStation + "_matbudget.png";
    can2->SaveAs(plotFile);
  }

  // Close files
  input->Close();
  output->Close();
  cout << "Material budget maps written to " << outFile << endl;

  return 0;
}


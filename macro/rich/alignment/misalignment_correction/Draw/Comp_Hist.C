#include <iomanip>
#include <vector>
#include "TH1.h"
#include "TH2D.h"

void Comp_Hist()
{
	TH2D *Aligned;
	TH2D *Misaligned;
	TString Histo_Name = "fh_ring_track_distance_vs_mom_truematch";

	TString Path_Histo = "/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned/merge.root";
	TFile* Histograms = new TFile(Path_Histo, "READ");
	Aligned = (TH2D*) gDirectory->Get(Histo_Name);

	TString Path_Histo = "/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned/merge.root";
	TFile* Histograms = new TFile(Path_Histo, "READ");
	Misaligned = (TH2D*) gDirectory->Get(Histo_Name);

	TCanvas *Can = new TCanvas("Can","Can", 10, 10, 1510, 510);
	Can->Divide(3,1);
	Can->cd(1);
	Aligned->Draw("colz");
	cout << "Size aligned histo: " << Aligned->GetSize()  << endl;
	Can->cd(2);
	Misaligned->Draw("colz");
	cout << "Size misaligned histo: " << Misaligned->GetSize()  << endl;

	Int_t nbin_range_X_ali = Aligned->GetXaxis()->FindBin(10.0) - Aligned->GetXaxis()->FindBin(0.);
	Int_t nbin_range_Y_ali = Aligned->GetYaxis()->FindBin(3.0) - Aligned->GetYaxis()->FindBin(0.);
	Int_t nbin_range_X_mis = Misaligned->GetXaxis()->FindBin(10.0) - Misaligned->GetXaxis()->FindBin(0.);
	Int_t nbin_range_Y_mis = Misaligned->GetYaxis()->FindBin(3.0) - Misaligned->GetYaxis()->FindBin(0.);
	cout << "Aligned:" << endl;
	cout << "nbin X = " << nbin_range_X_ali << " and nbin Y = " << nbin_range_Y_ali << endl;
	cout << "Misaligned:" << endl;
	cout << "nbin X = " << nbin_range_X_mis << " and nbin Y = " << nbin_range_Y_mis << endl;

	Can->cd(3);
	TH2D *Diff = new TH2D("Diff", "Diff", 50, 0., 10., 100, 0., 3.);
	Diff->Add(Aligned, Misaligned, -1., 1.);
	Diff->Draw("colz");
}

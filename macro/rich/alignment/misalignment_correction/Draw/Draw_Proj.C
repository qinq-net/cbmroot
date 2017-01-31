#include <iomanip>
#include <vector>
#include "/u/jbendar/CBMSRC/base/draw/CbmDrawHist.h"

void Draw_Proj()
{
	gROOT->LoadMacro("/lustre/nyx/cbm/users/jbendar/CBMINSTALL/share/cbmroot/macro/littrack/loadlibs.C");
	loadlibs();

	TH3D *Histo;
	TString Histo_Name = "fh_ring_track_distance_vs_xy_truematch";

	TString Path_Histo = "/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned/merge.root";
	TFile* Histograms = new TFile(Path_Histo, "READ");
	// cout << Histograms << endl;
	Histo = (TH3D*) gDirectory->Get(Histo_Name);

	TCanvas *Can = new TCanvas("Can","Can", 10, 10, 1010, 510);
	Can->Divide(2,1);
	Can->cd(1);
	Histo->Draw();

	TH2D *Proj;
	Proj = DrawH3Profile(Histo, false, false, 0., 1.);
	Can->cd(2);
	Proj->Draw("colz");
}

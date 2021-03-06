/** CbmAnaDielectronTaskDrawAll.cxx
 * @author Elena Lebedeva <e.lebedeva@gsi.de>
 * @since 2011
 * @version 2.0
 **/

#include "CbmAnaDielectronTaskDrawAll.h"

#include "CbmDrawHist.h"
#include "CbmUtils.h"
#include "CbmHistManager.h"

#include <string>
#include <iostream>
#include <iomanip>

#include "TText.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TMath.h"
#include "TKey.h"
#include "TClass.h"
#include "TF1.h"
#include "TEllipse.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TLatex.h"

using namespace std;
using namespace Cbm;

void CbmAnaDielectronTaskDrawAll::DrawHistosFromFile(
                                                     const string& fileNameInmed,
                                                     const string& fileNameQgp,
                                                     const string& fileNameOmega,
                                                     const string& fileNamePhi,
                                                     const string& fileNameOmegaDalitz,
                                                     const string& outputDir,
                                                     Bool_t useMvd)
{
    fOutputDir = outputDir;
    fUseMvd = useMvd;
    fDrawQgp = (fileNameQgp != "");
    
    //SetDefaultDrawStyle();
    vector<string> fileNames = {fileNameInmed, fileNameQgp, fileNameOmega, fileNamePhi, fileNameOmegaDalitz};
    
    fHM.resize(fNofSignals);
    for (int i = 0; i < fNofSignals; i++){
        fHM[i] = new CbmHistManager();
        if (!fDrawQgp && i == kQgp) continue;
        TFile* file = new TFile(fileNames[i].c_str());
        fHM[i]->ReadFromFile(file);
        Int_t nofEvents = (int) H1(i, "fh_event_number")->GetEntries();
        fHM[i]->ScaleByPattern(".*", 1./nofEvents);
        cout << "nofEvents = " << nofEvents << endl;
    }
    
    // index: AnalysisSteps
    fh_mean_bg_minv.resize(CbmLmvmHist::fNofAnaSteps);
    fh_mean_eta_minv.resize(CbmLmvmHist::fNofAnaSteps);
    fh_mean_pi0_minv.resize(CbmLmvmHist::fNofAnaSteps);
    fh_sum_s_minv.resize(CbmLmvmHist::fNofAnaSteps);
    fh_mean_eta_minv_pt.resize(CbmLmvmHist::fNofAnaSteps);
    fh_mean_pi0_minv_pt.resize(CbmLmvmHist::fNofAnaSteps);
    fh_mean_sbg_vs_minv.resize(CbmLmvmHist::fNofAnaSteps);
    
    FillMeanHist();
    FillSumSignalsHist();
    CalcCutEffRange(0.0, 0.2);
    CalcCutEffRange(0.2, 0.6);
    CalcCutEffRange(0.6, 1.2);
    SBgRangeAll();
    DrawSBgSignals();
    DrawMinvAll();
    DrawMinvPtAll();
    DrawSBgVsMinv();
    SaveHist();
    SaveCanvasToImage();
}


TH1D* CbmAnaDielectronTaskDrawAll::H1(
                                      int signalType,
                                      const string& name)
{
    return (TH1D*) fHM[signalType]->H1(name);
}

TH2D* CbmAnaDielectronTaskDrawAll::H2(
                                      int signalType,
                                      const string& name)
{
    return (TH2D*) fHM[signalType]->H1(name);
}

TH1D* CbmAnaDielectronTaskDrawAll::GetCoctailMinv(
                                                  CbmLmvmAnalysisSteps step)
{
    TH1D* sInmed = (TH1D*) H1(kInmed, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH1D* sQgp = (fDrawQgp)?(TH1D*) H1(kQgp, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone():nullptr;
    TH1D* sOmega = (TH1D*) H1(kOmega, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH1D* sPhi = (TH1D*) H1(kPhi, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH1D* sEta = fh_mean_eta_minv[step];
    TH1D* sPi0 = fh_mean_pi0_minv[step];
    TH1D* sOmegaDalitz = (TH1D*) H1(kOmegaD, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    
    TH1D* coctail = (TH1D*)sInmed->Clone();
    if (fDrawQgp) coctail->Add(sQgp);
    coctail->Add(sOmega);
    coctail->Add(sPhi);
    coctail->Add(sEta);
    coctail->Add(sPi0);
    coctail->Add(sOmegaDalitz);
    
    return coctail;
}

void CbmAnaDielectronTaskDrawAll::DrawMinvAll()
{
    TCanvas *cMc = fHM[0]->CreateCanvas("minv_all_mc", "minv_all_mc", 800, 800);
    DrawMinv(kMc);
    
    TCanvas *cAcc = fHM[0]->CreateCanvas("minv_all_acc", "minv_all_acc", 800, 800);
    DrawMinv(kAcc);
    
    TCanvas *c = fHM[0]->CreateCanvas("minv_all_ptcut", "minv_all_ptcut", 800, 800);
    DrawMinv(kPtCut);
    
    TCanvas *cTT = fHM[0]->CreateCanvas("minv_all_ttcut", "minv_all_ttcut", 800, 800);
    DrawMinv(kTtCut);
    
    TCanvas *celId = fHM[0]->CreateCanvas("minv_all_elid", "minv_all_elid", 800, 800);
    DrawMinv(kElId);
}

void CbmAnaDielectronTaskDrawAll::DrawMinv(
                                           CbmLmvmAnalysisSteps step)
{
    TH1D* sInmed = (TH1D*) H1(kInmed, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH1D* sQgp = (fDrawQgp)?(TH1D*) H1(kQgp, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone():nullptr;
    TH1D* sOmega = (TH1D*) H1(kOmega, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH1D* sPhi = (TH1D*) H1(kPhi, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH1D* bg = (TH1D*)fh_mean_bg_minv[step]->Clone();
    TH1D* sEta = (TH1D*)fh_mean_eta_minv[step]->Clone();
    TH1D* sPi0 = (TH1D*)fh_mean_pi0_minv[step]->Clone();
    TH1D* sOmegaDalitz = (TH1D*) H1(kOmegaD, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    
    TH1D* coctail = GetCoctailMinv(step);
    
    TH1D* sbg = (TH1D*)bg->Clone();
    sbg->Add(sInmed);
    if (fDrawQgp) sbg->Add(sQgp);
    sbg->Add(sOmega);
    sbg->Add(sPhi);
    sbg->Add(sEta);
    sbg->Add(sPi0);
    sbg->Add(sOmegaDalitz);
    
    
    int nRebin = 20;
    sbg->Rebin(nRebin);
    coctail->Rebin(nRebin);
    bg->Rebin(nRebin);
    sPi0->Rebin(nRebin);
    sEta->Rebin(nRebin);
    sOmegaDalitz->Rebin(nRebin);
    sOmega->Rebin(nRebin);
    sInmed->Rebin(nRebin);
    if (fDrawQgp) sQgp->Rebin(nRebin);
    sPhi->Rebin(nRebin);
    
    /*sbg->Scale(1./nRebin);
    coctail->Scale(1./nRebin);
    bg->Scale(1./nRebin);
    sPi0->Scale(1./nRebin);
    sEta->Scale(1./nRebin);
    sOmegaDalitz->Scale(1./nRebin);
    sOmega->Scale(1./nRebin);
    sInmed->Scale(1./nRebin);
    sQgp->Scale(1./nRebin);
    sPhi->Scale(1./nRebin);*/
    
    double binWidth = sbg->GetBinWidth(1);
    sbg->Scale(1./binWidth);
    coctail->Scale(1./binWidth);
    bg->Scale(1./binWidth);
    sPi0->Scale(1./binWidth);
    sEta->Scale(1./binWidth);
    sOmegaDalitz->Scale(1./binWidth);
    sOmega->Scale(1./binWidth);
    sInmed->Scale(1./binWidth);
    if (fDrawQgp) sQgp->Scale(1./binWidth);
    sPhi->Scale(1./binWidth);
    
    
    sbg->SetMinimum(5e-8);
    sbg->SetMaximum(2e-2);
    sbg->GetXaxis()->SetRangeUser(0, 2.);
    bg->GetXaxis()->SetRangeUser(0, 2.);
    coctail->GetXaxis()->SetRangeUser(0, 2.);
    sPi0->GetXaxis()->SetRangeUser(0, 2.);
    sEta->GetXaxis()->SetRangeUser(0, 2.);
    sOmegaDalitz->GetXaxis()->SetRangeUser(0, 2.);
    sOmega->GetXaxis()->SetRangeUser(0, 2.);
    sInmed->GetXaxis()->SetRangeUser(0, 2.);
    if (fDrawQgp) sQgp->GetXaxis()->SetRangeUser(0, 2.);
    sPhi->GetXaxis()->SetRangeUser(0, 2.);
    
/*    
    if (step == kMc) {
        DrawH1({coctail, sPi0, sEta, sOmegaDalitz, sOmega, sInmed, sQgp, sPhi},
                {"", "", "", "", "", "", "", ""}, kLinear, kLog, false, 0.8, 0.8, 0.99, 0.99);
    } else {
        DrawH1({sbg, bg, coctail, sPi0, sEta, sOmegaDalitz, sOmega, sInmed, sQgp, sPhi},
                {"", "", "", "", "", "", "", "", "", ""}, kLinear, kLog, false, 0.8, 0.8, 0.99, 0.99);
    }
  */  
  
    if (step == kMc) {
        if (fDrawQgp) {
            DrawH1({coctail, sPi0, sEta, sOmegaDalitz, sOmega, sInmed, sQgp, sPhi},
                {"", "", "", "", "", "", "", ""}, kLinear, kLog, false, 0.8, 0.8, 0.99, 0.99, "HIST L");
        } else {
            DrawH1({coctail, sPi0, sEta, sOmegaDalitz, sOmega, sInmed, sPhi},
                            {"", "", "", "", "", "", ""}, kLinear, kLog, false, 0.8, 0.8, 0.99, 0.99, "HIST L");
        }
    } else {
        if (fDrawQgp) {
            DrawH1({sbg, bg, coctail, sPi0, sEta, sOmegaDalitz, sOmega, sInmed, sQgp, sPhi},
                {"", "", "", "", "", "", "", "", "", ""}, kLinear, kLog, false, 0.8, 0.8, 0.99, 0.99, "HIST L");
        } else {
            DrawH1({sbg, bg, coctail, sPi0, sEta, sOmegaDalitz, sOmega, sInmed, sPhi},
                            {"", "", "", "", "", "", "", "", ""}, kLinear, kLog, false, 0.8, 0.8, 0.99, 0.99, "HIST L");
        }
    }
    
    string yTitle = "dN/dM_{ee} [GeV/c^{2}]^{-1}";
    coctail->GetYaxis()->SetTitle(yTitle.c_str());
    sbg->GetYaxis()->SetTitle(yTitle.c_str());
    coctail->GetYaxis()->SetLabelSize(0.05);
    sbg->GetYaxis()->SetLabelSize(0.05);
    
    sInmed->SetFillColor(kMagenta-3);
    sInmed->SetLineColor(kMagenta-2);
    sInmed->SetLineStyle(0);
    sInmed->SetLineWidth(3);
    sInmed->SetFillStyle(3344);

    if (fDrawQgp) {
        sQgp->SetFillColor(kOrange-2);
        sQgp->SetLineColor(kOrange-3);
        sQgp->SetLineStyle(0);
        sQgp->SetLineWidth(3);
        sQgp->SetFillStyle(3444);
    }
    

    sOmega->SetFillColor(kOrange+7);
    sOmega->SetLineColor(kOrange+4);
    sOmega->SetLineStyle(0);
    sOmega->SetLineWidth(2);

    sPhi->SetFillColor(kAzure+2);
    sPhi->SetLineColor(kAzure+3);
    sPhi->SetLineStyle(0);
    sPhi->SetLineWidth(2);
    sPhi->SetFillStyle(3112);
    gStyle->SetHatchesLineWidth(1);
    gStyle->SetHatchesSpacing(1.);
    
    
    bg->SetFillColor(kGray);
    bg->SetLineColor(kBlack);
    bg->SetLineStyle(0);
    bg->SetLineWidth(1);
    

    sEta->SetFillColor(kRed-4);
    sEta->SetLineColor(kRed+2);
    sEta->SetLineStyle(0);
    sEta->SetLineWidth(2);
    

    sPi0->SetFillColor(kGreen-3);
    sPi0->SetLineColor(kGreen+3);
    sPi0->SetLineStyle(0);
    sPi0->SetLineWidth(2);

    sOmegaDalitz->SetFillColor(kCyan+2);
    sOmegaDalitz->SetLineColor(kCyan+4);
    sOmegaDalitz->SetLineStyle(0);
    sOmegaDalitz->SetLineWidth(2);
    
    sbg->SetFillColor(kBlack);
    sbg->SetLineColor(kBlack);
    sbg->SetLineStyle(0);
    sbg->SetLineWidth(1);

    coctail->SetLineColor(kRed+2);
    coctail->SetFillStyle(0);
    coctail->SetLineWidth(3);
    
    if (step != kMc) {
        TLegend* legend = new TLegend(0.7, 0.6, 0.99, 0.99);
        legend->SetFillColor(kWhite);
        legend->SetTextSize(0.04);
        legend->AddEntry(sOmega, "#omega #rightarrow e^{+}e^{-}", "f");
        legend->AddEntry(sOmegaDalitz, "#omega #rightarrow #pi^{0}e^{+}e^{-}", "f");
        legend->AddEntry(sPhi, "#phi #rightarrow e^{+}e^{-}", "f");
        legend->AddEntry(sInmed, "in-medium #rho", "f");
        if (fDrawQgp) legend->AddEntry(sQgp, "QGP radiation", "f");
        legend->AddEntry(sEta, "#eta #rightarrow #gammae^{+}e^{-}", "f");
        legend->AddEntry(sPi0, "#pi^{0} #rightarrow #gammae^{+}e^{-}", "f");
        legend->AddEntry(coctail, "Coctail", "f");
        legend->AddEntry(bg, "Background", "f");
        legend->AddEntry(sbg, "Coctail+BG", "f");
        legend->Draw();
    } else {
        TLegend* legend = new TLegend(0.7, 0.7, 0.99, 0.99);
        legend->SetFillColor(kWhite);
        legend->SetTextSize(0.04);
        legend->AddEntry(sOmega, "#omega #rightarrow e^{+}e^{-}", "f");
        legend->AddEntry(sOmegaDalitz, "#omega #rightarrow #pi^{0}e^{+}e^{-}", "f");
        legend->AddEntry(sPhi, "#phi #rightarrow e^{+}e^{-}", "f");
        legend->AddEntry(sInmed, "in-medium #rho", "f");
        if (fDrawQgp)legend->AddEntry(sQgp, "QGP radiation", "f");
        legend->AddEntry(sEta, "#eta #rightarrow #gammae^{+}e^{-}", "f");
        legend->AddEntry(sPi0, "#pi^{0} #rightarrow #gammae^{+}e^{-}", "f");
        legend->AddEntry(coctail, "Coctail", "f");
        legend->Draw();
    }

    
    gPad->SetLogy(true);
}

void CbmAnaDielectronTaskDrawAll::DrawSBgVsMinv()
{
    TH1D* bg = (TH1D*)fh_mean_bg_minv[kTtCut]->Clone();
    TH1D* coctail = GetCoctailMinv(kTtCut);
    fh_mean_sbg_vs_minv[kTtCut] = new TH1D(("fh_sbg_vs_minv_" + CbmLmvmHist::fAnaSteps[kTtCut]).c_str(), ("fh_sbg_vs_minv_"+CbmLmvmHist::fAnaSteps[kTtCut]+";M_{ee} [GeV/c^{2}];Cocktail/Background").c_str(),
                                           bg->GetNbinsX(), bg->GetXaxis()->GetXmin(), bg->GetXaxis()->GetXmax());
    fh_mean_sbg_vs_minv[kTtCut]->Divide(coctail, bg, 1., 1., "B");
    fh_mean_sbg_vs_minv[kTtCut]->Rebin(20);
    fh_mean_sbg_vs_minv[kTtCut]->Scale(1./20.);
    fh_mean_sbg_vs_minv[kTtCut]->GetXaxis()->SetRangeUser(0, 2.);
    
    bg = (TH1D*)fh_mean_bg_minv[kPtCut]->Clone();
    coctail = GetCoctailMinv(kPtCut);
    fh_mean_sbg_vs_minv[kPtCut] = new TH1D(("fh_sbg_vs_minv_" + CbmLmvmHist::fAnaSteps[kPtCut]).c_str(), ("fh_sbg_vs_minv_"+CbmLmvmHist::fAnaSteps[kPtCut]+";M_{ee} [GeV/c^{2}];Cocktail/Background").c_str(),
                                           bg->GetNbinsX(), bg->GetXaxis()->GetXmin(), bg->GetXaxis()->GetXmax());
    fh_mean_sbg_vs_minv[kPtCut]->Divide(coctail, bg, 1., 1., "B");
    fh_mean_sbg_vs_minv[kPtCut]->Rebin(20);
    fh_mean_sbg_vs_minv[kPtCut]->Scale(1./20.);
    fh_mean_sbg_vs_minv[kPtCut]->GetXaxis()->SetRangeUser(0, 2.);
    
    TCanvas* c = fHM[0]->CreateCanvas("lmvm_sbg_vs_minv", "lmvm_sbg_vs_minv", 800, 800);
    DrawH1({fh_mean_sbg_vs_minv[kTtCut], fh_mean_sbg_vs_minv[kPtCut]}, {"Without Pt cut", "With Pt cut"}, kLinear, kLog, true, 0.6, 0.85, 0.99, 0.99);
    gPad->SetLogy(true);
}

void CbmAnaDielectronTaskDrawAll::DrawMinvPtAll()
{
    TCanvas *cptcut = fHM[0]->CreateCanvas("minv_pt_ptcut", "minv_pt_ptcut", 800, 800);
    DrawMinvPt(kPtCut);
}

void CbmAnaDielectronTaskDrawAll::DrawMinvPt(
                                             CbmLmvmAnalysisSteps step)
{
    TH2D* sInmed = (TH2D*) H2(kInmed, "fh_signal_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH2D* sQgp = (fDrawQgp)?(TH2D*) H2(kQgp, "fh_signal_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone():nullptr;
    TH2D* sOmega = (TH2D*) H2(kOmega, "fh_signal_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH2D* sOmegaDalitz = (TH2D*) H2(kOmegaD, "fh_signal_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH2D* sPhi = (TH2D*) H2(kPhi, "fh_signal_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone();
    TH2D* sEta = fh_mean_eta_minv_pt[step];
    TH2D* sPi0 = fh_mean_pi0_minv_pt[step];
    
    TH2D* coctail = (TH2D*)sInmed->Clone();
    if (fDrawQgp) coctail->Add(sQgp);
    coctail->Add(sOmega);
    coctail->Add(sPhi);
    coctail->Add(sOmegaDalitz);
    coctail->Add(sEta);
    coctail->Add(sPi0);
    
    DrawH2(coctail);
}

void CbmAnaDielectronTaskDrawAll::FillMeanHist()
{
    for (int step = 0; step < CbmLmvmHist::fNofAnaSteps; step++){
        for (int iS = 0; iS < fNofSignals; iS++){
            if (!fDrawQgp && iS == kQgp) continue;
            if (iS == 0) {
                fh_mean_bg_minv[step] = (TH1D*)H1(iS, "fh_bg_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
                fh_mean_eta_minv[step] = (TH1D*)H1(iS, "fh_eta_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
                fh_mean_pi0_minv[step] = (TH1D*)H1(iS, "fh_pi0_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
                fh_mean_eta_minv_pt[step] = (TH2D*)H2(iS, "fh_eta_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone();
                fh_mean_pi0_minv_pt[step] = (TH2D*)H2(iS, "fh_pi0_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone();
            } else {
                fh_mean_bg_minv[step]->Add( (TH1D*)H1(iS, "fh_bg_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
                fh_mean_eta_minv[step]->Add( (TH1D*)H1(iS, "fh_eta_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
                fh_mean_pi0_minv[step]->Add( (TH1D*)H1(iS, "fh_pi0_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
                fh_mean_eta_minv_pt[step]->Add( (TH2D*)H2(iS, "fh_eta_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
                fh_mean_pi0_minv_pt[step]->Add( (TH2D*)H2(iS, "fh_pi0_minv_pt_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
            }
        }
        fh_mean_bg_minv[step]->Scale(1./(double) fNofSignals);
        fh_mean_eta_minv[step]->Scale(1./(double) fNofSignals);
        fh_mean_pi0_minv[step]->Scale(1./(double) fNofSignals);
        fh_mean_eta_minv_pt[step]->Scale(1./(double) fNofSignals);
        fh_mean_pi0_minv_pt[step]->Scale(1./(double) fNofSignals);
    }
    
    
}

void CbmAnaDielectronTaskDrawAll::SaveHist()
{
    if (fOutputDir != ""){
        gSystem->mkdir(fOutputDir.c_str(), true);
        TFile* f = TFile::Open( string(fOutputDir + "/draw_all_hist.root").c_str(), "RECREATE" );
        for (int i = 0; i < CbmLmvmHist::fNofAnaSteps; i++){
            fh_mean_bg_minv[i]->Write();
            fh_mean_eta_minv[i]->Write();
            fh_mean_pi0_minv[i]->Write();
        }
        fh_mean_sbg_vs_minv[kTtCut]->Write();
        fh_mean_sbg_vs_minv[kPtCut]->Write();
        f->Close();
    }
}

void CbmAnaDielectronTaskDrawAll::FillSumSignalsHist()
{
    for (int step = 0; step < CbmLmvmHist::fNofAnaSteps; step++){
        fh_sum_s_minv[step] = (TH1D*)H1(kInmed, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
        if (fDrawQgp) fh_sum_s_minv[step]->Add( (TH1D*)H1(kQgp, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
        fh_sum_s_minv[step]->Add( (TH1D*)H1(kOmega, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
        fh_sum_s_minv[step]->Add( (TH1D*)H1(kPhi, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
        fh_sum_s_minv[step]->Add( (TH1D*)H1(kOmegaD, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone() );
        fh_sum_s_minv[step]->Add( (TH1D*)fh_mean_eta_minv[step]->Clone() );
        fh_sum_s_minv[step]->Add( (TH1D*)fh_mean_pi0_minv[step]->Clone() );
    }
}

void CbmAnaDielectronTaskDrawAll::CalcCutEffRange(
                                                  Double_t minMinv,
                                                  Double_t maxMinv)
{
    stringstream ss1;
    ss1 << minMinv << "_" << maxMinv;
    TH1D* grS = new TH1D(("grS_" + ss1.str()).c_str(), ";Analysis step;Efficiency [%]", CbmLmvmHist::fNofAnaSteps, 0, CbmLmvmHist::fNofAnaSteps);
    TH1D* grB = new TH1D(("grB_" + ss1.str()).c_str(), ";Analysis step;Efficiency [%]", CbmLmvmHist::fNofAnaSteps, 0, CbmLmvmHist::fNofAnaSteps);
    int x = 1;
    for (int step = kElId; step < CbmLmvmHist::fNofAnaSteps; step++){
        if ( !fUseMvd && (step == kMvd1Cut || step == kMvd2Cut)){
            continue;
        }
        Int_t x1 = fh_sum_s_minv[step]->FindBin(minMinv);
        Int_t x2 = fh_sum_s_minv[step]->FindBin(maxMinv);
        
        double yS = 100.* fh_sum_s_minv[step]->Integral(x1, x2) / fh_sum_s_minv[kElId]->Integral(x1, x2);
        double yB = 100.* fh_mean_bg_minv[step]->Integral(x1, x2) / fh_mean_bg_minv[kElId]->Integral(x1, x2);
        
        grB->GetXaxis()->SetBinLabel(x, CbmLmvmHist::fAnaStepsLatex[step].c_str());
        grB->SetBinContent(x, yB);
        grS->SetBinContent(x, yS);
        x++;
    }
    
    grB->GetXaxis()->SetLabelSize(0.06);
    grB->GetXaxis()->SetRange(1, x - 1);
    grS->GetXaxis()->SetRange(1, x - 1);
    
    stringstream ss;
    ss << "lmvm_cut_eff_" << minMinv << "_" << maxMinv;
    TCanvas* c = fHM[0]->CreateCanvas(ss.str().c_str(), ss.str().c_str(), 700, 700);
    DrawH1({grB, grS}, {"BG", "Signal"}, kLinear, kLinear, true, 0.75, 0.85, 1.0, 1.0);
    grS->SetLineWidth(4);
    grB->SetLineWidth(4);
    grB->SetMinimum(1);
    grB->SetMaximum(105);
    
    stringstream ss2;
    ss2 << minMinv <<"<M [GeV/c^2]<" << maxMinv;
    TText *t = new TText(0.5, 110, ss2.str().c_str());
    t->Draw();
}


TH1D* CbmAnaDielectronTaskDrawAll::SBgRange(
                                            Double_t min,
                                            Double_t max)
{
    stringstream ss;
    ss << "lmvm_s_bg_region_" << min << "_" << max;
    TH1D* h_s_bg = new TH1D(ss.str().c_str(), string(ss.str()+";Analysis steps;S/BG").c_str(), CbmLmvmHist::fNofAnaSteps, 0, CbmLmvmHist::fNofAnaSteps);
    h_s_bg->GetXaxis()->SetLabelSize(0.06);
    int x = 1;
    for (int step = kElId; step < CbmLmvmHist::fNofAnaSteps; step++){
        if ( !fUseMvd && (step == kMvd1Cut || step == kMvd2Cut)){
            continue;
        }
        Int_t bin1 = fh_sum_s_minv[step]->FindBin(min);
        Int_t bin2 = fh_sum_s_minv[step]->FindBin(max);
        double y = fh_sum_s_minv[step]->Integral(bin1, bin2) / fh_mean_bg_minv[step]->Integral(bin1, bin2);
        
        h_s_bg->GetXaxis()->SetBinLabel(x, CbmLmvmHist::fAnaStepsLatex[step].c_str());
        h_s_bg->SetBinContent(x, y);
        // replace "." with "_"
        string str = ss.str();
        for (string::iterator it = str.begin() ; it < str.end(); it++){
            if (*it == '.') *it = '_';
        }
        x++;
    }
    h_s_bg->GetXaxis()->SetRange(1, x - 1);
    return h_s_bg;
}

void CbmAnaDielectronTaskDrawAll::SBgRangeAll()
{
    TH1D* h_00_02 = SBgRange(0.0, 0.2);
    TH1D* h_02_06 = SBgRange(0.2, 0.6);
    TH1D* h_06_12 = SBgRange(0.6, 1.2);
    
    TCanvas* c = fHM[0]->CreateCanvas("lmvm_s_bg_ranges", "lmvm_s_bg_ranges", 700, 700);
    DrawH1({h_00_02, h_02_06, h_06_12},
            {"0.0<M [GeV/c^{2}]<0.2", "0.2<M [GeV/c^{2}]<0.6", "0.6<M [GeV/c^{2}]<1.2"},
           kLinear, kLog, true, 0.25, 0.8, 0.75, 0.99);
    
    h_00_02->SetMinimum(1e-3);
    h_00_02->SetMaximum(3);
    h_00_02->SetLineWidth(4);
    h_02_06->SetLineWidth(4);
    h_06_12->SetLineWidth(4);
    
    TH1D* h_05_06 = SBgRange(0.5, 0.6);
    TCanvas* c1 = fHM[0]->CreateCanvas("lmvm_s_bg_ranges_05_06", "lmvm_s_bg_ranges_05_06", 700, 700);
    DrawH1(h_05_06, kLinear, kLinear);
    h_05_06->SetMinimum(1e-3);
    h_05_06->SetMaximum(2e-2);
    h_05_06->SetLineWidth(4);
}

void CbmAnaDielectronTaskDrawAll::DrawSBgSignals()
{
    //   Double_t y[CbmLmvmHist::fNofAnaSteps];
    TCanvas* cFit = fHM[0]->CreateCanvas("lmvm_signal_fit", "lmvm_signal_fit", 600, 600);
    TCanvas* cDashboard = fHM[0]->CreateCanvas("lmvm_dashboard", "lmvm_dashboard", 1000, 900);
    int iDash = 2;
    TLatex* latex = new TLatex();
    latex->SetTextSize(0.03);
    latex->DrawLatex(0.05, 0.95, "signal");
    latex->DrawLatex(0.2, 0.95, "step");
    latex->DrawLatex(0.4, 0.95, "eff, %");
    latex->DrawLatex(0.55, 0.95, "S/BG");
    latex->DrawLatex(0.7, 0.95, "mean");
    latex->DrawLatex(0.85, 0.95, "sigma");
    TString str;
    for (int iF = 0; iF < fNofSignals - 1; iF++){
        if (!fDrawQgp && iF == kQgp) continue;
        string signalName = CbmLmvmHist::fSignalNames[iF];
        cout << "Signal: " << signalName << endl;
        stringstream ss;
        ss << "lmvm_s_bg_cuts_" << signalName;
        
        TH1D* h_s_bg = new TH1D(ss.str().c_str(), string(ss.str()+";Analysis steps;S/BG").c_str(), CbmLmvmHist::fNofAnaSteps, 0, CbmLmvmHist::fNofAnaSteps);
        h_s_bg->GetXaxis()->SetLabelSize(0.06);
        h_s_bg->SetLineWidth(4);
        int x = 1;
        iDash++; // empty string after each signal
        for (int step = 0; step < CbmLmvmHist::fNofAnaSteps; step++){
            if (step < kElId) continue;
            if ( !fUseMvd && (step == kMvd1Cut || step == kMvd2Cut)){
                continue;
            }

            TH1D* s = (TH1D*)H1(iF, "fh_signal_minv_" + CbmLmvmHist::fAnaSteps[step])->Clone();
            TH1D* bg = (TH1D*)fh_mean_bg_minv[step]->Clone();
            cFit->cd();
            if (iF == kPhi){
                if (s->GetEntries() > 0) s->Fit("gaus", "Q", "", 0.95, 1.05);
            } else if (iF == kOmega){
                if (s->GetEntries() > 0) s->Fit("gaus", "Q", "", 0.69, 0.81);
            } else{
                if (s->GetEntries() > 0) s->Fit("gaus", "Q");
            }

            TF1* func = s->GetFunction("gaus");
            Double_t mean = (func != NULL)?func->GetParameter("Mean"):0.;
            Double_t sigma = (func != NULL)?func->GetParameter("Sigma"):0.;
            Int_t minInd = s->FindBin(mean - 2.*sigma);
            Int_t maxInd = s->FindBin(mean + 2.*sigma);
            
            Double_t sumSignal = 0.;
            Double_t sumBg = 0.;
            for (Int_t i = minInd + 1; i <= maxInd - 1; i++){
                sumSignal += s->GetBinContent(i);
                sumBg += bg->GetBinContent(i);
            }
            Double_t sbg = sumSignal/sumBg;
            double eff = 100. * H1(iF, "fh_signal_pty_" + CbmLmvmHist::fAnaSteps[step])->GetEntries() / H1(iF, "fh_signal_pty_" + CbmLmvmHist::fAnaSteps[kMc])->GetEntries();

            bool isOmegaOrPhi = (iF == kPhi || iF == kOmega);
            cDashboard->cd();
            latex->DrawLatex(0.05, 1.0 - iDash*0.033, signalName.c_str());
            latex->DrawLatex(0.2, 1.0 - iDash*0.033, CbmLmvmHist::fAnaSteps[step].c_str());
            str.Form("%.2f", eff);
            latex->DrawLatex(0.4, 1.0 - iDash*0.033, str.Data());
            str.Form("%.3f", sumSignal/sumBg);
            latex->DrawLatex(0.55, 1.0 - iDash*0.033, (isOmegaOrPhi)?str.Data():"-");
            str.Form("%.1f", 1000.*mean);
            latex->DrawLatex(0.7, 1.0 - iDash*0.033, (isOmegaOrPhi)?str.Data():"-");
            str.Form("%.1f", 1000.*sigma);
            latex->DrawLatex(0.85, 1.0 - iDash*0.033, (isOmegaOrPhi)?str.Data():"-");

            h_s_bg->GetXaxis()->SetBinLabel(x, CbmLmvmHist::fAnaStepsLatex[step].c_str());
            if (sbg < 1000.) h_s_bg->SetBinContent(x, sbg);
            x++;
            iDash++;
        }
        h_s_bg->GetXaxis()->SetRange(1, x - 1);
        TCanvas* c = fHM[0]->CreateCanvas(ss.str().c_str(), ss.str().c_str(), 800, 800);
        DrawH1(h_s_bg);
        h_s_bg->SetLineWidth(4);

        cDashboard->Draw();
    }
}

void CbmAnaDielectronTaskDrawAll::SaveCanvasToImage()
{
    cout <<"Images output dir:" << fOutputDir << endl;
    fHM[0]->SaveCanvasToImage(fOutputDir, "eps;png");
}

ClassImp(CbmAnaDielectronTaskDrawAll);


#include "CbmReportElement.h"
#include "CbmHistManager.h"
#include "CbmDrawHist.h"
#include "CbmUtils.h"
#include "TH1.h"
#include "TF1.h"
#include "TPad.h"
#include "TDirectory.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TMarker.h"
#include <boost/assign/list_of.hpp>
#include <map>
#include <iostream>
#include "CbmAnaJpsiReport.h"
#include "CbmAnaJpsiHist.h"
#include "CbmAnaJpsiCuts.h"
#include "TLine.h"

using boost::assign::list_of;
using Cbm::NumberToString;
using Cbm::Split;
using std::map;
using std::endl;
using std::cout;

CbmAnaJpsiReport::CbmAnaJpsiReport():
		CbmSimulationReport()
{
	SetReportName("jpsi_qa");
}

CbmAnaJpsiReport::~CbmAnaJpsiReport()
{
}

void CbmAnaJpsiReport::Create()
{
   //Out().precision(3);
   Out() << R()->DocumentBegin();
   Out() << R()->Title(0, GetTitle());

   PrintCanvases();

   Out() << R()->DocumentEnd();
}

void CbmAnaJpsiReport::Draw()
{

	  SetDefaultDrawStyle();

	  {
	   TCanvas* c = CreateCanvas("jpsi_NofMcTracksInEvent", "jpsi_NofMcTracksInEvent",  600, 600);
	   //DrawH1(fhNofMcTracksInEvent);
	   DrawH1(H1("fhNofMcTracksInEvent"));
	  }

	  {
	   TCanvas* c = CreateCanvas("jpsi_NofStsPointInEvent", "jpsi_NofStsPointInEvent",  600, 600);
	   //DrawH1(fhNofStsPointInEvent);
	   DrawH1(H1("fhNofStsPointInEvent"));
	  }

	  {
	   TCanvas* c = CreateCanvas("jpsi_NofRichPointInEvent", "jpsi_NofRichPointInEvent",  600, 600);
	   //DrawH1(fhNofRichPointInEvent);
	   DrawH1(H1("fhNofRichPointInEvent"));
	  }

	  {
	   TCanvas* c = CreateCanvas("jpsi_NofTrdPointInEvent", "jpsi_NofTrdPointInEvent",  600, 600);
	   //DrawH1(fhNofTrdPointInEvent);
	   DrawH1(H1("fhNofTrdPointInEvent"));
	  }

	  {
	   TCanvas* c = CreateCanvas("jpsi_NofTofPointInEvent", "jpsi_NofTofPointInEvent",  600, 600);
	   //DrawH1(fhNofTofPointInEvent);
	   DrawH1(H1("fhNofTofPointInEvent"));
	  }

	  {
	   TCanvas* c = CreateCanvas("jpsi_MonteCarloMomentumOfAll", "jpsi_MonteCarloMomentumOfAll",  600, 600);
	   //DrawH1(fhMcMomAll);
	   DrawH1(H1("fhMcMomAll"));
	  }

	  {
	   TCanvas* c = CreateCanvas("jpsi_PgdCode", "jpsi_PgdCode",  600, 600);
	   //DrawH1(fhPdgCodeAll);
	   DrawH1(H1("fhPdgCodeAll"));
	  }

	  {
	   TCanvas* c = CreateCanvas("jpsi_McMomElectrons", "jpsi_McMomElectrons", 1000, 1000);
	   c->Divide(2,2);
	   c->cd(1);
	   DrawH1(H1("fhMcMomElectrons"));
	   c->cd(2);
	   DrawH2(H2("fhMcPtYElectrons"));
	   c->cd(3);
	   TH1D* ProjX = H2("fhMcPtYElectrons")->ProjectionX();
	   TH1D* ProjY = H2("fhMcPtYElectrons")->ProjectionY();
	   DrawH1(ProjX);
	   c->cd(4);
	   DrawH1(ProjY);
	  }

	  {
	    TCanvas* c = CreateCanvas("jpsi_NofStsHitInEvent","jpsi_NofStsHitInEvent", 600, 600);
	    DrawH1(H1("fhNofStsHitsInEvent"));
	  }

	  {
	    TCanvas* c = CreateCanvas("jpsi_NofHitsInObjectInEvent","jpsi_NofHItsInObjectInEvent", 1000, 1000);
	    c->Divide(2,2);
	    c->cd(1);
	    DrawH1(H1("fhNofStsHitsInEvent"));
	    c->cd(2);
	    DrawH1(H1("fhNofRichHitsInEvent"));
	    c->cd(3);
	    DrawH1(H1("fhNofTrdHitsInEvent"));
	    c->cd(4);
	    DrawH1(H1("fhNofTofHitsInEvent"));
	  }

	  {
	    TCanvas* c = CreateCanvas("jpsi_NofTracksOrRingsInObjectInEvent","jpsi_NofTracksOrRingsInObjectInEvent",1200,600);
	    c->Divide(3,1);
	    c->cd(1);
	    DrawH1(H1("fhNofStsTracksInEvent"));
	    c->cd(2);
	    DrawH1(H1("fhNofRichRingsInEvent"));
	    c->cd(3);
	    DrawH1(H1("fhNofTrdTracksInEvent"));
	  }

	  {
	    TCanvas* c = CreateCanvas("jpsi_StsHitsXY","jpsi_StsHitsXY",600,600);
	    DrawH2(H2("fhStsHitXY"));
	  }

	  {
	    TCanvas* c = CreateCanvas("jpsi_RichHitsXY","jpsi_RichHitsXY",600,600);
	    DrawH2(H2("fhRichHitXY"));
	  }

	  {
	    TCanvas* c = CreateCanvas("jpsi_TrdHitsXY","jpsi_TrdHitsXY",600,600);
	    DrawH2(H2("fhTrdHitXY"));
	  }

	  {
	    TCanvas* c = CreateCanvas("jpsi_TofHitsXY","jpsi_TofHitsXY",600,600);
	    DrawH2(H2("fhTofHitXY"));
	  }

	  {
	    TCanvas* c = CreateCanvas("jpsi_AllDetektorHitsInZPosition","jpsi_AllDetektorHitsInZPosition",600,600);
	    DrawH1(H1("fhAllDetektorHitsInZPosition"));
	  }

	  {
	      TCanvas* c = CreateCanvas("jpsi_SignalElectronMom","jpsi_SignalElectronMom",600,600);
	      DrawH1(H1("fhSignalElectronMom"));
	    }

	  { TCanvas* c = CreateCanvas("jpsi_SignalElectronPt","jpsi_SignalElectronPt",600,600);
	  DrawH1(H1("fhSignalElectronPt"));
	  	}

	  { TCanvas* c = CreateCanvas("jpsi_SignalMotherPdgCode","jpsi_SignalMotherPdgCode",600,600);
	  DrawH1(H1("fhSignalMotherPdgCode"));
	  	}

	  {
	      TCanvas* c = CreateCanvas("jpsi_GammaConvElectronMom","jpsi_GammaConvElectronMom",600,600);
	      DrawH1(H1("fhGammaConvElectronMom"));
	    }

	  {
	      TCanvas* c = CreateCanvas("jpsi_GammaConvElectronPt","jpsi_GammaConvElectronPt",600,600);
	      c->SetLogy();
	      DrawH1(H1("fhGammaConvElectronPt"));
	    }

	  {
	      TCanvas* c = CreateCanvas("jpsi_GammaConvElectronStartVertXY","jpsi_GammaConvElectronStartVertXY",600,600);
	      DrawH2(H2("fhGammaConvElectronStartVertXY"));
	  }

	  {
	      TCanvas* c = CreateCanvas("jpsi_GammaConvElectronStartVertZY","jpsi_GammaConvElectronStartVertZY",600,600);
	      DrawH2(H2("fhGammaConvElectronStartVertZY"));
	    }

	  {
	      TCanvas* c = CreateCanvas("jpsi_GammaConvElectronStartVertZX","jpsi_GammaConvElectronStartVertZX",600,600);
	      DrawH2(H2("fhGammaConvElectronStartVertZX"));
	    }

	  {
	      TCanvas* c = CreateCanvas("jpsi_GammaConvElectronStartVertZSqrt(X2+Y2)","jpsi_GammaConvElectronStartVertZSqrt(X2+Y2)",600,600);
	      DrawH2(H2("fhGammaConvElectronStartVertZSqrt(X2+Y2)"));
	    }


	  {
		  TCanvas* c = CreateCanvas("jpsi_RichPointSignalElectronPMTPlane","jpsi_RichPointSignalElectronPMTPlane",600,600);
		  DrawH2(H2("fhRichPointSignalElectronPMTPlane"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_pi0DalitzDecayMomentum","jpsi_pi0DalitzDecayMomentum",600,600);
		  DrawH1(H1("fhpi0DalitzDecayMomentum"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_RichPointGammaAndPi0PMTPlane","jpsi_RichPointGammaAndPi0PMTPlane",600,600);
		  DrawH2(H2("fhRichPointGammaAndPi0PMTPlane"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_RichPointGammaConversionPhotonInPETPlaneXY","jpsi_RichPointGammaConversionPhotonInPETPlaneXY",600,600);
		  DrawH2(H2("fhRichPointGammaConversionPhotonInPETPlaneXY"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_RichPointDalitzDecayInPETPlaneXY","jpsi_RichPointDalitzDecayInPETPlaneXY",600,600);
		  DrawH2(H2("fhRichPointDalitzDecayInPETPlaneXY"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_RichHitSignalElectronPMTPlane","jpsi_RichHitSignalElectronPMTPlane",600,600);
		  DrawH2(H2("fhRichHitSignalElectronPMTPlane"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_RichHitGammaConversionPhotonInPETPlaneXY","jpsi_RichHitGammaConversionPhotonInPETPlaneXY",600,600);
		  DrawH2(H2("fhRichHitGammaConversionPhotonInPETPlaneXY"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_RichHitDalitzDecayInPETPlaneXY","jpsi_RichHitDalitzDecayInPETPlaneXY",600,600);
		  DrawH2(H2("fhRichHitDalitzDecayInPETPlaneXY"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_McEpmRapidityPt","jpsi_McEpmRapidityPt",600,600);
		  DrawH2(H2("fhMcEpmRapidityPt"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_McEpmMometumMag","jpsi_McEpmMometumMag",600,600);
	  	  H1("fhMcEpmMomentumMag")->Scale(1. / H1("fhMcEpmMomentumMag")->Integral());//Scale Yield
	  	  DrawH1(H1("fhMcEpmMomentumMag"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_McEpmMinv","jpsi_McEpmMinv",600,600);
	  	  H1("fhMcEpmMinv")->Scale(1. / H1("fhMcEpmMinv")->Integral());//Scale Yield
	  	  DrawH1(H1("fhMcEpmMinv"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_AccEpmRapidityPt","jpsi_AccEpmRapidityPt",600,600);
		  DrawH2(H2("fhAccEpmRapidityPt"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_AccEpmMometumMag","jpsi_AccEpmMometumMag",600,600);
	  	  H1("fhAccEpmMomentumMag")->Scale(1. / H1("fhAccEpmMomentumMag")->Integral());//Scale Yield
	  	  DrawH1(H1("fhAccEpmMomentumMag"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_AccEpmMinv","jpsi_AccEpmMinv",600,600);
	  	  H1("fhAccEpmMinv")->Scale(1. / H1("fhAccEpmMinv")->Integral());//Scale Yield
	  	  DrawH1(H1("fhAccEpmMinv"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_CandMcEpmPtY","jpsi_CandMcEpmPtY",600,600);
	  	  DrawH2(H2("fhCandMcEpmPtY"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_CandEpmMinv","jpsi_CandEpmMinv",600,600);
	  	  H1("fhCandEpmMinv")->Scale(1. / H1("fhCandEpmMinv")->Integral());//Scale Yield
	  	  DrawH1(H1("fhCandEpmMinv"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_CandMcEpmPtYChi2PrimCut","jpsi_CandMcEpmPtYChi2PrimCut",600,600);
	  	  DrawH2(H2("fhCandMcEpmPtYChi2PrimCut"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_CandEpmMinvChi2PrimCut","jpsi_CandEpmMinvChi2PrimCut",600,600);
	  	  H1("fhCandEpmMinvChi2PrimCut")->Scale(1. / H1("fhCandEpmMinvChi2PrimCut")->Integral());//Scale Yield
	  	  DrawH1(H1("fhCandEpmMinvChi2PrimCut"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_TofMomM2Signal","jpsi_TofMomM2Signal",1200,1200);
	  	  c->Divide(2,2);
	  	  c->cd(1);
	  	  DrawH2(H2("fhTofMomM2Signal"));
	  	  c->cd(2);
	  	  DrawH2(H2("fhTofMomM2Background"));
	  	  c->cd(3);
	  	  DrawH2(H2("fhTofMomM2Gamma"));
	  	  c->cd(4);
	  	  DrawH2(H2("fhTofMomM2Pi0"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_RecoCandEpmPtYChi2PrimCut","jpsi_RecoCandEpmPtYChi2PrimCut",600,600);
	  	  DrawH2(H2("fhRecoCandEpmPtYChi2PrimCut"));
	  }

	  {
	  	  TCanvas* c = CreateCanvas("jpsi_RecoCandEpmMinvChi2PrimCut","jpsi_RecoCandEpmMinvChi2PrimCut",600,600);
	  	  H1("fhRecoCandEpmMinvChi2PrimCut")->Scale(1. / H1("fhRecoCandEpmMinvChi2PrimCut")->Integral());//Scale Yield
	      DrawH1(H1("fhRecoCandEpmMinvChi2PrimCut"));
	  }

	  DrawCutDistributions();
}


void CbmAnaJpsiReport::DrawSourceTypesH1(
      const string& hName,
      bool doScale)
{
   vector<TH1*> h;
   vector<string> hLegend;
   for (int i = 0; i < CbmAnaJpsiHist::fNofSourceTypes; i++){
      string fullName = hName+"_"+CbmAnaJpsiHist::fSourceTypes[i];
      h.push_back( H1(fullName) );
      h[i]->SetLineWidth(2);
      h[i]->SetLineColor(CbmAnaJpsiHist::fSourceTypesColor[i]);
      if (doScale) h[i]->Scale(1. / h[i]->Integral());
      hLegend.push_back( CbmAnaJpsiHist::fSourceTypesLatex[i] );
   }
   DrawH1(h, hLegend, kLinear, kLog, true, 0.90, 0.7, 0.99, 0.99);
}

void CbmAnaJpsiReport::DrawCutH1(
      const string& hName,
      double cutValue)
{
   Int_t w = 600;
   Int_t h = 600;
   TCanvas *c = CreateCanvas( ("jpsi_" + hName).c_str(), ("jpsi_" + hName).c_str(), w, h);
   DrawSourceTypesH1(hName);
   if (cutValue != -999999.){
      TLine* cutLine = new TLine(cutValue, 0.0, cutValue, 1.);
      cutLine->SetLineWidth(2);
      cutLine->Draw();
   }
}

void CbmAnaJpsiReport::DrawCutDistributions()
{
   DrawCutH1("fhChi2PrimEl", 2.0);
   DrawCutH1("fhMomEl", 5.0);
   DrawCutH1("fhChi2StsEl", 2.0);
   DrawCutH1("fhRapidityEl", 2.0);
   DrawCutH1("fhPtEl", 2.0);
   DrawCutH1("fhRichAnn", 2.0);
   DrawCutH1("fhTrdAnn", 2.0);
}


ClassImp(CbmAnaJpsiReport)


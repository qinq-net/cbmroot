
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
		  TCanvas* c = CreateCanvas("jpsi_ReconMomSignalElectron","jpsi_ReconMomSignalElectron",600,600);
		  DrawH1(H1("fhReconMomSignalElectron"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_ReconMomGammaConv","jpsi_ReconMomGammaConv",600,600);
		  DrawH1(H1("fhReconMomGammaConv"));
	  }

	  {
		  TCanvas* c = CreateCanvas("jpsi_ReconMomDalitzDecay","jpsi_ReconMomDalitzDecay",600,600);
		  DrawH1(H1("fhReconMomDalitzDecay"));
	  }
}


ClassImp(CbmAnaJpsiReport)



#include "CbmRichTrbRecoQaStudyReport.h"
#include "CbmReportElement.h"
#include "CbmHistManager.h"
#include "CbmDrawHist.h"
#include "CbmUtils.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include <vector>
#include <iostream>
#include <boost/assign/list_of.hpp>
using boost::assign::list_of;
using namespace std;


CbmRichTrbRecoQaStudyReport::CbmRichTrbRecoQaStudyReport():
		CbmStudyReport()
{
   SetReportName("rich_trb_reco_study_report");
   SetReportTitle("RICH study report");
}

CbmRichTrbRecoQaStudyReport::~CbmRichTrbRecoQaStudyReport()
{
}

void CbmRichTrbRecoQaStudyReport::Create()
{
   Out().precision(3);
   Out() << R()->DocumentBegin();
   Out() << R()->Title(0, GetTitle());
   PrintCanvases();
   Out() <<  R()->DocumentEnd();
}

void CbmRichTrbRecoQaStudyReport::Draw()
{
   SetDefaultDrawStyle();
   FitGausAndDrawH1("fhRadiusCircle", "rich_report_circle_radius");
   FitGausAndDrawH1("fhNofHitsInEvent", "rich_report_hits_in_event");
   FitGausAndDrawH1("fhNofHitsInRing", "rich_report_hits_in_ring");
}

void CbmRichTrbRecoQaStudyReport::FitGausAndDrawH1(
		const string& histName,
		const string& canvasName)
{
   Int_t nofStudies = HM().size();
   TCanvas* canvas2 = CreateCanvas(canvasName.c_str(), canvasName.c_str(), 600, 600);
   vector<TH1*> histos1(nofStudies);
   vector<string> legendNames;
   for (UInt_t iStudy = 0; iStudy < nofStudies; iStudy++) {
     histos1[iStudy] = HM()[iStudy]->H1(histName);
    // histos1[iStudy]->SetStats(true);
     histos1[iStudy]->Fit("gaus", "Q");
     histos1[iStudy]->SetMaximum(histos1[iStudy]->GetMaximum() * 1.25);
     TF1* fit = histos1[iStudy]->GetFunction("gaus");
     Double_t sigma = (NULL != fit) ? fit->GetParameter(2) : 0.;
     Double_t mean = (NULL != fit) ? fit->GetParameter(1) : 0.;
     TString str;
     str.Form(" (%.2f/%.2f)", mean, sigma);
     legendNames.push_back(GetStudyNames()[iStudy] + string(str.Data()));
   }
   DrawH1(histos1, legendNames, kLinear, kLinear);
   for (UInt_t iStudy = 0; iStudy < nofStudies; iStudy++) {
      histos1[iStudy]->GetFunction("gaus")->SetLineColor(histos1[iStudy]->GetLineColor());
   }
   gPad->SetGridx(true);
   gPad->SetGridy(true);
}

ClassImp(CbmRichTrbRecoQaStudyReport)

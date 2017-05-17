#include "CbmRichSmallPrototypeStudyReport.h"
#include "CbmReportElement.h"
#include "CbmDrawHist.h"
#include "CbmUtils.h"
#include "CbmReportElement.h"
#include "CbmHistManager.h"
#include "TSystem.h"
#include "TF1.h"

#include <sstream>
#include <boost/assign/list_of.hpp>

using boost::assign::list_of;
using std::stringstream;
using std::string;
using std::vector;

CbmRichSmallPrototypeStudyReport::CbmRichSmallPrototypeStudyReport():
CbmStudyReport()
{
    SetReportName("rich_smallprototype_study_report");
    SetReportTitle("rich_smallprototype_study_report");
}

CbmRichSmallPrototypeStudyReport::~CbmRichSmallPrototypeStudyReport()
{
}

void CbmRichSmallPrototypeStudyReport::Create()
{
    Out().precision(3);
    Out() << R()->DocumentBegin();
    Out() << R()->Title(0, GetTitle());
    PrintCanvases();
    Out() <<  R()->DocumentEnd();
}

void CbmRichSmallPrototypeStudyReport::Draw()
{
    SetDefaultDrawStyle();
    DrawNofHistAll();
}

void CbmRichSmallPrototypeStudyReport::DrawNofHistAll()
{

	DrawNofHist("fh_nof_rich_points");
	DrawNofHist("fh_nof_rich_hits");
	DrawNofHist("fh_hits_per_ring");
	DrawNofHist("fh_rich_ring_radius");
}

void CbmRichSmallPrototypeStudyReport::DrawNofHist(
		const std::string& histName)
{
    Int_t nofStudies = HM().size();
    {
		TCanvas* c = CreateCanvas(("rich_smallprototype_study_report_" + histName).c_str(), ("rich_smallprototype_study_report_" + histName).c_str(), 800, 800);
		vector<TH1*> histos1(nofStudies);
		vector<string> legendNames;
		for (Int_t iStudy = 0; iStudy < nofStudies; iStudy++) {
			histos1[iStudy] = HM()[iStudy]->H1(histName);
			TF1* func = histos1[iStudy]->GetFunction("gaus");
			string resText = "";
			if (func != NULL) {
				func->SetLineColor(CbmDrawingOptions::Color(iStudy) - 1);
				double m = func->GetParameter(1);
				double s = func->GetParameter(2);
				resText = " (" + Cbm::NumberToString<Double_t>(m, 2) + " / " + Cbm::NumberToString<Double_t>(s, 2) + ")";
			}
			// histos1[iStudy]->GetXaxis()->SetRangeUser(0, 2.);
			legendNames.push_back((GetStudyNames()[iStudy] + resText));
		}
		DrawH1(histos1, legendNames, kLinear, kLinear,  true, 0.6, 0.75, 0.99, 0.99);
    }
}


ClassImp(CbmRichSmallPrototypeStudyReport)


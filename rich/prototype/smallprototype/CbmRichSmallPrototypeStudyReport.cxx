#include "CbmRichSmallPrototypeStudyReport.h"
#include "CbmReportElement.h"
#include "CbmDrawHist.h"
#include "CbmUtils.h"
#include "CbmReportElement.h"
#include "CbmHistManager.h"
#include "TSystem.h"

#include <sstream>
#include <boost/assign/list_of.hpp>
using boost::assign::list_of;
using std::stringstream;


CbmRichSmallPrototypeStudyReport::CbmRichSmallPrototypeStudyReport():
CbmStudyReport()
{
    SetReportName("rich_small_prototype_study_report");
    SetReportTitle("RICH small protitype study report");
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
    DrawNofHitsPerEvent();
}

void CbmRichSmallPrototypeStudyReport::DrawNofHitsPerEvent()
{
    Int_t nofStudies = HM().size();
    TCanvas* c = CreateCanvas("rich_small_prot_study_report_nof_hits_in_event", "rich_small_prot_study_report_nof_hits_in_event", 800, 800);
    vector<TH1*> histos1(nofStudies);
    vector<string> legendNames;
    for (Int_t iStudy = 0; iStudy < nofStudies; iStudy++) {
        histos1[iStudy] = HM()[iStudy]->H1("nof_hits_histogram_name");
        // histos1[iStudy]->GetXaxis()->SetRangeUser(0, 2.);
        legendNames.push_back(GetStudyNames()[iStudy]);
    }
    DrawH1(histos1, legendNames, kLinear, kLinear,  true, 0.6, 0.75, 0.99, 0.99);
}


ClassImp(CbmRichSmallPrototypeStudyReport)


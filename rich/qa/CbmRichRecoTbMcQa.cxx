
#include "CbmRichRecoTbMcQa.h"

#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TF1.h"
#include "TStyle.h"
#include "TEllipse.h"
#include "TMarker.h"
#include "TMCProcess.h"

#include "CbmMCTrack.h"
#include "CbmGlobalTrack.h"
#include "FairTrackParam.h"
#include "CbmRichHit.h"
#include "FairMCPoint.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmRichRing.h"
#include "CbmRichHit.h"
#include "CbmMatchRecoToMC.h"
#include "CbmRichGeoManager.h"
#include "CbmRichPoint.h"
#include "CbmStsPoint.h"
#include "CbmRichUtil.h"
#include "utils/CbmRichDraw.h"
#include "elid/CbmLitGlobalElectronId.h"
#include "CbmMCDataArray.h"
#include "CbmMCDataManager.h"

#include "FairLogger.h"
#include "FairRunAna.h"
#include "FairRunSim.h"
#include "FairEventHeader.h"
#include "CbmDaqBuffer.h"

#include "CbmUtils.h"
#include "CbmHistManager.h"

#include <iostream>
#include <string>
#include <sstream>

using namespace std;

CbmRichRecoTbMcQa::CbmRichRecoTbMcQa()
: FairTask("CbmRichRecoTbMcQa"),
fHM(nullptr),
fEventNum(0),
fOutputDir(""),
fMCTracks(nullptr),
fRichPoints(nullptr),
fStsPoints(nullptr),
fEventTime(0.),
fNofLogEvents(40)
{

}


InitStatus CbmRichRecoTbMcQa::Init()
{
    cout << "CbmRichRecoTbMcQa::Init"<<endl;
    FairRootManager* ioman = FairRootManager::Instance();
    if (nullptr == ioman) { Fatal("CbmRichRecoTbMcQa::Init","RootManager not instantised!"); }

    fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
    if (nullptr == fMCTracks) { Fatal("CbmRichRecoTbMcQa::Init", "No MC Tracks!"); }

    fRichPoints =(TClonesArray*) ioman->GetObject("RichPoint");
    if (nullptr == fRichPoints) { Fatal("CbmRichRecoTbMcQa::Init", "No Rich Points!");}

    fStsPoints =(TClonesArray*) ioman->GetObject("StsPoint");
    if (nullptr == fStsPoints) { Fatal("CbmRichRecoTbMcQa::Init", "No Sts Points!");}

    InitHistograms();

    return kSUCCESS;
}

void CbmRichRecoTbMcQa::InitHistograms()
{
    fHM = new CbmHistManager();

    fHM->Create1<TH1D>("fhRichPointTime", "fhRichPointTime;RICH MC point time [ns];Yield", 100, 0., 100.);
    fHM->Create1<TH1D>("fhRichPointTimeChPhoton", "fhRichPointTimeChPhoton;RICH MC point time [ns];Yield", 100, 0., 100.);
    fHM->Create1<TH1D>("fhRichPointTimeNotChPhoton", "fhRichPointTimeNotChPhoton;RICH MC point time [ns];Yield", 100, 0., 100.);
    fHM->Create1<TH1D>("fhRichPointTimePrimEl", "fhRichPointTimePrimEl;RICH MC point time [ns];Yield", 100, 0., 100.);
    fHM->Create1<TH1D>("fhRichPointTimeNotPrimEl", "fhRichPointTimeNotPrimEl;RICH MC point time [ns];Yield", 100, 0., 100.);


    fHM->Create1<TH1D>("fhStsPointTime", "fhStsPointTime;STS MC point time [ns];Yield", 400, 0., 400.);

    for (int iEv = 0; iEv < fNofLogEvents; iEv++) {
        string richStr = "fhRichPointTimeLog_" + to_string(iEv);
        fHM->Create1<TH1D>(richStr, richStr + ";Time [ns];Yield", 3000, 0., 3000.);

        string stsStr = "fhStsPointTimeLog_" + to_string(iEv);
        fHM->Create1<TH1D>(stsStr, stsStr + ";Time [ns];Yield", 3000, 0., 3000.);
    }

    fHM->Create1<TH1D>("fhTimeBetweenEvents", "fhTimeBetweenEvents;Time between events [ns];Yield", 100, 0., 500.);
}

void CbmRichRecoTbMcQa::Exec(
                         Option_t* option)
{
    fEventNum++;
    cout << "CbmRichRecoTbMcQa, event No. " <<  fEventNum << endl;

    ProcessMc();
}

void CbmRichRecoTbMcQa::ProcessMc()
{
    Int_t eventNum = FairRootManager::Instance()->GetEntryNr();
    Double_t oldEventTime = fEventTime;
    fEventTime = FairRun::Instance()->GetEventHeader()->GetEventTime();

    fHM->H1("fhTimeBetweenEvents")->Fill(fEventTime - oldEventTime);

    Int_t nofRichPoints = fRichPoints->GetEntries();
    for(Int_t j = 0; j < nofRichPoints; j++){
        CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(j);
        fHM->H1("fhRichPointTime")->Fill(point->GetTime());
        if (IsCherenkovPhoton(point)) {
            fHM->H1("fhRichPointTimeChPhoton")->Fill(point->GetTime());
        } else {
            fHM->H1("fhRichPointTimeNotChPhoton")->Fill(point->GetTime());
        }

        if (IsCherenkovPhotonFromPrimaryElectron(point)) {
            fHM->H1("fhRichPointTimePrimEl")->Fill(point->GetTime());
        } else {
            fHM->H1("fhRichPointTimeNotPrimEl")->Fill(point->GetTime());
        }

        if (eventNum < fNofLogEvents) {
            string richStr = "fhRichPointTimeLog_" + to_string(eventNum);
            fHM->H1(richStr)->Fill(fEventTime + point->GetTime());
        }
    }

    Int_t nofStsPoints = fStsPoints->GetEntries();
    for(Int_t j = 0; j < nofStsPoints; j++){
        CbmStsPoint* point = (CbmStsPoint*) fStsPoints->At(j);
        fHM->H1("fhStsPointTime")->Fill(point->GetTime());
        if (eventNum < fNofLogEvents) {
            string stsStr = "fhStsPointTimeLog_" + to_string(eventNum);
            fHM->H1(stsStr)->Fill(fEventTime + point->GetTime());
        }
    }
}


void CbmRichRecoTbMcQa::DrawHist()
{

    SetDefaultDrawStyle();

    {
		TCanvas* c = fHM->CreateCanvas("rich_tb_fhTimeBetweenEvents", "rich_tb_fhTimeBetweenEvents", 800, 800);
		DrawH1(fHM->H1("fhTimeBetweenEvents"), kLinear, kLog);
		Double_t min = 0.;
		Double_t max = 10.;
		Double_t partIntegral= fHM->H1("fhTimeBetweenEvents")->Integral(fHM->H1("fhTimeBetweenEvents")->FindBin(min), fHM->H1("fhTimeBetweenEvents")->FindBin(max));
		Double_t allIntegral = fHM->H1("fhTimeBetweenEvents")->Integral();
		Double_t ratio = 100. * partIntegral / allIntegral;
		cout << ratio << "% of all time between events are in range (" << min << " ," << max << ") ns" << endl;
	}


    {
        TCanvas* c = fHM->CreateCanvas("rich_tb_fhRichPointTime", "rich_tb_fhRichPointTime", 800, 800);
        DrawH1(fHM->H1("fhRichPointTime"), kLinear, kLog);
    }

    {
        TCanvas* c = fHM->CreateCanvas("rich_tb_fhRichPointTimeSources", "rich_tb_fhRichPointTimeSources", 1600, 800);
        c->Divide(2,1);
        c->cd(1);
        DrawH1({fHM->H1("fhRichPointTimeChPhoton"), fHM->H1("fhRichPointTimeNotChPhoton")}, {"Ch. Photons", "Not Ch. Photons"}, kLinear, kLog);

        c->cd(2);
        DrawH1({fHM->H1("fhRichPointTimeNotPrimEl"), fHM->H1("fhRichPointTimePrimEl")}, {"Not e^{#pm}_{prim}", "e^{#pm}_{prim}"}, kLinear, kLog);
    }


    {
        TCanvas* c = fHM->CreateCanvas("rich_tb_fhStsPointTime", "rich_tb_fhStsPointTime", 800, 800);
        DrawH1(fHM->H1("fhStsPointTime"), kLinear, kLog);
    }

    {
        TCanvas* c = fHM->CreateCanvas("rich_tb_fhRichPointTimeLog", "rich_tb_fhRichPointTimeLog", 1600, 600);
        CbmRichRecoTbMcQa::DrawTimeLog("fhRichPointTimeLog", fHM, fNofLogEvents);
    }

    {
        TCanvas* c = fHM->CreateCanvas("rich_tb_fhStsPointTimeLog", "rich_tb_fhStsPointTimeLog", 1600, 600);
        CbmRichRecoTbMcQa::DrawTimeLog("fhStsPointTimeLog", fHM, fNofLogEvents);
    }
}

void CbmRichRecoTbMcQa::DrawTimeLog(const string& hMainName, CbmHistManager* hm, Int_t nofLogEvents, bool withNoise)
{
    Double_t max = std::numeric_limits<Double_t>::min();
    Int_t startInd = (withNoise)?-1:0;
    for (int iEv = startInd; iEv < nofLogEvents; iEv++) {
        string hName = hMainName + "_" + to_string(iEv);
        string option = ((iEv==0 && !withNoise) || (iEv == -1 && withNoise))?"":"same";
        int ind = iEv % 5;
        int color = (ind == 0)?kBlue:(ind == 1)?kRed:(ind == 2)?kGreen+3:(ind == 3)?kMagenta + 2:kYellow + 2;
        if (ind == -1) color = kAzure + 6;
        max = std::max(max, hm->H1(hName)->GetMaximum());
        DrawH1(hm->H1(hName), kLinear, kLog, option, color);
    }
    hm->H1(hMainName + "_" + to_string(startInd))->SetMaximum(max * 1.10);
    gPad->SetLeftMargin(0.07);
    gPad->SetRightMargin(0.05);
}


void CbmRichRecoTbMcQa::Finish()
{
    DrawHist();
    fHM->SaveCanvasToImage(fOutputDir);

    TDirectory * oldir = gDirectory;
    TFile* outFile = FairRootManager::Instance()->GetOutFile();
    if (outFile != NULL) {
        outFile->cd();
        fHM->WriteToFile();
    }
    gDirectory->cd( oldir->GetPath() );
}

void CbmRichRecoTbMcQa::DrawFromFile(
      const string& fileName,
      const string& outputDir)
{
	fOutputDir = outputDir;

	if (fHM != nullptr) delete fHM;

	fHM = new CbmHistManager();
	TFile* file = new TFile(fileName.c_str());
	fHM->ReadFromFile(file);

	DrawHist();

	fHM->SaveCanvasToImage(fOutputDir);
}

bool CbmRichRecoTbMcQa::IsCherenkovPhoton(CbmRichPoint* point)
{
    if (point == nullptr) return false;
    Int_t trackId = point->GetTrackID();
    if (trackId < 0) return false;
    CbmMCTrack* p = (CbmMCTrack*) fMCTracks->At(trackId);
    return (p != nullptr && TMath::Abs(p->GetPdgCode()) == 50000050);

}

bool CbmRichRecoTbMcQa::IsCherenkovPhotonFromPrimaryElectron(CbmRichPoint* point)
{
    if (point == nullptr) return false;
    Int_t trackId = point->GetTrackID();
    if (trackId < 0) return false;
    CbmMCTrack* mcTrack = (CbmMCTrack*) fMCTracks->At(trackId);
    if (mcTrack == nullptr || TMath::Abs(mcTrack->GetPdgCode()) != 50000050) return false;

    Int_t motherId = mcTrack->GetMotherId();
    if (motherId < 0) return false;
    CbmMCTrack* mcMotherTrack = (CbmMCTrack*) fMCTracks->At(motherId);
    return IsMcPrimaryElectron(mcMotherTrack);
}

bool CbmRichRecoTbMcQa::IsMcPrimaryElectron(
            const CbmMCTrack* mctrack)
{
    if (mctrack == nullptr) return false;
    int pdg = TMath::Abs(mctrack->GetPdgCode());
    if (mctrack->GetGeantProcessId() == kPPrimary && pdg == 11) return true;
    return false;
}

ClassImp(CbmRichRecoTbMcQa)


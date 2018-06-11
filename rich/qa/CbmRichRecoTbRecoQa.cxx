#include "CbmRichRecoTbRecoQa.h"
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
#include "CbmRichDigi.h"

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
#include "CbmRichRecoTbMcQa.h"

using namespace std;

CbmRichRecoTbRecoQa::CbmRichRecoTbRecoQa()
: FairTask("CbmRichRecoTbRecoQa"),
  fHM(nullptr),
  fOutputDir(""),
  fMCTracks(nullptr),
  fRichPoints(nullptr),
  fStsPoints(nullptr),
  fRichDigis(nullptr),
  fRichHits(nullptr),
  fTimeSliceNum(0.),
  fNofLogEvents(40)
{

}


InitStatus CbmRichRecoTbRecoQa::Init()
{
    cout << "CbmRichRecoTbRecoQa::Init"<<endl;
    FairRootManager* ioman = FairRootManager::Instance();
    if (nullptr == ioman) { Fatal("CbmRichRecoTbRecoQa::Init","RootManager not instantised!"); }

    CbmMCDataManager* mcManager = (CbmMCDataManager*)ioman->GetObject("MCDataManager");
    if (mcManager == nullptr) LOG(FATAL) << "CbmRichRecoTbRecoQa::ReadAndCreateDataBranches() NULL MCDataManager." << FairLogger::endl;

    fMCTracks = mcManager->InitBranch("MCTrack");
    fRichPoints = mcManager->InitBranch("RichPoint");
    fStsPoints = mcManager->InitBranch("StsPoint");

    fRichDigis = (TClonesArray*) ioman->GetObject("RichDigi");
    if (nullptr == fRichDigis) { Fatal("CbmRichRecoTbRecoQa::Init", "No Rich digis!");}

    fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if (nullptr == fRichHits) { Fatal("CbmRichRecoTbRecoQa::Init", "No Rich hits!");}

    InitHistograms();

    return kSUCCESS;
}

void CbmRichRecoTbRecoQa::InitHistograms()
{
    fHM = new CbmHistManager();

    fHM->Create1<TH1D>("fhNofRichDigisPerTS", "fhNofRichDigisPerTS;RICH digis per time slice;Yield", 200, -1, -1.);
    fHM->Create1<TH1D>("fhNofRichHitsPerTS", "fhNofRichHitsPerTS;RICH hits per time slice;Yield", 200, -1., -1.);

    fHM->Create1<TH1D>("fhRichDigiTimeLog", "fhRichDigiTimeLog;RICH digi time [ns];Yield", 3000, 0., 3000.);

    // -1 for noise hits
    for (int iEv = -1; iEv < fNofLogEvents; iEv++) {
        string hName = "fhRichDigiTimeLog_" + to_string(iEv);
        fHM->Create1<TH1D>(hName, hName + ";Time [ns];Yield", 3000, 0., 3000.);
    }

}

void CbmRichRecoTbRecoQa::Exec(
        Option_t* option)
{
    fTimeSliceNum++;
    cout << "CbmRichRecoTbRecoQa, time slice:" <<  fTimeSliceNum << endl;

    Process();
}

void CbmRichRecoTbRecoQa::Process()
{
    Int_t nofRichDigis = fRichDigis->GetEntries();
    Int_t nofRichHits = fRichHits->GetEntries();
    fHM->H1("fhNofRichDigisPerTS")->Fill(nofRichDigis);
    fHM->H1("fhNofRichHitsPerTS")->Fill(nofRichHits);

    for (UInt_t iDigi= 0; iDigi < nofRichDigis; iDigi++) {
        CbmDigi* digi = static_cast<CbmRichDigi*>(fRichDigis->At(iDigi));
        fHM->H1("fhRichDigiTimeLog")->Fill(digi->GetTime());

        Int_t eventNum = digi->GetMatch()->GetMatchedLink().GetEntry();
        Int_t index = digi->GetMatch()->GetMatchedLink().GetIndex();

        if (eventNum < 0 || index < 0) {
            fHM->H1("fhRichDigiTimeLog_-1")->Fill(digi->GetTime());
        } else {
            if (eventNum < fNofLogEvents) {
                string hName = "fhRichDigiTimeLog_" + to_string(eventNum);
                fHM->H1(hName)->Fill(digi->GetTime());
            }
        }
    }
}

void CbmRichRecoTbRecoQa::DrawHist()
{
    SetDefaultDrawStyle();

    {
        TCanvas* c = fHM->CreateCanvas("rich_tb_reco_fhRichDigiTimeLogAll", "rich_tb_reco_fhRichDigiTimeLogAll", 1600, 600);
        DrawH1(fHM->H1("fhRichDigiTimeLog"), kLinear, kLog);
        gPad->SetLeftMargin(0.07);
        gPad->SetRightMargin(0.05);
    }

    {
        TCanvas* c = fHM->CreateCanvas("rich_tb_reco_fhRichDigiTimeLog", "rich_tb_reco_fhRichDigiTimeLog", 1600, 600);
        CbmRichRecoTbMcQa::DrawTimeLog("fhRichDigiTimeLog", fHM, fNofLogEvents, true);
    }

    {
        TCanvas* c = fHM->CreateCanvas("rich_tb_reco_fhRichObectsPerTimeSlice", "rich_tb_reco_fhRichObectsPerTimeSlice", 1600, 800);
        c->Divide(2, 1);
        c->cd(1);
        DrawH1(fHM->H1("fhNofRichDigisPerTS"), kLinear, kLog);
        c->cd(2);
        DrawH1(fHM->H1("fhNofRichHitsPerTS"), kLinear, kLog);
    }
}

void CbmRichRecoTbRecoQa::Finish()
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


ClassImp(CbmRichRecoTbRecoQa)


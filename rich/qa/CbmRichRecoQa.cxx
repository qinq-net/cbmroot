
#include "CbmRichRecoQa.h"

#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TF1.h"
#include "TStyle.h"
#include "TEllipse.h"
#include "TMarker.h"

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
#include "utils/CbmRichDraw.h"

#include "CbmUtils.h"
#include "CbmHistManager.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>
#include <sstream>

using namespace std;
using boost::assign::list_of;

CbmRichRecoQa::CbmRichRecoQa()
: FairTask("CbmRichRecoQa"),
fHM(NULL),
fEventNum(0),
fOutputDir(""),
fMCTracks(NULL),
fRichPoints(NULL),
fRichDigis(NULL),
fRichHits(NULL),
fRichRings(NULL),
fRichRingMatches(NULL),
fStsTrackMatches(NULL),
fStsTracks(NULL),
fGlobalTracks(NULL)
{

}


InitStatus CbmRichRecoQa::Init()
{
    cout << "CbmRichRecoQa::Init"<<endl;
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) { Fatal("CbmRichRecoQa::Init","RootManager not instantised!"); }
    
    fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
    if (NULL == fMCTracks) { Fatal("CbmRichRecoQa::Init", "No MC Tracks!"); }
    
    fRichPoints =(TClonesArray*) ioman->GetObject("RichPoint");
    if (NULL == fRichPoints) { Fatal("CbmRichRecoQa::Init", "No Rich Points!");}
    
    fRichDigis =(TClonesArray*) ioman->GetObject("RichDigi");
    if (NULL == fRichDigis) { Fatal("CbmRichRecoQa::Init", "No Rich Digis!");}
    
    fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if ( NULL == fRichHits) { Fatal("CbmRichRecoQa::Init","No RichHits!"); }
    
    fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
    if ( NULL == fRichRings) { Fatal("CbmRichRecoQa::Init","No RichRings!"); }
    
    fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
    if ( NULL == fRichRingMatches) { Fatal("CbmRichRecoQa::Init","No RichRingMatch array!"); }
    
    fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
    if (NULL == fGlobalTracks) { Fatal("CbmRichRecoQa::Init","No GlobalTrack array!"); }
    
    fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
    if (NULL == fStsTracks) { Fatal("CbmRichRecoQa::Init",": No StsTrack array!"); }
    
    fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
    if (NULL == fStsTrackMatches) { Fatal("CbmRichRecoQa::Init",": No StsTrackMatch array!"); }
    
    InitHistograms();
    
    return kSUCCESS;
}

void CbmRichRecoQa::InitHistograms()
{
    fHM = new CbmHistManager();
    
    double xMin = -120.;
    double xMax = 120.;
    int nBinsX = 30;
    double yMin = -208;
    double yMax = 208.;
    int nBinsY = 52;

    fHM->Create2<TH2D>("fh_ring_track_distance_vs_mom_truematch", "fh_ring_track_distance_vs_mom_truematch;P [GeV/c];Ring-track distance [cm];Yield (a.u.)", 20, 0., 10., 100, 0., 5.);
    fHM->Create2<TH2D>("fh_ring_track_distance_vs_mom_wrongmatch", "fh_ring_track_distance_vs_mom_wrongmatch;P [GeV/c];Ring-track distance [cm];Yield (a.u.)", 20, 0., 10., 100, 0., 5.);
    
    fHM->Create3<TH3D>("fh_ring_track_distance_vs_xy_truematch", "fh_ring_track_distance_vs_xy_truematch;X [cm];Y [cm];Ring-track distance [cm];Yield (a.u.)", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 100, 0., 5.);

}

void CbmRichRecoQa::Exec(
                         Option_t* option)
{
    fEventNum++;
    cout << "CbmRichRecoQa, event No. " <<  fEventNum << endl;
    
    Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
    for(Int_t iTrack = 0; iTrack < nofGlobalTracks; iTrack++) {
        const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTracks->At(iTrack));

        Int_t stsId = globalTrack->GetStsTrackIndex();
        Int_t richId = globalTrack->GetRichRingIndex();
        
        const CbmTrackMatchNew* stsTrackMatch = static_cast<const CbmTrackMatchNew*>(fStsTrackMatches->At(stsId));
        if (stsTrackMatch == NULL) continue;
        int stsMcTrackId = stsTrackMatch->GetMatchedLink().GetIndex();
        
        const CbmTrackMatchNew* richRingMatch = static_cast<const CbmTrackMatchNew*>(fRichRingMatches->At(richId));
        if (richRingMatch == NULL) continue;
        int richMcTrackId = richRingMatch->GetMatchedLink().GetIndex();
        const CbmRichRing* ring = static_cast<const CbmRichRing*>(fRichRings->At(richId));
        if (NULL == ring) continue;
        
        
        double rtDistance = ring->GetDistance();
        double xc = ring->GetCenterX();
        double yc = ring->GetCenterY();
        
        CbmMCTrack* mctrack = static_cast<CbmMCTrack*>(fMCTracks->At(stsMcTrackId));
        if (mctrack == NULL) continue;
        double mom = mctrack->GetP();
        
        if (stsMcTrackId == richMcTrackId) {
            fHM->H2("fh_ring_track_distance_vs_mom_truematch")->Fill(mom, rtDistance);
            fHM->H3("fh_ring_track_distance_vs_xy_truematch")->Fill(xc, yc, rtDistance);
        } else {
            fHM->H2("fh_ring_track_distance_vs_mom_wrongmatch")->Fill(mom, rtDistance);
        }
    }
}


void CbmRichRecoQa::DrawHist()
{
    cout.precision(4);
    
    SetDefaultDrawStyle();
    //fHM->ScaleByPattern("fh_.*", 1./fEventNum);
    {
        TCanvas* c = fHM->CreateCanvas("fh_ring_track_distance_vs_mom_truematch", "fh_ring_track_distance_vs_mom_truematch", 1200, 600);
        c->Divide(2,1);
        c->cd(1);
        DrawH2WithProfile(fHM->H2("fh_ring_track_distance_vs_mom_truematch"), true);
        c->cd(2);
        TH1D* py = (TH1D*)(fHM->H2("fh_ring_track_distance_vs_mom_truematch")->ProjectionY("fh_ring_track_distance_vs_mom_truematch_py")->Clone());
        DrawH1(py);
        double overflow = py->GetBinContent(py->GetNbinsX() + 1);
        string txt1 = Cbm::NumberToString<Double_t>(py->GetMean(), 2) + " / " + Cbm::NumberToString<Double_t>(py->GetRMS(), 2) + "/" + Cbm::NumberToString<Double_t>(100.* overflow / py->Integral(0, py->GetNbinsX() + 1), 2) + "%";
        DrawTextOnPad(txt1, 0.2, 0.9, 0.8, 0.99);
        gPad->SetLogy(true);
        
        fHM->H2("fh_ring_track_distance_vs_mom_truematch")->GetYaxis()->SetRangeUser(0., 3.);
    }
    
    {
        TCanvas* c = fHM->CreateCanvas("fh_ring_track_distance_vs_mom_wrongmatch", "fh_ring_track_distance_vs_mom_wrongmatch", 1200, 600);
        c->Divide(2,1);
        c->cd(1);
        DrawH2WithProfile(fHM->H2("fh_ring_track_distance_vs_mom_wrongmatch"), true);
        c->cd(2);
        TH1D* py = (TH1D*)(fHM->H2("fh_ring_track_distance_vs_mom_wrongmatch")->ProjectionY("fh_ring_track_distance_vs_mom_wrongmatch_py")->Clone());
        DrawH1(py);
        double overflow = py->GetBinContent(py->GetNbinsX() + 1);
        string txt1 = Cbm::NumberToString<Double_t>(py->GetMean(), 2) + " / " + Cbm::NumberToString<Double_t>(py->GetRMS(), 2) + "/" + Cbm::NumberToString<Double_t>(100.* overflow / py->Integral(0, py->GetNbinsX() + 1), 2) + "%";
        DrawTextOnPad(txt1, 0.2, 0.9, 0.8, 0.99);
        gPad->SetLogy(true);
        
        fHM->H2("fh_ring_track_distance_vs_mom_wrongmatch")->GetYaxis()->SetRangeUser(0., 3.);
    }
    
    {
    	fHM->CreateCanvas("fh_ring_track_distance_vs_xy_truematch", "fh_ring_track_distance_vs_xy_truematch", 600, 600);
    	DrawH3Profile(fHM->H3("fh_ring_track_distance_vs_xy_truematch"), false, false, 0., .1);
    }
}

void CbmRichRecoQa::Finish()
{
    DrawHist();
    fHM->SaveCanvasToImage(fOutputDir);
    fHM->WriteToFile();
}

ClassImp(CbmRichRecoQa)


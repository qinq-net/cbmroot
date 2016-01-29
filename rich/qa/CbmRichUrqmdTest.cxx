/**
 * \file CbmRichUrqmdTest.cxx
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2012
 **/

#include "CbmRichUrqmdTest.h"

#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"

#include "CbmMCTrack.h"
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

CbmRichUrqmdTest::CbmRichUrqmdTest()
: FairTask("CbmRichUrqmdTest"),
fHM(NULL),
fOutputDir(""),
fRichHits(NULL),
fRichRings(NULL),
fRichPoints(NULL),
fMcTracks(NULL),
fRichRingMatches(NULL),
fRichProjections(NULL),
fRichDigis(NULL),
fCanvas(),
fEventNum(0),
fMinNofHits(7),
fNofHitsInRingMap()
{
}

CbmRichUrqmdTest::~CbmRichUrqmdTest()
{
    
}

InitStatus CbmRichUrqmdTest::Init()
{
    cout << "CbmRichUrqmdTest::Init"<<endl;
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) { Fatal("CbmRichUrqmdTest::Init","RootManager not instantised!"); }
    
    fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if ( NULL == fRichHits) { Fatal("CbmRichUrqmdTest::Init","No RichHit array!"); }
    
    fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
    if ( NULL == fRichRings) { Fatal("CbmRichUrqmdTest::Init","No RichRing array!"); }
    
    fRichDigis = (TClonesArray*) ioman->GetObject("RichDigi");
    if ( NULL == fRichDigis) { Fatal("CbmRichUrqmdTest::Init","No RichDigi array!"); }
    
    fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
    if ( NULL == fRichPoints) { Fatal("CbmRichUrqmdTest::Init","No RichPoint array!"); }
    
    fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
    if ( NULL == fMcTracks) { Fatal("CbmRichUrqmdTest::Init","No MCTrack array!"); }
    
    fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
    if ( NULL == fRichRingMatches) { Fatal("CbmRichUrqmdTest::Init","No RichRingMatch array!"); }
    
    fRichProjections = (TClonesArray*) ioman->GetObject("RichProjection");
    if ( NULL == fRichProjections) { Fatal("CbmRichUrqmdTest::Init","No fRichProjections array!"); }
    
    InitHistograms();
    
    return kSUCCESS;
}

void CbmRichUrqmdTest::Exec(
                            Option_t* option)
{
    fEventNum++;
    
    cout << "CbmRichUrqmdTest, event No. " <<  fEventNum << endl;
    
    FillRichRingNofHits();
    NofRings();
    NofHitsAndPoints();
    NofProjections();
    Vertex();
    PmtXYSource();
    
    
}

void CbmRichUrqmdTest::InitHistograms()
{
    fHM = new CbmHistManager();
    
    fHM->Create1<TH1D>("fh_vertex_z", "fh_vertex_z;z [cm];Number of vertices per event", 350, -1., 350);
    fHM->Create2<TH2D>("fh_vertex_xy", "fh_vertex_xy;x [cm];y [cm];Number of vertices per event", 100, -200., 200., 100, -200., 200.);
    fHM->Create2<TH2D>("fh_vertex_xy_z100_180", "fh_vertex_xy_z100_180;x [cm];y [cm];Number of vertices per event", 100, -200., 200., 100, -200., 200.);
    fHM->Create2<TH2D>("fh_vertex_xy_z180_370", "fh_vertex_xy_z180_370;x [cm];y [cm];Number of vertices per event", 100, -200., 200., 100, -200., 200.);
    
    fHM->Create1<TH1D>("fh_nof_rings_1hit", "fh_nof_rings_1hit;Number of detected particles/event;Yield", 250, -.5, 249.5);
    fHM->Create1<TH1D>("fh_nof_rings_7hits", "fh_nof_rings_7hits;Number of detected particles/event;Yield", 250, -.5, 249.5 );
    fHM->Create1<TH1D>("fh_nof_rings_prim_1hit", "fh_nof_rings_prim_1hit;Number of detected particles/event;Yield", 50, -.5, 69.5);
    fHM->Create1<TH1D>("fh_nof_rings_prim_7hits", "fh_nof_rings_prim_7hits;Number of detected particles/event;Yield", 50, -.5, 69.5 );
    fHM->Create1<TH1D>("fh_nof_rings_target_1hit", "fh_nof_rings_target_1hit;Number of detected particles/event;Yield", 60, -.5, 79.5);
    fHM->Create1<TH1D>("fh_nof_rings_target_7hits", "fh_nof_rings_target_7hits;Number of detected particles/event;Yield", 60, -.5, 79.5 );
    
    fHM->Create1<TH1D>("fh_secel_mom", "fh_secel_mom;p [GeV/c];Number per event", 100, 0., 20);
    fHM->Create1<TH1D>("fh_gamma_target_mom", "fh_gamma_target_mom;p [GeV/c];Number per event", 100, 0., 20);
    fHM->Create1<TH1D>("fh_gamma_nontarget_mom", "fh_gamma_nontarget_mom;p [GeV/c];Number per event", 100, 0., 20);
    fHM->Create1<TH1D>("fh_pi_mom", "fh_pi_mom;p [GeV/c];Number per event", 100, 0., 20);
    fHM->Create1<TH1D>("fh_kaon_mom", "fh_kaon_mom;p [GeV/c];Number per event", 100, 0., 20);
    fHM->Create1<TH1D>("fh_mu_mom", "fh_mu_mom;p [GeV/c];Number per event", 100, 0., 20);
    
    fHM->Create1<TH1D>("fh_nof_points_per_event", "fh_nof_points_per_event;Particle;Number of MC points per event", 7, .5, 7.5);
    
    fHM->Create2<TH2D>("fh_points_xy", "fh_points_xy;x [cm];y [cm];Number of MC points/cm^{2}/event", 240, -120, 120, 420, -210, 210);
    fHM->Create2<TH2D>("fh_points_xy_pions", "fh_points_xy_pions;x [cm];y [cm];Number of MC points/cm^{2}/event", 240, -120, 120, 420, -210, 210);
    fHM->Create2<TH2D>("fh_points_xy_gamma_target", "fh_points_xy_gamma_target;x [cm];y [cm];Number of MC points/cm^{2}/event", 240, -120, 120, 420, -210, 210);
    fHM->Create2<TH2D>("fh_points_xy_gamma_nontarget", "fh_points_xy_gamma_nontarget;x [cm];y [cm];Number of MC points/cm^{2}/event", 240, -120, 120, 420, -210, 210);
    
    fHM->Create1<TH1D>("fh_nof_hits_per_event", "fh_nof_hits_per_event;Number of hits per event;Yield", 100, 0, 3000);
    fHM->Create2<TH2D>("fh_hits_xy", "fh_hits_xy;x [cm];y [cm];Number of hits/cm^{2}/event", 240, -120, 120, 420, -210, 210);
    
    // bin size is set to 1.2 cm in order to cover 4 pixels, before drawing must be normalized by 1/4
    fHM->Create2<TH2D>("fh_hitrate_xy", "fh_hitrate_xy;x [cm];y [cm];Number of hits/pixel/s", 200, -120, 120, 350, -210, 210);
    
    fHM->Create1<TH1D>("fh_nof_proj_per_event", "fh_nof_proj_per_event;Number of tracks per event;Yield", 50, 0, 1000);
    fHM->Create2<TH2D>("fh_proj_xy", "fh_proj_xy;x [cm];y [cm];Number of tracks/cm^{2}/event", 240, -120, 120, 420, -210, 210);
}

void CbmRichUrqmdTest::FillRichRingNofHits()
{
    fNofHitsInRingMap.clear();
    Int_t nofRichHits = fRichHits->GetEntriesFast();
    for (Int_t iHit=0; iHit < nofRichHits; iHit++) {
        CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(iHit));
        if (NULL == hit) continue;
        
        vector<Int_t> motherIds = CbmMatchRecoToMC::GetMcTrackMotherIdsForRichHit(hit, fRichDigis, fRichPoints, fMcTracks);
        for (Int_t i = 0; i < motherIds.size(); i++) {
            fNofHitsInRingMap[motherIds[i]]++;
        }
    }
}

void CbmRichUrqmdTest::NofRings()
{
    Int_t nofRings = fRichRings->GetEntriesFast();
    int nRings1hit = 0, nRings7hits = 0;
    int nRingsPrim1hit = 0, nRingsPrim7hits = 0;
    int nRingsTarget1hit = 0, nRingsTarget7hits = 0;
    for (Int_t iR = 0; iR < nofRings; iR++){
        CbmRichRing *ring = (CbmRichRing*) fRichRings->At(iR);
        if (NULL == ring) continue;
        // cout << "ring: nofHits=" << ring->GetNofHits() << endl;
        CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
        if (NULL == ringMatch) continue;
        // cout << "nofHits=" << ringMatch->GetNofHits() << endl;
        // continue;
        Int_t mcTrackId = ringMatch->GetMatchedLink().GetIndex();
        if (mcTrackId < 0) continue;
        CbmMCTrack* mcTrack = (CbmMCTrack*)fMcTracks->At(mcTrackId);
        if (NULL == mcTrack) continue;
        Int_t motherId = mcTrack->GetMotherId();
        Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
        double mom = mcTrack->GetP();
        TVector3 vert;
        mcTrack->GetStartVertex(vert);
        double dZ = vert.Z();
        
        if (motherId == -1 && pdg == 11) continue; // do not calculate embedded electrons
        
        int nofHits = ring->GetNofHits();
        if (nofHits >= 1) nRings1hit++;
        if (nofHits >= fMinNofHits) nRings7hits++;
        
        if (motherId == -1 && nofHits >= 1) nRingsPrim1hit++;
        if (motherId == -1 && nofHits >= fMinNofHits) nRingsPrim7hits++;
        
        if (dZ < 0.1 && nofHits >= 1) nRingsTarget1hit++;
        if (dZ < 0.1 && nofHits >= fMinNofHits) nRingsTarget7hits++;
        
        if (nofHits >= 1) {
            if (motherId != -1) {
                int motherPdg;
                CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
                if (NULL != mother) motherPdg = mother->GetPdgCode();
                if (motherId != -1 && pdg == 11 && motherPdg != 22) fHM->H1("fh_secel_mom")->Fill(mom);
                
                if (motherId != -1 && pdg == 11 && motherPdg == 22){
                    if (dZ < 0.1){
                        fHM->H1("fh_gamma_target_mom")->Fill(mom);
                    } else {
                        fHM->H1("fh_gamma_nontarget_mom")->Fill(mom);
                    }
                }
                
            }
            if (pdg == 211) fHM->H1("fh_pi_mom")->Fill(mom);
            if (pdg == 321) fHM->H1("fh_kaon_mom")->Fill(mom);
            if (pdg == 13) fHM->H1("fh_mu_mom")->Fill(mom);
        }
    }
    fHM->H1("fh_nof_rings_1hit")->Fill(nRings1hit);
    fHM->H1("fh_nof_rings_7hits")->Fill(nRings7hits);
    
    fHM->H1("fh_nof_rings_prim_1hit")->Fill(nRingsPrim1hit);
    fHM->H1("fh_nof_rings_prim_7hits")->Fill(nRingsPrim7hits);
    
    fHM->H1("fh_nof_rings_target_1hit")->Fill(nRingsTarget1hit);
    fHM->H1("fh_nof_rings_target_7hits")->Fill(nRingsTarget7hits);
}

void CbmRichUrqmdTest::NofHitsAndPoints()
{
    int nofHits = fRichHits->GetEntriesFast();
    fHM->H1("fh_nof_hits_per_event")->Fill(nofHits);
    for (int i = 0; i < nofHits; i++) {
        CbmRichHit* hit = (CbmRichHit*) fRichHits->At(i);
        if (NULL == hit) continue;
        fHM->H2("fh_hits_xy")->Fill(hit->GetX(), hit->GetY());
        fHM->H2("fh_hitrate_xy")->Fill(hit->GetX(), hit->GetY());
    }
    
    
    int nofPoints = fRichPoints->GetEntriesFast();
    for (int i = 0; i < nofPoints; i++) {
        CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(i);
        if (NULL == point) continue;
        fHM->H1("fh_nof_points_per_event")->Fill(1);
        
        Int_t mcPhotonTrackId = point->GetTrackID();
        if (mcPhotonTrackId < 0) continue;
        CbmMCTrack* mcPhotonTrack = (CbmMCTrack*)fMcTracks->At(mcPhotonTrackId);
        if (NULL == mcPhotonTrack) continue;
        Int_t motherPhotonId = mcPhotonTrack->GetMotherId();
        if (motherPhotonId < 0) continue;
        CbmMCTrack* mcTrack = (CbmMCTrack*)fMcTracks->At(motherPhotonId);
        if (NULL == mcTrack) continue;
        Int_t motherId = mcTrack->GetMotherId();

        Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
        TVector3 vert;
        mcTrack->GetStartVertex(vert);
        double dZ = vert.Z();

        if (motherId == -1 && pdg == 11) continue; // do not calculate embedded electrons
        
        if (motherId != -1) {
            int motherPdg;
            CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
            if (NULL != mother) motherPdg = mother->GetPdgCode();
            if (motherId != -1 && pdg == 11 && motherPdg != 22) fHM->H1("fh_nof_points_per_event")->Fill(2);
            
            if (motherId != -1 && pdg == 11 && motherPdg == 22){
                if (dZ < 0.1){
                    fHM->H1("fh_nof_points_per_event")->Fill(3);
                } else {
                    fHM->H1("fh_nof_points_per_event")->Fill(4);
                }
            }
            
        }
        if (pdg == 211) fHM->H1("fh_nof_points_per_event")->Fill(5);
        if (pdg == 321) fHM->H1("fh_nof_points_per_event")->Fill(6);
        if (pdg == 13) fHM->H1("fh_nof_points_per_event")->Fill(7);
    }
}

void CbmRichUrqmdTest::PmtXYSource()
{
    Int_t nofPoints = fRichPoints->GetEntries();
    for (int i = 0; i < nofPoints; i++) {
        CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(i);
        if (NULL == point) continue;
        
        Int_t iMCTrack = point->GetTrackID();
        CbmMCTrack* track = static_cast<CbmMCTrack*>(fMcTracks->At(iMCTrack));
        if (NULL == track) continue;
        
        Int_t iMother = track->GetMotherId();
        if (iMother == -1) continue;
        
        CbmMCTrack* track2 = static_cast<CbmMCTrack*>(fMcTracks->At(iMother));
        if (NULL == track2) continue;
        int pdg = TMath::Abs(track2->GetPdgCode());
        int motherId = track2->GetMotherId();
        TVector3 inPos (point->GetX(), point->GetY(), point->GetZ());
        TVector3 outPos;
        CbmRichGeoManager::GetInstance().RotatePoint(&inPos, &outPos);
        
        fHM->H2("fh_points_xy")->Fill(outPos.X(), outPos.Y());
        if (motherId != -1) {
            int motherPdg;
            CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
            if (NULL != mother) motherPdg = mother->GetPdgCode();
            TVector3 vert;
            track2->GetStartVertex(vert);
            if (motherId != -1 && pdg == 11 && motherPdg == 22){
                if (vert.Z() < 0.1){
                    fHM->H2("fh_points_xy_gamma_target")->Fill(outPos.X(), outPos.Y());
                } else {
                    fHM->H2("fh_points_xy_gamma_nontarget")->Fill(outPos.X(), outPos.Y());
                }
            }
        }
        if (pdg == 211) fHM->H2("fh_points_xy_pions")->Fill(outPos.X(), outPos.Y());
        
    }
}

void CbmRichUrqmdTest::NofProjections()
{
    if (fRichProjections == NULL) return;
    int nofProj = fRichProjections->GetEntriesFast();
    int nofGoodProj = 0;
    for (int i = 0; i < nofProj; i++){
        FairTrackParam* proj = (FairTrackParam*) fRichProjections->At(i);
        if (NULL == proj) continue;
        fHM->H2("fh_proj_xy")->Fill(proj->GetX(), proj->GetY());
        if (proj->GetX() != 0 && proj->GetY() != 0) nofGoodProj++;
    }
    fHM->H1("fh_nof_proj_per_event")->Fill(nofGoodProj);
}

void CbmRichUrqmdTest::Vertex()
{
    int nMcTracks = fMcTracks->GetEntries();
    for (int i = 0; i < nMcTracks; i++){
        //At least one hit in RICH
        if (fNofHitsInRingMap[i] < 1) continue;
        CbmMCTrack* mctrack = (CbmMCTrack*) fMcTracks->At(i);
        TVector3 v;
        mctrack->GetStartVertex(v);
        fHM->H1("fh_vertex_z")->Fill(v.Z());
        fHM->H2("fh_vertex_xy")->Fill(v.X(), v.Y());
        if (v.Z() >= 100 && v.Z() <=180){
            fHM->H2("fh_vertex_xy_z100_180")->Fill(v.X(), v.Y());
        }
        if (v.Z() >=180 && v.Z() <=370){
            fHM->H2("fh_vertex_xy_z180_370")->Fill(v.X(), v.Y());
        }
    } // nMcTracks
}

void CbmRichUrqmdTest::DrawHist()
{
    cout.precision(4);
    
    SetDefaultDrawStyle();
    
    {
        fHM->H1("fh_vertex_z")->Scale(1./fEventNum);
        TCanvas* c = CreateCanvas("rich_urqmd_vertex_z", "rich_urqmd_vertex_z", 800, 800);
        DrawH1(fHM->H1("fh_vertex_z"));
        gPad->SetLogy(true);
    }
    
    
    {
        fHM->H2("fh_vertex_xy")->Scale(1./fEventNum);
        TCanvas* c = CreateCanvas("rich_urqmd_vertex_xy", "rich_urqmd_vertex_xy", 800, 800);
        DrawH2(fHM->H2("fh_vertex_xy"));
    }
    
    {
        fHM->H2("fh_vertex_xy_z100_180")->Scale(1./fEventNum);
        TCanvas* c = CreateCanvas("rich_urqmd_vertex_xy_z100_180", "rich_urqmd_vertex_xy_z100_180", 800, 800);
        DrawH2(fHM->H2("fh_vertex_xy_z100_180"));
    }
    
    {
        fHM->H2("fh_vertex_xy_z180_370")->Scale(1./fEventNum);
        TCanvas* c = CreateCanvas("rich_urqmd_vertex_xy_z180_370", "rich_urqmd_vertex_xy_z180_370", 800, 800);
        DrawH2(fHM->H2("fh_vertex_xy_z180_370"));
    }
    
    {
        fHM->H1("fh_nof_points_per_event")->Scale(1./fEventNum);
        TCanvas* c = CreateCanvas("rich_urqmd_nof_points_per_event", "rich_urqmd_nof_points_per_event", 800, 800);
        //gStyle->SetPaintTextFormat("4.1f");
        string labels[7] = {"all", "e^{#pm}_{sec} other", "e^{#pm}_{target} from #gamma", "e^{#pm}_{not target} from #gamma", "#pi^{#pm}", "K^{#pm}", "#mu^{#pm}" };
        for (Int_t i = 1; i <= 7; i++){
            fHM->H1("fh_nof_points_per_event")->GetXaxis()->SetBinLabel(i, labels[i-1].c_str());
        }
        fHM->H1("fh_nof_points_per_event")->GetXaxis()->SetLabelSize(0.05);
        //fHM->H1("fh_nof_points_per_event")->SetMarkerSize(0.15);
        DrawH1(fHM->H1("fh_nof_points_per_event"), kLinear, kLog, "hist text0");
    }
    
    {
        TCanvas* c = CreateCanvas("rich_urqmd_nof_rings", "rich_urqmd_nof_rings", 800, 800);
        fHM->H1("fh_nof_rings_1hit")->Scale(1./fHM->H1("fh_nof_rings_1hit")->Integral());
        fHM->H1("fh_nof_rings_7hits")->Scale(1./fHM->H1("fh_nof_rings_7hits")->Integral());
        stringstream ss1, ss2;
        ss1 << "At least 1 hit detected (" << fHM->H1("fh_nof_rings_1hit")->GetMean()  << ")" ;
        ss2 << "At least 7 hits detected (" << fHM->H1("fh_nof_rings_7hits")->GetMean()  << ")" ;
        DrawH1(list_of(fHM->H1("fh_nof_rings_1hit"))(fHM->H1("fh_nof_rings_7hits")),
               list_of(ss1.str())(ss2.str()),
               kLinear, kLinear, true, 0.3, 0.85, 0.99, 0.99);
    }
    
    {
        TCanvas* c = CreateCanvas("rich_urqmd_nof_rings_prim", "rich_urqmd_nof_rings_prim", 800, 800);
        fHM->H1("fh_nof_rings_prim_1hit")->Scale(1./fHM->H1("fh_nof_rings_prim_1hit")->Integral());
        fHM->H1("fh_nof_rings_prim_7hits")->Scale(1./fHM->H1("fh_nof_rings_prim_7hits")->Integral());
        stringstream ss1, ss2;
        ss1 << "At least 1 hit detected (" << fHM->H1("fh_nof_rings_prim_1hit")->GetMean()  << ")" ;
        ss2 << "At least 7 hits detected (" << fHM->H1("fh_nof_rings_prim_7hits")->GetMean()  << ")" ;
        DrawH1(list_of(fHM->H1("fh_nof_rings_prim_1hit"))(fHM->H1("fh_nof_rings_prim_7hits")),
               list_of(ss1.str())(ss2.str()),
               kLinear, kLinear, true, 0.3, 0.85, 0.99, 0.99);
    }
    
    {
        TCanvas* c = CreateCanvas("rich_urqmd_nof_rings_target", "rich_urqmd_nof_rings_target", 800, 800);
        fHM->H1("fh_nof_rings_target_1hit")->Scale(1./fHM->H1("fh_nof_rings_target_1hit")->Integral());
        fHM->H1("fh_nof_rings_target_7hits")->Scale(1./fHM->H1("fh_nof_rings_target_7hits")->Integral());
        stringstream ss1, ss2;
        ss1 << "At least 1 hit detected (" << fHM->H1("fh_nof_rings_target_1hit")->GetMean()  << ")" ;
        ss2 << "At least 7 hits detected (" << fHM->H1("fh_nof_rings_target_7hits")->GetMean()   << ")" ;
        DrawH1(list_of(fHM->H1("fh_nof_rings_target_1hit"))(fHM->H1("fh_nof_rings_target_7hits")),
               list_of(ss1.str())(ss2.str()),
               kLinear, kLinear, true, 0.3, 0.85, 0.99, 0.99);
    }
    
    {
        TCanvas* c = CreateCanvas("rich_urqmd_sources_mom", "rich_urqmd_sources_mom", 800, 800);
        fHM->H1("fh_gamma_target_mom")->Scale(1./fEventNum);
        fHM->H1("fh_gamma_nontarget_mom")->Scale(1./fEventNum);
        fHM->H1("fh_secel_mom")->Scale(1./fEventNum);
        fHM->H1("fh_pi_mom")->Scale(1./fEventNum);
        fHM->H1("fh_kaon_mom")->Scale(1./fEventNum);
        fHM->H1("fh_mu_mom")->Scale(1./fEventNum);
        stringstream ss1, ss2, ss3, ss4, ss5, ss6;
        ss1 << "e^{#pm}_{target} from #gamma (" << fHM->H1("fh_gamma_target_mom")->GetEntries() / fEventNum  << ")" ;
        ss2 << "e^{#pm}_{not target} from #gamma (" << fHM->H1("fh_gamma_nontarget_mom")->GetEntries() / fEventNum  << ")" ;
        ss3 << "e^{#pm}_{sec} other (" << fHM->H1("fh_secel_mom")->GetEntries() / fEventNum  << ")" ;
        ss4 << "#pi^{#pm} (" << fHM->H1("fh_pi_mom")->GetEntries() / fEventNum  << ")" ;
        ss5 << "K^{#pm} (" << fHM->H1("fh_kaon_mom")->GetEntries() / fEventNum  << ")" ;
        ss6 << "#mu^{#pm} (" << fHM->H1("fh_mu_mom")->GetEntries() / fEventNum  << ")" ;
        DrawH1(list_of(fHM->H1("fh_gamma_target_mom"))
               (fHM->H1("fh_gamma_nontarget_mom"))
               (fHM->H1("fh_secel_mom"))
               (fHM->H1("fh_pi_mom"))
               (fHM->H1("fh_kaon_mom"))
               (fHM->H1("fh_mu_mom")),
               list_of(ss1.str())(ss2.str())(ss3.str())(ss4.str())(ss5.str())(ss6.str()),
               kLinear, kLog, true, 0.5, 0.7, 0.99, 0.99);
    }
    
    {
        TCanvas *c = CreateCanvas("rich_urqmd_hits_xy", "rich_urqmd_hits_xy", 800, 800);
        double binArea = fHM->H2("fh_hits_xy")->GetXaxis()->GetBinWidth(1) * fHM->H2("fh_hits_xy")->GetYaxis()->GetBinWidth(1);
        fHM->H2("fh_hits_xy")->Scale(1./(fEventNum * binArea));
        CbmRichDraw::DrawPmtH2(fHM->H2("fh_hits_xy"), c);
    }
    
    {
        TCanvas *c = CreateCanvas("rich_urqmd_points_xy", "rich_urqmd_points_xy", 800, 800);
        fHM->H2("fh_points_xy")->Scale(1./fEventNum);
        CbmRichDraw::DrawPmtH2(fHM->H2("fh_points_xy"), c);
    }
    
    {
        TCanvas *c = CreateCanvas("rich_urqmd_points_xy_pions", "rich_urqmd_points_xy_pions", 800, 800);
        fHM->H2("fh_points_xy_pions")->Scale(1./fEventNum);
        CbmRichDraw::DrawPmtH2(fHM->H2("fh_points_xy_pions"), c);
    }
    
    {
        TCanvas *c = CreateCanvas("rich_urqmd_points_xy_gamma_target", "rich_urqmd_points_xy_gamma_target", 800, 800);
        fHM->H2("fh_points_xy_gamma_target")->Scale(1./fEventNum);
        CbmRichDraw::DrawPmtH2(fHM->H2("fh_points_xy_gamma_target"), c);
    }
    
    {
        TCanvas *c = CreateCanvas("rich_urqmd_points_xy_gamma_nontarget", "rich_urqmd_points_xy_gamma_nontarget", 800, 800);
        fHM->H2("fh_points_xy_gamma_nontarget")->Scale(1./fEventNum);
        CbmRichDraw::DrawPmtH2(fHM->H2("fh_points_xy_gamma_nontarget"), c);
    }
    
    {
        TCanvas* c = CreateCanvas("rich_urqmd_nof_hits_per_event", "rich_urqmd_nof_hits_per_event", 800, 800);
        fHM->H1("fh_nof_hits_per_event")->Scale(1./fHM->H1("fh_nof_hits_per_event")->Integral());
        DrawH1andFitGauss(fHM->H1("fh_nof_hits_per_event"));
        cout << "Mean number of hits per event = " << fHM->H1("fh_nof_hits_per_event")->GetMean() << endl;
    }
    
    {
        TCanvas *c = CreateCanvas("rich_urqmd_hitrate_xy", "rich_urqmd_hitrate_xy", 800, 800);
        fHM->H2("fh_hitrate_xy")->Scale(1e7/(fEventNum * 4.));
        CbmRichDraw::DrawPmtH2(fHM->H2("fh_hitrate_xy"), c);
    }
    
    {
        TCanvas *c = CreateCanvas("rich_urqmd_proj_xy", "rich_urqmd_proj_xy", 800, 800);
        fHM->H2("fh_proj_xy")->Scale(1./fEventNum);
        CbmRichDraw::DrawPmtH2(fHM->H2("fh_proj_xy"), c);
    }
    
    {
        TCanvas* c = CreateCanvas("rich_urqmd_nof_proj_per_event", "rich_urqmd_nof_proj_per_event", 800, 800);
        fHM->H1("fh_nof_proj_per_event")->Scale(1./fEventNum);
        DrawH1andFitGauss(fHM->H1("fh_nof_proj_per_event"));
        cout << "Number of track projections per event = " << fHM->H1("fh_nof_proj_per_event")->GetMean() << endl;
    }
}

void CbmRichUrqmdTest::Finish()
{
    DrawHist();
    SaveCanvasToImage();
    fHM->WriteToFile();
}


TCanvas* CbmRichUrqmdTest::CreateCanvas(
                                        const string& name,
                                        const string& title,
                                        int width,
                                        int height)
{
    TCanvas* c = new TCanvas(name.c_str(), title.c_str(), width, height);
    fCanvas.push_back(c);
    return c;
}

void CbmRichUrqmdTest::SaveCanvasToImage()
{
    for (int i = 0; i < fCanvas.size(); i++)
    {
        Cbm::SaveCanvasAsImage(fCanvas[i], fOutputDir);
    }
}

ClassImp(CbmRichUrqmdTest)


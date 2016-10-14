
#include "CbmRichSmallPrototypeQa.h"

#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TF1.h"
#include "TStyle.h"
#include "TEllipse.h"
#include "TMarker.h"

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

CbmRichSmallPrototypeQa::CbmRichSmallPrototypeQa()
: FairTask("CbmRichSmallPrototypeQa"),
fHM(NULL),
fEventNum(0),
fOutputDir(""),
fMCTracks(NULL),
fRichPoints(NULL),
fRichDigis(NULL),
fRichHits(NULL),
fRichRings(NULL),
fRichRingMatches(NULL),
fRefPlanePoints(NULL)
{
}


InitStatus CbmRichSmallPrototypeQa::Init()
{
    
    cout << "CbmRichSmallPrototypeQa::Init"<<endl;
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) { Fatal("CbmRichSmallPrototypeQa::Init","RootManager not instantised!"); }
    
    
    fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
    if (NULL == fMCTracks) { Fatal("CbmRichSmallPrototypeQa::Init", "No MC Tracks!"); }
    
    fRichPoints =(TClonesArray*) ioman->GetObject("RichPoint");
    if (NULL == fRichPoints) { Fatal("CbmRichSmallPrototypeQa::Init", "No Rich Points!");}
    
    fRichDigis =(TClonesArray*) ioman->GetObject("RichDigi");
    if (NULL == fRichDigis) { Fatal("CbmRichSmallPrototypeQa::Init", "No Rich Digis!");}
    
    fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if ( NULL == fRichHits) { Fatal("CbmRichSmallPrototypeQa::Init","No RichHits!"); }
    
    fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
    if ( NULL == fRichRings) { Fatal("CbmRichSmallPrototypeQa::Init","No RichRings!"); }
    
    fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
    if ( NULL == fRichRingMatches) { Fatal("CbmRichGeoTest::Init","No RichRingMatch array!"); }
    
    fRefPlanePoints = (TClonesArray*) ioman->GetObject("RefPlanePoint");
    if ( NULL == fRefPlanePoints) { Fatal("CbmRichSmallPrototypeQa::Init","No RefPlanePoints!"); }
    
    InitHistograms();
    
    return kSUCCESS;
}

void CbmRichSmallPrototypeQa::InitHistograms()
{
    fHM = new CbmHistManager();
    
    
    //RICH
    fHM->Create1<TH1D>("fh_nof_rich_points", "fh_nof_rich_points;# RICH Points per event;Yield (a.u.)", 1000, -.5, 999.5);
    fHM->Create1<TH1D>("fh_nof_rich_digis", "fh_nof_rich_digis;# RICH Digis per event;Yield (a.u.)", 200, -.5, 199.5);
    fHM->Create1<TH1D>("fh_nof_rich_hits", "fh_nof_rich_hits;# RICH hits per event;Yield (a.u.)", 100, -.5, 99.5);
    fHM->Create1<TH1D>("fh_nof_rich_rings", "fh_nof_rich_rings;# RICH rings;# per event", 5, -0.5, 4.5);
    fHM->Create1<TH1D>("fh_nof_good_rings", "fh_nof_good_rings;Nof good rings;# per event", 5, -0.5, 4.5);
    fHM->Create1<TH1D>("fh_rich_ring_radius","fh_rich_ring_radius;Ring Radius [cm];Yield (a.u.)", 300, 4., 7.);
    fHM->Create2<TH2D>("fh_rich_points_xy", "fh_rich_points_xy;X [cm];Y [cm];Yield (a.u.)", 1000, -10., 10., 700, -10., 10.);
    
    Float_t XBins[] = {-7.725, -7.11875, -6.5125, -5.90625, -5.3, -4.69375, -4.0875, -3.48125, -2.875, -2.425, -1.81875, -1.2125, -0.60625, 0., 0.60625, 1.2125, 1.81875, 2.425, 2.875, 3.48125, 4.0875, 4.69375, 5.3, 5.90625, 6.5125, 7.11875, 7.725};
    Float_t YBins[] = {-12.075, -11.46875, -10.8625, -10.25625, -9.65, -9.04375, -8.4375, -7.83125, -7.225, -6.775, -6.16875, -5.5625, -4.95625, -4.35, -3.74375, -3.1375, -2.53125, -1.925, 1.925, 2.53125, 3.1375, 3.74375, 4.35, 4.95625, 5.5625, 6.16875, 6.775, 7.225, 7.83125, 8.4375, 9.04375, 9.65, 10.25625, 10.8625, 11.46875, 12.075};
    Int_t NBinsX = sizeof(XBins)/sizeof(Float_t) - 1;
    Int_t NBinsY = sizeof(YBins)/sizeof(Float_t) - 1;
    fHM->Add("fh_rich_hits_xy", new TH2D("fh_rich_hits_xy", "fh_rich_hits_xy;X [cm];Y [cm];Yield (a.u.)", NBinsX, XBins, NBinsY, YBins));
    
    fHM->Add("fh_rich_hits_xy_dR>", new TH2D("fh_rich_hits_xy_dR>", "fh_rich_hits_xy_dR>;X [cm];Y [cm];Yield (a.u.)", NBinsX, XBins, NBinsY, YBins));

    fHM->Add("fh_rich_hits_xy_dR<", new TH2D("fh_rich_hits_xy_dR<", "fh_rich_hits_xy_dR<;X [cm];Y [cm];Yield (a.u.)", NBinsX, XBins, NBinsY, YBins));
    
    fHM->Create2<TH2D>("fh_nonphoton_pmt_points_xy","fh_nonphoton_pmt_points_xy;X [cm];Y [cm];Yield (a.u.)", 50, -10., 10., 50, -10., 10.);
    

    fHM->Create1<TH1D>("fh_hits_per_ring", "fh_hits_per_ring;Hits per Ring;Yield (a.u.)", 58, 2.5, 60.5);
    
    fHM->Create2<TH2D>("fh_ring_center_xy","fh_ring_center_xy;X [cm];Y [cm];Yield (a.u.)", 100, -10, 10, 100, -10, 10);
    
    fHM->Create2<TH2D>("fh_proton_start_xy","fh_proton_start_xy;X [cm];Y [cm];Yield (a.u.)", 100, -2., 2., 100, -2., 2.);
    fHM->Create1<TH1D>("fh_dR","fh_dR;dR [cm];Yield(a.u.)", 80, -0.8, 0.8);
    
    fHM->Create2<TH2D>("fh_refplane_xy_all", "fh_refplane_xy_all;X [cm];Y [cm];# per event", 100, -10., 10, 100, -10., 10.);
    fHM->Create2<TH2D>("fh_refplane_xy_prim", "fh_refplane_xy_prim;X [cm];Y [cm];# per event", 100, -10., 10., 100, -10., 10.);
    fHM->Create2<TH2D>("fh_refplane_xy_sec", "fh_refplane_xy_sec;X [cm];Y [cm];# per event", 100, -10., 10., 100, -10., 10.);
    fHM->Create1<TH1D>("fh_refplane_pdg", "fh_refplane_pdg;PDG Code;# per event", 6000, -3000., 3000.);
    
    fHM->Create1<TH1D>("fh_refplane_nof_particles_per_event", "fh_refplane_nof_particles_per_event;Particle;# per event", 6, -0.5, 5.5);
}

void CbmRichSmallPrototypeQa::Exec(
                                   Option_t* /*option*/)
{
    fEventNum++;
    cout << "CbmRichSmallPrototypeQa, event No. " <<  fEventNum << endl;
    
    Int_t nofMCTracks = fMCTracks->GetEntriesFast();
    Int_t nofRichPoints = fRichPoints->GetEntriesFast();
    Int_t nofRichDigis = fRichDigis->GetEntriesFast();
    Int_t nofRichHits = fRichHits->GetEntriesFast();
    Int_t nofRichRings = fRichRings->GetEntriesFast();
    Int_t nofRefPlanePoints = fRefPlanePoints->GetEntriesFast();
    
    fHM->H1("fh_nof_rich_points")->Fill(nofRichPoints);
    fHM->H1("fh_nof_rich_digis")->Fill(nofRichDigis);
    fHM->H1("fh_nof_rich_hits")->Fill(nofRichHits);
    fHM->H1("fh_nof_rich_rings")->Fill(nofRichRings);
    
    
    for (int i = 0; i < nofRichHits; i++) {
        CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(i));
        fHM->H2("fh_rich_hits_xy")->Fill(hit->GetX(), hit->GetY());
    }
    
    for(int i = 0; i < nofRichPoints; i++) {
        CbmRichPoint* point= static_cast<CbmRichPoint*>(fRichPoints->At(i));
        if (point == NULL) continue;
        fHM->H2("fh_rich_points_xy")->Fill(point->GetX(), point->GetY());
        
        Int_t trackid = point->GetTrackID();
        if (trackid < 0) continue;
        CbmMCTrack* mctrack= static_cast<CbmMCTrack*>(fMCTracks->At(trackid));
        Int_t pdg = mctrack->GetPdgCode();
        if(pdg != 50000050) {
            fHM->H2("fh_nonphoton_pmt_points_xy")->Fill(point->GetX(), point->GetY());
        }
    }
    
    for( int i = 0; i < nofMCTracks; i++) {
        CbmMCTrack* mctrack = static_cast<CbmMCTrack*>(fMCTracks->At(i));
        Double_t pdg = mctrack->GetPdgCode();
        Double_t motherId = mctrack->GetMotherId();
        if (pdg == 2212 && motherId < 0) {
            fHM->H2("fh_proton_start_xy")->Fill(mctrack->GetStartX(), mctrack->GetStartY());
        }
    }
    
    
    for(int iR = 0; iR < nofRichRings; iR++) {
        CbmRichRing* ring = static_cast<CbmRichRing*>(fRichRings->At(iR));
        if (NULL == ring) continue;
        int nofHits = ring->GetNofHits();
        CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
        if (NULL == ringMatch) continue;
        Int_t mcTrackId = ringMatch->GetMatchedLink().GetIndex();
        if (mcTrackId < 0) continue;
        CbmMCTrack* mcTrack = (CbmMCTrack*)fMCTracks->At(mcTrackId);
        if (mcTrack == NULL) continue;
        Int_t motherId = mcTrack->GetMotherId();
        Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
        //select only primary protons
        if (!(motherId == -1 && pdg == 2212)) continue;
        if (nofHits > 40) continue;
        
        if(nofHits>2) {
            fHM->H1("fh_nof_good_rings")->Fill(iR+1);
        }
        
        Double_t cX = ring->GetCenterX();
        Double_t cY = ring->GetCenterY();
        fHM->H2("fh_ring_center_xy")->Fill(cX,cY);
        fHM->H1("fh_hits_per_ring")->Fill(nofHits);
        
        Double_t radius = ring->GetRadius();
        fHM->H1("fh_rich_ring_radius")->Fill(radius);
        if(radius<4.0) continue;
        for (int iH = 0; iH < nofHits; iH++) {
            Int_t hitInd = ring->GetHit(iH);
            CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            if (NULL == hit) continue;
            Double_t hitX = hit->GetX();
            Double_t hitY = hit->GetY();
            if(hitX<3.0 && hitX>-3.0 && hitY<3.0 && hitY>-3.0) continue;
            Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            fHM->H1("fh_dR")->Fill(dR);
            
            if(dR>0)
            {
                for(int i = 0; i < nofRichHits; i++)
                {
                    
                    fHM->H2("fh_rich_hits_xy_dR>")->Fill(hitX, hitY);
                }
            }
            if(dR<0)
            {
                for(int j = 0; j < nofRichHits; j++)
                {
                    fHM->H2("fh_rich_hits_xy_dR<")->Fill(hitX, hitY);
                }
            }
        }
        
    }

    for (Int_t i = 0; i<nofRefPlanePoints; i++)
    {
        CbmRichPoint* point = (CbmRichPoint*) (fRefPlanePoints->At(i));
        
        Int_t trackid = point->GetTrackID();
        if( trackid < 0) continue;
        CbmMCTrack* mctrack= static_cast<CbmMCTrack*>(fMCTracks->At(trackid));
        if(mctrack ==NULL) continue;
        Int_t pdg = mctrack->GetPdgCode();
        Int_t motherId = mctrack->GetMotherId();
        fHM->H1("fh_refplane_pdg")->Fill(pdg);
        
        if(pdg == 211) {
            fHM->H1("fh_refplane_nof_particles_per_event")->Fill(0);
        } else if(pdg == -211) {
            fHM->H1("fh_refplane_nof_particles_per_event")->Fill(1);
        } else if(pdg == 11) {
            fHM->H1("fh_refplane_nof_particles_per_event")->Fill(2);
        } else if(pdg == -11) {
            fHM->H1("fh_refplane_nof_particles_per_event")->Fill(3);
        } else if(pdg == 13) {
            fHM->H1("fh_refplane_nof_particles_per_event")->Fill(4);
        } else if(pdg == -13) {
            fHM->H1("fh_refplane_nof_particles_per_event")->Fill(5);
        }

        fHM->H2("fh_refplane_xy_all")->Fill(point->GetX(), point->GetY());
        if (motherId >= 0){
            fHM->H2("fh_refplane_xy_sec")->Fill(point->GetX(), point->GetY());
        } else {
            fHM->H2("fh_refplane_xy_prim")->Fill(point->GetX(), point->GetY());
        }
    }
    
    if (fEventNum < 20) {
        DrawEvent();
    }
}


void CbmRichSmallPrototypeQa::DrawHist()
{
    cout.precision(4);
    
    SetDefaultDrawStyle();
    fHM->ScaleByPattern("fh_.*", 1./fEventNum);
    
    
    {
        TCanvas* c = fHM->CreateCanvas("richsp_nof_rich_hits_points", "richsp_nof_rich_hits_points", 1200, 600);
        c->Divide(2,1);
        c->cd(1);
        DrawH1andFitGauss(fHM->H1("fh_nof_rich_points"));
        //gPad->SetLogy(true);
        c->cd(2);
        DrawH1andFitGauss(fHM->H1("fh_nof_rich_hits"));
    }
    
    {
        TCanvas* c = fHM->CreateCanvas("richsp_rich_points_xy", "richsp_rich_points_xy", 600, 600);
        c->SetLogz();
        DrawH2(fHM->H2("fh_rich_points_xy"));
        fHM->H2("fh_rich_points_xy")->SetTitle("Rich Points");
    }
    {
        TCanvas* c = fHM->CreateCanvas("rich_hits_xy", "rich_hits_xy", 600, 600);
        c->SetLogz();
        DrawH2(fHM->H2("fh_rich_hits_xy"));
        fHM->H2("fh_rich_hits_xy")->SetTitle("Rich Hits");
    }

    
    {
        fHM->CreateCanvas("richsp_refplane_nof_particles_per_event", "richsp_refplane_nof_particles_per_event", 600, 600);
        DrawH1(fHM->H1("fh_refplane_nof_particles_per_event"));
        fHM->H1("fh_refplane_nof_particles_per_event")->SetTitle("Number of particles on reference plane");
        string labels[6] = {"#pi^{+}", "#pi^{-}", "e^{+}", "e^{-}", "#mu^{+}", "#mu^{-}"};
        for (Int_t i = 1; i <= 6; i++){
            fHM->H1("fh_refplane_nof_particles_per_event")->GetXaxis()->SetBinLabel(i, labels[i-1].c_str());
        }
    }
    
    {
        TCanvas* c = fHM->CreateCanvas("richsp_rich_hits_xy_dR", "richsp_rich_hits_xy_dR", 1200, 600);
        c->Divide(2,1);
        c->cd(1);
        c->SetLogz();
        DrawH2(fHM->H2("fh_rich_hits_xy_dR>"));
        fHM->H2("fh_rich_hits_xy_dR>")->SetTitle("Rich Hits dR>");
        c->cd(2);
        c->SetLogz();
        DrawH2(fHM->H2("fh_rich_hits_xy_dR<"));
        fHM->H2("fh_rich_hits_xy_dR<")->SetTitle("Rich Hits dR<");
    }
    
    
    {
        TCanvas* c = fHM->CreateCanvas("richsp_proton_start_xy", "richsp_proton_start_xy", 600, 600);
        c->SetLogz();
        DrawH2(fHM->H2("fh_proton_start_xy"));
        fHM->H2("fh_proton_start_xy")->SetTitle("Proton start XY");
    }
    
    {
        fHM->CreateCanvas("richsp_rich_ring_radius", "richsp_rich_ring_radius", 600, 600);
        DrawH1andFitGauss(fHM->H1("fh_rich_ring_radius"));
        fHM->H1("fh_rich_ring_radius")->SetTitle("Ring Radius");
    }
    
    {
        fHM->CreateCanvas("richsp_nof_rich_rings", "richsp_nof_rich_rings", 600, 600);
        DrawH1(fHM->H1("fh_nof_rich_rings"));
        fHM->H1("fh_nof_rich_rings")->SetTitle("Nof Rich Rings");
    }
    
    {
    	fHM->CreateCanvas("richsp_dR", "richsp_dR", 600, 600);
        DrawH1andFitGauss(fHM->H1("fh_dR"));
        fHM->H1("fh_dR")->SetTitle("dR");
    }
    
    {
        TCanvas* c = fHM->CreateCanvas("richsp_refplane_xy", "richsp_refplane_xy", 1500, 500);
        c->Divide(3,1);
        c->cd(1);
        DrawH2(fHM->H2("fh_refplane_xy_all"));
        DrawTextOnPad("All particles", 0.1, 0.9, .7, .99);
        c->cd(2);
        DrawH2(fHM->H2("fh_refplane_xy_prim"));
        DrawTextOnPad("Primary particles", 0.1, 0.9, .7, .99);
        //c->SetLogz();
        c->cd(3);
        DrawH2(fHM->H2("fh_refplane_xy_sec"));
        DrawTextOnPad("Secondary particle", 0.1, 0.9, .7, .99);
    }
    
    {
        TCanvas* c = fHM->CreateCanvas("richsp_nonphoton_pmt_points_xy", "richsp_nonphoton_pmt_points_xy", 600, 600);
        c->SetLogz();
        DrawH2(fHM->H2("fh_nonphoton_pmt_points_xy"));
        fHM->H2("fh_nonphoton_pmt_points_xy")->SetTitle("Non photons on PMT plane");
    }
    
    {
        TCanvas* c = fHM->CreateCanvas("richsp_refplane_pdg", "richsp_refplane_pdg", 1200, 600);
        c->SetLogy();
        DrawH1(fHM->H1("fh_refplane_pdg"));
        fHM->H1("fh_refplane_pdg")->SetTitle("Reference plane, PDG codes");
    }
    
    {
        fHM->CreateCanvas("richsp_hits_per_ring", "richsp_hits_per_ring", 600, 600);
        DrawH1andFitGauss(fHM->H1("fh_hits_per_ring"));
        fHM->H1("fh_hits_per_ring")->SetTitle("Hits per Ring");
    }
    
    {
        TCanvas* c = fHM->CreateCanvas("richsp_ring_center_xy", "richsp_ring_center_xy", 600, 600);
        c->SetLogz();
        DrawH2(fHM->H2("fh_ring_center_xy"));
        fHM->H1("fh_ring_center_xy")->SetTitle("Ring center");
    }
    
    {
        fHM->CreateCanvas("richsp_nof_good_rings", "richsp_nof_good_rings", 600, 600);
        DrawH1(fHM->H1("fh_nof_good_rings"));
        fHM->H1("fh_nof_good_rings")->SetTitle("Nof good Rings");
    }
}


void CbmRichSmallPrototypeQa::DrawEvent()
{
    stringstream ss;
    ss << "richsp_event_display_event_"<< fEventNum;
    TCanvas *c = fHM->CreateCanvas(ss.str().c_str(), ss.str().c_str(), 705, 800);
    TH2D* pad = new TH2D("event_display_pad", ";X [cm];Y [cm]", 1, -12., 12., 1, -12., 12);
    DrawH2(pad);
    pad->GetYaxis()->SetTitleOffset(0.9);
    gPad->SetLeftMargin(0.13);
    gPad->SetRightMargin(0.05);
    
    // Draw hits
    int nofHits = fRichHits->GetEntriesFast();
    for (int iH = 0; iH < nofHits; iH++){
        CbmRichHit* hit = (CbmRichHit*) fRichHits->At(iH);
        if (NULL == hit) continue;
        TEllipse* hitDr = new TEllipse(hit->GetX(), hit->GetY(), 0.5);
        hitDr->SetFillColor(kRed);
        hitDr->SetLineColor(kRed);
        hitDr->Draw();
    }
    
    // Draw RICH MC Points
    int nofPoints = fRichPoints->GetEntriesFast();
    for (int iP = 0; iP < nofPoints; iP++){
        CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(iP);
        if (NULL == point) continue;
        TEllipse* pointDr = new TEllipse(point->GetX(), point->GetY(), 0.15);
        pointDr->SetFillColor(kBlue);
        pointDr->SetLineColor(kBlue);
        pointDr->Draw();
    }
    
    //Draw proton start XY
    for( int i = 0; i < fMCTracks->GetEntriesFast(); i++) {
        CbmMCTrack* mctrack= static_cast<CbmMCTrack*>(fMCTracks->At(i));
        if (mctrack == NULL) continue;
        Double_t pdg = mctrack->GetPdgCode();
        Double_t motherId = mctrack->GetMotherId();
        if (pdg == 2212 && motherId < 0) {
            TEllipse* pointDr = new TEllipse(mctrack->GetStartX(), mctrack->GetStartY(), 0.35);
            pointDr->SetFillColor(kGreen+3);
            pointDr->SetLineColor(kGreen+3);
            pointDr->Draw();
            
        }
    }
    
    // Draw proton ref plane
    for (Int_t i = 0; i < fRefPlanePoints->GetEntriesFast(); i++) {
        CbmRichPoint* point = static_cast<CbmRichPoint*>(fRefPlanePoints->At(i));
        Int_t trackid = point->GetTrackID();
        if( trackid < 0) continue;
        CbmMCTrack* mctrack = static_cast<CbmMCTrack*>(fMCTracks->At(trackid));
        if(mctrack ==NULL) continue;
        Int_t pdg = mctrack->GetPdgCode();
        Int_t motherId = mctrack->GetMotherId();
        if (pdg == 2212 && motherId < 0) {
            TEllipse* pointDr = new TEllipse(point->GetX(), point->GetY(), 0.25);
            pointDr->SetFillColor(kYellow+1);
            pointDr->SetLineColor(kYellow+1);
            pointDr->Draw();
        }
    }
    
    // Draw rings
    int nofRings = fRichRings->GetEntriesFast();
    for (int iR = 0; iR < nofRings; iR++){
        CbmRichRing* ring = (CbmRichRing*) fRichRings->At(iR);
        if (NULL == ring) continue;
        DrawCircle(ring);
    }
    
}

void CbmRichSmallPrototypeQa::DrawCircle(
                                         CbmRichRing* ring)
{
    TEllipse* circle = new TEllipse(ring->GetCenterX(), ring->GetCenterY(), ring->GetRadius());
    circle->SetFillStyle(0);
    circle->SetLineWidth(4);
    circle->SetLineColor(kBlack);
    circle->Draw();
    
    TEllipse* center = new TEllipse(ring->GetCenterX(), ring->GetCenterY(), 0.2);
    center->SetFillColor(kBlack);
    center->SetLineColor(kBlack);
    center->Draw();
}


void CbmRichSmallPrototypeQa::Finish()
{
    DrawHist();
    fHM->SaveCanvasToImage(fOutputDir);
    fHM->WriteToFile();
}

ClassImp(CbmRichSmallPrototypeQa)


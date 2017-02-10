/**
 * \file CbmRichGeoTest.cxx
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 **/

#include "CbmRichGeoTest.h"
#include "CbmRichGeoTestStudyReport.h"
#include "FairRootManager.h"
#include "CbmRichHit.h"
#include "CbmRichRing.h"
#include "CbmTrackMatchNew.h"
#include "CbmMCTrack.h"
#include "CbmRichPoint.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "CbmGeoRichPar.h"
#include "FairGeoTransform.h"
#include "FairGeoNode.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "CbmDrawHist.h"
#include "utils/CbmUtils.h"
#include "CbmRichConverter.h"
#include "CbmReport.h"
#include "CbmStudyReport.h"
#include "CbmHistManager.h"
#include "detector/CbmRichGeoManager.h"
#include "CbmRichDigiMapManager.h"
#include "CbmRichDigi.h"
#include "utils/CbmRichDraw.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TCanvas.h"
#include "TEllipse.h"
#include "TClonesArray.h"
#include "TMath.h"
#include "TPad.h"
#include "TLatex.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <string>

#include <boost/assign/list_of.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using boost::assign::list_of;
using boost::property_tree::ptree;

CbmRichGeoTest::CbmRichGeoTest():
FairTask("RichGeoTestQa"),
fOutputDir(""),
fRichHits(NULL),
fRichRings(NULL),
fRichDigis(NULL),
fRichPoints(NULL),
fMCTracks(NULL),
fRichRingMatches(NULL),
fCopFit(NULL),
fTauFit(NULL),
fHM(NULL),
fEventNum(0),
fMinNofHits(0),
fMinAaxis(0.),
fMaxAaxis(0.),
fMinBaxis(0.),
fMaxBaxis(0.),
fMinRadius(0.),
fMaxRadius(0.),
fNofDrawnRings(0)
{
    fEventNum = 0;
    fNofDrawnRings = 0;
    fMinNofHits = 7;
    
    fMinAaxis = 3.;
    fMaxAaxis = 7.;
    fMinBaxis = 3.;
    fMaxBaxis = 7.;
    fMinRadius = 3.;
    fMaxRadius = 7.;
    
}

CbmRichGeoTest::~CbmRichGeoTest()
{
    
}

InitStatus CbmRichGeoTest::Init()
{
    cout << "CbmRichGeoTest::Init"<<endl;
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) { Fatal("CbmRichGeoTest::Init","RootManager not instantised!"); }
    
    fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if ( NULL == fRichHits) { Fatal("CbmRichGeoTest::Init","No RichHit array!"); }
    
    fRichDigis = (TClonesArray*) ioman->GetObject("RichDigi");
    if ( NULL== fRichDigis) { Fatal("CbmRichGeoTest::Init","No RichDigi array!"); }
    
    fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
    if ( NULL == fRichRings) { Fatal("CbmRichGeoTest::Init","No RichRing array!"); }
    
    fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
    if ( NULL == fRichPoints) { Fatal("CbmRichGeoTest::Init","No RichPoint array!"); }
    
    fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
    if ( NULL == fMCTracks) { Fatal("CbmRichGeoTest::Init","No MCTrack array!"); }
    
    fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
    if ( NULL == fRichRingMatches) { Fatal("CbmRichGeoTest::Init","No RichRingMatch array!"); }
    
    fCopFit = new CbmRichRingFitterCOP();
    fTauFit = new CbmRichRingFitterEllipseTau();
    
    InitHistograms();
    
    return kSUCCESS;
}

void CbmRichGeoTest::Exec(
                          Option_t* /*option*/)
{
    fEventNum++;
    cout << "CbmRichGeoTest, event No. " <<  fEventNum << endl;
    FillMcHist();
    RingParameters();
    HitsAndPoints();
}

void CbmRichGeoTest::InitHistograms()
{
//    double xMin = -120.;
//    double xMax = 120.;
//    int nBinsX = 60;
//    double yMin = -210;
//    double yMax = 210.;
//    int nBinsY = 105;

    double xMin = -120.;
    double xMax = 120.;
    int nBinsX = 300;
    double yMin = -210;
    double yMax = 210.;
    int nBinsY = 105;
    
    fHM = new CbmHistManager();
    
    fHM->Create2<TH2D>("fhHitsXY", "fhHitsXY;X [cm];Y [cm];Counter", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
    fHM->Create2<TH2D>("fhPointsXY", "fhPointsXY;X [cm];Y [cm];Counter", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
    fHM->Create1<TH1D>("fhHitsZ", "fhHitsZ;Z [cm];Yield", 1000, 150, 250);
    fHM->Create1<TH1D>("fhPointsZ", "fhPointsZ;Z [cm];Yield", 100, 190, 250);

    
    for (Int_t i = 0; i < 2; i++){
        stringstream ss;
        if (i == 0) ss << "_hits";
        if (i == 1) ss << "_points";
        string t = ss.str();
        if (i == 0) fHM->Create1<TH1D>("fhNofHits"+t, "fhNofHits"+t+";Nof hits in ring;Yield", 50, -.5, 49.5);
        if (i == 1) fHM->Create1<TH1D>("fhNofHits"+t, "fhNofHits"+t+";Nof points in ring;Yield", 300, -.5, 299.5);
        // ellipse fitting parameters
        fHM->Create2<TH2D>("fhBoverAVsMom"+t, "fhBoverAVsMom"+t+";p [GeV/c];B/A;Yield", 40, 0., 10, 100, 0, 1);
        fHM->Create2<TH2D>("fhXcYcEllipse"+t, "fhXcYcEllipse"+t+";X [cm];Y [cm];Yield", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
        fHM->Create2<TH2D>("fhBaxisVsMom"+t, "fhBaxisVsMom"+t+";p [GeV/c];B axis [cm];Yield", 40, 0., 10, 200, 0., 10.);
        fHM->Create2<TH2D>("fhAaxisVsMom"+t, "fhAaxisVsMom"+t+";p [GeV/c];A axis [cm];Yield", 40, 0., 10, 200, 0., 10.);
        fHM->Create2<TH2D>("fhChi2EllipseVsMom"+t, "fhChi2EllipseVsMom"+t+";p [GeV/c];#Chi^{2};Yield", 40, 0., 10., 50, 0., 0.5);
        // circle fitting parameters
        fHM->Create2<TH2D>("fhXcYcCircle"+t, "fhXcYcCircle"+t+";x [cm];y [cm];Yield", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
        fHM->Create2<TH2D>("fhRadiusVsMom"+t, "fhRadiusVsMom"+t+";p [GeV/c];Radius [cm];Yield", 40, 0., 10, 200, 0., 10.);
        fHM->Create2<TH2D>("fhChi2CircleVsMom"+t, "fhChi2CircleVsMom"+t+";p [GeV/c];#Chi^{2};Yield", 40, 0., 10., 50, 0., .5);
        fHM->Create2<TH2D>("fhDRVsMom"+t, "fhDRVsMom"+t+";p [GeV/c];dR [cm];Yield", 40, 0, 10, 200, -2., 2.);

        fHM->Create1<TH1D>("fhBaxisUpHalf"+t, "fhBaxisUpHalf"+t+";B axis [cm];Yield", 200, 0., 10.);
        fHM->Create1<TH1D>("fhBaxisDownHalf"+t, "fhBaxisDownHalf"+t+";B axis [cm];Yield", 200, 0., 10.);
    }
    
    fHM->Create1<TH1D>("fhNofPhotonsPerHit", "fhNofPhotonsPerHit;Number of photons per hit;Yield", 10, -0.5, 9.5);
    
    // Difference between Mc Points and Hits fitting.
    fHM->Create2<TH2D>("fhDiffAaxis", "fhDiffAaxis;Nof hits in ring;A_{point}-A_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
    fHM->Create2<TH2D>("fhDiffBaxis", "fhDiffBaxis;Nof hits in ring;B_{point}-B_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
    fHM->Create2<TH2D>("fhDiffXcEllipse", "fhDiffXcEllipse;Nof hits in ring;Xc_{point}-Xc_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
    fHM->Create2<TH2D>("fhDiffYcEllipse", "fhDiffYcEllipse;Nof hits in ring;Yc_{point}-Yc_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
    fHM->Create2<TH2D>("fhDiffXcCircle", "fhDiffXcCircle;Nof hits in ring;Xc_{point}-Xc_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
    fHM->Create2<TH2D>("fhDiffYcCircle", "fhDiffYcCircle;Nof hits in ring;Yc_{point}-Yc_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
    fHM->Create2<TH2D>("fhDiffRadius", "fhDiffRadius;Nof hits in ring;Radius_{point}-Radius_{hit} [cm];Yield", 40, 0., 40., 100, -1.5, 1.5);
    
    // R, A, B distribution for different number of hits from 0 to 40.
    fHM->Create2<TH2D>("fhRadiusVsNofHits", "fhRadiusVsNofHits;Nof hits in ring;Radius [cm];Yield", 40, 0., 40., 100, 0., 10.);
    fHM->Create2<TH2D>("fhAaxisVsNofHits", "fhAaxisVsNofHits;Nof hits in ring;A axis [cm];Yield", 40, 0., 40., 100, 0., 10.);
    fHM->Create2<TH2D>("fhBaxisVsNofHits", "fhBaxisVsNofHits;Nof hits in ring;B axis [cm];Yield", 40, 0., 40., 100, 0., 10.);
    fHM->Create2<TH2D>("fhDRVsNofHits", "fhDRVsNofHits;Nof hits in ring;dR [cm];Yield", 40, 0., 40., 200, -2., 2.);
    
    // Hits and points.
    fHM->Create1<TH1D>("fhDiffXhit", "fhDiffXhit;X_{point}-X_{hit} [cm];Yield", 200, -.5, .5);
    fHM->Create1<TH1D>("fhDiffYhit", "fhDiffYhit;Y_{point}-Y_{hit} [cm];Yield", 200, -.5, .5);
    
    // Fitting efficiency.
    fHM->Create1<TH1D>("fhNofHitsAll", "fhNofHitsAll;Nof hits in ring;Yield", 50, 0., 50.);
    fHM->Create1<TH1D>("fhNofHitsCircleFit", "fhNofHitsCircleFit;Nof hits in ring;Yield", 50, 0., 50.);
    fHM->Create1<TH1D>("fhNofHitsEllipseFit", "fhNofHitsEllipseFit;Nof hits in ring;Yield", 50, 0., 50.);
    fHM->Create1<TH1D>("fhNofHitsCircleFitEff", "fhNofHitsCircleFitEff;Nof hits in ring;Efficiency [%]", 50, 0., 50.);
    fHM->Create1<TH1D>("fhNofHitsEllipseFitEff", "fhNofHitsEllipseFitEff;Nof hits in ring;Efficiency [%]", 50, 0., 50.);
    
    // Detector acceptance efficiency vs. (pt,y) and p
    fHM->Create1<TH1D>("fhMcMomEl", "fhMcMomEl;p [GeV/c];Yield", 24, 0., 12.);
    fHM->Create2<TH2D>("fhMcPtyEl", "fhMcPtyEl;Rapidity;P_{t} [GeV/c];Yield", 25, 0., 4., 20, 0., 3.);
    fHM->Create1<TH1D>("fhAccMomEl", "fhAccMomEl;p [GeV/c];Yield", 24, 0., 12.);
    fHM->Create2<TH2D>("fhAccPtyEl", "fhAccPtyEl;Rapidity;P_{t} [GeV/c];Yield",25, 0., 4., 20, 0., 3.);
    
    fHM->Create1<TH1D>("fhMcMomPi", "fhMcMomPi;p [GeV/c];Yield", 24, 0., 12.);
    fHM->Create2<TH2D>("fhMcPtyPi", "fhMcPtyPi;Rapidity;P_{t} [GeV/c];Yield", 25, 0., 4., 20, 0., 3.);
    fHM->Create1<TH1D>("fhAccMomPi", "fhAccMomPi;p [GeV/c];Yield", 24, 0., 12.);
    fHM->Create2<TH2D>("fhAccPtyPi", "fhAccPtyPi;Rapidity;P_{t} [GeV/c];Yield", 25, 0., 4., 20, 0., 3.);
    
    // Numbers in dependence on XY position onto the photodetector.
    fHM->Create3<TH3D>("fhNofHitsXYZ", "fhNofHitsXYZ;X [cm];Y [cm];Nof hits in ring", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 50, 0., 50);
    fHM->Create3<TH3D>("fhNofPointsXYZ", "fhNofPointsXYZ;X [cm];Y [cm];Nof points in ring", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 50, 100., 300.);
    fHM->Create3<TH3D>("fhBoverAXYZ", "fhBoverAXYZ;X [cm];Y [cm];B/A", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 100, 0., 1.);
    fHM->Create3<TH3D>("fhBaxisXYZ", "fhBaxisXYZ;X [cm];Y [cm];B axis [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 80, 3., 7.);
    fHM->Create3<TH3D>("fhAaxisXYZ", "fhAaxisXYZ;X [cm];Y [cm];A axis [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 80, 3., 7.);
    fHM->Create3<TH3D>("fhRadiusXYZ", "fhRadiusXYZ;X [cm];Y [cm];Radius [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 80, 3., 7.);
    fHM->Create3<TH3D>("fhdRXYZ", "fhdRXYZ;X [cm];Y [cm];dR [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 100, -1., 1.);

    int nBinsX1 = 60;
    int xMin1 = -120;
    int xMax1 = 120;
    int nBinsY1 = 25;
    int yMin1 = 100;
    int yMax1 = 200;
    // Numbers in dependence X or Y position onto the photodetector plane
    fHM->Create2<TH2D>("fhNofHitsVsX", "fhNofHitsVsX;X [cm];Nof hits in ring", nBinsX1, xMin1, xMax1, 50, 0., 50);
    fHM->Create2<TH2D>("fhNofHitsVsY", "fhNofHitsVsY;Abs(Y) [cm];Nof hits in ring", nBinsY1, yMin1, yMax1, 50, 0., 50);

    fHM->Create2<TH2D>("fhNofPointsVsX", "fhNofPointsVsX;X [cm];Nof points in ring", nBinsX1, xMin1, xMax1, 50, 100., 300.);
    fHM->Create2<TH2D>("fhNofPointsVsY", "fhNofPointsVsY;Abs(Y) [cm];Nof points in ring", nBinsY1, yMin1, yMax1, 50, 100., 300.);

    fHM->Create2<TH2D>("fhBoverAVsX", "fhBoverAVsX;X [cm];B/A", nBinsX1, xMin1, xMax1, 100, 0., 1.);
    fHM->Create2<TH2D>("fhBoverAVsY", "fhBoverAVsY;Abs(Y) [cm];B/A", nBinsY1, yMin1, yMax1, 100, 0., 1.);

    fHM->Create2<TH2D>("fhBaxisVsX", "fhBaxisVsX;X [cm];B axis [cm]", nBinsX1, xMin1, xMax1, 80, 3., 7.);
    fHM->Create2<TH2D>("fhBaxisVsY", "fhBaxisVsY;Abs(Y) [cm];B axis [cm]", nBinsY1, yMin1, yMax1, 80, 3., 7.);

    fHM->Create2<TH2D>("fhAaxisVsX", "fhAaxisVsX;X [cm];A axis [cm]", nBinsX1, xMin1, xMax1, 80, 3., 7.);
    fHM->Create2<TH2D>("fhAaxisVsY", "fhAaxisVsY;Abs(Y) [cm];A axis [cm]", nBinsY1, yMin1, yMax1, 80, 3., 7.);

    fHM->Create2<TH2D>("fhRadiusVsX", "fhRadiusVsX;X [cm];Radius [cm]", nBinsX1, xMin1, xMax1, 80, 3., 7.);
    fHM->Create2<TH2D>("fhRadiusVsY", "fhRadiusVsY;Abs(Y) [cm];Radius [cm]", nBinsY1, yMin1, yMax1, 80, 3., 7.);

    fHM->Create2<TH2D>("fhdRVsX", "fhdRVsX;X [cm];dR [cm]", nBinsX1, xMin1, xMax1, 100, -1., 1.);
    fHM->Create2<TH2D>("fhdRVsY", "fhdRVsY;Abs(Y) [cm];dR [cm]", nBinsY1, yMin1, yMax1, 100, -1., 1.);
}

void CbmRichGeoTest::FillMcHist()
{
    for (Int_t i = 0; i < fMCTracks->GetEntriesFast(); i++){
        CbmMCTrack* mcTrack = (CbmMCTrack*)fMCTracks->At(i);
        if (!mcTrack) continue;
        Int_t motherId = mcTrack->GetMotherId();
        Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
        
        if (pdg == 11 && motherId == -1){
            fHM->H1("fhMcMomEl")->Fill(mcTrack->GetP());
            fHM->H2("fhMcPtyEl")->Fill(mcTrack->GetRapidity(), mcTrack->GetPt());
        }
        
        if (pdg == 211 && motherId == -1){
            fHM->H1("fhMcMomPi")->Fill(mcTrack->GetP());
            fHM->H2("fhMcPtyPi")->Fill(mcTrack->GetRapidity(), mcTrack->GetPt());
        }
    }
}

void CbmRichGeoTest::RingParameters()
{
    Int_t nofRings = fRichRings->GetEntriesFast();
    for (Int_t iR = 0; iR < nofRings; iR++){
        CbmRichRing *ring = (CbmRichRing*) fRichRings->At(iR);
        if (NULL == ring) continue;
        CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
        if (NULL == ringMatch) continue;
        
        Int_t mcTrackId = ringMatch->GetMatchedLink().GetIndex();
        if (mcTrackId < 0) continue;
        CbmMCTrack* mcTrack = (CbmMCTrack*)fMCTracks->At(mcTrackId);
        if (!mcTrack) continue;
        Int_t motherId = mcTrack->GetMotherId();
        Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
        Double_t momentum = mcTrack->GetP();
        Double_t pt = mcTrack->GetPt();
        Double_t rapidity = mcTrack->GetRapidity();
        
        
        if (ring->GetNofHits() >= fMinNofHits){
            if (pdg == 11 && motherId == -1){
                fHM->H1("fhAccMomEl")->Fill(momentum);
                fHM->H2("fhAccPtyEl")->Fill(rapidity, pt);
            }
            if (pdg == 211 && motherId == -1){
                fHM->H1("fhAccMomPi")->Fill(momentum);
                fHM->H2("fhAccPtyPi")->Fill(rapidity, pt);
            }
        }
        
        if (pdg != 11 || motherId != -1) continue; // only primary electrons
        
        CbmRichRingLight ringPoint;
        int nofRichPoints = fRichPoints->GetEntriesFast();
        for (int iPoint = 0; iPoint < nofRichPoints; iPoint++){
            CbmRichPoint* richPoint = (CbmRichPoint*) fRichPoints->At(iPoint);
            if (NULL == richPoint) continue;
            Int_t trackId = richPoint->GetTrackID();
            if (trackId < 0) continue;
            CbmMCTrack* mcTrackRich = (CbmMCTrack*)fMCTracks->At(trackId);
            if (NULL == mcTrackRich) continue;
            int motherIdRich = mcTrackRich->GetMotherId();
            if (motherIdRich == mcTrackId){
                TVector3 posPoint;
                richPoint->Position(posPoint);
                TVector3 detPoint;
                CbmRichGeoManager::GetInstance().RotatePoint(&posPoint, &detPoint);
                CbmRichHitLight hit(detPoint.X(), detPoint.Y());
                ringPoint.AddHit(hit);
            }
        }
        fHM->H1("fhNofHitsAll")->Fill(ring->GetNofHits());
        
        CbmRichRingLight ringHit;
        CbmRichConverter::CopyHitsToRingLight(ring, &ringHit);
        
        FitAndFillHistCircle(0, &ringHit, momentum); //hits
        FitAndFillHistCircle(1, &ringPoint, momentum); // points
        FillMcVsHitFitCircle(&ringHit, &ringPoint);
        
        double r = ringHit.GetRadius();
        double xc = ringHit.GetCenterX();
        double yc = ringHit.GetCenterY();
        
        if (ringHit.GetRadius() > fMinRadius && ringHit.GetRadius() < fMaxRadius){
            fHM->H1("fhNofHitsCircleFit")->Fill(ringHit.GetNofHits());
        }
        if (fNofDrawnRings < 10 && ringHit.GetNofHits() <= 500){
            DrawRing(&ringHit, &ringPoint);
        }
        
        FitAndFillHistEllipse(0, &ringHit, momentum);// hits
        FitAndFillHistEllipse(1, &ringPoint, momentum); // points
        FillMcVsHitFitEllipse(&ringHit, &ringPoint);
        
        if (ringHit.GetAaxis() > fMinAaxis && ringHit.GetAaxis() < fMaxAaxis
            &&  ringHit.GetBaxis() > fMinBaxis && ringHit.GetAaxis() < fMaxBaxis ){
            fHM->H1("fhNofHitsEllipseFit")->Fill(ringHit.GetNofHits());
            
            double np = ringPoint.GetNofHits();
            double a = ringHit.GetAaxis();
            double b = ringHit.GetBaxis();
            double nh = ring->GetNofHits();
            
            fHM->H3("fhNofHitsXYZ")->Fill(xc, yc, nh);
            fHM->H3("fhNofPointsXYZ")->Fill(xc, yc, np);
            fHM->H3("fhBoverAXYZ")->Fill(xc, yc, b/a);
            fHM->H3("fhBaxisXYZ")->Fill(xc, yc, b);
            fHM->H3("fhAaxisXYZ")->Fill(xc, yc, a);
            fHM->H3("fhRadiusXYZ")->Fill(xc, yc, r);
            
            fHM->H2("fhNofHitsVsX")->Fill(xc, nh);
            fHM->H2("fhNofPointsVsX")->Fill(xc, np);
            fHM->H2("fhBoverAVsX")->Fill(xc, b/a);
            fHM->H2("fhBaxisVsX")->Fill(xc, b);
            fHM->H2("fhAaxisVsX")->Fill(xc, a);
            fHM->H2("fhRadiusVsX")->Fill(xc, r);

            fHM->H2("fhNofHitsVsY")->Fill(abs(yc), nh);
            fHM->H2("fhNofPointsVsY")->Fill(abs(yc), np);
            fHM->H2("fhBoverAVsY")->Fill(abs(yc), b/a);
            fHM->H2("fhBaxisVsY")->Fill(abs(yc), b);
            fHM->H2("fhAaxisVsY")->Fill(abs(yc), a);
            fHM->H2("fhRadiusVsY")->Fill(abs(yc), r);

            for (int iH = 0; iH < ringHit.GetNofHits(); iH++){
                double xh = ringHit.GetHit(iH).fX;
                double yh = ringHit.GetHit(iH).fY;
                double dr = r - sqrt((xc - xh)*(xc - xh) + (yc - yh)*(yc - yh));
                fHM->H3("fhdRXYZ")->Fill(xc, yc, dr);
                fHM->H2("fhdRVsX")->Fill(xc, dr);
                fHM->H2("fhdRVsY")->Fill(abs(yc), dr);
            }
        }
    }// iR
}

void CbmRichGeoTest::FitAndFillHistEllipse(
                                           int histIndex,
                                           CbmRichRingLight* ring,
                                           double momentum)
{
    fTauFit->DoFit(ring);
    double axisA = ring->GetAaxis();
    double axisB = ring->GetBaxis();
    double xcEllipse = ring->GetCenterX();
    double ycEllipse = ring->GetCenterY();
    int nofHitsRing = ring->GetNofHits();
    string t = "";
    if (histIndex == 0){
        t = "_hits";
    } else if (histIndex == 1) {
        t = "_points";
    }
    if (axisA > fMinAaxis && axisA < fMaxAaxis &&  axisB > fMinBaxis && axisB < fMaxBaxis ){
        fHM->H1("fhBoverAVsMom"+t)->Fill(momentum, axisB/axisA);
        fHM->H2("fhXcYcEllipse"+t)->Fill(xcEllipse, ycEllipse);
    }
    fHM->H1("fhNofHits"+t)->Fill(nofHitsRing);

    if (ycEllipse > 149 || ycEllipse < -149) {
    	fHM->H1("fhBaxisUpHalf"+t)->Fill(axisB);
    } else {
    	fHM->H1("fhBaxisDownHalf"+t)->Fill(axisB);
    }

    fHM->H2("fhBaxisVsMom"+t)->Fill(momentum, axisB);
    fHM->H2("fhAaxisVsMom"+t)->Fill(momentum, axisA);
    fHM->H2("fhChi2EllipseVsMom"+t)->Fill(momentum, ring->GetChi2()/ring->GetNofHits());
    if (histIndex == 0){ // only hit fit
        fHM->H2("fhAaxisVsNofHits")->Fill(nofHitsRing, axisA);
        fHM->H2("fhBaxisVsNofHits")->Fill(nofHitsRing, axisB);
    }
}

void CbmRichGeoTest::FitAndFillHistCircle(
                                          int histIndex,
                                          CbmRichRingLight* ring,
                                          double momentum)
{
    fCopFit->DoFit(ring);
    double radius = ring->GetRadius();
    double xcCircle = ring->GetCenterX();
    double ycCircle = ring->GetCenterY();
    int nofHitsRing = ring->GetNofHits();
    string t = "";
    if (histIndex == 0){
        t = "_hits";
    } else if (histIndex == 1) {
        t = "_points";
    }
    fHM->H1("fhXcYcCircle"+t)->Fill(xcCircle, ycCircle);
    fHM->H1("fhRadiusVsMom"+t)->Fill(momentum, radius);
    fHM->H1("fhChi2CircleVsMom"+t)->Fill(momentum, ring->GetChi2()/ring->GetNofHits());
    
    for (int iH = 0; iH < nofHitsRing; iH++){
        double xh = ring->GetHit(iH).fX;
        double yh = ring->GetHit(iH).fY;
        double dr = radius - sqrt((xcCircle - xh)*(xcCircle - xh) + (ycCircle - yh)*(ycCircle - yh));
        fHM->H1("fhDRVsMom"+t)->Fill(momentum, dr);
        
        if (histIndex == 0) {// only hit fit
           fHM->H2("fhDRVsNofHits")->Fill(nofHitsRing, dr);
        }
    }
    
    if (histIndex == 0){ // only hit fit
        fHM->H2("fhRadiusVsNofHits")->Fill(nofHitsRing, radius);
    }
}

void CbmRichGeoTest::FillMcVsHitFitEllipse(
                                           CbmRichRingLight* ring,
                                           CbmRichRingLight* ringMc)
{
    fHM->H2("fhDiffAaxis")->Fill(ring->GetNofHits(), ringMc->GetAaxis() - ring->GetAaxis());
    fHM->H2("fhDiffBaxis")->Fill(ring->GetNofHits(),ringMc->GetBaxis() - ring->GetBaxis());
    fHM->H2("fhDiffXcEllipse")->Fill(ring->GetNofHits(),ringMc->GetCenterX() - ring->GetCenterX());
    fHM->H2("fhDiffYcEllipse")->Fill(ring->GetNofHits(),ringMc->GetCenterY() - ring->GetCenterY());
}

void CbmRichGeoTest::FillMcVsHitFitCircle(
                                          CbmRichRingLight* ring,
                                          CbmRichRingLight* ringMc)
{
    fHM->H2("fhDiffXcCircle")->Fill(ring->GetNofHits(),ringMc->GetCenterX() - ring->GetCenterX());
    fHM->H2("fhDiffYcCircle")->Fill(ring->GetNofHits(),ringMc->GetCenterY() - ring->GetCenterY());
    fHM->H2("fhDiffRadius")->Fill(ring->GetNofHits(),ringMc->GetRadius() - ring->GetRadius());
}

void CbmRichGeoTest::HitsAndPoints()
{
    Int_t nofHits = fRichHits->GetEntriesFast();
    for (Int_t iH = 0; iH < nofHits; iH++){
        CbmRichHit *hit = static_cast<CbmRichHit*>(fRichHits->At(iH));
        if ( hit == NULL ) continue;
        
        Int_t digiIndex = hit->GetRefId();
        if (digiIndex < 0) continue;
        CbmRichDigi* digi = (CbmRichDigi*) fRichDigis->At(digiIndex);
        if (NULL == digi) continue;
        CbmMatch* digiMatch = digi->GetMatch();
        
        vector<CbmLink> links = digiMatch->GetLinks();
        for (UInt_t i = 0; i < links.size(); i++) {
            Int_t pointId = links[i].GetIndex();
            if (pointId < 0) continue; // noise hit
            
            CbmRichPoint* pMCpt = (CbmRichPoint*) (fRichPoints->At(pointId));
            if ( NULL == pMCpt ) continue;
            
            TVector3 inPos(pMCpt->GetX(), pMCpt->GetY(), pMCpt->GetZ());
            TVector3 outPos;
            CbmRichGeoManager::GetInstance().RotatePoint(&inPos, &outPos);
            fHM->H1("fhDiffXhit")->Fill(hit->GetX() - outPos.X());
            fHM->H1("fhDiffYhit")->Fill(hit->GetY() - outPos.Y());
            
        }
        
        //fHM->H1("fhNofPhotonsPerHit")->Fill(hit->GetNPhotons());
        fHM->H2("fhHitsXY")->Fill(hit->GetX(), hit->GetY());
        fHM->H1("fhHitsZ")->Fill(hit->GetZ());
    }
    
    Int_t nofPoints = fRichPoints->GetEntriesFast();
    for (Int_t iP = 0; iP < nofPoints; iP++){
        CbmRichPoint *point = (CbmRichPoint*) fRichPoints->At(iP);
        if ( point == NULL ) continue;
        TVector3 inPos(point->GetX(), point->GetY(), point->GetZ());
        TVector3 outPos;
        CbmRichGeoManager::GetInstance().RotatePoint(&inPos, &outPos);
      //  fHM->H2("fhPointsXY")->Fill(inPos.X(), inPos.Y());
        fHM->H2("fhPointsXY")->Fill(outPos.X(), outPos.Y());
        fHM->H1("fhPointsZ")->Fill(point->GetZ());
    }
}

void CbmRichGeoTest::DrawRing(
                              CbmRichRingLight* ringHit,
                              CbmRichRingLight* ringPoint)
{
    stringstream ss;
    ss << "Event" << fNofDrawnRings;
    fNofDrawnRings++;
    TCanvas *c = fHM->CreateCanvas(ss.str().c_str(), ss.str().c_str(), 500, 500);
    c->SetGrid(true, true);
    TH2D* pad = new TH2D("pad", ";X [cm];Y [cm]", 1, -15., 15., 1, -15., 15);
    pad->SetStats(false);
    pad->Draw();
    
    // find min and max x and y positions of the hits
    // in order to shift drawing
    double xmin = 99999., xmax = -99999., ymin = 99999., ymax = -99999.;
    for (int i = 0; i < ringHit->GetNofHits(); i++){
        double hitX = ringHit->GetHit(i).fX;
        double hitY = ringHit->GetHit(i).fY;
        if (xmin > hitX) xmin = hitX;
        if (xmax < hitX) xmax = hitX;
        if (ymin > hitY) ymin = hitY;
        if (ymax < hitY) ymax = hitY;
    }
    double xCur = (xmin + xmax) / 2.;
    double yCur = (ymin + ymax) / 2.;
    
    //Draw circle and center
    TEllipse* circle = new TEllipse(ringHit->GetCenterX() - xCur,
                                    ringHit->GetCenterY() - yCur, ringHit->GetRadius());
    circle->SetFillStyle(0);
    circle->SetLineWidth(3);
    circle->Draw();
    TEllipse* center = new TEllipse(ringHit->GetCenterX() - xCur, ringHit->GetCenterY() - yCur, .5);
    center->Draw();
    
    // Draw hits
    for (int i = 0; i < ringHit->GetNofHits(); i++){
        TEllipse* hitDr = new TEllipse(ringHit->GetHit(i).fX - xCur, ringHit->GetHit(i).fY - yCur, .5);
        hitDr->SetFillColor(kRed);
        hitDr->Draw();
    }
    
    // Draw MC Points
    for (int i = 0; i < ringPoint->GetNofHits(); i++){
        TEllipse* pointDr = new TEllipse(ringPoint->GetHit(i).fX - xCur, ringPoint->GetHit(i).fY - yCur, 0.15);
        pointDr->SetFillColor(kBlue);
        pointDr->Draw();
    }
    
    //Draw information
    stringstream ss2;
    ss2 << "(r, n)=(" << setprecision(3) << ringHit->GetRadius() << ", " << ringHit->GetNofHits()<<")";
    TLatex* latex = new TLatex(-8., 8., ss2.str().c_str());
    latex->Draw();
}

TH1D* CbmRichGeoTest::CreateAccVsMinNofHitsHist()
{
    Int_t nofMc = (Int_t)fHM->H1("fhMcMomEl")->GetEntries();
    TH1D* hist = (TH1D*)fHM->H1("fhNofHits_hits")->Clone("fhAccVsMinNofHitsHist");
    hist->GetXaxis()->SetTitle("Required min nof hits in ring");
    hist->GetYaxis()->SetTitle("Detector acceptance [%]");
    Double_t sum = 0.;
    for (int i = hist->GetNbinsX(); i > 1 ; i--){
        sum += fHM->H1("fhNofHits_hits")->GetBinContent(i);
        hist->SetBinContent(i, 100. * sum / nofMc);
    }
    return hist;
}

void CbmRichGeoTest::DrawH2MeanRms(
                                   TH2* hist,
                                   const string& canvasName)
{
    TCanvas *c = fHM->CreateCanvas(canvasName.c_str(), canvasName.c_str(), 1200, 600);
    c->Divide(2, 1);
    c->cd(1);
    DrawH2WithProfile(hist);
    c->cd(2);
    TH1D* py = (TH1D*)hist->ProjectionY( (string(hist->GetName())+ "_py" ).c_str() )->Clone();
    DrawH1andFitGauss(py);
    py->Scale(1./py->Integral());
    py->GetYaxis()->SetTitle("Yield");
}

void CbmRichGeoTest::DrawHist()
{
    SetDefaultDrawStyle();
    
    {
        TCanvas *c = fHM->CreateCanvas("richgeo_hits_xy", "richgeo_hits_xy", 1200, 1200);
        CbmRichDraw::DrawPmtH2(fHM->H2("fhHitsXY"), c);
    }
    
    {
        TCanvas *c = fHM->CreateCanvas("richgeo_points_xy", "richgeo_points_xy", 1200, 1200);
        CbmRichDraw::DrawPmtH2(fHM->H2("fhPointsXY"), c);
    }
    
    {
        fHM->CreateCanvas("richgeo_hits_z", "richgeo_hits_z", 800, 800);
        fHM->H1("fhHitsZ")->Scale(1./fHM->H1("fhHitsZ")->Integral());
        DrawH1(fHM->H1("fhHitsZ"));
    }
    
    {
        fHM->CreateCanvas("richgeo_points_z", "richgeo_points_z", 800, 800);
        fHM->H1("fhPointsZ")->Scale(1./fHM->H1("fhPointsZ")->Integral());
        DrawH1(fHM->H1("fhPointsZ"));
    }
    
    for (int i = 0; i < 2; i++) {
        string t;
        if (i == 0) {
            t = "_hits";
        } else if (i == 1) {
            t = "_points";
        }
        
        DrawH2MeanRms((TH2D*)fHM->H2("fhBoverAVsMom"+t), "richgeo" + t + "_ellipse_boa_vs_mom");
        fHM->H2("fhBoverAVsMom"+t)->GetYaxis()->SetRangeUser(0.8, 1.0);
        
        {
			fHM->CreateCanvas(("richgeo" + t +"_ellipse_xc_yc").c_str(), ("richgeo" + t + "_ellipse_xc_yc").c_str(), 800, 800);
			DrawH2(fHM->H2("fhXcYcEllipse"+t));
        }
        
        DrawH2MeanRms((TH2D*)fHM->H2("fhChi2EllipseVsMom"+t),  "richgeo" + t + "_chi2_ellipse_vs_mom");
        DrawH2MeanRms((TH2D*)fHM->H2("fhAaxisVsMom"+t), "richgeo" + t + "_a_vs_mom");
        DrawH2MeanRms((TH2D*)fHM->H2("fhBaxisVsMom"+t), "richgeo" + t + "_b_vs_mom");
        
        {
        	TCanvas *c = fHM->CreateCanvas(("richgeo" + t + "_b_up_down_halves").c_str(), ("richgeo" + t + "_b_up_down_halves").c_str(), 1200, 600);
			c->Divide(2,1);
			c->cd(1);
			DrawH1andFitGauss((TH1D*)fHM->H1("fhBaxisUpHalf"+t)->Clone(), true, true, 3., 6.);
			c->cd(2);
			DrawH1andFitGauss((TH1D*)fHM->H1("fhBaxisDownHalf"+t)->Clone(), true, true, 3., 6.);
        }

        {
			TCanvas *c = fHM->CreateCanvas(("richgeo" + t + "_circle").c_str(), ("richgeo" + t + "_circle").c_str(), 1200, 600);
			c->Divide(2,1);
			c->cd(1);
			DrawH1andFitGauss((TH1D*)fHM->H1("fhNofHits"+t)->Clone());
			cout << "Number of hits/points = "  << fHM->H1("fhNofHits"+t)->GetMean() << endl;
			//gPad->SetLogy(true);
			c->cd(2);
			DrawH2(fHM->H2("fhXcYcCircle"+t));
        }
        
        DrawH2MeanRms((TH2D*)fHM->H2("fhChi2CircleVsMom"+t), "richgeo" + t + "_chi2_circle_vs_mom");
        DrawH2MeanRms((TH2D*)fHM->H2("fhRadiusVsMom"+t), "richgeo" + t + "_r_vs_mom");
        DrawH2MeanRms((TH2D*)fHM->H2("fhDRVsMom"+t), "richgeo" + t + "_dr_vs_mom");
        fHM->H2("fhDRVsMom"+t)->GetYaxis()->SetRangeUser(-1.05, 1.05);
    }// for loop
    
    {
		fHM->CreateCanvas("richgeo_nof_photons_per_hit", "richgeo_nof_photons_per_hit", 800, 800);
		fHM->H1("fhNofPhotonsPerHit")->Scale(1./fHM->H1("fhNofPhotonsPerHit")->Integral());
		DrawH1(fHM->H1("fhNofPhotonsPerHit"));
    }
    
    {
		TCanvas *c = fHM->CreateCanvas("richgeo_diff_ellipse", "richgeo_diff_ellipse", 1200, 600);
		c->Divide(4,2);
		c->cd(1);
		DrawH2(fHM->H2("fhDiffAaxis"));
		c->cd(2);
		DrawH2(fHM->H2("fhDiffBaxis"));
		c->cd(3);
		DrawH2(fHM->H2("fhDiffXcEllipse"));
		c->cd(4);
		DrawH2(fHM->H2("fhDiffYcEllipse"));
		c->cd(5);
		DrawH1(fHM->H2("fhDiffAaxis")->ProjectionY());
		gPad->SetLogy(true);
		c->cd(6);
		DrawH1(fHM->H2("fhDiffBaxis")->ProjectionY());
		gPad->SetLogy(true);
		c->cd(7);
		DrawH1(fHM->H2("fhDiffXcEllipse")->ProjectionY());
		gPad->SetLogy(true);
		c->cd(8);
		DrawH1(fHM->H2("fhDiffYcEllipse")->ProjectionY());
		gPad->SetLogy(true);
    }
    
    {
		TCanvas *c = fHM->CreateCanvas("richgeo_diff_circle", "richgeo_diff_circle", 900, 600);
		c->Divide(3,2);
		c->cd(1);
		DrawH2(fHM->H2("fhDiffXcCircle"));
		c->cd(2);
		DrawH2(fHM->H2("fhDiffYcCircle"));
		c->cd(3);
		DrawH2(fHM->H2("fhDiffRadius"));
		c->cd(4);
		DrawH1(fHM->H2("fhDiffXcCircle")->ProjectionY());
		gPad->SetLogy(true);
		c->cd(5);
		DrawH1(fHM->H2("fhDiffYcCircle")->ProjectionY());
		gPad->SetLogy(true);
		c->cd(6);
		DrawH1(fHM->H2("fhDiffRadius")->ProjectionY());
		gPad->SetLogy(true);
    }
    
    {
		TCanvas *c = fHM->CreateCanvas("richgeo_hits_residual", "richgeo_hits", 1200, 600);
		c->Divide(2,1);
		c->cd(1);
		fHM->H1("fhDiffXhit")->Scale(1./fHM->H1("fhDiffXhit")->Integral());
		DrawH1(fHM->H1("fhDiffXhit"));
		c->cd(2);
		fHM->H1("fhDiffYhit")->Scale(1./fHM->H1("fhDiffYhit")->Integral());
		DrawH1(fHM->H1("fhDiffYhit"));
    }
    
    {
		TCanvas *c = fHM->CreateCanvas("richgeo_fit_eff", "richgeo_fit_eff", 1800, 600);
		c->Divide(3,1);
		c->cd(1);
		DrawH1( list_of((TH1D*)fHM->H1("fhNofHitsAll")->Clone())((TH1D*)fHM->H1("fhNofHitsCircleFit")->Clone())((TH1D*)fHM->H1("fhNofHitsEllipseFit")->Clone()),
			   list_of("All")("Circle fit")("Ellipse fit"), kLinear, kLog, true, 0.7, 0.7, 0.99, 0.99);
		TH1D* fhNofHitsCircleFitEff = Cbm::DivideH1(fHM->H1("fhNofHitsCircleFit"), fHM->H1("fhNofHitsAll"));
		TH1D* fhNofHitsEllipseFitEff = Cbm::DivideH1(fHM->H1("fhNofHitsEllipseFit"), fHM->H1("fhNofHitsAll"));
		c->cd(2);
		DrawH1(fhNofHitsCircleFitEff);
		TLatex* circleFitEffTxt = new TLatex(15, 0.5, CalcEfficiency(fHM->H1("fhNofHitsCircleFit"), fHM->H1("fhNofHitsAll")).c_str());
		circleFitEffTxt->Draw();
		c->cd(3);
		DrawH1(fhNofHitsEllipseFitEff);
		TLatex* ellipseFitEffTxt = new TLatex(15, 0.5, CalcEfficiency(fHM->H1("fhNofHitsEllipseFit"), fHM->H1("fhNofHitsAll")).c_str());
		ellipseFitEffTxt->Draw();
    }
    
    {
		TCanvas *c = fHM->CreateCanvas("richgeo_acc_eff_el", "richgeo_acc_eff_el", 1200, 800);
		c->Divide(3,2);
		c->cd(1);
		DrawH1(list_of((TH1D*)fHM->H1("fhMcMomEl"))((TH1D*)fHM->H1("fhAccMomEl")), list_of("MC")("ACC"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
		c->cd(2);
		DrawH2(fHM->H2("fhMcPtyEl"));
		c->cd(3);
		DrawH2(fHM->H2("fhAccPtyEl"));
    }
    
    TH1D* pxEff = Cbm::DivideH1((TH1D*)fHM->H1("fhAccMomEl")->Clone(), (TH1D*)fHM->H1("fhMcMomEl")->Clone(), "", 100., "Geometrical acceptance [%]");
    TH2D* pyzEff = Cbm::DivideH2((TH2D*)fHM->H1("fhAccPtyEl")->Clone(), (TH2D*)fHM->H1("fhMcPtyEl")->Clone(), "", 100., "Geometrical acceptance [%]");
    {
		fHM->CreateCanvas("richgeo_acc_eff_el_mom", "richgeo_acc_eff_el_mom", 800, 800);
		DrawH1(pxEff);
    }

    {
		fHM->CreateCanvas("richgeo_acc_eff_el_pty", "richgeo_acc_eff_el_pty", 800, 800);
		DrawH2(pyzEff);
    }
    
    {
		TCanvas *c = fHM->CreateCanvas("richgeo_acc_eff_pi", "richgeo_acc_eff_pi", 1200, 800);
		c->Divide(3,2);
		c->cd(1);
		DrawH1(list_of((TH1D*)fHM->H1("fhMcMomPi"))((TH1D*)fHM->H1("fhAccMomPi")), list_of("MC")("ACC"), kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
		c->cd(2);
		DrawH2(fHM->H2("fhMcPtyPi"));
		c->cd(3);
		DrawH2(fHM->H2("fhAccPtyPi"));
    }

	TH1D* pxPiEff = Cbm::DivideH1((TH1D*)(TH1D*)fHM->H1("fhAccMomPi")->Clone(), (TH1D*)(TH1D*)fHM->H1("fhMcMomPi")->Clone(), "", 100., "Geometrical acceptance [%]");
	TH2D* pyzPiEff = Cbm::DivideH2((TH2D*)(TH1D*)fHM->H1("fhAccPtyPi")->Clone(), (TH2D*)(TH1D*)fHM->H1("fhMcPtyPi")->Clone(), "", 100., "Geometrical acceptance [%]");
	{
		fHM->CreateCanvas("richgeo_acc_eff_pi_mom", "richgeo_acc_eff_pi_mom", 800, 800);
		DrawH1(pxPiEff);
    }

    {
		fHM->CreateCanvas("richgeo_acc_eff_pi_pty", "richgeo_acc_eff_pi_pty", 800, 800);
		DrawH2(pyzPiEff);
		pyzPiEff->GetZaxis()->SetRangeUser(0, 100);
    }
    
    {
		string effPi = CalcEfficiency((TH1D*)fHM->H1("fhAccMomPi")->Clone(), (TH1D*)fHM->H1("fhMcMomPi")->Clone());
		string effEl = CalcEfficiency((TH1D*)fHM->H1("fhAccMomEl")->Clone(), (TH1D*)fHM->H1("fhMcMomEl")->Clone());
		fHM->CreateCanvas("richgeo_acc_eff_el_pi_mom", "richgeo_acc_eff_el_pi_mom", 800, 800);
		DrawH1({pxEff, pxPiEff}, {"e^{#pm} ("+effEl+"%)", "#pi^{#pm} ("+effPi+"%)"}, kLinear, kLinear, true, 0.7, 0.5, 0.87, 0.7);
    }
    
    {
		TCanvas *c = fHM->CreateCanvas("richgeo_acc_eff_el_zoom", "richgeo_acc_eff_el_zoom", 1000, 500);
		c->Divide(2,1);
		c->cd(1);
		TH1D* fhMcMomElClone = (TH1D*)fHM->H1("fhMcMomEl")->Clone();
		TH1D* fhAccMomElClone = (TH1D*)fHM->H1("fhAccMomEl")->Clone();
		fhMcMomElClone->GetXaxis()->SetRangeUser(0., 3.);
		fhAccMomElClone->GetXaxis()->SetRangeUser(0., 3.);
		fhMcMomElClone->SetMinimum(0.);
		DrawH1({fhMcMomElClone, fhAccMomElClone}, {"MC", "ACC"}, kLinear, kLog, true, 0.8, 0.8, 0.99, 0.99);
		gPad->SetLogy(false);
		c->cd(2);
		TH1D* px_eff_clone = (TH1D*) pxEff->Clone();
		px_eff_clone->GetXaxis()->SetRangeUser(0., 3.);
		px_eff_clone->SetMinimum(0.);
		DrawH1(px_eff_clone);
    }
    
    // Draw number vs position onto the photodetector plane
    {
    	TCanvas* c = fHM->CreateCanvas("richgeo_numbers_vs_xy_hits", "richgeo_numbers_vs_xy_hits", 1800, 600);
    	c->Divide(3, 1);
    	c->cd(1);
    	DrawH3Profile(fHM->H3("fhNofHitsXYZ"), true, false, 10, 30);
    	c->cd(2);
    	DrawH2WithProfile(fHM->H2("fhNofHitsVsX"), false, true);
    	c->cd(3);
    	DrawH2WithProfile(fHM->H2("fhNofHitsVsY"), false, true);
    }

    {
    	TCanvas* c = fHM->CreateCanvas("richgeo_numbers_vs_xy_points", "richgeo_numbers_vs_xy_points", 1800, 600);
    	c->Divide(3, 1);
    	c->cd(1);
    	DrawH3Profile(fHM->H3("fhNofPointsXYZ"), true, false, 100., 300.);
    	c->cd(2);
    	DrawH2WithProfile(fHM->H2("fhNofPointsVsX"), false, true);
       	c->cd(3);
       	DrawH2WithProfile(fHM->H2("fhNofPointsVsY"), false, true);
    }

    {
    	TCanvas* c = fHM->CreateCanvas("richgeo_numbers_vs_xy_boa", "richgeo_numbers_vs_xy_boa", 1800, 600);
    	c->Divide(3, 1);
    	c->cd(1);
    	DrawH3Profile(fHM->H3("fhBoverAXYZ"), true, false, 0.75, 1.0);
    	c->cd(2);
    	DrawH2WithProfile(fHM->H2("fhBoverAVsX"), false, true);
    	fHM->H2("fhBoverAVsX")->GetYaxis()->SetRangeUser(0.75, 1.0);
    	c->cd(3);
    	DrawH2WithProfile(fHM->H2("fhBoverAVsY"), false, true);
    	fHM->H2("fhBoverAVsY")->GetYaxis()->SetRangeUser(0.75, 1.0);
    }

    {
    	TCanvas* c = fHM->CreateCanvas("richgeo_numbers_vs_xy_b", "richgeo_numbers_vs_xy_b", 1800, 600);
    	c->Divide(3, 1);
    	c->cd(1);
    	DrawH3Profile(fHM->H3("fhBaxisXYZ"), true, false, 4., 5.);
    	c->cd(2);
    	DrawH2WithProfile(fHM->H2("fhBaxisVsX"), false, true);
    	c->cd(3);
    	DrawH2WithProfile(fHM->H2("fhBaxisVsY"), false, true);
    }

    {
    	TCanvas* c = fHM->CreateCanvas("richgeo_numbers_vs_xy_a", "richgeo_numbers_vs_xy_a", 1800, 600);
    	c->Divide(3, 1);
    	c->cd(1);
    	DrawH3Profile(fHM->H3("fhAaxisXYZ"), true, false, 4.4, 5.7);
    	c->cd(2);
    	DrawH2WithProfile(fHM->H2("fhAaxisVsX"), false, true);
    	c->cd(3);
    	DrawH2WithProfile(fHM->H2("fhAaxisVsY"), false, true);
    }

    {
    	TCanvas* c = fHM->CreateCanvas("richgeo_numbers_vs_xy_r", "richgeo_numbers_vs_xy_r", 1800, 600);
    	c->Divide(3, 1);
    	c->cd(1);
    	DrawH3Profile(fHM->H3("fhRadiusXYZ"), true, false, 4.2, 5.2);
    	c->cd(2);
    	DrawH2WithProfile(fHM->H2("fhRadiusVsX"), false, true);
       	c->cd(3);
       	DrawH2WithProfile(fHM->H2("fhRadiusVsY"), false, true);
    }

    {
    	TCanvas* c = fHM->CreateCanvas("richgeo_numbers_vs_xy_dr", "richgeo_numbers_vs_xy_dr", 1800, 600);
    	c->Divide(3, 1);
    	c->cd(1);
    	DrawH3Profile(fHM->H3("fhdRXYZ"), false, false, 0., .5);
    	c->cd(2);
    	DrawH2WithProfile(fHM->H2("fhdRVsX"), false, false);
    	c->cd(3);
    	DrawH2WithProfile(fHM->H2("fhdRVsY"), false, false);
    }
    



    {
		fHM->CreateCanvas("richgeo_acc_vs_min_nof_hits", "richgeo_acc_vs_min_nof_hits", 600, 600);
		TH1D* h = CreateAccVsMinNofHitsHist();
		h->GetXaxis()->SetRangeUser(0., 40.0);
		DrawH1(h);
    }
    
    DrawH2MeanRms(fHM->H2("fhRadiusVsNofHits"), "richgeo_hits_r_vs_nof_hits");
    DrawH2MeanRms(fHM->H2("fhAaxisVsNofHits"), "richgeo_hits_a_vs_nof_hits");
    DrawH2MeanRms(fHM->H2("fhBaxisVsNofHits"), "richgeo_hits_b_vs_nof_hits");
    DrawH2MeanRms(fHM->H2("fhDRVsNofHits"), "richgeo_hits_dr_vs_nof_hits");
    fHM->H2("fhDRVsNofHits")->GetYaxis()->SetRangeUser(-1.05, 1.05);
    
    {
		TCanvas* c = fHM->CreateCanvas("richgeo_hits_rab", "richgeo_hits_rab", 1500, 600);
		c->Divide(3, 1);
		c->cd(1);
		DrawH1andFitGauss(fHM->H2("fhRadiusVsNofHits")->ProjectionY( (string(fHM->H2("fhRadiusVsNofHits")->GetName())+ "_py" ).c_str() ), true, true, 2., 8.);
		c->cd(2);
		DrawH1andFitGauss(fHM->H2("fhAaxisVsNofHits")->ProjectionY( (string(fHM->H2("fhAaxisVsNofHits")->GetName())+ "_py" ).c_str() ), true, true, 2., 8.);
		c->cd(3);
		DrawH1andFitGauss(fHM->H2("fhBaxisVsNofHits")->ProjectionY( (string(fHM->H2("fhBaxisVsNofHits")->GetName())+ "_py" ).c_str() ), true, true, 2., 8.);
    }
}

void CbmRichGeoTest::DrawPmts()
{
    fHM->Create3<TH3D>("fhPointsXYZ", "fhPointsXYZ;X [cm];Y [cm];Z [cm];Yield", 100, -50, 50, 100, -300, 300, 100, 100, 300);
    fHM->Create3<TH3D>("fhHitsXYZ", "fhPointsXYZ;X [cm];Y [cm];Z [cm];Yield",100, -50, 50, 100, -300, 300, 100, 100, 300);
    vector<Int_t> addr = CbmRichDigiMapManager::GetInstance().GetAddresses();
    
    {
        TCanvas *c = fHM->CreateCanvas("richgeo_pmts_xy", "richgeo_pmts_xy", 1500, 1500);
        c->SetGrid(true, true);
        TH2D* pad = new TH2D("richgeo_pmts_xy", ";X [cm];Y [cm]", 1, -120, 120, 1, -210, 210);
        pad->SetStats(false);
        pad->Draw();
        DrawPmtPoint("xy", addr);
    }
    
    {
        TCanvas *c = fHM->CreateCanvas("richgeo_pmts_xz", "richgeo_pmts_xz", 1500, 1500);
        c->SetGrid(true, true);
        TH2D* pad = new TH2D("richgeo_pmts_xz", ";Z [cm];X [cm]", 1, 200, 250, 1, -120., 120. );
        pad->SetStats(false);
        pad->Draw();
        DrawPmtPoint("xz", addr);
    }
    
    {
        fHM->CreateCanvas("richgeo_pmts_yz", "richgeo_pmts_yz", 1500, 1500);
        TH2D* pad = new TH2D("richgeo_pmts_yz", ";Z [cm];Y [cm]", 1, 200, 250, 1, -220, 220);
        pad->SetStats(false);
        pad->Draw();
        DrawPmtPoint("yz", addr);
    }
    

    for (unsigned int i = 0; i < addr.size(); i++){
        CbmRichMapData* mapData = CbmRichDigiMapManager::GetInstance().GetDataByAddress(addr[i]);
        TVector3 inPos (mapData->fX, mapData->fY, mapData->fZ);
        TVector3 outPos;
        CbmRichGeoManager::GetInstance().RotatePoint(&inPos, &outPos);
    
        fHM->H3("fhPointsXYZ")->Fill(inPos.X(), inPos.Y(), inPos.Z());
        fHM->H3("fhHitsXYZ")->Fill(outPos.X(), outPos.Y(), outPos.Z());
    }
    
    {
        fHM->CreateCanvas("richgeo_pmts_points_xyz", "richgeo_pmts_points_xyz", 1500, 1500);
        fHM->H3("fhPointsXYZ")->Draw();
    }
    
    {
        fHM->CreateCanvas("richgeo_pmts_hits_xyz", "richgeo_pmts_hits_xyz", 1500, 1500);
        fHM->H3("fhHitsXYZ")->Draw();
    }
    
}

void CbmRichGeoTest::DrawPmtPoint(
		const string& options,
		const vector<Int_t>& addr)
{
    
    for (unsigned int i = 0; i < addr.size(); i++){
        CbmRichMapData* mapData = CbmRichDigiMapManager::GetInstance().GetDataByAddress(addr[i]);
        TVector3 inPos (mapData->fX, mapData->fY, mapData->fZ);
        TVector3 outPos;
        //if ( !(inPos.X() > 0 && inPos.Y() > 0)) continue;
        //cout << "DrawPmtPoint  IN x:" << inPos.X() << " y:" << inPos.Y() << " z:" << inPos.Z() << endl;
        CbmRichGeoManager::GetInstance().RotatePoint(&inPos, &outPos);
        TEllipse* pointDr = NULL;
       // cout << "DrawPmtPoint OUT x:" << outPos.X() << " y:" << outPos.Y() << " z:" << outPos.Z() << endl;
        if (options == string("xy")) pointDr = new TEllipse(outPos.X(), outPos.Y(), 0.2);
        if (options == string("xz")) pointDr = new TEllipse(outPos.Z(), outPos.X(), 0.2);
        if (options == string("yz")) pointDr = new TEllipse(outPos.Z(), outPos.Y(), 0.2);
        if (pointDr != NULL) {
            pointDr->SetFillColor(kBlue);
            pointDr->SetLineColor(kBlue);
            pointDr->Draw();
        }
        
        TEllipse* pointDrIn = NULL;
        if (options == string("xy")) pointDrIn = new TEllipse(inPos.X(), inPos.Y(), 0.15);
        if (options == string("xz")) pointDrIn = new TEllipse(inPos.Z(), inPos.X(), 0.15);
        if (options == string("yz")) pointDrIn = new TEllipse(inPos.Z(), inPos.Y(), 0.15);
        if (pointDrIn != NULL) {
            pointDrIn->SetFillColor(kRed);
            pointDrIn->SetLineColor(kRed);
            pointDrIn->Draw();
        }
    }
}

void CbmRichGeoTest::CreateStudyReport(
                                       const string& title,
                                       const vector<string>& fileNames,
                                       const vector<string>& studyNames,
                                       const string& outputDir)
{
    if (outputDir != "") gSystem->mkdir(outputDir.c_str(), true);
    
    CbmStudyReport* report = new CbmRichGeoTestStudyReport();
    fTitle = title;
    cout << "Report can be found here: " << outputDir << endl;
    report->Create(fileNames, studyNames, outputDir);
    delete report;
}

void CbmRichGeoTest::Finish()
{
    DrawHist();
    DrawPmts();
    //fHM->WriteToFile();
    fHM->SaveCanvasToImage(fOutputDir);
}

string CbmRichGeoTest::CalcEfficiency(
                                      TH1* histRec,
                                      TH1* histAcc)
{
    if (histAcc->GetEntries() == 0) { return "0"; }
    else {
        Double_t eff = 100. * Double_t(histRec->GetEntries()) / Double_t(histAcc->GetEntries());
        cout << "Efficiency:" << eff << endl;
        return Cbm::NumberToString(eff, 2);
        
        /*  stringstream ss;
         
         ss.precision(4);
         ss << eff;
         return ss.str();*/
    }
}

ClassImp(CbmRichGeoTest)


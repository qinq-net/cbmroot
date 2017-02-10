
#include "CbmRichRecoQa.h"

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
fRichProjections(NULL),
fGlobalTracks(NULL),
fNofHitsInRingMap()
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
    
    fRichProjections = (TClonesArray*) ioman->GetObject("RichProjection");
    if ( NULL == fRichProjections) { Fatal("CbmRichUrqmdTest::Init","No fRichProjections array!"); }

    InitHistograms();
    
    return kSUCCESS;
}

void CbmRichRecoQa::InitHistograms()
{
    fHM = new CbmHistManager();
    
    // 2D histogramm XY
    double xMin = -120.;
    double xMax = 120.;
    int nBinsX = 30;
    double yMin = -208;
    double yMax = 208.;
    int nBinsY = 52;

    // 1D histogram X or Y
    int nBinsX1 = 60;
    int xMin1 = -120;
    int xMax1 = 120;
    int nBinsY1 = 25;
    int yMin1 = 100;
    int yMax1 = 200;
    
    for (Int_t i = 0; i < 4; i++){
    	string s;
		if (i == 0) s = "_primel";
		if (i == 1) s = "_pi";
		if (i == 2) s = "_primel_plus";
		if (i == 3) s = "_primel_minus";

		fHM->Create2<TH2D>("fh_ring_track_distance_vs_mom_truematch"+s, "fh_ring_track_distance_vs_mom_truematch"+s+";P [GeV/c];Ring-track distance [cm];Yield (a.u.)", 20, 0., 10., 100, 0., 5.);
		fHM->Create2<TH2D>("fh_ring_track_distance_vs_mom_wrongmatch"+s, "fh_ring_track_distance_vs_mom_wrongmatch"+s+";P [GeV/c];Ring-track distance [cm];Yield (a.u.)", 20, 0., 10., 100, 0., 5.);

		fHM->Create2<TH2D>("fh_ring_track_distance_vs_nofHits_truematch"+s, "fh_ring_track_distance_vs_nofHits_truematch"+s+";Nof hits in found ring;Ring-track distance [cm];Yield (a.u.)", 40, -.5, 39.5, 100, 0., 5.);
		fHM->Create3<TH3D>("fh_ring_track_distance_vs_xy_truematch"+s, "fh_ring_track_distance_vs_xy_truematch"+s+";X [cm];Y [cm];Ring-track distance [cm]", nBinsX, xMin, xMax, nBinsY, yMin, yMax, 100, 0., 5.);
		fHM->Create2<TH2D>("fh_ring_track_distance_vs_x_truematch"+s, "fh_ring_track_distance_vs_x_truematch"+s+";X [cm];Ring-track distance [cm]", nBinsX1, xMin1, xMax1, 100, 0., 5.);
		fHM->Create2<TH2D>("fh_ring_track_distance_vs_y_truematch"+s, "fh_ring_track_distance_vs_y_truematch"+s+";Abs(Y) [cm];Ring-track distance [cm]", nBinsY1, yMin1, yMax1, 100, 0., 5.);
    }

    fHM->Create1<TH1D>("fh_mismatch_source", "fh_mismatch_source;Global track category;% from MC", 15, -0.5, 14.5);


    fHM->Create1<TH1D>("fh_mismatch_source_mom_mc", "fh_mismatch_source_mom_mc;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_sts", "fh_mismatch_source_mom_sts;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsAccRich", "fh_mismatch_source_mom_stsAccRich;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsRich", "fh_mismatch_source_mom_stsRich;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsRichTrue", "fh_mismatch_source_mom_stsRichTrue;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsNoRich", "fh_mismatch_source_mom_stsNoRich;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsNoRichRF", "fh_mismatch_source_mom_stsNoRichRF;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsNoRichRM", "fh_mismatch_source_mom_stsNoRichRM;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsNoRichNoRF", "fh_mismatch_source_mom_stsNoRichNoRF;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsNoRichProj", "fh_mismatch_source_mom_stsNoRichProj;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsNoRichNoProj", "fh_mismatch_source_mom_stsNoRichNoProj;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsNoRichRFProj", "fh_mismatch_source_mom_stsNoRichRFProj;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsRichWrong", "fh_mismatch_source_mom_stsRichWrong;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsRichWrongRF", "fh_mismatch_source_mom_stsRichWrongRF;Momentum [GeV/c];Yield", 20, 0., 10.);
    fHM->Create1<TH1D>("fh_mismatch_source_mom_stsRichWrongRM", "fh_mismatch_source_mom_stsRichWrongRM;Momentum [GeV/c];Yield", 20, 0., 10.);
}

void CbmRichRecoQa::Exec(
                         Option_t* option)
{
    fEventNum++;
    cout << "CbmRichRecoQa, event No. " <<  fEventNum << endl;
    
    FillRichRingNofHits();
    FillRingTrackDistance();
    RingTrackMismatchSource();
}

void CbmRichRecoQa::FillRichRingNofHits()
{
    fNofHitsInRingMap.clear();
    Int_t nofRichHits = fRichHits->GetEntriesFast();
    for (Int_t iHit=0; iHit < nofRichHits; iHit++) {
        CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(iHit));
        if (NULL == hit) continue;

        vector<Int_t> motherIds = CbmMatchRecoToMC::GetMcTrackMotherIdsForRichHit(hit, fRichDigis, fRichPoints, fMCTracks);
        for (UInt_t i = 0; i < motherIds.size(); i++) {
            fNofHitsInRingMap[motherIds[i]]++;
        }
    }
}

void CbmRichRecoQa::RingTrackMismatchSource()
{
	Int_t nofMcTracks = fMCTracks->GetEntriesFast();
	for(Int_t iTrack = 0; iTrack < nofMcTracks; iTrack++) {
		const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*>(fMCTracks->At(iTrack));
		if (mcTrack == NULL) continue;
		bool isEl = IsMcPrimaryElectron(mcTrack);
		if (isEl) {
			//MC
			fHM->H1("fh_mismatch_source")->Fill(0);
			fHM->H1("fh_mismatch_source_mom_mc")->Fill(mcTrack->GetP());
		}
	}


	Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
	for(Int_t iTrack = 0; iTrack < nofGlobalTracks; iTrack++) {
		const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTracks->At(iTrack));

		Int_t stsId = globalTrack->GetStsTrackIndex();
		if (stsId < 0) continue;
		const CbmTrackMatchNew* stsTrackMatch = static_cast<const CbmTrackMatchNew*>(fStsTrackMatches->At(stsId));
		if (stsTrackMatch == NULL) continue;
		int stsMcTrackId = stsTrackMatch->GetMatchedLink().GetIndex();

		CbmMCTrack* mctrack = static_cast<CbmMCTrack*>(fMCTracks->At(stsMcTrackId));
		if (mctrack == NULL) continue;
		Double_t mom = mctrack->GetP();

		bool isEl = IsMcPrimaryElectron(mctrack);
		if (!isEl) continue;

		//STS
		fHM->H1("fh_mismatch_source")->Fill(1);
		fHM->H1("fh_mismatch_source_mom_sts")->Fill(mom);

		if (fNofHitsInRingMap[stsMcTrackId] >= 7) {
			//Sts-AccRich
			fHM->H1("fh_mismatch_source")->Fill(2);
			fHM->H1("fh_mismatch_source_mom_stsAccRich")->Fill(mctrack->GetP());
		}

		Int_t richId = globalTrack->GetRichRingIndex();

		// No RICH segment
		if (richId < 0){
			//STS-noRICH
			fHM->H1("fh_mismatch_source")->Fill(5);
			fHM->H1("fh_mismatch_source_mom_stsNoRich")->Fill(mom);
			bool ringFound = WasRingFound(stsMcTrackId);
			bool ringMatched = WasRingMatched(stsMcTrackId);
			bool hasProj = HasRichProjection(stsId);
			if (ringFound) {
				//STS-NoRich RF
				fHM->H1("fh_mismatch_source")->Fill(6);
				fHM->H1("fh_mismatch_source_mom_stsNoRichRF")->Fill(mom);
			} else {
				//STS-NoRich NoRF
				fHM->H1("fh_mismatch_source")->Fill(8);
				fHM->H1("fh_mismatch_source_mom_stsNoRichNoRF")->Fill(mom);
			}

			if (ringMatched) {
				//STS-NoRich RM
				fHM->H1("fh_mismatch_source")->Fill(7);
				fHM->H1("fh_mismatch_source_mom_stsNoRichRM")->Fill(mom);
			}

			if (hasProj) {
				//STS-NoRich Proj
				fHM->H1("fh_mismatch_source")->Fill(9);
				fHM->H1("fh_mismatch_source_mom_stsNoRichProj")->Fill(mom);
			} else {
				//STS-NoRich NoProj
				fHM->H1("fh_mismatch_source")->Fill(10);
				fHM->H1("fh_mismatch_source_mom_stsNoRichNoProj")->Fill(mom);
			}


			if (ringFound && hasProj) {
				//STS-NoRich RF Proj
				fHM->H1("fh_mismatch_source")->Fill(11);
				fHM->H1("fh_mismatch_source_mom_stsNoRichRFProj")->Fill(mom);
			}

			continue;
		}

		//STS-RICH
		fHM->H1("fh_mismatch_source")->Fill(3);
		fHM->H1("fh_mismatch_source_mom_stsRich")->Fill(mom);

		const CbmTrackMatchNew* richRingMatch = static_cast<const CbmTrackMatchNew*>(fRichRingMatches->At(richId));
		if (richRingMatch == NULL) continue;
		int richMcTrackId = richRingMatch->GetMatchedLink().GetIndex();
		const CbmRichRing* ring = static_cast<const CbmRichRing*>(fRichRings->At(richId));
		if (NULL == ring) continue;

		if (stsMcTrackId == richMcTrackId) {
			//STS-RICH true
			fHM->H1("fh_mismatch_source")->Fill(4);
			fHM->H1("fh_mismatch_source_mom_stsRichTrue")->Fill(mom);
		} else {
			//STS-RICH wrong
			fHM->H1("fh_mismatch_source")->Fill(12);
			fHM->H1("fh_mismatch_source_mom_stsRichWrong")->Fill(mom);
			if (WasRingFound(stsMcTrackId)) {
				//STS-RICH wrong RF
				fHM->H1("fh_mismatch_source")->Fill(13);
				fHM->H1("fh_mismatch_source_mom_stsRichWrongRF")->Fill(mom);
			}

			if (WasRingFound(stsMcTrackId)) {
				//STS-RICH wrong RM
				fHM->H1("fh_mismatch_source")->Fill(14);
				fHM->H1("fh_mismatch_source_mom_stsRichWrongRM")->Fill(mom);
			}
		}
	}
}

bool CbmRichRecoQa::WasRingFound(Int_t mcTrackId)
{
	Int_t nofRings = fRichRings->GetEntriesFast();
	for(Int_t iR = 0; iR < nofRings; iR++) {
		const CbmRichRing* ring = static_cast<const CbmRichRing*>(fRichRings->At(iR));
		if (ring == NULL) continue;
		const CbmTrackMatchNew* richRingMatch = static_cast<const CbmTrackMatchNew*>(fRichRingMatches->At(iR));
		if (richRingMatch == NULL) continue;
		int richMcTrackId = richRingMatch->GetMatchedLink().GetIndex();
		if (richMcTrackId == mcTrackId) return true;
	}
	return false;
}

bool CbmRichRecoQa::WasRingMatched(Int_t mcTrackId)
{
	Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
	for(Int_t iTrack = 0; iTrack < nofGlobalTracks; iTrack++) {
		const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTracks->At(iTrack));

		Int_t richId = globalTrack->GetRichRingIndex();
		if (richId < 0) continue;
		const CbmTrackMatchNew* richRingMatch = static_cast<const CbmTrackMatchNew*>(fRichRingMatches->At(richId));
		if (richRingMatch == NULL) continue;
		int richMcTrackId = richRingMatch->GetMatchedLink().GetIndex();
		if (richMcTrackId == mcTrackId) {
			return true;
		}
	}

	return false;
}


bool CbmRichRecoQa::HasRichProjection(Int_t stsTrackId)
{
	if (stsTrackId < 0){
		return false;
	}
	FairTrackParam* pTrack = (FairTrackParam*)fRichProjections->At(stsTrackId);
	if (pTrack == NULL) {
		return false;
	}

	if (pTrack->GetX() == 0. && pTrack->GetY() == 0.) {
		return false;
	} else {
		return true;
	}
}

void CbmRichRecoQa::FillRingTrackDistance()
{
    Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
    for(Int_t iTrack = 0; iTrack < nofGlobalTracks; iTrack++) {
        const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTracks->At(iTrack));

        Int_t stsId = globalTrack->GetStsTrackIndex();
        Int_t richId = globalTrack->GetRichRingIndex();
        if (stsId < 0 || richId < 0) continue;
        
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
        int nofHits = ring->GetNofHits();
        
        CbmMCTrack* mctrack = static_cast<CbmMCTrack*>(fMCTracks->At(stsMcTrackId));
        if (mctrack == NULL) continue;
        double mom = mctrack->GetP();
        int charge = mctrack->GetCharge();
        
        bool isEl = IsMcPrimaryElectron(mctrack);
        bool isPi = IsMcPion(mctrack);

        if (!isEl && !isPi) continue;

        string s = "";

        for (int i = 0; i < 2; i++){
        	if (i == 0) {
        		if (isEl) {
					s = "_primel";
				} else if (isPi) {
					s = "_pi";
				} else {
					continue;
				}
        	} else if (i == 1) {
        		if (isEl) {
					if (charge > 0) {
						s = "_primel_plus";
					} else if (charge < 0) {
						s = "_primel_minus";
					} else {
						continue;
					}
				} else {
					continue;
				}
        	}

			if (stsMcTrackId == richMcTrackId) {
				fHM->H2("fh_ring_track_distance_vs_mom_truematch"+s)->Fill(mom, rtDistance);
				fHM->H3("fh_ring_track_distance_vs_xy_truematch"+s)->Fill(xc, yc, rtDistance);
				fHM->H2("fh_ring_track_distance_vs_x_truematch"+s)->Fill(xc, rtDistance);
				fHM->H2("fh_ring_track_distance_vs_y_truematch"+s)->Fill(abs(yc), rtDistance);
				fHM->H2("fh_ring_track_distance_vs_nofHits_truematch"+s)->Fill(nofHits, rtDistance);

			} else {
				fHM->H2("fh_ring_track_distance_vs_mom_wrongmatch"+s)->Fill(mom, rtDistance);
			}
        }
    }
}


void CbmRichRecoQa::DrawHist()
{
    cout.precision(4);

    
    SetDefaultDrawStyle();
    //fHM->ScaleByPattern("fh_.*", 1./fEventNum);

    {
		TCanvas* c = fHM->CreateCanvas("fh_ring_track_distance_truematch_comparison_primel", "fh_ring_track_distance_truematch_comparison_primel", 800, 800);
		TH1D* py_minus = (TH1D*)(fHM->H2("fh_ring_track_distance_vs_mom_truematch_primel_minus")->ProjectionY("fh_ring_track_distance_vs_mom_truematch_primel_minus_py")->Clone());
		py_minus->Scale(1./py_minus->Integral());
		TH1D* py_plus = (TH1D*)(fHM->H2("fh_ring_track_distance_vs_mom_truematch_primel_plus")->ProjectionY("fh_ring_track_distance_vs_mom_truematch_primel_plus_py")->Clone());
		py_plus->Scale(1./py_plus->Integral());
		DrawH1({py_minus, py_plus}, {"e_{prim}^{-} (" + this->GetMeanRmsOverflowString(py_minus, false) + ")", "e_{prim}^{+} (" + this->GetMeanRmsOverflowString(py_plus, false) + ")"},
				kLinear, kLog, true, 0.5, 0.85, 0.99, 0.99);
	}

	{
		TCanvas* c = fHM->CreateCanvas("fh_ring_track_distance_truematch_comparison_elpi", "fh_ring_track_distance_truematch_comparison_elpi", 800, 800);
		TH1D* py_el = (TH1D*)(fHM->H2("fh_ring_track_distance_vs_mom_truematch_primel")->ProjectionY("fh_ring_track_distance_vs_mom_truematch_primel_py")->Clone());
		py_el->Scale(1./py_el->Integral());
		TH1D* py_pi = (TH1D*)(fHM->H2("fh_ring_track_distance_vs_mom_truematch_pi")->ProjectionY("fh_ring_track_distance_vs_mom_truematch_pi_py")->Clone());
		py_pi->Scale(1./py_pi->Integral());
		DrawH1({py_el, py_pi}, {"e_{prim}^{#pm} (" + this->GetMeanRmsOverflowString(py_el, false) + ")", "#pi^{#pm} (" + this->GetMeanRmsOverflowString(py_pi, false) + ")"},
				kLinear, kLog, true, 0.5, 0.85, 0.99, 0.99);
	}

	{
		gStyle->SetPaintTextFormat("4.1f");
		TCanvas* c = fHM->CreateCanvas("fh_mismatch_source", "fh_mismatch_source", 1000, 800);
		Double_t nofMcEl = fHM->H1("fh_mismatch_source")->GetBinContent(1);
		cout << "nofMcEl = " << nofMcEl << endl;
		fHM->H1("fh_mismatch_source")->Scale(100./nofMcEl);
		DrawH1(fHM->H1("fh_mismatch_source"), kLinear, kLog, "hist text");
		fHM->H1("fh_mismatch_source")->SetMarkerSize(1.9);

		vector<string> labels = {"MC", "STS", "STS-Acc RICH", "STS-RICH", "STS-RICH true", "STS-NoRICH", "STS-NoRICH RF", "STS-NoRICH RM", "STS-NoRICH NoRF",
				"STS-NoRICH Prj", "STS-NoRICH NoPrj", "STS-NoRICH RF-Prj", "STS-RICH wrong", "STS-RICH wrong RF", "STS-RICH wrong RM"};
		for (int i = 0; i < labels.size(); i++) {
			fHM->H1("fh_mismatch_source")->GetXaxis()->SetBinLabel(i+1, labels[i].c_str());
		}
		fHM->H1("fh_mismatch_source")->GetXaxis()->SetLabelSize(0.03);
	}

	{
		TCanvas* c = fHM->CreateCanvas("fh_mismatch_source_mom", "fh_mismatch_source_mom", 1000, 800);
		vector<string> labels = {"MC", "STS", "STS-Acc RICH", "STS-RICH", "STS-RICH true", "STS-NoRICH", "STS-NoRICH RF","STS-NoRICH RM",
				"STS-NoRICH NoRF",  "STS-NoRICH Prj", "STS-NoRICH NoPrj",  "STS-NoRICH RF-Prj", "STS-RICH wrong", "STS-RICH wrong RF", "STS-RICH wrong RM"};
		vector<TH1*> hists = {fHM->H1("fh_mismatch_source_mom_mc"), fHM->H1("fh_mismatch_source_mom_sts"), fHM->H1("fh_mismatch_source_mom_stsAccRich"),
				fHM->H1("fh_mismatch_source_mom_stsRich"), fHM->H1("fh_mismatch_source_mom_stsRichTrue"), fHM->H1("fh_mismatch_source_mom_stsNoRich"),
				fHM->H1("fh_mismatch_source_mom_stsNoRichRF"), fHM->H1("fh_mismatch_source_mom_stsNoRichRM"), fHM->H1("fh_mismatch_source_mom_stsNoRichNoRF"),
				fHM->H1("fh_mismatch_source_mom_stsNoRichProj"), fHM->H1("fh_mismatch_source_mom_stsNoRichNoProj"), fHM->H1("fh_mismatch_source_mom_stsNoRichRFProj"),
				fHM->H1("fh_mismatch_source_mom_stsRichWrong"),fHM->H1("fh_mismatch_source_mom_stsRichWrongRF"),fHM->H1("fh_mismatch_source_mom_stsRichWrongRM")  };

		DrawH1(hists, labels, kLinear, kLog, true, 0.8, 0.35, 0.99, 0.99);
		fHM->H1("fh_mismatch_source_mom_mc")->SetMinimum(0.9);
	}

    DrawRingTrackDistHistWithSuffix("_primel");
    DrawRingTrackDistHistWithSuffix("_primel_plus");
    DrawRingTrackDistHistWithSuffix("_primel_minus");
    DrawRingTrackDistHistWithSuffix("_pi");
}

string CbmRichRecoQa::GetMeanRmsOverflowString(
		TH1* h,
		Bool_t withOverflow)
{
	if (withOverflow) {
		double overflow = h->GetBinContent(h->GetNbinsX() + 1);
		return Cbm::NumberToString<Double_t>(h->GetMean(), 2) + " / " + Cbm::NumberToString<Double_t>(h->GetRMS(), 2) +
			"/" + Cbm::NumberToString<Double_t>(100.* overflow / h->Integral(0, h->GetNbinsX() + 1), 2) + "%";
	} else {
		return Cbm::NumberToString<Double_t>(h->GetMean(), 2) + " / " + Cbm::NumberToString<Double_t>(h->GetRMS(), 2);
	}
}


void CbmRichRecoQa::DrawRingTrackDistHistWithSuffix(const string& s )
{
    {
        TCanvas* c = fHM->CreateCanvas("fh_ring_track_distance_truematch"+s, "fh_ring_track_distance_truematch"+s, 1800, 600);
        c->Divide(3,1);
        c->cd(1);
        DrawH2WithProfile(fHM->H2("fh_ring_track_distance_vs_mom_truematch"+s), false, true);
        c->cd(2);
        DrawH2WithProfile(fHM->H2("fh_ring_track_distance_vs_nofHits_truematch"+s), false, true);
        c->cd(3);
        TH1D* py = (TH1D*)(fHM->H2("fh_ring_track_distance_vs_mom_truematch"+s)->ProjectionY(("fh_ring_track_distance_vs_mom_truematch_py"+s).c_str())->Clone());
        DrawH1(py);
        DrawTextOnPad(this->GetMeanRmsOverflowString(py), 0.2, 0.9, 0.8, 0.99);
        gPad->SetLogy(true);
        
        fHM->H2("fh_ring_track_distance_vs_mom_truematch"+s)->GetYaxis()->SetRangeUser(0., 2.);
        fHM->H2("fh_ring_track_distance_vs_nofHits_truematch"+s)->GetYaxis()->SetRangeUser(0., 2.);
    }
    
    {
        TCanvas* c = fHM->CreateCanvas("fh_ring_track_distance_wrongmatch"+s, "fh_ring_track_distance_wrongmatch"+s, 600, 600);
        TH1D* py = (TH1D*)(fHM->H2("fh_ring_track_distance_vs_mom_wrongmatch"+s)->ProjectionY(("fh_ring_track_distance_vs_mom_wrongmatch_py"+s).c_str())->Clone());
        DrawH1(py);
        DrawTextOnPad(this->GetMeanRmsOverflowString(py), 0.2, 0.9, 0.8, 0.99);
        gPad->SetLogy(true);
    }
    
    {
    	TCanvas* c = fHM->CreateCanvas("fh_ring_track_distance_vs_xy_truematch"+s, "fh_ring_track_distance_vs_xy_truematch"+s, 1800, 600);
    	c->Divide(3, 1);
    	c->cd(1);
    	DrawH3Profile(fHM->H3("fh_ring_track_distance_vs_xy_truematch"+s), true, false, 0., .4);
    	c->cd(2);
    	DrawH2WithProfile(fHM->H2("fh_ring_track_distance_vs_x_truematch"+s), false, true);
    	fHM->H2("fh_ring_track_distance_vs_x_truematch"+s)->GetYaxis()->SetRangeUser(0., 2.);
    	c->cd(3);
    	DrawH2WithProfile(fHM->H2("fh_ring_track_distance_vs_y_truematch"+s), false, true);
    	fHM->H2("fh_ring_track_distance_vs_y_truematch"+s)->GetYaxis()->SetRangeUser(0., 2.);
    }
}

bool CbmRichRecoQa::IsMcPrimaryElectron(
			const CbmMCTrack* mctrack)
{
	if (mctrack == NULL) return false;
	int pdg = TMath::Abs(mctrack->GetPdgCode());
	if (mctrack->GetGeantProcessId() == kPPrimary && pdg == 11) return true;
	return false;
}

bool CbmRichRecoQa::IsMcPion(
				const CbmMCTrack* mctrack)
{
	if (mctrack == NULL) return false;
	int pdg = TMath::Abs(mctrack->GetPdgCode());
	if (pdg == 211) return true;
	return false;
}

void CbmRichRecoQa::Finish()
{
    DrawHist();
    fHM->SaveCanvasToImage(fOutputDir);
    fHM->WriteToFile();
}

void CbmRichRecoQa::DrawFromFile(
      const string& fileName,
      const string& outputDir)
{
	fOutputDir = outputDir;

	if (fHM != NULL) delete fHM;

	fHM = new CbmHistManager();
	TFile* file = new TFile(fileName.c_str());
	fHM->ReadFromFile(file);

	DrawHist();

	fHM->SaveCanvasToImage(fOutputDir);
}

ClassImp(CbmRichRecoQa)


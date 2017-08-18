#include "CbmAnaConversion2.h"
#include "../../rich/fitter/CbmRichRingFitterCOP.h"
#include "../../rich/fitter/CbmRichRingFitterEllipseTau.h"
#include "../../rich/fitter/CbmRichRingFitterEllipseMinuit.h"
#include "TProfile2D.h"
#include "TProfile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TH1.h"
#include "TF1.h"
#include "TGraph.h"
#include "TText.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "CbmRichRingLight.h"
#include "CbmRichHitProducer.h"
#include "CbmRichConverter.h"
#include "FairTrackParam.h"
#include "CbmRichHit.h"
#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmRichPoint.h"
#include "CbmRichRing.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"
#include "CbmHistManager.h"
#include "detector/CbmRichGeoManager.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"
#include "CbmStsKFTrackFitter.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>

#include "CbmStsHit.h"
#include "../../data/mvd/CbmMvdHit.h"

// includes of further conversion classes
#include "CbmAnaConversion2Reconstruction.h"
#include "CbmAnaConversion2KF.h"
#include "CbmAnaConversion2Manual.h"

#define M2E 2.6112004954086e-7

using namespace std;
using boost::assign::list_of;

CbmAnaConversion2::CbmAnaConversion2()
  : FairTask("CbmAnaConversion2"),
    fHistoList(),
    fMcTracks(nullptr),
    fRichHits(nullptr),
    fRichRings(nullptr),
    fRichPoints(nullptr),
    fStsTrackMatches(nullptr),
    fGlobalTracks(nullptr),
    fRichRingMatches(nullptr),
    fRichProjections(nullptr),
    fTauFit(nullptr),
    fRichRings_Aaxis(nullptr),
    fRichRings_Baxis(nullptr),
    fMinAaxis(0.),
    fMaxAaxis(0.),
    fMinBaxis(0.),
    fMaxBaxis(0.),
    fMinRadius(0.),
    fMaxRadius(0.),
    DecayedParticlePdg(0.),
    LeptonsInDecay(0.),
    Dalitz (0.),
    fhPio_pt(nullptr),
    fhPio_pt_vs_rap(nullptr),
    fhPio_theta(nullptr),
    fhPio_theta_vs_rap(nullptr),
    fhPdgCodes(nullptr),
    dR_plot_electrons(nullptr),
    NumberOfRings_electrons(nullptr),
    AllPoints_from_hits_electrons(nullptr),
    AllPoints2D(nullptr),
    eff(nullptr),
    STSmomenta(nullptr),
    ConversionPoints2D(nullptr),
    Invariant_masses(nullptr),
    AllPoints3D(nullptr),
    ConversionPoints(nullptr),
    A_electrons(nullptr),
    B_electrons(nullptr),
    hprof2d_electrons(nullptr),
    BoA_electrons(nullptr),
    counter_full_ind_STS(0.),
    counter_full_ind_RICH(0.),
    UsedRingsId(),
    fPrimVertex(nullptr),
    fKFVertex(),
    fStsTracks(nullptr),
    DoReconstruction(0),
    DoKFAnalysis(0),
    fAnaReco(nullptr),
    fMCTracklist_gamma_from_our_particle(),                  // particle -> gamma                                        from our particle
    fMCTracklist_gamma_all(),                                // gamma                                                    from everywhere (target, all particles, bremstralung)
    fMCTracklist_elpositrons_from_our_particle(),            // particle ->  e+ and  e-                                  decayed from our particle
    fMCTracklist_elpositrons_from_gamma(),                   // gamma (conversion) -> e+ and  e-                         conversion from gamma (target, all particles)
    fMCTracklist_elpositrons_all(),                          // --> e+ or e-                                             from everywhere (target, all particles, conversions)
    fMCTracklist_elpositrons_from_gamma_from_our_particle(), // particle -> gamma (conversion) -> e+ and  e-             conversion from our particle
    fTracklist_noRichInd_momentum(),                          // fitted momentum after STS and befor RICH identification
    fTracklist_noRichInd_mcTrack(),
    fTracklist_noRichInd_conversion(),
    fTracklist_noRichInd_refmomentum_direct(),
    fTracklist_noRichInd_refmomentum_conversion(),
    fTracklist_withRichInd_refmomentum_direct(),
    fTracklist_withRichInd_refmomentum_conversion(),
    fTracklist_withRichInd_mcTrack(),
    fTracklist_noRichInd_refmomentum_all(),
    fTracklist_withRichInd_refmomentum_all(),
    fTracklist_noRichInd_mcTrack_STS_Id(),

    fAnaKF(nullptr),
    fKFparticle(nullptr),
    fKFparticleFinderQA(nullptr),

    fAnaFitter(nullptr),
    DoFitter(0),

    fArrayStsHit(nullptr),
    fArrayMvdHit(nullptr),

    DoManualAnalysis(0),
    fAnaManual(nullptr),


    fEventNum(0)

{
   fMinAaxis = 3.;
   fMaxAaxis = 7.;
   fMinBaxis = 3.;
   fMaxBaxis = 7.;
   fMinRadius = 3.;
   fMaxRadius = 7.;
   DecayedParticlePdg = 111.;  // omega = 223; rho0 = 113; pi0 = 111;
   LeptonsInDecay = 4;
   Dalitz = 2;
}

CbmAnaConversion2::~CbmAnaConversion2()
{

}

InitStatus CbmAnaConversion2::Init()
{

   cout << "CbmAnaConversion2::Init"<<endl;
   FairRootManager* ioman = FairRootManager::Instance();
   if (nullptr == ioman) { Fatal("CbmAnaConversion2::Init","RootManager not instantised!"); }

   fRichProjections = (TClonesArray*) ioman->GetObject("RichProjection");
   if (nullptr == fRichProjections) {Fatal("CbmRichEventDisplay::Init", "No RichProjection array!");}

   fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
   if ( nullptr == fRichPoints) { Fatal("CbmAnaConversion2::Init","No RichPoint array!"); }

   fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( nullptr == fMcTracks) { Fatal("CbmAnaConversion2::Init","No MCTrack array!"); }

   fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
   if ( nullptr == fStsTracks) { Fatal("CbmAnaConversion2::Init","No StsTrack array!"); }

   fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
   if (nullptr == fStsTrackMatches) { Fatal("CbmAnaConversion2::Init","No StsTrackMatch array!"); }

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if (nullptr == fRichRingMatches) { Fatal("CbmAnaConversion::Init","No RichRingMatch array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (nullptr == fGlobalTracks) { Fatal("CbmAnaConversion2::Init","No GlobalTrack array!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if (nullptr == fRichRings) { Fatal("CbmAnaConversion::Init","No RichRing array!"); }

	fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
	if ( nullptr == fRichHits) { Fatal("CbmRichGeoTest::Init","No RichHit array!"); }

	fTauFit = new CbmRichRingFitterEllipseTau();

	InitHistograms();


        // Get pointer to PrimaryVertex object from IOManager if it exists
        // The old name for the object is "PrimaryVertex" the new one
        // "PrimaryVertex." Check first for the new name
        fPrimVertex = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex."));
        if (nullptr == fPrimVertex) {
          fPrimVertex = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex"));
        }
        if (nullptr == fPrimVertex) { 
          LOG(FATAL) << "No PrimaryVertex array!" << FairLogger::endl;
        }

	fArrayStsHit = (TClonesArray*) ioman->GetObject("StsHit");
	if(nullptr == fArrayStsHit) { Fatal("CbmAnaConversion2Fitter::Init","No StsHit array!"); }

	fArrayMvdHit = (TClonesArray*) ioman->GetObject("MvdHit");
	if(nullptr == fArrayMvdHit) { Fatal("CbmAnaConversion2Fitter::Init","No MvdHit array!"); }


	DoReconstruction = 1;
	DoKFAnalysis = 1;
	DoFitter = 0;
	DoManualAnalysis = 1;


	if(DoReconstruction) {
		fAnaReco = new CbmAnaConversion2Reconstruction();
		fAnaReco->Init();
	}
	if(DoKFAnalysis) {
		fAnaKF = new CbmAnaConversion2KF();
		fAnaKF->SetKF(fKFparticle, fKFparticleFinderQA);
		fAnaKF->Init();
	}
	if(DoFitter) {
		fAnaFitter = new CbmAnaConversion2Fitter();
		fAnaFitter->Init();
	}
	if(DoManualAnalysis) {
		fAnaManual = new CbmAnaConversion2Manual();
		fAnaManual->Init();
	}



   return kSUCCESS;
}

void CbmAnaConversion2::InitHistograms()
{
	double xMin = -150.;
	double xMax = 150.;
	int nBinsX = 50;
	double yMin = 105.;
	double yMax = 205.;
	int nBinsY = 20;
	int factor = 10;

	AllPoints2D = new TH2D("AllPoints2D", "AllPoints2D ;X [cm];Y [cm]; Nof", nBinsX*factor, xMin, xMax, nBinsY*factor, yMin, yMax);
	fHistoList.push_back(AllPoints2D);

	AllPoints3D = new TH3D("AllPoints3D", "AllPoints3D ;x; y; z", 500, -120., 120, 50, 100., 200, 300, 190, 250);
	fHistoList.push_back(AllPoints3D);

	fhPdgCodes = new TH1D("fhPdfCodes",	"fhPdgCodes;pdg code;#", 1000, 0, 1000);
	fHistoList.push_back(fhPdgCodes);

	fhPio_pt = new TH1D("fhPio_pt",			"fhPio_pt;p_{t} in GeV/c;Entries", 200., 0., 10.);
	fHistoList.push_back(fhPio_pt);

	fhPio_pt_vs_rap = new TH2D("fhPio_pt_vs_rap",	"fhPio_pt_vs_rap;p_{t} in GeV/c; rapidity y", 120, -2., 10., 90, -2., 7.);
	fHistoList.push_back(fhPio_pt_vs_rap);

	fhPio_theta = new TH1D("fhPio_theta",		"fhPio_theta;emission angle #theta in deg;Entries", 180., 0., 180.);
	fHistoList.push_back(fhPio_theta);

	fhPio_theta_vs_rap = new TH2D("fhPio_theta_vs_rap","fhPio_theta_vs_rap;emission angle #theta in deg;rapidity y", 180., 0., 180., 270, -2., 7.);
	fHistoList.push_back(fhPio_theta_vs_rap);

	fRichRings_Aaxis = new TH1D("fRichRings_Aaxis", "fRichRings_Aaxis;A-axis;#", 200, 0., 10.);
	fHistoList.push_back(fRichRings_Aaxis);

	fRichRings_Baxis = new TH1D("fRichRings_Baxis", "fRichRings_Baxis;B-axis;#", 200, 0., 10.);  
	fHistoList.push_back(fRichRings_Baxis);

	BoA_electrons = new TProfile2D("BoA_electrons"," BoA_electrons  ",nBinsX, xMin, xMax, nBinsY, yMin, yMax);
	fHistoList.push_back(BoA_electrons);

	A_electrons = new TProfile2D("A_electrons"," A_electrons  ",nBinsX, xMin, xMax, nBinsY, yMin, yMax);
	fHistoList.push_back(A_electrons);

	B_electrons = new TProfile2D("B_electrons"," B_electrons  ",nBinsX, xMin, xMax, nBinsY, yMin, yMax);
	fHistoList.push_back(B_electrons);

	NumberOfRings_electrons = new TH2D("NumberOfRings_electrons", "NumberOfRings_electrons ;X [cm];Y [cm]; Nof", nBinsX, xMin, xMax, nBinsY, yMin, yMax);
	fHistoList.push_back(NumberOfRings_electrons);

	AllPoints_from_hits_electrons = new TH2D("AllPoints_from_hits_electrons", "Hits_after_tilt_calculation_electrons ;X [cm];Y [cm]; Nof", nBinsX*factor, xMin, xMax, nBinsY*factor, yMin, yMax);
	fHistoList.push_back(AllPoints_from_hits_electrons);

	dR_plot_electrons = new TH1D("dR_plot_electrons", "dR_plot_electrons", 50, -2., 2.);
	fHistoList.push_back(dR_plot_electrons);

	hprof2d_electrons  = new TProfile2D("hprof2d_electrons","dr_electrons ",nBinsX, xMin, xMax, nBinsY, yMin, yMax, 0, 1);
	fHistoList.push_back(hprof2d_electrons);

	eff = new TH2D("eff", "efficiency ;STS [cm];RICH [cm]; Nof", 100, -1, 10, 100, -1, 10);
	fHistoList.push_back(eff);

	STSmomenta = new TH2D ("STSmomenta", "momenta from STS ; MCTrue_Pt [GeV]; reffited_Pt[GeV]", 1000, 0, 1.1, 1000, 0, 1.1);
	fHistoList.push_back(STSmomenta);

	ConversionPoints = new TH3D("ConversionPoints", "ConversionPoints ;z [cm];x [cm];y [cm]", 200, -1, 200, 300, -150, 150, 400, -200, 200);
	fHistoList.push_back(ConversionPoints);

	ConversionPoints2D = new TH2D("ConversionPoints2D", "ConversionPoints2D ;z [cm];y [cm]", 200, -1, 200,  400, -200, 200);
	fHistoList.push_back(ConversionPoints2D);

	Invariant_masses = new TH2D ("Invariant_masses", "invariant masses ; reco [MeV]; MC [MeV]", 300, 100, 700, 100, 100, 200);
	fHistoList.push_back(Invariant_masses);

}

void CbmAnaConversion2::Exec(
      Option_t* option)
{
	fEventNum++;
//	cout << "CbmAnaConversion2, event No. " <<  fEventNum-1 << endl;

	if (fPrimVertex != nullptr){
		fKFVertex = CbmKFVertex(*fPrimVertex);
	} else {
		Fatal("CbmAnaConversion::Exec","No PrimaryVertex array!");
	}

	double OpeningAngleCut = 1.;
	double GammaInvMassCut = 0.01;


	if(DoKFAnalysis) {
		fAnaKF->Exec(fEventNum-1, OpeningAngleCut, GammaInvMassCut);
	}

	if(DoManualAnalysis) {
		fAnaManual->Exec(fEventNum-1, OpeningAngleCut, GammaInvMassCut);
	}

	if(DoFitter) {
		fAnaFitter->Exec(fEventNum-1);
	}




	fMCTracklist_gamma_from_our_particle.clear();
	fMCTracklist_gamma_all.clear();
	fMCTracklist_elpositrons_from_our_particle.clear();
	fMCTracklist_elpositrons_from_gamma.clear();
	fMCTracklist_elpositrons_all.clear();
	fMCTracklist_elpositrons_from_gamma_from_our_particle.clear();
	fTracklist_noRichInd_momentum.clear();
	fTracklist_noRichInd_mcTrack.clear();
	fTracklist_noRichInd_conversion.clear();
	fTracklist_noRichInd_refmomentum_direct.clear();
	fTracklist_noRichInd_refmomentum_conversion.clear();
	fTracklist_withRichInd_refmomentum_direct.clear();
	fTracklist_withRichInd_refmomentum_conversion.clear();
	fTracklist_withRichInd_mcTrack.clear();
	fTracklist_noRichInd_refmomentum_all.clear();
	fTracklist_withRichInd_refmomentum_all.clear();
	fTracklist_noRichInd_mcTrack_STS_Id.clear();




	// ========================================================================================
	///////   Rotate all Points
	CbmRichRingLight ringPoint;
	int nofRichPoints = fRichPoints->GetEntriesFast();
	for (int iPoint = 0; iPoint < nofRichPoints; iPoint++){
		CbmRichPoint* richPoint = (CbmRichPoint*) fRichPoints->At(iPoint);
		TVector3 posPoint;
		richPoint->Position(posPoint);
		TVector3 detPoint;
		CbmRichGeoManager::GetInstance().RotatePoint(&posPoint, &detPoint);
		CbmRichHitLight hit(detPoint.X(), detPoint.Y());
		ringPoint.AddHit(hit);
	}
	///////   Rotate all Points (END)
	// ========================================================================================




	// ========================================================================================
	///////   Fill all rotated points in one histogram
	for (int i = 0; i < nofRichPoints; i++) {
		CbmRichPoint* point = (CbmRichPoint*)fRichPoints->At(i);
		if (point == nullptr) continue;
		double xPOINT = point->GetX();
		double yPOINT = point->GetY();
		double zPOINT = point->GetZ();
		AllPoints3D->Fill(xPOINT,yPOINT,zPOINT);
		AllPoints2D->Fill(xPOINT,yPOINT);
	}
	///////   Fill all rotated points in one histogram (END)
	// ========================================================================================





	// ========================================================================================
	///////   START - Analyse MC tracks
	Int_t nofMcTracks = fMcTracks->GetEntriesFast();
	for (int i = 0; i < nofMcTracks; i++) {
		CbmMCTrack* mctrack = (CbmMCTrack*)fMcTracks->At(i);
		if (mctrack == nullptr) continue;

		FillMCTracklists(mctrack, i);	// fill tracklists for further analyses ???????

		fhPdgCodes->Fill(TMath::Abs(mctrack->GetPdgCode()));
		if (TMath::Abs(mctrack->GetPdgCode() ) == DecayedParticlePdg) { 
			TVector3 v, momentum;
			mctrack->GetStartVertex(v);
			mctrack->GetMomentum(momentum);
			fhPio_pt->Fill(mctrack->GetPt() );
			fhPio_pt_vs_rap->Fill(mctrack->GetPt(), mctrack->GetRapidity() );
			fhPio_theta->Fill(momentum.Theta() * 180. / TMath::Pi() );
			fhPio_theta_vs_rap->Fill(momentum.Theta() * 180. / TMath::Pi(), mctrack->GetRapidity() );
		}
	}
	///////   START - Analyse MC tracks (END)
	// ========================================================================================




	///////   Fill all parameters of fitted rings from RICH
	// ========================================================================================
	Int_t nofRings = fRichRings->GetEntriesFast();
    int nofProjections = fRichProjections->GetEntriesFast();
	for (int i = 0; i < nofRings; i++) {
		CbmRichRing* richRing = (CbmRichRing*)fRichRings->At(i);
		if(richRing == nullptr) continue;
		fRichRings_Aaxis->Fill(richRing->GetAaxis());
		fRichRings_Baxis->Fill(richRing->GetBaxis());
	}
	///////   Fill all parameters of fitted rings from RICH (END)
	// ========================================================================================




	Int_t PrimaryCandidateSTS = 0;
	Int_t ConversionCandidateSTS = 0;
	Int_t PrimaryCandidateRICH = 0;
	Int_t candidateFromGammawithoutSTS = 0;
	Int_t ConversionCandidateRICH = 0;
	UsedRingsId.clear();
	Int_t NofReconstructedInSTS = 0;
	Int_t NofReconstructedInRICH = 0;
	Int_t NofFullCorrectReconstruction = 0;
	///////   Global tracks analysis
	// ========================================================================================
	Int_t ngTracks = fGlobalTracks->GetEntriesFast();
	for (Int_t iTr = 0; iTr < ngTracks; iTr++) {
		CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(iTr);
		if(nullptr == gTrack) continue;
		int stsInd = gTrack->GetStsTrackIndex();
		int richInd = gTrack->GetRichRingIndex();


		// ========================================================================================
		///////   STS
		if (stsInd < 0) continue;    
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
		if (stsTrack == nullptr) continue;
		CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*) fStsTrackMatches->At(stsInd);
		if (stsMatch == nullptr) continue;
		if (stsMatch->GetNofLinks() <= 0) continue;
		int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
		if (stsMcTrackId < 0) continue;
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
		if (mcTrack1 == nullptr) continue;
		int pdgSTS = mcTrack1->GetPdgCode();
		int motherIdSTS = mcTrack1->GetMotherId();
		double momentumSTS = mcTrack1->GetP();

		if(TMath::Abs(pdgSTS) != 11) continue;
		if (motherIdSTS == -1) continue;
		CbmMCTrack* mcTrackMotherSTS = (CbmMCTrack*) fMcTracks->At(motherIdSTS);
		if (mcTrackMotherSTS == nullptr) continue;
		TVector3 refmomentum_direct_elposinton;
		TVector3 refmomentum_gamma_conversion_elposinton;
		TVector3 refmomentum_all;
		if (mcTrackMotherSTS->GetPdgCode() == DecayedParticlePdg) {
			if (mcTrackMotherSTS->GetMotherId() != -1) continue; /// which were created inside the target
			PrimaryCandidateSTS++;
			//refmomentum_direct_elposinton = STSReffit(stsTrack);
			refmomentum_direct_elposinton = FitToVertexSTS(stsTrack, mcTrack1);
			fTracklist_noRichInd_refmomentum_direct.push_back(refmomentum_direct_elposinton);
			//cout << "from refmomentum Pt = " << refmomentum.Perp() << "; Px = " << refmomentum.X() << "; Py = " << refmomentum.Y() << "; Pz = " << refmomentum.Z()<< endl;
			//cout << "pdg of registered particle in  STS =  " << pdgSTS << "; momentum = " << mcTrack1->GetP() << "; Pt = " << mcTrack1->GetPt() << endl;
		}
		else if (mcTrackMotherSTS->GetPdgCode() == 22) { // particle is gamma = 22 --> conversion
			if(mcTrackMotherSTS->GetMotherId() == -1) continue;
			CbmMCTrack* mcTrackMotherOfGammaSTS = (CbmMCTrack*) fMcTracks->At(mcTrackMotherSTS->GetMotherId());
			if (mcTrackMotherOfGammaSTS->GetPdgCode() != DecayedParticlePdg) continue;
			float xConv = mcTrack1->GetStartX();
			float yConv = mcTrack1->GetStartY();
			float zConv = mcTrack1->GetStartZ();
			ConversionPoints->Fill(zConv, xConv, yConv);
			ConversionPoints2D->Fill(zConv, yConv);
			fTracklist_noRichInd_conversion.push_back(mcTrack1);
			ConversionCandidateSTS++;
			//refmomentum_gamma_conversion_elposinton = STSReffit(stsTrack);
			refmomentum_gamma_conversion_elposinton = FitToVertexSTS(stsTrack, mcTrack1);
			fTracklist_noRichInd_refmomentum_conversion.push_back(refmomentum_gamma_conversion_elposinton);
			//cout << "pdg in STS from gamma =  " << pdgSTS << "; momentum of gamma = " << mcTrackMotherSTS->GetP() << "; Pt = " << mcTrackMotherSTS->GetPt() << endl;
			//cout << "pdg in STS from gamma =  " << pdgSTS << "; momentum = " << mcTrack1->GetP() << "; Pt = " << mcTrack1->GetPt() << " --> "  << " xConv = " << xConv << "; yConv = " << yConv << 
			//"; zConv = " << zConv << " <-- Coordinates of start vertex [cm]" << endl;
		}
		else { 
			continue;
		}
		NofReconstructedInSTS++;

		//refmomentum_all = STSReffit(stsTrack);
		refmomentum_all = FitToVertexSTS(stsTrack, mcTrack1);
		fTracklist_noRichInd_refmomentum_all.push_back(refmomentum_all);
		fTracklist_noRichInd_mcTrack.push_back(mcTrack1);
		fTracklist_noRichInd_mcTrack_STS_Id.push_back(stsMcTrackId);
		///////   STS (END)
		// ========================================================================================




		// ========================================================================================
		///////   RICH 
		if (nofRings == 0) continue; 
		int TT=0;
		for(TT; TT<nofRings; TT++){
			CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(TT);
			if (richMatch == nullptr) continue;
			int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
			if (richMcTrackId < 0) continue;
			if(stsMcTrackId == richMcTrackId) {
				UsedRingsId.push_back(TT);
				break;
			}
		}



		CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(TT);
		if (richMatch == nullptr) continue;
		int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
		if (richMcTrackId < 0) continue;
		CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
		if (mcTrack2 == nullptr) continue;
		int pdgRICH = mcTrack2->GetPdgCode();
		int motherIdRICH = mcTrack2->GetMotherId();
		double momentumRICH = mcTrack2->GetP();

		if(TMath::Abs(pdgRICH) != 11) continue;
		if (motherIdRICH == -1) continue;
		CbmMCTrack* mcTrackMotherRICH = (CbmMCTrack*) fMcTracks->At(motherIdRICH);
		if (mcTrackMotherRICH == nullptr) continue;
		if (mcTrackMotherRICH->GetPdgCode() == DecayedParticlePdg) {
			if (mcTrackMotherRICH->GetMotherId() != -1) continue; /// which were created inside the target
			PrimaryCandidateRICH++;
			fTracklist_withRichInd_refmomentum_direct.push_back(refmomentum_direct_elposinton);
		}
		else if (mcTrackMotherRICH->GetPdgCode() == 22) { // particle is gamma = 22 --> conversion
			CbmMCTrack* mcTrackMotherOfGammaRICH = (CbmMCTrack*) fMcTracks->At(mcTrackMotherRICH->GetMotherId());
			if (mcTrackMotherOfGammaRICH->GetPdgCode() != DecayedParticlePdg) continue;
			ConversionCandidateRICH++;
			fTracklist_withRichInd_refmomentum_conversion.push_back(refmomentum_gamma_conversion_elposinton);
		}
		else { 
			continue;
		}
		NofReconstructedInRICH++;
		//cout << "pdg of registered particle in RICH =  " << mcTrack2->GetPdgCode() << "; momentum = " << mcTrack2->GetP() << "; Pt = " << mcTrack2->GetPt() <<  endl;
		//cout << " " << endl;

		fTracklist_withRichInd_refmomentum_all.push_back(refmomentum_all);
		fTracklist_withRichInd_mcTrack.push_back(mcTrack1);

		///////   RICH (END)
		// ========================================================================================



		// ========================================================================================
		///////   dR 
		if(stsMcTrackId == richMcTrackId) {
			NofFullCorrectReconstruction++;
			CbmRichRing* ring = static_cast<CbmRichRing*> (fRichRings->At(TT));
			if (nullptr == ring) continue;
			CbmRichRingLight ringHit;
			int nofHits = ring->GetNofHits();
			for (int i = 0; i < nofHits; i++){
				Int_t hitInd = ring->GetHit(i);
				CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
				if (nullptr == hit) continue;
				CbmRichHitLight hl(hit->GetX(), hit->GetY());
				ringHit.AddHit(hl);
			}
			FitAndFillHistEllipse(0, &ringHit); // hits
			if (ringHit.GetAaxis() > fMinAaxis && ringHit.GetAaxis() < fMaxAaxis  &&  ringHit.GetBaxis() > fMinBaxis && ringHit.GetAaxis() < fMaxBaxis ){
				double np = ringHit.GetNofHits();
				double a = ringHit.GetAaxis();
				double b = ringHit.GetBaxis();
				double p = ringHit.GetPhi();
				double nh = ring->GetNofHits();
				double xc = ringHit.GetCenterX();
				double yc = ringHit.GetCenterY();
				BoA_electrons->Fill(xc, yc, b/a);
				A_electrons->Fill(xc, yc, a);
				B_electrons->Fill(xc, yc, b);
				NumberOfRings_electrons->Fill(xc, yc);

				double minAngle = 0.;
				double maxAngle = 2*3.14159265358979323846;
				double stepAngle = 0.01;
				for (int iH = 0; iH < ringHit.GetNofHits(); iH++){
					double xh = ringHit.GetHit(iH).fX;
					double yh = ringHit.GetHit(iH).fY;
					AllPoints_from_hits_electrons->Fill(xh,yh);
					double Lmin = 50.;
					double xmin = 0.;
					double ymin = 0.;
					Int_t index=0;
					for (double iT = minAngle; iT <maxAngle; iT=iT+stepAngle){
						double xEll = a*cos(iT)*cos(p) - b*sin(iT)*sin(p)+xc;
						double yEll = a*cos(iT)*sin(p) + b*sin(iT)*cos(p)+yc;
						double L = sqrt((xh-xEll)*(xh-xEll)+(yh-yEll)*(yh-yEll));
						if (L < Lmin){
							Lmin = L;
							xmin = xEll;
							ymin = yEll;
						}
					}
					Double_t sign = sqrt((xmin)*(xmin)+(ymin)*(ymin)) - sqrt((xh)*(xh)+(yh)*(yh));
					double dr = Lmin;
					if (sign < 0){
						dr = -Lmin;
					}
					dR_plot_electrons->Fill(dr);
					hprof2d_electrons->Fill(xc, yc, dr);
				}
			}
		}
		///////   dR (END)
		// ========================================================================================
	}
	///////   Global tracks analysis (END)
	// ========================================================================================






	///////   Dalitz analysis --> check if we have rings from conversion, which tracks were not registered in STS
	// ========================================================================================
/*	if (Dalitz != 0){ 
		if (UsedRingsId.size() < nofRings){
			vector<int>::iterator it;
			for (int UnidentifiedRingId = 0; UnidentifiedRingId < nofRings; UnidentifiedRingId++){
				it = find (UsedRingsId.begin(), UsedRingsId.end(), UnidentifiedRingId);
				if (it == UsedRingsId.end()){
					CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(UnidentifiedRingId);
					if (richMatch != nullptr){
						int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
						if (richMcTrackId >= 0){
							CbmMCTrack* mcTrackFromG = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
							if (mcTrackFromG != nullptr){
								int pdgRICH = mcTrackFromG->GetPdgCode();
								int motherIdRICH = mcTrackFromG->GetMotherId();
								double momentumRICH = mcTrackFromG->GetP();
								if(TMath::Abs(pdgRICH) == 11 && motherIdRICH != -1){
									CbmMCTrack* mcTrackMotherRICH = (CbmMCTrack*) fMcTracks->At(motherIdRICH);
									if (mcTrackMotherRICH != nullptr && mcTrackMotherRICH->GetPdgCode() == 22 && mcTrackMotherRICH->GetMotherId() != -1){
										CbmMCTrack* mcTrackMotherMotherRICH = (CbmMCTrack*) fMcTracks->At(mcTrackMotherRICH->GetMotherId());
										if (mcTrackMotherMotherRICH->GetPdgCode() == DecayedParticlePdg){
											float xConv = mcTrackFromG->GetStartX();
											float yConv = mcTrackFromG->GetStartY();
											float zConv = mcTrackFromG->GetStartZ();
											//cout << "pdg in RICH from gamma =  " << mcTrackFromG->GetPdgCode() << "; momentum of gamma = " << mcTrackMotherRICH->GetP() << "; Pt = " <<
											//mcTrackMotherRICH->GetPt()<< endl;
											//cout << "*** *** *** Here was lepton from GAMMA in RICH without STS" << endl;
											//cout << "pdg in RICH from gamma =  " << mcTrackFromG->GetPdgCode() << "; momentum = " << mcTrackFromG->GetP() << "; Pt = " << mcTrackFromG->GetPt() 
											//<< " --> "  << " xConv = " << xConv << "; yConv = " << yConv << "; zConv = " << zConv << " <-- Coordinates of start vertex [cm]" << endl;
											candidateFromGammawithoutSTS++;
											ConversionPoints->Fill(zConv, xConv, yConv);
											ConversionPoints2D->Fill(zConv, yConv);
											///  -->  here without dR calculation, because I didn't want to copy huge part of code from below <-- ///
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	///////   Dalitz analysis --> check if we have rings from conversion, which tracks were not registered in STS (END)
	// ========================================================================================
*/





	if(DoReconstruction) {
//		fAnaReco->SetTracklistReco(fTracklist_noRichInd_mcTrack, fTracklist_noRichInd_MCindex, fTracklist_noRichInd_gTrackId, fTracklist_noRichInd_refmomentum_all);
		fAnaReco->SetTracklistReco(fTracklist_noRichInd_mcTrack, fTracklist_withRichInd_mcTrack,  fTracklist_noRichInd_refmomentum_all, fTracklist_withRichInd_refmomentum_all, DecayedParticlePdg, fEventNum-1, fTracklist_noRichInd_mcTrack_STS_Id);

	}














//cout << "NofReconstructedInSTS = " << NofReconstructedInSTS << "; ConversionCandidateSTS = " << ConversionCandidateSTS << "; PrimaryCandidateSTS" << PrimaryCandidateSTS << "" << endl;
/*	if (ConversionCandidateSTS == 2){
		Double_t inv_mass_mc = Invmass_2particles(fTracklist_noRichInd_conversion.at(0), fTracklist_noRichInd_conversion.at(1));
//		cout << "inv_mass_mc = " << inv_mass_mc <<  endl;
	}*/

/*	if (NofReconstructedInSTS == 4 ){//&& ConversionCandidateSTS == 2 && PrimaryCandidateSTS == 2 && fTracklist_noRichInd_refmomentum_all.size() == 4){
		cout << "CbmAnaConversion2, event No. " <<  fEventNum-1 << endl;
		Double_t inv_mass_mc_4 = Invmass_4particles(fTracklist_noRichInd_mcTrack.at(0), fTracklist_noRichInd_mcTrack.at(1), fTracklist_noRichInd_mcTrack.at(2), fTracklist_noRichInd_mcTrack.at(3));
		cout << "HERE IS 4:       inv_mass_mc_4_pure = " << inv_mass_mc_4 <<  endl;
		Double_t inv_mass_mc_4_reco = Invmass_4particlesRECO(fTracklist_noRichInd_refmomentum_all.at(0), fTracklist_noRichInd_refmomentum_all.at(1), fTracklist_noRichInd_refmomentum_all.at(2), fTracklist_noRichInd_refmomentum_all.at(3));
		cout << "HERE IS 4_reco:  inv_mass_mc_4_reco = " << inv_mass_mc_4_reco <<  endl;
		cout << " " << endl;
		Invariant_masses->Fill(inv_mass_mc_4_reco*1000, inv_mass_mc_4*1000);
	}*/







	// ========================================================================================
/*	if (NofReconstructedInSTS == LeptonsInDecay){
		counter_full_ind_STS++;
		if (NofReconstructedInRICH == LeptonsInDecay){
			counter_full_ind_RICH++;
		}
	}*/

//	eff->Fill(PrimaryCandidateSTS, PrimaryCandidateRICH);
	//cout << "PrimaryCandidateSTS = " << PrimaryCandidateSTS << "; PrimaryCandidateRICH = " << PrimaryCandidateRICH << "; ConversionCandidateSTS = " << ConversionCandidateSTS << "; ConversionCandidateRICH = " << ConversionCandidateRICH <<"; candidateFromGammawithoutSTS = " << candidateFromGammawithoutSTS << endl;
	//cout << " " << endl;
	//cout << " " << endl;
	//cout << " " << endl;
	// ========================================================================================
}

void CbmAnaConversion2::FitAndFillHistEllipse(
      int histIndex,
      CbmRichRingLight* ring)
{
   fTauFit->DoFit(ring);
}

Double_t CbmAnaConversion2::Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2)
// calculation of invariant mass from four electrons/positrons
{
	TLorentzVector lorVec1;
	mctrack1->Get4Momentum(lorVec1);
    
	TLorentzVector lorVec2;
	mctrack2->Get4Momentum(lorVec2); 

	TLorentzVector sum;
	sum = lorVec1 + lorVec2; 

	return sum.Mag();
}

Double_t CbmAnaConversion2::Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4)
// calculation of invariant mass from four electrons/positrons
{
	TLorentzVector lorVec1;
	mctrack1->Get4Momentum(lorVec1);
    
	TLorentzVector lorVec2;
	mctrack2->Get4Momentum(lorVec2);
    
	TLorentzVector lorVec3;
	mctrack3->Get4Momentum(lorVec3);
    
	TLorentzVector lorVec4;
	mctrack4->Get4Momentum(lorVec4);

	TLorentzVector sum;
	sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;    

	return sum.Mag();
}

Double_t CbmAnaConversion2::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
// calculation of invariant mass from four electrons/positrons
{
	Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
	TLorentzVector lorVec1(part1, energy1);

	Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
	TLorentzVector lorVec2(part2, energy2);

	Double_t energy3 = TMath::Sqrt(part3.Mag2() + M2E);
	TLorentzVector lorVec3(part3, energy3);

	Double_t energy4 = TMath::Sqrt(part4.Mag2() + M2E);
	TLorentzVector lorVec4(part4, energy4);

	TLorentzVector sum;
	sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;    

	return sum.Mag();
}

void CbmAnaConversion2::FillMCTracklists(CbmMCTrack* mctrack, int)
// fill all relevant tracklists containing MC tracks
{
	Bool_t electrons = true;
	Bool_t gammas = true;


	// fill all gamma tracklist
	if(TMath::Abs( mctrack->GetPdgCode()) == 22 && gammas) {
		fMCTracklist_gamma_all.push_back(mctrack);
	}


	// fill all elpositrons tracklist
	if(TMath::Abs( mctrack->GetPdgCode()) == 11 && electrons) {
		fMCTracklist_elpositrons_all.push_back(mctrack);
	}


	// fill elpositron tracklists coming from gamma
	if(TMath::Abs( mctrack->GetPdgCode()) == 11 && electrons) { 
		int motherId = mctrack->GetMotherId();
		if (motherId != -1 || motherId == -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (nullptr != mother) mcMotherPdg = mother->GetPdgCode();
			if (mcMotherPdg == 22) {
				fMCTracklist_elpositrons_from_gamma.push_back(mctrack);
			}
		}
	}


	// fill gamma tracklist coming from our particle
	if(TMath::Abs( mctrack->GetPdgCode()) == 22 && gammas) {
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (nullptr != mother) mcMotherPdg = mother->GetPdgCode();
			if (mcMotherPdg == DecayedParticlePdg) {	// pdg code 111 = pi0
				fMCTracklist_gamma_from_our_particle.push_back(mctrack);
			}
		}
	}


	// fill elpositron tracklists coming from our patricle
	if(TMath::Abs( mctrack->GetPdgCode()) == 11 && electrons) { 
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (nullptr != mother) mcMotherPdg = mother->GetPdgCode();
			if (mcMotherPdg == DecayedParticlePdg) {
				fMCTracklist_elpositrons_from_our_particle.push_back(mctrack);
			}
		}
	}


	// fill elpositron tracklists coming from gamma from our patricle
	if(TMath::Abs( mctrack->GetPdgCode()) == 11 && electrons) { 
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (nullptr != mother) mcMotherPdg = mother->GetPdgCode();
			if (mcMotherPdg == 22) {
				int grandmotherId = mother->GetMotherId();
				if(grandmotherId != -1 || grandmotherId == -1) {
					CbmMCTrack* grandmother = (CbmMCTrack*) fMcTracks->At(grandmotherId);
					int mcGrandmotherPdg = -1;
					if (nullptr != grandmother) mcGrandmotherPdg = grandmother->GetPdgCode();
					if(mcGrandmotherPdg == 111) {
						fMCTracklist_elpositrons_from_gamma_from_our_particle.push_back(mctrack);
					}
				}
			}
		}
	}
}


TVector3 CbmAnaConversion2::STSReffit(CbmStsTrack* stsTrack)
{
		// Doing refit of momenta with electron assumption
		CbmL1PFFitter fPFFitter_electron;
		vector<CbmStsTrack> stsTracks_electron;
		stsTracks_electron.resize(1);
		stsTracks_electron[0] = *stsTrack;
		vector<L1FieldRegion> vField_electron;
		vector<float> chiPrim_electron;
		vector<int> pidHypo_electron;
		pidHypo_electron.push_back(11);
		fPFFitter_electron.Fit(stsTracks_electron, pidHypo_electron); 
		fPFFitter_electron.GetChiToVertex(stsTracks_electron, vField_electron, chiPrim_electron, fKFVertex, 3e6);
		TVector3 refittedMomentum_electron;
		TVector3 refittedPosition_electron;
		const FairTrackParam* vtxTrack_electron = stsTracks_electron[0].GetParamFirst();
		vtxTrack_electron->Momentum(refittedMomentum_electron);
		vtxTrack_electron->Position(refittedPosition_electron);
//		STSmomenta->Fill(refittedMomentum_electron.Perp(),mcTrack1->GetPt());
	return refittedMomentum_electron;
}

TVector3 CbmAnaConversion2::FitToVertexSTS(CbmStsTrack* stsTrack, CbmMCTrack* mcTrack)
{
	CbmVertex* vtx = new CbmVertex();
	TMatrixFSym* covMat = new TMatrixFSym(3);
	vtx->SetVertex(mcTrack->GetStartX(), mcTrack->GetStartY(), mcTrack->GetStartZ(), 0, 0, 0, *covMat);

/*    TEST extract first hit from the track and make a fit to this vertex
	Int_t stsHitIndex;
	Int_t mvdHitIndex;
	CbmStsHit* stsHit;
	CbmMvdHit* mvdHit;
	if(stsTrack->GetNofMvdHits() != 0){
		mvdHitIndex = stsTrack->GetMvdHitIndex(0);
		mvdHit = (CbmMvdHit*) fArrayMvdHit->At(mvdHitIndex);
		vtx->SetVertex(mvdHit->GetX(), mvdHit->GetY(), mvdHit->GetZ()-5, 0, 0, 0, *covMat);
	} else if (stsTrack->GetNofStsHits() != 0){
		stsHitIndex = stsTrack->GetStsHitIndex(0);
		stsHit = (CbmStsHit*) fArrayStsHit->At(stsHitIndex);
		vtx->SetVertex(stsHit->GetX(), stsHit->GetY(), stsHit->GetZ()-9, 0, 0, 0, *covMat);
	}*/

	CbmStsKFTrackFitter fitter;
	FairTrackParam neu_track;
//	fitter.FitToVertex(stsTrack, fPrimVertex, &neu_track);
	fitter.FitToVertex(stsTrack, vtx, &neu_track);

	TVector3 position;
	TVector3 momentum;

	neu_track.Position(position);
	neu_track.Momentum(momentum);

	return momentum;
}




void CbmAnaConversion2::SetKF(CbmKFParticleFinder* kfparticle, CbmKFParticleFinderQA* kfparticleQA)
{
	fKFparticle = kfparticle;
	fKFparticleFinderQA = kfparticleQA;
	if(fKFparticle) {
		cout << "kf works" << endl;
	}
	else {
		cout << "kf works not" << endl;
	}
}


void CbmAnaConversion2::Finish()
{
	gDirectory->mkdir("conversion2");
	gDirectory->cd("conversion2");


	if(DoReconstruction)	{ fAnaReco->Finish(); }
	if(DoKFAnalysis)		{ fAnaKF->Finish(); }
	if(DoFitter)			{ fAnaFitter->Finish(); }
	if(DoManualAnalysis)	{ fAnaManual->Finish(); }



	BoA_electrons->SetMaximum(1.);
	BoA_electrons->SetMinimum(0.8);
	A_electrons->SetMaximum(5.5);
	A_electrons->SetMinimum(4.);
	B_electrons->SetMaximum(5.);
	B_electrons->SetMinimum(4.);
	hprof2d_electrons->SetMaximum(0.35);
	hprof2d_electrons->SetMinimum(0.15);

	for (Int_t i = 0; i < fHistoList.size(); i++){
		fHistoList[i]->Write();
	}
	gDirectory->cd("..");

//	TCanvas* c3 = new TCanvas();
//	Invariant_masses->Draw();

	cout << "counter_full_ind_STS = " << counter_full_ind_STS << endl; 
	cout << "counter_full_ind_RICH = " << counter_full_ind_RICH << endl;
	cout << "eff = " << counter_full_ind_RICH/counter_full_ind_STS << endl; 

	cout << "                                 " << endl;
	cout << "END... we wil continue soon... " <<  fEventNum << endl;
}

ClassImp(CbmAnaConversion2)



/**
 * \file CbmAnaConversion.cxx
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2014
 **/

#include "CbmAnaConversion.h"

#include "TH1D.h"
#include "TH1.h"
#include "TH3.h"
#include "TCanvas.h"
#include "TClonesArray.h"

#include "CbmMCTrack.h"
#include "FairTrackParam.h"
#include "CbmRichHit.h"
#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmRichPoint.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"

#include "CbmUtils.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>


#include "L1Field.h"
#include "CbmL1PFFitter.h"
#include "CbmStsKFTrackFitter.h"

#include "TRandom3.h"




#define M2E 2.6112004954086e-7

using namespace std;
using boost::assign::list_of;

CbmAnaConversion::CbmAnaConversion()
  : FairTask("CbmAnaConversion"),
    fhGammaZ(NULL),
    fTest(NULL),
    fhNofElPrim(NULL),
    fhNofElSec(NULL),
    fhNofElAll(NULL),
    fhElectronSources(NULL),
    fhNofPi0_perEvent(NULL),
    fhNofPi0_perEvent_cut(NULL),
    fhPi0_z(NULL),
    fhPi0_z_cut(NULL),
    fhElectronsFromPi0_z(NULL),
    fhTomography(NULL),
    fhTomography_XZ(NULL),
    fhTomography_YZ(NULL),
    fhTomography_uptoRICH(NULL),
    fhTomography_RICH_complete(NULL),
    fhTomography_RICH_beampipe(NULL),
    fhTomography_STS_end(NULL),
    fhTomography_STS_lastStation(NULL),    
    fhTomography_RICH_frontplate(NULL),    
    fhTomography_RICH_backplate(NULL), 
    fhTomography_reco(NULL),
    fhTomography_reco_XZ(NULL),
    fhTomography_reco_YZ(NULL),
    fhConversion(NULL),
    fhConversion_inSTS(NULL),
    fhConversion_prob(NULL),
    fhConversion_energy(NULL),
    fhConversion_p(NULL),
    fhInvariantMass_test(NULL),
    fhInvariantMass_test2(NULL),
    fhInvariantMass_test3(NULL),
    fhInvariantMassReco_test(NULL),
    fhInvariantMassReco_test2(NULL),
    fhInvariantMassReco_test3(NULL),
    fhInvariantMass_MC_all(NULL),
    fhInvariantMass_MC_omega(NULL),
    fhInvariantMass_MC_pi0(NULL),
    fhInvariantMass_MC_eta(NULL),
    fhInvariantMassReco_pi0(NULL),
    fhMomentum_MCvsReco(NULL),     
    fhMomentum_MCvsReco_diff(NULL),   
    fhInvMass_EPEM_mc(NULL),
    fhInvMass_EPEM_stsMomVec(NULL),
    fhInvMass_EPEM_refitted(NULL),
    fhInvMass_EPEM_error_stsMomVec(NULL),
    fhInvMass_EPEM_error_refitted(NULL),
    fhUsedMomenta_stsMomVec(NULL),
    fhUsedMomenta_mc(NULL),
    fhUsedMomenta_error_stsMomVec(NULL),
    fhUsedMomenta_error_refitted(NULL),
    fhUsedMomenta_errorX_stsMomVec(NULL),
    fhUsedMomenta_errorY_stsMomVec(NULL),
    fhUsedMomenta_errorZ_stsMomVec(NULL),
    fhUsedMomenta_vsX_stsMomVec(NULL),
    fhUsedMomenta_vsY_stsMomVec(NULL),
    fhUsedMomenta_vsZ_stsMomVec(NULL),
    fhUsedMomenta_errorX_refitted(NULL),
    fhUsedMomenta_errorY_refitted(NULL),
    fhUsedMomenta_errorZ_refitted(NULL),
    fhUsedMomenta_vsX_refitted(NULL),
    fhUsedMomenta_vsY_refitted(NULL),
    fhUsedMomenta_vsZ_refitted(NULL),
    fhInvariantMass_pi0epem(NULL),
    fhPi0_startvertex(NULL),
    fhSearchGammas(NULL),
    fhMCtest(NULL),
    fRichPoints(NULL),
    fMcTracks(NULL),
    fStsTracks(NULL),
    fStsTrackMatches(NULL),
    fGlobalTracks(NULL),
    fPrimVertex(NULL),
    fKFVertex(),
    fEventNum(0),
    test(0),
    testint(0),
    fAnalyseMode(0),
    fHistoList(),
    fHistoList_MC(),
    fHistoList_tomography(),
    fHistoList_reco(),
    fMCTracklist(),
    fMCTracklist_all(),
    fRecoTracklist(),
    fRecoTracklistEPEM(),
    fRecoMomentum(),
    fRecoRefittedMomentum()  
{
}

CbmAnaConversion::~CbmAnaConversion()
{

}

InitStatus CbmAnaConversion::Init()
{
	cout << "CbmAnaConversion::Init"<<endl;
	FairRootManager* ioman = FairRootManager::Instance();
	if (NULL == ioman) { Fatal("CbmAnaConversion::Init","RootManager not instantised!"); }

	fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
	if ( NULL == fRichPoints) { Fatal("CbmAnaConversion::Init","No RichPoint array!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( NULL == fMcTracks) { Fatal("CbmAnaConversion::Init","No MCTrack array!"); }

	fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	if ( NULL == fStsTracks) { Fatal("CbmAnaConversion::Init","No StsTrack array!"); }

	fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
	if (NULL == fStsTrackMatches) { Fatal("CbmAnaConversion::Init","No StsTrackMatch array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (NULL == fGlobalTracks) { Fatal("CbmAnaConversion::Init","No GlobalTrack array!"); }

	fPrimVertex = (CbmVertex*) ioman->GetObject("PrimaryVertex");
	if (NULL == fPrimVertex) { Fatal("CbmAnaConversion::Init","No PrimaryVertex array!"); }

	InitHistograms();
   
     
   
	testint = 0;
	test = 0;


	return kSUCCESS;
}

void CbmAnaConversion::InitHistograms()
{
	fHistoList.clear();
	fHistoList_MC.clear();
	fHistoList_tomography.clear();
	fHistoList_reco.clear();
	
	fhGammaZ = new TH1D("fhGammaZ", "fhGammaZ;Z [cm];Entries", 300, 0.0, 300.);
	fHistoList.push_back(fhGammaZ);
	
	fTest = new TH2D("fTest", "fTest; X; Y", 200, -100, 100, 400, -200, 200);
	fHistoList.push_back(fTest);
	
	
	fhNofElPrim				= new TH1D("fhNofElPrim", "fhNofElPrim;Nof prim El;Entries", 10., -0.5, 9.5);
	fhNofElSec				= new TH1D("fhNofElSec", "fhNofElSec;Nof Sec El;Entries", 20., -0.5, 19.5);
	fhNofElAll				= new TH1D("fhNofElAll", "fhNofElAll;Nof All El;Entries", 30., -0.5, 29.5);
	fhNofPi0_perEvent		= new TH1D("fhNofPi0_perEvent", "fhNofPi0_perEvent;Nof pi0;Entries", 300., -0.5, 599.5);
	fhNofPi0_perEvent_cut	= new TH1D("fhNofPi0_perEvent_cut", "fhNofPi0_perEvent_cut;Nof pi0;Entries", 300., -0.5, 599.5);
	fhPi0_z					= new TH1D("fhPi0_z", "fhPi0_z;z [cm];Entries", 600., -0.5, 599.5);
	fhPi0_z_cut				= new TH1D("fhPi0_z_cut", "fhPi0_z_cut;z [cm];Entries", 600., -0.5, 599.5);
	fhElectronSources		= new TH1D("fhElectronSources", "fhElectronSources;Source;Entries", 5., 0., 5.);
	fhElectronsFromPi0_z	= new TH1D("fhElectronsFromPi0_z", "fhElectronsFromPi0_z (= pos. of gamma conversion);z [cm];Entries", 600., -0.5, 599.5);
	fHistoList.push_back(fhNofPi0_perEvent);
	fHistoList.push_back(fhNofPi0_perEvent_cut);
	fHistoList.push_back(fhPi0_z);
	fHistoList.push_back(fhPi0_z_cut);
	fHistoList.push_back(fhElectronSources);
	fHistoList.push_back(fhElectronsFromPi0_z);
	
	fhElectronSources->GetXaxis()->SetBinLabel(1, "gamma");
	fhElectronSources->GetXaxis()->SetBinLabel(2, "pi0");
	fhElectronSources->GetXaxis()->SetBinLabel(3, "eta");
	fhElectronSources->GetXaxis()->SetBinLabel(4, "gamma from pi0");
	fhElectronSources->GetXaxis()->SetBinLabel(5, "gamma from eta");
	
	
	// for all events including gammas and gamma-conversion
	fhTomography	= new TH3D("fhTomography", "Tomography/fhTomography;X [cm];Y [cm];Z [cm]", 200, -200., 200., 200, -200., 200., 500, 0., 1000.);
	fhTomography_XZ	= new TH2D("fhTomography_XZ", "fhTomography_XZ;X [cm];Z [cm]", 2400, -600., 600., 2200, 0., 1100.);
	fhTomography_YZ	= new TH2D("fhTomography_YZ", "fhTomography_YZ;Y [cm];Z [cm]", 2000, -500., 500., 2200, 0., 1100.);
	fHistoList_tomography.push_back(fhTomography);
	fHistoList_tomography.push_back(fhTomography_XZ);
	fHistoList_tomography.push_back(fhTomography_YZ);
	
	fhTomography_uptoRICH			= new TH2D("fhTomography_uptoRICH", "fhTomography_uptoRICH;X [cm];Y [cm]", 400, -200., 200., 400, -200., 200.);
	fhTomography_RICH_complete		= new TH2D("fhTomography_RICH_complete", "fhTomography_RICH_complete;X [cm];Y [cm]", 400, -200., 200., 400, -200., 200.);
	fhTomography_RICH_beampipe		= new TH2D("fhTomography_RICH_beampipe", "fhTomography_RICH_beampipe;X [cm];Y [cm]", 400, -100., 100., 200, -50., 50.);
	fhTomography_STS_end			= new TH2D("fhTomography_STS_end", "fhTomography_STS_end;X [cm];Y [cm]", 400, -200., 200., 200, -100., 100.);
	fhTomography_STS_lastStation	= new TH2D("fhTomography_STS_lastStation", "fhTomography_STS_lastStation;X [cm];Y [cm]", 200, -100., 100., 200, -100., 100.);
	fhTomography_RICH_frontplate	= new TH2D("fhTomography_RICH_frontplate", "fhTomography_RICH_frontplate;X [cm];Y [cm]", 400, -200., 200., 400, -200., 200.);
	fhTomography_RICH_backplate		= new TH2D("fhTomography_RICH_backplate", "fhTomography_RICH_backplate;X [cm];Y [cm]", 400, -200., 200., 400, -200., 200.);
	fHistoList_tomography.push_back(fhTomography_uptoRICH);
	fHistoList_tomography.push_back(fhTomography_RICH_complete);
	fHistoList_tomography.push_back(fhTomography_RICH_beampipe);
	fHistoList_tomography.push_back(fhTomography_STS_end);
	fHistoList_tomography.push_back(fhTomography_STS_lastStation);
	fHistoList_tomography.push_back(fhTomography_RICH_frontplate);
	fHistoList_tomography.push_back(fhTomography_RICH_backplate);
	
	
	// tomography from reconstructed tracks
	fhTomography_reco		= new TH3D("fhTomography_reco", "fhTomography_reco;X [cm];Y [cm];Z [cm]", 200, -200., 200., 200, -200., 200., 500, 0., 1000.);
	fhTomography_reco_XZ	= new TH2D("fhTomography_reco_XZ", "fhTomography_reco_XZ;X [cm];Z [cm]", 1600, -400., 400., 2400, 0., 1200.);
	fhTomography_reco_YZ	= new TH2D("fhTomography_reco_YZ", "fhTomography_reco_YZ;Y [cm];Z [cm]", 1600, -400., 400., 2400, 0., 1200.);
	fHistoList.push_back(fhTomography_reco);
	fHistoList.push_back(fhTomography_reco_XZ);
	fHistoList.push_back(fhTomography_reco_YZ);
	
	
	
	fhConversion		= new TH1D("fhConversion", "fhConversion;Z [cm];# conversions", 4800, -0.5, 1199.5);
	fhConversion_inSTS	= new TH1D("fhConversion_inSTS", "fhConversion in STS;Z [cm];# conversions", 800, -0.5, 199.5);
	fhConversion_prob	= new TH1D("fhConversion_prob", "fhConversion_prob;Z [cm];# conversions", 1200, -0.5, 1199.5);
	fHistoList.push_back(fhConversion);
	fHistoList.push_back(fhConversion_inSTS);
	fHistoList.push_back(fhConversion_prob);
	
	fhConversion_energy	= new TH1D("fhConversion_energy", "fhConversion_energy;energy;#", 1000, 0., 100.);
	fhConversion_p		= new TH1D("fhConversion_p", "fhConversion_p;p;#", 1000, 0., 100.);
	fHistoList.push_back(fhConversion_energy);
	fHistoList.push_back(fhConversion_p);
	
	
	// for UrQMD events (invariant mass from pi0 -> gamma + gamma
	fhInvariantMass_test	= new TH1D("fhInvariant", "fhInvariant;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_test2	= new TH1D("fhInvariant2", "fhInvariant2;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_test3	= new TH1D("fhInvariant3", "fhInvariant3;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList.push_back(fhInvariantMass_test);
	fHistoList.push_back(fhInvariantMass_test2);
	fHistoList.push_back(fhInvariantMass_test3);
	
	fhInvariantMassReco_test	= new TH1D("fhInvariantReco", "fhInvariantReco;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMassReco_test2	= new TH1D("fhInvariantReco2", "fhInvariantReco2;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMassReco_test3	= new TH1D("fhInvariantReco3", "fhInvariantReco3;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList.push_back(fhInvariantMassReco_test);
	fHistoList.push_back(fhInvariantMassReco_test2);
	fHistoList.push_back(fhInvariantMassReco_test3);
	
	fhInvariantMassReco_pi0 = new TH1D("fhInvariantReco_pi0", "fhInvariantReco_pi0;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList.push_back(fhInvariantMassReco_pi0);
	
	fhInvariantMass_MC_all		= new TH1D("fhInvariantMass_MC_all", "fhInvariantMass_MC_all;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_MC_omega	= new TH1D("fhInvariantMass_MC_omega", "fhInvariantMass_MC_omega;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_MC_pi0		= new TH1D("fhInvariantMass_MC_pi0", "fhInvariantMass_MC_pi0;mass [GeV/c^2];#", 2000, 0., 2.);
	fhInvariantMass_MC_eta		= new TH1D("fhInvariantMass_MC_eta", "fhInvariantMass_MC_eta;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList_MC.push_back(fhInvariantMass_MC_all);
	fHistoList_MC.push_back(fhInvariantMass_MC_omega);
	fHistoList_MC.push_back(fhInvariantMass_MC_pi0);
	fHistoList_MC.push_back(fhInvariantMass_MC_eta);
	
	
	fhMCtest	= new TH1D("fhMCtest", "fhMCtest;mass [GeV/c^2];#", 2000, 0., 2.);
	fHistoList_MC.push_back(fhMCtest);
	
	
	// for reconstructed tracks
	fhMomentum_MCvsReco			= new TH2D("fhMomentum_MCvsReco","fhMomentum_MCvsReco;MC;Reco", 400, 0., 40., 400, 0., 40.); 
	fhMomentum_MCvsReco_diff	= new TH1D("fhMomentum_MCvsReco_diff","fhMomentum_MCvsReco_diff;(MC-Reco)/MC", 500, -0.01, 4.); 
	fHistoList.push_back(fhMomentum_MCvsReco);
	fHistoList.push_back(fhMomentum_MCvsReco_diff);
	
	fhInvMass_EPEM_mc				= new TH1D("fhInvMass_EPEM_mc","fhInvariantMass_recoMomentum1 (mc);mass;#", 400, -0.0025, 1.9975);
	fhInvMass_EPEM_stsMomVec		= new TH1D("fhInvMass_EPEM_stsMomVec","fhInvariantMass_recoMomentum2 (stsMomentumVec);mass;#", 200, -0.005, 1.995);
	fhInvMass_EPEM_refitted			= new TH1D("fhInvMass_EPEM_refitted","fhInvariantMass_recoMomentum3 (refitted at primary);mass;#", 200, -0.005, 1.995);
	fhInvMass_EPEM_error_stsMomVec	= new TH1D("fhInvMass_EPEM_error_stsMomVec","fhInvariantMass_recoMomentum4 (error, stsMomentumVec);(mc-reco)/mc;#", 500, -0.005, 4.995);
	fhInvMass_EPEM_error_refitted	= new TH1D("fhInvMass_EPEM_error_refitted","fhInvariantMass_recoMomentum5 (error, refitted);(mc-reco)/mc;#", 500, -0.005, 4.995);
	fhUsedMomenta_stsMomVec			= new TH1D("fhUsedMomenta_stsMomVec","fhMomentumtest1;momentum;#", 100, 0., 2.);
	fhUsedMomenta_mc				= new TH1D("fhUsedMomenta_mc","fhMomentumtest2;momentum;#", 100, 0., 2.);
	fhUsedMomenta_error_stsMomVec	= new TH1D("fhUsedMomenta_error_stsMomVec","fhMomentumtest3 (error);(mc-reco)/mc;#", 400, -2.005, 1.995);
	fhUsedMomenta_error_refitted	= new TH1D("fhUsedMomenta_error_refitted","fhMomentumtest4 (error);(mc-reco_refitted)/mc;#", 400, -2.005, 1.995);
	
	fhUsedMomenta_errorX_stsMomVec	= new TH1D("fhUsedMomenta_errorX_stsMomVec","fhMomentumtest5 (error of x-momentum);(mc-reco_reco)/mc;#", 401, -4.01, 4.01);
	fhUsedMomenta_vsX_stsMomVec		= new TH2D("fhUsedMomenta_vsX_stsMomVec","fhMomentumtest5vs (error of x-momentum);mc;reco", 101, -1.01, 1.01, 101, -1.01, 1.01);
	fhUsedMomenta_errorY_stsMomVec	= new TH1D("fhUsedMomenta_errorY_stsMomVec","fhMomentumtest6 (error of y-momentum);(mc-reco_reco)/mc;#", 401, -4.01, 4.01);
	fhUsedMomenta_vsY_stsMomVec		= new TH2D("fhUsedMomenta_vsY_stsMomVec","fhMomentumtest6vs (error of y-momentum);mc;reco", 101, -1.01, 1.01, 101, -1.01, 1.01);
	fhUsedMomenta_errorZ_stsMomVec	= new TH1D("fhUsedMomenta_errorZ_stsMomVec","fhMomentumtest7 (error of z-momentum);(mc-reco_reco)/mc;#", 401, -4.01, 4.01);
	fhUsedMomenta_vsZ_stsMomVec		= new TH2D("fhUsedMomenta_vsZ_stsMomVec","fhMomentumtest7vs (error of z-momentum);mc;reco", 201, -0.01, 4.01, 201, -0.01, 4.01);
	fhUsedMomenta_errorX_refitted	= new TH1D("fhUsedMomenta_errorX_refitted","fhMomentumtest5 (error of x-momentum);(mc-reco_reco)/mc;#", 401, -4.01, 4.01);
	fhUsedMomenta_vsX_refitted		= new TH2D("fhUsedMomenta_vsX_refitted","fhMomentumtest5vs (error of x-momentum);mc;reco", 101, -1.01, 1.01, 101, -1.01, 1.01);
	fhUsedMomenta_errorY_refitted	= new TH1D("fhUsedMomenta_errorY_refitted","fhMomentumtest6 (error of y-momentum);(mc-reco_reco)/mc;#", 401, -4.01, 4.01);
	fhUsedMomenta_vsY_refitted		= new TH2D("fhUsedMomenta_vsY_refitted","fhMomentumtest6vs (error of y-momentum);mc;reco", 101, -1.01, 1.01, 101, -1.01, 1.01);
	fhUsedMomenta_errorZ_refitted	= new TH1D("fhUsedMomenta_errorZ_refitted","fhMomentumtest7 (error of z-momentum);(mc-reco_reco)/mc;#", 401, -4.01, 4.01);
	fhUsedMomenta_vsZ_refitted		= new TH2D("fhUsedMomenta_vsZ_refitted","fhMomentumtest7vs (error of z-momentum);mc;reco", 201, -0.01, 4.01, 201, -0.01, 4.01);
	fHistoList_reco.push_back(fhInvMass_EPEM_mc);
	fHistoList_reco.push_back(fhInvMass_EPEM_stsMomVec);
	fHistoList_reco.push_back(fhInvMass_EPEM_refitted);
	fHistoList_reco.push_back(fhInvMass_EPEM_error_stsMomVec);
	fHistoList_reco.push_back(fhInvMass_EPEM_error_refitted);
	fHistoList_reco.push_back(fhUsedMomenta_stsMomVec);
	fHistoList_reco.push_back(fhUsedMomenta_mc);
	fHistoList_reco.push_back(fhUsedMomenta_error_stsMomVec);
	fHistoList_reco.push_back(fhUsedMomenta_error_refitted);
	fHistoList_reco.push_back(fhUsedMomenta_errorX_stsMomVec);		// error of x-component of reconstructed momentum
	fHistoList_reco.push_back(fhUsedMomenta_errorY_stsMomVec);		// error of y-component of reconstructed momentum
	fHistoList_reco.push_back(fhUsedMomenta_errorZ_stsMomVec);		// error of z-component of reconstructed momentum
	fHistoList_reco.push_back(fhUsedMomenta_vsX_stsMomVec);	// x-component of reconstructed momentum vs mc-momentum
	fHistoList_reco.push_back(fhUsedMomenta_vsY_stsMomVec);	// y-component of reconstructed momentum vs mc-momentum
	fHistoList_reco.push_back(fhUsedMomenta_vsZ_stsMomVec);	// z-component of reconstructed momentum vs mc-momentum
	fHistoList_reco.push_back(fhUsedMomenta_errorX_refitted);		// error of x-component of reconstructed momentum
	fHistoList_reco.push_back(fhUsedMomenta_errorY_refitted);		// error of y-component of reconstructed momentum
	fHistoList_reco.push_back(fhUsedMomenta_errorZ_refitted);		// error of z-component of reconstructed momentum
	fHistoList_reco.push_back(fhUsedMomenta_vsX_refitted);	// x-component of reconstructed momentum vs mc-momentum
	fHistoList_reco.push_back(fhUsedMomenta_vsY_refitted);	// y-component of reconstructed momentum vs mc-momentum
	fHistoList_reco.push_back(fhUsedMomenta_vsZ_refitted);	// z-component of reconstructed momentum vs mc-momentum
	
	fhInvariantMass_pi0epem = new TH1D("fhInvariantMass_pi0epem","fhInvariantMass_pi0epem;mass;#", 400, 0., 2.);
	fHistoList_reco.push_back(fhInvariantMass_pi0epem);
	
	fhPi0_startvertex = new TH1D("fhPi0_startvertex","fhPi0_startvertex;z[cm];#", 200, -0.25, 99.75);
	fHistoList_reco.push_back(fhPi0_startvertex);
	
	
	fhSearchGammas = new TH1D("fhSearchGammas","fhSearchGammas;mass;#", 100, -0.005, 0.995);
	fHistoList.push_back(fhSearchGammas);

}



void CbmAnaConversion::Exec(Option_t* option)
{

//	const KFParticleTopoReconstructor * recokon = fKFparticle->GetTopoReconstructor();
//	const int test = recokon->NPrimaryVertices();
//	cout << "test kf: " << test << endl;


	cout << "===================================================" << endl;
   cout << "CbmAnaConversion, event No. " <<  fEventNum << endl;
   fEventNum++;

   fMCTracklist.clear();
   fMCTracklist_all.clear();
   fRecoTracklist.clear();
   fRecoTracklistEPEM.clear();
   fRecoMomentum.clear();
   fRecoRefittedMomentum.clear();
   
   int countPrimEl = 0;
   int countSecEl = 0;
   int countAllEl  = 0;
   int countGammaEl = 0;
   int countMothers = 0;
   int countPrimPart = 0;   
   int countPi0MC = 0; 
   int countPi0MC_cut = 0;
   int countPi0MC_fromPrimary = 0;
   
	if (fPrimVertex != NULL){
		fKFVertex = CbmKFVertex(*fPrimVertex);
	} else {
		Fatal("CbmAnaConversion::Exec","No PrimaryVertex array!");
	}
	
	



   Int_t nofPoints = fRichPoints->GetEntriesFast();
   for (int i = 0; i < nofPoints; i++) {
	   CbmRichPoint* point = (CbmRichPoint*)fRichPoints->At(i);
	   if (point == NULL) continue;
	   //cout << point->GetX() << "\t" << point->GetY() << endl;
	   fTest->Fill(point->GetX(), point->GetY());
   }
   

	// ========================================================================================
	// START - Analyse MC tracks
	Int_t nofMcTracks = fMcTracks->GetEntriesFast();
	for (int i = 0; i < nofMcTracks; i++) {

		CbmMCTrack* mctrack = (CbmMCTrack*)fMcTracks->At(i);
		if (mctrack == NULL) continue;   
   
		TomographyMC(mctrack); 		// analyse gamma-conversions with MC data
		FillMCTracklists(mctrack);	// fill tracklists for further analyses

  
		if (mctrack->GetMotherId() == -1) {
			countPrimPart++;
		}   
		if (mctrack->GetMotherId() == -1 && TMath::Abs( mctrack->GetPdgCode()) == 11) {
			countPrimEl++;
		}   
		if (mctrack->GetMotherId() != -1 && TMath::Abs( mctrack->GetPdgCode()) == 11) {
			countSecEl++;
		}   
		if (TMath::Abs( mctrack->GetPdgCode()) == 11) {
			countAllEl++;
		}  
		if (mctrack->GetPdgCode() == 111) {
			countPi0MC++;
			TVector3 v;
			mctrack->GetStartVertex(v);
			if(v.Z() <= 10) {
				countPi0MC_cut++;
			}
			fhPi0_z->Fill(v.Z());
			Double_t r2 = v.Z()*v.Z() * tan(25./180*TMath::Pi()) * tan(25./180*TMath::Pi());
			if( (v.X()*v.X() + v.Y()*v.Y()) <= r2) {
				fhPi0_z_cut->Fill(v.Z());
			}
			
			int motherId = mctrack->GetMotherId();
			if (motherId == -1) countPi0MC_fromPrimary++;
		}


		if (TMath::Abs( mctrack->GetPdgCode())  == 11) {
			AnalyseElectrons(mctrack);
		}

	}

	cout << "Number of pi0 in MC sample: " << countPi0MC << endl;
	cout << "Number of pi0 from primary: " << countPi0MC_fromPrimary << endl;
	fhNofPi0_perEvent->Fill(countPi0MC);
	fhNofPi0_perEvent_cut->Fill(countPi0MC_cut);

	InvariantMassTest();
	InvariantMassMC_all();

	ReconstructGamma();

   
	fhNofElPrim->Fill(countPrimEl);
	fhNofElSec->Fill(countSecEl);
	fhNofElAll->Fill(countAllEl);


	// END - Analyse MC tracks
	// ========================================================================================
   
   
   
	// ========================================================================================
	// START - Analyse reconstructed tracks
	Int_t ngTracks = fGlobalTracks->GetEntriesFast();
	for (Int_t i = 0; i < ngTracks; i++) {
       CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(i);
       if(NULL == gTrack) continue;
       int stsInd = gTrack->GetStsTrackIndex();
       int richInd = gTrack->GetRichRingIndex();
       int trdInd = gTrack->GetTrdTrackIndex();
       int tofInd = gTrack->GetTofHitIndex();

       if (stsInd < 0) continue;
       CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
       if (stsTrack == NULL) continue;
       CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*) fStsTrackMatches->At(stsInd);
       if (stsMatch == NULL) continue;
       int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
       if (stsMcTrackId < 0) continue;
       CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
       if (mcTrack1 == NULL) continue;
       int pdg = TMath::Abs(mcTrack1->GetPdgCode());
       int motherId = mcTrack1->GetMotherId();
       double momentum = mcTrack1->GetP();
       stsMatch->GetTrueOverAllHitsRatio();
       
       TomographyReco(mcTrack1);
       FillRecoTracklist(mcTrack1);
       
       
       TVector3 stsMomentumVec; 
       stsTrack->GetParamFirst()->Momentum(stsMomentumVec);
       Double_t stsMomentum = stsMomentumVec.Mag();
       
       TVector3 mcMomentumVec;
       mcTrack1->GetMomentum(mcMomentumVec);
       Double_t mcMomentum = mcMomentumVec.Mag();
       fhMomentum_MCvsReco->Fill(mcMomentum, stsMomentum);
       fhMomentum_MCvsReco_diff->Fill(TMath::Abs(mcMomentum-stsMomentum)/mcMomentum);
       
       TVector3 bothtogether;
       bothtogether.SetX(mcMomentumVec.X());
       bothtogether.SetY(stsMomentumVec.Y());
       bothtogether.SetZ(stsMomentumVec.Z());
       

		// calculate refitted momenta at primary vertex
		TVector3 refittedMomentum;
		CbmL1PFFitter fPFFitter;
		vector<CbmStsTrack> stsTracks;
		stsTracks.resize(1);
		stsTracks[0] = *stsTrack;
		vector<L1FieldRegion> vField;
		vector<float> chiPrim;
		fPFFitter.GetChiToVertex(stsTracks, vField, chiPrim, fKFVertex, 3e6);
		//cand.chi2sts = stsTracks[0].GetChiSq() / stsTracks[0].GetNDF();
		//cand.chi2Prim = chiPrim[0];
		const FairTrackParam* vtxTrack = stsTracks[0].GetParamFirst();
		vtxTrack->Momentum(refittedMomentum);
       
       
		FillRecoTracklistEPEM(mcTrack1, stsMomentumVec, refittedMomentum);
       
	}
//	InvariantMassTestReco();
	InvariantMassTest_4epem();
	
	// END - analyse reconstructed tracks
	// ========================================================================================
   

   // example of how to get refitted momenta at primary vertex
  /*
   CbmL1PFFitter fPFFitter;
   vector<CbmStsTrack> stsTracks;
   stsTracks.resize(1);
   stsTracks[0] = *stsTrack;
   vector<L1FieldRegion> vField;
   vector<float> chiPrim;
   fPFFitter.GetChiToVertex(stsTracks, vField, chiPrim, fKFVertex, 3e6);
   cand.chi2sts = stsTracks[0].GetChiSq() / stsTracks[0].GetNDF();
   cand.chi2Prim = chiPrim[0];
   const FairTrackParam* vtxTrack = stsTracks[0].GetParamFirst();

   vtxTrack->Position(cand.position);
   vtxTrack->Momentum(cand.momentum);*/


// =========================================================================================================================
// ============================================== END - EXEC function ======================================================
// =========================================================================================================================
}



void CbmAnaConversion::Finish()
{
	//Probability();

/*	
	TCanvas* c = new TCanvas();
	c->SetWindowSize(800, 1600);
	DrawH1(fTest);
	//fhGammaZ->Write();
*/
	
	// Write histograms to a file
	gDirectory->mkdir("analysis-conversion");
	gDirectory->cd("analysis-conversion");

	gDirectory->mkdir("MC data");
	gDirectory->cd("MC data");
	for (Int_t i = 0; i < fHistoList_MC.size(); i++){
		fHistoList_MC[i]->Write();
	}
	gDirectory->cd("..");

	gDirectory->mkdir("Tomography");
	gDirectory->cd("Tomography");
	for (Int_t i = 0; i < fHistoList_tomography.size(); i++){
		fHistoList_tomography[i]->Write();
	}
	gDirectory->cd("..");

	gDirectory->mkdir("Reconstruction");
	gDirectory->cd("Reconstruction");
	for (Int_t i = 0; i < fHistoList_reco.size(); i++){
		fHistoList_reco[i]->Write();
	}
	gDirectory->cd("..");

	for (Int_t i = 0; i < fHistoList.size(); i++){
		fHistoList[i]->Write();
	}

	gDirectory->cd("..");
// =========================================================================================================================
// ============================================== END - FINISH function ====================================================
// =========================================================================================================================
}





void CbmAnaConversion::AnalyseElectrons(CbmMCTrack* mctrack)
{
	int motherId = mctrack->GetMotherId();
	if (motherId == -1) return;
	CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
	int mcMotherPdg  = -1;
	if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
	//cout << mcMotherPdg << endl;

	if(mcMotherPdg == 22) {
		fhElectronSources->Fill(0);
		int grandmotherId = mother->GetMotherId();
		if (grandmotherId == -1) return;
		CbmMCTrack* grandmother = (CbmMCTrack*) fMcTracks->At(grandmotherId);
		int mcGrandmotherPdg  = -1;
		if (NULL != grandmother) mcGrandmotherPdg = grandmother->GetPdgCode();
		if(mcGrandmotherPdg == 111) fhElectronSources->Fill(3);
		if(mcGrandmotherPdg == 221) fhElectronSources->Fill(4);

		if(mcGrandmotherPdg == 111) {
			TVector3 v;
			mctrack->GetStartVertex(v);
			fhElectronsFromPi0_z->Fill(v.Z());
		}

	}
	if(mcMotherPdg == 111) fhElectronSources->Fill(1);
	if(mcMotherPdg == 221) fhElectronSources->Fill(2);

	if (mcMotherPdg == 22) {
		TVector3 v;
		mctrack->GetStartVertex(v);
		fhGammaZ->Fill(v.Z());
	//	countGammaEl++;
	}
}




KinematicParams CbmAnaConversion::CalculateKinematicParams(const CbmMCTrack* mctrackP, const CbmMCTrack* mctrackM)
{
    KinematicParams params;

    TVector3 momP;  //momentum e+
    mctrackP->GetMomentum(momP);
    Double_t energyP = TMath::Sqrt(momP.Mag2() + M2E);
    TLorentzVector lorVecP(momP, energyP);

    TVector3 momM;  //momentum e-
    mctrackM->GetMomentum(momM);
    Double_t energyM = TMath::Sqrt(momM.Mag2() + M2E);
    TLorentzVector lorVecM(momM, energyM);

    TVector3 momPair = momP + momM;
    Double_t energyPair = energyP + energyM;
    Double_t ptPair = momPair.Perp();
    Double_t pzPair = momPair.Pz();
    Double_t yPair = 0.5*TMath::Log((energyPair+pzPair)/(energyPair-pzPair));
    Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
    Double_t theta = 180.*anglePair/TMath::Pi();
    Double_t minv = 2.*TMath::Sin(anglePair / 2.)*TMath::Sqrt(momM.Mag()*momP.Mag());

    params.momentumMag = momPair.Mag();
    params.pt = ptPair;
    params.rapidity = yPair;
    params.minv = minv;
    params.angle = theta;
    return params;
}

Double_t CbmAnaConversion::Invmass_2gammas(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2) 
// calculation of invariant mass from two gammas (m=0)
{
    TVector3 mom1;
    mctrack1->GetMomentum(mom1);
    Double_t energy1 = TMath::Sqrt(mom1.Mag2());
    TLorentzVector lorVec1(mom1, energy1);

    TVector3 mom2;
    mctrack2->GetMomentum(mom2);
    Double_t energy2 = TMath::Sqrt(mom2.Mag2());
    TLorentzVector lorVec2(mom2, energy2);
    
    TLorentzVector sum = lorVec1 + lorVec2;
	return sum.Mag();
}




Double_t CbmAnaConversion::Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2) 
// calculation of invariant mass from two electrons/positrons
{
    TVector3 mom1;
    mctrack1->GetMomentum(mom1);
    Double_t energy1 = TMath::Sqrt(mom1.Mag2() + M2E);
    TLorentzVector lorVec1(mom1, energy1);

    TVector3 mom2;
    mctrack2->GetMomentum(mom2);
    Double_t energy2 = TMath::Sqrt(mom2.Mag2() + M2E);
    TLorentzVector lorVec2(mom2, energy2);
    
    TLorentzVector sum = lorVec1 + lorVec2;
	return sum.Mag();
}


Double_t CbmAnaConversion::SmearValue(Double_t value) 
{
	TRandom3 generator(0);
	Double_t result = 0;
	Double_t smear = 0;
	Int_t plusminus = 0;
	while (plusminus == 0) { // should be either 1 or -1, not 0
		plusminus = generator.Uniform(-2, 2);
	}
	Double_t gaus = generator.Gaus(1,1);
	smear = gaus * plusminus;
//	result = value * (1. + 1.0*smear/100);		//smearing as wished
	
	result = value;		// -> no smearing
	
	return result;
}


Double_t CbmAnaConversion::Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4)
// calculation of invariant mass from four electrons/positrons
{
    TVector3 mom1;
    mctrack1->GetMomentum(mom1);
    TVector3 tempmom1;
    tempmom1.SetX(SmearValue(mom1.X()));
    tempmom1.SetY(SmearValue(mom1.Y()));
    tempmom1.SetZ(SmearValue(mom1.Z()));
    Double_t energy1 = TMath::Sqrt(tempmom1.Mag2() + M2E);
    TLorentzVector lorVec1(tempmom1, energy1);

    TVector3 mom2;
    mctrack2->GetMomentum(mom2);
    TVector3 tempmom2;
    tempmom2.SetX(SmearValue(mom2.X()));
    tempmom2.SetY(SmearValue(mom2.Y()));
    tempmom2.SetZ(SmearValue(mom2.Z()));
    Double_t energy2 = TMath::Sqrt(tempmom2.Mag2() + M2E);
    TLorentzVector lorVec2(tempmom2, energy2);

    TVector3 mom3;
    mctrack3->GetMomentum(mom3);
    TVector3 tempmom3;
    tempmom3.SetX(SmearValue(mom3.X()));
    tempmom3.SetY(SmearValue(mom3.Y()));
    tempmom3.SetZ(SmearValue(mom3.Z()));
    Double_t energy3 = TMath::Sqrt(tempmom3.Mag2() + M2E);
    TLorentzVector lorVec3(tempmom3, energy3);

    TVector3 mom4;
    mctrack4->GetMomentum(mom4);
    TVector3 tempmom4;
    tempmom4.SetX(SmearValue(mom4.X()));
    tempmom4.SetY(SmearValue(mom4.Y()));
    tempmom4.SetZ(SmearValue(mom4.Z()));
    Double_t energy4 = TMath::Sqrt(tempmom4.Mag2() + M2E);
    TLorentzVector lorVec4(tempmom4, energy4);
    
/*    TLorentzVector lorVec1;
    mctrack1->Get4Momentum(lorVec1);
    
    TLorentzVector lorVec2;
    mctrack2->Get4Momentum(lorVec2);
    
    TLorentzVector lorVec3;
    mctrack3->Get4Momentum(lorVec3);
    
    TLorentzVector lorVec4;
    mctrack4->Get4Momentum(lorVec4);
*/    
    
    TLorentzVector sum;
    sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;
    cout << "mc: \t" << sum.Px() << " / " << sum.Py() << " / " << sum.Pz() << " / " << sum.E() << "\t => mag = " << sum.Mag() << endl;
	return sum.Mag();
}




Double_t CbmAnaConversion::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
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
    cout << "reco: \t" << sum.Px() << " / " << sum.Py() << " / " << sum.Pz() << " / " << sum.E() << "\t => mag = " << sum.Mag() << endl;
	return sum.Mag();
}


// =========================================================================================================================
// ============================================== UP TO HERE: Invariant mass calculation ===================================
// =========================================================================================================================




void CbmAnaConversion::TomographyMC(CbmMCTrack* mctrack)
// doing tomography from gamma -> e+ e- decays, MC DATA
{
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) {
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (NULL != mother) mcMotherPdg = mother->GetPdgCode();

			if (mcMotherPdg == 22) {
				TVector3 v;
				mctrack->GetStartVertex(v);
				fhGammaZ->Fill(v.Z());
		
				fhTomography->Fill(v.X(), v.Y(), v.Z());
				fhTomography_XZ->Fill(v.X(), v.Z());
				fhTomography_YZ->Fill(v.Y(), v.Z());
				fhConversion_energy->Fill(mctrack->GetEnergy());
				fhConversion_p->Fill(mctrack->GetP());

				if(v.z() >= 0 && v.Z() <= 170) { // only in region before the RICH
					fhTomography_uptoRICH->Fill(v.X(), v.Y());
				}
				if(v.z() >= 170 && v.Z() <= 400) { // only in region of the RICH detector
					fhTomography_RICH_complete->Fill(v.X(), v.Y());
				}
				if(v.z() >= 220 && v.Z() <= 280) { // only in region of RICH beampipe, without being distorted by mirrors or camera
					fhTomography_RICH_beampipe->Fill(v.X(), v.Y());
				}
				if(v.z() >= 100 && v.Z() <= 170) { // only in a downstream part of STS and magnet
					fhTomography_STS_end->Fill(v.X(), v.Y());
				}
				if(v.z() >= 98 && v.Z() <= 102) { // only in a downstream part of STS and magnet, only last STS station
					fhTomography_STS_lastStation->Fill(v.X(), v.Y());
				}
				if(v.z() >= 179 && v.Z() <= 181) { // only in region of RICH frontplate
					fhTomography_RICH_frontplate->Fill(v.X(), v.Y());
				}
				if(v.z() >= 369 && v.Z() <= 371) { // only in region of RICH backplate
					fhTomography_RICH_backplate->Fill(v.X(), v.Y());
				}

				fhConversion->Fill(v.Z());
				
				if( (TMath::Abs(v.X()) <= 100 && TMath::Abs(v.X()) > 3) && (TMath::Abs(v.Y()) <= 50 && TMath::Abs(v.Y()) > 3) && TMath::Abs(v.Z()) <= 169) {
					fhConversion_inSTS->Fill(v.Z());
				}
			}
		}
	}
}


void CbmAnaConversion::TomographyReco(CbmMCTrack* mctrack)
// doing tomography from gamma -> e+ e- decays, RECONSTRUCTED TRACKS WITH MC DATA
{
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) {
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (NULL != mother) mcMotherPdg = mother->GetPdgCode();

			if (mcMotherPdg == 22) {
				TVector3 v;
				mctrack->GetStartVertex(v);
			  
				fhTomography_reco->Fill(v.X(), v.Y(), v.Z());
				fhTomography_reco_XZ->Fill(v.X(), v.Z());
				fhTomography_reco_YZ->Fill(v.Y(), v.Z());
			}
		}
	}
}




void CbmAnaConversion::Probability() 
{
	for(int i=1; i<=fhConversion->GetNbinsX(); i++) {
		int temp = 0;
		for(int run=1; run<=i; run++ ) {
			temp += fhConversion->GetBinContent(run);
		}
		fhConversion_prob->SetBinContent(i, temp);
	}
}


// =========================================================================================================================
// ============================================== UP TO HERE: Tomography related ===========================================
// =========================================================================================================================




void CbmAnaConversion::FillMCTracklists(CbmMCTrack* mctrack)
// fill all relevant tracklists containing MC tracks
{
	Bool_t electrons = true;
	Bool_t gammas = true;
	
	// fill gamma tracklist
	if(TMath::Abs( mctrack->GetPdgCode()) == 22 && gammas) {
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			int mcMotherPdg  = -1;
			if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
			if (mcMotherPdg == 111) {	// pdg code 111 = pi0
				fMCTracklist.push_back(mctrack);
			}
		}
	}

	// fill electron tracklists
	if(TMath::Abs( mctrack->GetPdgCode()) == 11 && electrons) { 
		TVector3 v;
		mctrack->GetStartVertex(v);
		if(v.Z() <= 70) {
			int motherId = mctrack->GetMotherId();
			if (motherId != -1 || motherId == -1) {
				CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
				int mcMotherPdg  = -1;
				if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
				if (mcMotherPdg == 22 || mcMotherPdg == 111 || mcMotherPdg == 221) {
					int grandmotherId = mother->GetMotherId();
					if(grandmotherId != -1 || grandmotherId == -1) {
					//	CbmMCTrack* grandmother = (CbmMCTrack*) fMcTracks->At(grandmotherId);
					//	int mcGrandmotherPdg = -1;
					//	if (NULL != grandmother) mcGrandmotherPdg = grandmother->GetPdgCode();
					//	if(mcGrandmotherPdg == 111) {
							fMCTracklist_all.push_back(mctrack);
					//	}
					}
				}
			}
		}
	}
}





void CbmAnaConversion::FillRecoTracklist(CbmMCTrack* mctrack) 
{
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) { 
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
		CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
		int mcMotherPdg  = -1;
		if (NULL != mother) mcMotherPdg = mother->GetPdgCode();
		if (mcMotherPdg == 111 || mcMotherPdg == 22) {	// pdg code 111 = pi0, 22 = gamma
			fRecoTracklist.push_back(mctrack);
		//	cout << "pdg " << mctrack->GetPdgCode() << "\t motherid " << motherId << endl;
			test++;
		}
		}
	}
}




void CbmAnaConversion::FillRecoTracklistEPEM(CbmMCTrack* mctrack, TVector3 stsMomentum, TVector3 refittedMom) 
{
	if (TMath::Abs( mctrack->GetPdgCode())  == 11) { 
		int motherId = mctrack->GetMotherId();
		if (motherId != -1) {
			fRecoTracklistEPEM.push_back(mctrack);
			fRecoMomentum.push_back(stsMomentum);
			fRecoRefittedMomentum.push_back(refittedMom);
		}
	}
}




void CbmAnaConversion::CalculateInvMass_MC_2particles()
{
	for(int i=0; i<fMCTracklist_all.size(); i++) {
		for(int j=i+1; j<fMCTracklist_all.size(); j++) {
			Double_t invmass = Invmass_2particles(fMCTracklist_all[i],fMCTracklist_all[j]);
			
			int motherId_i = fMCTracklist_all[i]->GetMotherId();
			int motherId_j = fMCTracklist_all[j]->GetMotherId();
			
			CbmMCTrack* mother_i = (CbmMCTrack*) fMcTracks->At(motherId_i);
			int mcMotherPdg_i  = -1;
			if (NULL != mother_i) mcMotherPdg_i = mother_i->GetPdgCode();
		
			CbmMCTrack* mother_j = (CbmMCTrack*) fMcTracks->At(motherId_j);
			int mcMotherPdg_j  = -1;
			if (NULL != mother_j) mcMotherPdg_j = mother_j->GetPdgCode();
			
			if(motherId_i == motherId_j && ( (fMCTracklist_all[i]->GetPdgCode() == 11 && fMCTracklist_all[j]->GetPdgCode() == -11) || (fMCTracklist_all[i]->GetPdgCode() == -11 && fMCTracklist_all[j]->GetPdgCode() == 11))) {
			//	fhInvariantMass_MC_all->Fill(invmass);
				//cout << "e+e- decay detected! MotherId " << motherId_i << "\t invariant mass: " << invmass << endl;
				
				if(mcMotherPdg_i == 223) {
					fhInvariantMass_MC_omega->Fill(invmass);
					cout << "-- mother particle of decay: omega" << endl;
				}
				if(mcMotherPdg_i == 111) {
					fhInvariantMass_MC_pi0->Fill(invmass);
					cout << "-- mother particle of decay: pi0" << endl;
				}
			}
		}
	}



}



void CbmAnaConversion::InvariantMassTest() 
// calculation of invariant mass via pi0-> gamma gamma, ONLY FROM MC DATA!
{
	for(int i=0; i<fMCTracklist.size(); i++) {
		for(int j=i+1; j<fMCTracklist.size(); j++) {
			
			//if(fMCTracklist[i]->GetPx() != fMCTracklist[j]->GetPx() && fMCTracklist[i]->GetPy() != fMCTracklist[j]->GetPy()) {
			Double_t invmass = Invmass_2gammas(fMCTracklist[i],fMCTracklist[j]);
			fhInvariantMass_test->Fill(invmass);
			TVector3 vi;
			fMCTracklist[i]->GetStartVertex(vi);
			TVector3 vj;
			fMCTracklist[j]->GetStartVertex(vj);
			
			int motherId_i = fMCTracklist[i]->GetMotherId();
			int motherId_j = fMCTracklist[j]->GetMotherId();
			
			if(motherId_i == motherId_j) {
				fhInvariantMass_test2->Fill(invmass);
				
				if(invmass < 0.001) {
				//	cout << "gamma1 " << fMCTracklist[i]->GetPx() << "\t" << fMCTracklist[i]->GetPy() << "\t" << fMCTracklist[i]->GetPz() << endl;
				//	cout << "gamma2 " << fMCTracklist[j]->GetPx() << "\t" << fMCTracklist[j]->GetPy() << "\t" << fMCTracklist[j]->GetPz() << endl;
				}
			}
			if(vi.Z() == vj.Z() && vi.Y() == vj.Y() && vi.X() == vj.X()) {
				fhInvariantMass_test3->Fill(invmass);
			}
			//}
		}
	}
}





void CbmAnaConversion::InvariantMassTest_4epem()
// Calculating invariant mass of 4 ep/em, using MC data AND reconstructed momentum
{
	cout << "4epem start" << endl;
	cout << fRecoTracklistEPEM.size() << "\t" << fRecoMomentum.size() << endl;
	int fill = 0;
	for(int i=0; i<fRecoTracklistEPEM.size(); i++) {
		for(int j=i+1; j<fRecoTracklistEPEM.size(); j++) {
			for(int k=j+1; k<fRecoTracklistEPEM.size(); k++) {
				for(int l=k+1; l<fRecoTracklistEPEM.size(); l++) {
					if(fRecoTracklistEPEM[i]->GetPdgCode() + fRecoTracklistEPEM[j]->GetPdgCode() + fRecoTracklistEPEM[k]->GetPdgCode() + fRecoTracklistEPEM[l]->GetPdgCode() != 0) continue;
					
					if(fRecoTracklistEPEM.size() != fRecoMomentum.size()) {
						cout << "not matching number of entries!" << endl;
						continue;
					}
					
					
					int motherId1 = fRecoTracklistEPEM[i]->GetMotherId();
					int motherId2 = fRecoTracklistEPEM[j]->GetMotherId();
					int motherId3 = fRecoTracklistEPEM[k]->GetMotherId();
					int motherId4 = fRecoTracklistEPEM[l]->GetMotherId();
					
					if(motherId1 == motherId2 && motherId1 == motherId3 && motherId1 == motherId4) {
						if (motherId1 != -1) {
							int mcMotherPdg1  = -1;
							CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
							if (NULL != mother1) mcMotherPdg1 = mother1->GetPdgCode();
							if( (mcMotherPdg1 == 111 || mcMotherPdg1 == 221) && NofDaughters(motherId1) == 4) {
								Double_t invmass2 = 0;	// momenta from stsMomentumVec
							//	invmass2 = Invmass_4particlesRECO(fRecoMomentum[i], fRecoMomentum[j], fRecoMomentum[k], fRecoMomentum[l]);
								invmass2 = Invmass_4particlesRECO(fRecoRefittedMomentum[i], fRecoRefittedMomentum[j], fRecoRefittedMomentum[k], fRecoRefittedMomentum[l]);
							//	fhInvariantMass_pi0epem->Fill(invmass2);
								cout << "Decay pi0 -> e+e-e+e- detected!\t\t" << invmass2 << endl;
							}
						}
						else {
							continue;
						}
					}
					
					if( (motherId1 == motherId2 && motherId3 == motherId4) ||
						(motherId1 == motherId3 && motherId2 == motherId4) ||
						(motherId1 == motherId4 && motherId2 == motherId3) ) {
						
					
						int grandmotherId1 = -1;
						int grandmotherId2 = -1;
						int grandmotherId3 = -1;
						int grandmotherId4 = -1;
						
						int mcMotherPdg1  = -1;
						int mcMotherPdg2  = -1;
						int mcMotherPdg3  = -1;
						int mcMotherPdg4  = -1;
						int mcGrandmotherPdg1  = -1;
						int mcGrandmotherPdg2  = -1;
						int mcGrandmotherPdg3  = -1;
						int mcGrandmotherPdg4  = -1;
						
						CbmMCTrack* grandmother1;
						if (motherId1 != -1) {
							CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
							if (NULL != mother1) mcMotherPdg1 = mother1->GetPdgCode();
							grandmotherId1 = mother1->GetMotherId();
							if(grandmotherId1 != -1) {
								grandmother1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
								if (NULL != grandmother1) mcGrandmotherPdg1 = grandmother1->GetPdgCode();
							}
						}
						if (motherId2 != -1) {
							CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
							if (NULL != mother2) mcMotherPdg2 = mother2->GetPdgCode();
							grandmotherId2 = mother2->GetMotherId();
							if(grandmotherId2 != -1) {
								CbmMCTrack* grandmother2 = (CbmMCTrack*) fMcTracks->At(grandmotherId2);
								if (NULL != grandmother2) mcGrandmotherPdg2 = grandmother2->GetPdgCode();
							}
						}
						if (motherId3 != -1) {
							CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
							if (NULL != mother3) mcMotherPdg3 = mother3->GetPdgCode();
							grandmotherId3 = mother3->GetMotherId();
							if(grandmotherId3 != -1) {
								CbmMCTrack* grandmother3 = (CbmMCTrack*) fMcTracks->At(grandmotherId3);
								if (NULL != grandmother3) mcGrandmotherPdg3 = grandmother3->GetPdgCode();
							}
						}
						if (motherId4 != -1) {
							CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);
							if (NULL != mother4) mcMotherPdg4 = mother4->GetPdgCode();
							grandmotherId4 = mother4->GetMotherId();
							if(grandmotherId4 != -1) {
								CbmMCTrack* grandmother4 = (CbmMCTrack*) fMcTracks->At(grandmotherId4);
								if (NULL != grandmother4) mcGrandmotherPdg4 = grandmother4->GetPdgCode();
							}
						}
					
					//if( (motherId_i == motherId_j && motherId_k == motherId_l) ||
					//	(motherId_i == motherId_k && motherId_j == motherId_l) ||
					//	(motherId_i == motherId_l && motherId_j == motherId_k) )
					//	{
					
					if(grandmotherId1 == -1) continue;
					
					
					
						if(motherId1 == motherId2 && motherId3 == motherId4) {
							if(NofDaughters(motherId1) != 2 || NofDaughters(motherId3) != 2) continue;
							if( (grandmotherId1 == motherId3 && mcGrandmotherPdg1 == 111) || (motherId1 == grandmotherId3 && mcMotherPdg1 == 111)) {
								cout << "HURRAY! .-.-.-.-.-.-..-.-.-.-.-.-.-.-.-.-.-.-." << endl;
								Double_t invmass1 = 0;	// true MC values
								invmass1 = Invmass_4particles(fRecoTracklistEPEM[i], fRecoTracklistEPEM[j], fRecoTracklistEPEM[k], fRecoTracklistEPEM[l]);
								fhInvariantMass_pi0epem->Fill(invmass1);
							}
						}
						if(motherId1 == motherId3 && motherId2 == motherId4) {
							if(NofDaughters(motherId1) != 2 || NofDaughters(motherId2) != 2) continue;
							if( (grandmotherId1 == motherId2 && mcGrandmotherPdg1 == 111) || (motherId1 == grandmotherId2 && mcMotherPdg1 == 111)) {
								cout << "HURRAY! .-.-.-.-.-.-..-.-.-.-.-.-.-.-.-.-.-.-." << endl;
								Double_t invmass1 = 0;	// true MC values
								invmass1 = Invmass_4particles(fRecoTracklistEPEM[i], fRecoTracklistEPEM[j], fRecoTracklistEPEM[k], fRecoTracklistEPEM[l]);
								fhInvariantMass_pi0epem->Fill(invmass1);
							}
						}
						if(motherId1 == motherId4 && motherId2 == motherId3) {
							if(NofDaughters(motherId1) != 2 || NofDaughters(motherId2) != 2) continue;
							if( (grandmotherId1 == motherId2 && mcGrandmotherPdg1 == 111) || (motherId1 == grandmotherId2 && mcMotherPdg1 == 111)) {
								cout << "HURRAY! .-.-.-.-.-.-..-.-.-.-.-.-.-.-.-.-.-.-." << endl;
								Double_t invmass1 = 0;	// true MC values
								invmass1 = Invmass_4particles(fRecoTracklistEPEM[i], fRecoTracklistEPEM[j], fRecoTracklistEPEM[k], fRecoTracklistEPEM[l]);
								fhInvariantMass_pi0epem->Fill(invmass1);
							}
						}
					
					
					
					
					
					// ===================================================================================================
					// HERE DECAY pi0 -> gamma gamma -> e+e- e+e-
					if(grandmotherId1 == grandmotherId2 && grandmotherId1 == grandmotherId3 && grandmotherId1 == grandmotherId4) {
						if(mcGrandmotherPdg1 != 111 && mcGrandmotherPdg1 != 221) continue; // 111 = pi0, 221 = eta
						
						TVector3 pi0start;
						grandmother1->GetStartVertex(pi0start);
						fhPi0_startvertex->Fill(pi0start.Z());
						
						Double_t invmass1 = 0;	// true MC values
						invmass1 = Invmass_4particles(fRecoTracklistEPEM[i], fRecoTracklistEPEM[j], fRecoTracklistEPEM[k], fRecoTracklistEPEM[l]);
						Double_t invmass2 = 0;	// momenta from stsMomentumVec
						invmass2 = Invmass_4particlesRECO(fRecoMomentum[i], fRecoMomentum[j], fRecoMomentum[k], fRecoMomentum[l]);
						Double_t invmass3 = 0;	// momenta from refitted at primary vertex
						invmass3 = Invmass_4particlesRECO(fRecoRefittedMomentum[i], fRecoRefittedMomentum[j], fRecoRefittedMomentum[k], fRecoRefittedMomentum[l]);
						
						cout << "######################################################################" << endl;
						cout << fRecoMomentum[i].X() << "\t" << fRecoRefittedMomentum[i].X() << endl;
						cout << fRecoMomentum[i].Y() << "\t" << fRecoRefittedMomentum[i].Y() << endl;
						cout << fRecoMomentum[i].Z() << "\t" << fRecoRefittedMomentum[i].Z() << endl;
						
						
						cout << "######################################################################" << endl;
						cout << "index: " << i << "\t" << j << "\t" << k << "\t" << l << endl;
						cout << "motherid: " << motherId1 << "\t" << motherId2 << "\t" << motherId3 << "\t" << motherId4 << endl;
						cout << "motherpdg: " << mcMotherPdg1 << "\t" << mcMotherPdg2 << "\t" << mcMotherPdg3 << "\t" << mcMotherPdg4 << endl;
						cout << "grandmotherid: " << grandmotherId1 << "\t" << grandmotherId2 << "\t" << grandmotherId3 << "\t" << grandmotherId4 << endl;
						cout << "pdg: " << fRecoTracklistEPEM[i]->GetPdgCode() << "\t" << fRecoTracklistEPEM[j]->GetPdgCode() << "\t" << fRecoTracklistEPEM[k]->GetPdgCode() << "\t" << fRecoTracklistEPEM[l]->GetPdgCode() << endl;
						cout << "invmass reco: " << invmass2 << "\t invmass mc: " << invmass1 << endl;
					
						fhInvMass_EPEM_mc->Fill(invmass1);
						fhInvMass_EPEM_stsMomVec->Fill(invmass2);
						fhInvMass_EPEM_refitted->Fill(invmass3);
						fhInvMass_EPEM_error_stsMomVec->Fill(1.0*TMath::Abs(invmass1-invmass2)/invmass1);
						fhInvMass_EPEM_error_refitted->Fill(1.0*TMath::Abs(invmass1-invmass3)/invmass1);
					
						fhUsedMomenta_stsMomVec->Fill(fRecoMomentum[i].Mag());
						fhUsedMomenta_stsMomVec->Fill(fRecoMomentum[j].Mag());
						fhUsedMomenta_stsMomVec->Fill(fRecoMomentum[k].Mag());
						fhUsedMomenta_stsMomVec->Fill(fRecoMomentum[l].Mag());
					
						fhUsedMomenta_mc->Fill(fRecoTracklistEPEM[i]->GetP());
						fhUsedMomenta_mc->Fill(fRecoTracklistEPEM[j]->GetP());
						fhUsedMomenta_mc->Fill(fRecoTracklistEPEM[k]->GetP());
						fhUsedMomenta_mc->Fill(fRecoTracklistEPEM[l]->GetP());
					
						fhUsedMomenta_error_stsMomVec->Fill(TMath::Abs(fRecoTracklistEPEM[i]->GetP() - fRecoMomentum[i].Mag())/fRecoTracklistEPEM[i]->GetP());
						fhUsedMomenta_error_stsMomVec->Fill(TMath::Abs(fRecoTracklistEPEM[j]->GetP() - fRecoMomentum[j].Mag())/fRecoTracklistEPEM[j]->GetP());
						fhUsedMomenta_error_stsMomVec->Fill(TMath::Abs(fRecoTracklistEPEM[k]->GetP() - fRecoMomentum[k].Mag())/fRecoTracklistEPEM[k]->GetP());
						fhUsedMomenta_error_stsMomVec->Fill(TMath::Abs(fRecoTracklistEPEM[l]->GetP() - fRecoMomentum[l].Mag())/fRecoTracklistEPEM[l]->GetP());
					
					//	fhUsedMomenta_error_refitted->Fill(TMath::Abs(fRecoRefittedMomentum[i].Mag() - fRecoMomentum[i].Mag())/fRecoRefittedMomentum[i].Mag());
					//	fhUsedMomenta_error_refitted->Fill(TMath::Abs(fRecoRefittedMomentum[j].Mag() - fRecoMomentum[j].Mag())/fRecoRefittedMomentum[j].Mag());
					//	fhUsedMomenta_error_refitted->Fill(TMath::Abs(fRecoRefittedMomentum[k].Mag() - fRecoMomentum[k].Mag())/fRecoRefittedMomentum[k].Mag());
					//	fhUsedMomenta_error_refitted->Fill(TMath::Abs(fRecoRefittedMomentum[l].Mag() - fRecoMomentum[l].Mag())/fRecoRefittedMomentum[l].Mag());
		
						fhUsedMomenta_error_refitted->Fill(TMath::Abs(fRecoTracklistEPEM[i]->GetP() - fRecoRefittedMomentum[i].Mag())/fRecoTracklistEPEM[i]->GetP());
						fhUsedMomenta_error_refitted->Fill(TMath::Abs(fRecoTracklistEPEM[j]->GetP() - fRecoRefittedMomentum[j].Mag())/fRecoTracklistEPEM[j]->GetP());
						fhUsedMomenta_error_refitted->Fill(TMath::Abs(fRecoTracklistEPEM[k]->GetP() - fRecoRefittedMomentum[k].Mag())/fRecoTracklistEPEM[k]->GetP());
						fhUsedMomenta_error_refitted->Fill(TMath::Abs(fRecoTracklistEPEM[l]->GetP() - fRecoRefittedMomentum[l].Mag())/fRecoTracklistEPEM[l]->GetP());


						TVector3 momentumtest5a;
						fRecoTracklistEPEM[i]->GetMomentum(momentumtest5a);
						fhUsedMomenta_errorX_stsMomVec->Fill(TMath::Abs(momentumtest5a.X() - fRecoMomentum[i].X())/momentumtest5a.X());
						fhUsedMomenta_errorY_stsMomVec->Fill(TMath::Abs(momentumtest5a.Y() - fRecoMomentum[i].Y())/momentumtest5a.Y());
						fhUsedMomenta_errorZ_stsMomVec->Fill(TMath::Abs(momentumtest5a.Z() - fRecoMomentum[i].Z())/momentumtest5a.Z());
						fhUsedMomenta_vsX_stsMomVec->Fill(momentumtest5a.X(), fRecoMomentum[i].X());
						fhUsedMomenta_vsY_stsMomVec->Fill(momentumtest5a.Y(), fRecoMomentum[i].Y());
						fhUsedMomenta_vsZ_stsMomVec->Fill(momentumtest5a.Z(), fRecoMomentum[i].Z());
						fhUsedMomenta_errorX_refitted->Fill(TMath::Abs(momentumtest5a.X() - fRecoRefittedMomentum[i].X())/momentumtest5a.X());
						fhUsedMomenta_errorY_refitted->Fill(TMath::Abs(momentumtest5a.Y() - fRecoRefittedMomentum[i].Y())/momentumtest5a.Y());
						fhUsedMomenta_errorZ_refitted->Fill(TMath::Abs(momentumtest5a.Z() - fRecoRefittedMomentum[i].Z())/momentumtest5a.Z());
						fhUsedMomenta_vsX_refitted->Fill(momentumtest5a.X(), fRecoRefittedMomentum[i].X());
						fhUsedMomenta_vsY_refitted->Fill(momentumtest5a.Y(), fRecoRefittedMomentum[i].Y());
						fhUsedMomenta_vsZ_refitted->Fill(momentumtest5a.Z(), fRecoRefittedMomentum[i].Z());

						TVector3 momentumtest5b;
						fRecoTracklistEPEM[j]->GetMomentum(momentumtest5b);
						fhUsedMomenta_errorX_stsMomVec->Fill(TMath::Abs(momentumtest5b.X() - fRecoMomentum[j].X())/momentumtest5b.X());
						fhUsedMomenta_errorY_stsMomVec->Fill(TMath::Abs(momentumtest5b.Y() - fRecoMomentum[j].Y())/momentumtest5b.Y());
						fhUsedMomenta_errorZ_stsMomVec->Fill(TMath::Abs(momentumtest5b.Z() - fRecoMomentum[j].Z())/momentumtest5b.Z());
						fhUsedMomenta_vsX_stsMomVec->Fill(momentumtest5b.X(), fRecoMomentum[j].X());
						fhUsedMomenta_vsY_stsMomVec->Fill(momentumtest5b.Y(), fRecoMomentum[j].Y());
						fhUsedMomenta_vsZ_stsMomVec->Fill(momentumtest5b.Z(), fRecoMomentum[j].Z());
						fhUsedMomenta_errorX_refitted->Fill(TMath::Abs(momentumtest5b.X() - fRecoRefittedMomentum[j].X())/momentumtest5b.X());
						fhUsedMomenta_errorY_refitted->Fill(TMath::Abs(momentumtest5b.Y() - fRecoRefittedMomentum[j].Y())/momentumtest5b.Y());
						fhUsedMomenta_errorZ_refitted->Fill(TMath::Abs(momentumtest5b.Z() - fRecoRefittedMomentum[j].Z())/momentumtest5b.Z());
						fhUsedMomenta_vsX_refitted->Fill(momentumtest5b.X(), fRecoRefittedMomentum[j].X());
						fhUsedMomenta_vsY_refitted->Fill(momentumtest5b.Y(), fRecoRefittedMomentum[j].Y());
						fhUsedMomenta_vsZ_refitted->Fill(momentumtest5b.Z(), fRecoRefittedMomentum[j].Z());

						TVector3 momentumtest5c;
						fRecoTracklistEPEM[k]->GetMomentum(momentumtest5c);
						fhUsedMomenta_errorX_stsMomVec->Fill(TMath::Abs(momentumtest5c.X() - fRecoMomentum[k].X())/momentumtest5c.X());
						fhUsedMomenta_errorY_stsMomVec->Fill(TMath::Abs(momentumtest5c.Y() - fRecoMomentum[k].Y())/momentumtest5c.Y());
						fhUsedMomenta_errorZ_stsMomVec->Fill(TMath::Abs(momentumtest5c.Z() - fRecoMomentum[k].Z())/momentumtest5c.Z());
						fhUsedMomenta_vsX_stsMomVec->Fill(momentumtest5c.X(), fRecoMomentum[k].X());
						fhUsedMomenta_vsY_stsMomVec->Fill(momentumtest5c.Y(), fRecoMomentum[k].Y());
						fhUsedMomenta_vsZ_stsMomVec->Fill(momentumtest5c.Z(), fRecoMomentum[k].Z());
						fhUsedMomenta_errorX_refitted->Fill(TMath::Abs(momentumtest5c.X() - fRecoRefittedMomentum[k].X())/momentumtest5c.X());
						fhUsedMomenta_errorY_refitted->Fill(TMath::Abs(momentumtest5c.Y() - fRecoRefittedMomentum[k].Y())/momentumtest5c.Y());
						fhUsedMomenta_errorZ_refitted->Fill(TMath::Abs(momentumtest5c.Z() - fRecoRefittedMomentum[k].Z())/momentumtest5c.Z());
						fhUsedMomenta_vsX_refitted->Fill(momentumtest5c.X(), fRecoRefittedMomentum[k].X());
						fhUsedMomenta_vsY_refitted->Fill(momentumtest5c.Y(), fRecoRefittedMomentum[k].Y());
						fhUsedMomenta_vsZ_refitted->Fill(momentumtest5c.Z(), fRecoRefittedMomentum[k].Z());

						TVector3 momentumtest5d;
						fRecoTracklistEPEM[l]->GetMomentum(momentumtest5d);
						fhUsedMomenta_errorX_stsMomVec->Fill(TMath::Abs(momentumtest5d.X() - fRecoMomentum[l].X())/momentumtest5d.X());
						fhUsedMomenta_errorY_stsMomVec->Fill(TMath::Abs(momentumtest5d.Y() - fRecoMomentum[l].Y())/momentumtest5d.Y());
						fhUsedMomenta_errorZ_stsMomVec->Fill(TMath::Abs(momentumtest5d.Z() - fRecoMomentum[l].Z())/momentumtest5d.Z());
						fhUsedMomenta_vsX_stsMomVec->Fill(momentumtest5d.X(), fRecoMomentum[l].X());
						fhUsedMomenta_vsY_stsMomVec->Fill(momentumtest5d.Y(), fRecoMomentum[l].Y());
						fhUsedMomenta_vsZ_stsMomVec->Fill(momentumtest5d.Z(), fRecoMomentum[l].Z());
						fhUsedMomenta_errorX_refitted->Fill(TMath::Abs(momentumtest5d.X() - fRecoRefittedMomentum[l].X())/momentumtest5d.X());
						fhUsedMomenta_errorY_refitted->Fill(TMath::Abs(momentumtest5d.Y() - fRecoRefittedMomentum[l].Y())/momentumtest5d.Y());
						fhUsedMomenta_errorZ_refitted->Fill(TMath::Abs(momentumtest5d.Z() - fRecoRefittedMomentum[l].Z())/momentumtest5d.Z());
						fhUsedMomenta_vsX_refitted->Fill(momentumtest5d.X(), fRecoRefittedMomentum[l].X());
						fhUsedMomenta_vsY_refitted->Fill(momentumtest5d.Y(), fRecoRefittedMomentum[l].Y());
						fhUsedMomenta_vsZ_refitted->Fill(momentumtest5d.Z(), fRecoRefittedMomentum[l].Z());


						cout << "reco/mc: " << fRecoMomentum[i].Mag() << " / " << fRecoTracklistEPEM[i]->GetP() << " ### "  << fRecoMomentum[j].Mag() << " / " << fRecoTracklistEPEM[j]->GetP() << " ### "  << fRecoMomentum[k].Mag() << " / " << fRecoTracklistEPEM[k]->GetP() << " ### "  << fRecoMomentum[l].Mag() << " / " << fRecoTracklistEPEM[l]->GetP() << endl;


						fill++;
					}
					//if() {
					
					//}
					}
				}
			}
		}
	}
	cout << "Filled events: " << fill << endl;
	cout << "4epem end" << endl;
}











void CbmAnaConversion::InvariantMassTestReco() 
// calculation of invariant mass via pi0 -> .. -> e+ e- e+ e-, ONLY FROM RECONSTRUCTED TRACKS!
{
//	cout << "InvariantMassTestReco - Start..." << endl;
//	cout << "InvariantMassTestReco - Size of fRecoTracklist:\t " << fRecoTracklist.size() << endl;
	if(fRecoTracklist.size() >= 4) {
		for(int i=0; i<fRecoTracklist.size(); i++) {
			for(int j=i+1; j<fRecoTracklist.size(); j++) {
				for(int k=j+1; k<fRecoTracklist.size(); k++) {
					for(int l=k+1; l<fRecoTracklist.size(); l++) {
					
						if(fRecoTracklist[i]->GetPdgCode() + fRecoTracklist[j]->GetPdgCode() + fRecoTracklist[k]->GetPdgCode() + fRecoTracklist[l]->GetPdgCode() != 0) continue;
					
						int motherId1 = fRecoTracklist[i]->GetMotherId();
						int motherId2 = fRecoTracklist[j]->GetMotherId();
						int motherId3 = fRecoTracklist[k]->GetMotherId();
						int motherId4 = fRecoTracklist[l]->GetMotherId();
						int grandmotherId1 = -1;
						int grandmotherId2 = -1;
						int grandmotherId3 = -1;
						int grandmotherId4 = -1;
						
						int mcMotherPdg1  = -1;
						int mcMotherPdg2  = -1;
						int mcMotherPdg3  = -1;
						int mcMotherPdg4  = -1;
						int mcGrandmotherPdg1  = -1;
						int mcGrandmotherPdg2  = -1;
						int mcGrandmotherPdg3  = -1;
						int mcGrandmotherPdg4  = -1;
						
						
						if (motherId1 != -1) {
							CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
							if (NULL != mother1) mcMotherPdg1 = mother1->GetPdgCode();
							grandmotherId1 = mother1->GetMotherId();
							if(grandmotherId1 != -1) {
								CbmMCTrack* grandmother1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
								if (NULL != grandmother1) mcGrandmotherPdg1 = grandmother1->GetPdgCode();
							}
						}
						if (motherId2 != -1) {
							CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
							if (NULL != mother2) mcMotherPdg2 = mother2->GetPdgCode();
							grandmotherId2 = mother2->GetMotherId();
							if(grandmotherId2 != -1) {
								CbmMCTrack* grandmother2 = (CbmMCTrack*) fMcTracks->At(grandmotherId2);
								if (NULL != grandmother2) mcGrandmotherPdg2 = grandmother2->GetPdgCode();
							}
						}
						if (motherId3 != -1) {
							CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
							if (NULL != mother3) mcMotherPdg3 = mother3->GetPdgCode();
							grandmotherId3 = mother3->GetMotherId();
							if(grandmotherId3 != -1) {
								CbmMCTrack* grandmother3 = (CbmMCTrack*) fMcTracks->At(grandmotherId3);
								if (NULL != grandmother3) mcGrandmotherPdg3 = grandmother3->GetPdgCode();
							}
						}
						if (motherId4 != -1) {
							CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);
							if (NULL != mother4) mcMotherPdg4 = mother4->GetPdgCode();
							grandmotherId4 = mother4->GetMotherId();
							if(grandmotherId4 != -1) {
								CbmMCTrack* grandmother4 = (CbmMCTrack*) fMcTracks->At(grandmotherId4);
								if (NULL != grandmother4) mcGrandmotherPdg4 = grandmother4->GetPdgCode();
							}
						}
					
					
						if(motherId1 == motherId2 && motherId2 == motherId3 && motherId3 == motherId4) {
							Double_t invmass = Invmass_4particles(fRecoTracklist[i], fRecoTracklist[j], fRecoTracklist[k], fRecoTracklist[l]);
							fhInvariantMassReco_pi0->Fill(invmass);
					//		cout << "Decay pi0 -> e+ e- e+ e- detected!" << endl;
						}
						
						if(motherId1 == motherId2 && motherId1 == motherId3) {
						
						}
						if(motherId1 == motherId2 && motherId1 == motherId4) {
						
						}
						if(motherId1 == motherId3 && motherId1 == motherId4) {
						
						}
						if(motherId2 == motherId3 && motherId2 == motherId4) {
							
						}
						
					//	if(  ((motherId1 == motherId2 && motherId3 == motherId4) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg3 == 22 || mcMotherPdg3 == 111))
					//	  || ((motherId1 == motherId3 && motherId2 == motherId4) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg2 == 22 || mcMotherPdg2 == 111))
					//	  || ((motherId1 == motherId4 && motherId2 == motherId3) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg2 == 22 || mcMotherPdg2 == 111))) {
						if(  ((motherId1 == motherId2 && motherId3 == motherId4) && (mcMotherPdg1 == 22) && (mcMotherPdg3 == 22) && grandmotherId1 == grandmotherId3)
						  || ((motherId1 == motherId3 && motherId2 == motherId4) && (mcMotherPdg1 == 22) && (mcMotherPdg2 == 22) && grandmotherId1 == grandmotherId2)
						  || ((motherId1 == motherId4 && motherId2 == motherId3) && (mcMotherPdg1 == 22) && (mcMotherPdg2 == 22) && grandmotherId1 == grandmotherId2)) {
							Double_t invmass = Invmass_4particles(fRecoTracklist[i], fRecoTracklist[j], fRecoTracklist[k], fRecoTracklist[l]);
							fhInvariantMassReco_pi0->Fill(invmass);
					//		cout << "Decay pi0 -> 2gamma -> e+ e-! \t MotherId " << motherId1 << "\t" << motherId2 << "\t" << motherId3 << "\t" << motherId4 <<
					//			 "\t GrandmotherId " << grandmotherId1 << "\t" << grandmotherId2 << "\t" << grandmotherId3 << "\t" << grandmotherId4 << endl;
						
						}
					}
				}
			}
		}
	}
//	cout << "InvariantMassTestReco - End!" << endl;
}






int CbmAnaConversion::GetTest()
{
	return testint;
}


int CbmAnaConversion::GetNofEvents()
{
	return fEventNum;
}


void CbmAnaConversion::SetMode(int mode = 0)
	// mode 1 = tomography
	// mode 2 = urqmd
	// mode 3 = pluto
{
	fAnalyseMode = mode;
}




void CbmAnaConversion::InvariantMassMC_all() 
// calculation of invariant mass via pi0 -> .. -> e+ e- e+ e-, ONLY FROM RECONSTRUCTED TRACKS!
{
	cout << "InvariantMassTestMC - Start..." << endl;
	cout << "InvariantMassTestMC - Size of fTracklistMC_all:\t " << fMCTracklist_all.size() << endl;
	if(fMCTracklist_all.size() >= 4) {
		for(int i=0; i<fMCTracklist_all.size(); i++) {
			for(int j=i+1; j<fMCTracklist_all.size(); j++) {
				for(int k=j+1; k<fMCTracklist_all.size(); k++) {
					for(int l=k+1; l<fMCTracklist_all.size(); l++) {
					
						if(fMCTracklist_all[i]->GetPdgCode() + fMCTracklist_all[j]->GetPdgCode() + fMCTracklist_all[k]->GetPdgCode() + fMCTracklist_all[l]->GetPdgCode() != 0) continue;
					
						int motherId1 = fMCTracklist_all[i]->GetMotherId();
						int motherId2 = fMCTracklist_all[j]->GetMotherId();
						int motherId3 = fMCTracklist_all[k]->GetMotherId();
						int motherId4 = fMCTracklist_all[l]->GetMotherId();
						
						if(motherId1 == motherId2 && motherId1 == motherId3 && motherId1 == motherId4) {
							cout << "testxyz" << endl;
						if (motherId1 != -1) {
							int mcMotherPdg1  = -1;
							CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
							if (NULL != mother1) mcMotherPdg1 = mother1->GetPdgCode();
							if(mcMotherPdg1 == 111) {
								Double_t invmass2 = 0;
								invmass2 = Invmass_4particles(fMCTracklist_all[i], fMCTracklist_all[j], fMCTracklist_all[k], fMCTracklist_all[l]);
								fhInvariantMass_pi0epem->Fill(invmass2);
								cout << "#######################################  Decay pi0 -> e+e-e+e- detected!\t\t" << invmass2 << endl;
							}
						}
						else {
							continue;
						}
					}
						
						
						
						
						int grandmotherId1 = -1;
						int grandmotherId2 = -1;
						int grandmotherId3 = -1;
						int grandmotherId4 = -1;
						
						int mcMotherPdg1  = -1;
						int mcMotherPdg2  = -1;
						int mcMotherPdg3  = -1;
						int mcMotherPdg4  = -1;
						int mcGrandmotherPdg1  = -1;
						int mcGrandmotherPdg2  = -1;
						int mcGrandmotherPdg3  = -1;
						int mcGrandmotherPdg4  = -1;
						
						
						if (motherId1 != -1) {
							CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
							if (NULL != mother1) mcMotherPdg1 = mother1->GetPdgCode();
							grandmotherId1 = mother1->GetMotherId();
							if(grandmotherId1 != -1) {
								CbmMCTrack* grandmother1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
								if (NULL != grandmother1) mcGrandmotherPdg1 = grandmother1->GetPdgCode();
							}
						}
						if (motherId2 != -1) {
							CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
							if (NULL != mother2) mcMotherPdg2 = mother2->GetPdgCode();
							grandmotherId2 = mother2->GetMotherId();
							if(grandmotherId2 != -1) {
								CbmMCTrack* grandmother2 = (CbmMCTrack*) fMcTracks->At(grandmotherId2);
								if (NULL != grandmother2) mcGrandmotherPdg2 = grandmother2->GetPdgCode();
							}
						}
						if (motherId3 != -1) {
							CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
							if (NULL != mother3) mcMotherPdg3 = mother3->GetPdgCode();
							grandmotherId3 = mother3->GetMotherId();
							if(grandmotherId3 != -1) {
								CbmMCTrack* grandmother3 = (CbmMCTrack*) fMcTracks->At(grandmotherId3);
								if (NULL != grandmother3) mcGrandmotherPdg3 = grandmother3->GetPdgCode();
							}
						}
						if (motherId4 != -1) {
							CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);
							if (NULL != mother4) mcMotherPdg4 = mother4->GetPdgCode();
							grandmotherId4 = mother4->GetMotherId();
							if(grandmotherId4 != -1) {
								CbmMCTrack* grandmother4 = (CbmMCTrack*) fMcTracks->At(grandmotherId4);
								if (NULL != grandmother4) mcGrandmotherPdg4 = grandmother4->GetPdgCode();
							}
						}
					
					
						if(motherId1 == motherId2 && motherId2 == motherId3 && motherId3 == motherId4) {
							Double_t invmass = Invmass_4particles(fMCTracklist_all[i], fMCTracklist_all[j], fMCTracklist_all[k], fMCTracklist_all[l]);
							fhMCtest->Fill(invmass);
					//		cout << "Decay pi0 -> e+ e- e+ e- detected!" << endl;
						}
						
						if(motherId1 == motherId2 && motherId1 == motherId3) {
						
						}
						if(motherId1 == motherId2 && motherId1 == motherId4) {
						
						}
						if(motherId1 == motherId3 && motherId1 == motherId4) {
						
						}
						if(motherId2 == motherId3 && motherId2 == motherId4) {
							
						}
						
					//	if(  ((motherId1 == motherId2 && motherId3 == motherId4) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg3 == 22 || mcMotherPdg3 == 111))
					//	  || ((motherId1 == motherId3 && motherId2 == motherId4) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg2 == 22 || mcMotherPdg2 == 111))
					//	  || ((motherId1 == motherId4 && motherId2 == motherId3) && (mcMotherPdg1 == 22 || mcMotherPdg1 == 111) && (mcMotherPdg2 == 22 || mcMotherPdg2 == 111))) {
						if(  ((motherId1 == motherId2 && motherId3 == motherId4) && (mcMotherPdg1 == 22) && (mcMotherPdg3 == 22) && grandmotherId1 == grandmotherId3)
						  || ((motherId1 == motherId3 && motherId2 == motherId4) && (mcMotherPdg1 == 22) && (mcMotherPdg2 == 22) && grandmotherId1 == grandmotherId2)
						  || ((motherId1 == motherId4 && motherId2 == motherId3) && (mcMotherPdg1 == 22) && (mcMotherPdg2 == 22) && grandmotherId1 == grandmotherId2)) {
							Double_t invmass = Invmass_4particles(fMCTracklist_all[i], fMCTracklist_all[j], fMCTracklist_all[k], fMCTracklist_all[l]);
							fhMCtest->Fill(invmass);
							fhInvariantMass_MC_all->Fill(invmass);
					//		cout << "Decay pi0 -> 2gamma -> e+ e-! \t MotherId " << motherId1 << "\t" << motherId2 << "\t" << motherId3 << "\t" << motherId4 <<
					//			 "\t GrandmotherId " << grandmotherId1 << "\t" << grandmotherId2 << "\t" << grandmotherId3 << "\t" << grandmotherId4 << endl;
							if(mcGrandmotherPdg1 == 111) {
								fhInvariantMass_MC_pi0->Fill(invmass);
							}
							if(mcGrandmotherPdg1 == 221) {
								fhInvariantMass_MC_eta->Fill(invmass);
							}
						}
					}
				}
			}
		}
	}
	cout << "InvariantMassTestMC - End!" << endl;
}


Int_t CbmAnaConversion::NofDaughters(Int_t motherId) 
{
	Int_t nofDaughters = 0;
	for(int i=0; i<fRecoTracklistEPEM.size(); i++) {
		Int_t motherId_temp = fRecoTracklistEPEM[i]->GetMotherId();
		if(motherId == motherId_temp) nofDaughters++;
	}
	return nofDaughters;
}



void CbmAnaConversion::ReconstructGamma()
{
	if(fMCTracklist_all.size() >= 2) {
		for(int i=0; i<fMCTracklist_all.size(); i++) {
			for(int j=i+1; j<fMCTracklist_all.size(); j++) {
				if(fMCTracklist_all[i]->GetPdgCode() + fMCTracklist_all[j]->GetPdgCode() != 0) continue;
				
				int motherId1 = fMCTracklist_all[i]->GetMotherId();
				int motherId2 = fMCTracklist_all[j]->GetMotherId();
				
				if(motherId1 == motherId2) {
					Double_t invmass = Invmass_2particles(fMCTracklist_all[i], fMCTracklist_all[j]);
					fhSearchGammas->Fill(invmass);
				}
			}
		}
	}
}


/*
void CbmAnaConversion::SetKF(CbmKFParticleFinder* kfparticle)
{
	fKFparticle = kfparticle;
	if(fKFparticle) {
		cout << "kf works" << endl;
	}
	else {
		cout << "kf works not" << endl;
	}
}
*/




ClassImp(CbmAnaConversion)


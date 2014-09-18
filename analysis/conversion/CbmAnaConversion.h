/**
* \file CbmAnaConversion.h
*
* \brief Optimization of the RICH geometry.
*
* \author Sascha Reinecke <reinecke@uni-wuppertal.de>
* \date 2014
**/

#ifndef CBM_ANA_CONVERSION
#define CBM_ANA_CONVERSION

#include "FairTask.h"
#include "CbmMCTrack.h"
#include "../dielectron/CbmAnaDielectronTask.h" 
class TH1;
class TH2;
class TH3;
class TH1D;
class TH2D;
class TH2I;
class TH3D;
class TClonesArray;
class CbmRichRing;
class TCanvas;
class TRandom3;

#include <vector>
#include <map>

using namespace std;

/**
* \class CbmAnaConversion
*
* \brief Optimization of the RICH geometry.
*
* \author Tariq Mahmoud<t.mahmoud@gsi.de>
* \date 2014
**/

/*class KinematicParams{
public:
   Double_t momentumMag; // Absolute value of momentum
   Double_t pt; // Transverse momentum
   Double_t rapidity; // Rapidity
   Double_t minv; // Invariant mass
   Double_t angle; // Opening angle
};*/

class CbmAnaConversion : public FairTask
{

public:
   /**
    * \brief Standard constructor.
    */
	CbmAnaConversion();

   /**
    * \brief Standard destructor.
    */
   virtual ~CbmAnaConversion();

   /**
    * \brief Inherited from FairTask.
    */
   virtual InitStatus Init();

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Exec(
		   Option_t* option);
		   
	KinematicParams CalculateKinematicParams(const CbmMCTrack* mctrackP, const CbmMCTrack* mctrackM);
        
	Double_t	Invmass_2gammas(const CbmMCTrack* gamma1, const CbmMCTrack* gamma2);
	Double_t	Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2);
	Double_t	SmearValue(Double_t value);
	Double_t	Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4);
	Double_t	Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);


	void	CalculateInvMass_MC_2particles();

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Finish();
   
   void		AnalyseElectrons(CbmMCTrack* mctrack);
   void 	TomographyMC(CbmMCTrack* mctrack);
   void 	TomographyReco(CbmMCTrack* mctrack);
   void 	Probability();
   void 	FillMCTracklists(CbmMCTrack* mctrack);
   void 	FillRecoTracklist(CbmMCTrack* mtrack);
   void 	FillRecoTracklistEPEM(CbmMCTrack* mctrack, TVector3 stsMomentum, TVector3 refittedMom);
   void 	InvariantMassTest();
   void 	InvariantMassTest_4epem();
   void 	InvariantMassTestReco();
   int 		GetTest();
   int 		GetNofEvents();
   void		SetMode(int mode);
   void		InvariantMassMC_all();
   Int_t	NofDaughters(Int_t motherId);
   
   void		ReconstructGamma();



private:
   TH1D * fhGammaZ;
   TH2D * fTest;
   
   TH1D * fhNofElPrim;
   TH1D * fhNofElSec;
   TH1D * fhNofElAll;
   TH1D * fhElectronSources;
   TH1D * fhNofPi0_perEvent;		// number of pi0 per event
   TH1D * fhNofPi0_perEvent_cut;	// number of pi0 with cut on z-axis (z <= 70cm, i.e. generated before 70cm)
   TH1D * fhPi0_z;					// number of pi0 per z-bin
   TH1D * fhPi0_z_cut;				// number of pi0 per z-bin with cut on acceptance (25° via x^2 + y^2 <= r^2 with r = z*tan 25°)
   TH1D * fhElectronsFromPi0_z;		//
   
   TH3D * fhTomography;
   TH2D * fhTomography_XZ;
   TH2D * fhTomography_YZ;
   TH2D * fhTomography_uptoRICH;
   TH2D * fhTomography_RICH_complete;
   TH2D * fhTomography_RICH_beampipe;
   TH2D * fhTomography_STS_end;
   TH2D * fhTomography_STS_lastStation;
   TH2D * fhTomography_RICH_frontplate;
   TH2D * fhTomography_RICH_backplate;
      
   TH3D * fhTomography_reco;
   TH2D * fhTomography_reco_XZ;
   TH2D * fhTomography_reco_YZ;
   
   TH1D * fhConversion;
   TH1D * fhConversion_prob;
   TH1D * fhConversion_energy;
   TH1D * fhConversion_p;
   
   TH1D * fhInvariantMass_test;
   TH1D * fhInvariantMass_test2;
   TH1D * fhInvariantMass_test3;
   TH1D * fhInvariantMassReco_test;
   TH1D * fhInvariantMassReco_test2;
   TH1D * fhInvariantMassReco_test3;
      
   TH1D * fhInvariantMass_MC_all;
   TH1D * fhInvariantMass_MC_omega;
   TH1D * fhInvariantMass_MC_pi0;
   TH1D * fhInvariantMass_MC_eta;
   
   TH1D * fhInvariantMassReco_pi0;
   
   TH2D * fhMomentum_MCvsReco;
   TH1D * fhMomentum_MCvsReco_diff;
   
   TH1D * fhInvMass_EPEM_mc;
   TH1D * fhInvMass_EPEM_stsMomVec;
   TH1D * fhInvMass_EPEM_refitted;
   TH1D * fhInvMass_EPEM_error_stsMomVec;
   TH1D * fhInvMass_EPEM_error_refitted;
   TH1D * fhUsedMomenta_stsMomVec;
   TH1D * fhUsedMomenta_mc;
   TH1D * fhUsedMomenta_error_stsMomVec;
   TH1D * fhUsedMomenta_error_refitted;
   TH1D * fhUsedMomenta_errorX_stsMomVec;
   TH2D * fhUsedMomenta_vsX_stsMomVec;
   TH1D * fhUsedMomenta_errorY_stsMomVec;
   TH2D * fhUsedMomenta_vsY_stsMomVec;
   TH1D * fhUsedMomenta_errorZ_stsMomVec;
   TH2D * fhUsedMomenta_vsZ_stsMomVec;
   
   TH1D * fhInvariantMass_pi0epem;
   
   TH1D * fhMCtest;
   
   TH1D * fhSearchGammas;

   TClonesArray* fRichPoints;
   TClonesArray* fMcTracks;
   TClonesArray* fStsTracks;
   TClonesArray* fStsTrackMatches;
   TClonesArray* fGlobalTracks;

   Int_t fEventNum;
   
   Int_t test;
   
   int testint;
   
   Int_t fAnalyseMode;
   
   
   
   
   
   
	CbmVertex *fPrimVertex;
	CbmKFVertex fKFVertex;



   /**
    * \brief Initialize histograms.
    */
   void InitHistograms();
   
   vector<TH1*> fHistoList;				// list of all histograms
   vector<TH1*> fHistoList_MC;			// list of all histograms generated with MC data
   vector<TH1*> fHistoList_tomography;	// list of all histograms of tomography data (photon conversion)
   vector<TH1*> fHistoList_reco;		// list of all histograms of reconstruction data
   
   vector<CbmMCTrack*> fMCTracklist;
   vector<CbmMCTrack*> fMCTracklist_all;
   vector<CbmMCTrack*> fRecoTracklist;
   vector<CbmMCTrack*> fRecoTracklistEPEM;
   
   vector<TVector3> fRecoMomentum;
   vector<TVector3> fRecoRefittedMomentum;


   /**
    * \brief Copy constructor.
    */
   CbmAnaConversion(const CbmAnaConversion&);

   /**
    * \brief Assignment operator.
    */
   CbmAnaConversion& operator=(const CbmAnaConversion&);


	ClassDef(CbmAnaConversion,1)
};

#endif


#ifndef CBM_ANA_CONVERSION2
#define CBM_ANA_CONVERSION2

#include "FairTask.h"
#include "CbmRichRecGeoPar.h"
class TH1;
class TProfile2D;
class TProfile;
class TH2;
class TH1D;
class TH2D;
class TH3D;
class TGraph;
class TClonesArray;
class CbmRichRing;
class TCanvas;
class CbmRichRingLight;
class CbmRichRingFitterEllipseTau;

#include <vector>
#include <map>
#include "CbmKFVertex.h"

#include "FairTask.h"
#include "CbmMCTrack.h"
#include "../dielectron/CbmLmvmKinematicParams.h"
#include "CbmStsTrack.h"

#include "CbmKFParticleFinder.h"
#include "CbmKFParticleFinderQA.h"

#include "CbmAnaConversion2Reconstruction.h"
#include "CbmAnaConversion2KF.h"
#include "CbmAnaConversion2Fitter.h"
#include "CbmAnaConversion2Manual.h"


using namespace std;

class CbmAnaConversion2 : public FairTask
{

public:
   /**
    * \brief Standard constructor.
    */
	CbmAnaConversion2();

   /**
    * \brief Standard destructor.
    */
   virtual ~CbmAnaConversion2();

   /**
    * \brief Inherited from FairTask.
    */
   virtual InitStatus Init();

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Exec(
		   Option_t* option);

  /**
    * \brief Fit ring using circle fitter and fill histograms.
    * \param[in] histIndex Fitting type index, 0 - hit fitting, 1 - MC points fitting.
    * \param[in] ring Pointer to CbmRichRingLight to be fitted and filled in histograms.
    * \param[in] momentum MC momentum of particle produced ring.
    */

	void FitAndFillHistEllipse(
	      Int_t histIndex,
	      CbmRichRingLight* ring);

	void	FillMCTracklists(CbmMCTrack* mctrack, int i);
	Double_t	Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2);
	Double_t	Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4);
	Double_t	Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	TVector3 STSReffit(CbmStsTrack* stsTrack);
	TVector3 FitToVertexSTS(CbmStsTrack* stsTrack, CbmMCTrack* mcTrack);

	void	SetKF(CbmKFParticleFinder* kfparticle, CbmKFParticleFinderQA* kfparticleQA);


 
   /**
    * \brief Inherited from FairTask.
    */
   virtual void Finish();



private:
   vector<TH1*> fHistoList;

   Int_t fEventNum;
   Int_t DecayedParticlePdg;
   Int_t LeptonsInDecay;
   Int_t Dalitz;
   Int_t DoReconstruction;
   Int_t DoKFAnalysis;

   Double_t counter_full_ind_STS;
   Double_t counter_full_ind_RICH;
   Double_t fMinAaxis;
   Double_t fMaxAaxis;
   Double_t fMinBaxis;
   Double_t fMaxBaxis;
   Double_t fMinRadius;
   Double_t fMaxRadius;

   TClonesArray* fRichRings;
   TClonesArray* fRichHits;
   TClonesArray* fRichPoints;
   TClonesArray* fRichProjections;
   TClonesArray* fMcTracks;
   TClonesArray* fStsTracks;
   TClonesArray* fStsTrackMatches;
   TClonesArray* fGlobalTracks;
   TClonesArray* fRichRingMatches;
	TClonesArray *fArrayStsHit;
	TClonesArray *fArrayMvdHit; 

   CbmRichRingFitterEllipseTau* fTauFit;

   TH1D * fRichRings_Aaxis;
   TH1D * fRichRings_Baxis;
   TH1D * fhPio_pt;
   TH1D * fhPio_theta;
   TH1D * fhPdgCodes;
   TH1D * dR_plot_electrons;

   TH2D * NumberOfRings_electrons;
   TH2D * AllPoints_from_hits_electrons;
   TH2D * fhPio_pt_vs_rap;
   TH2D * fhPio_theta_vs_rap;
   TH2D * AllPoints2D;
   TH2D * eff;
   TH2D * STSmomenta;
   TH2D * ConversionPoints2D;
   TH2D * Invariant_masses;

   TH3D * AllPoints3D;
   TH3D * ConversionPoints;

   TProfile2D * A_electrons;
   TProfile2D * B_electrons;
   TProfile2D * hprof2d_electrons;
   TProfile2D * BoA_electrons;

   vector<Int_t>       UsedRingsId;
   vector<CbmMCTrack*> fMCTracklist_gamma_from_our_particle;
   vector<CbmMCTrack*> fMCTracklist_gamma_all;
   vector<CbmMCTrack*> fMCTracklist_elpositrons_from_our_particle;
   vector<CbmMCTrack*> fMCTracklist_elpositrons_from_gamma;
   vector<CbmMCTrack*> fMCTracklist_elpositrons_all;
   vector<CbmMCTrack*> fMCTracklist_elpositrons_from_gamma_from_our_particle;
   vector<TVector3>    fTracklist_noRichInd_momentum;
   vector<CbmMCTrack*> fTracklist_noRichInd_mcTrack;
   vector<CbmMCTrack*> fTracklist_noRichInd_conversion;
   vector<TVector3>    fTracklist_noRichInd_refmomentum_direct;
   vector<TVector3>    fTracklist_noRichInd_refmomentum_conversion;
   vector<TVector3>    fTracklist_withRichInd_refmomentum_direct;
   vector<TVector3>    fTracklist_withRichInd_refmomentum_conversion;
   vector<CbmMCTrack*> fTracklist_withRichInd_mcTrack;
   vector<TVector3>    fTracklist_noRichInd_refmomentum_all;
   vector<TVector3>    fTracklist_withRichInd_refmomentum_all;
   vector<Int_t>       fTracklist_noRichInd_mcTrack_STS_Id;


	CbmVertex *fPrimVertex;
	CbmKFVertex fKFVertex;



   /**
    * \brief Initialize histograms.
    */
   void InitHistograms();

   void RingParameters();



   CbmAnaConversion2Reconstruction			*fAnaReco;
   CbmAnaConversion2KF						*fAnaKF;
   CbmKFParticleFinder* fKFparticle;
   CbmKFParticleFinderQA* fKFparticleFinderQA;

   Int_t DoFitter;
   CbmAnaConversion2Fitter			*fAnaFitter;

   CbmAnaConversion2Manual						*fAnaManual;
   Int_t DoManualAnalysis;



   /**
    * \brief Copy constructor.
    */
   CbmAnaConversion2(const CbmAnaConversion2&);

   /**
    * \brief Assignment operator.
    */
   CbmAnaConversion2& operator=(const CbmAnaConversion2&);


	ClassDef(CbmAnaConversion2,1)
};

#endif


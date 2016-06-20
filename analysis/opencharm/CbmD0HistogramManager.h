// ------------------------------------------------------
// -----         CbmD0HistogramManager header file  -----
// -----          Created 09.11.2015 by P.Sitzmann  -----
// ------------------------------------------------------


#ifndef CBMD0HISTOGRAMMANAGER
#define CBMD0HISTOGRAMMANAGER 1


#include "TObject.h"
#include "CbmHistManager.h"
#include "TFile.h"
#include "TBranch.h"
#include "FairTask.h"

class CbmVertex;
class CbmKF;
class CbmL1;
class FairRootManager;
class CbmGlobalTrack;

enum HistoGroup {SINGELTRACK, PAIR, MCQA, RECO, ALL};

class CbmD0HistogramManager :  public FairTask
{
public:
/** Default constructor **/
    CbmD0HistogramManager();
    CbmD0HistogramManager(TString name, Float_t PTCut,Float_t SvZCut, Float_t PZCut);

/** Destructor **/
~CbmD0HistogramManager();

/** Accessors **/


/** Modifiers **/
void SetPostCuts(Float_t Chi2SingelCut = 9., Float_t PTCut = 1., Float_t SvZCut = 0.07, Float_t PZCut = 1.);
void SetHistogramChois(TString group);

void SetWriteOutFile(TFile* file){fwriteOutFile = file;};

InitStatus Init();
void Exec(Option_t* option);
void Finish();

Float_t GetIPRadius(CbmGlobalTrack* track);

private:

    FairRootManager* ioman;

    Int_t fEventNr;

    Float_t fchi2Singel;
    Float_t fcutPT;
    Float_t fcutSvZ;
    Float_t fcutPZ;
    HistoGroup fChois;

    Int_t fnrMcEvents;
    Int_t fnrPairEvents;
    Int_t fnrRecoEvents;
    Int_t fnrTrackEvents;
    Int_t fnrSingelEvents;

    TFile* fwriteOutFile;

    TClonesArray* fListTrackMatch;
    TClonesArray* fListMCTracks;
    TClonesArray* fListMCPointsMvd;
    TClonesArray* fListMCPointsSts;
    TClonesArray* fListMCPointsRich;
    TClonesArray* fListMCPointsTrd;
    TClonesArray* fListMCPointsTof;

    TClonesArray* fListPairs;
    TClonesArray* fListCandidates;
    TClonesArray* fListKaons;
    TClonesArray* fListPions;
    TClonesArray* fListReco;
    TClonesArray* fListGlobal;
    TClonesArray* fKaonTrackArray;
    TClonesArray* fPionTrackArray;
    CbmVertex* fPrimVtxs;


    TH1* mcMomentum;
    TH1* mcMomentumMvd;
    TH1* mcMomentumMvdTrack;
    TH1* mcMomentumMvdStsTrack;
    TH1* mcMomentumMvdStsTrackPID;

    TH1* mcMomentumStsTrackMvdVertex;
    TH1* mcMomentumKaonTrackable;
    TH1* mcMomentumKaonTrackPID;
    TH1* mcMomentumPion1Trackable;
    TH1* mcMomentumPion1TrackPID;
    TH1* mcMomentumSignal;
    TH1* mcMomentumSignalPID;
    TH1* diffRecoSVMCSV;
    TH1* diffRecoSVMCSV1GeV;

    TH2* chi2vsDiffVertex;

    TH1* pairMomentum;

    TH1* massSpectra;
    TH1* massSpectraSVZ3H;
    TH1* massSpectraChi23H;
    TH1* massSpectraSVZ4H;
    TH1* massSpectraChi24H;

    TH1* chi2RecoD0;
    TH1* zPosDecay;
    TH1* secondaryVertexRes;
    TH1* secondaryVertexResmin3MVD;
    TH1* secondaryVertexRes3MVD;
    TH1* secondaryVertexRes4MVD;
    TH1* iPRadiusD0;

    TH1* chi2RecoD0PostCut;
    TH1* zPosDecayPostCut;
    TH1* secondaryVertexResPostCut;
    TH1* secondaryVertexResPostCut3H;
    TH1* secondaryVertexResPostCutex3H;
    TH1* secondaryVertexResPostCut4H;
    TH1* secVertexResChiCut;
    TH1* secVertexResChiCutSVZcut;
    TH1* secVertexResChiCutSVZcut4MVD;

    TH1* iPRadiusD0PostCut;
    TH1* chi2singelTrackBadD0;
    TH1* iPsingelTrackBadD0;
    TH1* nrMvdHitsBadD0;

    TH2* ptY;
    TH2* chi2IP;

    TH1* recoMomdist;
    TH1* recoMomdistAll;
    TH1* recoMomdistmin3Hit;
    TH1* recoMomdistmin4Hit;
    TH1* recoMomdistSecVCut;
    TH1* recoMomdistSecVCutmin4Hit;


    TH2* recoSigma;

    TH2* recoPionSigma ;
    TH2* recoKaonSigma; 
    TH2* recoPionSigmaSec ;
    TH2* recoKaonSigmaSec;

    TH1* recoIPAll;
    TH1* recoIP4Perfect;
    TH1* recoIPfirstWrong;


    TH1* pionIP;
    TH1* kaonIP;
    TH1* momKaon;
    TH1* momPion;
    TH1* chi2NDFKaon;
    TH1* chi2NDFPion;

    TH2* kaonPtY;
    TH2* pionPtY;



    void Register();

    void InitSingel();
    void InitPair();
    void InitMc();
    void InitReco();
    void InitAll();

    void ExecSingel();
    void ExecReco();
    void ExecPair();
    void ExecMc();
    void ExecAll();

    CbmD0HistogramManager(const CbmD0HistogramManager&);
    CbmD0HistogramManager& operator=(const CbmD0HistogramManager&);

ClassDef(CbmD0HistogramManager,1);

};
#endif
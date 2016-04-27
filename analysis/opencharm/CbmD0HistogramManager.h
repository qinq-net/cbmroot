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
void SetPostCuts(Float_t PTCut, Float_t SvZCut, Float_t PZCut);
void SetHistogramChois(TString group);

void SetWriteOutFile(TFile* file){fwriteOutFile = file;};
void SetMCFile(TFile* file){fMCFile = file;};
void SetRecoFile(TFile* file){fRecoFile = file;};
void SetPairFile(TFile* file){fPairFile = file;};
void SetTrackFile(TFile* file){fTrackFile = file;};

InitStatus Init();
void Exec(Option_t* option);
void Finish();

private:
    CbmKF* kalman;
    CbmL1* l1;
    FairRootManager* ioman;

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
    TFile* fMCFile;
    TFile* fRecoFile;
    TFile* fPairFile;
    TFile* fTrackFile;

    TBranch* fmcTrackBranch;
    TBranch* fmvdPointBranch;
    TBranch* fStsPointBranch;

    TBranch* fpairBranch;
    TBranch* fKaonBranch;
    TBranch* fPionBranch;
    TBranch* fRecoBranch;
    TBranch* fRecoVtxBranch;
    TBranch* fTrackMatchBranch;

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
    CbmVertex* fPrimVtxs;


    TH1* mcMomentum;
    TH1* mcMomentumMvd;
    TH1* mcMomentumMvdTrack;
    TH1* mcMomentumMvdStsTrack;
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
    TH1* chi2RecoD0;
    TH1* zPosDecay;
    TH1* secondaryVertexRes;
    TH1* iPRadiusD0;

    TH1* chi2RecoD0PostCut;
    TH1* zPosDecayPostCut;
    TH1* secondaryVertexResPostCut;
    TH1* iPRadiusD0PostCut;

    TH2* ptY;
    TH2* chi2IP;

        TH1* recoMomdist;

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
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


enum HistoGroup {SINGELTRACK, PAIR, MCQA, RECO, ALL};

class CbmD0HistogramManager : public CbmHistManager
{
public:
/** Default constructor **/
    CbmD0HistogramManager();
    CbmD0HistogramManager(const char* group,Float_t PTCut,Float_t SvZCut, Float_t PZCut);

/** Destructor **/
~CbmD0HistogramManager();

/** Accessors **/


/** Modifiers **/
void SetCuts(Float_t PTCut, Float_t SvZCut, Float_t PZCut);
void SetHistogramChois(const char* group);

void SetOutFile(TFile* file){fwriteOutFile = file;};
void SetMCFile(TFile* file){fMCFile = file;};
void SetRecoFile(TFile* file){fRecoFile = file;};
void SetPairFile(TFile* file){fPairFile = file;};
void SetTrackFile(TFile* file){fTrackFile = file;};

void Init();
void Exec(Option_t* option);
void Finish();

private:
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

    TClonesArray* fListMCTracks;
    TClonesArray* fListMCPointsMvd;
    TClonesArray* fListMCPointsSts;
    TClonesArray* fListMCPointsRich;
    TClonesArray* fListMCPointsTrd;
    TClonesArray* fListMCPointsTof;

    TClonesArray* fListPairs;
    TClonesArray* fListKaons;
    TClonesArray* fListPions;

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
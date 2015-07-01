// -----------------------------------------------------------------
// -----             CbmD0Candidate header file                -----
// -----          Created 23/01/07  by C.Dritsa and M.Deveaux  -----
// -----------------------------------------------------------------


#ifndef CBMD0CANDIDATE
#define CBMD0CANDIDATE 1


#include "TObject.h"


class CbmD0Candidate : public TObject

{

public:

    /** Default constructor. Do not use it **/
    CbmD0Candidate();

    /** Constructor with all variables **/

    CbmD0Candidate(Int_t signal,
		   Int_t Pid1, Double_t p1, Double_t pt1, Double_t PV1, Double_t IP1, Double_t imx1, Double_t imy1, Int_t nMvdHits1, Int_t nStsHits1,
		   Int_t Pid2, Double_t p2, Double_t pt2, Double_t PV2, Double_t IP2, Double_t imx2, Double_t imy2, Int_t nMvdHits2, Int_t nStsHits2,
		   Double_t SvChi,  Double_t SvZ,    Double_t IPD0,   Double_t IM,     Double_t cos12, Double_t IPAngle,
		   Double_t SvXErr, Double_t SvYErr, Double_t SvZErr, Double_t ptD0, Double_t pzD0, Double_t SvChiT, Double_t SvZT,  Double_t ptt,
		   Double_t alpha,  Double_t PvZ,  Double_t cosA);

    /** Destructor **/
    virtual ~CbmD0Candidate();

    /** Accessors **/

    /** Modifiers **/

//private:
public:
    Int_t fSignal;
    Int_t fPid1;
    Double_t fP1;
    Double_t fPt1;
    Double_t fPV1;
    Double_t fIP1;
    Double_t fImx1;
    Double_t fImy1;
    Int_t fNMvdHits1;
    Int_t fNStsHits1;

    Int_t fPid2;
    Double_t fP2;
    Double_t fPt2;
    Double_t fPV2;
    Double_t fIP2;
    Double_t fImx2;
    Double_t fImy2;
    Int_t fNMvdHits2;
    Int_t fNStsHits2;


    Double_t fSvChi;
    Double_t fSvZ;
    Double_t fIPD0;
    Double_t fIM;
    Double_t fCos12;
    Double_t fIPAngle;

    Double_t fSvXErr;
    Double_t fSvYErr;
    Double_t fSvZErr;
    Double_t fPtD0;
    Double_t fPzD0;
    Double_t fSvChiT;
    Double_t fSvZT;
    Double_t fPtt;

    Double_t fAlpha;
    Double_t fPvZ;
    Double_t fCosA;


    ClassDef(CbmD0Candidate,1);

};


#endif

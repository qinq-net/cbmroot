// -----------------------------------------------------------------------
// -----              CbmTrackCandidate source file                  -----
// -----         Created 23/01/07  by C. Dritsa and M. Deveaux       -----
// -----------------------------------------------------------------------
#include "CbmD0Candidate.h"

// -----   Default constructor   -------------------------------------------
CbmD0Candidate::CbmD0Candidate()
    :TObject(),
     fSignal(-1),
     fPid1(-1),
     fP1(-1.1),
     fPt1(-1.1),
     fPV1(-1.1),
     fIP1(-1.1),
     fImx1(-1.1),
     fImy1(-1.1),
     fNMvdHits1(-1),
     fNStsHits1(-1),
     fPid2(-1),
     fP2(-1.1),
     fPt2(-1.1),
     fPV2(-1.1),
     fIP2(-1.1),
     fImx2(-1.1),
     fImy2(-1.1),
     fNMvdHits2(-1),
     fNStsHits2(-1),
     fSvChi(-1.1),
     fSvZ(-1.1),
     fIPD0(-1.1),
     fIM(-1.1),
     fCos12(-1.1),
     fIPAngle(-1.1),
     fSvXErr(-1.1),
     fSvYErr(-1.1),
     fSvZErr(-1.1),
     fPtD0(-1.1),
     fPzD0(-1.1),
     fSvChiT(-1.1),
     fSvZT(-1.1),
     fPtt(-1.1),
     fAlpha(-1.1),
     fPvZ(-1.1),
     fCosA(2.1),
     fRapidity(),
     ftrack1(-1),
     ftrack2(-1)
{

}
// -------------------------------------------------------------------------



// -----   Constructor with parameters   -----------------------------------
CbmD0Candidate::CbmD0Candidate(Int_t signal,
		   Int_t Pid1, Double_t p1, Double_t pt1, Double_t PV1, Double_t IP1, Double_t imx1, Double_t imy1, Int_t nMvdHits1, Int_t nStsHits1,
		   Int_t Pid2, Double_t p2, Double_t pt2, Double_t PV2, Double_t IP2, Double_t imx2, Double_t imy2, Int_t nMvdHits2, Int_t nStsHits2,
		   Double_t SvChi,  Double_t SvZ,  Double_t IPD0, Double_t IM,     Double_t cos12, Double_t IPAngle,
		   Double_t SvXErr, Double_t SvYErr, Double_t SvZErr, Double_t ptD0, Double_t pzD0, Double_t SvChiT, Double_t SvZT,  Double_t ptt,
			       Double_t alpha,  Double_t PvZ,  Double_t cosA, Double_t rapidity, Int_t trackCand1, Int_t trackCand2)
    :TObject(),
     fSignal(signal),
     fPid1(Pid1),
     fP1(p1),
     fPt1(pt1),
     fPV1(PV1),
     fIP1(IP1 ),
     fImx1(imx1),
     fImy1(imy1),
     fNMvdHits1(nMvdHits1),
     fNStsHits1(nStsHits1),
     fPid2(Pid2),
     fP2(p2),
     fPt2(pt2),
     fPV2(PV2),
     fIP2(IP2),
     fImx2(imx2),
     fImy2(imy2),
     fNMvdHits2(nMvdHits2),
     fNStsHits2(nStsHits2),
     fSvChi(SvChi),
     fSvZ(SvZ),
     fIPD0(IPD0),
     fIM(IM),
     fCos12(cos12),
     fIPAngle(IPAngle),
     fSvXErr(SvXErr),
     fSvYErr(SvYErr),
     fSvZErr(SvZErr),
     fPtD0(ptD0),
     fPzD0(pzD0),
     fSvChiT(SvChiT),
     fSvZT(SvZT),
     fPtt(ptt),
     fAlpha(alpha),
     fPvZ(PvZ),
     fCosA(cosA),
     fRapidity(rapidity),
     ftrack1(trackCand1),
     ftrack2(trackCand2)
{
/*
    fSignal  = signal;

    fPid1    = Pid1;
    fP1      = p1;
    fPt1     = pt1;
    fPV1     = PV1;
    fIP1     = IP1;
    fImx1    = imx1;
    fImy1    = imy1;
    fNMvdHits1 = nMvdHits1;
    fNStsHits1 = nStsHits1;

    fPid2    = Pid2;
    fP2      = p2;
    fPt2     = pt2;
    fPV2     = PV2;
    fIP2     = IP2;
    fImx2    = imx2;
    fImy2    = imy2;
    fNMvdHits2 = nMvdHits2;
    fNStsHits2 = nStsHits2;


    fSvChi   = SvChi;
    fSvZ     = SvZ;
    fIPD0    = IPD0;
    fIM      = IM;
    fCos12   = cos12;
    fIPAngle = IPAngle;
    fSvXErr  = SvXErr;
    fSvYErr  = SvYErr;
    fSvZErr  = SvZErr;
    fPtD0    = ptD0;
    fPzD0    = pzD0;
    fSvChiT  = SvChiT;
    fSvZT    = SvZT;
    fPtt     = ptt;
    fAlpha   = alpha;
    fPvZ     = PvZ;
    fCosA    = cosA;
    fRapidity= rapidity;

    ftrack1 = trackCand1;
    ftrack2 = trackCand2;
*/   
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmD0Candidate::~CbmD0Candidate() {}
// -------------------------------------------------------------------------

ClassImp(CbmD0Candidate)

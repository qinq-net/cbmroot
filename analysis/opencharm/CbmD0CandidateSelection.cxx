

//includes from cbm
#include "CbmStsTrack.h"
#include "CbmVertex.h"
#include "CbmMCTrack.h"
#include "CbmKFTrack.h"
#include "CbmTrackMatch.h"
#include "CbmD0CandidateSelection.h" 
#include "CbmD0Tools.h"

//includes from ROOT
#include "TClonesArray.h"
#include "TVector3.h"

//includes from Fair
#include "FairRootManager.h"

//includes from KF
#include "CbmKF.h"
#include "CbmKFTrack.h"
#include "CbmKFVertex.h"
#include "KFParticle.h"
#include "KFPVertex.h"

// Includes from C++
#include <iostream>
#include <iomanip>
#include <vector>

using std::cout;
using std::endl;
using std::map;
using std::setw;
using std::left;
using std::right;
using std::fixed;
using std::pair;
using std::setprecision;
using std::ios_base;
using std::vector;


// -------------------------------------------------------------------------
CbmD0CandidateSelection::CbmD0CandidateSelection()
:FairTask(),
fEventNumber(),
fKaonParticleArray(),
 fPionParticleArray(),
 fD0Candidates(),
fListMCTracksPos(),
 fListMCTracksNeg(),
 fListMCTracks(),
fListD0TrackCandidate(),
fD0TrackArray(),
 fStsTrackMatches(),
 fPosStsTracks(),
 fNegStsTracks(),
 fPrimVtx(),
 fvtx(),
 tools(),
 fMF(),
 bTestMode(),
    f_particleIsMCD0(),
    fcutSVZ(),
    fcutIPD0()

{
    
Fatal( "CbmD0Candidates: Do not use the standard constructor","Wrong constructor");
    
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0CandidateSelection::CbmD0CandidateSelection(char* name, Int_t iVerbose, Double_t cutIPD0, Double_t cutSVZ)
:FairTask(name,iVerbose),
fEventNumber(),
fKaonParticleArray(),
 fPionParticleArray(),
 fD0Candidates(),
fListMCTracksPos(),
 fListMCTracksNeg(),
 fListMCTracks(),
fListD0TrackCandidate(),
fD0TrackArray(),
 fStsTrackMatches(),
 fPosStsTracks(),
 fNegStsTracks(),
 fPrimVtx(),
 fvtx(),
 tools(),
 fMF(),
 bTestMode(),
 f_particleIsMCD0(),
    fcutSVZ(),
    fcutIPD0()

{
fEventNumber = 0;
fcutIPD0 = cutIPD0;
fcutSVZ  = cutSVZ;
bTestMode = kFALSE;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0CandidateSelection::~CbmD0CandidateSelection() {
    delete fD0Candidates;

}
// -------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void CbmD0CandidateSelection::Finish() {
 
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
InitStatus CbmD0CandidateSelection::Init() {

    Register();

    if(! fKaonParticleArray || !fPionParticleArray)
        {Fatal("CbmD0CandidateSelection: Kaon or Pion Array not found"," That's bad. ");}
    if (!fPrimVtx){
	cout << "-W- : CbmD0CandidateSelection - No primary Vtx, assume (0,0,0)" << endl;
	fPrimVtx=new CbmVertex("PrimaryVertex","PrimaryVertex");
    }
    if( (!fListMCTracksPos || !fListMCTracksNeg) && bTestMode)
    {
	Fatal("List of mc Tracks are not available, but TestMode is enabled", "Check Track Selection");
    }
    CbmKF* fKF = CbmKF::Instance();
    if(fKF)fMF = fKF->GetMagneticField();
    else return kFATAL;

    tools = new CbmD0Tools();
    fvtx[0] = fPrimVtx->GetX();
    fvtx[1] = fPrimVtx->GetY();
    fvtx[2] = fPrimVtx->GetZ();

    return kSUCCESS;
}
// -------------------------------------------------------------------------


 // -------------------------------------------------------------------------
void CbmD0CandidateSelection::Exec(Option_t* option){

    fEventNumber++;
    Int_t D0counter;
    f_particleIsMCD0 = kFALSE;
    Int_t nAcceptedD0 = 0;
    Int_t crossCheck=0;
    cout << endl;
    cout << endl;
    cout << "========================================================================================"<<endl;
    cout << endl << "CbmD0CandidateSelection:: Event: " << fEventNumber << endl;


    fD0Candidates->Clear();

    Int_t nPionTracks = fPionParticleArray->GetEntriesFast();
    Int_t nKaonTracks = fKaonParticleArray->GetEntriesFast();
    cout << "Number of KaonTracks: " << nKaonTracks << endl;
    cout << "Number of PionTracks: " << nPionTracks << endl;

for( Int_t itr1=0; itr1<nKaonTracks; itr1++)
   {
       KFParticle kaon = *((KFParticle*)fKaonParticleArray->At(itr1));


   for( Int_t itr2=0; itr2<nPionTracks; itr2++)
      {
	  KFParticle pion = *((KFParticle*)fPionParticleArray->At(itr2));

      if(bTestMode)
        {
        CheckIfParticleIsD0(itr1, itr2);
	}

      crossCheck++;

      const KFParticle* daughters[2] = {&kaon, &pion};
      KFParticle d0;
      d0.Construct(daughters, 2);

      Double_t SvZ     = d0.GetZ();
      Double_t IPD0    = GetPairImpactParameterR(&d0);
    
      if( SvZ != SvZ || IPD0 != IPD0 ){cout << "NaN detected!! " << endl;  continue;}         // one of the parameter is NaN

      //--- Apply cuts ---
      if( IPD0 > fcutIPD0 || SvZ < fcutSVZ)continue;   // parameter didn't pass the cuts or
    
      nAcceptedD0 ++;

      TClonesArray& clrefD0 = *fD0Candidates;
      Int_t sizeD0 = clrefD0.GetEntriesFast();
      new (clrefD0[sizeD0]) KFParticle(d0);
        
      }// second for loop

    }// first for loop

  
    cout << endl << "Number of combinations: " << crossCheck << " Number of candidates in acceptance: "<< nAcceptedD0 <<endl;
    cout << "========================================================================================"<<endl;
}//Exec
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
void CbmD0CandidateSelection::CheckIfParticleIsD0(Int_t iNeg, Int_t iPos)
{
TVector3 vertex1;
TVector3 vertex2;

CbmMCTrack* mcTrack1 = NULL;
CbmMCTrack* mcTrack2 = NULL;

mcTrack1  = (CbmMCTrack*) fListMCTracksNeg->At(iNeg);
mcTrack2  = (CbmMCTrack*) fListMCTracksPos->At(iPos);

mcTrack1->GetStartVertex(vertex1);
mcTrack2->GetStartVertex(vertex2);

Int_t mcPid1 = mcTrack1->GetPdgCode();
Int_t mcPid2 = mcTrack2->GetPdgCode();


// ------------ Identify Signal on MC base --------------
if (
   ( (mcPid1 == -321) && (mcPid2 == 211) )
   &&
   ( ( vertex1.Z() > 0.0000001 ) &&  ( vertex2.Z() > 0.0000001 ) )
   &&
   ( ( mcTrack1->GetMotherId() == -1 ) && ( mcTrack2->GetMotherId() == -1) )
   )
   {
    f_particleIsMCD0=kTRUE;
    cout << "----------------------------------" << endl;
    cout << "********< MC-D0 detected >********" << endl;
    cout << "----------------------------------" << endl;
    }
else
   {
   f_particleIsMCD0=kFALSE;
   }
}
//-----------------------------------------------------------------------------------------

 // -----   Private method Register   ---------------------------------------
void CbmD0CandidateSelection::Register() {
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman) Fatal(" CbmD0CandidateSelection::Register",
		      "No FairRootManager");

  fD0Candidates = new TClonesArray("KFParticle",100);
  ioman->Register("CbmD0Candidate", "OpenCharm", fD0Candidates, kTRUE);

    fStsTrackMatches    = (TClonesArray*) ioman->GetObject("StsTrackMatch");
    fListMCTracks       = (TClonesArray*) ioman->GetObject("MCTrack");
    fKaonParticleArray  = (TClonesArray*) ioman->GetObject("CbmD0KaonParticles");
    fPionParticleArray  = (TClonesArray*) ioman->GetObject("CbmD0PionParticles");
    fPrimVtx            = (CbmVertex*) ioman->GetObject("PrimaryVertex");
    fListMCTracksPos    = (TClonesArray*) ioman->GetObject("PositiveMCTracks");
    fListMCTracksNeg    = (TClonesArray*) ioman->GetObject("NegativeMCTracks");
}
// -------------------------------------------------------------------------  


//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetPairImpactParameterR(KFParticle* particle)
{
 particle->TransportToPoint(&*fvtx);
 Double_t Pair_IP  = sqrt((particle->GetX()*particle->GetX()) + (particle->GetY()*particle->GetY()) );
 particle->TransportToDecayVertex();
 return  Pair_IP;
}


//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
/*
//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetPairTx(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2){

    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);

    Double_t Pair_Px = mom1.X() + mom2.X();
    Double_t Pair_Pz = mom1.Z() + mom2.Z();

    Double_t Pair_Tx;

    if( Pair_Pz != 0.0){
	Pair_Tx = Pair_Px/Pair_Pz;
    }
    else{
	Pair_Tx = 0.0;
    }

    return Pair_Tx;
}

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetPairTy(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2){

    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);

    Double_t Pair_Py = mom1.Y() + mom2.Y();
    Double_t Pair_Pz = mom1.Z() + mom2.Z();

    Double_t Pair_Ty;

    if( Pair_Pz != 0.0){
	Pair_Ty = Pair_Py/Pair_Pz;
    }
    else{
	Pair_Ty = 0.0;
    }

    return Pair_Ty;
}

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetPairPt(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2){

    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);

    Double_t PairPx = mom1.X() + mom2.X();
    Double_t PairPy = mom1.Y() + mom2.Y();

    Double_t PairPt = sqrt( PairPx*PairPx + PairPy*PairPy );

    return PairPt;
}

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetPairPz( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 ){

    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);

    Double_t PairPz = mom1.Z() + mom2.Z();

    return PairPz;
}

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetMomentum( CbmD0TrackCandidate* tr1 ){

    TVector3 mom;
    tr1->Momentum(mom);

    Double_t Momentum = mom.Mag();

    return Momentum;
}

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetPairMomentum(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2){

    Double_t PairPt = GetPairPt( tr1, tr2 );
    Double_t PairPz = GetPairPz( tr1, tr2 );
    Double_t PairMomentum = sqrt( PairPt*PairPt + PairPz*PairPz );
    return PairMomentum;
}

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetEnergy(CbmD0TrackCandidate* tr1){

    return 0.0;
}   

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetPairEnergy( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 ){

    Double_t Energy1 = GetEnergy(tr1);
    Double_t Energy2 = GetEnergy(tr2);

    Double_t PairEnergy = Energy1 + Energy2;

    return PairEnergy;
}

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetIM( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2){

    return 0.0;
}

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetCos12( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 ){

    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);


    Double_t P1 = mom1.Mag();
    Double_t P2 = mom2.Mag();

    Double_t Px1 = mom1.X();
    Double_t Px2 = mom2.X();

    Double_t Py1 = mom1.Y();
    Double_t Py2 = mom2.Y();

    Double_t Pz1 = mom1.Z();
    Double_t Pz2 = mom2.Z();

    Double_t p1p2 = Px1*Px2 + Py1*Py2 + Pz1*Pz2;

    Double_t cos12;

    if( P1*P2 != 0 ){ cos12 = p1p2/(P1*P2) ;  }
    else { cos12 = 0;  }

    return cos12;
}

//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidateSelection::GetIPAngle( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2  ){

    TVector3 pos1;
    TVector3 pos2;
    tr1->Position(pos1);
    tr2->Position(pos2);

    Double_t x1 = pos1.X();
    Double_t x2 = pos2.X();

    Double_t y1 = pos1.Y();
    Double_t y2 = pos2.Y();

    Double_t r1 = sqrt(x1*x1+y1*y1);
    Double_t r2 = sqrt(x2*x2+y2*y2);

    Double_t IPAngle;

    if ( r1*r2!=0 ){ IPAngle = ( x1*x2 + y1*y2 )/(r1*r2); }
    else { IPAngle = 0; }

    return IPAngle;
}

//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
Double_t CbmD0CandidateSelection::GetAPalpha(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 ) // Armenteros - Podolansky plot alpha
{
    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);

    Double_t P1 = mom1.Mag();
    Double_t P2 = mom2.Mag();

    Double_t Px1 = mom1.X();
    Double_t Px2 = mom2.X();

    Double_t Py1 = mom1.Y();
    Double_t Py2 = mom2.Y();

    Double_t Pz1 = mom1.Z();
    Double_t Pz2 = mom2.Z();


	Double_t alpha, ptt;
	Double_t spx = Px1 + Px2;
	Double_t spy = Py1 + Py2;
	Double_t spz = Pz1 + Pz2;
	Double_t sp  = sqrt(spx*spx + spy*spy + spz*spz);

	if( sp == 0.0 ) return 0.;

	Double_t pn, pp, pln, plp;
	if( tr1->GetQp() < 0.) {
	    pn = P1;
	    pp = P2;
	    pln  = (Px1*spx+Py1*spy+Pz1*spz)/sp;
	    plp  = (Px2*spx+Py2*spy+Pz2*spz)/sp;
	}
	else{
	    pn = P2;
	    pp = P1;
	    pln  = (Px2*spx+Py2*spy+Pz2*spz)/sp;
	    plp  = (Px1*spx+Py1*spy+Pz1*spz)/sp;
	}

	if( pn == 0.0 ) return 0.;

	Double_t ptm  = (1.-((pln/pn)*(pln/pn)));
	ptt   = (ptm>=0.)?  pn*sqrt(ptm) :0;
	alpha = (plp-pln)/(plp+pln);

	return alpha;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
Double_t CbmD0CandidateSelection::GetAPptt(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 ) // Armenteros - Podolansky plot Pt
{
    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);

    Double_t P1 = mom1.Mag();
    Double_t P2 = mom2.Mag();

    Double_t Px1 = mom1.X();
    Double_t Px2 = mom2.X();

    Double_t Py1 = mom1.Y();
    Double_t Py2 = mom2.Y();

    Double_t Pz1 = mom1.Z();
    Double_t Pz2 = mom2.Z();


	Double_t alpha, ptt;
	Double_t spx = Px1 + Px2;
	Double_t spy = Py1 + Py2;
	Double_t spz = Pz1 + Pz2;
	Double_t sp  = sqrt(spx*spx + spy*spy + spz*spz);

	if( sp == 0.0 ) return 0.;

	Double_t pn, pp, pln, plp;
	if( tr1->GetQp() < 0.) {
	    pn = P1;
	    pp = P2;
	    pln  = (Px1*spx+Py1*spy+Pz1*spz)/sp;
	    plp  = (Px2*spx+Py2*spy+Pz2*spz)/sp;
	}
	else{
	    pn = P2;
	    pp = P1;
	    pln  = (Px2*spx+Py2*spy+Pz2*spz)/sp;
	    plp  = (Px1*spx+Py1*spy+Pz1*spz)/sp;
	}


	if( pn == 0.0 ) return 0.;

	Double_t ptm  = (1.-((pln/pn)*(pln/pn)));
	ptt   = (ptm>=0.)?  pn*sqrt(ptm) : 0;
	alpha = (plp-pln)/(plp+pln);

	return ptt;
}
//-----------------------------------------------------------------------------------------
   */

ClassImp(CbmD0CandidateSelection)
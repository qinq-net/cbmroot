

#include "CbmD0Candidate.h"
#include "CbmD0CandidatesSE.h"
#include "CbmD0TrackCandidate.h"
#include "TObjArray.h"
#include "CbmMapsFileManager.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "FairRootManager.h"
#include "CbmStsTrack.h"

#include "CbmVertex.h"
#include "TVector3.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatch.h"


//includes from KF
#include "CbmKFParticleInterface.h"
#include "KFParticle.h"

// Includes from C++
#include <iostream>
#include <iomanip>
#include <vector>
#include "CbmKF.h"
#include "TDatabasePDG.h"
//#include <exception>

using namespace std;


// -------------------------------------------------------------------------
CbmD0CandidatesSE::CbmD0CandidatesSE()
    :FairTask(),
    fEventNumber(),
    fcutIPD0(),
    fcutSVZ(),
    fStsTrackMatches(),
    fListMCTracks(),
    fKaonParticleArray(),
    fPionParticleArray(),
    fListMCTracksPos(),
    fListMCTracksNeg(),
    fD0Candidates(),
    fKaonParticles(),
    fKaonBuffer(),
    fPrimVtx(),
    fvtx(),
    fFrameWorkEvent(),
    fNegativeFileName(),
    bTestMode()
{

    Fatal( "CbmD0CandidatesSE: Do not use the standard constructor","Wrong constructor");
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0CandidatesSE::CbmD0CandidatesSE(char* name, Int_t iVerbose, Double_t cutIPD0, Double_t cutSVZ)
  :FairTask(name,iVerbose),
   fEventNumber(),
   fcutIPD0(),
   fcutSVZ(),
   fStsTrackMatches(),
   fListMCTracks(),
   fKaonParticleArray(),
   fPionParticleArray(),
   fListMCTracksPos(),
   fListMCTracksNeg(),
   fD0Candidates(),
   fKaonParticles(),
   fKaonBuffer(),
   fPrimVtx(),
   fvtx(),
   fFrameWorkEvent(),
   fNegativeFileName(),
   bTestMode()
{
    fEventNumber = 0;
    fcutIPD0 = cutIPD0;
    fcutSVZ  = cutSVZ;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0CandidatesSE::~CbmD0CandidatesSE() {

fD0Candidates->Delete();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0CandidatesSE::Finish() {

  

}
// -------------------------------------------------------------------------

 // -----   Private method Register   ---------------------------------------
void CbmD0CandidatesSE::Register() {
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman) Fatal(" CbmD0CandidateSelection::Register",
		      "No FairRootManager");

  fD0Candidates = new TClonesArray("KFParticle",100);
  ioman->Register("CbmD0Candidate", "OpenCharm Superevent", fD0Candidates, IsOutputBranchPersistent("CbmD0Candidate"));

    fStsTrackMatches    = (TClonesArray*) ioman->GetObject("StsTrackMatch");
    fListMCTracks       = (TClonesArray*) ioman->GetObject("MCTrack");
    fKaonParticleArray  = (TClonesArray*) ioman->GetObject("CbmD0KaonParticles");
    fPionParticleArray  = (TClonesArray*) ioman->GetObject("CbmD0PionParticles");
    // Get pointer to PrimaryVertex object from IOManager if it exists
    // The old name for the object is "PrimaryVertex" the new one
    // "PrimaryVertex." Check first for the new name
    fPrimVtx = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex."));
    if (nullptr == fPrimVtx) {
      fPrimVtx = dynamic_cast<CbmVertex*>(ioman->GetObject("PrimaryVertex"));
    }

    fListMCTracksPos    = (TClonesArray*) ioman->GetObject("PositiveMCTracks");
    fListMCTracksNeg    = (TClonesArray*) ioman->GetObject("NegativeMCTracks");

         // --- Fill the buffer ---
    CbmMapsFileManager* file4 = new CbmMapsFileManager( fNegativeFileName, "CbmD0KaonParticles" );
    fKaonBuffer = new TObjArray();
    FillBuffer( file4, fKaonBuffer );
    delete file4;

}
// -------------------------------------------------------------------------  


// -------------------------------------------------------------------------
InitStatus CbmD0CandidatesSE::Init() {

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
//    CbmKF* fKF = CbmKF::Instance();

    fvtx[0] = fPrimVtx->GetX();
    fvtx[1] = fPrimVtx->GetY();
    fvtx[2] = fPrimVtx->GetZ();


    LOG(INFO) <<"CbmD0CandidatesSE: Init completed" << FairLogger::endl;

    return kSUCCESS;
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

void CbmD0CandidatesSE::FillBuffer( CbmMapsFileManager* BackgroundFile, TObjArray* BackgroundArray ){

    if (BackgroundFile==0) {Fatal("CbmD0CandidatesSE:",
				  "Can't fill buffer, background-file not open.");};

    if (BackgroundArray){ BackgroundArray->Delete(); };

    cout << "CbmD0CandidatesSE: Filling background buffer. " <<endl;

    Int_t entries = BackgroundFile->GetNumberOfEventsInFile();
    for (Int_t i=0; i<entries; i++)
    {
	BackgroundArray->AddLast( BackgroundFile->GetEntry(i)->Clone() );
    }
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmD0CandidatesSE::Exec(Option_t* /*option*/){
  
    fEventNumber++;
    Int_t nAcceptedD0 = 0;
//    Bool_t f_particleIsMCD0;
    Int_t crossCheck = 0;

    cout << endl;
    cout << endl;
    cout << "========================================================================================"<<endl;
    cout << endl << "CbmD0CandidatesSE:: Event: " << fEventNumber << endl;

    Int_t nPionTracks = fPionParticleArray->GetEntriesFast();

    Int_t entriesInNegativeFile = fKaonBuffer->GetEntries();
 
    if(nPionTracks==0){
	cout <<" -W- CbmD0CandidatesSE: No Pion Tracks found, ignoring this event." << endl;
	return;
    }
      crossCheck = 0;
      nAcceptedD0 = 0;
    for( Int_t itrNegEvt=0; itrNegEvt<entriesInNegativeFile; itrNegEvt++ )
    {
      
        fKaonParticles = (TClonesArray*) fKaonBuffer->At(itrNegEvt);

	Int_t nKaonTracks = fKaonParticles->GetEntriesFast();
      
	//--- First loop: over negative tracks ---
	for( Int_t itr1=0; itr1<nKaonTracks; itr1++)
	{
	   // cout << "nummber of Kaons: " << nKaonTracks << " Track number " << itr1 << endl;
             KFParticle kaon = *((KFParticle*)fKaonParticles->At(itr1));
	    //--- Second loop: over positive tracks ---
	    for( Int_t itr2=0; itr2<nPionTracks; itr2++ )
	    {
	      //  cout << "nummber of Pions: " << nPionTracks << " Track number " << itr2 << endl;
		KFParticle pion = *((KFParticle*)fPionParticleArray->At(itr2));


		const KFParticle* daughters[2] = {&kaon, &pion};
		KFParticle D0_KF;
                D0_KF.Construct(daughters, 2);

      Double_t SvZ     = D0_KF.GetZ();
      Double_t IPD0    = GetPairImpactParameterR(&D0_KF);
      crossCheck++;
      if( SvZ != SvZ || IPD0 != IPD0 ){cout << "NaN detected!! " << endl;  continue;}         // one of the parameter is NaN

      //--- Apply cuts ---
      if( IPD0 > fcutIPD0 || SvZ < fcutSVZ) continue;    // parameter didn't pass the cuts or

      nAcceptedD0++;
      TClonesArray& clrefD0 = *fD0Candidates;
      Int_t sizeD0 = clrefD0.GetEntriesFast();
      new (clrefD0[sizeD0]) KFParticle(D0_KF);
	    }
	}

    }

    cout << endl << "Number of combinations: " << crossCheck << " Number of candidates in acceptance: "<< nAcceptedD0 <<endl;
    cout << "========================================================================================"<<endl;


}//Exec
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidatesSE::GetPairTx( TVector3& mom1, TVector3& mom2 ){


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
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairTy(TVector3& mom1, TVector3& mom2 ){


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
 // -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
Double_t  CbmD0CandidatesSE::GetPairImpactParameterR(KFParticle* particle)
{
 particle->TransportToPoint(&*fvtx);
 Double_t Pair_IP  = sqrt((particle->GetX()*particle->GetX()) + (particle->GetY()*particle->GetY()) );
 particle->TransportToDecayVertex();
 return  Pair_IP;
}


//-----------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairPt( TVector3& mom1, TVector3& mom2 ){

    Double_t PairPx = mom1.X() + mom2.X();
    Double_t PairPy = mom1.Y() + mom2.Y();

    Double_t PairPt = sqrt( PairPx*PairPx + PairPy*PairPy );

    return PairPt;
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairPz( TVector3& mom1, TVector3& mom2 ){

    Double_t PairPz = mom1.Z() + mom2.Z();

    return PairPz;
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetMomentum( TVector3& mom ){

    Double_t Momentum = mom.Mag();

    return Momentum;
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairMomentum( TVector3& mom1, TVector3& mom2 ){

    Double_t PairPt = GetPairPt( mom1, mom2 );
    Double_t PairPz = GetPairPz( mom1, mom2 );
    Double_t PairMomentum = sqrt( PairPt*PairPt + PairPz*PairPz );

    return PairMomentum;
}
// -------------------------------------------------------------------------
 /*

//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetEnergy1( CbmD0TrackCandidate* tr1){

    TVector3 mom;
    tr1->Momentum(mom);
    CbmStsTrack* stsTr;
    stsTr = (CbmStsTrack*) fNegStsTrackArray->At(tr1->GetTrackIndex());
    Double_t Mass     = TDatabasePDG::Instance()->GetParticle( stsTr->GetPidHypo() )->Mass();
    Double_t Momentum = mom.Mag();
    Double_t Energy   = sqrt(Mass*Mass + Momentum*Momentum);

    return Energy;
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetEnergy2( CbmD0TrackCandidate* tr1){

    TVector3 mom;
    tr1->Momentum(mom);
    CbmStsTrack* stsTr;
    stsTr = (CbmStsTrack*) fPosStsTrackArray->At(tr1->GetTrackIndex());
    Double_t Mass     = TDatabasePDG::Instance()->GetParticle( stsTr->GetPidHypo() )->Mass();
    Double_t Momentum = mom.Mag();
    Double_t Energy   = sqrt(Mass*Mass + Momentum*Momentum);

    return Energy;
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairEnergy( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 ){

    Double_t Energy1 = GetEnergy1(tr1);
    Double_t Energy2 = GetEnergy2(tr2);

    Double_t PairEnergy = Energy1 + Energy2;

    return PairEnergy;
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetIM( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2){

    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);
    CbmStsTrack* stsTr1;
    CbmStsTrack* stsTr2;

    stsTr1 = (CbmStsTrack*) fNegStsTrackArray->At(tr1->GetTrackIndex());

    stsTr2 = (CbmStsTrack*) fPosStsTrackArray->At(tr2->GetTrackIndex());

    Double_t Energy1 = GetEnergy1(tr1);
    Double_t Energy2 = GetEnergy2(tr2);

    Double_t Mass1 = TDatabasePDG::Instance()->GetParticle( stsTr1->GetPidHypo() )->Mass();
    Double_t Mass2 = TDatabasePDG::Instance()->GetParticle( stsTr2->GetPidHypo() )->Mass();

    Double_t Px1 = mom1.X();
    Double_t Px2 = mom2.X();

    Double_t Py1 = mom1.Y();
    Double_t Py2 = mom2.Y();

    Double_t Pz1 = mom1.Z();
    Double_t Pz2 = mom2.Z();

    Double_t p1p2 = Px1*Px2 + Py1*Py2 + Pz1*Pz2;
    Double_t IM   = sqrt( Mass1*Mass1 + Mass2*Mass2 + 2*( Energy1*Energy2 - p1p2 ) );

    return IM;
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetCos12( TVector3& mom1, TVector3& mom2 ){

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
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetIPAngle(  TVector3& pos1, TVector3& pos2  ){

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
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------

void CbmD0CandidatesSE::GetAP( TVector3 &mom1, TVector3 &mom2, Double_t Qp, Double_t &alpha, Double_t  &ptt ) // Armenteros - Podolansky plot
{
    alpha=0;
    ptt=0;

    Double_t P1 = mom1.Mag();
    Double_t P2 = mom2.Mag();

    Double_t Px1 = mom1.X();
    Double_t Px2 = mom2.X();

    Double_t Py1 = mom1.Y();
    Double_t Py2 = mom2.Y();

    Double_t Pz1 = mom1.Z();
    Double_t Pz2 = mom2.Z();


    Double_t spx = Px1 + Px2;
    Double_t spy = Py1 + Py2;
    Double_t spz = Pz1 + Pz2;
    Double_t sp  = sqrt(spx*spx + spy*spy + spz*spz);


    if( sp == 0.0 ) return;

    Double_t pn, pp, pln, plp;
    if( Qp < 0.) {
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

    if( pn == 0.0 ) return;

    Double_t ptm  = (1.-((pln/pn)*(pln/pn)));
    ptt   = (ptm>=0.)?  pn*sqrt(ptm) :0;
    alpha = (plp-pln)/(plp+pln);

}
// -------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------

Double_t CbmD0CandidatesSE::BoostMomentum( TVector3& mom1, TVector3& mom2, CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 ){

    Double_t Energy1 = GetEnergy1(tr1);
    Double_t Energy2 = GetEnergy2(tr2);

    // --- particle 4-vectors in the lab frame ---
    TLorentzVector v1( mom1.X(), mom1.Y(), mom1.Z(), Energy1 );
    TLorentzVector v2( mom2.X(), mom2.Y(), mom2.Z(), Energy2 );

    TLorentzVector v3 = v1+v2;

    // --- components of the mother particle ---
    Double_t px = v3.Px();
    Double_t py = v3.Py();
    Double_t pz = v3.Pz();

    Double_t Mass = TDatabasePDG::Instance()->GetParticle( 421 )->Mass(); //--- 421: PDG code of D0
    Double_t E  = sqrt( Mass*Mass + px*px + py*py + pz*pz );

    // ---
    TVector3 b( -px/E, -py/E, -pz/E );


    // --- Calculate momenta in the CM frame ---
    v1.Boost(b);
    v2.Boost(b);

    // --- angle of daughter particles in the CM frame ---
    Double_t P1DotP2 = v1.X()*v2.X() + v1.Y()*v2.Y() + v1.Z()*v2.Z();
    Double_t modP1   = sqrt( v1.X()*v1.X() + v1.Y()*v1.Y() + v1.Z()*v1.Z() );
    Double_t modP2   = sqrt( v2.X()*v2.X() + v2.Y()*v2.Y() + v2.Z()*v2.Z() );

    Double_t cosA;

    if( modP1 * modP2 != 0 ) cosA = ( P1DotP2 )/( modP1 * modP2 );
    else cosA = 1;

    //if (cosA < -0.99) cout <<setprecision(8)<< "cosA " << cosA << endl;

    return cosA;
}
*/
//-----------------------------------------------------------------------------------------
ClassImp(CbmD0CandidatesSE)

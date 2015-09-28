

//includes from cbm
#include "CbmStsTrack.h"
#include "CbmVertex.h"
#include "CbmStsKFTrackFitter.h"
#include "CbmMCTrack.h"
#include "CbmKFTrack.h"
#include "CbmTrackMatch.h"
#include "CbmD0CandidateSelection.h"
#include "CbmD0Candidate.h"
#include "CbmD0TrackCandidate.h"
#include "CbmD0Tools.h"


//includes from ROOT
#include "TNtuple.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "TH1F.h"
#include "TH2F.h"


//includes from Fair
#include "FairRootManager.h"


//includes from KF
#include "CbmKF.h"
#include "CbmKFMath.h"
#include "CbmKFHit.h"
#include "CbmKFTrack.h"
#include "CbmKFVertex.h"
#include "KFParticle.h"
#include "KFPVertex.h"
#include "CbmKFFieldMath.h"
#include "CbmKFTrackInterface.h"
#include "CbmKFSecondaryVertexFinder.h"
#include "CbmKFParticleInterface.h"


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
    CbmD0CandidateSelection::CbmD0CandidateSelection(){
    
    Fatal( "CbmD0Candidates: Do not use the standard constructor","");
    
    }
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0CandidateSelection::CbmD0CandidateSelection(char* name, Int_t iVerbose, Double_t cutIPD0, Double_t cutSVZ):FairTask(name,iVerbose){

    fInfoArray            = new TClonesArray("FairTrackParam",1);
    fFit                  = new CbmStsKFTrackFitter();
    fEventNumber = 0;
    fHistoFileName = "Histo.root";
    fcutIPD0 = cutIPD0;
    fcutSVZ  = cutSVZ;
    fUseKF   = kTRUE;
    bTestMode = kFALSE;

    // ---  Histos for control ---
    test = new TH1F("test", "test", 1000, 0, 3 );
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0CandidateSelection::~CbmD0CandidateSelection() {
   
    delete test;
}
// -------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void CbmD0CandidateSelection::Finish() {
 
}
// -------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
InitStatus CbmD0CandidateSelection::Init() {

    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman) Fatal("Register", "No FairRootManager");
    Register();

    fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
    fStsTrackArray   = (TClonesArray*) ioman->GetObject("StsTrack");
    fD0TrackArray    = (TClonesArray*) ioman->GetObject("CbmD0TrackCandidate");
    fPosStsTracks    = (TClonesArray*) ioman->GetObject("PositiveStsTracks");
    fNegStsTracks    = (TClonesArray*) ioman->GetObject("NegativeStsTracks");
    fListMCTracks    = (TClonesArray*) ioman->GetObject("MCTrack");

    if(! fStsTrackArray) {Fatal("CbmD0CandidateSelection: StsTrackArray not found"," That's bad. ");}
    if(! fD0TrackArray)  {Fatal("CbmD0CandidateSelection: D0TrackArray not found"," That's bad. ");}
    fSecVertexFinder = new CbmKFSecondaryVertexFinder();
    fFit             = new CbmStsKFTrackFitter();
    fFit->Init();
 
    fPrimVtx         = (CbmVertex*) ioman->GetObject("PrimaryVertex");
    if (!fPrimVtx){
	cout << "-W- : CbmD0CandidateSelection - No primary Vtx, assume (0,0,0)" << endl;
	fPrimVtx=new CbmVertex("PrimaryVertex","PrimaryVertex");
    }
    
    CbmKF* fKF = CbmKF::Instance();
    if(fKF)fMF = fKF->GetMagneticField();
    else return kFATAL;

    tools = new CbmD0Tools();
    kfpInterface = new CbmKFParticleInterface();

    return kSUCCESS;
}
// -------------------------------------------------------------------------


 // -------------------------------------------------------------------------
void CbmD0CandidateSelection::Exec(Option_t* option){

    fEventNumber++;
    Int_t D0counter;
    Bool_t f_particleIsMCD0 = kFALSE;
    Int_t nAcceptedD0 = 0;
    TVector3 vertex1; //cdritsa 17/04/09
    TVector3 vertex2; //cdritsa 17/04/09

    cout << endl; 
    cout << endl;
    cout << "========================================================================================"<<endl;
    cout << endl << "CbmD0CandidateSelection:: Event: " << fEventNumber << endl;


    fD0Candidates->Clear();

    Int_t nTracks    = fD0TrackArray->GetEntriesFast();
    cout << "nTracks: " << nTracks << endl;

    Int_t nPosTracks = fPosStsTracks->GetEntriesFast();
    cout << "nPosTracks: " << nPosTracks << endl;

    Int_t nNegTracks = fNegStsTracks->GetEntriesFast();
    cout << "nNegTracks: " << nNegTracks << endl;

    CbmD0TrackCandidate * track1;
    CbmD0TrackCandidate * track2;

    if (fVerbose>0) printf("CbmD0CandidateSelection:: Entries:%i\n",nTracks);

    if(nTracks==0){
	cout <<" -W- CbmD0CandidateSelection:: No D0Tracks found, ignoring this event." << endl;
	return;
    }

    CbmStsTrack* stsTrack1;
    CbmStsTrack* stsTrack2;
    CbmMCTrack*  mcTrack1;
    CbmMCTrack*  mcTrack2;
    TVector3 mom1;
    TVector3 mom2;
    CbmVertex vtx;
    CbmVertex vtxT;
    CbmVertex kf_vertex;
    Double_t mass, merr, ct, cterr, l, lerr, p, perr;

    CbmKFVertex PV( *fPrimVtx );
    Int_t crossCheck=0;
    Int_t mcPid1 = -1;
    Int_t mcPid2 = -1;
    Int_t signal = 0;
    Double_t SvXErr = 0.;
    Double_t SvYErr = 0.;
    Double_t SvChiT = 0.;
    Double_t SvZT = 0.;
    Double_t cosA = 0.;


    for( Int_t itr1=0; itr1<nTracks; itr1++)
       {
       track1    = (CbmD0TrackCandidate*) fD0TrackArray->At(itr1);
       if (track1->GetPidHypo() != -321)
          continue;
       track1->Momentum(mom1);
       stsTrack1 = (CbmStsTrack*) fStsTrackArray->At( track1->GetTrackIndex() );

    for( Int_t itr2=0; itr2<nTracks; itr2++)
       {
       track2    = (CbmD0TrackCandidate*) fD0TrackArray->At(itr2);
       if (track2->GetPidHypo() != 211)
           continue;
       track2->Momentum(mom2);
       stsTrack2 = (CbmStsTrack*) fStsTrackArray->At( track2->GetTrackIndex() );

            if(bTestMode)
               {

               	 mcTrack1  = (CbmMCTrack*) fListMCTracks->At( track1->GetTrackIndexMC() );
               	 mcTrack2  = (CbmMCTrack*) fListMCTracks->At( track2->GetTrackIndexMC() );

      	   	 mcTrack1->GetStartVertex(vertex1);
	   	 mcTrack2->GetStartVertex(vertex2);

	   	 mcPid1    = mcTrack1->GetPdgCode();
	   	 mcPid2    = mcTrack2->GetPdgCode();
	    

	   	 // ------------ Identify Signal on MC base --------------
	   	 if (  ( track1->GetQp() * track2->GetQp() < 0 )
			&&
			( ( ( TMath::Abs( mcPid1) == 211 ) && ( TMath::Abs( mcPid2 ) == 321 ) ) ||
			  ( ( TMath::Abs( mcPid1) == 321 ) && ( TMath::Abs( mcPid2 ) == 211 ) )
			)
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
			D0counter++;
                        signal = 1;
	   	 }
	   	 else{
		     f_particleIsMCD0=kFALSE;

	   	 }
		}


                if (fVerbose>0) cout <<endl<< "||--- KF ---||" << endl;;

		crossCheck=crossCheck+1;

		// --- using KFPartical methode
		KFParticle* particle1 = new KFParticle(); 
		if (fVerbose>0)  cout << "new Particle with id "<< track1->GetPidHypo();
		kfpInterface->SetKFParticleFromStsTrack(&*stsTrack1, particle1, track1->GetPidHypo());

		
		KFParticle* particle2 = new KFParticle();	
               if (fVerbose>0)  cout <<endl<< "new Particle 2  with id "<< track2->GetPidHypo();
        	kfpInterface->SetKFParticleFromStsTrack(&*stsTrack2, particle2, track2->GetPidHypo());


		KFParticle* D0_KF = new KFParticle(*particle1, *particle2);
	         if (fVerbose>0)  cout << endl << "Found new possible D0 with mass: " << D0_KF->GetMass();
		 
		mass = D0_KF->GetMass();
		l    = D0_KF->GetDecayLength();
		lerr = D0_KF->GetErrDecayLength();
		ct   = D0_KF->GetLifeTime();
		cterr= D0_KF->GetErrLifeTime();
		p    = D0_KF->GetMomentum();

		D0_KF->TransportToDecayVertex();

		// --- Define variables of the Ntuple ---
		Double_t p1      = mom1.Mag();
		Double_t p2      = mom2.Mag();
		Double_t pt1     = sqrt(mom1.X()*mom1.X() + mom1.Y()*mom1.Y());
		Double_t pt2     = sqrt(mom2.X()*mom2.X() + mom2.Y()*mom2.Y());
		Double_t PV1     = track1->GetPVSigma();
		Double_t PV2     = track2->GetPVSigma();
		Double_t IP1     = sqrt( track1->GetX()*track1->GetX() + track1->GetY()*track1->GetY() );
		Double_t IP2     = sqrt( track2->GetX()*track2->GetX() + track2->GetY()*track2->GetY() );
		Double_t imx1    = track1->GetX();
		Double_t imx2    = track2->GetX();
		Double_t imy1    = track1->GetY();
		Double_t imy2    = track2->GetY();
		Double_t SvChi   = D0_KF->GetChi2() / D0_KF->GetNDF();
		Double_t SvZ     = D0_KF->GetZ();
		//Double_t SvChiT  = vt.GetChi2()/vt.GetNDF();
		//Double_t SvZT    = vt.Z();
		Double_t SvX     = D0_KF->GetX();
		Double_t SvY     = D0_KF->GetY();
		Double_t PvZ     = fPrimVtx->GetZ();
		Double_t Pair_Tx = GetPairTx( track1, track2);
		Double_t Pair_Ty = GetPairTy( track1, track2);
		Double_t IPD0    = GetPairImpactParameterR( Pair_Tx, Pair_Ty, SvX, SvY, SvZ, PvZ );
		Double_t cos12   = GetCos12(track1, track2);
		Double_t IPAngle = GetIPAngle(track1, track2);
		Double_t SvZErr  = (vertex1.Z() - D0_KF->GetZ());
		Double_t ptD0    = GetPairPt(track1, track2);
		Double_t pzD0    = GetPairPz(track1, track2);
		Double_t ptt     = GetAPptt(track1, track2);
		Double_t alpha   = GetAPalpha(track1, track2);
                Double_t rapidity= D0_KF->GetRapidity();
		//--- Apply cuts ---
		if( IPD0 > fcutIPD0 || SvZ < fcutSVZ ) continue;

		nAcceptedD0 ++;


		  new((*fD0Candidates)[fD0Candidates->GetEntriesFast()]) CbmD0Candidate(signal,
											track1->GetPidHypo(), p1, pt1, PV1, IP1, imx1, imy1, stsTrack1->GetNofMvdHits(), stsTrack1->GetNofStsHits(),
											track2->GetPidHypo(), p2, pt2, PV2, IP2, imx2, imy2, stsTrack2->GetNofMvdHits(), stsTrack2->GetNofStsHits(),
											SvChi, SvZ, IPD0, mass, cos12, IPAngle,
											SvXErr, SvYErr, SvZErr, ptD0, pzD0, SvChiT, SvZT, ptt,
											alpha, PvZ, cosA, rapidity);


	 

	}// second for loop

    }// first for loop

  
    cout << endl << "Number of combinations: " << crossCheck << " Number of candidates in acceptance: "<< nAcceptedD0 <<endl;
    cout << "========================================================================================"<<endl;
}//Exec
//-----------------------------------------------------------------------------------------




 // -----   Private method Register   ---------------------------------------
void CbmD0CandidateSelection::Register() {
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman) Fatal(" CbmD0CandidateSelection::Register",
		      "No FairRootManager");

  fD0Candidates = new TClonesArray("CbmD0Candidate",100);
  ioman->Register("CbmD0Candidate", "OpenCharm", fD0Candidates, kTRUE);

}
// -------------------------------------------------------------------------  



//-----------------------------------------------------------------------------------------

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
Double_t  CbmD0CandidateSelection::GetPairImpactParameterR( Double_t Pair_Tx, Double_t Pair_Ty, Double_t SvX, Double_t SvY, Double_t SvZ, Double_t PvZ ){

    Double_t Pair_IPx = SvX - Pair_Tx*(SvZ - PvZ);
    Double_t Pair_IPy = SvY - Pair_Ty*(SvZ - PvZ);

    Double_t Pair_IP  = sqrt( Pair_IPx*Pair_IPx + Pair_IPy*Pair_IPy );

    return  Pair_IP;
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

    TVector3 mom;
    tr1->Momentum(mom);
    CbmStsTrack* stsTr;
    stsTr = (CbmStsTrack*) fStsTrackArray->At(tr1->GetTrackIndex());

    Double_t Mass;

    if (tr1->GetQp()>0){Mass=0.1395;}
    if (tr1->GetQp()<0){Mass=0.4936;}

//    Double_t Mass     = TDatabasePDG::Instance()->GetParticle( stsTr->GetPidHypo() )->Mass();
    Double_t Momentum = mom.Mag();
    Double_t Energy   = sqrt(Mass*Mass + Momentum*Momentum);

    return Energy;
}    // T.m = TDatabasePDG::Instance()->GetParticle(T.pdg)->Mass();

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

    TVector3 mom1;
    TVector3 mom2;
    tr1->Momentum(mom1);
    tr2->Momentum(mom2);
    CbmStsTrack* stsTr1;
    CbmStsTrack* stsTr2;
    stsTr1 = (CbmStsTrack*) fStsTrackArray->At(tr1->GetTrackIndex());
    stsTr2 = (CbmStsTrack*) fStsTrackArray->At(tr2->GetTrackIndex());
    Double_t Energy1 = GetEnergy(tr1);
    Double_t Energy2 = GetEnergy(tr2);

   // Double_t Mass1 = TDatabasePDG::Instance()->GetParticle( stsTr1->GetPidHypo() )->Mass();
   // Double_t Mass2 = TDatabasePDG::Instance()->GetParticle( stsTr2->GetPidHypo() )->Mass();
    Double_t Mass1=0;
    Double_t Mass2=0;

    if (tr1->GetQp()>0){Mass1=0.1395;}
    if (tr1->GetQp()<0){Mass1=0.4936;}

    if (tr2->GetQp()>0){Mass2=0.1395;}
    if (tr2->GetQp()<0){Mass2=0.4936;}


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


ClassImp(CbmD0CandidateSelection)
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
#include "CbmStsKFTrackFitter.h"
#include "CbmMCTrack.h"
#include "CbmKFTrack.h"
#include "TH1F.h"
#include "TH2F.h"
#include "CbmTrackMatch.h"
//#include "CbmD0Candidates.h"
//#include "CbmD0TrackCandidate.h"
//#include "CbmKFParticle.h"
#include "TNtuple.h"

//includes from KF
#include "CbmKFMath.h"
#include "CbmKFHit.h"
#include "CbmKFTrack.h"
#include "CbmKFVertex.h"
#include "CbmKFParticleInterface.h"
#include "CbmKFTrackInterface.h"
#include "CbmStsKFSecondaryVertexFinder.h"
#include "KFParticle.h"
#include "CbmKFFieldMath.h"

// Includes from C++
#include <iostream>
#include <iomanip>
#include <vector>
#include "CbmKF.h"
#include "TDatabasePDG.h"
//#include <exception>

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
ClassImp(CbmD0CandidatesSE)

    // -------------------------------------------------------------------------
    CbmD0CandidatesSE::CbmD0CandidatesSE(){
    
    Fatal( "CbmD0CandidatesSE: Do not use the standard constructor","");
    
    }
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0CandidatesSE::CbmD0CandidatesSE(char* name, Int_t iVerbose, Bool_t SuperEventMode, Double_t cutIPD0max, Double_t cutSVZmin, Double_t cutSVZmax):FairTask(name,iVerbose){

    //fPairs           = new TClonesArray("CbmD0Candidate",1);
    fPairs           = new TClonesArray("CbmD0Candidate");
    fFit             = new CbmStsKFTrackFitter();
    fEventNumber = 0;
    fHistoFileName = "Histo.root";
    fNegativeFileName = "out.test10.root";
    fcutIPD0max = cutIPD0max;
    fcutSVZmin  = cutSVZmin;
    fcutSVZmax  = cutSVZmax;


    fBufferD0TrackArraysN  = new TObjArray();
    fBufferStsTrackArraysN = new TObjArray();
    fBufferMCTrackArraysN  = new TObjArray();

    fSuperEventMode = SuperEventMode;
    fFrameWorkEvent = 0;   // Do not change this!


}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0CandidatesSE::~CbmD0CandidatesSE() {


    if ( fPairs) {
	fPairs->Delete();
	delete fPairs;
    }

    delete fBufferD0TrackArraysN;
    delete fBufferStsTrackArraysN;
    delete fBufferMCTrackArraysN;
    delete fFit;
    //delete fTrackPairNtuple;
    //DELETE all NEW!!!
}
// -------------------------------------------------------------------------


void CbmD0CandidatesSE::Finish() {
    //fInfoArray->Write();
    //fInfoArray->Delete();
    //fTrackPairNtuple->Write();
    //fTrackPairNtuple->Delete();
    //fPairs->Write();
    fPairs->Delete();

}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
InitStatus CbmD0CandidatesSE::Init() {

    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman) Fatal("Register", "No FairRootManager");

    fStsTrackMatches  = (TClonesArray*) ioman->GetObject("StsTrackMatch");

    //--- Arrays containing POSITIVE tracks ---
    fPosStsTrackArray = (TClonesArray*) ioman->GetObject("PositiveStsTracks");
    fPosD0TrackArray  = (TClonesArray*) ioman->GetObject("CbmD0TrackCandidateP");
    fPosMCTrackArray  = (TClonesArray*) ioman->GetObject("PositiveMCTracks");

    //--- Arrays containing NEGATIVE tracks ---
    //fNegStsTrackArray = (TClonesArray*) ioman->GetObject("NegativeStsTracks");
    //fNegD0TrackArray  = (TClonesArray*) ioman->GetObject("CbmD0TrackCandidate");
    //fNegMCTrackArray  = (TClonesArray*) ioman->GetObject("NegativeMCTracks");

    fAllD0TrackArray = (TClonesArray*) ioman->GetObject("CbmD0TrackCandidate");

   

    if( ! fPosStsTrackArray ) { Fatal("CbmD0Candidates: fPosStsTrackArray not found", " That's bad." ) ;}
    if( ! fPosMCTrackArray  ) { Fatal("CbmD0Candidates: fPosMCTrackArray not found",  " That's bad." ) ;}
    if( ! fPosD0TrackArray  ) { Fatal("CbmD0Candidates: fPosD0TrackArray not found",  " That's bad." ) ;}
    //cout <<"ping0"<< endl;
    fPrimVtx         = (CbmVertex*)    ioman->GetObject("PrimaryVertex");
    //cout <<"ping1"<< endl;
    fListMCTracks    = (TClonesArray*) ioman->GetObject("MCTrack");
    //cout <<"ping2" << endl;
    fSVF             = new CbmStsKFSecondaryVertexFinder();
    //cout <<"ping3" << endl;

    fFit             = new CbmStsKFTrackFitter();
    fFit->Init();
    //cout <<"ping4" << endl;

    fMF = CbmKF::Instance()->GetMagneticField();
    fListMCTracks    = (TClonesArray*) ioman->GetObject("MCTrack");
    //cout << "Ping " << endl;

    if (!fPrimVtx) {Fatal("CbmD0Candidates: fPrimVtx not found", " That's bad." ) ;}
    if (!fListMCTracks) {Fatal("CbmD0Candidates: fListMCTracks not found", " That's bad." ) ;}
    if (!fMF) {Fatal("CbmD0Candidates: Magnetic field  not found", " That's bad." ) ;}

    kfpInterface = new CbmKFParticleInterface();


    // **********  Register output array
    fPairs = new TClonesArray("CbmD0Candidate", 10000);
    ioman->Register("CbmD0Candidate", "MVD", fPairs, kTRUE);

     // --- Fill the buffer ---
    CbmMapsFileManager* file1 = new CbmMapsFileManager( fNegativeFileName, "CbmD0TrackCandidateN" );
    FillBuffer( file1, fBufferD0TrackArraysN );
    delete file1;

    CbmMapsFileManager* file2 = new CbmMapsFileManager( fNegativeFileName, "NegativeStsTracks" );
    FillBuffer( file2, fBufferStsTrackArraysN );
    delete file2;

    CbmMapsFileManager* file3 = new CbmMapsFileManager( fNegativeFileName, "NegativeMCTracks" );
    FillBuffer( file3, fBufferMCTrackArraysN );
    delete file3;

     CbmMapsFileManager* file4 = new CbmMapsFileManager( fNegativeFileName, "KFParticles" );
    FillBuffer( file4, fKFParticleBuffer );
    delete file4;

    cout << "-I- CbmD0Candidates: Buffers filled " << endl;

    return kSUCCESS;
    cout <<"-I-  CbmD0CandidatesSE: Init completed" << endl;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmD0CandidatesSE::Exec(Option_t* option){
    // Clear output array
    fPairs->Clear("C");

    fEventNumber++;
    Int_t D0counter = 0;
    Bool_t f_particleIsMCD0;
    float pairInfo[30];
    Int_t    signal=-3;

    TVector3 vertex1; //cdritsa 17/04/09
    TVector3 vertex2; //cdritsa 17/04/09


    if (!fPrimVtx){
	cout << "-W- : CbmD0Candidates - No primary Vtx!" << endl;
	//fPrimVtx=new CbmVertex("PrimaryVertex","PrimaryVertex");
    }


    //if(fEventNumber%200==1)printf("\nCbmD0CandidatesSE: Event:%i\n", fEventNumber);
    printf("\nCbmD0CandidatesSE: Event:%i\n", fEventNumber);

    //fInfoArray->Clear();
   // cout << endl << "read number of Tracks" << endl;
    Int_t nTracks = fAllD0TrackArray->GetEntriesFast();
    // Readout the number of entries
   // cout << endl << "read number buffer entries " << endl;
    Int_t entriesInNegativeFile = fBufferD0TrackArraysN->GetEntries();
    //cout << "Number of Events In Negative File: " << entriesInNegativeFile << endl;
   // cout << endl << "read number pos Tracks " << endl;
    Int_t nTracksP = fPosD0TrackArray->GetEntriesFast();
    //cout << "\tnTracksP  " << nTracksP << endl;

    CbmD0TrackCandidate * track1;
    CbmD0TrackCandidate * track2;

    if (fVerbose>0) printf("CbmD0CandidatesSE: Entries:%i\n",nTracks);

    if(nTracks==0){
	cout <<" -W- CbmD0CandidatesSE: No D0Tracks found, ignoring this event." << endl;
	return;
    }

    TVector3 pos1;
    TVector3 pos2;
    TVector3 mom01;
    TVector3 mom02;
    TVector3 v;
    CbmVertex vtx;
    CbmVertex vtxT;

    CbmStsTrack* stsTrack1;
    CbmStsTrack* stsTrack2;
    CbmMCTrack*  mcTrack1;
    CbmMCTrack*  mcTrack2;
    KFParticle* particle1;
    KFParticle* particle2;
    TVector3 mom1;
    TVector3 mom2;
    //CbmVertex vtx;
    //CbmVertex vtxT;
    CbmKFVertex kf_vertex;
    CbmKFVertex kf_vertexT;
    Double_t mass, merr, ct, cterr, l, lerr, p, perr;
    vector<CbmKFTrackInterface*> tt;
    CbmKFVertex PV( *fPrimVtx );
    Double_t SvChiT = 0.;
    Double_t SvZT = 0.;



    for( Int_t itrNegEvt=0; itrNegEvt<entriesInNegativeFile; itrNegEvt++ ){
    //for( Int_t itrNegEvt=0; itrNegEvt<20; itrNegEvt++ ){

	if( !fSuperEventMode && !(fFrameWorkEvent == itrNegEvt) ) continue;
        //cout << "\r itrNegEvt " << itrNegEvt << flush;
       // cout << endl << "get Array neg D0 Tracks" << endl;
	fNegD0TrackArray  = (TClonesArray*) fBufferD0TrackArraysN->At(itrNegEvt);
        //cout << endl << "get Array neg sts Tracks" << endl;
	fNegStsTrackArray = (TClonesArray*) fBufferStsTrackArraysN->At(itrNegEvt);
       // cout << endl << "get Array neg Mc Tracks" << endl;
	fNegMCTrackArray  = (TClonesArray*) fBufferMCTrackArraysN->At(itrNegEvt);
       // cout << endl << "got all Arrays, mc array size is " << fNegMCTrackArray->GetEntriesFast() << endl;
	fKFParticles = (TClonesArray*) fKFParticleBuffer->At(itrNegEvt);

	Int_t nTracksN = fNegD0TrackArray->GetEntriesFast();
	//cout << "\r itrNegEvt " << itrNegEvt << " ... nTracksN " << nTracksN <<" "<< flush;
        //cout << "\tnTracksP  " << nTracksP << "\tnTracksN  " << nTracksN << endl;

	//--- First loop: over negative tracks ---
	for( Int_t itr1=0; itr1<nTracksN; itr1++)
	{
           // cout << endl << " try to get negativ candidate" << endl;
	    track1    = (CbmD0TrackCandidate*) fNegD0TrackArray->At(itr1);
	   // cout << endl << "try to get corresponding sts Track" << endl;
	    stsTrack1 = (CbmStsTrack*) fNegStsTrackArray->At( track1->GetTrackIndex() );
         
	    if (track1->GetTrackIndexMC() != -1)
           // cout << endl << "try to get corresponding match" << endl;
	     mcTrack1  = (CbmMCTrack*) fNegMCTrackArray->At( track1->GetTrackIndexMC() );
         
            Double_t Qp = track1->GetQp();

	    //--- Second loop: over positive tracks ---
	    for( Int_t itr2=0; itr2<nTracksP; itr2++ )
	    {
               // cout << endl << "try to get positiv candidate" << endl;
	       track2    = (CbmD0TrackCandidate*) fPosD0TrackArray->At(itr2);
                //cout << endl << "try to get corresponding sts track" << endl;
		stsTrack2 = (CbmStsTrack*) fPosStsTrackArray->At( track2->GetTrackIndex() );
               
		if (track2->GetTrackIndexMC() != -1)
               // cout << endl << "try to get match" << endl;
	         mcTrack2  = (CbmMCTrack*)   fPosMCTrackArray->At( track2->GetTrackIndexMC() );
             
		//---------------------------------------------
               

		track1->Momentum(mom01);
		track2->Momentum(mom02);
		track1->Position(pos1);
		track2->Position(pos2);

		Double_t IPAngle = GetIPAngle( pos1, pos2 );
		if( IPAngle > -0.8 ) continue;

                Double_t IM      = GetIM( track1, track2 );


		//if( IM<1.7 || IM>3.5 ){continue;}

		//---  Armenteros Podolansky ------------------
		Double_t ptt     = 0;
		Double_t alpha   = 0;
		GetAP( mom01, mom02, Qp, alpha, ptt );
		//if( ptt < 0.22 ) continue;
		//----------------------------------------------

		Double_t cosA = BoostMomentum( mom01, mom02, track1, track2 );
		//if (cosA > -0.80) continue;
	    
               /* cout << endl << "try to make new particle " << endl;
		KFParticle* particle1 = new KFParticle();
		if (fVerbose>0)  cout <<endl<< "new Particle 1 with id "<< track1->GetPidHypo();
		cout << endl << stsTrack1->GetNofHits() << "    " << stsTrack1->GetNofMvdHits() << endl;
	        kfpInterface->SetKFParticleFromStsTrack(&*stsTrack1, particle1, track1->GetPidHypo());
		cout << endl << "set new particle1 with mass: " << particle1->GetMass() << endl;

	       // if(particle1->GetMass() == 0) continue;

		KFParticle* particle2 = new KFParticle();	
		if (fVerbose>0)  cout <<endl<< "new Particle 2  with id "<< track2->GetPidHypo();
                 cout << endl << stsTrack2->GetNofHits() << "    " << stsTrack2->GetNofMvdHits() << endl;
                kfpInterface->SetKFParticleFromStsTrack(&*stsTrack2, particle2, track2->GetPidHypo());
		cout << endl << "set new particle2 with mass: " << particle2->GetMass() << endl;

               // if(particle2->GetMass() == 0) continue;
                                                           */
                cout << endl << "try to get D0" << endl;
		KFParticle* D0_KF = new KFParticle(*particle1, *particle2);
		cout << endl << "got new D0" << endl;
	        D0_KF->Print();

	        if (fVerbose>0)  cout << endl << "Found new possible D0 with mass: " << D0_KF->GetMass();
		
		D0_KF->TransportToDecayVertex();
 
		mass = D0_KF->GetMass();
		l    = D0_KF->GetDecayLength();
		lerr = D0_KF->GetErrDecayLength();
		ct   = D0_KF->GetLifeTime();
		cterr= D0_KF->GetErrLifeTime();
		p    = D0_KF->GetMomentum();


	        //---------------------------------------------
        	track1->Momentum(mom1);
		track2->Momentum(mom2);
	       

		if(mcTrack1)mcTrack1->GetStartVertex(vertex1);
		if(mcTrack2)mcTrack2->GetStartVertex(vertex2);


                //------------------------------------------------------
		// *********** Fill the TTree and apply cuts ***********
                //------------------------------------------------------
   		Int_t    Pid1    = track1->GetPidHypo();
		Int_t    Pid2    = track2->GetPidHypo();
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

		Double_t cos12   = GetCos12( mom1, mom2 );
		//Double_t SvZErr  = (mcTrack1->GetStartVertex()).Z()- v.Z();
                Double_t SvX     = D0_KF->GetX();
		Double_t SvY     = D0_KF->GetY();
		Double_t SvXErr  = (vertex1.X() - v.X());
		Double_t SvYErr  = (vertex1.Y() - v.Y());
		Double_t SvZErr  = (vertex1.Z() - v.Z());
	        Double_t ptD0    = GetPairPt( mom1, mom2 );
		Double_t pzD0    = GetPairPz( mom1, mom2 );

		Int_t nMvdHits1 = stsTrack1->GetNofMvdHits();
                Int_t nStsHits1 = stsTrack1->GetNofHits();

		Int_t nMvdHits2 = stsTrack2->GetNofMvdHits();
                Int_t nStsHits2 = stsTrack2->GetNofHits();

		Double_t PvZ     = fPrimVtx->GetZ();
		Double_t Pair_Tx = GetPairTx( mom1, mom2 );
		Double_t Pair_Ty = GetPairTy( mom1, mom2 );
		Double_t IPD0    = GetPairImpactParameterR( Pair_Tx, Pair_Ty, SvX, SvY, SvZ, PvZ );
                Double_t rapidity= D0_KF->GetRapidity();
  		//--- cut on the impact parameter of the D0 ---
		if( IPD0 > fcutIPD0max ) continue;

        	//--- cut on the secondary vertex ---
		if ( SvZ < fcutSVZmin || SvZ > fcutSVZmax ) continue;

          

		// ------------ Identify Signal on MC base --------------
                // -------- Valid ONLY for Event by Event Analysis ------

		if ( !fSuperEventMode && ( track1->GetQp() * track2->GetQp() < 0 )
		    &&
		    ( ( ( TMath::Abs( Pid1 ) == 211 ) && ( TMath::Abs( Pid2 ) == 321 ) ) ||
		      ( ( TMath::Abs( Pid1 ) == 321 ) && ( TMath::Abs( Pid2 ) == 211 ) )
		    )
		    &&
		    ( vertex1.Z() > 0.000001  &&  vertex2.Z() > 0.000001  )
		    &&
		    ( ( mcTrack1->GetMotherId() == -1 ) && ( mcTrack2->GetMotherId() == -1) )
		   )
		{
		    f_particleIsMCD0=kTRUE;
		    cout << "\n----------------------------------" << endl;
		    cout << "********< MC-D0 detected >********" << endl;
		    cout << "----------------------------------" << endl;
		    D0counter++;
		}
		else{
		    f_particleIsMCD0=kFALSE;
		}

		if (f_particleIsMCD0) { signal = 1; } else { signal = 0; };
		Int_t nPairs = fPairs->GetEntriesFast();
		new ((*fPairs)[nPairs])
		    CbmD0Candidate(signal,
				   Pid1,  p1,  pt1,  PV1,  IP1,  imx1,  imy1, nMvdHits1, nStsHits1,
				   Pid2,  p2,  pt2,  PV2,  IP2,  imx2,  imy2, nMvdHits2, nStsHits2,
				   SvChi,   SvZ,   IPD0,  IM,      cos12,  IPAngle,
				   SvXErr, SvYErr, SvZErr,  ptD0,  pzD0,  SvChiT,  SvZT,   ptt,
				   alpha,   PvZ,   cosA, rapidity);
		particle1->Delete();
		particle2->Delete();
                D0_KF->Delete();
	  
	    }// second for loop: over positive tracks

	}// first for loop: over negative tracks

    }//loop over negative events

    fFrameWorkEvent++;
    //cout << "\n" << D0counter << " MC-D0 were detected in this event" << endl;

}//Exec





 // -----   Private method Register   ---------------------------------------
void CbmD0CandidatesSE::Register() {
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman) Fatal("Register",
		      "No FairRootManager");

  //ioman->Register("CbmD0Candidate", "MVD", fPairs, kTRUE);
}
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

//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairImpactParameterR( Double_t Pair_Tx, Double_t Pair_Ty, Double_t SvX, Double_t SvY, Double_t SvZ, Double_t PvZ ){

    Double_t Pair_IPx = SvX - Pair_Tx*(SvZ - PvZ);
    Double_t Pair_IPy = SvY - Pair_Ty*(SvZ - PvZ);

    Double_t Pair_IP  = sqrt( Pair_IPx*Pair_IPx + Pair_IPy*Pair_IPy );

    return  Pair_IP;
}


//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairPt( TVector3& mom1, TVector3& mom2 ){

    Double_t PairPx = mom1.X() + mom2.X();
    Double_t PairPy = mom1.Y() + mom2.Y();

    Double_t PairPt = sqrt( PairPx*PairPx + PairPy*PairPy );

    return PairPt;
}

//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairPz( TVector3& mom1, TVector3& mom2 ){

    Double_t PairPz = mom1.Z() + mom2.Z();

    return PairPz;
}

//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetMomentum( TVector3& mom ){

    Double_t Momentum = mom.Mag();

    return Momentum;
}

//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairMomentum( TVector3& mom1, TVector3& mom2 ){

    Double_t PairPt = GetPairPt( mom1, mom2 );
    Double_t PairPz = GetPairPz( mom1, mom2 );
    Double_t PairMomentum = sqrt( PairPt*PairPt + PairPz*PairPz );

    return PairMomentum;
}

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

//-----------------------------------------------------------------------------------------

Double_t  CbmD0CandidatesSE::GetPairEnergy( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 ){

    Double_t Energy1 = GetEnergy1(tr1);
    Double_t Energy2 = GetEnergy2(tr2);

    Double_t PairEnergy = Energy1 + Energy2;

    return PairEnergy;
}

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

//-----------------------------------------------------------------------------------------

void CbmD0CandidatesSE::FillBuffer( CbmMapsFileManager* BackgroundFile, TObjArray* BackgroundArray ){

    if (BackgroundFile==0) {Fatal("CbmD0CandidatesSE:",
				  "Can't fill buffer, background-file not open.");};

    if (BackgroundArray){ BackgroundArray->Delete(); };

    cout << "CbmD0CandidatesSE: Filling background buffer. " <<endl;

    Int_t entries = BackgroundFile->GetNumberOfEventsInFile();

    for (Int_t i=0; i<entries; i++)
    { BackgroundArray->AddLast( BackgroundFile->GetEntry(i)->Clone() );  };
}

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

//-----------------------------------------------------------------------------------------

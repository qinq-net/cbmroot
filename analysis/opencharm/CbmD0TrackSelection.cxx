#include "CbmD0TrackSelection.h"
#include "TClonesArray.h"
#include "FairRootManager.h"
#include "CbmStsTrack.h"
#include "CbmStsKFSecondaryVertexFinder.h"
#include "CbmVertex.h"
#include "TVector3.h"
#include "CbmStsKFTrackFitter.h"
#include "CbmMCTrack.h"
#include "TH1F.h"
#include "TH2F.h"
#include "CbmMvdPoint.h"
#include "CbmKF.h"
#include "CbmTrackMatchNew.h"
#include "CbmD0TrackCandidate.h"

// Includes from C++
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "TCanvas.h"
#include "TStyle.h"
#include "TNtuple.h"

//-- Include from Fair --//
#include "FairLogger.h"

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


ClassImp(CbmD0TrackSelection)

    // -------------------------------------------------------------------------
    CbmD0TrackSelection::CbmD0TrackSelection(){
    
    Fatal( "CbmD0TrackSelection: Do not use the standard constructor","");
    
    }
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0TrackSelection::CbmD0TrackSelection(char* name, Int_t iVerbose, Double_t cutP, Double_t cutPt, Double_t cutPV, Double_t cutIP):FairTask(name,iVerbose)
{



    CbmKF* kalman = CbmKF::Instance();
    fFit                   = new CbmStsKFTrackFitter();
    fEventNumber = 0;
    fPVCutPassed = 0;
    fPVCutNotPassed = 0;
    fHistoFileName = "Histo.root";
    fCutP  = cutP;
    fCutPt = cutPt;
    fCutPV = cutPV;
    fCutIP = cutIP;
    fNHitsOfLongTracks=1;

	//added by c.tragser
    fShowDebugHistos= kFALSE;
	fadi = kFALSE;
	fNoHPassed=0;
    fNoHNotPassed=0;
    logfile = "./CutEff_D0TrackSelection.log";
  
    bUseMCInfo = kTRUE;
   




}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0TrackSelection::~CbmD0TrackSelection() {
    fInfoArray->Clear("C");
    delete fListD0TrackCandidate;

    delete fStsTrackArrayP;
    delete fStsTrackArrayN;
    delete fMCTrackArrayP;
    delete fMCTrackArrayN;


    //delete fFit;

}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
InitStatus CbmD0TrackSelection::Init() {

    FairRootManager* ioman = FairRootManager::Instance();
    
    
    fListD0TrackCandidate  = new TClonesArray("CbmD0TrackCandidate",100);
    fStsTrackArrayP        = new TClonesArray("CbmStsTrack",100);
    fStsTrackArrayN        = new TClonesArray("CbmStsTrack",100);
    fMCTrackArrayP         = new TClonesArray("CbmMCTrack",100);
    fMCTrackArrayN         = new TClonesArray("CbmMCTrack",100);

    ioman->Register("CbmD0TrackCandidate",  "Open Charm Tracks", fListD0TrackCandidate, kTRUE);
    ioman->Register("PositiveStsTracks", "Open Charm Sts Positiv", fStsTrackArrayP, kTRUE);
    ioman->Register("NegativeStsTracks", "Open Charm Sts Negativ", fStsTrackArrayN, kTRUE);
    ioman->Register("PositiveMCTracks",  "Open Charm Mc Positiv", fMCTrackArrayP,  kTRUE);
    ioman->Register("NegativeMCTracks",  "Open Charm Mc Negativ", fMCTrackArrayN,  kTRUE);

    fMcPoints        = (TClonesArray*) ioman->GetObject("MvdPoint");
    fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
    fStsTrackArray   = (TClonesArray*) ioman->GetObject("StsTrack");
    fMvdHitMatchArray= (TClonesArray*) ioman->GetObject("MvdHitMatch");
    fGlobalTracks    = (TClonesArray*) ioman->GetObject("GlobalTrack");
    fTrdTracks       = (TClonesArray*) ioman->GetObject("TrdTrack");
    fRichRings       = (TClonesArray*) ioman->GetObject("RichRing");
    fTofTracks       = (TClonesArray*) ioman->GetObject("TofTrack");
    
    
    
    
    
    
    if(! fStsTrackArray) {Fatal("CbmD0TrackSelection: StsTrackArray not found (!)"," That's bad. ");}
    if(! fMvdHitMatchArray) {Fatal("CbmD0TrackSelection: MVDHitMatchArray not found","Good bye");}
    fPrimVtx         = (CbmVertex*) ioman->GetObject("PrimaryVertex");
    fListMCTracks    = (TClonesArray*) ioman->GetObject("MCTrack");
    fFit             = new  CbmStsKFTrackFitter();
    fFit->Init();


    // Init DebugHistos ==============================================

    if(fShowDebugHistos=kTRUE)
	{
		PVnTracks = new TH1F("PVnTracks","PVnTracks", 1000, 0., 1000);
		PVZ = new TH1F("PVZ","PVZ", 1000, -0.005, 0.005);

		fpZMcHisto      = new TH1F("PMc","momentumMC",100, -1., 30.);
		fpZRecoHisto    = new TH1F("PReco","momentumReco",100, -1., 30.);
		ptHisto      = new TH1F("Pt",  "\tTransvMomentum", 1000, -1., 10.);
		imrHisto     = new TH1F("imr", "\tImpactParam",    600, -1.,  2.0);
		QpHisto      = new TH1F("Qp",  "\tQp",             600, -10., 10.);
		fpErrorHisto        = new TH1F("fpErrorHisto","DeltaP over P",1000,-0.1,0.1);
		fpErrorHistoPcut    = new TH1F("fpErrorHistoPcut","DeltaP over P, with P cut",1000,-0.1,0.1);
		fpErrorHisto_vs_mom = new TH2F("fpErrorHisto_vs_mom","DeltaP over P vs P",1000,0, 1,1000,0,10);
		fpZErrorHisto= new TH1F("RelPZError","RelPZError",100,-0.1,0.1);
		fptErrorHisto= new TH1F("RelPtError","RelPtError",100,-0.1,0.1);
		fMvdEfficiencyHisto=new TH2F("MvdEfficiency","MvdEfficiency",6,0,6,6,0,6);
		fPVSigmaHisto = new TH1F("fPVSigma","fPVSigma",100,0,0);
		fIPHisto = new TH1F("IP","IP",100,0,0);
                fIPRes = new TH1F("ip_res","IP Res",2000,-10,10);

		fIPResMergedHits   = new TH1F("ip_res_merg","IP Res for at least one merged hit",2000,-1,1);
                fIPResMergedHitsX  = new TH1F("ip_res_mergX","IP Res for at least one merged hit X",2000,-1,1);
                fIPResMergedHitsY  = new TH1F("ip_res_mergY","IP Res for at least one merged hit Y",2000,-1,1);
                fIPResCorrectHits  = new TH1F("ip_res_corr","IP Res for all correct hits",2000,-1,1);
                fIPResCorrectHitsX = new TH1F("ip_res_corrX","IP Res for all correct hits X",2000,-1,1);
                fIPResCorrectHitsY = new TH1F("ip_res_corrY","IP Res for all correct hits Y",2000,-1,1);

		fIPResMergedHitsMom   = new TH1F("ip_res_mergMom","IP Res for at least one merged hit, with mom cut",2000,-1,1);
		fIPResMergedHitsXMom  = new TH1F("ip_res_mergXMom","IP Res for at least one merged hit X, with mom cut",2000,-1,1);
		fIPResMergedHitsYMom  = new TH1F("ip_res_mergYMom","IP Res for at least one merged hit Y, with mom cut",2000,-1,1);
                fIPResCorrectHitsMom  = new TH1F("ip_res_corrMom","IP Res for all correct hits, with mom cut",2000,-1,1);
                fIPResCorrectHitsXMom = new TH1F("ip_res_corrXMom","IP Res for all correct hits X, with mom cut",2000,-1,1);
                fIPResCorrectHitsYMom = new TH1F("ip_res_corrYMom","IP Res for all correct hits Y, with mom cut",2000,-1,1);

		fIPResAllHitsX    = new TH1F("IPResAllHitsX","IP Res for all tracks with 2mvd hits,X",2000,-1,1);
		fIPResAllHitsY    = new TH1F("IPResAllHitsY","IP Res for all tracks with 2mvd hits,Y",2000,-1,1);
		fIPResAllHitsXMom = new TH1F("IPResAllHitsMomX","IP Res for all tracks with 2mvd hits, X, with mom cut",2000,-1,1);
		fIPResAllHitsYMom = new TH1F("IPResAllHitsMomY","IP Res for all tracks with 2mvd hits, Y, with mom cut",2000,-1,1);


		fIPResMergedHitsVsMom  = new TH2F("IPResMergedHitsMom" , "ip for merged hits vs mom",  1000,0,1,200,0,5);
		fIPResCorrectHitsVsMom = new TH2F("IPResCorrectHitsMom" ,"ip for correct hits vs mom", 1000,0,1,200,0,5);

		fpErrorHistoCorr    = new TH1F("fpErrorHistoCorr","Mom Resol, corr",1000,-0.1,0.1);
		fpErrorHistoCorrMom = new TH1F("fpErrorHistoCorrMom","Mom Resol, corr, with mom cut",1000,-0.1,0.1);
                fpErrorHistoMerg    = new TH1F("fpErrorHistoMerg","Mom Resol, merg",1000,-0.1,0.1);
                fpErrorHistoMergMom  = new TH1F("fpErrorHistoMergMom","Mom Resol, merg, with mom cut",1000,-0.1,0.1);

                fIPAllHitsSigX  = new TH1F("ip_res_all_SigX" ,"IP for all hitX, signal",2000,-1,1);
                fIPAllHitsSigY  = new TH1F("ip_res_all_SigY" ,"IP for all hitY, signal",2000,-1,1);

		fIPMergedHitsSig    = new TH1F("ip_res_merg_Sig"  ,"IP for at least one merged hit,  signal",2000,-1,1);
		fIPMergedHitsSigX   = new TH1F("ip_res_merg_SigX" ,"IP for at least one merged hitX, signal",2000,-1,1);
		fIPMergedHitsSigY   = new TH1F("ip_res_merg_SigY" ,"IP for at least one merged hitY, signal",2000,-1,1);

		fIPCorrHitsSig    = new TH1F("ip_res_corr_Sig"  ,"IP for 2correct mvd hit,  signal",2000,-1,1);
		fIPCorrHitsSigX   = new TH1F("ip_res_corr_SigX" ,"IP for 2correct mvd hitX , signal",2000,-1,1);
		fIPCorrHitsSigY   = new TH1F("ip_res_corr_SigY" ,"IP for 2correct mvd hitY, signal",2000,-1,1);



                
		fIPResMissing1HitsX  = new TH1F("fIPResMissing1HitsX","IP Res for one missing hit X",2000,-1,1);
                fIPResMissing2HitsX  = new TH1F("fIPResMissing2HitsX","IP Res two missing hit X",2000,-1,1);
                fIPResWrong1HitsX    = new TH1F("fIPResWrong1HitsX","IP Res for one wrong hit X",2000,-1,1);
                fIPResWrong2HitsX    = new TH1F("fIPResWrong2HitsX","IP Res for two wrong hit X",2000,-1,1);


		fIPResMissing1HitsY  = new TH1F("fIPResMissing1HitsY","IP Res for one missing hit Y",2000,-1,1);
                fIPResMissing2HitsY  = new TH1F("fIPResMissing2HitsY","IP Res two missing hit Y",2000,-1,1);
                fIPResWrong1HitsY    = new TH1F("fIPResWrong1HitsY","IP Res for one wrong hit Y",2000,-1,1);
                fIPResWrong2HitsY    = new TH1F("fIPResWrong2HitsY","IP Res for two wrong hit Y",2000,-1,1);
              






		//fIPMergedHitsSigP  = new TH2F("ip_res_merg_SigP" ,"IP for at least one merged hit, signal-mom",1000,0,100,100,0,10);
		//fIPMergedHitsSigP  = new TH2F("ip_res_merg_SigP" ,"IP for at least one merged hit, signal-mom",1000,0,100,100,0,10);

		fPVMergedHitsBg    = new TH1F("pv_res_merg_Bg"  ,"PV for at least one merged hit, bg",1000,0,100);
		fPVAllHitsBg       = new TH1F("pv_res_all_Bg"  ,"PV for all classes of tracks, bg",1000,0,100);
		fPVMergedHitsBgP   = new TH2F("pv_res_merg_BgP"  ,"PV for at least one merged hit, bg-mom",1000,0,100,100,0,10);
		fPVCorrectHitsSig  = new TH1F("pv_res_clean_Sig","PV for all correct hits, signal",1000,0,100);
		fPVCorrectHitsSigP = new TH2F("pv_res_clean_SigP","PV for all correct hits, signal-mom",1000,0,100,100,0,10);
		fPVCorrectHitsBg   = new TH1F("pv_res_clean_Bg" ,"PV for all correct hits, bg",1000,0,100);
		fPVCorrectHitsBgP  = new TH2F("pv_res_clean_BgP" ,"PV for all correct hits, bg-mom",1000,0,100,100,0,10);



		//fPVCorrectHitsSigP = new TH2F("pv_res_clean_Sig_vs_P","PV for all correct hits, signal vs mom",1000,0,100,1000,0,100);
		//fPVCorrectHitsBgP  = new TH2F("pv_res_clean_Bg_vs_P","PV for all correct hits, bg vs mom",1000,0,100,1000,0,100);

    };
    fTrackAnalysisTuple=new TNtuple("TrackTuple","TrackTuple1","MotherID:x:y:xMC:yMC:zMC:PV:p:pMC:gp:wp:chi2",320000);
    // ==============================================================

    return kSUCCESS;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmD0TrackSelection::Exec(Option_t* option){

fEventNumber++;
if(fEventNumber % 50 == 1) LOG(INFO) << "Event: " << fEventNumber << FairLogger::endl;

    fListD0TrackCandidate->Clear();
    fStsTrackArrayP->Clear();
    fStsTrackArrayN->Clear();
    fMCTrackArrayP->Clear();
    fMCTrackArrayN->Clear();

Int_t nTracks = fStsTrackArray->GetEntriesFast();

    if (fVerbose>0) LOG(INFO) <<" CbmD0TrackSelection: Entries: " << nTracks << FairLogger::endl;

    if(nTracks==0)
	{
	LOG(INFO) << "CbmD0TrackSelection:: No CbmStsTracks found, ignoring this event." << FairLogger::endl;
	return;
	}

    CbmStsTrack*         stsTrack;
    CbmTrackMatchNew*    mcTrackMatch;
    CbmMCTrack*          mcTrack;
    TVector3             vertex;

    Int_t NAcceptedTrackP = 0;
    Int_t NAcceptedTrackN = 0;

    Int_t count=0;
    Int_t pidHypo=211;
    Int_t mcTrackIndex = -1;
    // --- control histos ---
              /*
    if (fPrimVtx) 
	{
	       
		PVnTracks->Fill( fPrimVtx->GetNTracks() );
		PVZ->Fill( fPrimVtx->GetZ() );
    };      */


    // --- Loop over reconstructed tracks ---
for ( Int_t itr=0; itr<nTracks; itr++ )
	{
	
	if (fVerbose>0)LOG(INFO) << "CbmD0TrackSelection: stsTrack: " << itr << FairLogger::endl;
	stsTrack = (CbmStsTrack*) fStsTrackArray->At(itr);
	if(! IsLong(stsTrack))
		{
		fNoHNotPassed++;
		if (fVerbose>0) LOG(INFO) << "CbmD0TrackSelection: stsTrack is short" << FairLogger::endl;
		continue;
		}
	else 
		fNoHPassed++;

	KminusReFit(stsTrack);
        const FairTrackParam* par1 = stsTrack->GetParamFirst();
	Double_t       	      qp   = par1->GetQp();
	
	if (fVerbose>0) cout << "qp: " << qp << endl;
	
	if(qp < 0.0)
	  pidHypo = 321;
	else
	  pidHypo = 211;
	
	FairTrackParam  e_track;
	
	if (fPrimVtx) 
		{
		fFit->Extrapolate( stsTrack, fPrimVtx->GetZ(), &e_track);
		}
	else 
		{
		fFit->Extrapolate( stsTrack, 0 , &e_track);
		}
		
	Double_t   ip        = GetImpactParameterRadius( &e_track );
	Double_t   ipx       = GetImpactParameterX( &e_track );
	Double_t   ipy       = GetImpactParameterY( &e_track );
	Double_t   pt        = GetTransverseMom( &e_track );
	Double_t   p         = GetMom( &e_track );
	Double_t   PVsigma;
	
	if (fPrimVtx) 
		{
			PVsigma= fFit->GetChiToVertex( stsTrack, fPrimVtx );     // * <-- wert fuer pvsigma
		} 
		else 
		{
			PVsigma=-1;
			LOG(INFO) << "No PrimaryVtx" << FairLogger::endl;
		}



                             /*
	if ((PVsigma != -1) && (fShowDebugHistos) && (p<100)) //wieso hier p<100 ?? wuerden die ersten zwei bedingungen nicht reichen?
	{
	    fPVSigmaHisto->Fill(PVsigma);
	    if (PVsigma>fCutPV){    fPVCutPassed++;} //!
	    else {fPVCutNotPassed++;};

	}
                           */
    /////////////////////////////////////////////////////
    
	//if (PVsigma != -1 ) cout << "PVS:: " << PVsigma;
    //cout << amount << "/" << totamount;
        
	// --- single track pre-selection ---
	if ( PVsigma < fCutPV ) continue;
	if ( p  < fCutP       ) continue;
	if ( pt < fCutPt      ) continue;
	if ( ip > fCutIP      ) continue;
    /////////////////////////////////////////////////////   
    if(bUseMCInfo)
    {
        LOG(INFO) << "found possible opencharm track candidate, use MC-Data to start QA" << FairLogger::endl;
	// --- find the mcTrack ---
	mcTrackMatch       = (CbmTrackMatchNew*) fStsTrackMatches->At(itr);
	if(!mcTrackMatch)
		{
		LOG(FATAL) << "TrackMatch problem "<< fEventNumber << FairLogger::endl;
		}
	mcTrackIndex = mcTrackMatch->GetMatchedLink().GetIndex();

	if(mcTrackIndex>fListMCTracks->GetEntriesFast())
		{
		LOG(FATAL) << "McIndexProblem at index "<< mcTrackIndex << FairLogger::endl;
		} 
	if(mcTrackIndex<0)
		{
	        count++;
	        if(count%50==1)cout << "-W- mcTrackIndex < 0 " << endl;
	        continue;
		}
	mcTrack = (CbmMCTrack*) fListMCTracks->At(mcTrackIndex);
		// --- Save only positive MCtracks ---
	if (qp>0) {
	    TClonesArray& MCTrackArrayP = *fMCTrackArrayP;
	    Int_t i_MCtracksP = MCTrackArrayP.GetEntriesFast();
	    new( MCTrackArrayP[i_MCtracksP] ) CbmMCTrack(*mcTrack);
	}

	// --- Save only negative MCtracks ---
	if (qp<0) {
	    	Int_t i_MCtracksN = fMCTrackArrayN->GetEntriesFast();
		new((*fMCTrackArrayN)[i_MCtracksN]) CbmMCTrack(*((CbmMCTrack*)mcTrack));
	}
    
	if (fShowDebugHistos && mcTrack)
		{
            	Int_t goodMatch=0; Int_t badMatch=0;
	    	CheckMvdMatch(stsTrack, mcTrackIndex, goodMatch,badMatch);
	    	FillDebugHistos(stsTrack, mcTrack, badMatch, goodMatch);
		}
    } 	
        Int_t nMvdHits = stsTrack->GetNofMvdHits();
        Int_t nStsHits = stsTrack->GetNofHits() - nMvdHits;


	// --- cut on protons-antiprotons ---
	if ( mcTrack) 
		{
		if(TMath::Abs( mcTrack->GetPdgCode() ) == 2212 ) continue;
		}

	// --- Save D0TrackCandidate  ---
	TClonesArray& clref = *fListD0TrackCandidate;
	Int_t size = clref.GetEntriesFast();
	new( clref[size] ) CbmD0TrackCandidate( &e_track, PVsigma, itr, mcTrackIndex, nMvdHits, nStsHits, pidHypo );
        if (fVerbose>0) LOG(INFO) << "new Track Candidate nummber: " << size << " with pid " << pidHypo << " and qp " << e_track.GetQp() << FairLogger::endl;

	// --- Save only positive STStracks ---

	if (qp>0) {
	    TClonesArray& StsTrackArrayP = *fStsTrackArrayP;
	    Int_t i_tracksP = StsTrackArrayP.GetEntriesFast();
	    new( StsTrackArrayP[i_tracksP] ) CbmStsTrack(*stsTrack);
	}

	// --- Save only negative STStracks ---
	if (qp<0) {
	    TClonesArray& StsTrackArrayN = *fStsTrackArrayN;
	    Int_t i_tracksN = StsTrackArrayN.GetEntriesFast();
	    new( StsTrackArrayN[i_tracksN] ) CbmStsTrack(*stsTrack);
	}

    }// for loop track1
	/*
    TFile f(fHistoFileName,"recreate");


      PVnTracks->Write();
     PVZ->Write();
     pHisto->Write();
     ptHisto->Write();
     imrHisto->Write();
     QpHisto->Write();
    */
}

// -----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------
void CbmD0TrackSelection::FillDebugHistos(CbmStsTrack* stsTrack, CbmMCTrack* mcTrack, Int_t badMatch, Int_t goodMatch){

        const FairTrackParam* par1 = stsTrack->GetParamFirst();
	Double_t       	      qp   = par1->GetQp();
	if (fVerbose>0) cout << "qp: " << qp << endl;
	FairTrackParam  e_track;
	if (fPrimVtx) 
		{
		fFit->Extrapolate( stsTrack, fPrimVtx->GetZ(), &e_track);
		}
	else 
		{
		fFit->Extrapolate( stsTrack, 0 , &e_track);
		}
	Double_t   ip        = GetImpactParameterRadius( &e_track );
	Double_t   ipx       = GetImpactParameterX( &e_track );
	Double_t   ipy       = GetImpactParameterY( &e_track );
	Double_t   pt        = GetTransverseMom( &e_track );
	Double_t   p         = GetMom( &e_track );
	Double_t   PVsigma;
	    TVector3             vertex;
	    TVector3 mom;
	    e_track.Momentum(mom);



			if( mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 && mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2)
			{
			    fIPResAllHitsX->Fill(ipx);
			    fIPResAllHitsY->Fill(ipy);
			    fpErrorHisto->Fill((mcTrack->GetP()-p)/mcTrack->GetP());  //only primary tracks with 2 mvd hits
			    //fpErrorHisto_vs_mom->Fill((mcTrack->GetP()-p)/mcTrack->GetP(),mcTrack->GetP());  //only primary tracks with 2 mvd hits
			}

			if(mcTrack->GetP()>1 && mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 && mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2)
			{
			    fIPResAllHitsXMom->Fill(ipx);
			    fIPResAllHitsYMom->Fill(ipy);
			    fpErrorHistoPcut->Fill((mcTrack->GetP()-p)/mcTrack->GetP());  //only primary tracks with 2 mvd hits
			    //fpErrorHisto_vs_mom->Fill((mcTrack->GetP()-p)/mcTrack->GetP(),mcTrack->GetP());  //only primary tracks with 2 mvd hits
			}


			//fpZErrorHisto->Fill((mcTrack->GetPz()-mom.Z())/mcTrack->GetPz());
			//fpZMcHisto->Fill(mcTrack->GetPz());
			//fpZRecoHisto->Fill(mom.Z());


                     /*   TVector3 vec;
		      mcTrack->GetStartVertex(&vec);
		      Int_t nMvdHits=stsTrack->GetNofMvdHits();
		      */



			mcTrack->GetStartVertex(vertex);
                        //merged-signal
                        if(mcTrack->GetMotherId()==-1
			   && ( TMath::Abs(mcTrack->GetPdgCode()) == 211 ||  TMath::Abs(mcTrack->GetPdgCode()) == 321)
			   && vertex.Z() >0
                           //&& vertex.Z() <5
			   && mcTrack->GetNPoints(kMVD)==2
			   //&& (badMatch!=0 || stsTrack->GetNofMvdHits()<2)
			  )
			{
			    
			    fIPAllHitsSigX->Fill(ipx);
			    fIPAllHitsSigY->Fill(ipy);

			}
			if(mcTrack->GetMotherId()==-1
			   && ( TMath::Abs(mcTrack->GetPdgCode()) == 211 ||  TMath::Abs(mcTrack->GetPdgCode()) == 321)
			   && vertex.Z() >0
                           //&& vertex.Z() <5
			   && mcTrack->GetNPoints(kMVD)==2
			   && (badMatch!=0 || stsTrack->GetNofMvdHits()<2) )
			{


			    fIPMergedHitsSig->Fill(ip);
			    fIPMergedHitsSigX->Fill(ipx);
			    fIPMergedHitsSigY->Fill(ipy);

			}
			//clean-signal
		        if(mcTrack->GetMotherId()==-1
			   && ( TMath::Abs(mcTrack->GetPdgCode()) == 211 ||  TMath::Abs(mcTrack->GetPdgCode()) == 321)
			   && vertex.Z() >0
			   //&& vertex.Z() <5
			   && mcTrack->GetNPoints(kMVD)==2
			   && (badMatch==0  && stsTrack->GetNofMvdHits()==2) )
			{

                             
			      fIPCorrHitsSigX->Fill(ipx);
			      fIPCorrHitsSigY->Fill(ipy);




			}


			/*


			fIPResMissing1HitsX
			fIPResMissing2HitsX
			fIPResWrong1HitsX
			fIPResWrong2HitsX


			fIPResMissing1HitsY
			fIPResMissing2HitsY
			fIPResWrong1HitsY
			fIPResWrong2HitsY

			*/


			//1 MISSING HIT AND THE OTHER HIT IS CORRECT
			if(mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 &&mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2 && (badMatch==0 || stsTrack->GetNofMvdHits()==1)){
                           fIPResMissing1HitsX->Fill(ipx);
                           fIPResMissing1HitsY->Fill(ipy);
			}

			//2MISS
			if(mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 &&mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2 && ( stsTrack->GetNofMvdHits()==0)){
			    fIPResMissing2HitsX->Fill(ipx);
			    fIPResMissing2HitsY->Fill(ipy);
			}

			//1WRONG  HIT AND THE OTHER HIT IS CORRECT
			if(mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 &&mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2 && (badMatch==1 && goodMatch==1 )){
                           fIPResWrong1HitsX->Fill(ipx);
                           fIPResWrong1HitsY->Fill(ipy);

			}

			//2WRONG
			if(mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 &&mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2 && (badMatch==2)){
                           fIPResWrong2HitsX->Fill(ipx);
                           fIPResWrong2HitsY->Fill(ipy);



			}



			if(mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 &&mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2 && (badMatch!=0 || stsTrack->GetNofMvdHits()<2)){

			    fIPResMergedHits->Fill(ip);
			    fIPResMergedHitsX->Fill(ipx);
			    fIPResMergedHitsY->Fill(ipy);

			    fIPResMergedHitsVsMom->Fill(ip,mcTrack->GetP());//no need to add this histo to the mom cut

			    fpErrorHistoMerg->Fill((mcTrack->GetP()-p)/mcTrack->GetP());




			}

			if(mcTrack->GetP()>1 && mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 &&mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2 && (badMatch!=0 || stsTrack->GetNofMvdHits()<2)){

			    fIPResMergedHitsMom->Fill(ip);
			    fIPResMergedHitsXMom->Fill(ipx);
			    fIPResMergedHitsYMom->Fill(ipy);

			    fpErrorHistoMergMom->Fill((mcTrack->GetP()-p)/mcTrack->GetP());




			}


			if( mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 && mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2 && (badMatch==0 && stsTrack->GetNofMvdHits()==2) ){

			    fIPResCorrectHits->Fill(ip);
			    fIPResCorrectHitsX->Fill(ipx);
			    fIPResCorrectHitsY->Fill(ipy);

			    fpErrorHistoCorr->Fill((mcTrack->GetP()-p)/mcTrack->GetP());
			}



			if(mcTrack->GetP()>1 &&  mcTrack->GetStartZ() == 0 && mcTrack->GetStartY() == 0 && mcTrack->GetStartX() == 0 && mcTrack->GetNPoints(kMVD)==2 && (badMatch==0 && stsTrack->GetNofMvdHits()==2) ){

			    fIPResCorrectHitsMom->Fill(ip);
			    fIPResCorrectHitsXMom->Fill(ipx);
			    fIPResCorrectHitsYMom->Fill(ipy);

			    fpErrorHistoCorrMom->Fill((mcTrack->GetP()-p)/mcTrack->GetP());

			   


			}

			//if(mom.Z()>30){cout << endl << "-I- momentum: " << mom.Z() << endl;}
			TVector3 pos;





			//fptErrorHisto->Fill((mcTrack->GetPt()-mom.Pt())/mcTrack->GetPt());



			//fPVSigmaHisto->Fill(fListD0TrackCandidate->GetPVSigma());
			// fIPHisto->Fill();

	        
			// fill fTrackAnalysis
			//fTrackAnalysisTuple->Fill(mcTrack->GetMotherId(),(float)e_track.GetX(),(float)e_track.GetY(),
			  //      mcTrack->GetStartX(),mcTrack->GetStartY(),mcTrack->GetStartZ(),(float)PVsigma,(float)p,
			  //      (float)mcTrack->GetP(),goodMatch,badMatch,stsTrack->GetChi2());
			
			if (fadi) cout  << "-I- saved data for Analysis Tuple!"<< endl;
            
            //  fTrackAnalysisTuple=new TNtuple("TrackTuple","TrackTuple1","MotherID:x:y:xMC:yMC:PV:p:pMC:gp:wp:chi2");

}
// -----------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------
void CbmD0TrackSelection::CheckMvdMatch(CbmStsTrack* stsTrack, Int_t mcTrackIndex,Int_t& goodMatch, Int_t& badMatch)
{
goodMatch=0;
badMatch=0;
Int_t nrOfMvdHits = stsTrack->GetNofMvdHits();
Int_t mcTrackId = 0;
Int_t nrOfLinks = 0;
Int_t mcPointId = 0;

const CbmMvdPoint* point = NULL;
for(Int_t iHit = 0; iHit < nrOfMvdHits; iHit++)	
	{
        CbmMatch* mvdMatch = (CbmMatch*)fMvdHitMatchArray->At(stsTrack->GetMvdHitIndex(iHit));
	if(mvdMatch)
		{
		nrOfLinks = mvdMatch->GetNofLinks();
		}	
	else 
		{
		continue; // any kind of error in the matching
		}
	for(Int_t iLink = 0; iLink < nrOfLinks; iLink++)
		{
		Int_t pointIndex = mvdMatch->GetLink(iLink).GetIndex();
		if(pointIndex < fMcPoints->GetEntriesFast())	
 		point = (CbmMvdPoint*)fMcPoints->At(pointIndex);
		if (NULL == point)
			{
			continue; //delta or background event
			}
		else
			mcTrackId = point->GetTrackID();
		if(mcTrackId == mcTrackIndex)
			{
			goodMatch++;
			}
		else 
			badMatch++;
		}
	}    
}


// -------------------------------------------------------------------------------------------

void CbmD0TrackSelection::KminusReFit(CbmStsTrack* track){

    const FairTrackParam* par1 = track->GetParamFirst();    // when using ParamFirst pb of charge change appear...
    Double_t  qpBefFit1 = par1->GetQp();

    if ( qpBefFit1<0. ){fFit->DoFit(track, 321);}

}


//-----------------------------------------------------------------------------------------

Double_t  CbmD0TrackSelection::GetImpactParameterRadius( FairTrackParam* t ){

    TVector3 pos;
    t->Position(pos);

    Double_t imx  = pos.X();
    Double_t imy  = pos.Y();
    Double_t imr  = sqrt(imx*imx + imy*imy);

   return  imr;
}



//-----------------------------------------------------------------------------------------

Double_t  CbmD0TrackSelection::GetImpactParameterX( FairTrackParam* t ){

    TVector3 pos;
    t->Position(pos);

    Double_t imx  = pos.X();

   return  imx;
}



//-----------------------------------------------------------------------------------------

Double_t  CbmD0TrackSelection::GetImpactParameterY( FairTrackParam* t ){

    TVector3 pos;
    t->Position(pos);

    Double_t imy  = pos.Y();

   return  imy;
}



//-----------------------------------------------------------------------------------------

Double_t  CbmD0TrackSelection::GetTransverseMom( FairTrackParam* t ){

    TVector3 mom;
    t->Momentum(mom);

    Double_t pt   = sqrt( mom.X()*mom.X() + mom.Y()*mom.Y() );

    return  pt;
}

//-----------------------------------------------------------------------------------------

Double_t  CbmD0TrackSelection::GetMom( FairTrackParam* t ){

    TVector3 mom;
    t->Momentum(mom);

    Double_t p    = mom.Mag();

    return  p;
}
//-----------------------------------------------------------------------------------------
void CbmD0TrackSelection::SetCuts(Double_t p, Double_t pt, Double_t PVsigma, Double_t IP){
    fCutP  = p;
    fCutPt = pt;
    fCutPV = PVsigma;
    fCutIP = IP;
}
//-----------------------------------------------------------------------------------------

Bool_t CbmD0TrackSelection::IsLong(CbmStsTrack *track){

  Int_t nMapsHits   = track->GetNofMvdHits();
  Int_t nStripHits  = track->GetNofHits() - nMapsHits;

  if ( nStripHits < fNHitsOfLongTracks ) 
	return 0;
 
  return 1;
}

// -------------------------------------------------------------------------------

void CbmD0TrackSelection::Finish(){

	//cout << endl << endl << " * * * * * \t strack - information \t * * * * * ";

    fIPResMergedHits->Write();
    fIPResMergedHitsX->Write();
    fIPResMergedHitsY->Write();
    fIPResCorrectHits->Write();
    fIPResCorrectHitsX->Write();
    fIPResCorrectHitsY->Write();


    fIPResMergedHitsMom->Write();   
    fIPResMergedHitsXMom->Write();  
    fIPResMergedHitsYMom->Write();  
    fIPResCorrectHitsMom->Write();  
    fIPResCorrectHitsXMom->Write(); 
    fIPResCorrectHitsYMom->Write(); 

    fIPResAllHitsX->Write();
    fIPResAllHitsY->Write();
    fIPResAllHitsXMom->Write();
    fIPResAllHitsYMom->Write();

    fIPResMissing1HitsX ->Write();
    fIPResMissing2HitsX ->Write();
    fIPResWrong1HitsX ->Write();
    fIPResWrong2HitsX->Write();


    fIPResMissing1HitsY ->Write();
    fIPResMissing2HitsY ->Write();
    fIPResWrong1HitsY ->Write();
    fIPResWrong2HitsY ->Write();



    fIPMergedHitsSig->Write();
    fIPMergedHitsSigX->Write();
    fIPMergedHitsSigY->Write();

    fIPAllHitsSigX->Write();
    fIPAllHitsSigY->Write();

    fIPCorrHitsSig->Write();
    fIPCorrHitsSigX->Write();
    fIPCorrHitsSigY->Write();


    fIPResMergedHitsVsMom->Write();
    fIPResCorrectHitsVsMom->Write();

    fpErrorHisto->Write();
    fpErrorHistoCorr->Write();   
    fpErrorHistoCorrMom->Write();
    fpErrorHistoMerg->Write();   
    fpErrorHistoMergMom->Write();

    fpErrorHisto_vs_mom->Write();
    fpErrorHistoPcut->Write();
    //fPVMergedHits->Write();
    //fPVCorrectHits->Write();
    //fPVMergedHitsSig->Write();
    //fPVMergedHitsBg->Write();
    //fPVCorrectHitsSig->Write();
    //fPVCorrectHitsBg->Write();



      /*
    fPVMergedHitsSig->Write();
    fPVMergedHitsSigP->Write();
    fPVMergedHitsBg->Write();
    fPVMergedHitsBgP->Write();
    fPVCorrectHitsSig->Write();
    fPVCorrectHitsSigP->Write();
    fPVCorrectHitsBg->Write();
    fPVCorrectHitsBgP->Write();
    fPVAllHitsBg->Write();
    */


    if (fShowDebugHistos)
    {
	                      
		TCanvas* c=new TCanvas();
		c->Divide(3,3);
		c->cd(1);
		fpErrorHisto->Draw();
			fpErrorHisto->Write();
			c->cd(2);
		fpZErrorHisto->Draw();
		fpZErrorHisto->Write();
			//c->cd(3);
		//fptErrorHisto->Draw();
		c->cd(3);
		fpZMcHisto->Draw();
		cout << endl << "black: " << fpZMcHisto->GetEntries()<<endl;
		fpZRecoHisto->SetLineColor(kRed);
			fpZMcHisto->Write();
			c->cd(4);
		cout << endl << "red: " <<fpZRecoHisto->GetEntries()<<endl;
		fpZRecoHisto->Draw();
			fpZRecoHisto->Write();
		c->cd(5);
		fptErrorHisto->Draw();
			fptErrorHisto->Write();
		c->cd(6);
			gStyle->SetPalette(1);
		fMvdEfficiencyHisto->Draw("COLZTEXT");
		fMvdEfficiencyHisto->Write();
		c->cd(7);
			fPVSigmaHisto->Draw();
		fPVSigmaHisto->Write();
		c->cd(8);
	        fIPHisto->Draw();
		fIPHisto->Write();
		c->cd(9);
		fIPRes->Draw();
                fIPRes->Write();
		  //  cout << ":: " << amount << " / " << totamount;
	  
	
		cout << "-> PVCut passed by: " <<    fPVCutPassed << " Tracks (" << 100./(fPVCutNotPassed+fPVCutPassed)*fPVCutPassed <<"% of all tracks)."<< endl;
		cout << "-> " << fNoHPassed << " Tracks passed by the minimal number of hits in mvd + sts this is " << (double) fNoHPassed/(fNoHNotPassed+fNoHPassed)*100 << " %" << endl;

                SaveCutEff();

		fTrackAnalysisTuple->Write();
                                

    }
}
// -------------------------------------------------------------------------------

// -------------------------------------------------------------------------------
void CbmD0TrackSelection::SaveCutEff(void)
{
    cout << "-I- logging CutEff to" << logfile << endl;
    using namespace std;

    ofstream fout;

    fout.open(logfile);

    fout << "-I- logging Cut Efficieny: CbmD0TrackSelection" << endl;
    fout << "-I- for file: " << endl;
    fout << "-I- PVCut passed by: " <<    fPVCutPassed << " Tracks (" << 100./(fPVCutNotPassed+fPVCutPassed)*fPVCutPassed <<"% of all tracks)."<< endl;
    fout << "-I- " << fNoHPassed << " Tracks passed by the minimal number of hits in mvd + sts this is " << (double) fNoHPassed/(fNoHNotPassed+fNoHPassed)*100 << " %" << endl;


    fout.close();
	      
}
// -------------------------------------------------------------------------------



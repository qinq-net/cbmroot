// -------------------------------------------------------------------------
// -----              CbmMvdQa  source file                            -----
// -----              Created 12/01/15  by P. Sitzmann                 -----
// ------------------------------------------------------------------------

//-- Include from Cbm --//
#include "CbmMvdQa.h"
#include "CbmStsTrack.h"
#include "CbmMvdHit.h"
#include "CbmMvdPoint.h"

#include "CbmVertex.h"
#include "CbmMatch.h"
#include "CbmLink.h"
#include "CbmTrackMatchNew.h"
#include "CbmMCTrack.h"

#include "base/CbmLitToolFactory.h"
#include "data/CbmLitTrackParam.h"
#include "utils/CbmLitConverter.h"


//-- Include from Fair --//
#include "FairLogger.h"
#include "FairTrackParam.h"


//-- Include from Root --//
#include "TCanvas.h"
#include "TMath.h"

//-- Include from C++ --//
#include <iostream>


using std::cout;
using std::endl;
using std::flush;

// -----   Default constructor   -------------------------------------------
CbmMvdQa::CbmMvdQa() 
  : FairTask("MvdQa"),
  fNHitsOfLongTracks(),
  fEventNumber(),
  fStsTrackArray(),
  fStsTrackMatches(),
  fListMCTracks(),
  fMvdHitMatchArray(),
  fMvdResHistoX(),
  fMvdResHistoR(),
  fMvdResHistoY(),
  fMvdResHistoXY(),
  fMatchingEffiHisto(),
  fInfoArray(),
  fExtrapolator(),
  fPrimVtx(),
  fSecVtx(),
  fminHitReq(0),
  fMvdRecoRatio(0),
  fBadTrack(0),
  fUsedTracks(0),
  fnrOfMergedHits(0.)
{
;
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMvdQa::CbmMvdQa(const char* name, Int_t iMode, Int_t iVerbose) 
  : FairTask(name, iVerbose),
  fNHitsOfLongTracks(),
  fEventNumber(),
  fStsTrackArray(),
  fStsTrackMatches(),
  fListMCTracks(),
  fMvdHitMatchArray(),
  fMvdResHistoX(),
  fMvdResHistoR(),
  fMvdResHistoY(),
  fMvdResHistoXY(),
  fMatchingEffiHisto(),
  fInfoArray(),
  fExtrapolator(),
  fPrimVtx(),
  fSecVtx(),
  fminHitReq(0),
  fMvdRecoRatio(0),
  fBadTrack(0),
  fUsedTracks(0),
  fnrOfMergedHits(0.)
{
 ;
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMvdQa::~CbmMvdQa() 
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
InitStatus CbmMvdQa::Init()
{
cout << "-------------------------------------------------------------------------" << endl
     << "-I- " << GetName() << "::Init: " 
     << " Start Initilisation " << endl
     << "-------------------------------------------------------------------------" << endl;

    FairRootManager* ioman = FairRootManager::Instance();
      if (! ioman) {
    cout << "-E- " << GetName() << "::Init: "
	 << "RootManager not instantised!" << endl;
    return kFATAL;
    }
    fBadTracks = new TClonesArray("CbmStsTrack", 5000);
    ioman->Register("BadTracks", "sts", fBadTracks, kTRUE);

    fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
    fStsTrackArray   = (TClonesArray*) ioman->GetObject("StsTrack");
    if(! fStsTrackArray) {Fatal("CbmMvdQa: StsTrackArray not found (!)"," That's bad. ");}
    fMvdHits         = (TClonesArray*) ioman->GetObject("MvdHit");
    fMvdHitMatchArray= (TClonesArray*) ioman->GetObject("MvdHitMatch");
    fMcPoints       = (TClonesArray*) ioman->GetObject("MvdPoint");
    if( fMvdHits->GetEntriesFast() != fMvdHitMatchArray->GetEntriesFast())
	cout << endl << "MvdHit and MvdHitMatch Arrays do not have the same size" << endl;
    fPrimVtx         = (CbmVertex*) ioman->GetObject("PrimaryVertex");
    fListMCTracks    = (TClonesArray*) ioman->GetObject("MCTrack");
    fExtrapolator = CbmLitToolFactory::Instance()->CreateTrackExtrapolator("rk4");

    

    // --- all the histogramms --- //
    fMvdResHistoX=new TH2F("MvdResMom","Mvd Resolution Momentum in x",70,0,10,70,-0.03,0.03);
    fMvdResHistoX->GetYaxis()->SetTitle("Impactparameter x [cm]");
    fMvdResHistoX->GetXaxis()->SetTitle("Momentum [GeV/c]");

    fMvdResHistoX->GetYaxis()->SetTitleOffset(1.5);
    fMvdResHistoX->GetXaxis()->SetTitleOffset(1.5);


    fMvdResHistoR=new TH2F("MvdResMom","Mvd Resolution Momentum in r",70,0,10,70, 0,0.05);
    fMvdResHistoR->GetYaxis()->SetTitle("Impactparameter r [cm]");
    fMvdResHistoR->GetXaxis()->SetTitle("Momentum [GeV/c]");

    fMvdResHistoR->GetYaxis()->SetTitleOffset(1.5);
    fMvdResHistoR->GetXaxis()->SetTitleOffset(1.5);


   fMvdResHistoY=new TH2F("Mvd4ResMom","Mvd Resolution Momentum in y",70,0,10,70,-0.03,0.03);
   fMvdResHistoY->GetYaxis()->SetTitle("Impactparameter y [cm]");
   fMvdResHistoY->GetXaxis()->SetTitle("Momentum [GeV/c]");

    fMvdResHistoY->GetYaxis()->SetTitleOffset(1.5);
    fMvdResHistoY->GetXaxis()->SetTitleOffset(1.5);

    fMvdResHistoXY=new TH2F("MvdResMom","Mvd Resolution Momentum X vs Y",70,-0.03,0.03,70,-0.03,0.03);
    fMvdResHistoXY->GetYaxis()->SetTitle("Impactparameter x [cm]");
    fMvdResHistoXY->GetXaxis()->SetTitle("Impactparameter y [cm]");

    fMvdResHistoXY->GetYaxis()->SetTitleOffset(1.5);
    fMvdResHistoXY->GetXaxis()->SetTitleOffset(1.5);

   fMatchingEffiHisto=new TH1F("MvdMatchingEffi","Matching Efficiency",70,0,1.1);
   fMatchingEffiHisto->GetXaxis()->SetTitle("Matching Efficiency");
   fMatchingEffiHisto->GetXaxis()->SetTitleOffset(1.5);
 fMatchingEffiHisto->GetYaxis()->SetTitle("Entrys");
   fMatchingEffiHisto->GetYaxis()->SetTitleOffset(1.5);

cout << "-------------------------------------------------------------------------" << endl
     << "-I- " << GetName() << "::Init: " 
     << " Finished Initilisation " << endl
     << "-------------------------------------------------------------------------" << endl;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdQa::Exec(Option_t* /*opt*/) 
{
fEventNumber++;

LOG(INFO) << "CbmMvdQa is running" << FairLogger::endl;

Int_t usedTracks=0;
Double_t zOut;
CbmStsTrack* stsTrack;
Int_t nTracks = fStsTrackArray->GetEntriesFast();
Int_t nTrackMatches = fStsTrackMatches->GetEntriesFast();
Int_t nMcTracks = fListMCTracks->GetEntriesFast();
Int_t motherID = 0;
Int_t trackIndex = 0;
CbmTrackMatchNew* trackMatch;
CbmMatch* Match;
CbmMCTrack* mcTrack;


    if (fVerbose>0) printf(" CbmMvdQa: Entries:  %i StsTracks from %i McTracks\n",nTracks,nMcTracks);

    if(nTracks==0){
	cout << endl << " -W- CbmMvdQa:: No CbmStsTracks found, ignoring this event." << endl;
	return;
    }
    if( nTracks != nTrackMatches )
	{
	cout << endl << " -W- CbmMvdQa:: CbmSletsTrackMatches has not the same size as CbmStsTracks" << endl;
	return;  
	} 
    // --- Loop over reconstructed tracks ---
for ( Int_t itr=0; itr<nTracks; itr++ )
	{
	stsTrack = (CbmStsTrack*) fStsTrackArray->At(itr);
        trackMatch = (CbmTrackMatchNew*)fStsTrackMatches->At(itr);
        if(trackMatch)
		{ 
		if ( ! trackMatch->GetNofLinks() ) continue;
		trackIndex = trackMatch->GetMatchedLink().GetIndex();
		if ( 0 <= trackIndex <= nMcTracks)
			{ 
			mcTrack = (CbmMCTrack*) fListMCTracks->At(trackIndex);
			motherID = mcTrack->GetMotherId();
			}

		else
			cout << endl << "index of match to hight" << endl;
		}
	else
		{
		cout << endl << "Ups there was no Match here" << endl;
		continue;
		}
	Int_t mvdHitsInTrack = stsTrack->GetNofMvdHits();
        Int_t mvdStsRatio;
       if(mvdHitsInTrack > 0) 
		mvdStsRatio = stsTrack->GetNofHits() / mvdHitsInTrack;
       else mvdStsRatio = 10;
       if(stsTrack->GetNofHits() - mvdHitsInTrack > 3)
	{
        if(mvdHitsInTrack >= fminHitReq && motherID == -1)
		{
		fUsedTracks++;
		//cout << endl << "found good track" << endl;
                SetMatches(trackIndex, stsTrack);
		//	if(fMvdRecoRatio != 1) 
		//		{
		//		new((*fBadTracks)[fBadTracks->GetEntriesFast()]) CbmStsTrack(*((CbmStsTrack*)stsTrack));
		//		fBadTrack++;
		//		continue;
		//		}
    		const FairTrackParam *paramIn = stsTrack->GetParamFirst();
   	 	CbmLitTrackParam litParamIn; // parameter at last attached Hit
   	 	CbmLitConverter::FairTrackParamToCbmLitTrackParam(paramIn, &litParamIn);
    		CbmLitTrackParam litParamOut; // parameter at vertex
	
   	 	fExtrapolator->Extrapolate(&litParamIn, &litParamOut,0, NULL);
		fMvdResHistoX->Fill(GetMomentum(litParamOut), GetImpactParameterX(litParamOut));
		fMvdResHistoR->Fill(GetMomentum(litParamOut), GetImpactParameterRadius(litParamOut));
		fMvdResHistoY->Fill(GetMomentum(litParamOut),GetImpactParameterY(litParamOut));
		fMvdResHistoXY->Fill(GetImpactParameterX(litParamOut),GetImpactParameterY(litParamOut));
		
		}

        }
      }
fStsTrackArray->Clear();
fStsTrackMatches->Clear();

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void  CbmMvdQa::SetMatches(Int_t trackID, CbmStsTrack* stsTrack){

Int_t nrOfMvdHits = stsTrack->GetNofMvdHits();
Int_t mcTrackId = 0;
Int_t trueCounter = 0;
Float_t falseCounter = 0;
Bool_t hasTrack;
Int_t nrOfLinks = 0;
Int_t mcPointId = 0;
fMvdRecoRatio = 0.;
const CbmMvdPoint* point = NULL;
for(Int_t iHit = 0; iHit < nrOfMvdHits; iHit++)	
	{
	hasTrack = kFALSE;
        CbmMatch* mvdMatch = (CbmMatch*)fMvdHitMatchArray->At(stsTrack->GetMvdHitIndex(iHit));
	if(mvdMatch)
		{
		nrOfLinks = mvdMatch->GetNofLinks();
		if(nrOfLinks > 1)
			fnrOfMergedHits++;
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
		if(mcTrackId == trackID)
			{
			hasTrack = kTRUE;
			}
		}
	if(!hasTrack)
	falseCounter++;
	}
Float_t counter = (Float_t)falseCounter / (Float_t)nrOfMvdHits;
fMvdRecoRatio = (Float_t)1.0 - (Float_t)counter;

	
fMatchingEffiHisto->Fill(fMvdRecoRatio);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Double_t  CbmMvdQa::GetImpactParameterRadius( CbmLitTrackParam t ){

    Double_t imx  = t.GetX();
    Double_t imy  = t.GetY();
    Double_t imr  = sqrt(imx*imx + imy*imy);

   return  imr;
}
// -------------------------------------------------------------------------


//--------------------------------------------------------------------------
Double_t  CbmMvdQa::GetImpactParameterX( CbmLitTrackParam t ){

Double_t imx  = t.GetX();

   return  imx;
}
// -------------------------------------------------------------------------


//--------------------------------------------------------------------------
Double_t  CbmMvdQa::GetImpactParameterY( CbmLitTrackParam t ){

    Double_t imy  = t.GetY();

   return  imy;
}
// -------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------
Double_t  CbmMvdQa::GetTransverseMomentum( CbmLitTrackParam t ){

const std::vector<double> state= t.GetStateVector();

Float_t pz   = GetMomentumZ(t);
Float_t px = state[2] * pz;
Float_t py = state[3] * pz;

Double_t pt = sqrt(px * px + py * py);

    return  pt;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
Double_t  CbmMvdQa::GetMomentum( CbmLitTrackParam t ){

    Float_t p = fabs(1.F / t.GetQp());

    return  p;
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
Double_t  CbmMvdQa::GetMomentumZ( CbmLitTrackParam t ){

   const std::vector<double> state= t.GetStateVector();

Float_t pz   = sqrt(GetMomentum(t) * GetMomentum(t) / (state[2] * state[2] + state[3] * state[3] + 1));

    return  pz;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
Double_t  CbmMvdQa::GetAngle( CbmLitTrackParam t ){

    Float_t angle_rad = std::atan(GetTransverseMomentum(t) / GetMomentumZ(t));
    Float_t rad_to_grad = ( 180 / TMath::Pi() );
    Float_t angle_grad = angle_rad * rad_to_grad;
//cout << endl << "calc angle of " << angle_grad << " from rad angle " << angle_rad << endl;
    return  angle_grad;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
void CbmMvdQa::Finish()
{
Float_t mvdTrackingEffi = (fBadTrack)/(fUsedTracks/100);
cout << endl << "-I- " << GetName() << " Finished" << endl
     << "Total of " << fBadTrack << " StsTracks where removed due bad matched hits in the MVD" << endl
     << "Removed: " << mvdTrackingEffi << " % of all Tracks from Plots" << endl
     << fnrOfMergedHits << " Hit where merged" << endl;
TCanvas* c=new TCanvas();
c->Divide(2,2);
c->cd(1);
fMvdResHistoX->Draw("COL");
fMvdResHistoX->Write();
c->cd(2);
fMvdResHistoY->Draw("COL");
fMvdResHistoY->Write();
c->cd(3);
fMvdResHistoXY->Draw("COL");
fMvdResHistoXY->Write();
c->cd(4);
fMvdResHistoR->Draw("COL");
fMvdResHistoR->Write();


}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
ClassImp(CbmMvdQa);



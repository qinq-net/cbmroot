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

#include "CbmMvdDetector.h"
#include "CbmMvdSensor.h"

#include "tools/CbmMvdGeoHandler.h"


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
    fNHitsOfLongTracks(0),
    fEventNumber(0),
    fminHitReq(0),
    fMvdRecoRatio(0),
    fBadTrack(0),
    fUsedTracks(0),
    fnrOfMergedHits(0.),
    fStsTrackArray(NULL),
    fStsTrackArrayP(NULL),
    fStsTrackArrayN(NULL),
    fStsTrackMatches(NULL),
    fGlobalTrackArray(NULL),   
    fListMCTracks(NULL),
    fMCTrackArrayP(NULL),
    fMCTrackArrayN(NULL),
    fMcPoints(NULL),
    fMvdDigis(NULL),
    fMvdCluster(NULL),
    fMvdHits(NULL),
    fMvdHitMatchArray(NULL),
    fBadTracks(NULL),
    fInfoArray(NULL),
    fMvdResHistoX(NULL),
    fMvdResHistoR(NULL),
    fMvdResHistoY(NULL),
    fMvdResHistoXY(NULL),
    fMatchingEffiHisto(NULL),
    fMvdDigiDist1(NULL),
    fMvdDigiDist2(NULL),
    fMvdDigiDist3(NULL),
    fMvdDigiDist4(NULL),
    fMvdDigiWorst(NULL),
    fMvdHitWorst(NULL),
    fMvdMCWorst(NULL),
    fMvdMCWorstDelta(NULL),
    fMvdMCBank(),
    fMvdMCHitsStations(),
    fWordsPerSuperRegion(NULL),
    fWorstSuperPerEvent(NULL),
    fMvdBankDist(NULL),
    fMvdTrackQa1F(),
    fMvdTrackQa2F(),
    fPrimVtx(NULL),
    fSecVtx(NULL),
    fDetector(NULL),
    useMcQa(kFALSE),
    useDigiQa(kFALSE),
    useHitQa(kFALSE),
    useTrackQa(kFALSE)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMvdQa::CbmMvdQa(const char* name, Int_t iMode, Int_t iVerbose) 
  : FairTask(name, iVerbose),
    fNHitsOfLongTracks(0),
    fEventNumber(0),
    fminHitReq(0),
    fMvdRecoRatio(0),
    fBadTrack(0),
    fUsedTracks(0),
    fnrOfMergedHits(0.),
    fStsTrackArray(NULL),
    fStsTrackArrayP(NULL),
    fStsTrackArrayN(NULL),
    fStsTrackMatches(NULL),
    fGlobalTrackArray(NULL),   
    fListMCTracks(NULL),
    fMCTrackArrayP(NULL),
    fMCTrackArrayN(NULL),
    fMcPoints(NULL),
    fMvdDigis(NULL),
    fMvdCluster(NULL),
    fMvdHits(NULL),
    fMvdHitMatchArray(NULL),
    fBadTracks(NULL),
    fInfoArray(NULL),
    fMvdResHistoX(NULL),
    fMvdResHistoR(NULL),
    fMvdResHistoY(NULL),
    fMvdResHistoXY(NULL),
    fMatchingEffiHisto(NULL),
    fMvdDigiDist1(NULL),
    fMvdDigiDist2(NULL),
    fMvdDigiDist3(NULL),
    fMvdDigiDist4(NULL),
    fMvdDigiWorst(NULL),
    fMvdHitWorst(NULL),
    fMvdMCWorst(NULL),
    fMvdMCWorstDelta(NULL),
    fMvdMCBank(),
    fMvdMCHitsStations(),
    fWordsPerSuperRegion(NULL),
    fWorstSuperPerEvent(NULL),
    fMvdBankDist(NULL),
    fMvdTrackQa1F(),
    fMvdTrackQa2F(),
    fPrimVtx(NULL),
    fSecVtx(NULL),
    fDetector(NULL),
    useMcQa(kFALSE),
    useDigiQa(kFALSE),
    useHitQa(kFALSE),
    useTrackQa(kFALSE)
{
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
    ioman->Register("BadTracks", "sts", fBadTracks, IsOutputBranchPersistent("BadTracks"));

    fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
    fStsTrackArray   = (TClonesArray*) ioman->GetObject("StsTrack");
    fGlobalTrackArray = (TClonesArray*) ioman->GetObject("GlobalTrack");

    if(! fStsTrackArray) {Fatal("CbmMvdQa: StsTrackArray not found (!)"," That's bad. ");}

    fMcPoints        = (TClonesArray*) ioman->GetObject("MvdPileUpMC");  // PileUp Mc points
    fMvdDigis        = (TClonesArray*) ioman->GetObject("MvdDigi");
    fMvdCluster      = (TClonesArray*) ioman->GetObject("MvdCluster");
    fMvdHits         = (TClonesArray*) ioman->GetObject("MvdHit");
    fMvdHitMatchArray= (TClonesArray*) ioman->GetObject("MvdHitMatch");

    if( fMvdHits->GetEntriesFast() != fMvdHitMatchArray->GetEntriesFast())
	cout << endl << "MvdHit and MvdHitMatch Arrays do not have the same size" << endl;
    fPrimVtx         = (CbmVertex*) ioman->GetObject("PrimaryVertex");
    fListMCTracks    = (TClonesArray*) ioman->GetObject("MCTrack");

    if(! fMcPoints )
    {
        cout << endl << "Mvd Pile Up Mc array missing";
	return kFATAL;
    }
    fDetector = CbmMvdDetector::Instance();
    fDetector->Init();


    SetupHistograms();

       for(Int_t i = 0; i < 63; i++)
       {
	   ioman->Register(Form("BankHistos%d", i), "Histograms of banks on sensor", fMvdMCBank[i], kTRUE);
       }
       for(Int_t j = 0; j < 4; j++)
       {
	   ioman->Register(Form("MCHistoStation%d",j),"Histogram of MC distribuion on Stations", fMvdMCHitsStations[j], kTRUE);
       }
       ioman->Register("MCHitDistribution", "MC Hits per Region", fMvdBankDist, kTRUE);
       ioman->Register("fWordsPerSuperRegion","words per super Region", fWordsPerSuperRegion, kTRUE);

    cout << "-------------------------------------------------------------------------" << endl
     << "-I- " << GetName() << "::Init: " 
     << " Finished Initialisation " << endl
     << "-------------------------------------------------------------------------" << endl;
    return kSUCCESS;
}

// -------------------------------------------------------------------------
void CbmMvdQa::SetupHistograms()
{
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

   fMvdResHistoXY=new TH2F("MvdResMomXY","Mvd Resolution Momentum X vs Y",70,-0.03,0.03,70,-0.03,0.03);
   fMvdResHistoXY->GetYaxis()->SetTitle("Impactparameter x [cm]");
   fMvdResHistoXY->GetXaxis()->SetTitle("Impactparameter y [cm]");

   fMvdResHistoXY->GetYaxis()->SetTitleOffset(1.5);
   fMvdResHistoXY->GetXaxis()->SetTitleOffset(1.5);

   fMatchingEffiHisto=new TH1F("MvdMatchingEffi","Matching Efficiency",70,0,1.1);
   fMatchingEffiHisto->GetXaxis()->SetTitle("Matching Efficiency");
   fMatchingEffiHisto->GetXaxis()->SetTitleOffset(1.5);
   fMatchingEffiHisto->GetYaxis()->SetTitle("Entrys");
   fMatchingEffiHisto->GetYaxis()->SetTitleOffset(1.5);

   fMvdDigiDist1 = new TH2F("MvdDigiDist1","Mvd Digi distribution Station 0",50,-2.5,2.5,50,-2.5,2.5);
   fMvdDigiDist1->GetYaxis()->SetTitle("x [cm]");
   fMvdDigiDist1->GetXaxis()->SetTitle("y [cm]");

   fMvdDigiDist2 = new TH2F("MvdDigiDist2","Mvd Digi distribution Station 1",100,-6,6,100,-6,6);
   fMvdDigiDist2->GetYaxis()->SetTitle("x [cm]");
   fMvdDigiDist2->GetXaxis()->SetTitle("y [cm]");

   fMvdDigiDist3 = new TH2F("MvdDigiDist3","Mvd Digi distribution Station 2",3000,-9,9,3000,-9,9);
   fMvdDigiDist3->GetYaxis()->SetTitle("x [cm]");
   fMvdDigiDist3->GetXaxis()->SetTitle("y [cm]");

   fMvdDigiDist4 = new TH2F("MvdDigiDist4","Mvd Digi distribution Station 3",3500,-12,12,3500,-12,12);
   fMvdDigiDist4->GetYaxis()->SetTitle("x [cm]");
   fMvdDigiDist4->GetXaxis()->SetTitle("y [cm]");

   fMvdDigiWorst= new TH2F("MvdDigiWorst","Mvd Digi distribution worst spot",10,-3,0,10,-2,2);
   fMvdDigiWorst->GetYaxis()->SetTitle("x [cm]");
   fMvdDigiWorst->GetXaxis()->SetTitle("y [cm]");

   fMvdHitWorst= new TH2F("fMvdHitWorst","Mvd hit distribution worst spot",300,-3,0,1500,-2,2);
   fMvdHitWorst->GetYaxis()->SetTitle("x [cm]");
   fMvdHitWorst->GetXaxis()->SetTitle("y [cm]");


   fMvdMCHitsStations[0] = new TH2F("fMvdMCStation0","Mvd mc distribution",2,-2.5,-0.5,3,-0.5,2.5);
   fMvdMCHitsStations[1] = new TH2F("fMvdMCStation1","Mvd mc distribution",4,-4.5,-0.5,6,-0.5,5.5);
   fMvdMCHitsStations[2] = new TH2F("fMvdMCStation2","Mvd mc distribution",6,-7.5,-1.5,9,-1.5,7.5);
   fMvdMCHitsStations[3] = new TH2F("fMvdMCStation3","Mvd mc distribution",8,-9.5,-1.5,12,-1.5,10.5);

   fMvdMCWorst= new TH2F("fMvdMCWorst","Mvd mc distribution worst spot",300,-2,0,1500,3.5,0);
   fMvdMCWorst->GetYaxis()->SetTitle("y [cm]");
   fMvdMCWorst->GetXaxis()->SetTitle("x [cm]");

   fMvdMCWorstDelta= new TH2F("fMvdMCWorstDelta","Mvd mc distribution worst spot only delta electrons",300,-2,0,1500,3.5,0);
   fMvdMCWorstDelta->GetYaxis()->SetTitle("y [cm]");
   fMvdMCWorstDelta->GetXaxis()->SetTitle("x [cm]");

   for(Int_t i = 0; i < 63; i++)
   {
    fMvdMCBank[i] = new TH2F(Form("fMvdMCBank%d",i),"Mvd mc distribution worst spot only delta electrons",300,-2,0,1500,3.5,0);
   }

   fMvdBankDist = new TH2I("fMvdBankDist", "Avarage Hits per Region",63,0,63,50,0,50);

   fWordsPerSuperRegion = new TH1F("fWordsPerSuperRegion","Words send to a super region",1000,0,400);

   fWorstSuperPerEvent = new TH1F("fWorstSuperRegion","Words send to worst super region",1000,0,400);


   fMvdTrackQa1F[0] = new TH1F("fMvdTracksQa1","Mvd Hits per Global Track",10,0,5);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdQa::Exec(Option_t* /*opt*/) 
{
fEventNumber++;


if( fEventNumber%10 == 0)LOG(INFO) << "CbmMvdQa is running Event " << fEventNumber <<  FairLogger::endl;

if(useMcQa)ExecMCQa();
if(useDigiQa)ExecDigiQa();
if(useHitQa)ExecHitQa();
if(useTrackQa)ExecTrackQa();

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdQa::ExecDigiQa()
{
    Int_t nrSensor = fDetector->GetSensorArraySize();

for(Int_t n = 0; n < nrSensor; n++)
{
    CbmMvdSensor* curSens = fDetector->GetSensor(n);

    for(Int_t i = 0; i < fMvdDigis->GetEntriesFast(); i++)
    {
	CbmMvdDigi* curDigi = (CbmMvdDigi*)fMvdDigis->At(i);
	Double_t lab[3]={0.,0.,0.};
	if(curSens->GetDetectorID() == curDigi->GetDetectorId())
	{
	    curSens->PixelToTop(curDigi->GetPixelX(), curDigi->GetPixelY(), lab);
	    if(curSens->GetZ() < 6)
		fMvdDigiDist1->Fill(lab[0], lab[1]);
            else if(curSens->GetZ() < 11)
		fMvdDigiDist2->Fill(lab[0], lab[1]);
            else if(curSens->GetZ() < 16)
		fMvdDigiDist3->Fill(lab[0], lab[1]);
            else
		fMvdDigiDist4->Fill(lab[0], lab[1]);

             if(lab[0]>-2 && lab[0]<=-0.5 && lab[1]>=-1.5 && lab[1]<=1.5)
	{
	    fMvdDigiWorst->Fill(lab[0], lab[1]);
	}
	}
       
    }
}




}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdQa::ExecHitQa()
{
for(Int_t k = 0; k < fMvdHits->GetEntriesFast(); k++)
{
    CbmMvdHit* curHit = (CbmMvdHit*)fMvdHits->At(k);
         if(curHit->GetX() > -2 && curHit->GetX() <= -0.5 && curHit->GetY() >= -1.5 && curHit->GetY() <= 1.5)
	{
	    fMvdHitWorst->Fill(curHit->GetX(), curHit->GetY());
	}
	}
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdQa::ExecMCQa()
{
    Float_t wordsPerRegion[64] = {0};
    Float_t wordsPerSuper[16] = {0};

    Float_t yPosMin = -0.73;
    Float_t yPosMax = 2.5;

for(Int_t k = 0; k < fMcPoints->GetEntriesFast(); k++)
{
    CbmMvdPoint* curPoint = (CbmMvdPoint*)fMcPoints->At(k);
    if(curPoint->GetZ() < 8)
    {
         if(curPoint->GetX() > -1.93 && curPoint->GetX() <= -0.55 && curPoint->GetY() >= yPosMin && curPoint->GetY() <= yPosMax)
	{
	    fMvdMCWorst->Fill(curPoint->GetX(), curPoint->GetY());
	    if(curPoint->GetTrackID() == -3) fMvdMCWorstDelta->Fill(curPoint->GetX(), curPoint->GetY());
	    for(Int_t nBank = 0; nBank < 64; nBank++)
		{
		    if( curPoint->GetY() >= (yPosMin+(nBank*0.05)) && curPoint->GetY() < (yPosMin + ((nBank+1)*0.05)) )
		    {
			fMvdMCBank[nBank]->Fill(curPoint->GetX(), curPoint->GetY());
			wordsPerRegion[nBank] = wordsPerRegion[nBank] + 1.5;
			
			break;
		    }
		}

	}
    }
	 if(curPoint->GetZ() < 8)
	 {
             if(curPoint->GetX() < -0.5 && curPoint->GetY() > -0.5)
	     fMvdMCHitsStations[0]->Fill(curPoint->GetX(), curPoint->GetY());
	 }
	 else if(curPoint->GetZ() < 13)
	 {
             if(curPoint->GetX() < -0.5 && curPoint->GetY() > -1.5)
	     fMvdMCHitsStations[1]->Fill(curPoint->GetX(), curPoint->GetY());
	 }
	 else if(curPoint->GetZ() < 18 )
	 {
             if(curPoint->GetX() < -1.5 && curPoint->GetY() > -1.5)
	     fMvdMCHitsStations[2]->Fill(curPoint->GetX(), curPoint->GetY());
	 }
	 else
	 {
             if(curPoint->GetX() < -1.5 && curPoint->GetY() > -1.5)
	     fMvdMCHitsStations[3]->Fill(curPoint->GetX(), curPoint->GetY());
	 }
}
cout << "//--------------- New Event -----------------------\\" << endl;

Int_t i = 0;
Int_t wordsInWorst = 0;

for(Int_t supReg = 0; supReg < 16; supReg ++)
{
    for(Int_t k = 0; k < 4; k++)
    {
	wordsPerSuper[supReg] += wordsPerRegion[i];
	cout << "Words in Region " << i << ": " <<  wordsPerRegion[i] << endl;
        i++;
    }

    cout  << " Words in super region " << supReg << ": " << wordsPerSuper[supReg] << endl;

    fWordsPerSuperRegion->Fill(wordsPerSuper[supReg]);

    if(wordsPerSuper[supReg] > wordsInWorst)
	wordsInWorst = wordsPerSuper[supReg];

    //cout << "Words in worst super regin now: " << wordsInWorst << endl;
}

fWorstSuperPerEvent->Fill(wordsInWorst);

    cout << "//--------------- End Event -----------------------\\" << endl;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdQa::ExecTrackQa(){

/*
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
		    SetMatches(trackIndex, stsTrack);
		}

        }
      }
fStsTrackArray->Clear();
fStsTrackMatches->Clear();  */


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
/*
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
 */

//-----------------------------------------------------------------------------------------
void CbmMvdQa::Finish()
{


if(useMcQa)FinishMCQa();
if(useDigiQa)FinishDigiQa();
if(useHitQa)FinishHitQa();
if(useTrackQa)FinishTrackQa();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void CbmMvdQa::FinishMCQa()
{
Double_t scalingfactor = 0.0005;

TCanvas* mcCanvas1 = new TCanvas();
mcCanvas1->Divide(2,2);
for(Int_t iPad = 0; iPad < 4; iPad++)
   {
   mcCanvas1->cd(iPad+1);
   fMvdMCHitsStations[iPad]->Scale(scalingfactor);
   fMvdMCHitsStations[iPad]->Draw("COLZ");
   }

TCanvas* mcCanvas2 = new TCanvas();
mcCanvas2->Divide(1,2);
mcCanvas2->cd(1);
fWordsPerSuperRegion->Draw();
mcCanvas2->cd(2);
fWorstSuperPerEvent->Draw();

TCanvas* mcCanvas3 = new TCanvas();
fMvdMCHitsStations[0]->Draw("COLZ");

TCanvas* mcCanvas4 = new TCanvas();
mcCanvas4->Divide(8,8);
for(Int_t pad = 0; pad < 63; pad++)
  {
      mcCanvas4->cd(pad+1);
      fMvdMCBank[pad]->Draw("COL");
      fMvdMCBank[pad]->Write();
      cout << "Bank " << pad << " avarage Entries " << fMvdMCBank[pad]->GetEntries()/fEventNumber << endl;
      fMvdBankDist->Fill(pad, fMvdMCBank[pad]->GetEntries()/fEventNumber/2);
  }
TCanvas* mcCanvas5 = new TCanvas();
fMvdBankDist->Draw();

TCanvas* mcCanvas6 = new TCanvas();
fMvdMCWorst->Draw("COL");
fMvdMCWorst->Write();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void CbmMvdQa::FinishDigiQa()
{
TCanvas* digiCanvas1 = new TCanvas();
fMvdDigiDist1->Draw("COL");
fMvdDigiDist1->Write();

TCanvas* digiCanvas2 = new TCanvas();
fMvdDigiDist2->Draw("COL");
fMvdDigiDist2->Write();

TCanvas* digiCanvas3 = new TCanvas();
fMvdDigiDist3->Draw("COL");
fMvdDigiDist3->Write();

TCanvas* digiCanvas4 = new TCanvas();
fMvdDigiDist4->Draw("COL");
fMvdDigiDist4->Write();

TCanvas* digiCanvas5 = new TCanvas();
fMvdDigiWorst->Draw("COL");
fMvdDigiWorst->Write();

TCanvas* digiCanvas6 = new TCanvas();
fMvdMCWorstDelta->Draw("COL");
fMvdMCWorstDelta->Write();

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void CbmMvdQa::FinishHitQa()
{
TCanvas* hitCanvas1 = new TCanvas();
fMvdHitWorst->Draw("COL");
fMvdHitWorst->Write();
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void CbmMvdQa::FinishTrackQa()
{
TCanvas* TrackCanvas1 = new TCanvas();

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
ClassImp(CbmMvdQa);



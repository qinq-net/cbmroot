// -------------------------------------------------------------------------
// -----                  CbmMvdReadoutSimple source file              -----
// -----                  Created 17/10/16 by P. Sitzmann              -----
// -------------------------------------------------------------------------

#include "CbmMvdReadoutSimple.h"

#include "CbmMvdPoint.h"
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

#include <iostream>

using std::cout;
using std::endl;

// -----   Default constructor   -------------------------------------------
CbmMvdReadoutSimple::CbmMvdReadoutSimple()
    : FairTask("MvdReadoutSimple"),
    fshow(kFALSE),
    fMvdMCBank(),
    fMvdMCHitsStations(),
    fWordsPerSuperRegion(),
    fWorstSuperPerEvent(),
    fMvdBankDist(),
    fMvdMCWorst(),
    fMvdMCWorstDelta(),
    fMcPoints(),
    fListMCTracks(),
    fEventNumber(0)
{
;
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdReadoutSimple::CbmMvdReadoutSimple(const char* name, Int_t iVerbose)
    : FairTask(name , iVerbose),
    fshow(kFALSE),
    fMvdMCBank(),
    fMvdMCHitsStations(),
    fWordsPerSuperRegion(),
    fWorstSuperPerEvent(),
    fMvdBankDist(),
    fMvdMCWorst(),
    fMvdMCWorstDelta(),
    fMcPoints(),
    fListMCTracks(),
    fEventNumber(0)
{
;
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdReadoutSimple::~CbmMvdReadoutSimple()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
InitStatus CbmMvdReadoutSimple::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
      if (! ioman) {
	  LOG(FATAL) << "RootManager not instantised!" << FairLogger::endl;
      }

    fMcPoints        = (TClonesArray*) ioman->GetObject("MvdPileUpMC");  // PileUp Mc points
    fListMCTracks    = (TClonesArray*) ioman->GetObject("MCTrack");

    if(! fMcPoints) LOG(FATAL) << "Mvd Pile Up Mc array missing" << FairLogger::endl;

    SetupHistograms();

        for(Int_t i = 0; i < 63; i++)
       {
	   ioman->Register(Form("BankHistos%d", i), "Histograms of banks on sensor", fMvdMCBank[i], kTRUE);
       }
       for(Int_t j = 0; j < 4; j++)
       {
	   ioman->Register(Form("MCHistoStation%d",j),"Histogram of MC distribuion on Stations", fMvdMCHitsStations[j], kTRUE);
       }

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdReadoutSimple::SetupHistograms()
{
    for(Int_t i = 0; i < 63; i++)
       {
       fMvdMCBank[i] = new TH2F(Form("fMvdMCBank%d",i),"Mvd mc distribution worst spot only delta electrons",300,-2,0,1500,3.5,0);
       }

    fMvdMCHitsStations[0] = new TH2F("fMvdMCStation0","Mvd mc distribution",2,-2.5,-0.5,3,-0.5,2.5);
    fMvdMCHitsStations[1] = new TH2F("fMvdMCStation1","Mvd mc distribution",4,-4.5,-0.5,6,-0.5,5.5);
    fMvdMCHitsStations[2] = new TH2F("fMvdMCStation2","Mvd mc distribution",6,-7.5,-1.5,9,-1.5,7.5);
    fMvdMCHitsStations[3] = new TH2F("fMvdMCStation3","Mvd mc distribution",8,-9.5,-1.5,12,-1.5,10.5);

    fWordsPerSuperRegion = new TH1F("fWordsPerSuperRegion","Words send to a super region",1000,0,400);
    fWorstSuperPerEvent = new TH1F("fWorstSuperRegion","Words send to worst super region",1000,0,400);

    fMvdMCWorst= new TH2F("fMvdMCWorst","Mvd mc distribution worst spot",300,-2,0,1500,3.5,0);
    fMvdMCWorst->GetYaxis()->SetTitle("y [cm]");
    fMvdMCWorst->GetXaxis()->SetTitle("x [cm]");

    fMvdMCWorstDelta= new TH2F("fMvdMCWorstDelta","Mvd mc distribution worst spot only delta electrons",300,-2,0,1500,3.5,0);
    fMvdMCWorstDelta->GetYaxis()->SetTitle("y [cm]");
    fMvdMCWorstDelta->GetXaxis()->SetTitle("x [cm]");



}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMvdReadoutSimple::Exec(Option_t* opt)
{
    fEventNumber++;

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
void CbmMvdReadoutSimple::Finish()
{
if(fshow) DrawHistograms();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdReadoutSimple::DrawHistograms()
{
Double_t scalingfactor = 0.0005; // From 1000 events @ 30 mu s mc hits to 5 mu s words per Event

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
// -------------------------------------------------------------------------

ClassImp(CbmMvdReadoutSimple);
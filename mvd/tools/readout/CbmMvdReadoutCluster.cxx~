// -------------------------------------------------------------------------
// -----                  CbmMvdReadoutCluster source file             -----
// -----                  Created 17/10/16 by P. Sitzmann              -----
// -------------------------------------------------------------------------

#include "CbmMvdReadoutCluster.h"

#include "CbmMvdCluster.h"

//-- Include from Fair --//
#include "FairLogger.h"

//-- Include from Root --//
#include "TCanvas.h"
#include "TMath.h"
#include "TF1.h"
#include "TClonesArray.h"

#include <iostream>
#include <map>
#include <utility>

using std::cout;
using std::endl;
using std::map;
using std::pair;

// -----   Default constructor   -------------------------------------------
CbmMvdReadoutCluster::CbmMvdReadoutCluster()
    : CbmMvdReadoutCluster::CbmMvdReadoutCluster("MvdReadoutCluster", 0)
{
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmMvdReadoutCluster::CbmMvdReadoutCluster(const char* name, Int_t iVerbose)
    : FairTask(name , iVerbose),
    foutFile(nullptr),
    fshow(kFALSE),
    fWordsPerRegion(),
    fWordsPerSuperRegion(),
    fMvdCluster(nullptr),
    fEventNumber(0)
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmMvdReadoutCluster::~CbmMvdReadoutCluster()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
InitStatus CbmMvdReadoutCluster::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
      if (! ioman) {
	  LOG(FATAL) << "RootManager not instantised!" << FairLogger::endl;
      }

    fMvdCluster = (TClonesArray*) ioman->GetObject("MvdCluster");

    if(! fMvdCluster) LOG(FATAL) << "Data array missing" << FairLogger::endl;

    SetupHistograms();

    return kSUCCESS;

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdReadoutCluster::SetupHistograms()
{
    for(Int_t i = 0; i < 350; ++i)
    {
	fWordsPerRegion[i] = new TH1F(Form("fWordsPerRegionSensor%d",i),"Words send to a region",65 ,0 ,64);
	fWordsPerSuperRegion[i] = new TH1F(Form("fWordsPerSuperRegionSensor%d",i),"Words send to a region",16 ,0 ,15);
    }
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMvdReadoutCluster::Exec(Option_t* opt)
{
    LOG(DEBUG) << "//--------------- New Event " << fEventNumber << " -----------------------\\" << FairLogger::endl;

for(Int_t nClusters = 0; nClusters < fMvdCluster->GetEntriesFast(); ++nClusters)
   {
       CbmMvdCluster* cluster = (CbmMvdCluster*)fMvdCluster->At(nClusters);
       Int_t sensorNr = cluster->GetSensorNr();
       map<pair<Int_t, Int_t>, Int_t > pixelMap = cluster->GetPixelMap();

       for(auto cntr = pixelMap.begin(); cntr != pixelMap.end(); ++cntr)
       {
	   Int_t xPix = cntr->first.first; 
	   Int_t yPix = cntr->first.second;
	   Int_t regionNr = (Int_t) (cntr->first.first / fPixelsPerRegion);
	   fWordsPerRegion[sensorNr]->Fill(regionNr);
	   Int_t superRegionNr = (Int_t)(cntr->first.first / fPixelsPerSuperRegion);
           fWordsPerSuperRegion[sensorNr]->Fill(superRegionNr);
       }

   }

LOG(DEBUG) << "//--------------- End Event -----------------------\\" <<  FairLogger::endl;
fEventNumber++;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdReadoutCluster::Finish()
{
    foutFile->cd();

    Float_t scale = 1./(Float_t)fEventNumber;
        for(Int_t i = 0; i < 350; ++i)
    {
	fWordsPerRegion[i]->Scale(scale, "nosw2");
	fWordsPerSuperRegion[i]->Scale(scale, "nosw2");
    }

if(fshow)DrawHistograms();
WriteHistograms();

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdReadoutCluster::WriteHistograms()
{

    for(Int_t i = 0; i < 350; ++i)
    {
	fWordsPerRegion[i]->Write();
    }

    for(Int_t i = 0; i < 350; ++i)
    {
	fWordsPerSuperRegion[i]->Write();
    }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMvdReadoutCluster::DrawHistograms()
{

}
// -------------------------------------------------------------------------

ClassImp(CbmMvdReadoutCluster);
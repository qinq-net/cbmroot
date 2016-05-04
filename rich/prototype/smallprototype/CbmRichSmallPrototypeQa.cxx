
#include "CbmRichSmallPrototypeQa.h"

#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"

#include "CbmMCTrack.h"
#include "FairTrackParam.h"
#include "CbmRichHit.h"
#include "FairMCPoint.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmRichRing.h"
#include "CbmRichHit.h"
#include "CbmMatchRecoToMC.h"
#include "CbmRichGeoManager.h"
#include "CbmRichPoint.h"
#include "utils/CbmRichDraw.h"

#include "CbmUtils.h"
#include "CbmHistManager.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>
#include <sstream>

using namespace std;
using boost::assign::list_of;

CbmRichSmallPrototypeQa::CbmRichSmallPrototypeQa()
: FairTask("CbmRichSmallPrototypeQa"),
fHM(NULL),
fOutputDir(""),
fRichHits(NULL),
fCanvas(),
fEventNum(0)
//fMinNofHits(7),
//fNofHitsInRingMap()
{
}


InitStatus CbmRichSmallPrototypeQa::Init()
{
    cout << "CbmRichSmallPrototypeQa::Init"<<endl;
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) { Fatal("CbmRichSmallPrototypeQa::Init","RootManager not instantised!"); }
    
    fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if ( NULL == fRichHits) { Fatal("CbmRichSmallPrototypeQa::Init","No RichHit array!"); }
    
    
    InitHistograms();
    
    return kSUCCESS;
}

void CbmRichSmallPrototypeQa::Exec(
                            Option_t* option)
{
    fEventNum++;
    
    cout << "CbmRichSmallPrototypeQa, event No. " <<  fEventNum << endl;
    
    //fHM->H1("fh_nof_rings_1hit")->Fill(nRings1hit);
    
}

void CbmRichSmallPrototypeQa::InitHistograms()
{
    fHM = new CbmHistManager();
    
    fHM->Create1<TH1D>("fh_nof_rich_hits", "fh_nof_rich_hits;Nof RICH hits;Yield", 1000, 0., 3000.);

}

void CbmRichSmallPrototypeQa::DrawHist()
{
    cout.precision(4);
    
    SetDefaultDrawStyle();
    
    {
        fHM->H1("fh_nof_rich_hits")->Scale(1./fEventNum);
        TCanvas* c = CreateCanvas("rich_sp_nof_rich_hits", "rich_sp_nof_rich_hits", 800, 800);
        DrawH1(fHM->H1("fh_nof_rich_hits"));
        gPad->SetLogy(true);
    }
  }

void CbmRichSmallPrototypeQa::Finish()
{
 //   DrawHist();
  //  SaveCanvasToImage();
   // fHM->WriteToFile();
}


TCanvas* CbmRichSmallPrototypeQa::CreateCanvas(
                                        const string& name,
                                        const string& title,
                                        int width,
                                        int height)
{
    TCanvas* c = new TCanvas(name.c_str(), title.c_str(), width, height);
    fCanvas.push_back(c);
    return c;
}

void CbmRichSmallPrototypeQa::SaveCanvasToImage()
{
    for (int i = 0; i < fCanvas.size(); i++)
    {
        Cbm::SaveCanvasAsImage(fCanvas[i], fOutputDir);
    }
}

ClassImp(CbmRichSmallPrototypeQa)


// -------------------------------------------------------------------------
// -----               CbmPointSetArrayDraw source file                -----
// -----                Created 18/06/22  by J. Brandt                 -----
// -----               Following class FairPointSetDraw                -----
// -------------------------------------------------------------------------

#include "CbmPointSetArrayDraw.h"

#include "FairEventManager.h"           // for FairEventManager
#include "FairRootManager.h"            // for FairRootManager
#include "CbmPointSetArray.h"           // for CbmPointSetArray
#include "CbmPixelHit.h"                // for CbmPixelHit
#include "CbmTofHit.h"                  // for CbmTofHit

//#include "Riosfwd.h"                    // for ostream
#include "TClonesArray.h"               // for TClonesArray
#include "TEveManager.h"                // for TEveManager, gEve
#include "TEveTreeTools.h"              // for TEvePointSelectorConsumer, etc
#include "TNamed.h"                     // for TNamed
#include "TString.h"                    // for Form
#include "TVector3.h"                   // for TVector3
#include <TEveRGBAPalette.h>            // for RGBColorPalette
#include <TColor.h>                     // for TColor

#include <stddef.h>                     // for NULL
#include <iostream>                     // for operator<<, basic_ostream, etc
#include <iomanip>

using std::cout;
using std::endl;

// -----   Default constructor   -------------------------------------------
CbmPointSetArrayDraw::CbmPointSetArrayDraw()
  : FairTask("CbmPointSetArrayDraw", 0),
    fVerbose(0),
    fPointList(NULL),
    fEventManager(NULL),
    fl(NULL),
    fColor(0),
    fStyle(0),
    fTimeMax(0),
    fTimeOffset(0),
    fColorMode(1),
    fMarkerMode(1),
    fRender(kTRUE)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmPointSetArrayDraw::CbmPointSetArrayDraw(const char* name, Int_t colorMode ,Int_t markerMode,Int_t iVerbose, Bool_t render)
  : FairTask(name, iVerbose),
    fVerbose(iVerbose),
    fPointList(NULL),
    fEventManager(NULL),
    fl(NULL),
    fColor(kRed),
    fStyle(4),
    fTimeMax(0),
    fTimeOffset(0),
    fColorMode(colorMode),
    fMarkerMode(markerMode),
    fRender(render)
{

}
// -------------------------------------------------------------------------
InitStatus CbmPointSetArrayDraw::Init()
{
  if(fVerbose>1) { cout<<  "CbmPointSetArrayDraw::Init()" << endl; }
  FairRootManager* fManager = FairRootManager::Instance();
  fPointList = static_cast<TClonesArray*>(fManager->GetObject(GetName()));
  if(fPointList==0) {
    cout << "CbmPointSetArrayDraw::Init()  branch " << GetName() << " Not found! Task will be deactivated "<< endl;
    SetActive(kFALSE);
  }
  if(fVerbose>2) { cout<<  "CbmPointSetArrayDraw::Init() get track list" <<  fPointList<< endl; }
  fEventManager =FairEventManager::Instance();
  if(fVerbose>2) { cout<<  "CbmPointSetArrayDraw::Init() get instance of FairEventManager " << endl; }
  fl =0;

  return kSUCCESS;
}
// -------------------------------------------------------------------------
void CbmPointSetArrayDraw::Exec(Option_t* /*option*/)
{
  if (IsActive()) {
    Int_t npoints=fPointList->GetEntriesFast();
    Reset();

    // initialize CbmPointSetArray to display set of hits
    CbmPointSetArray* l = new CbmPointSetArray("TofHitTime", "");
    l->SetColorMode(fColorMode);
    l->SetMarkerMode(fMarkerMode);
    l->SetSourceCS(TEvePointSelectorConsumer::kTVT_XYZ);
    l->SetMarkerColor(kRed);
    l->SetMarkerStyle(4);
    l->SetMarkerSize(2.0);

    DetermineTimeOffset();
    l->InitBins("Hits",npoints,0.5,npoints+0.5);
    l->InitValues(npoints);
    
    for (Int_t i=1; i<=npoints; i++){                                           //loop over all hits in event
      TObject*  p=static_cast<TObject*>(fPointList->At(i-1));
      if(p!=0) {
        TVector3 vec(GetVector(p));
        l->Fill(vec.X(),vec.Y(),vec.Z(),i);                                     // fill 3D position
        l->FillValues(GetPointId(p),GetTime(p),GetTot(p),GetClusterSize(p),i);  // fill physical information used for color and markersize
      }
    }

    l->ApplyColorMode();                                                        // apply colorMode and calculate color of each bin
    l->ApplyMarkerMode();                                                       // apply markerMode and calculate markersize of each bin
    l->ApplyTitles();                                                           // set BBox-title of each bin and computeBBox
  
    l->SetRnrChildren(fRender);
    gEve->AddElement(l);
    gEve->Redraw3D(kFALSE);
    fl=l;
  }
}
// --------------------------------------------------------------------------------
// returns 3D-vector with position data of hit
TVector3 CbmPointSetArrayDraw::GetVector(TObject* obj)
{
	CbmPixelHit* p = (CbmPixelHit*)obj;
	if (fVerbose > 3){
                cout <<"-I- CbmPointSetArrayDraw::GetVector(): " << p->GetX() << " " << p->GetY() << " " << p->GetZ() <<" "<< endl;
	}
        return TVector3(p->GetX(), p->GetY(), p->GetZ());
}
// --------------------------------------------------------------------------------
// returns hit-time against first hit
Double_t CbmPointSetArrayDraw::GetTime(TObject* obj)
{
       CbmPixelHit* p = (CbmPixelHit*)obj;
       if (fVerbose > 3){
               cout <<"-I- CbmPointSetArrayDraw::GetTime(): " << p->GetTime()-fTimeOffset << endl;
       }
       return p->GetTime()-fTimeOffset;
}
// --------------------------------------------------------------------------------
// returns ClusterSize of Hit
Int_t CbmPointSetArrayDraw::GetClusterSize(TObject* obj)
{
       //CluSize of TofHit is stored in Flag-Variable (set in Clusterizer)
       CbmTofHit* p= (CbmTofHit*)obj;
       Double_t cluSize=p->GetFlag();
       //Flag= #digis = 2*cluSize  +100 if used for track
       if (cluSize>100){
         cluSize=(cluSize-100)/2;
       }
       else {
         cluSize/=2;
       }
       if (fVerbose > 4){
               cout <<"-I- CbmPointSetArrayDraw::GetClusterSize(): " << cluSize << endl;
       }
       return cluSize;
}
// --------------------------------------------------------------------------------
// returns ToT of hit
Double_t CbmPointSetArrayDraw::GetTot(TObject* obj)
{
       // ToT of TofHit is stored in Channel-Variable (set in Clusterizer)
       CbmTofHit* p= (CbmTofHit*)obj;
       Double_t tot=Double_t(p->GetCh())/(20*GetClusterSize(p));
       if (fVerbose > 4){
               cout <<"-I- CbmPointSetArrayDraw::GetTot(): " << tot << endl;
       }
       return tot;
}
// --------------------------------------------------------------------------------
// returns Id of hit
Int_t CbmPointSetArrayDraw::GetPointId(TObject* obj)
{
       CbmPixelHit* p= (CbmPixelHit*)obj;
       return p->GetRefId();
}
// ---------------------------------------------------------------------------------
// Determine time of first hit in event to use as offset
void CbmPointSetArrayDraw::DetermineTimeOffset()
{
  Int_t npoints=fPointList->GetEntriesFast();
  fTimeOffset=115200000000000;  //32hours in ns as maximum of clock 
  fTimeMax=0;
  Double_t currtime;
  CbmPixelHit* hit;
  for (Int_t i=0; i<npoints; i++){ //loop over all hits in event
    hit=static_cast<CbmPixelHit*>(fPointList->At(i));
    currtime=hit->GetTime();
    if (currtime<fTimeOffset){
      fTimeOffset=currtime;
    }
    else if (currtime>fTimeMax){
      fTimeMax=currtime;
    }
  }
  fTimeMax-=fTimeOffset; //time of latest hit in event
  if (fVerbose > 4){
    cout<<std::setprecision(15)<<"-I- CbmPointSetArrayDraw::DetermineTimeBins: fTimeOffset "<<fTimeOffset<<endl;
  }
}

// -----   Destructor   ----------------------------------------------------
CbmPointSetArrayDraw::~CbmPointSetArrayDraw()
{
}
// -------------------------------------------------------------------------
void CbmPointSetArrayDraw::SetParContainers()
{

}
// -------------------------------------------------------------------------
/** Action after each event**/
void CbmPointSetArrayDraw::Finish()
{
}
// -------------------------------------------------------------------------
void CbmPointSetArrayDraw::Reset()
{
  if(fl!=0) {
    fl->RemoveElementsLocal();
    gEve->RemoveElement(fl, fEventManager );
  }
}


ClassImp(CbmPointSetArrayDraw);



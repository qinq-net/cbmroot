#include "CbmTutorial2DetDigitizer.h"

#include "CbmTutorial2DetDigiPar.h"
#include "CbmTutorial2DetPoint.h"

#include "FairRootManager.h"
#include "CbmMCTrack.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairBaseParSet.h"

#include "TRandom.h"
#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoNode.h"
#include "TGeoMatrix.h"
#include "TGeoBBox.h"

#include <iostream>
using std::cout;
using std::endl;

// ---- Default constructor -------------------------------------------
CbmTutorial2DetDigitizer::CbmTutorial2DetDigitizer()
    :FairTask("Tutorial2DetDigitizer")
	//:fRef(0)
{
  //    fDigiCollection = new TClonesArray("CbmTrdDigi");
}
// --------------------------------------------------------------------

// ---- Constructor ----------------------------------------------------
CbmTutorial2DetDigitizer::CbmTutorial2DetDigitizer(const char *name, const char *title)
	:FairTask(name)
{

}
// --------------------------------------------------------------------

// ---- Destructor ----------------------------------------------------
CbmTutorial2DetDigitizer::~CbmTutorial2DetDigitizer()
{
  //    FairRootManager *ioman =FairRootManager::Instance();
  //ioman->Write();
  //fDigiCollection->Clear("C");
  //delete fDigiCollection;

}
// --------------------------------------------------------------------

// ----  Initialisation  ----------------------------------------------
void CbmTutorial2DetDigitizer::SetParContainers()
{
    cout<<" * CbmTutorial2DetDigitizer * :: SetParContainers() "<<endl;


    // Get Base Container
    FairRunAna* ana = FairRunAna::Instance();
    FairRuntimeDb* rtdb=ana->GetRuntimeDb();

    fDigiPar = (CbmTutorial2DetDigiPar*)
               (rtdb->getContainer("CbmTutorial2DetDigiPar"));

    fDigiPar->printparams();
}
// --------------------------------------------------------------------

// ---- ReInit  -------------------------------------------------------
InitStatus CbmTutorial2DetDigitizer::ReInit(){

  cout<<" * CbmTutorial2DetDigitizer * :: ReInit() "<<endl;


  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

  fDigiPar = (CbmTutorial2DetDigiPar*)
    (rtdb->getContainer("CbmTutorial2DetDigiPar"));
  
  return kSUCCESS;
}
// --------------------------------------------------------------------

// ---- Init ----------------------------------------------------------
InitStatus CbmTutorial2DetDigitizer::Init()
{

    cout<<" * CbmTutorial2DetDigitizer * :: Init() "<<endl;

    FairRootManager *ioman = FairRootManager::Instance();
    if ( ! ioman ) Fatal("Init", "No FairRootManager");
    
    fTutorial2DetPoints=(TClonesArray *)  
      ioman->GetObject("Tutorial2DetPoint");
 
    if ( ! fTutorial2DetPoints ) {
      cout << "-W CbmTutorial2DetDigitizer::Init: No Tutorial2DetPoints array!" << endl;
      cout << "                            Task will be inactive" << endl;
      return kERROR;
    }

    //fListStack = (TClonesArray*)ioman->GetObject("MCTrack");
    //fDigiCollection = new TClonesArray("CbmTrdDigi", 100);
    //ioman->Register("TrdDigi","TRD Digis",fDigiCollection,kTRUE);

    return kSUCCESS;

}
// --------------------------------------------------------------------


// ---- Exec ----------------------------------------------------------
void CbmTutorial2DetDigitizer::Exec(Option_t * option)
{

  // Here we print something

  cout <<" I am in CbmTutorial2DetDigitizer::Exec" << endl;

  /*
    
  fNHits = 0;
  CbmTutorial2DetPoint *pt=NULL;
  
  Int_t nentries = fTutorial2DetPoints->GetEntriesFast();
  
  Double_t ELoss;       // total energy loss
  
  for (int j=0; j < nentries; j++ ) {
  
    pt = (CbmTutorial2DetPoint*) fTutorial2DetPoints->At(j);
  
    if(NULL == pt) continue;

//        Double_t x_mean = (pt->GetXIn()+pt->GetXOut())/2.;
//        Double_t y_mean = (pt->GetYIn()+pt->GetYOut())/2.;
//        Double_t z_mean = (pt->GetZIn()+pt->GetZOut())/2.;


        Double_t x_mean = pt->GetXOut();
        Double_t y_mean = pt->GetYOut();
        Double_t z_mean = pt->GetZOut();

        gGeoManager->FindNode(x_mean, y_mean, z_mean);
        TString path = gGeoManager->GetPath();
        Int_t ID =  gGeoManager->GetCurrentNodeId();

        TGeoNode *curnode = gGeoManager->GetCurrentNode();
        TGeoVolume *curvol = gGeoManager->GetCurrentVolume();
        TGeoMaterial *curmat =curvol->GetMedium()->GetMaterial();

        const Double_t *global_point = gGeoManager->GetCurrentPoint();
        Double_t local_point[3];  // global_point[3];
 
        gGeoManager->MasterToLocal(global_point, local_point);
        
        TGeoHMatrix *matrix = gGeoManager->GetCurrentMatrix(); 

        TGeoBBox *shape = (TGeoBBox*)curvol->GetShape(); 
        Float_t sizex = shape->GetDX();
        Float_t sizey = shape->GetDY();
        Float_t sizez = shape->GetDZ();

	//        cout <<"x_in: " << pt->GetXIn() << endl;
	// cout <<"x_mean: " << x_mean << endl;
        //cout <<"x_out: " << pt->GetXOut() << endl;

        //cout <<"y: " << y_mean << endl;

        //cout <<"z_in: " << pt->GetZIn() << endl;
        //cout <<"z_mean: " << z_mean << endl;
	//        cout <<"z_out: " << pt->GetZOut() << endl;


        if (matrix->IsRotation()){
          cout <<"Module is rotated" << endl;
	} else {
          cout <<"Module is not rotated" << endl;
	}


    }
  */
}
// --------------------------------------------------------------------

// --------------------------------------------------------------------

// ---- Finish --------------------------------------------------------
void CbmTutorial2DetDigitizer::Finish()
{
}
// --------------------------------------------------------------------

// ---- Register ------------------------------------------------------
void CbmTutorial2DetDigitizer::Register(){

  //FairRootManager::Instance()->Register("TrdDigi","Trd Digi", fDigiCollection, kTRUE);

}
// --------------------------------------------------------------------

ClassImp(CbmTutorial2DetDigitizer)

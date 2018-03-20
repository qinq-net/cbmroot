// -------------------------------------------------------------------------
// -----                CbmPsdSimpleDigitizer source file              -----
// -----                  Created 15/05/12  by  Alla & SELIM & FLORIAN -----
// -----                 Modified 17/03/18  by  Sergey Morozov         -----
// -------------------------------------------------------------------------
#include <iostream>

#include "TClonesArray.h"

#include "FairRootManager.h"
#include "FairLogger.h"

#include "CbmPsdDigi.h"
#include "CbmPsdSimpleDigitizer.h"
#include "CbmPsdPoint.h"
#include "TMath.h"

using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmPsdSimpleDigitizer::CbmPsdSimpleDigitizer() :
  FairTask("Simple Psd Digitizer",1),
  fNDigis(0),
  fRandom3(NULL),
  fPointArray(NULL),
  fDigiArray(NULL)
{
  //  Reset();
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmPsdSimpleDigitizer::~CbmPsdSimpleDigitizer()
{
  if ( fDigiArray ) {
    fDigiArray->Delete();
    delete fDigiArray;
  }
}
// -------------------------------------------------------------------------



// -----   Public method Init   --------------------------------------------
InitStatus CbmPsdSimpleDigitizer::Init() {

  fRandom3 = new TRandom3();

  // Get RootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman )
  {
      LOG(FATAL) << "CbmPsdSimpleDigitizer::Init: RootManager not instantised!" << FairLogger::endl;    //FLORIAN
      return kFATAL;
  }

  // Get input array
  fPointArray = (TClonesArray*) ioman->GetObject("PsdPoint");
  if ( ! fPointArray )
  {
      LOG(FATAL) << "CbmPsdSimpleDigitizer::Init: No PSDPoint array!" << FairLogger::endl;             //FLORIAN
      return kERROR;
  }

  // Create and register output array
  fDigiArray = new TClonesArray("CbmPsdDigi", 1000);
  ioman->Register("PsdDigi", "PSD", fDigiArray, IsOutputBranchPersistent("PsdDigi"));

  cout << "-I- CbmPsdSimpleDigitizer: Intialisation successfull " << kSUCCESS<< endl;
  return kSUCCESS;

}


// -------------------------------------------------------------------------



// -----   Public method Exec   --------------------------------------------
void CbmPsdSimpleDigitizer::Exec(Option_t* /*opt*/) {

  cout<<" CbmPsdSimpleDigitizer::Exec begin "<<endl;
  // Reset output array
   if ( ! fDigiArray ) Fatal("Exec", "No PsdDigi array");
  Reset();  // SELIM: reset!!!
   
  // Declare some variables
  CbmPsdPoint* point = NULL;
  Int_t modID   = -1;        // module ID
  Int_t scinID = -1;        // #sciillator

  Double_t edep[N_PSD_SECT][N_PSD_MODS];                 //SELIM: 49 modules, including central & corner modules (rejected in analysis/flow/eventPlane.cxx)
  memset(edep, 0,(N_PSD_SECT*N_PSD_MODS)*sizeof(Double_t));

  TVector3 pos;       // Position vector
  fNDigis=0;

  //for (Int_t imod=0; imod<100; imod++)                   //SELIM: 49 modules, including central & corner modules (rejected in analysis/flow/eventPlane.cxx)
  for (Int_t imod=0; imod<N_PSD_MODS; imod++)//marina
  {
    for (Int_t isec=0; isec<N_PSD_SECT; isec++)
      {
	  edep[isec][imod] = 0.;
      }
  }

  // Loop over PsdPoints
  Int_t nPoints = fPointArray->GetEntriesFast();
  cout<<" nPoints "<<nPoints<<endl;

  Int_t sec;

  for (Int_t iPoint=0; iPoint<nPoints; iPoint++)
  {
      point = (CbmPsdPoint*) fPointArray->At(iPoint);
      if ( ! point ) continue;

      modID = point->GetModuleID(); //marina  1-44 (45)
      scinID = point->GetDetectorID();//1-60
      Double_t eLoss = point->GetEnergyLoss();

      sec = (Int_t)((scinID-1)/6)+1;   //marina   1-10

      //cout <<"PSD modID,scinID,eloss " << modID << ", " << scinID << ", " << eLoss <<endl;

      if ( ((sec-1)>=0 && (sec-1)<N_PSD_SECT) && ((modID-1)>=0 && (modID-1)<N_PSD_MODS) ) {
        edep[sec-1][modID-1] += eLoss;
      }

  }// Loop over MCPoints

      for (Int_t imod=0; imod<N_PSD_MODS; imod++) {
        for (Int_t isec=0; isec<N_PSD_SECT; isec++) {
	  //if (edep[isec][imod]<=0.) cout << "!!  edep  !! : " << edep[isec][imod] << endl;
	  if ( edep[isec][imod] <= 0. ) continue;
	  else {
            Double_t eLossMIP = edep[isec][imod] / 0.005; // 5MeV per MIP
            Double_t pixPerMIP = 15.; // 15 pix per MIP
            Double_t eLossMIPSmeared =
                 fRandom3->Gaus(eLossMIP * pixPerMIP,sqrt(eLossMIP * pixPerMIP)) / pixPerMIP;
            Double_t eLossSmeared = eLossMIPSmeared * 0.005;
            Double_t eNoise = fRandom3->Gaus(0,15) / 50. * 0.005;
            eLossSmeared += eNoise;

	    new ((*fDigiArray)[fNDigis]) CbmPsdDigi(isec+1, imod+1, eLossSmeared);
	    fNDigis++;
	    cout <<"CbmPsdSimpleDigitizer " <<fNDigis <<" " <<isec+1 <<" " <<imod+1 <<" " << eLossSmeared <<endl; 
	  }
        }// section
      }//module

  // Event summary
  cout << "-I- CbmPsdSimpleDigitizer: " <<fNDigis<< " CbmPsdDigi created." << endl;

}
// -------------------------------------------------------------------------

// -----   Private method Reset   ------------------------------------------
void CbmPsdSimpleDigitizer::Reset() {
 fNDigis = 0;
 if ( fDigiArray ) fDigiArray->Delete();

}


ClassImp(CbmPsdSimpleDigitizer)

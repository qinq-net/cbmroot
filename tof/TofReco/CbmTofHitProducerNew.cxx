// --------------------------------------------------------------------------
// -----                 Class CbmTofHitProducerNew                    ------
// -----           Created  by E. Cordier 14/09/05                     ------
// -----           Modified by D. Gonzalez-Diaz 07/09/06               ------
// -----           Modified by D. Gonzalez-Diaz 02/02/07               ------
// -----           modified nh 24/10/2012                              ------
// --------------------------------------------------------------------------

#include "CbmTofHitProducerNew.h"

#include "CbmMCTrack.h"
#include "CbmMatch.h"
#include "CbmTofPoint.h"      // in cbmdata/tof
#include "CbmTofHit.h"        // in cbmdata/tof
#include "CbmTofGeoHandler.h" // in tof/TofTools
#include "CbmTofCell.h"       // in tof/TofData
#include "CbmTofDigiPar.h"    // in tof/TofParam

#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"

#include "TRandom.h"
#include "TString.h"
#include "TVector3.h"
#include "TSystem.h"
#include "TClonesArray.h"
#include "TH2.h"

#include <iostream>

using std::cout;
using std::endl;

// ---- Default constructor -------------------------------------------

CbmTofHitProducerNew::CbmTofHitProducerNew()
  : FairTask("CbmTofHitProducerNew"),
    fVerbose(1),
    fParFileName(""),
    fTofPoints(NULL),
    fMCTracks(NULL),
    fHitCollection(NULL),
    fTofHitMatches(NULL),
    X(),
    Dx(),
    Y(),
    Dy(),
    Z(),
    Ch(),
    ActSMtypMax(),
    ActnSMMax(),
    ActnModMax(),
    ActnCellMax(),
    ActSMtypMin(),
    ActnSMMin(),
    ActnModMin(),
    ActnCellMin(),
    tl(),
    tr(),
    trackID_left(),
    trackID_right(),
    point_left(),
    point_right(),
    match_fired(),
    fbUseOnePntPerTrkRpc(kFALSE),
    fvlTrckRpcAddr(),
    fvdTrckRpcTime(),
    fSigmaT(0.),
    fSigmaEl(0.),
    fSigmaXY(0.),
    fSigmaY(0.),
    fSigmaZ(0.),
    fVersion(""),
    fNHits(-1),
    fGeoHandler(new CbmTofGeoHandler()),
    fDigiPar(NULL),
    fCellInfo(NULL),
    fParInitFromAscii(kTRUE),
    fhSinglePointHitDeltaX(NULL),
    fhSinglePointHitDeltaY(NULL),
    fhSinglePointHitDeltaZ(NULL),
    fhSinglePointHitDeltaR(NULL),
    fhSinglePointHitDeltaT(NULL),
    fhSinglePointHitPullX(NULL),
    fhSinglePointHitPullY(NULL),
    fhSinglePointHitPullZ(NULL),
    fhSinglePointHitPullR(NULL),
    fhDiffPointHitLeftDeltaX(NULL),
    fhDiffPointHitLeftDeltaY(NULL),
    fhDiffPointHitLeftDeltaZ(NULL),
    fhDiffPointHitLeftDeltaR(NULL),
    fhDiffPointHitLeftDeltaT(NULL),
    fhDiffPointHitLeftPullX(NULL),
    fhDiffPointHitLeftPullY(NULL),
    fhDiffPointHitLeftPullZ(NULL),
    fhDiffPointHitLeftPullR(NULL),
    fhDiffPointHitRightDeltaX(NULL),
    fhDiffPointHitRightDeltaY(NULL),
    fhDiffPointHitRightDeltaZ(NULL),
    fhDiffPointHitRightDeltaR(NULL),
    fhDiffPointHitRightDeltaT(NULL),
    fhDiffPointHitRightPullX(NULL),
    fhDiffPointHitRightPullY(NULL),
    fhDiffPointHitRightPullZ(NULL),
    fhDiffPointHitRightPullR(NULL),
    fhSingleTrackHitLeftDeltaX(NULL),
    fhSingleTrackHitLeftDeltaY(NULL),
    fhSingleTrackHitLeftDeltaZ(NULL),
    fhSingleTrackHitLeftDeltaR(NULL),
    fhSingleTrackHitLeftDeltaT(NULL),
    fhSingleTrackHitLeftPullX(NULL),
    fhSingleTrackHitLeftPullY(NULL),
    fhSingleTrackHitLeftPullZ(NULL),
    fhSingleTrackHitLeftPullR(NULL),
    fhSingleTrackHitRightDeltaX(NULL),
    fhSingleTrackHitRightDeltaY(NULL),
    fhSingleTrackHitRightDeltaZ(NULL),
    fhSingleTrackHitRightDeltaR(NULL),
    fhSingleTrackHitRightDeltaT(NULL),
    fhSingleTrackHitRightPullX(NULL),
    fhSingleTrackHitRightPullY(NULL),
    fhSingleTrackHitRightPullZ(NULL),
    fhSingleTrackHitRightPullR(NULL),
    fhDiffTrackHitLeftDeltaX(NULL),
    fhDiffTrackHitLeftDeltaY(NULL),
    fhDiffTrackHitLeftDeltaZ(NULL),
    fhDiffTrackHitLeftDeltaR(NULL),
    fhDiffTrackHitLeftDeltaT(NULL),
    fhDiffTrackHitLeftPullX(NULL),
    fhDiffTrackHitLeftPullY(NULL),
    fhDiffTrackHitLeftPullZ(NULL),
    fhDiffTrackHitLeftPullR(NULL),
    fhDiffTrackHitRightDeltaX(NULL),
    fhDiffTrackHitRightDeltaY(NULL),
    fhDiffTrackHitRightDeltaZ(NULL),
    fhDiffTrackHitRightDeltaR(NULL),
    fhDiffTrackHitRightDeltaT(NULL),
    fhDiffTrackHitRightPullX(NULL),
    fhDiffTrackHitRightPullY(NULL),
    fhDiffTrackHitRightPullZ(NULL),
    fhDiffTrackHitRightPullR(NULL),
    fhNbPrimTrkHits(NULL),
    fhNbAllTrkHits(NULL),
    fhNbPrimTrkTofHits(NULL),
    fhNbAllTrkTofHits(NULL),
    fsHistosFileName("HitProdNew_QA.hst.root")
{
}


// ---- Constructor ----------------------------------------------------

CbmTofHitProducerNew::CbmTofHitProducerNew(const char *name, Int_t verbose)
  :FairTask(TString(name),verbose),
   fVerbose(1),
   fParFileName(""),
   fTofPoints(NULL),
   fMCTracks(NULL),
   fHitCollection(NULL),
   fTofHitMatches(NULL),
   X(),
   Dx(),
   Y(),
   Dy(),
   Z(),
   Ch(),
   ActSMtypMax(),
   ActnSMMax(),
   ActnModMax(),
   ActnCellMax(),
    ActSMtypMin(),
    ActnSMMin(),
    ActnModMin(),
    ActnCellMin(),
   tl(),
   tr(),
   trackID_left(),
   trackID_right(),
   point_left(),
   point_right(),
    match_fired(),
    fbUseOnePntPerTrkRpc(kFALSE),
    fvlTrckRpcAddr(),
    fvdTrckRpcTime(),
   fSigmaT(0.),
   fSigmaEl(0.),
   fSigmaXY(0.),
   fSigmaY(0.),
   fSigmaZ(0.),
   fVersion(""),
   fNHits(-1),
   fGeoHandler(new CbmTofGeoHandler()),
   fDigiPar(NULL),
   fCellInfo(NULL),
   fParInitFromAscii(kTRUE),
    fhSinglePointHitDeltaX(NULL),
    fhSinglePointHitDeltaY(NULL),
    fhSinglePointHitDeltaZ(NULL),
    fhSinglePointHitDeltaR(NULL),
    fhSinglePointHitDeltaT(NULL),
    fhSinglePointHitPullX(NULL),
    fhSinglePointHitPullY(NULL),
    fhSinglePointHitPullZ(NULL),
    fhSinglePointHitPullR(NULL),
    fhDiffPointHitLeftDeltaX(NULL),
    fhDiffPointHitLeftDeltaY(NULL),
    fhDiffPointHitLeftDeltaZ(NULL),
    fhDiffPointHitLeftDeltaR(NULL),
    fhDiffPointHitLeftDeltaT(NULL),
    fhDiffPointHitLeftPullX(NULL),
    fhDiffPointHitLeftPullY(NULL),
    fhDiffPointHitLeftPullZ(NULL),
    fhDiffPointHitLeftPullR(NULL),
    fhDiffPointHitRightDeltaX(NULL),
    fhDiffPointHitRightDeltaY(NULL),
    fhDiffPointHitRightDeltaZ(NULL),
    fhDiffPointHitRightDeltaR(NULL),
    fhDiffPointHitRightDeltaT(NULL),
    fhDiffPointHitRightPullX(NULL),
    fhDiffPointHitRightPullY(NULL),
    fhDiffPointHitRightPullZ(NULL),
    fhDiffPointHitRightPullR(NULL),
    fhSingleTrackHitLeftDeltaX(NULL),
    fhSingleTrackHitLeftDeltaY(NULL),
    fhSingleTrackHitLeftDeltaZ(NULL),
    fhSingleTrackHitLeftDeltaR(NULL),
    fhSingleTrackHitLeftDeltaT(NULL),
    fhSingleTrackHitLeftPullX(NULL),
    fhSingleTrackHitLeftPullY(NULL),
    fhSingleTrackHitLeftPullZ(NULL),
    fhSingleTrackHitLeftPullR(NULL),
    fhSingleTrackHitRightDeltaX(NULL),
    fhSingleTrackHitRightDeltaY(NULL),
    fhSingleTrackHitRightDeltaZ(NULL),
    fhSingleTrackHitRightDeltaR(NULL),
    fhSingleTrackHitRightDeltaT(NULL),
    fhSingleTrackHitRightPullX(NULL),
    fhSingleTrackHitRightPullY(NULL),
    fhSingleTrackHitRightPullZ(NULL),
    fhSingleTrackHitRightPullR(NULL),
    fhDiffTrackHitLeftDeltaX(NULL),
    fhDiffTrackHitLeftDeltaY(NULL),
    fhDiffTrackHitLeftDeltaZ(NULL),
    fhDiffTrackHitLeftDeltaR(NULL),
    fhDiffTrackHitLeftDeltaT(NULL),
    fhDiffTrackHitLeftPullX(NULL),
    fhDiffTrackHitLeftPullY(NULL),
    fhDiffTrackHitLeftPullZ(NULL),
    fhDiffTrackHitLeftPullR(NULL),
    fhDiffTrackHitRightDeltaX(NULL),
    fhDiffTrackHitRightDeltaY(NULL),
    fhDiffTrackHitRightDeltaZ(NULL),
    fhDiffTrackHitRightDeltaR(NULL),
    fhDiffTrackHitRightDeltaT(NULL),
    fhDiffTrackHitRightPullX(NULL),
    fhDiffTrackHitRightPullY(NULL),
    fhDiffTrackHitRightPullZ(NULL),
    fhDiffTrackHitRightPullR(NULL),
    fhNbPrimTrkHits(NULL),
    fhNbAllTrkHits(NULL),
    fhNbPrimTrkTofHits(NULL),
    fhNbAllTrkTofHits(NULL),
    fsHistosFileName("HitProdNew_QA.hst.root")
{
  cout << "CbmTofHitProducerNew instantiated with verbose = "<<fVerbose<<endl;
}


// ---- Destructor ----------------------------------------------------

CbmTofHitProducerNew::~CbmTofHitProducerNew()
{
//	FairRootManager *fManager =FairRootManager::Instance();
//	fManager->Write();
  if (fGeoHandler) {
    delete fGeoHandler;
  }
}

// --------------------------------------------------
void CbmTofHitProducerNew::SetParContainers()
{
  if (!fParInitFromAscii) {
    LOG(INFO)<<" Get the digi parameters for tof"<<FairLogger::endl;

    // Get Base Container
    FairRunAna* ana = FairRunAna::Instance();
    FairRuntimeDb* rtdb=ana->GetRuntimeDb();

    fDigiPar = (CbmTofDigiPar*)
               (rtdb->getContainer("CbmTofDigiPar"));

  }
}

// --------------------------------------------------------------------
InitStatus CbmTofHitProducerNew::ReInit()
{
  if (!fParInitFromAscii) {
    LOG(INFO)<<"Reinitialize the digi parameters for tof"<<FairLogger::endl;


    // Get Base Container
    FairRunAna* ana = FairRunAna::Instance();
    FairRuntimeDb* rtdb=ana->GetRuntimeDb();

    fDigiPar = (CbmTofDigiPar*)
               (rtdb->getContainer("CbmTofDigiPar"));

  }
   return kSUCCESS;
}

// ---- Init ----------------------------------------------------------

InitStatus CbmTofHitProducerNew::Init()
{
  CreateHistos();

  cout << "nh - version of CbmTofHitProducerNew initializing with file " << fParFileName << endl;
  FairRootManager *fManager = FairRootManager::Instance();

    fTofPoints  = (TClonesArray *) fManager->GetObject("TofPoint");
    fMCTracks   = (TClonesArray *) fManager->GetObject("MCTrack");
    
    // Initialize the TOF GeoHandler
    Bool_t isSimulation=kFALSE;
    /*Int_t bla =*/ fGeoHandler->Init(isSimulation);
/*    Int_t   nsmtyp=10, nsm=255, nmodules=10, ncells=255;*/
    Int_t iCh=0; // channel identifier 
  
    //    Initialize the matrixes [make this index visible in all the macro]. FIXME
/** Use the same constant variables as for the arrays initialization for safety and 
 ** to remove warnings
    for(int t=0;t<nsmtyp;t++){
     for(int i=0;i<nsm;i++){
      for(int j=0;j<nmodules;j++){
	for(int k=0;k<ncells;k++){
 **/
/*
    for(int t=0;t<maxSMtyp;t++){
     for(int i=0;i<maxnSM;i++){
      for(int j=0;j<maxnMod;j++){
	for(int k=0;k<maxnCell;k++){
	  X[t][i][j][k] = -1;
	  Y[t][i][j][k] = -1;
	  Z[t][i][j][k] = -1;
	  Dx[t][i][j][k]= -1;
	  Dy[t][i][j][k]= -1;
	  Ch[t][i][j][k]= -1;
	}
      }
     }
    }
*/

// initialize accounting variables    
    ActSMtypMax=0;
    ActSMtypMin= 2*maxSMtyp;
    for (Int_t i=0; i<maxSMtyp; i++){
	ActnSMMax[i]=0;
	ActnModMax[i]=0;
	ActnCellMax[i]=0;
        ActnSMMin[i]= 2*maxnSM;
        ActnModMin[i]= 2*maxnMod;
        ActnCellMin[i]= 2*maxnCell;
    }

    if (fParInitFromAscii) {
      FILE *par;

    //Reading the parameter file. In the future this must be done in a different way.
 
      char    header='#';
      int     module, cell, smodule, smtype;
      Float_t X_tmp, Y_tmp, Z_tmp, Dx_tmp, Dy_tmp;

      TString tofGeoFile = gSystem->Getenv("VMCWORKDIR");
//      tofGeoFile += "/parameters/tof/tof_standard.geom.par";
      if (fParFileName==""){
	tofGeoFile += "/parameters/tof/par_tof.txt";
      }else{
	tofGeoFile += fParFileName;
      }
      cout << "<I> CbmTofHitProducerNew::Read parameters from "<< fParFileName 
           << "," <<tofGeoFile << endl;
      par=fopen(tofGeoFile,"r");
   
      if(par==NULL) {
      printf("\n ERROR WHILE OPENING THE PARAMETER FILE IN TOF HIT PRODUCER!");
      return kFATAL;
    }
     

    //Skip the header. In the future the header structure must be defined. FIXME
    while (fscanf(par,"%c",&header)>=0){
//      cout << "ReadH "<< header << endl;
      if((int)(header-'0')==0) break;
    }

    //Read the first line
//    fscanf(par,"%5d %5d %5d %2d %5.1f %5.1f %5.1f %2.1f %2.1f ", 
    fscanf(par,"%d %d %d %d %f %f %f %f %f ", 
    &smodule, &module, &cell, &smtype, &X_tmp, &Y_tmp, &Z_tmp, &Dx_tmp, &Dy_tmp);
//    cout << "Read " << smodule << " mod " << module << " cell "<< cell << " SMtyp " <<  smtype << " X,Y " << X_tmp <<","<< Y_tmp <<endl;
    if(smtype>ActSMtypMax)             ActSMtypMax=smtype;
    if(smodule>ActnSMMax[ActSMtypMax]) ActnSMMax[ActSMtypMax]=smodule;
    if(module>ActnModMax[ActSMtypMax]) ActnModMax[ActSMtypMax]=module;
    if(cell>ActnCellMax[ActSMtypMax])  ActnCellMax[ActSMtypMax]=cell;
/*    
    X[smtype][smodule][module][cell]    = X_tmp/10.;
    Y[smtype][smodule][module][cell]    = Y_tmp/10.;
    Z[smtype][smodule][module][cell]    = Z_tmp/10.;
    Dx[smtype][smodule][module][cell]   = Dx_tmp/10.;
    Dy[smtype][smodule][module][cell]   = Dy_tmp/10.;
    Ch[smtype][smodule][module][cell]   = iCh++;
*/    
    //Read all the lines
     
    while(fscanf(par,"%5d %5d %5d %2d %f %f %f %f %f", 
      &smodule, &module, &cell, &smtype, &X_tmp, &Y_tmp, &Z_tmp, &Dx_tmp, &Dy_tmp)>=0){ 
//      cout << "Read " << smodule << " mod " << module << " cell "<< cell << " SMtyp " <<  smtype << " X,Y " << X_tmp <<","<< Y_tmp <<endl;
  
      if(smtype>ActSMtypMax)             ActSMtypMax=smtype;
      if(smodule>ActnSMMax[ActSMtypMax]) ActnSMMax[ActSMtypMax]=smodule;
      if(module>ActnModMax[ActSMtypMax]) ActnModMax[ActSMtypMax]=module;
      if(cell>ActnCellMax[ActSMtypMax])  ActnCellMax[ActSMtypMax]=cell;
/*
      X[smtype][smodule][module][cell]     = X_tmp/10.;  // units are cm 
      Y[smtype][smodule][module][cell]     = Y_tmp/10.;
      Z[smtype][smodule][module][cell]     = Z_tmp/10.;
      Dx[smtype][smodule][module][cell]    = Dx_tmp/10.;
      Dy[smtype][smodule][module][cell]    = Dy_tmp/10.;      
      Ch[smtype][smodule][module][cell]    = iCh++;
*/
    }

   // Initialize the storage vectors
   X.resize(ActSMtypMax + 1);
   Y.resize(ActSMtypMax + 1);
   Z.resize(ActSMtypMax + 1);
   Dx.resize(ActSMtypMax + 1);
   Dy.resize(ActSMtypMax + 1);
   Ch.resize(ActSMtypMax + 1);
   tl.resize(ActSMtypMax + 1);
   tr.resize(ActSMtypMax + 1);
   trackID_left.resize(ActSMtypMax + 1);
   trackID_right.resize(ActSMtypMax + 1);
   point_left.resize(ActSMtypMax + 1);
   point_right.resize(ActSMtypMax + 1);
   match_fired.resize(ActSMtypMax + 1);
   for( Int_t iType = 0; iType <= ActSMtypMax; iType++)
   {
      X[iType].resize(            ActnSMMax[iType] + 1);
      Y[iType].resize(            ActnSMMax[iType] + 1);
      Z[iType].resize(            ActnSMMax[iType] + 1);
      Dx[iType].resize(           ActnSMMax[iType] + 1);
      Dy[iType].resize(           ActnSMMax[iType] + 1);
      Ch[iType].resize(           ActnSMMax[iType] + 1);
      tl[iType].resize(           ActnSMMax[iType] + 1);
      tr[iType].resize(           ActnSMMax[iType] + 1);
      trackID_left[iType].resize( ActnSMMax[iType] + 1);
      trackID_right[iType].resize(ActnSMMax[iType] + 1);
      point_left[iType].resize(   ActnSMMax[iType] + 1);
      point_right[iType].resize(  ActnSMMax[iType] + 1);
      match_fired[iType].resize(  ActnSMMax[iType] + 1);
      for( Int_t iSm = 0; iSm <= ActnSMMax[iType]; iSm++)
      {
         X[iType][iSm].resize(            ActnModMax[iType] + 1);
         Y[iType][iSm].resize(            ActnModMax[iType] + 1);
         Z[iType][iSm].resize(            ActnModMax[iType] + 1);
         Dx[iType][iSm].resize(           ActnModMax[iType] + 1);
         Dy[iType][iSm].resize(           ActnModMax[iType] + 1);
         Ch[iType][iSm].resize(           ActnModMax[iType] + 1);
         tl[iType][iSm].resize(           ActnModMax[iType] + 1);
         tr[iType][iSm].resize(           ActnModMax[iType] + 1);
         trackID_left[iType][iSm].resize( ActnModMax[iType] + 1);
         trackID_right[iType][iSm].resize(ActnModMax[iType] + 1);
         point_left[iType][iSm].resize(   ActnModMax[iType] + 1);
         point_right[iType][iSm].resize(  ActnModMax[iType] + 1);
         match_fired[iType][iSm].resize(   ActnModMax[iType] + 1);
         for( Int_t iMod = 0; iMod <= ActnModMax[iType];  iMod++)
         {
            X[iType][iSm][iMod].resize(            ActnCellMax[iType] + 1, -1);
            Y[iType][iSm][iMod].resize(            ActnCellMax[iType] + 1, -1);
            Z[iType][iSm][iMod].resize(            ActnCellMax[iType] + 1, -1);
            Dx[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            Dy[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            Ch[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            tl[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            tr[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            trackID_left[iType][iSm][iMod].resize( ActnCellMax[iType] + 1, -1);
            trackID_right[iType][iSm][iMod].resize(ActnCellMax[iType] + 1, -1);
            point_left[iType][iSm][iMod].resize(   ActnCellMax[iType] + 1, -1);
            point_right[iType][iSm][iMod].resize(  ActnCellMax[iType] + 1, -1);
            match_fired[iType][iSm][iMod].resize(   ActnCellMax[iType] + 1, NULL);
            
            for(Int_t iCell = ActnCellMin[iType]; iCell <= ActnCellMax[iType]; iCell++)
               match_fired[iType][iSm][iMod][iCell]  = new CbmMatch();
               
         } // for( Int_t iMod = 0; iMod <= ActnModMax[iType];  iMod++)
      } // for( Int_t iMod = 0; iMod <= ActnSMMax[iType]; iMod++)
   } // for( Int_t type = 0; type <= ActSMtypMax; type++)

   // Read again all lines to fill the vectors
    // Go back to beginning 
    //Skip the header. In the future the header structure must be defined. FIXME
    while (fscanf(par,"%c",&header)>=0){
      if((int)(header-'0')==0) break;
    }

    //Read the first line
    fscanf(par,"%d %d %d %d %f %f %f %f %f ",
    &smodule, &module, &cell, &smtype, &X_tmp, &Y_tmp, &Z_tmp, &Dx_tmp, &Dy_tmp);

    X[smtype][smodule][module][cell]    = X_tmp/10.;
    Y[smtype][smodule][module][cell]    = Y_tmp/10.;
    Z[smtype][smodule][module][cell]    = Z_tmp/10.;
    Dx[smtype][smodule][module][cell]   = Dx_tmp/10.;
    Dy[smtype][smodule][module][cell]   = Dy_tmp/10.;
    Ch[smtype][smodule][module][cell]   = iCh++;

    //Read all the lines
    while(fscanf(par,"%5d %5d %5d %2d %f %f %f %f %f", 
      &smodule, &module, &cell, &smtype, &X_tmp, &Y_tmp, &Z_tmp, &Dx_tmp, &Dy_tmp)>=0){

      X[smtype][smodule][module][cell]     = X_tmp/10.;  // units are cm
      Y[smtype][smodule][module][cell]     = Y_tmp/10.;
      Z[smtype][smodule][module][cell]     = Z_tmp/10.;
      Dx[smtype][smodule][module][cell]    = Dx_tmp/10.;
      Dy[smtype][smodule][module][cell]    = Dy_tmp/10.;
      Ch[smtype][smodule][module][cell]    = iCh++;
    }

    fclose(par);
    cout << "Filled position array with ActSMtypMax " << ActSMtypMax 
         << " and " << iCh << " active Channels "<< endl;
   } else {
     cout <<" InitParametersFromContainer "<<endl;
     InitParametersFromContainer();
   }

    cout << "-I- CbmTofHitProducerNew: found following setup: "<<endl; 
    Int_t nCh=0;
    for (Int_t i=0; i<=ActSMtypMax; i++){
	cout << " SMtype " << i <<" nsmod " << ActnSMMax[i]+1 
             << " nmod "<< ActnModMax[i]+1  << " ncell " << ActnCellMax[i] << endl;
	nCh += (ActnSMMax[i]+1) * (ActnModMax[i]+1) * ActnCellMax[i] * 2;
    }
    
    fHitCollection = new TClonesArray("CbmTofHit");
    fManager->Register("TofHit","Tof",fHitCollection, kTRUE);

    fTofHitMatches = new TClonesArray("CbmMatch");
    fManager->Register("TofHitMatch","TOF",fTofHitMatches, kTRUE);
     
    cout << "-I- CbmTofHitProducerNew: Initialization successful for " 
         << nCh <<" electronics channels"<< endl;

    return kSUCCESS;
}

void CbmTofHitProducerNew::InitParametersFromContainer()
{

   //    Initialize the matrixes [make this index visible in all the macro]. FIXME
/*    Int_t   nsmtyp=10, nsm=255, nmodules=10, ncells=255;*/

/** Use the same constant variables as for the arrays initialization for safety and 
 ** to remove warnings and error => the used size for the first dimensions was
 ** bigger than the real size of the array
    for(int t=0;t<nsmtyp;t++){
     for(int i=0;i<nsm;i++){
      for(int j=0;j<nmodules;j++){
	for(int k=0;k<ncells;k++){
 **/
/*
    for(int t=0;t<maxSMtyp;t++){
     for(int i=0;i<maxnSM;i++){
      for(int j=0;j<maxnMod;j++){
	for(int k=0;k<maxnCell;k++){
	  X[t][i][j][k] = -1;
	  Y[t][i][j][k] = -1;
	  Z[t][i][j][k] = -1;
	  Dx[t][i][j][k]= -1;
	  Dy[t][i][j][k]= -1;
	  Ch[t][i][j][k]= -1;
	}
      }
     }
    }
*/
   Int_t nrOfCells = fDigiPar->GetNrOfModules();
   LOG(INFO)<<"Parameter container contain "<<nrOfCells<<" cells."<<FairLogger::endl;

   for (Int_t icell = 0; icell < nrOfCells; ++icell) {

     Int_t cellId = fDigiPar->GetCellId(icell); // cellId is assigned in CbmTofCreateDigiPar
     fCellInfo =fDigiPar->GetCell(cellId);

     Int_t smtype  = fGeoHandler->GetSMType(cellId);
     Int_t smodule = fGeoHandler->GetSModule(cellId);
     Int_t module  = fGeoHandler->GetCounter(cellId);
     Int_t cell    = fGeoHandler->GetCell(cellId);

     Double_t x = fCellInfo->GetX();
     Double_t y = fCellInfo->GetY();
     Double_t z = fCellInfo->GetZ();
     Double_t dx = fCellInfo->GetSizex();
     Double_t dy = fCellInfo->GetSizey();

     if(icell < 0){
       cout << "-I- InitPar "<<icell<<" Id: "<<cellId
	    << " "<< cell << " tmcs: "<< smtype <<" "<<smodule<<" "<<module<<" "<<cell   
            << " x="<<Form("%6.2f",x)<<" y="<<Form("%6.2f",y)<<" z="<<Form("%6.2f",z)
            <<" dx="<<dx<<" dy="<<dy<<endl;
     }
    if(smtype>ActSMtypMax)        ActSMtypMax=smtype;
    if(smodule>ActnSMMax[smtype]) ActnSMMax[smtype]=smodule;
    if(module>ActnModMax[smtype]) ActnModMax[smtype]=module;
    if(cell>ActnCellMax[smtype])  ActnCellMax[smtype]=cell;

    if(smtype<ActSMtypMin)        ActSMtypMin=smtype;
    if(smodule<ActnSMMin[smtype]) ActnSMMin[smtype]=smodule;
    if(module<ActnModMin[smtype]) ActnModMin[smtype]=module;
    if(cell<ActnCellMin[smtype])  ActnCellMin[smtype]=cell;
/* 
     X[smtype][smodule][module][cell] = x;
     Y[smtype][smodule][module][cell] = y;
     Z[smtype][smodule][module][cell] = z;
     Dx[smtype][smodule][module][cell]= dx;
     Dy[smtype][smodule][module][cell]= dy;
     Ch[smtype][smodule][module][cell]= icell;
*/
   }

   // Initialize the storage vectors
   X.resize(ActSMtypMax + 1);
   Y.resize(ActSMtypMax + 1);
   Z.resize(ActSMtypMax + 1);
   Dx.resize(ActSMtypMax + 1);
   Dy.resize(ActSMtypMax + 1);
   Ch.resize(ActSMtypMax + 1);
   tl.resize(ActSMtypMax + 1);
   tr.resize(ActSMtypMax + 1);
   trackID_left.resize(ActSMtypMax + 1);
   trackID_right.resize(ActSMtypMax + 1);
   point_left.resize(ActSMtypMax + 1);
   point_right.resize(ActSMtypMax + 1);
   match_fired.resize(ActSMtypMax + 1);
   for( Int_t iType = 0; iType <= ActSMtypMax; iType++)
   {
      X[iType].resize(            ActnSMMax[iType] + 1);
      Y[iType].resize(            ActnSMMax[iType] + 1);
      Z[iType].resize(            ActnSMMax[iType] + 1);
      Dx[iType].resize(           ActnSMMax[iType] + 1);
      Dy[iType].resize(           ActnSMMax[iType] + 1);
      Ch[iType].resize(           ActnSMMax[iType] + 1);
      tl[iType].resize(           ActnSMMax[iType] + 1);
      tr[iType].resize(           ActnSMMax[iType] + 1);
      trackID_left[iType].resize( ActnSMMax[iType] + 1);
      trackID_right[iType].resize(ActnSMMax[iType] + 1);
      point_left[iType].resize(   ActnSMMax[iType] + 1);
      point_right[iType].resize(  ActnSMMax[iType] + 1);
      match_fired[iType].resize(   ActnSMMax[iType] + 1);
      for( Int_t iSm = 0; iSm <= ActnSMMax[iType]; iSm++)
      {
         X[iType][iSm].resize(            ActnModMax[iType] + 1);
         Y[iType][iSm].resize(            ActnModMax[iType] + 1);
         Z[iType][iSm].resize(            ActnModMax[iType] + 1);
         Dx[iType][iSm].resize(           ActnModMax[iType] + 1);
         Dy[iType][iSm].resize(           ActnModMax[iType] + 1);
         Ch[iType][iSm].resize(           ActnModMax[iType] + 1);
         tl[iType][iSm].resize(           ActnModMax[iType] + 1);
         tr[iType][iSm].resize(           ActnModMax[iType] + 1);
         trackID_left[iType][iSm].resize( ActnModMax[iType] + 1);
         trackID_right[iType][iSm].resize(ActnModMax[iType] + 1);
         point_left[iType][iSm].resize(   ActnModMax[iType] + 1);
         point_right[iType][iSm].resize(  ActnModMax[iType] + 1);
         match_fired[iType][iSm].resize(   ActnModMax[iType] + 1);
         for( Int_t iMod = 0; iMod <= ActnModMax[iType];  iMod++)
         {
            X[iType][iSm][iMod].resize(            ActnCellMax[iType] + 1, -1);
            Y[iType][iSm][iMod].resize(            ActnCellMax[iType] + 1, -1);
            Z[iType][iSm][iMod].resize(            ActnCellMax[iType] + 1, -1);
            Dx[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            Dy[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            Ch[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            tl[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            tr[iType][iSm][iMod].resize(           ActnCellMax[iType] + 1, -1);
            trackID_left[iType][iSm][iMod].resize( ActnCellMax[iType] + 1, -1);
            trackID_right[iType][iSm][iMod].resize(ActnCellMax[iType] + 1, -1);
            point_left[iType][iSm][iMod].resize(   ActnCellMax[iType] + 1, -1);
            point_right[iType][iSm][iMod].resize(  ActnCellMax[iType] + 1, -1);
            match_fired[iType][iSm][iMod].resize(   ActnCellMax[iType] + 1, NULL);
            
            for(Int_t iCell = ActnCellMin[iType]; iCell <= ActnCellMax[iType]; iCell++)
               match_fired[iType][iSm][iMod][iCell]  = new CbmMatch();
               
         } // for( Int_t iMod = 0; iMod <= ActnModMax[iType];  iMod++)
      } // for( Int_t iMod = 0; iMod <= ActnSMMax[iType]; iMod++)
   } // for( Int_t type = 0; type <= ActSMtypMax; type++)

   // Loop again to fill the vectors
   for (Int_t icell = 0; icell < nrOfCells; ++icell) {

     Int_t cellId = fDigiPar->GetCellId(icell); // cellId is assigned in CbmTofCreateDigiPar
     fCellInfo =fDigiPar->GetCell(cellId);

     Int_t smtype  = fGeoHandler->GetSMType(cellId);
     Int_t smodule = fGeoHandler->GetSModule(cellId);
     Int_t module  = fGeoHandler->GetCounter(cellId);
     Int_t cell    = fGeoHandler->GetCell(cellId); 

     Double_t x = fCellInfo->GetX();
     Double_t y = fCellInfo->GetY();
     Double_t z = fCellInfo->GetZ();
     Double_t dx = fCellInfo->GetSizex();
     Double_t dy = fCellInfo->GetSizey();

     X[smtype][smodule][module][cell] = x;
     Y[smtype][smodule][module][cell] = y;
     Z[smtype][smodule][module][cell] = z;
     Dx[smtype][smodule][module][cell]= dx;
     Dy[smtype][smodule][module][cell]= dy;
     Ch[smtype][smodule][module][cell]= icell;
   }

    LOG(INFO) << "CbmTofHitProducerNew::InitParametersFromContainer => Number of SM types: "
              << ActSMtypMax << " Min is " << ActSMtypMin << FairLogger::endl;
    for (Int_t i=0; i<=ActSMtypMax; i++)
    {
       LOG(INFO) << "CbmTofHitProducerNew::InitParametersFromContainer => SM Type "
                 << i <<" Max Nb SM " << ActnSMMax[i]
                 << " Max Nb RPC " << ActnModMax[i]
                 << " Max Nb Channels " << ActnCellMax[i]
                 << FairLogger::endl;
       LOG(INFO) << "CbmTofHitProducerNew::InitParametersFromContainer => SM Type "
                 << i <<" Min Nb SM " << ActnSMMin[i]
                 << " Min Nb RPC " << ActnModMin[i]
                 << " Min Nb Channels " << ActnCellMin[i]
                 << FairLogger::endl;
    }
}

// ---- Exec ----------------------------------------------------------

void CbmTofHitProducerNew::Exec(Option_t * /*option*/)
{
  fHitCollection->Clear();
  fTofHitMatches->Delete();
  fNHits = -1;              //Must start in -1

  CbmTofPoint *pt;
  CbmMCTrack  *mc;
  
  Int_t nTofPoint = fTofPoints->GetEntries();  
  Int_t nMCTracks = fMCTracks ->GetEntries();
  Int_t nMCTracks_vert = 0;
  Int_t tof_tracks = 0, tof_tracks_vert = 0/*, tof_tracks_local = 0*/;

  cout << "-I- CbmTofHitProducerNew(Exec): " << nTofPoint
       << " points in Tof for this event with " << nMCTracks
       << " MC tracks "<< endl;

   // Prepare the temporary storing of the Track/Point/Digi info
   if( kTRUE == fbUseOnePntPerTrkRpc )
   {
      fvlTrckRpcAddr.resize( nMCTracks );
      fvdTrckRpcTime.resize( nMCTracks );
   } // if( kTRUE == fbUseOnePntPerTrkRpc )

  //Some numbers on TOF distributions

  for(Int_t p=0;p<nMCTracks;p++) {
    mc = (CbmMCTrack*) fMCTracks->At(p);
    if(mc->GetNPoints(kTOF)>0) tof_tracks++;
    if(mc->GetNPoints(kTOF)>0 && mc->GetMotherId()==-1) tof_tracks_vert++;
    if(mc->GetMotherId()==-1) nMCTracks_vert++;
    
      if( kTRUE == fbUseOnePntPerTrkRpc )
      {
         fvlTrckRpcAddr[p].clear();
         fvdTrckRpcTime[p].clear();
      } // if( kTRUE == fbUseOnePntPerTrkRpc )
  }
  
  cout << "-I- CbmTofHitProducerNew : " << tof_tracks << " tracks in Tof " << endl;
  cout << "-I- CbmTofHitProducerNew : " << tof_tracks_vert << " tracks in Tof from vertex" << endl;
//  cout << "-I- CbmTofHitProducerNew : " << tof_tracks-tof_tracks_local
//       << " tracks in Tof able to produce a hit" << endl;

  TVector3 pos;

  Double_t xHit, yHit, zHit, tHit, xHitErr, yHitErr, zHitErr;
  Double_t tl_new, tr_new;
  Double_t Dz=2.04;  //FIXME: Introduce also Dz and Z as (constant) parameters 
  Double_t sigma_T=0.098;
/*  Double_t sigma_Y=0.7;*/
/*  Double_t sigma_t_gap;*/
/*  Double_t t_o;*/
  Double_t T_smearing = 0, sigma_el=0.04, 
    vprop = 15., Pgap = 0.75;
  //time[ns], position[cm], velocity[cm/ns]
  //FIXME: these parameters must be provided externally

  Int_t trackID, smtype, smodule, module, cell, gap, flag, ref;
  
  if  (fSigmaT>0.) sigma_T=fSigmaT;   // single gap resolution 
  else fSigmaT=sigma_T;               // take default 

  if  (fSigmaEl>0.) sigma_el=fSigmaEl;   // electronics channel resolution 
  else fSigmaEl=sigma_el;                // take default

  //Here check for the validity of the parameters
  if(fSigmaY>1) cout<<"UNREALISTIC TOF POSITION RESOLUTION!! (HitProducer may crash)"<<endl;
  if((fSigmaT<0.01 && fSigmaT>0)||fSigmaT>0.2) cout<<"UNREALISTIC TOF RESOLUTION!! (HitProducer may crash)"<<endl;

  //Parameterizations. Now they depend on the geometry/algorithm. FIXME

//  if(fSigmaY!=0)    sigma_el = sqrt(2.)*fSigmaY/vprop*1.3;
//  else              sigma_el = sqrt(2.)*sigma_Y/vprop*1.3;

//  if(fSigmaT!=0)  {  
//    sigma_t_gap = sqrt(pow(fSigmaT/(0.5*pow(ngaps,-0.361) + 0.40),2)-1.4/1.3*pow(sigma_el,2));
//    t_o = 1.93*fSigmaT;
//  }
//  else {
//    sigma_t_gap = sqrt(pow(sigma_T/(0.5*pow(ngaps,-0.361) + 0.40),2)-1.4/1.3*pow(sigma_el,2));
//    t_o = 1.93*sigma_T;
//  }

  //Initialization of cell times

  for(   Int_t t = ActSMtypMin;    t <= ActSMtypMax;    t++){
   for(  Int_t i = ActnSMMin[t];   i <= ActnSMMax[t];   i++){
    for( Int_t j = ActnModMin[t];  j <= ActnModMax[t];  j++){
     for(Int_t k = ActnCellMin[t]; k <= ActnCellMax[t]; k++){
      tl[t][i][j][k]= 1e+5;
      tr[t][i][j][k]= 1e+5;
      match_fired[t][i][j][k] ->ClearLinks();
     }
    }
   }
  }

// Loop over the TOF points
//  nTofPoint=1; //debugging 
  
  for (Int_t j=0; j < nTofPoint; j++ ) {
    // Probability that the avalanche is detected
    if(gRandom->Uniform(1)>Pgap)     continue;
    // Get a pointer to the TOF point      
    pt = (CbmTofPoint*) fTofPoints->At(j);
    if(pt == NULL) {
      cout<<"Be careful: hole in the CbmTofPoint TClonesArray!"<<endl;
      continue;
    }
    //Reject particles produced in the last 4 cm. Better job must be done here. For example:
    //it could better to go up to the parent particle and get its trackID, then the
    //secondary is processed. FIXME.
    //if(pt->GetTrackID() < 0) continue;
    mc = (CbmMCTrack*) fMCTracks-> At(pt->GetTrackID()); // pointer to MC - info
    //if((mc->GetStartZ())>996) continue;
    //Get relevant information from the point

    trackID = pt->GetTrackID();
    Int_t detID = pt->GetDetectorID();

    cell    = fGeoHandler->GetCell(detID);
    module  = fGeoHandler->GetCounter(detID);
    gap     = fGeoHandler->GetGap(detID);
    smodule = fGeoHandler->GetSModule(detID);
    smtype  = fGeoHandler->GetSMType(detID);
    Int_t cellID = fGeoHandler->GetCellId(detID);
   //    fCellInfo =fDigiPar->GetCell(cellID);

    if( //0){
          smtype  < ActSMtypMin         || smtype  > ActSMtypMax
       || smodule < ActnSMMin[smtype]   || smodule > ActnSMMax[smtype] // May lead to seg fault
       || module  < ActnModMin[smtype]  || module  > ActnModMax[smtype]   // May lead to seg fault
       || cell    < ActnCellMin[smtype] || cell    > ActnCellMax[smtype]    // May lead to seg fault
       || Dx[smtype][smodule][module][cell]<0.
       )
    {
     LOG(INFO)<<"-E- TofHitProducerNew: detId: "<< detID <<" SMType: "<<smtype;
     LOG(INFO)<<" SModule: "<<smodule<<" of "<<ActnSMMax[smtype]+1;
     LOG(INFO)<<" Module: "<<module<<" of "<<ActnModMax[smtype]+1;
     LOG(INFO)<<" Gap: "<<gap;
     LOG(INFO)<<" Cell: "<<cell<<" of "<<ActnCellMax[smtype]+1 <<FairLogger::endl;
     continue;
    }
    pt->Position(pos);

    if(fVerbose >2) {
      pt->Print(""); //FIXME
      cout << endl;
    }

//    T_smearing      = gRandom->Gaus(t_o, sigma_t_gap);
    T_smearing      = gRandom->Gaus(1.21*sigma_T, sigma_T);

    Float_t X_local = pos.X()-X[smtype][smodule][module][cell];
    Float_t Y_local = pos.Y()-Y[smtype][smodule][module][cell];
    
    tl_new = pt->GetTime() + T_smearing - Y_local/vprop
           + gRandom->Gaus(0,sigma_el);
    tr_new = pt->GetTime() + T_smearing + Y_local/vprop
           + gRandom->Gaus(0,sigma_el);
           
   // Check if there was already a Point from the same track created in this RPC
   if( kTRUE == fbUseOnePntPerTrkRpc )
   {
      ULong64_t uRpcAddr = ( smtype*1000 + smodule )*1000
                           + module;
      Bool_t bFoundIt = kFALSE;
      UInt_t uTrkRpcPair = 0;
      for( uTrkRpcPair = 0; uTrkRpcPair < fvlTrckRpcAddr[trackID].size(); uTrkRpcPair ++)
         if( uRpcAddr == fvlTrckRpcAddr[trackID][uTrkRpcPair])
         {
            bFoundIt = kTRUE;
            break;
         }
      // If it is the case, we should reuse the timing already assigned to this (track, RPC) pair
      if( kTRUE == bFoundIt)
      {
         tl_new = fvdTrckRpcTime[trackID][uTrkRpcPair] 
                  - Y_local/vprop
                  + gRandom->Gaus(0,sigma_el);
         tr_new = fvdTrckRpcTime[trackID][uTrkRpcPair] 
                  + Y_local/vprop
                  + gRandom->Gaus(0,sigma_el);
      } // Already a point in this RPC for this Track
         else
         {
            fvlTrckRpcAddr[trackID].push_back(uRpcAddr);
            fvdTrckRpcTime[trackID].push_back(pt->GetTime() + T_smearing);              
         } // No Point yet in this RPC for this Track
   } // if( kTRUE == fbUseOnePntPerTrkRpc )

    if(fVerbose >1 || TMath::Abs(X_local)>1.5) {
      cout << "-W- TofHitProNew " << j <<". Poi," 
         << " TID:" << trackID 
	 << " detID: " << detID
	 << " cellID: " << cellID
         << " SMtype: " << smtype 
         << " SM: " << smodule 
         << " Mod: " << module 
         << " Str: " << cell 
         << " G: " << gap 
	   << " posX " << Form("%6.2f",pos.X())<<","<< Form("%6.2f",X[smtype][smodule][module][cell])
	   << " posY " << Form("%6.2f",pos.Y())<<","<< Form("%6.2f",Y[smtype][smodule][module][cell])
	   << " tl " << Form("%6.2f",tl_new)   << " tr " << Form("%6.2f",tr_new)
         << endl; 

      if(TMath::Abs(X_local)>1.5) {
        continue; //prevent crashes 
      }
    }
    
    //Take the fastest time from all the points/gaps in this cell
    
    if(tl_new<tl[smtype][smodule][module][cell]) {
      tl[smtype][smodule][module][cell]             = tl_new;
      trackID_left[smtype][smodule][module][cell]   = trackID;
      point_left[smtype][smodule][module][cell]     = j;
    }

    if(tr_new<tr[smtype][smodule][module][cell]) {
      tr[smtype][smodule][module][cell]             = tr_new;
      trackID_right[smtype][smodule][module][cell]  = trackID;
      point_right[smtype][smodule][module][cell]    = j;
    }
    //X and Y depend on the orientation of the cell. FIXME
    
    // Keep track of all Points firing the cells, even the slow ones
      match_fired[smtype][smodule][module][cell] ->AddLink(CbmLink(0.,j));
  } // end of loop on TOF points 

// Extract TOF Hits
  
  Int_t nFl1=0;
  Int_t nFl2=0;
  Int_t ii=0;

  //  fVerbose=3;  // debugging 

  for(   Int_t t = ActSMtypMin;    t <= ActSMtypMax;    t++){
   for(  Int_t i = ActnSMMin[t];   i <= ActnSMMax[t];   i++){
    for( Int_t j = ActnModMin[t];  j <= ActnModMax[t];  j++){
     for(Int_t k = ActnCellMin[t]; k <= ActnCellMax[t]; k++){

       //      cout <<"-D- HitProd: tijk "<<t<<" "<<i<<" "<<j<<" "<<k<<" "<<tl[t][i][j][k]<<" "<<tr[t][i][j][k]<<endl;
//Increase the counter for the TofHit TClonesArray if the first time a hit is attached to this cell
    
      if( tl[t][i][j][k]<1e+5 
       && tr[t][i][j][k]<1e+5) {
       fNHits++;

       xHit    = X[t][i][j][k]; 
       xHitErr = Dx[t][i][j][k]/sqrt(12.);
       zHit    = Z[t][i][j][k];
       zHitErr = Dz/sqrt(12.);
       yHit    = (tr[t][i][j][k] - tl[t][i][j][k])*vprop*0.5
               + Y[t][i][j][k];
       
    //Reference to the point that contributes to the left side.
       yHitErr = sigma_el*vprop;
       tHit    = 0.5*(tl[t][i][j][k] + tr[t][i][j][k]);
       ref     = point_left[t][i][j][k];
       pt      = (CbmTofPoint*) fTofPoints->At(ref);
       if(trackID_left[t][i][j][k]==trackID_right[t][i][j][k]){
	   flag = 1; nFl1++;
           // Check consistency
           if(fVerbose >2) {
           pt->Position(pos);
           cout << " pos check for point "<<ref<<" x:  "<< xHit << " " << pos.X() 
                << " y: " << yHit << " " << pos.Y() << endl;   
	   }
       }
       else {
	   flag = 2; nFl2++;
       }
       
      // Check hit quality
      if(point_left[t][i][j][k]==point_right[t][i][j][k])
      {
         CbmTofPoint * ptqa = (CbmTofPoint*) fTofPoints->At(point_left[t][i][j][k]);
         // Obtain Point position
         TVector3 vPntPos;
         ptqa->Position( vPntPos );
         
         Double_t dDeltaX = xHit - vPntPos.X();
         Double_t dDeltaY = yHit - vPntPos.Y();
         Double_t dDeltaZ = zHit - vPntPos.Z();
         Double_t dDeltaR = TMath::Sqrt(   dDeltaX*dDeltaX
                                         + dDeltaY*dDeltaY );
         Double_t rHitErr = TMath::Sqrt(   xHitErr*xHitErr
                                         + yHitErr*yHitErr );
         fhSinglePointHitDeltaX->Fill(dDeltaX);
         fhSinglePointHitDeltaY->Fill(dDeltaY);
         fhSinglePointHitDeltaZ->Fill(dDeltaZ);
         fhSinglePointHitDeltaR->Fill(dDeltaR);
         fhSinglePointHitDeltaT->Fill( 1000.0*(tHit - ptqa->GetTime()) );
         fhSinglePointHitPullX->Fill(dDeltaX/xHitErr);
         fhSinglePointHitPullY->Fill(dDeltaY/yHitErr);
         fhSinglePointHitPullZ->Fill(dDeltaZ/zHitErr);
         fhSinglePointHitPullR->Fill(dDeltaR/rHitErr);
      } // if(point_left[t][i][j][k]==point_right[t][i][j][k])
         else
         {
            // Left Point   
            CbmTofPoint * ptqaL = (CbmTofPoint*) fTofPoints->At(point_left[t][i][j][k]);
            // Obtain Point position
            TVector3 vPntPosL;
            ptqaL->Position( vPntPosL );
            
            Double_t dDeltaXL = xHit - vPntPosL.X();
            Double_t dDeltaYL = yHit - vPntPosL.Y();
            Double_t dDeltaZL = zHit - vPntPosL.Z();
            Double_t dDeltaRL = TMath::Sqrt(   dDeltaXL*dDeltaXL
                                             + dDeltaYL*dDeltaYL );
            Double_t rHitErr = TMath::Sqrt(   xHitErr*xHitErr
                                            + yHitErr*yHitErr );
            fhDiffPointHitLeftDeltaX->Fill(dDeltaXL);
            fhDiffPointHitLeftDeltaY->Fill(dDeltaYL);
            fhDiffPointHitLeftDeltaZ->Fill(dDeltaZL);
            fhDiffPointHitLeftDeltaR->Fill(dDeltaRL);
            fhDiffPointHitLeftDeltaT->Fill( 1000.0*(tHit - ptqaL->GetTime()) );
            fhDiffPointHitLeftPullX->Fill(dDeltaXL/xHitErr);
            fhDiffPointHitLeftPullY->Fill(dDeltaYL/yHitErr);
            fhDiffPointHitLeftPullZ->Fill(dDeltaZL/zHitErr);
            fhDiffPointHitLeftPullR->Fill(dDeltaRL/rHitErr);
            
            // Right Point
            CbmTofPoint * ptqaR = (CbmTofPoint*) fTofPoints->At(point_right[t][i][j][k]);
            // Obtain Point position
            TVector3 vPntPosR;
            ptqaR->Position( vPntPosR );
            
            Double_t dDeltaXR = xHit - vPntPosR.X();
            Double_t dDeltaYR = yHit - vPntPosR.Y();
            Double_t dDeltaZR = zHit - vPntPosR.Z();
            Double_t dDeltaRR = TMath::Sqrt(  dDeltaXR*dDeltaXR
                                            + dDeltaYR*dDeltaYR );
            fhDiffPointHitRightDeltaX->Fill(dDeltaXR);
            fhDiffPointHitRightDeltaY->Fill(dDeltaYR);
            fhDiffPointHitRightDeltaZ->Fill(dDeltaZR);
            fhDiffPointHitRightDeltaR->Fill(dDeltaRR);
            fhDiffPointHitRightDeltaT->Fill( 1000.0*(tHit - ptqaR->GetTime()) );
            fhDiffPointHitRightPullX->Fill(dDeltaXR/xHitErr);
            fhDiffPointHitRightPullY->Fill(dDeltaYR/yHitErr);
            fhDiffPointHitRightPullZ->Fill(dDeltaZR/zHitErr);
            fhDiffPointHitRightPullR->Fill(dDeltaRR/rHitErr);
            
            if(trackID_left[t][i][j][k]==trackID_right[t][i][j][k])
            {
               // Single Track but different points
               fhSingleTrackHitLeftDeltaX->Fill(dDeltaXL);
               fhSingleTrackHitLeftDeltaY->Fill(dDeltaYL);
               fhSingleTrackHitLeftDeltaZ->Fill(dDeltaZL);
               fhSingleTrackHitLeftDeltaR->Fill(dDeltaRL);
               fhSingleTrackHitLeftDeltaT->Fill( 1000.0*(tHit - ptqaL->GetTime()) );
               fhSingleTrackHitLeftPullX->Fill(dDeltaXL/xHitErr);
               fhSingleTrackHitLeftPullY->Fill(dDeltaYL/yHitErr);
               fhSingleTrackHitLeftPullZ->Fill(dDeltaZL/zHitErr);
               fhSingleTrackHitLeftPullR->Fill(dDeltaRL/rHitErr);
               
               fhSingleTrackHitRightDeltaX->Fill(dDeltaXR);
               fhSingleTrackHitRightDeltaY->Fill(dDeltaYR);
               fhSingleTrackHitRightDeltaZ->Fill(dDeltaZR);
               fhSingleTrackHitRightDeltaR->Fill(dDeltaRR);
               fhSingleTrackHitRightDeltaT->Fill( 1000.0*(tHit - ptqaR->GetTime()) );
               fhSingleTrackHitRightPullX->Fill(dDeltaXR/xHitErr);
               fhSingleTrackHitRightPullY->Fill(dDeltaYR/yHitErr);
               fhSingleTrackHitRightPullZ->Fill(dDeltaZR/zHitErr);
               fhSingleTrackHitRightPullR->Fill(dDeltaRR/rHitErr);
            } // if(trackID_left[t][i][j][k]==trackID_right[t][i][j][k])
            else
            {
               // multiple tracks
               fhDiffTrackHitLeftDeltaX->Fill(dDeltaXL);
               fhDiffTrackHitLeftDeltaY->Fill(dDeltaYL);
               fhDiffTrackHitLeftDeltaZ->Fill(dDeltaZL);
               fhDiffTrackHitLeftDeltaR->Fill(dDeltaRL);
               fhDiffTrackHitLeftDeltaT->Fill( 1000.0*(tHit - ptqaL->GetTime()) );
               fhDiffTrackHitLeftPullX->Fill(dDeltaXL/xHitErr);
               fhDiffTrackHitLeftPullY->Fill(dDeltaYL/yHitErr);
               fhDiffTrackHitLeftPullZ->Fill(dDeltaZL/zHitErr);
               fhDiffTrackHitLeftPullR->Fill(dDeltaRL/rHitErr);
               
               fhDiffTrackHitRightDeltaX->Fill(dDeltaXR);
               fhDiffTrackHitRightDeltaY->Fill(dDeltaYR);
               fhDiffTrackHitRightDeltaZ->Fill(dDeltaZR);
               fhDiffTrackHitRightDeltaR->Fill(dDeltaRR);
               fhDiffTrackHitRightDeltaT->Fill( 1000.0*(tHit - ptqaR->GetTime()) );
               fhDiffTrackHitRightPullX->Fill(dDeltaXR/xHitErr);
               fhDiffTrackHitRightPullY->Fill(dDeltaYR/yHitErr);
               fhDiffTrackHitRightPullZ->Fill(dDeltaZR/zHitErr);
               fhDiffTrackHitRightPullR->Fill(dDeltaRR/rHitErr);
            } // else of if(trackID_left[t][i][j][k]==trackID_right[t][i][j][k])
         } // else of if(point_left[t][i][j][k]==point_right[t][i][j][k])
       
       TVector3 hitPos(xHit, yHit, zHit);
       TVector3 hitPosErr(xHitErr, yHitErr, zHitErr);
       Int_t iCh = Ch[t][i][j][k];
       if(fVerbose >1) {
	 //       if(1) {
	 cout << ii++ << " Add hit smt " << t << " sm " << i << " mod " << j << " str " << k
	 <<" Ch " << iCh
         <<" tl " << tl[t][i][j][k] << " tr " << tr[t][i][j][k] 
	 <<" xh " << xHit << " yh " << yHit << " fl "<< flag << " refPoi " << ref 
	 <<" TID "<<  trackID_left[t][i][j][k] <<","<<trackID_right[t][i][j][k]<< endl;
       }
       
       AddHit(pt->GetDetectorID(), hitPos, hitPosErr, ref, tHit, flag, iCh);
       
       CbmMatch* hitMatch = new CbmMatch();
       
       // Add the links with weight 0 for all point firing the channel
       hitMatch->AddLinks( *(match_fired[t][i][j][k]) ); 
       // Add the links for the fastest signals
       hitMatch->AddLink(CbmLink(0.5,point_left[t][i][j][k]));
       hitMatch->AddLink(CbmLink(0.5,point_right[t][i][j][k]));
       
       new((*fTofHitMatches)[fNHits]) CbmMatch(*hitMatch);
       delete hitMatch;
      }
     }
    }
   }
  }  
  cout << "-I- CbmTofHitProducerNew : " << fNHits+1
       << " hits in Tof created, "<< nFl1<< " single, "<< nFl2<< " multiple hits " << endl;


   Int_t iNbTofHits = fHitCollection->GetEntries();
   fhNbPrimTrkHits->Fill(    nMCTracks_vert,  iNbTofHits );
   fhNbAllTrkHits->Fill(     nMCTracks,       iNbTofHits );
   fhNbPrimTrkTofHits->Fill( tof_tracks_vert, iNbTofHits );
   fhNbAllTrkTofHits->Fill(  tof_tracks,      iNbTofHits );
   
   // Clear the Track to channel temporary storage
   if( kTRUE == fbUseOnePntPerTrkRpc )
   {
      for(Int_t iTrkInd = 0; iTrkInd < nMCTracks; iTrkInd++)
      {
         fvlTrckRpcAddr[iTrkInd].clear();
         fvdTrckRpcTime[iTrkInd].clear();
      } // for(Int_t iTrkInd = 0; iTrkInd < nMCTracks; iTrkInd++)
      fvlTrckRpcAddr.clear();
      fvdTrckRpcTime.clear();
   } // if( kTRUE == fbUseOnePntPerTrkRpc )
}
// ---- Add Hit to HitCollection --------------------------------------

void CbmTofHitProducerNew::AddHit(Int_t detID, TVector3 &posHit, TVector3 &posHitErr,
			       Int_t ref, Double_t tHit, Int_t flag, Int_t /*iChannel*/)
{
//  new((*fHitCollection)[fNHits]) CbmTofHit(detID, posHit, posHitErr, ref, tHit, flag, iChannel);
  new((*fHitCollection)[fNHits]) CbmTofHit(detID, posHit, posHitErr, ref, tHit, flag);

  if(fVerbose > 1) {
    CbmTofHit* tofHit = (CbmTofHit*) fHitCollection->At(fNHits);
    tofHit->Print();
    cout << endl;
  }
}


// ---- Finish --------------------------------------------------------

void CbmTofHitProducerNew::Finish()
{
   WriteHistos();
   DeleteHistos();
}


// ---- SetSigmaT -----------------------------------------------------

void CbmTofHitProducerNew::SetSigmaT(Double_t sigma)
{
    fSigmaT = sigma;
}


// ---- SetSigmaEl -----------------------------------------------------

void CbmTofHitProducerNew::SetSigmaEl(Double_t sigma)
{
    fSigmaEl = sigma;
}

// ---- SetSigmaXY -----------------------------------------------------

void CbmTofHitProducerNew::SetSigmaXY(Double_t sigma)
{
    fSigmaXY = sigma;
}

// ---- SetSigmaY -----------------------------------------------------

void CbmTofHitProducerNew::SetSigmaY(Double_t sigma)
{
    fSigmaY = sigma;
}


// ---- SetSigmaZ -----------------------------------------------------

void CbmTofHitProducerNew::SetSigmaZ(Double_t sigma)
{
    fSigmaZ = sigma;
}


// ---- GetSigmaT -----------------------------------------------------

Double_t CbmTofHitProducerNew::GetSigmaT()
{  
    return  fSigmaT;
}


// ---- GetSigmaEl -----------------------------------------------------

Double_t CbmTofHitProducerNew::GetSigmaEl()
{  
    return  fSigmaEl;
}

// ---- GetSigmaXY -----------------------------------------------------

Double_t CbmTofHitProducerNew::GetSigmaXY()
{  
    return  fSigmaXY;
}

// ---- GetSigmaY -----------------------------------------------------

Double_t CbmTofHitProducerNew::GetSigmaY()
{  
    return  fSigmaY;
}

// ---- GetSigmaZ -----------------------------------------------------

Double_t CbmTofHitProducerNew::GetSigmaZ()
{  
    return  fSigmaZ;
}

void CbmTofHitProducerNew::CreateHistos()
{
   // Range hit deviation from MC
   Int_t    iNbBinsDeltaPos  =  600;
   Double_t dDeltaPosRange   =   30; // cm
   Int_t    iNbBinsDeltaTime = 1000;
   Double_t dDeltaTimeRange  = 1000; // ps 
   // Range pulls from MC
   Int_t    iNbBinsPullPos  =  500;
   Double_t dPullPosRange   =    5; 
         
   fhSinglePointHitDeltaX = new TH1D("HitProd_SinglePointHitDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit coming from a single MC Point; X(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaY = new TH1D("HitProd_SinglePointHitDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit coming from a single MC Point; Y(Hit) - Y(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaZ = new TH1D("HitProd_SinglePointHitDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit coming from a single MC Point; Z(Hit) - Z(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaR = new TH1D("HitProd_SinglePointHitDeltaR", 
                              "Quality of the Tof Hits position, for hit coming from a single MC Point; R(Hit -> Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaT = new TH1D("HitProd_SinglePointHitDeltaT", 
                              "Quality of the Tof Hits Time, for hit coming from a single MC Point; T(hit) - T(Point) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhSinglePointHitPullX = new TH1D("HitProd_SinglePointHitPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit coming from a single MC Point; Pull X(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSinglePointHitPullY = new TH1D("HitProd_SinglePointHitPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit coming from a single MC Point; Pull Y(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSinglePointHitPullZ = new TH1D("HitProd_SinglePointHitPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit coming from a single MC Point; Pull Z(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSinglePointHitPullR = new TH1D("HitProd_SinglePointHitPullR", 
                              "Quality of the Tof Hits position error, for hit coming from a single MC Point; Pull R(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
         
   fhDiffPointHitLeftDeltaX = new TH1D("HitProd_DiffPointHitLeftDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit from diff. MC Point, vs left point; X(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffPointHitLeftDeltaY = new TH1D("HitProd_DiffPointHitLeftDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit from diff. MC Point, vs left point; Y(Hit) - Y(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffPointHitLeftDeltaZ = new TH1D("HitProd_DiffPointHitLeftDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit from diff. MC Point, vs left point; Z(Hit) - Z(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffPointHitLeftDeltaR = new TH1D("HitProd_DiffPointHitLeftDeltaR", 
                              "Quality of the Tof Hits position, for hit from diff. MC Point, vs left point; R(Hit -> Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffPointHitLeftDeltaT = new TH1D("HitProd_DiffPointHitLeftDeltaT", 
                              "Quality of the Tof Hits Time, for hit from diff. MC Point, vs left point; T(hit) - T(Point) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhDiffPointHitLeftPullX = new TH1D("HitProd_DiffPointHitLeftPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit from diff. MC Point, vs left point; Pull X(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffPointHitLeftPullY = new TH1D("HitProd_DiffPointHitLeftPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit from diff. MC Point, vs left point; Pull Y(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffPointHitLeftPullZ = new TH1D("HitProd_DiffPointHitLeftPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit from diff. MC Point, vs left point; Pull Z(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffPointHitLeftPullR = new TH1D("HitProd_DiffPointHitLeftPullR", 
                              "Quality of the Tof Hits position error, for hit from diff. MC Point, vs left point; Pull R(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
         
   fhDiffPointHitRightDeltaX = new TH1D("HitProd_DiffPointHitRightDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit from diff. MC Point, vs right point; X(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffPointHitRightDeltaY = new TH1D("HitProd_DiffPointHitRightDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit from diff. MC Point, vs right point; Y(Hit) - Y(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffPointHitRightDeltaZ = new TH1D("HitProd_DiffPointHitRightDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit from diff. MC Point, vs right point; Z(Hit) - Z(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffPointHitRightDeltaR = new TH1D("HitProd_DiffPointHitRightDeltaR", 
                              "Quality of the Tof Hits position, for hit from diff. MC Point, vs right point; R(Hit -> Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffPointHitRightDeltaT = new TH1D("HitProd_DiffPointHitRightDeltaT", 
                              "Quality of the Tof Hits Time, for hit from diff. MC Point, vs right point; T(hit) - T(Point) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhDiffPointHitRightPullX = new TH1D("HitProd_DiffPointHitRightPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit from diff. MC Point, vs right point; Pull X(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffPointHitRightPullY = new TH1D("HitProd_DiffPointHitRightPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit from diff. MC Point, vs right point; Pull Y(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffPointHitRightPullZ = new TH1D("HitProd_DiffPointHitRightPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit from diff. MC Point, vs right point; Pull Z(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffPointHitRightPullR = new TH1D("HitProd_DiffPointHitRightPullR", 
                              "Quality of the Tof Hits position error, for hit from diff. MC Point, vs right point; Pull R(Hit -> Point) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
            
            
         
   fhSingleTrackHitLeftDeltaX = new TH1D("HitProd_SingleTrackHitLeftDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit coming from a single MC Track but multi Pnt; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitLeftDeltaY = new TH1D("HitProd_SingleTrackHitLeftDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit coming from a single MC Track but multi Pnt; Y(Hit) - Y(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitLeftDeltaZ = new TH1D("HitProd_SingleTrackHitLeftDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit coming from a single MC Track but multi Pnt; Z(Hit) - Z(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitLeftDeltaR = new TH1D("HitProd_SingleTrackHitLeftDeltaR", 
                              "Quality of the Tof Hits position, for hit coming from a single MC Track but multi Pnt; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitLeftDeltaT = new TH1D("HitProd_SingleTrackHitLeftDeltaT", 
                              "Quality of the Tof Hits Time, for hit coming from a single MC Track but multi Pnt; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhSingleTrackHitLeftPullX = new TH1D("HitProd_SingleTrackHitLeftPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit coming from a single MC Track but multi Pnt; Pull X(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitLeftPullY = new TH1D("HitProd_SingleTrackHitLeftPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit coming from a single MC Track but multi Pnt; Pull Y(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitLeftPullZ = new TH1D("HitProd_SingleTrackHitLeftPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit coming from a single MC Track but multi Pnt; Pull Z(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitLeftPullR = new TH1D("HitProd_SingleTrackHitLeftPullR", 
                              "Quality of the Tof Hits position error, for hit coming from a single MC Track but multi Pnt; Pull R(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
                              
   fhSingleTrackHitRightDeltaX = new TH1D("HitProd_SingleTrackHitRightDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit coming from a single MC Track but multi Pnt; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitRightDeltaY = new TH1D("HitProd_SingleTrackHitRightDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit coming from a single MC Track but multi Pnt; Y(Hit) - Y(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitRightDeltaZ = new TH1D("HitProd_SingleTrackHitRightDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit coming from a single MC Track but multi Pnt; Z(Hit) - Z(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitRightDeltaR = new TH1D("HitProd_SingleTrackHitRightDeltaR", 
                              "Quality of the Tof Hits position, for hit coming from a single MC Track but multi Pnt; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitRightDeltaT = new TH1D("HitProd_SingleTrackHitRightDeltaT", 
                              "Quality of the Tof Hits Time, for hit coming from a single MC Track but multi Pnt; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhSingleTrackHitRightPullX = new TH1D("HitProd_SingleTrackHitRightPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit coming from a single MC Track but multi Pnt; Pull X(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitRightPullY = new TH1D("HitProd_SingleTrackHitRightPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit coming from a single MC Track but multi Pnt; Pull Y(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitRightPullZ = new TH1D("HitProd_SingleTrackHitRightPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit coming from a single MC Track but multi Pnt; Pull Z(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhSingleTrackHitRightPullR = new TH1D("HitProd_SingleTrackHitRightPullR", 
                              "Quality of the Tof Hits position error, for hit coming from a single MC Track but multi Pnt; Pull R(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
         
   fhDiffTrackHitLeftDeltaX = new TH1D("HitProd_DiffTrackHitLeftDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit from diff. MC Track, vs left point; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffTrackHitLeftDeltaY = new TH1D("HitProd_DiffTrackHitLeftDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit from diff. MC Track, vs left point; Y(Hit) - Y(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffTrackHitLeftDeltaZ = new TH1D("HitProd_DiffTrackHitLeftDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit from diff. MC Track, vs left point; Z(Hit) - Z(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffTrackHitLeftDeltaR = new TH1D("HitProd_DiffTrackHitLeftDeltaR", 
                              "Quality of the Tof Hits position, for hit from diff. MC Track, vs left point; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffTrackHitLeftDeltaT = new TH1D("HitProd_DiffTrackHitLeftDeltaT", 
                              "Quality of the Tof Hits Time, for hit from diff. MC Track, vs left point; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhDiffTrackHitLeftPullX = new TH1D("HitProd_DiffTrackHitLeftPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit from diff. MC Track, vs left point; Pull X(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffTrackHitLeftPullY = new TH1D("HitProd_DiffTrackHitLeftPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit from diff. MC Track, vs left point; Pull Y(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffTrackHitLeftPullZ = new TH1D("HitProd_DiffTrackHitLeftPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit from diff. MC Track, vs left point; Pull Z(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffTrackHitLeftPullR = new TH1D("HitProd_DiffTrackHitLeftPullR", 
                              "Quality of the Tof Hits position error, for hit from diff. MC Track, vs left point; Pull R(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
         
   fhDiffTrackHitRightDeltaX = new TH1D("HitProd_DiffTrackHitRightDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit from diff. MC Track, vs right point; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffTrackHitRightDeltaY = new TH1D("HitProd_DiffTrackHitRightDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit from diff. MC Track, vs right point; Y(Hit) - Y(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffTrackHitRightDeltaZ = new TH1D("HitProd_DiffTrackHitRightDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit from diff. MC Track, vs right point; Z(Hit) - Z(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffTrackHitRightDeltaR = new TH1D("HitProd_DiffTrackHitRightDeltaR", 
                              "Quality of the Tof Hits position, for hit from diff. MC Track, vs right point; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhDiffTrackHitRightDeltaT = new TH1D("HitProd_DiffTrackHitRightDeltaT", 
                              "Quality of the Tof Hits Time, for hit from diff. MC Track, vs right point; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   fhDiffTrackHitRightPullX = new TH1D("HitProd_DiffTrackHitRightPullX", 
                              "Quality of the Tof Hits position error on X axis, for hit from diff. MC Track, vs right point; Pull X(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffTrackHitRightPullY = new TH1D("HitProd_DiffTrackHitRightPullY", 
                              "Quality of the Tof Hits position error on Y axis, for hit from diff. MC Track, vs right point; Pull Y(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffTrackHitRightPullZ = new TH1D("HitProd_DiffTrackHitRightPullZ", 
                              "Quality of the Tof Hits position error on Z axis, for hit from diff. MC Track, vs right point; Pull Z(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
   fhDiffTrackHitRightPullR = new TH1D("HitProd_DiffTrackHitRightPullR", 
                              "Quality of the Tof Hits position error, for hit from diff. MC Track, vs right point; Pull R(Hit -> Track) []; # [Hits]",
                              iNbBinsPullPos, -dPullPosRange, dPullPosRange);
                              
   fhNbPrimTrkHits          = new TH2D("HitProd_NbPrimTrkHits", 
                              "Nb of hits VS nb of Primary MC tracks for each event; # Primary MC tracks []; # Hits []; Events []",
                              100, -0.5, 99.5,
                              100, -0.5, 99.5 );
   fhNbAllTrkHits           = new TH2D("HitProd_NbAllTrkHits", 
                              "Nb of hits VS nb of MC tracks for each event; # MC tracks []; # Hits []; Events []",
                              100, -0.5, 99.5,
                              100, -0.5, 99.5 );
   fhNbPrimTrkTofHits       = new TH2D("HitProd_NbPrimTrkTofHits", 
                              "Nb of hits VS nb of Primary MC tracks with TOF Points for each event; # Primary TOF MC tracks []; # Hits []; Events []",
                              100, -0.5, 99.5,
                              100, -0.5, 99.5 );
   fhNbAllTrkTofHits        = new TH2D("HitProd_NbAllTrkTofHits", 
                              "Nb of hits VS nb of MC tracks with TOF Points for each event; # TOF MC tracks []; # Hits []; Events []",
                              100, -0.5, 99.5,
                              100, -0.5, 99.5 );
}

void CbmTofHitProducerNew::WriteHistos()
{
   if( "" != fsHistosFileName )
   {
      // Write histogramms to the file
      TDirectory * oldir = gDirectory;
      TFile *fHist = new TFile( fsHistosFileName,"RECREATE");
      fHist->cd();
      
      fhSinglePointHitDeltaX->Write();
      fhSinglePointHitDeltaY->Write();
      fhSinglePointHitDeltaZ->Write();
      fhSinglePointHitDeltaR->Write();
      fhSinglePointHitDeltaT->Write();
      fhSinglePointHitPullX->Write();
      fhSinglePointHitPullY->Write();
      fhSinglePointHitPullZ->Write();
      fhSinglePointHitPullR->Write();
      fhDiffPointHitLeftDeltaX->Write();
      fhDiffPointHitLeftDeltaY->Write();
      fhDiffPointHitLeftDeltaZ->Write();
      fhDiffPointHitLeftDeltaR->Write();
      fhDiffPointHitLeftDeltaT->Write();
      fhDiffPointHitLeftPullX->Write();
      fhDiffPointHitLeftPullY->Write();
      fhDiffPointHitLeftPullZ->Write();
      fhDiffPointHitLeftPullR->Write();
      fhDiffPointHitRightDeltaX->Write();
      fhDiffPointHitRightDeltaY->Write();
      fhDiffPointHitRightDeltaZ->Write();
      fhDiffPointHitRightDeltaR->Write();
      fhDiffPointHitRightDeltaT->Write();
      fhDiffPointHitRightPullX->Write();
      fhDiffPointHitRightPullY->Write();
      fhDiffPointHitRightPullZ->Write();
      fhDiffPointHitRightPullR->Write();
      
      fhSingleTrackHitLeftDeltaX->Write();
      fhSingleTrackHitLeftDeltaY->Write();
      fhSingleTrackHitLeftDeltaZ->Write();
      fhSingleTrackHitLeftDeltaR->Write();
      fhSingleTrackHitLeftDeltaT->Write();
      fhSingleTrackHitLeftPullX->Write();
      fhSingleTrackHitLeftPullY->Write();
      fhSingleTrackHitLeftPullZ->Write();
      fhSingleTrackHitLeftPullR->Write();
      fhSingleTrackHitRightDeltaX->Write();
      fhSingleTrackHitRightDeltaY->Write();
      fhSingleTrackHitRightDeltaZ->Write();
      fhSingleTrackHitRightDeltaR->Write();
      fhSingleTrackHitRightDeltaT->Write();
      fhSingleTrackHitRightPullX->Write();
      fhSingleTrackHitRightPullY->Write();
      fhSingleTrackHitRightPullZ->Write();
      fhSingleTrackHitRightPullR->Write();
      fhDiffTrackHitLeftDeltaX->Write();
      fhDiffTrackHitLeftDeltaY->Write();
      fhDiffTrackHitLeftDeltaZ->Write();
      fhDiffTrackHitLeftDeltaR->Write();
      fhDiffTrackHitLeftDeltaT->Write();
      fhDiffTrackHitLeftPullX->Write();
      fhDiffTrackHitLeftPullY->Write();
      fhDiffTrackHitLeftPullZ->Write();
      fhDiffTrackHitLeftPullR->Write();
      fhDiffTrackHitRightDeltaX->Write();
      fhDiffTrackHitRightDeltaY->Write();
      fhDiffTrackHitRightDeltaZ->Write();
      fhDiffTrackHitRightDeltaR->Write();
      fhDiffTrackHitRightDeltaT->Write();
      fhDiffTrackHitRightPullX->Write();
      fhDiffTrackHitRightPullY->Write();
      fhDiffTrackHitRightPullZ->Write();
      fhDiffTrackHitRightPullR->Write();
      
      fhNbPrimTrkHits->Write();
      fhNbAllTrkHits->Write();
      fhNbPrimTrkTofHits->Write();
      fhNbAllTrkTofHits->Write();
      
      gDirectory->cd( oldir->GetPath() );

      fHist->Close();
   } // if( "" != fsHistosFileName )
}

void CbmTofHitProducerNew::DeleteHistos()
{
   delete fhSinglePointHitDeltaX;
   delete fhSinglePointHitDeltaY;
   delete fhSinglePointHitDeltaZ;
   delete fhSinglePointHitDeltaR;
   delete fhSinglePointHitDeltaT;
   delete fhSinglePointHitPullX;
   delete fhSinglePointHitPullY;
   delete fhSinglePointHitPullZ;
   delete fhSinglePointHitPullR;
   delete fhDiffPointHitLeftDeltaX;
   delete fhDiffPointHitLeftDeltaY;
   delete fhDiffPointHitLeftDeltaZ;
   delete fhDiffPointHitLeftDeltaR;
   delete fhDiffPointHitLeftDeltaT;
   delete fhDiffPointHitLeftPullX;
   delete fhDiffPointHitLeftPullY;
   delete fhDiffPointHitLeftPullZ;
   delete fhDiffPointHitLeftPullR;
   delete fhDiffPointHitRightDeltaX;
   delete fhDiffPointHitRightDeltaY;
   delete fhDiffPointHitRightDeltaZ;
   delete fhDiffPointHitRightDeltaR;
   delete fhDiffPointHitRightDeltaT;
   delete fhDiffPointHitRightPullX;
   delete fhDiffPointHitRightPullY;
   delete fhDiffPointHitRightPullZ;
   delete fhDiffPointHitRightPullR;
   
   delete fhSingleTrackHitLeftDeltaX;
   delete fhSingleTrackHitLeftDeltaY;
   delete fhSingleTrackHitLeftDeltaZ;
   delete fhSingleTrackHitLeftDeltaR;
   delete fhSingleTrackHitLeftDeltaT;
   delete fhSingleTrackHitLeftPullX;
   delete fhSingleTrackHitLeftPullY;
   delete fhSingleTrackHitLeftPullZ;
   delete fhSingleTrackHitLeftPullR;
   delete fhSingleTrackHitRightDeltaX;
   delete fhSingleTrackHitRightDeltaY;
   delete fhSingleTrackHitRightDeltaZ;
   delete fhSingleTrackHitRightDeltaR;
   delete fhSingleTrackHitRightDeltaT;
   delete fhSingleTrackHitRightPullX;
   delete fhSingleTrackHitRightPullY;
   delete fhSingleTrackHitRightPullZ;
   delete fhSingleTrackHitRightPullR;
   delete fhDiffTrackHitLeftDeltaX;
   delete fhDiffTrackHitLeftDeltaY;
   delete fhDiffTrackHitLeftDeltaZ;
   delete fhDiffTrackHitLeftDeltaR;
   delete fhDiffTrackHitLeftDeltaT;
   delete fhDiffTrackHitLeftPullX;
   delete fhDiffTrackHitLeftPullY;
   delete fhDiffTrackHitLeftPullZ;
   delete fhDiffTrackHitLeftPullR;
   delete fhDiffTrackHitRightDeltaX;
   delete fhDiffTrackHitRightDeltaY;
   delete fhDiffTrackHitRightDeltaZ;
   delete fhDiffTrackHitRightDeltaR;
   delete fhDiffTrackHitRightDeltaT;
   delete fhDiffTrackHitRightPullX;
   delete fhDiffTrackHitRightPullY;
   delete fhDiffTrackHitRightPullZ;
   delete fhDiffTrackHitRightPullR;
   
   delete fhNbPrimTrkHits;
   delete fhNbAllTrkHits;
   delete fhNbPrimTrkTofHits;
   delete fhNbAllTrkTofHits;
}

ClassImp(CbmTofHitProducerNew)


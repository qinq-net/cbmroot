/** @file CbmTofHitFinderQa.cxx
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 27/08/2015
 **/
 
#include "CbmTofHitFinderQa.h"

// TOF Classes and includes
#include "CbmTofPoint.h"      // in cbmdata/tof
#include "CbmTofDigi.h"       // in cbmdata/tof
#include "CbmTofDigiExp.h"    // in cbmdata/tof
#include "CbmTofHit.h"        // in cbmdata/tof
#include "CbmTofGeoHandler.h" // in tof/TofTools
#include "CbmTofDetectorId_v12b.h" // in cbmdata/tof
#include "CbmTofDetectorId_v14a.h" // in cbmdata/tof
#include "CbmTofCell.h"       // in tof/TofData
#include "CbmTofDigiPar.h"    // in tof/TofParam
#include "CbmTofDigiBdfPar.h" // in tof/TofParam
#include "CbmTofAddress.h"    // in cbmdata/tof

// CBMroot classes and includes
#include "CbmMCTrack.h"
#include "CbmMatch.h"

// FAIR classes and includes
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"
#include "FairLogger.h"

// ROOT Classes and includes
#include "Riostream.h"
#include "TClonesArray.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TFile.h"
#include "TMath.h"
#include "TRandom.h"
#include "TROOT.h"

//___________________________________________________________________
//
// CbmTofHitFinderQa
//
// Task for QA of TOF event based hit finders and digitizers
//
// ------------------------------------------------------------------
CbmTofHitFinderQa::CbmTofHitFinderQa()
  : FairTask("TofHitFinderQa"),
    fEvents(0),
    fMCEventHeader(NULL),
    fGeoHandler(new CbmTofGeoHandler()),
    fTofId(NULL),
    fChannelInfo(NULL),
    iNbSmTot(0),
    fvTypeSmOffs(),
    iNbRpcTot(0),
    fvSmRpcOffs(),
    iNbChTot(0),
    fvRpcChOffs(),
    fDigiPar(NULL),
    fDigiBdfPar(NULL),
    fTofPointsColl(NULL),
    fMcTracksColl(NULL),
    fTofDigisColl(NULL),
    fTofDigiMatchPointsColl(NULL),
    fTofHitsColl(NULL),
    fTofDigiMatchColl(NULL),
    fbNormHistGenMode( kFALSE ),
    fsHistoInNormCartFilename(""),  
    fsHistoInNormAngFilename(""),
    fsHistoInNormSphFilename(""),
    fsHistoOutFilename("./tofQa.hst.root"),
    fdWallPosZ(1000),
    fhTrackMapXY(NULL),  // Only when creating normalization histos
    fhTrackMapXZ(NULL),  // Only when creating normalization histos
    fhTrackMapYZ(NULL),  // Only when creating normalization histos
    fhTrackMapAng(NULL), // Only when creating normalization histos
    fhTrackMapSph(NULL), // Only when creating normalization histos
    fhPointMapXY(NULL),
    fhPointMapXZ(NULL),
    fhPointMapYZ(NULL),
    fhPointMapAng(NULL),
    fhPointMapSph(NULL),
    fhDigiMapXY(NULL),
    fhDigiMapXZ(NULL),
    fhDigiMapYZ(NULL),
    fhDigiMapAng(NULL),
    fhDigiMapSph(NULL),
    fhHitMapXY(NULL),
    fhHitMapXZ(NULL),
    fhHitMapYZ(NULL),
    fhHitMapAng(NULL),
    fhHitMapSph(NULL),
    fhLeftRightDigiMatch(NULL),
    fhNbPointsInHit(NULL),
    fhNbTracksInHit(NULL),
    fhSinglePointHitDeltaX(NULL),
    fhSinglePointHitDeltaY(NULL),
    fhSinglePointHitDeltaZ(NULL),
    fhSinglePointHitDeltaR(NULL),
    fhSinglePointHitDeltaT(NULL),
    fhMultiPntHitClosestDeltaX(NULL),
    fhMultiPntHitClosestDeltaY(NULL),
    fhMultiPntHitClosestDeltaZ(NULL),
    fhMultiPntHitClosestDeltaR(NULL),
    fhMultiPntHitClosestDeltaT(NULL),
    fhMultiPntHitFurthestDeltaX(NULL),
    fhMultiPntHitFurthestDeltaY(NULL),
    fhMultiPntHitFurthestDeltaZ(NULL),
    fhMultiPntHitFurthestDeltaR(NULL),
    fhMultiPntHitFurthestDeltaT(NULL),
    fhMultiPntHitMeanDeltaX(NULL),
    fhMultiPntHitMeanDeltaY(NULL),
    fhMultiPntHitMeanDeltaZ(NULL),
    fhMultiPntHitMeanDeltaR(NULL),
    fhMultiPntHitMeanDeltaT(NULL),
    fhSingleTrackHitDeltaX(NULL),
    fhSingleTrackHitDeltaY(NULL),
    fhSingleTrackHitDeltaZ(NULL),
    fhSingleTrackHitDeltaR(NULL),
    fhSingleTrackHitDeltaT(NULL),
    fhMultiTrkHitClosestDeltaX(NULL),
    fhMultiTrkHitClosestDeltaY(NULL),
    fhMultiTrkHitClosestDeltaZ(NULL),
    fhMultiTrkHitClosestDeltaR(NULL),
    fhMultiTrkHitClosestDeltaT(NULL),
    fhMultiTrkHitFurthestDeltaX(NULL),
    fhMultiTrkHitFurthestDeltaY(NULL),
    fhMultiTrkHitFurthestDeltaZ(NULL),
    fhMultiTrkHitFurthestDeltaR(NULL),
    fhMultiTrkHitFurthestDeltaT(NULL),
    fhMultiTrkHitMeanDeltaX(NULL),
    fhMultiTrkHitMeanDeltaY(NULL),
    fhMultiTrkHitMeanDeltaZ(NULL),
    fhMultiTrkHitMeanDeltaR(NULL),
    fhMultiTrkHitMeanDeltaT(NULL)
{
  cout << "CbmTofHitFinderQa: Task started " << endl;
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
CbmTofHitFinderQa::CbmTofHitFinderQa(const char* name, Int_t verbose)
  : FairTask(name, verbose),
    fEvents(0),
    fMCEventHeader(NULL),
    fGeoHandler(new CbmTofGeoHandler()),
    fTofId(NULL),
    fChannelInfo(NULL),
    iNbSmTot(0),
    fvTypeSmOffs(),
    iNbRpcTot(0),
    fvSmRpcOffs(),
    iNbChTot(0),
    fvRpcChOffs(),
    fDigiPar(NULL),
    fDigiBdfPar(NULL),
    fTofPointsColl(NULL),
    fMcTracksColl(NULL),
    fTofDigisColl(NULL),
    fTofDigiMatchPointsColl(NULL),
    fTofHitsColl(NULL),
    fTofDigiMatchColl(NULL),
    fbNormHistGenMode( kFALSE ),
    fsHistoInNormCartFilename(""),  
    fsHistoInNormAngFilename(""),
    fsHistoInNormSphFilename(""),
    fsHistoOutFilename("./tofQa.hst.root"),
    fdWallPosZ(1000),
    fhTrackMapXY(NULL),  // Only when creating normalization histos
    fhTrackMapXZ(NULL),  // Only when creating normalization histos
    fhTrackMapYZ(NULL),  // Only when creating normalization histos
    fhTrackMapAng(NULL), // Only when creating normalization histos
    fhTrackMapSph(NULL), // Only when creating normalization histos
    fhPointMapXY(NULL),
    fhPointMapXZ(NULL),
    fhPointMapYZ(NULL),
    fhPointMapAng(NULL),
    fhPointMapSph(NULL),
    fhDigiMapXY(NULL),
    fhDigiMapXZ(NULL),
    fhDigiMapYZ(NULL),
    fhDigiMapAng(NULL),
    fhDigiMapSph(NULL),
    fhHitMapXY(NULL),
    fhHitMapXZ(NULL),
    fhHitMapYZ(NULL),
    fhHitMapAng(NULL),
    fhHitMapSph(NULL),
    fhLeftRightDigiMatch(NULL),
    fhNbPointsInHit(NULL),
    fhNbTracksInHit(NULL),
    fhSinglePointHitDeltaX(NULL),
    fhSinglePointHitDeltaY(NULL),
    fhSinglePointHitDeltaZ(NULL),
    fhSinglePointHitDeltaR(NULL),
    fhSinglePointHitDeltaT(NULL),
    fhMultiPntHitClosestDeltaX(NULL),
    fhMultiPntHitClosestDeltaY(NULL),
    fhMultiPntHitClosestDeltaZ(NULL),
    fhMultiPntHitClosestDeltaR(NULL),
    fhMultiPntHitClosestDeltaT(NULL),
    fhMultiPntHitFurthestDeltaX(NULL),
    fhMultiPntHitFurthestDeltaY(NULL),
    fhMultiPntHitFurthestDeltaZ(NULL),
    fhMultiPntHitFurthestDeltaR(NULL),
    fhMultiPntHitFurthestDeltaT(NULL),
    fhMultiPntHitMeanDeltaX(NULL),
    fhMultiPntHitMeanDeltaY(NULL),
    fhMultiPntHitMeanDeltaZ(NULL),
    fhMultiPntHitMeanDeltaR(NULL),
    fhMultiPntHitMeanDeltaT(NULL),
    fhSingleTrackHitDeltaX(NULL),
    fhSingleTrackHitDeltaY(NULL),
    fhSingleTrackHitDeltaZ(NULL),
    fhSingleTrackHitDeltaR(NULL),
    fhSingleTrackHitDeltaT(NULL),
    fhMultiTrkHitClosestDeltaX(NULL),
    fhMultiTrkHitClosestDeltaY(NULL),
    fhMultiTrkHitClosestDeltaZ(NULL),
    fhMultiTrkHitClosestDeltaR(NULL),
    fhMultiTrkHitClosestDeltaT(NULL),
    fhMultiTrkHitFurthestDeltaX(NULL),
    fhMultiTrkHitFurthestDeltaY(NULL),
    fhMultiTrkHitFurthestDeltaZ(NULL),
    fhMultiTrkHitFurthestDeltaR(NULL),
    fhMultiTrkHitFurthestDeltaT(NULL),
    fhMultiTrkHitMeanDeltaX(NULL),
    fhMultiTrkHitMeanDeltaY(NULL),
    fhMultiTrkHitMeanDeltaZ(NULL),
    fhMultiTrkHitMeanDeltaR(NULL),
    fhMultiTrkHitMeanDeltaT(NULL)
{
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
CbmTofHitFinderQa::~CbmTofHitFinderQa()
{
    // Destructor
}
// ------------------------------------------------------------------
/************************************************************************************/
// FairTasks inherited functions
InitStatus CbmTofHitFinderQa::Init()
{
   if( kFALSE == RegisterInputs() )
      return kFATAL;

   // Initialize the TOF GeoHandler
   Bool_t isSimulation=kFALSE;
   Int_t iGeoVersion = fGeoHandler->Init(isSimulation);
   LOG(INFO)<<"CbmTofSimpClusterizer::InitParameters with GeoVersion "<<iGeoVersion<<FairLogger::endl;

   if( k12b > iGeoVersion )
   {
      LOG(ERROR)<<"CbmTofSimpClusterizer::InitParameters => Only compatible with geometries after v12b !!!"
                <<FairLogger::endl;
      return kFATAL;
   } // if( k12b > iGeoVersion )
   
   if(NULL != fTofId) 
     LOG(INFO)<<"CbmTofSimpClusterizer::InitParameters with GeoVersion "<<fGeoHandler->GetGeoVersion()<<FairLogger::endl;
   else
   {
      switch(iGeoVersion)
      {
         case k12b: 
            fTofId = new CbmTofDetectorId_v12b();
            break;
         case k14a:
            fTofId = new CbmTofDetectorId_v14a();
            break;
         default:
            LOG(ERROR)<<"CbmTofSimpClusterizer::InitParameters => Invalid geometry!!!"<<iGeoVersion
                      <<FairLogger::endl;
         return kFATAL;
      } // switch(iGeoVersion)
   } // else of if(NULL != fTofId) 

   if( kFALSE == LoadGeometry() )
      return kFATAL;

   if( kFALSE == CreateHistos() )
      return kFATAL;

   return kSUCCESS;
}

void CbmTofHitFinderQa::SetParContainers()
{
   LOG(INFO)<<" CbmTofHitFinderQa => Get the digi parameters for tof"<<FairLogger::endl;

   // Get Base Container
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();

   fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));

   fDigiBdfPar = (CbmTofDigiBdfPar*) (rtdb->getContainer("CbmTofDigiBdfPar"));
}

void CbmTofHitFinderQa::Exec(Option_t * option)
{
   // Task execution

   LOG(DEBUG)<<" CbmTofHitFinderQa => New event"<<FairLogger::endl;

   FillHistos();

   if( 0 == ( fEvents%100 ) && 0 < fEvents )
   {
      cout << "-I- CbmTofHitFinderQa::Exec : "
           << "event " << fEvents << " processed." << endl;
   }
   fEvents += 1;
}

void CbmTofHitFinderQa::Finish()
{
   // Normalisations
   cout << "CbmTofHitFinderQa::Finish up with " << fEvents << " analyzed events " << endl;

   if( kFALSE == fbNormHistGenMode )
      NormalizeMapHistos();
      else NormalizeNormHistos();

   WriteHistos();
   // Prevent them from being sucked in by the CbmHadronAnalysis WriteHistograms method
   DeleteHistos();
}

/************************************************************************************/
// Functions common for all clusters approximations
Bool_t   CbmTofHitFinderQa::RegisterInputs()
{
   FairRootManager *fManager = FairRootManager::Instance();
   
   fTofPointsColl  = (TClonesArray *) fManager->GetObject("TofPoint");
   if( NULL == fTofPointsColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofPoint TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofPointsColl)

   fMcTracksColl   = (TClonesArray *) fManager->GetObject("MCTrack");
   if( NULL == fMcTracksColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the MCTrack TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fMcTracksColl)

   fTofDigisColl   = (TClonesArray *) fManager->GetObject("TofDigi");
   if( NULL == fTofDigisColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofDigi TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigisColl)
   fTofDigiMatchPointsColl   = (TClonesArray *) fManager->GetObject("TofDigiMatchPoints");
   if( NULL == fTofDigiMatchPointsColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofDigiMatchPoints TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigiMatchPointsColl)

   fTofHitsColl   = (TClonesArray *) fManager->GetObject("TofHit");
   if( NULL == fTofHitsColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofHit TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofHitsColl)
   fTofDigiMatchColl   = (TClonesArray *) fManager->GetObject("TofDigiMatch");
   if( NULL == fTofDigiMatchColl)
   {
      LOG(ERROR)<<"CbmTofHitFinderQa::RegisterInputs => Could not get the TofDigiMatch TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigiMatchColl)

   return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofHitFinderQa::LoadGeometry()
{
   /*
     Type 0: 5 RPC/SM,  24 SM, 32 ch/RPC =>  3840 ch          , 120 RPC         ,
     Type 1: 5 RPC/SM, 142 SM, 32 ch/RPC => 22720 ch => 26560 , 710 RPC =>  830 , => 166
     Type 3: 3 RPC/SM,  50 SM, 56 ch/RPC =>  8400 ch => 34960 , 150 RPC =>  980 , => 216
     Type 4: 5 RPC/SM,   8 SM, 96 ch/RPC =>  3840 ch => 38800 ,  40 RPC => 1020 , => 224
     Type 5: 5 RPC/SM,   8 SM, 96 ch/RPC =>  3840 ch => 42640 ,  40 RPC => 1060 , => 232
     Type 6: 2 RPC/SM,  10 SM, 96 ch/RPC =>  1920 ch => 44560 ,  20 RPC => 1080 , => 242
   */

   // Count the total number of channels and
   // generate an array with the global channel index of the first channe in each RPC
   Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();
   fvTypeSmOffs.resize( iNbSmTypes );
   fvSmRpcOffs.resize( iNbSmTypes );
   fvRpcChOffs.resize( iNbSmTypes );
   iNbSmTot  = 0;
   iNbRpcTot = 0;
   iNbChTot  = 0;
   for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
   {
      Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
      Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);

      fvTypeSmOffs[iSmType] = iNbSmTot;
      iNbSmTot += iNbSm;

      fvSmRpcOffs[iSmType].resize( iNbSm );
      fvRpcChOffs[iSmType].resize( iNbSm );

      for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
      {
         fvSmRpcOffs[iSmType][iSm] = iNbRpcTot;
         iNbRpcTot += iNbRpc;

         fvRpcChOffs[iSmType][iSm].resize( iNbRpc );
         for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
         {
            fvRpcChOffs[iSmType][iSm][iRpc] = iNbChTot;
            iNbChTot += fDigiBdfPar->GetNbChan( iSmType, iRpc );
         } // for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
      } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
   } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )

   return kTRUE;
}
/************************************************************************************/
// ------------------------------------------------------------------
Bool_t CbmTofHitFinderQa::SetWallPosZ( Double_t dWallPosCm)
{
   fdWallPosZ = dWallPosCm;
   LOG(INFO)<<"CbmTofHitFinderQa::SetWallPosZ => Change histograms center on Z axis to "<<dWallPosCm<<" cm"<<FairLogger::endl;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::CreateHistos()
{
  // Create histogramms

   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !

   Double_t ymin=-1.;
   Double_t ymax=4.;
   Double_t ptmmax=2.5;
   Int_t    ptm_nbx=30;
   Int_t    ptm_nby=30;

   Double_t v1_nbx=20.;
   Double_t v1_nby=20.;
   Double_t yvmax=1.3;

   // xy - hit densities and rates
   Int_t nbinx=1500;
   Int_t nbiny=1000;
   Int_t nbinz=1500;
   Double_t xrange=750.;
   Double_t yrange=500.;
   Double_t zmin  = fdWallPosZ -  50.;
   Double_t zmax  = fdWallPosZ + 100.;

   // angular densities for overlap check
   Int_t iNbBinThetaX  = 1200;
   Double_t dThetaXMin = -  60.0;
   Double_t dThetaXMax =    60.0;
   Int_t iNbBinThetaY  = 800;
   Double_t dThetaYMin = -  40.0;
   Double_t dThetaYMax =    40.0;

   Int_t iNbBinTheta   = 180;
   Double_t dThetaMin  =   0;
   Double_t dThetaMax  =  TMath::Pi()*90/180;
   Int_t iNbBinPhi     = 180;
   Double_t dPhiMin    = - TMath::Pi();
   Double_t dPhiMax    =   TMath::Pi();
   
   // Range hit deviation from MC
   Int_t    iNbBinsDeltaPos  =  600;
   Double_t dDeltaPosRange   =   30; // cm
   Int_t    iNbBinsDeltaTime = 1000;
   Double_t dDeltaTimeRange  = 1000; // ps 

   // Mapping
      // tracks: Only when creating normalization histos
   if( kTRUE == fbNormHistGenMode )
   {
      fhTrackMapXY     = new TH2D("TofTests_TracksMapXY",  "Position of the MC Tracks assuming along Z axis; X[cm]; Y[cm]; # [Tracks]",
                                 nbinx,-xrange,xrange,nbiny,-yrange,yrange);
      fhTrackMapXZ     = new TH2D("TofTests_TracksMapXZ",  "Position of the MC Tracks assuming along Z axis; X[cm]; Z[cm]; # [Tracks]",
                                 nbinx,-xrange,xrange,nbinz,zmin,zmax);
      fhTrackMapYZ     = new TH2D("TofTests_TracksMapYZ",  "Position of the MC Tracks assuming along Z axis; Y[cm]; Z[cm]; # [Tracks]",
                                 nbiny,-yrange,yrange,nbinz,zmin,zmax);
      fhTrackMapAng    = new TH2D("TofTests_TracksMapAng",  "Position of the MC Tracks assuming from origin; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Tracks]",
                                 iNbBinThetaX, dThetaXMin, dThetaXMax,
                                 iNbBinThetaY, dThetaYMin, dThetaYMax);
      fhTrackMapSph    = new TH2D("TofTests_TracksMapSph",  "Position of the MC Tracks assuming from origin; #theta[rad.]; #phi[rad.]; # [Tracks]",
                                 iNbBinTheta, dThetaMin, dThetaMax,
                                 iNbBinPhi,   dPhiMin,   dPhiMax);
   } // if( kTRUE == fbNormHistGenMode )
      // points
   fhPointMapXY     = new TH2D("TofTests_PointsMapXY",  "Position of the Tof Points; X[cm]; Y[cm]; # [Points]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhPointMapXZ     = new TH2D("TofTests_PointsMapXZ",  "Position of the Tof Points; X[cm]; Z[cm]; # [Points]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhPointMapYZ     = new TH2D("TofTests_PointsMapYZ",  "Position of the Tof Points; Y[cm]; Z[cm]; # [Points]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhPointMapAng    = new TH2D("TofTests_PointsMapAng",  "Position of the Tof Points; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Points]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhPointMapSph    = new TH2D("TofTests_PointsMapSph",  "Position of the Tof Points; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);
      // Digis
   fhDigiMapXY     = new TH2D("TofTests_DigisMapXY",  "Position of the Tof Digis; X[cm]; Y[cm]; # [Digi]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhDigiMapXZ     = new TH2D("TofTests_DigisMapXZ",  "Position of the Tof Digis; X[cm]; Z[cm]; # [Digi]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhDigiMapYZ     = new TH2D("TofTests_DigisMapYZ",  "Position of the Tof Digis; Y[cm]; Z[cm]; # [Digi]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhDigiMapAng    = new TH2D("TofTests_DigisMapAng",  "Position of the Tof Digis; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Digi]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhDigiMapSph    = new TH2D("TofTests_DigisMapSph",  "Position of the Tof Digis; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);

      // Hits
   fhHitMapXY      = new TH2D("TofTests_HitsMapXY",   "Position of the Tof Hits; X[cm]; Y[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbiny,-yrange,yrange);
   fhHitMapXZ      = new TH2D("TofTests_HitsMapXZ",   "Position of the Tof Hits; X[cm]; Z[cm]; # [Hits]",
                              nbinx,-xrange,xrange,nbinz,zmin,zmax);
   fhHitMapYZ      = new TH2D("TofTests_HitsMapYZ",   "Position of the Tof Hits; Y[cm]; Z[cm]; # [Hits]",
                              nbiny,-yrange,yrange,nbinz,zmin,zmax);
   fhHitMapAng     = new TH2D("TofTests_HitsMapAng",  "Position of the Tof Hits; #theta_{x}[Deg.]; #theta_{y}[Deg.]; # [Hits]",
                              iNbBinThetaX, dThetaXMin, dThetaXMax,
                              iNbBinThetaY, dThetaYMin, dThetaYMax);
   fhHitMapSph     = new TH2D("TofTests_HitsMapSph",  "Position of the Tof Hits; #theta[rad.]; #phi[rad.]; # [Points]",
                              iNbBinTheta, dThetaMin, dThetaMax,
                              iNbBinPhi,   dPhiMin,   dPhiMax);

   // L/R digis missmatch
   fhLeftRightDigiMatch = new TH2D("TofTests_DigiMatchLR","Matching between the left an right digi for strips added to hit; Global Strip Index []",
                              iNbChTot, 0, iNbChTot,
                              3, -0.5, 2.5);
   fhLeftRightDigiMatch->GetYaxis()->SetBinLabel( 1 , "Same MC Point");
   fhLeftRightDigiMatch->GetYaxis()->SetBinLabel( 2 , "Same MC Track");
   fhLeftRightDigiMatch->GetYaxis()->SetBinLabel( 3 , "Diff MC Track");
   
   // Nb different MC Points and Tracks in Hit
   fhNbPointsInHit = new TH1D("TofTests_NbPointsInHit", "Number of different MC Points in Hit; # [MC Points]",
                              100, -0.5, 99.5 );
   fhNbTracksInHit = new TH1D("TofTests_NbTracksInHit", "Number of different MC Tracks in Hit; # [MC Tracks]",
                              100, -0.5, 99.5 );
                              
   // Hit Quality for Hits coming from a single MC Point
   fhSinglePointHitDeltaX = new TH1D("TofTests_SinglePointHitDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit coming from a single MC Point; X(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaY = new TH1D("TofTests_SinglePointHitDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit coming from a single MC Point; Y(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaZ = new TH1D("TofTests_SinglePointHitDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit coming from a single MC Point; Z(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaR = new TH1D("TofTests_SinglePointHitDeltaR", 
                              "Quality of the Tof Hits position, for hit coming from a single MC Point; R(Hit -> Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSinglePointHitDeltaT = new TH1D("TofTests_SinglePointHitDeltaT", 
                              "Quality of the Tof Hits Time, for hit coming from a single MC Point; T(hit) - T(Point) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);

   // Hit Quality for Hits coming from a multiple MC Points
      // To Point closest to Hit
   fhMultiPntHitClosestDeltaX  = new TH1D("TofTests_MultiPntHitClosestDeltaX", 
                              "Quality of the Tof Hits position on X axis relative to closest Point, for hit coming from multiple MC Point; X(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitClosestDeltaY  = new TH1D("TofTests_MultiPntHitClosestDeltaY", 
                              "Quality of the Tof Hits position on Y axis relative to closest Point, for hit coming from multiple MC Point; Y(Hit) - Y(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitClosestDeltaZ  = new TH1D("TofTests_MultiPntHitClosestDeltaZ", 
                              "Quality of the Tof Hits position on Z axis relative to closest Point, for hit coming from multiple MC Point; Z(Hit) - Z(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitClosestDeltaR  = new TH1D("TofTests_MultiPntHitClosestDeltaR", 
                              "Quality of the Tof Hits position relative to closest Point, for hit coming from multiple MC Point; R(Hit -> Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitClosestDeltaT= new TH1D("TofTests_MultiPntHitClosestDeltaT", 
                              "Quality of the Tof Hits Time relative to closest Point, for hit coming from multiple MC Point; T(hit) - T(Point) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
      // To Point furthest from Hit
   fhMultiPntHitFurthestDeltaX  = new TH1D("TofTests_MultiPntHitFurthestDeltaX", 
                              "Quality of the Tof Hits position on X axis relative to furthest Point, for hit coming from multiple MC Point; X(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitFurthestDeltaY  = new TH1D("TofTests_MultiPntHitFurthestDeltaY", 
                              "Quality of the Tof Hits position on Y axis relative to furthest Point, for hit coming from multiple MC Point; Y(Hit) - Y(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitFurthestDeltaZ  = new TH1D("TofTests_MultiPntHitFurthestDeltaZ", 
                              "Quality of the Tof Hits position on Z axis relative to furthest Point, for hit coming from multiple MC Point; Z(Hit) - Z(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitFurthestDeltaR  = new TH1D("TofTests_MultiPntHitFurthestDeltaR", 
                              "Quality of the Tof Hits position relative to furthest Point, for hit coming from multiple MC Point; R(Hit -> Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitFurthestDeltaT= new TH1D("TofTests_MultiPntHitFurthestDeltaT", 
                              "Quality of the Tof Hits Time relative to furthest Point, for hit coming from multiple MC Point; T(hit) - T(Point) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
      // To mean Point position
   fhMultiPntHitMeanDeltaX  = new TH1D("TofTests_MultiPntHitMeanDeltaX", 
                              "Quality of the Tof Hits position on X axis relative to mean Point position, for hit coming from multiple MC Point; X(Hit) - X(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitMeanDeltaY  = new TH1D("TofTests_MultiPntHitMeanDeltaY", 
                              "Quality of the Tof Hits position on Y axis relative to mean Point position, for hit coming from multiple MC Point; Y(Hit) - Y(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitMeanDeltaZ  = new TH1D("TofTests_MultiPntHitMeanDeltaZ", 
                              "Quality of the Tof Hits position on Z axis relative to mean Point position, for hit coming from multiple MC Point; Z(Hit) - Z(Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitMeanDeltaR  = new TH1D("TofTests_MultiPntHitMeanDeltaR", 
                              "Quality of the Tof Hits position relative to mean Point position, for hit coming from multiple MC Point; R(Hit -> Point) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiPntHitMeanDeltaT= new TH1D("TofTests_MultiPntHitMeanDeltaT", 
                              "Quality of the Tof Hits Time relative to mean Point time, for hit coming from multiple MC Point; T(hit) - T(Point) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
                              
   // Hit Quality for Hits coming from a single MC Point
   fhSingleTrackHitDeltaX = new TH1D("TofTests_SingleTrackHitDeltaX", 
                              "Quality of the Tof Hits position on X axis, for hit coming from a single MC Track; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitDeltaY = new TH1D("TofTests_SingleTrackHitDeltaY", 
                              "Quality of the Tof Hits position on Y axis, for hit coming from a single MC Track; Y(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitDeltaZ = new TH1D("TofTests_SingleTrackHitDeltaZ", 
                              "Quality of the Tof Hits position on Z axis, for hit coming from a single MC Track; Z(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitDeltaR = new TH1D("TofTests_SingleTrackHitDeltaR", 
                              "Quality of the Tof Hits position, for hit coming from a single MC Track; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhSingleTrackHitDeltaT = new TH1D("TofTests_SingleTrackHitDeltaT", 
                              "Quality of the Tof Hits Time, for hit coming from a single MC Track; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);

   // Hit Quality for Hits coming from a multiple MC Tracks
      // To Point closest to Hit
   fhMultiTrkHitClosestDeltaX  = new TH1D("TofTests_MultiTrkHitClosestDeltaX", 
                              "Quality of the Tof Hits position on X axis relative to closest Track, for hit coming from multiple MC Track; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitClosestDeltaY  = new TH1D("TofTests_MultiTrkHitClosestDeltaY", 
                              "Quality of the Tof Hits position on Y axis relative to closest Track, for hit coming from multiple MC Track; Y(Hit) - Y(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitClosestDeltaZ  = new TH1D("TofTests_MultiTrkHitClosestDeltaZ", 
                              "Quality of the Tof Hits position on Z axis relative to closest Track, for hit coming from multiple MC Track; Z(Hit) - Z(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitClosestDeltaR  = new TH1D("TofTests_MultiTrkHitClosestDeltaR", 
                              "Quality of the Tof Hits position relative to closest Track, for hit coming from multiple MC Track; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitClosestDeltaT= new TH1D("TofTests_MultiTrkHitClosestDeltaT", 
                              "Quality of the Tof Hits Time relative to closest Track, for hit coming from multiple MC Track; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
      // To Track furthest from Hit
   fhMultiTrkHitFurthestDeltaX  = new TH1D("TofTests_MultiTrkHitFurthestDeltaX", 
                              "Quality of the Tof Hits position on X axis relative to furthest Track, for hit coming from multiple MC Track; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitFurthestDeltaY  = new TH1D("TofTests_MultiTrkHitFurthestDeltaY", 
                              "Quality of the Tof Hits position on Y axis relative to furthest Track, for hit coming from multiple MC Track; Y(Hit) - Y(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitFurthestDeltaZ  = new TH1D("TofTests_MultiTrkHitFurthestDeltaZ", 
                              "Quality of the Tof Hits position on Z axis relative to furthest Track, for hit coming from multiple MC Track; Z(Hit) - Z(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitFurthestDeltaR  = new TH1D("TofTests_MultiTrkHitFurthestDeltaR", 
                              "Quality of the Tof Hits position relative to furthest Track, for hit coming from multiple MC Track; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitFurthestDeltaT= new TH1D("TofTests_MultiTrkHitFurthestDeltaT", 
                              "Quality of the Tof Hits Time relative to furthest Track, for hit coming from multiple MC Track; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
      // To mean Track position
   fhMultiTrkHitMeanDeltaX  = new TH1D("TofTests_MultiTrkHitMeanDeltaX", 
                              "Quality of the Tof Hits position on X axis relative to mean Track position, for hit coming from multiple MC Track; X(Hit) - X(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitMeanDeltaY  = new TH1D("TofTests_MultiTrkHitMeanDeltaY", 
                              "Quality of the Tof Hits position on Y axis relative to mean Track position, for hit coming from multiple MC Track; Y(Hit) - Y(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitMeanDeltaZ  = new TH1D("TofTests_MultiTrkHitMeanDeltaZ", 
                              "Quality of the Tof Hits position on Z axis relative to mean Track position, for hit coming from multiple MC Track; Z(Hit) - Z(Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitMeanDeltaR  = new TH1D("TofTests_MultiTrkHitMeanDeltaR", 
                              "Quality of the Tof Hits position relative to mean Track position, for hit coming from multiple MC Track; R(Hit -> Track) [cm]; # [Hits]",
                              iNbBinsDeltaPos, -dDeltaPosRange, dDeltaPosRange);
   fhMultiTrkHitMeanDeltaT= new TH1D("TofTests_MultiTrkHitMeanDeltaT", 
                              "Quality of the Tof Hits Time relative to mean Track time, for hit coming from multiple MC Track; T(hit) - T(Track) [ps]; # [Hits]",
                              iNbBinsDeltaTime, -dDeltaTimeRange, dDeltaTimeRange);
   
   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!

   return kTRUE;
}

// ------------------------------------------------------------------
Bool_t CbmTofHitFinderQa::FillHistos()
{
   // Declare variables outside the loop
   CbmMCTrack  * pMcTrk;
   CbmTofPoint * pTofPoint;
   CbmMatch    * pMatchDigiPnt;
   CbmMatch    * pMatchDigiPntB;
   CbmTofHit   * pTofHit;
   CbmMatch    * pMatchHitDigi;

   Int_t iNbTracks, iNbTofPts, iNbTofDigis, 
         iNbTofDigisMatch, iNbTofHits, iNbTofHitsMatch;

   iNbTracks        = fMcTracksColl->GetEntriesFast();
   iNbTofPts        = fTofPointsColl->GetEntriesFast();
   iNbTofDigis      = fTofDigisColl->GetEntriesFast();
   iNbTofDigisMatch = fTofDigiMatchPointsColl->GetEntriesFast();
   iNbTofHits       = fTofHitsColl->GetEntriesFast();
   iNbTofHitsMatch  = fTofDigiMatchColl->GetEntriesFast();

   if( iNbTofDigis != iNbTofDigisMatch )
      LOG(FATAL)<<"CbmTofHitFinderQa::FillHistos => Nb entries in TofDigiMatchPoints TClonesArray doe not match nb entries in TofDigi!!!"<<FairLogger::endl;
   if( iNbTofHits != iNbTofHitsMatch )
      LOG(FATAL)<<"CbmTofHitFinderQa::FillHistos => Nb entries in TofDigiMatch TClonesArray doe not match nb entries in TofHit!!! "
                <<iNbTofHits<<" VS "<<iNbTofHitsMatch
                <<" (Prev step: "<<iNbTofDigis<<" VS "<<iNbTofDigisMatch<<" )"<<FairLogger::endl;      
   
   // Tracks Info
   Int_t iNbTofTracks     = 0;
   Int_t iNbTofTracksPrim = 0;
   for(Int_t iTrkInd = 0; iTrkInd < iNbTracks; iTrkInd++)
   {
      pMcTrk = (CbmMCTrack*) fMcTracksColl->At( iTrkInd );

      if( 0 < pMcTrk->GetNPoints(kTOF) )
      {
         iNbTofTracks++;
      }
      if( 0 < pMcTrk->GetNPoints(kTOF) && -1 == pMcTrk->GetMotherId() )
         iNbTofTracksPrim++;
         
      // tracks mapping: Only when creating normalization histos
      // Assume only TOF in setup, no field (only straight tracks)
      // and all tracks reach TOF (protons)
      if( kTRUE == fbNormHistGenMode )
      {
         // XYZ mapping: assume tracks along Z axis
         if( pMcTrk->GetPz() == pMcTrk->GetP() )
         {
            fhTrackMapXY->Fill( pMcTrk->GetStartX(), pMcTrk->GetStartY() );
            fhTrackMapXZ->Fill( pMcTrk->GetStartX(), fdWallPosZ ); // Not sure how to get Z here
            fhTrackMapYZ->Fill( pMcTrk->GetStartY(), fdWallPosZ ); // Not sure how to get Z here
         } // if( pMcTrk->GetPz() == pMcTrk->GetP() )
         
         // Angular mapping: assume tracks all coming from origin and not necess. along Z axis
         if( 0 != pMcTrk->GetPz() && ( (0 != pMcTrk->GetPx() ) || (0 != pMcTrk->GetPy() ) ))
            fhTrackMapAng->Fill( TMath::ATan2( pMcTrk->GetPx(), pMcTrk->GetPz() )*180.0/TMath::Pi(),
                                 TMath::ATan2( pMcTrk->GetPy(), pMcTrk->GetPz() )*180.0/TMath::Pi() );
                                 
         // Spherical mapping: assume tracks all coming from origin and not necess. along Z axis
         if( 0 != pMcTrk->GetPz() && 0 != pMcTrk->GetPx() )
            fhTrackMapSph->Fill( TMath::ATan2( pMcTrk->GetPt(), pMcTrk->GetPz() ),
                                 TMath::ATan2( pMcTrk->GetPy(), pMcTrk->GetPx() ) );
      } // if( kTRUE == fbNormHistGenMode )
   } // for(Int_t iTrkInd = 0; iTrkInd < nMcTracks; iTrkInd++)
   
   // Loop over Points and map them?
   for (Int_t iPntInd = 0; iPntInd < iNbTofPts; iPntInd++ )
   {
      // Get a pointer to the TOF point
      pTofPoint = (CbmTofPoint*) fTofPointsColl->At( iPntInd );

      // Obtain position
      TVector3 vPntPos;
      pTofPoint->Position( vPntPos );

      Double_t dX = vPntPos.X();
      Double_t dY = vPntPos.Y();
      Double_t dZ = vPntPos.Z();

      fhPointMapXY->Fill( dX, dY );
      fhPointMapXZ->Fill( dX, dZ );
      fhPointMapYZ->Fill( dY, dZ );

      Double_t dThetaX = TMath::ATan2( dX, dZ )*180.0/TMath::Pi();
      Double_t dThetaY = TMath::ATan2( dY, dZ )*180.0/TMath::Pi();
      fhPointMapAng->Fill( dThetaX, dThetaY );

      Double_t dTheta  = TMath::ATan2( TMath::Sqrt( dX*dX + dY*dY ), dZ );//*180.0/TMath::Pi();
      Double_t dPhi    = TMath::ATan2( dY, dX );//*180.0/TMath::Pi();
      fhPointMapSph->Fill( dTheta, dPhi );
   } // for (Int_t iPntInd = 0; iPntInd < nTofPoint; iPntInd++ )
 
   // Loop over Digis and map them?
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      CbmTofDigiExp *pTofDigi;
      for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
      {
         pTofDigi = (CbmTofDigiExp*) fTofDigisColl->At( iDigInd );

         Int_t iSmType = pTofDigi->GetType();
         Int_t iSm     = pTofDigi->GetSm();
         Int_t iRpc    = pTofDigi->GetRpc();
         Int_t iCh     = pTofDigi->GetChannel();
         // First Get X/Y position info
         CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh + 1);
         Int_t iChId =  fTofId->SetDetectorInfo( xDetInfo );
         fChannelInfo = fDigiPar->GetCell( iChId );

         Double_t dX = fChannelInfo->GetX();
         Double_t dY = fChannelInfo->GetY();
         Double_t dZ = fChannelInfo->GetZ();

         fhDigiMapXY->Fill(   dX, dY );
         fhDigiMapXZ->Fill(   dX, dZ );
         fhDigiMapYZ->Fill(   dY, dZ );

         Double_t dThetaX = TMath::ATan2( dX, dZ )*180.0/TMath::Pi();
         Double_t dThetaY = TMath::ATan2( dY, dZ )*180.0/TMath::Pi();
         fhDigiMapAng->Fill( dThetaX, dThetaY );

         Double_t dTheta  = TMath::ATan2( TMath::Sqrt( dX*dX + dY*dY ), dZ );//*180.0/TMath::Pi();
         Double_t dPhi    = TMath::ATan2( dY, dX );//*180.0/TMath::Pi();
         fhDigiMapSph->Fill( dTheta, dPhi );
      } // for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      else
      {
         CbmTofDigi *pTofDigi;
         for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
         {
            pTofDigi = (CbmTofDigi*) fTofDigisColl->At( iDigInd );

            Int_t iSmType = pTofDigi->GetType();
            Int_t iSm     = pTofDigi->GetSm();
            Int_t iRpc    = pTofDigi->GetRpc();
            Int_t iCh     = pTofDigi->GetChannel();
            // First Get X/Y position info
            CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh + 1);
            Int_t iChId = fTofId->SetDetectorInfo( xDetInfo );
            fChannelInfo = fDigiPar->GetCell( iChId );

            Double_t dX = fChannelInfo->GetX();
            Double_t dY = fChannelInfo->GetY();
            Double_t dZ = fChannelInfo->GetZ();

            fhDigiMapXY->Fill(   dX, dY );
            fhDigiMapXZ->Fill(   dX, dZ );
            fhDigiMapYZ->Fill(   dY, dZ );

            Double_t dThetaX = TMath::ATan2( dX, dZ )*180.0/TMath::Pi();
            Double_t dThetaY = TMath::ATan2( dY, dZ )*180.0/TMath::Pi();
            fhDigiMapAng->Fill( dThetaX, dThetaY );

            Double_t dTheta  = TMath::ATan2( TMath::Sqrt( dX*dX + dY*dY ), dZ );//*180.0/TMath::Pi();
            Double_t dPhi    = TMath::ATan2( dY, dX );//*180.0/TMath::Pi();
            fhDigiMapSph->Fill( dTheta, dPhi );
         } // for( Int_t iDigInd = 0; iDigInd < iNbTofDigis; iDigInd++ )
      } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
     
   // Loop Over Hits
   for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++ )
   {
      Int_t iNbTofPoint = 0;
      Int_t iNbTofTrack = 0;
      std::vector<Int_t> vTofPointsId;
      std::vector<Int_t> vTofTracksId;
      Double_t dPntMeanPosX = 0;
      Double_t dPntMeanPosY = 0;
      Double_t dPntMeanPosZ = 0;
      Double_t dPntMeanTime = 0;
      Double_t dTrkMeanPosX = 0;
      Double_t dTrkMeanPosY = 0;
      Double_t dTrkMeanPosZ = 0;
      Double_t dTrkMeanTime = 0;
      Int_t    iClosestPntIdx  = -1;
      Double_t dClosestPntDr   = 1e18;
      Int_t    iFurthestPntIdx = -1;
      Double_t dFurthestPntDr  = -1;
      Int_t    iClosestTrkIdx  = -1;
      Double_t dClosestTrkDr   = 1e18;
      Int_t    iFurthestTrkIdx = -1;
      Double_t dFurthestTrkDr  = -1;
         
      pTofHit       = (CbmTofHit*) fTofHitsColl->At( iHitInd );
      pMatchHitDigi = (CbmMatch*) fTofDigiMatchColl->At( iHitInd );
      
      Double_t dX = pTofHit->GetX();
      Double_t dY = pTofHit->GetY();
      Double_t dZ = pTofHit->GetZ();

      fhHitMapXY->Fill(   dX, dY );
      fhHitMapXZ->Fill(   dX, dZ );
      fhHitMapYZ->Fill(   dY, dZ );

      Double_t dThetaX = TMath::ATan2( dX, dZ )*180.0/TMath::Pi();
      Double_t dThetaY = TMath::ATan2( dY, dZ )*180.0/TMath::Pi();
      fhHitMapAng->Fill( dThetaX, dThetaY );

      Double_t dTheta  = TMath::ATan2( TMath::Sqrt( dX*dX + dY*dY ), dZ );//*180.0/TMath::Pi();
      Double_t dPhi    = TMath::ATan2( dY, dX );//*180.0/TMath::Pi();
      fhHitMapSph->Fill( dTheta, dPhi );
      
      Int_t iNbDigisHit = pMatchHitDigi->GetNofLinks();
      if( 0 != iNbDigisHit%2 )
         LOG(FATAL)<<"CbmTofHitFinderQa::FillHistos => Nb of digis matching Hit #"
                   <<iHitInd<<" in event #"<<fEvents
                   <<" is not a multiple of 2 => should not happen as both ends of strp required!!!"<<FairLogger::endl;
         
      // Loop over Digis inside Hit
      if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      {
         CbmTofDigiExp *pTofDigi;
         CbmTofDigiExp *pTofDigiB;
         for( Int_t iDigi = 0; iDigi < iNbDigisHit; iDigi++)
         {
            CbmLink lDigi    = pMatchHitDigi->GetLink(iDigi); 
            Int_t   iDigiIdx = lDigi.GetIndex();
            
            if( iNbTofDigis <= iDigiIdx )
            {
               LOG(ERROR)<<"CbmTofHitFinderQa::FillHistos => Digi index from Hit #"
                   <<iHitInd<<" in event #"<<fEvents
                   <<" is bigger than nb entries in Digis arrays => ignore it!!!"<<FairLogger::endl;
               continue;
            } // if( iNbTofDigis <= iDigiIdx )
            
            pTofDigi      = (CbmTofDigiExp*) fTofDigisColl->At( iDigiIdx );
            pMatchDigiPnt = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx );

            CbmLink lPt    = pMatchDigiPnt->GetLink(0); 
            Int_t   iPtIdx = lPt.GetIndex();
            Int_t   iTrkId = ((CbmTofPoint*) fTofPointsColl->At(iPtIdx))->GetTrackID();
               
            Int_t iSmType = pTofDigi->GetType();
            Int_t iSm     = pTofDigi->GetSm();
            Int_t iRpc    = pTofDigi->GetRpc();
            Int_t iCh     = pTofDigi->GetChannel();
            Int_t iGlobalChan = iCh  + fvRpcChOffs[iSmType][iSm][iRpc];
            
            // Check Left-Right MC missmatch (digis match always stored by pairs)
            if( 0 == iDigi%2 )
            {
               // Get Info about the other end of the strip
               pMatchDigiPntB = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx + 1 );

               CbmLink lPtB    = pMatchDigiPntB->GetLink(0); 
               Int_t   iPtIdxB = lPtB.GetIndex();
               
               // Check Left-Right missmatch for MC Point
               if( iPtIdx != iPtIdxB )
               {
                  // Check Left-Right missmatch for MC Track
                  if( iTrkId != ((CbmTofPoint*) fTofPointsColl->At(iPtIdxB))->GetTrackID() )
                  {
                     fhLeftRightDigiMatch->Fill(iGlobalChan, 2);
                  } // if( iTrkId != ((CbmTofPoint*) fTofPointsColl->At(iPtIdxB))->GetTrackID() )
                     else fhLeftRightDigiMatch->Fill(iGlobalChan, 1);
               } // if( iPtIdx != iPtIdxB )
                  else fhLeftRightDigiMatch->Fill(iGlobalChan, 0);

            } // if( 0 == iDigi%2 )
            
            // Count Nb different MC Points in Hit
            Bool_t bPointFound = kFALSE;
            for( Int_t iPrevPtIdx = 0; iPrevPtIdx < vTofPointsId.size(); iPrevPtIdx++)
               if( iPtIdx == vTofPointsId[iPrevPtIdx] )
               {
                  bPointFound = kTRUE;
                  break;
               } // if( iPtIdx == vTofPointsId[iPrevPtIdx] )
            if( kFALSE == bPointFound )
            {
               vTofPointsId.push_back(iPtIdx);
               
               // Obtain Point position
               pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iPtIdx);
               TVector3 vPntPos;
               pTofPoint->Position( vPntPos );
         
               // Compute mean MC Points position (X, Y, Z, T)
               dPntMeanPosX += vPntPos.X();
               dPntMeanPosY += vPntPos.Y();
               dPntMeanPosZ += vPntPos.Z();
               dPntMeanTime += pTofPoint->GetTime();
               
               // Check if closest MC Point to Hit position
               Double_t dPntDeltaR = TMath::Sqrt(
                     (dX - vPntPos.X())*(dX - vPntPos.X()) 
                   + (dY - vPntPos.Y())*(dY - vPntPos.Y()) 
                   + (dZ - vPntPos.Z())*(dZ - vPntPos.Z()) );
               if( dPntDeltaR < dClosestPntDr )
               {
                  iClosestPntIdx  = iPtIdx;
                  dClosestPntDr   = dPntDeltaR;
               } // if( dPntDeltaR < dClosestPntDr )
               // Check if furthest MC Point to Hit position
               if( dFurthestPntDr < dPntDeltaR )
               {
                  iFurthestPntIdx  = iPtIdx;
                  dFurthestPntDr   = dPntDeltaR;
               } // if( dFurthestPntDr < dPntDeltaR )
            } // if( kFALSE == bPointFound )
               
            // Count Nb different MC Tracks in Hit
            Bool_t bTrackFound = kFALSE;
            for( Int_t iPrevTrkIdx = 0; iPrevTrkIdx < vTofTracksId.size(); iPrevTrkIdx++)
               if( iTrkId == vTofTracksId[iPrevTrkIdx] )
               {
                  bTrackFound = kTRUE;
                  break;
               } // if( iTrkId == vTofPointsId[iPrevTrkIdx] )
            if( kFALSE == bTrackFound )
            {
               vTofTracksId.push_back(iTrkId);
               
               // Obtain Point position (Consider 1st Pnt of each Trk is approximate coord)
               pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iPtIdx);
               TVector3 vPntPos;
               pTofPoint->Position( vPntPos );
         
               // Compute mean MC Tracks position (X, Y, Z, T)
               dTrkMeanPosX += vPntPos.X();
               dTrkMeanPosY += vPntPos.Y();
               dTrkMeanPosZ += vPntPos.Z();
               dTrkMeanTime += pTofPoint->GetTime();
               
               // Check if closest MC track to Hit position
               Double_t dTrkDeltaR = TMath::Sqrt(
                     (dX - vPntPos.X())*(dX - vPntPos.X()) 
                   + (dY - vPntPos.Y())*(dY - vPntPos.Y()) 
                   + (dZ - vPntPos.Z())*(dZ - vPntPos.Z()) );
               if( dTrkDeltaR < dClosestTrkDr )
               {
                  iClosestTrkIdx  = iPtIdx;
                  dClosestTrkDr   = dTrkDeltaR;
               } // if( dTrkDeltaR < dClosestTrkDr )
               // Check if furthest MC track to Hit position
               if( dFurthestTrkDr < dTrkDeltaR )
               {
                  iFurthestTrkIdx  = iPtIdx;
                  dFurthestTrkDr   = dTrkDeltaR;
               } // if( dFurthestTrkDr < dTrkDeltaR )
            } // if( kFALSE == bTrackFound )
         } // for( Int_t iDigiIdx = 0; iDigiIdx < iNbDigisHit; iDigiIdx++)
      } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
         else
         {
            CbmTofDigi *pTofDigi;
            CbmTofDigi *pTofDigiB;
            for( Int_t iDigi = 0; iDigi < iNbDigisHit; iDigi++)
            {
               CbmLink lDigi    = pMatchHitDigi->GetLink(iDigi); 
               Int_t   iDigiIdx = lDigi.GetIndex();
               
               if( iNbTofDigis <= iDigiIdx )
               {
                  LOG(ERROR)<<"CbmTofHitFinderQa::FillHistos => Digi index from Hit #"
                      <<iHitInd<<" in event #"<<fEvents
                      <<" is bigger than nb entries in Digis arrays => ignore it!!!"<<FairLogger::endl;
                  continue;
               } // if( iNbTofDigis <= iDigiIdx )
               
               pTofDigi      = (CbmTofDigi*) fTofDigisColl->At( iDigiIdx );
               pMatchDigiPnt = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx );

               CbmLink lPt    = pMatchDigiPnt->GetLink(0); 
               Int_t   iPtIdx = lPt.GetIndex();
               Int_t   iTrkId = ((CbmTofPoint*) fTofPointsColl->At(iPtIdx))->GetTrackID();
                  
               Int_t iSmType = pTofDigi->GetType();
               Int_t iSm     = pTofDigi->GetSm();
               Int_t iRpc    = pTofDigi->GetRpc();
               Int_t iCh     = pTofDigi->GetChannel();
               Int_t iGlobalChan = iCh  + fvRpcChOffs[iSmType][iSm][iRpc];
               
               // Check Left-Right MC missmatch (digis match always stored by pairs)
               if( 0 == iDigi%2 )
               {
                  // Get Info about the other end of the strip
                  pMatchDigiPntB = (CbmMatch*) fTofDigiMatchPointsColl->At( iDigiIdx + 1 );

                  CbmLink lPtB    = pMatchDigiPntB->GetLink(0); 
                  Int_t   iPtIdxB = lPtB.GetIndex();
                  
                  // Check Left-Right missmatch for MC Point
                  if( iPtIdx != iPtIdxB )
                  {
                     // Check Left-Right missmatch for MC Track
                     if( iTrkId != ((CbmTofPoint*) fTofPointsColl->At(iPtIdxB))->GetTrackID() )
                     {
                        fhLeftRightDigiMatch->Fill(iGlobalChan, 2);
                     } // if( iTrkId != ((CbmTofPoint*) fTofPointsColl->At(iPtIdxB))->GetTrackID() )
                        else fhLeftRightDigiMatch->Fill(iGlobalChan, 1);
                  } // if( iPtIdx != iPtIdxB )
                     else fhLeftRightDigiMatch->Fill(iGlobalChan, 0);

               } // if( 0 == iDigi%2 )
               
               // Count Nb different MC Points in Hit
               Bool_t bPointFound = kFALSE;
               for( Int_t iPrevPtIdx = 0; iPrevPtIdx < vTofPointsId.size(); iPrevPtIdx++)
                  if( iPtIdx == vTofPointsId[iPrevPtIdx] )
                  {
                     bPointFound = kTRUE;
                     break;
                  } // if( iPtIdx == vTofPointsId[iPrevPtIdx] )
               if( kFALSE == bPointFound )
               {
                  vTofPointsId.push_back(iPtIdx);
                  
                  // Obtain Point position
                  pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iPtIdx);
                  TVector3 vPntPos;
                  pTofPoint->Position( vPntPos );
            
                  // Compute mean MC Points position (X, Y, Z, T)
                  dPntMeanPosX += vPntPos.X();
                  dPntMeanPosY += vPntPos.Y();
                  dPntMeanPosZ += vPntPos.Z();
                  dPntMeanTime += pTofPoint->GetTime();
               
                  // Check if closest MC Point to Hit position
                  Double_t dPntDeltaR = TMath::Sqrt(
                        (dX - vPntPos.X())*(dX - vPntPos.X()) 
                      + (dY - vPntPos.Y())*(dY - vPntPos.Y()) 
                      + (dZ - vPntPos.Z())*(dZ - vPntPos.Z()) );
                  if( dPntDeltaR < dClosestPntDr )
                  {
                     iClosestPntIdx  = iPtIdx;
                     dClosestPntDr   = dPntDeltaR;
                  } // if( dPntDeltaR < dClosestPntDr )
                  // Check if furthest MC Point to Hit position
                  if( dFurthestPntDr < dPntDeltaR )
                  {
                     iFurthestPntIdx  = iPtIdx;
                     dFurthestPntDr   = dPntDeltaR;
                  } // if( dFurthestPntDr < dPntDeltaR )
               } // if( kFALSE == bPointFound )
                  
               // Count Nb different MC Tracks in Hit
               Bool_t bTrackFound = kFALSE;
               for( Int_t iPrevTrkIdx = 0; iPrevTrkIdx < vTofTracksId.size(); iPrevTrkIdx++)
                  if( iTrkId == vTofTracksId[iPrevTrkIdx] )
                  {
                     bTrackFound = kTRUE;
                     break;
                  } // if( iTrkId == vTofPointsId[iPrevTrkIdx] )
               if( kFALSE == bTrackFound )
               {
                  vTofTracksId.push_back(iTrkId);
                  
                  // Obtain Point position (Consider 1st Pnt of each Trk is approximate coord)
                  pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iPtIdx);
                  TVector3 vPntPos;
                  pTofPoint->Position( vPntPos );
            
                  // Compute mean MC Tracks position (X, Y, Z, T)
                  dTrkMeanPosX += vPntPos.X();
                  dTrkMeanPosY += vPntPos.Y();
                  dTrkMeanPosZ += vPntPos.Z();
                  dTrkMeanTime += pTofPoint->GetTime();
               
                  // Check if closest MC track to Hit position
                  Double_t dTrkDeltaR = TMath::Sqrt(
                        (dX - vPntPos.X())*(dX - vPntPos.X()) 
                      + (dY - vPntPos.Y())*(dY - vPntPos.Y()) 
                      + (dZ - vPntPos.Z())*(dZ - vPntPos.Z()) );
                  if( dTrkDeltaR < dClosestTrkDr )
                  {
                     iClosestTrkIdx  = iPtIdx;
                     dClosestTrkDr   = dTrkDeltaR;
                  } // if( dTrkDeltaR < dClosestTrkDr )
                  // Check if furthest MC track to Hit position
                  if( dFurthestTrkDr < dTrkDeltaR )
                  {
                     iFurthestTrkIdx  = iPtIdx;
                     dFurthestTrkDr   = dTrkDeltaR;
                  } // if( dFurthestTrkDr < dTrkDeltaR )
               } // if( kFALSE == bTrackFound )
            } // for( Int_t iDigiIdx = 0; iDigiIdx < iNbDigisHit; iDigiIdx++)
         } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
         
      // Count Nb different MC Points in Hit
      fhNbPointsInHit->Fill( vTofPointsId.size() );
      // Count Nb different MC Tracks in Hit
      fhNbTracksInHit->Fill( vTofTracksId.size() );
      
      // Check Hit Quality for Hits coming from a single MC Point
      if( 1 == vTofPointsId.size() )
      {
         pTofPoint = (CbmTofPoint*) fTofPointsColl->At(vTofPointsId[0]);
         
         // Obtain Point position
         TVector3 vPntPos;
         pTofPoint->Position( vPntPos );
         
         Double_t dDeltaX = dX - vPntPos.X();
         Double_t dDeltaY = dY - vPntPos.Y();
         Double_t dDeltaZ = dZ - vPntPos.Z();
         Double_t dDeltaR = TMath::Sqrt(   dDeltaX*dDeltaX 
                                         + dDeltaY*dDeltaY 
                                         + dDeltaZ*dDeltaZ );
      
         fhSinglePointHitDeltaX->Fill( dDeltaX );
         fhSinglePointHitDeltaY->Fill( dDeltaY );
         fhSinglePointHitDeltaZ->Fill( dDeltaZ );
         fhSinglePointHitDeltaR->Fill( dDeltaR );
         fhSinglePointHitDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
      } // if( 1 == vTofPointsId.size() )
      // For Hits with multiple MC Points
         else
         {
            // Check Hit Quality relative to mean of all MC Points
            dPntMeanPosX /= vTofPointsId.size();
            dPntMeanPosY /= vTofPointsId.size();
            dPntMeanPosZ /= vTofPointsId.size();
            dPntMeanTime /= vTofPointsId.size();
            
            Double_t dDeltaMeanX = dX - dPntMeanPosX;
            Double_t dDeltaMeanY = dY - dPntMeanPosY;
            Double_t dDeltaMeanZ = dZ - dPntMeanPosZ;
            Double_t dDeltaMeanR = TMath::Sqrt(   dDeltaMeanX*dDeltaMeanX 
                                                + dDeltaMeanY*dDeltaMeanY 
                                                + dDeltaMeanZ*dDeltaMeanZ );
                                                
            fhMultiPntHitMeanDeltaX->Fill( dDeltaMeanX );
            fhMultiPntHitMeanDeltaY->Fill( dDeltaMeanY );
            fhMultiPntHitMeanDeltaZ->Fill( dDeltaMeanZ );
            fhMultiPntHitMeanDeltaR->Fill( dDeltaMeanR );
            fhMultiPntHitMeanDeltaT->Fill( 1000.0*(pTofHit->GetTime() - dPntMeanTime) );
            
            // Check Hit Quality relative to closest MC Point
            pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iClosestPntIdx);
            
               // Obtain Point position
            TVector3 vPntPosClo;
            pTofPoint->Position( vPntPosClo );
         
            Double_t dDeltaCloX = dX - vPntPosClo.X();
            Double_t dDeltaCloY = dY - vPntPosClo.Y();
            Double_t dDeltaCloZ = dZ - vPntPosClo.Z();
            Double_t dDeltaCloR = TMath::Sqrt(   dDeltaCloX*dDeltaCloX 
                                               + dDeltaCloY*dDeltaCloY 
                                               + dDeltaCloZ*dDeltaCloZ );
         
            fhMultiPntHitClosestDeltaX->Fill( dDeltaCloX );
            fhMultiPntHitClosestDeltaY->Fill( dDeltaCloY );
            fhMultiPntHitClosestDeltaZ->Fill( dDeltaCloZ );
            fhMultiPntHitClosestDeltaR->Fill( dDeltaCloR );
            fhMultiPntHitClosestDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
            // Check Hit Quality relative to furthest MC Point
            pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iFurthestPntIdx);
            
               // Obtain Point position
            TVector3 vPntPosFar;
            pTofPoint->Position( vPntPosFar );
         
            Double_t dDeltaFarX = dX - vPntPosFar.X();
            Double_t dDeltaFarY = dY - vPntPosFar.Y();
            Double_t dDeltaFarZ = dZ - vPntPosFar.Z();
            Double_t dDeltaFarR = TMath::Sqrt(   dDeltaFarX*dDeltaFarX 
                                               + dDeltaFarY*dDeltaFarY 
                                               + dDeltaFarZ*dDeltaFarZ );
         
            fhMultiPntHitFurthestDeltaX->Fill( dDeltaFarX );
            fhMultiPntHitFurthestDeltaY->Fill( dDeltaFarY );
            fhMultiPntHitFurthestDeltaZ->Fill( dDeltaFarZ );
            fhMultiPntHitFurthestDeltaR->Fill( dDeltaFarR );
            fhMultiPntHitFurthestDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
         } // else of if( 1 == vTofPointsId.size() )
         
      // Check Hit Quality for Hits coming from a single MC Track
      if( 1 == vTofTracksId.size() )
      {
         // => If all points come from the same track, any of them should give
         //    the approximate position and time of the track at the detector level
         pTofPoint = (CbmTofPoint*) fTofPointsColl->At(vTofPointsId[0]);
         
         // Obtain Point position
         TVector3 vPntPos;
         pTofPoint->Position( vPntPos );
         
         Double_t dDeltaX = dX - vPntPos.X();
         Double_t dDeltaY = dY - vPntPos.Y();
         Double_t dDeltaZ = dZ - vPntPos.Z();
         Double_t dDeltaR = TMath::Sqrt(   dDeltaX*dDeltaX 
                                         + dDeltaY*dDeltaY 
                                         + dDeltaZ*dDeltaZ );
      
         fhSingleTrackHitDeltaX->Fill( dDeltaX );
         fhSingleTrackHitDeltaY->Fill( dDeltaY );
         fhSingleTrackHitDeltaZ->Fill( dDeltaZ );
         fhSingleTrackHitDeltaR->Fill( dDeltaR );
         fhSingleTrackHitDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
      } // if( 1 == vTofTracksId.size() )
      // For Hits with multiple MC Tracks
         else
         {
            // Check Hit Quality relative to mean of all MC Tracks            
            dTrkMeanPosX /= vTofTracksId.size();
            dTrkMeanPosY /= vTofTracksId.size();
            dTrkMeanPosZ /= vTofTracksId.size();
            dTrkMeanTime /= vTofTracksId.size();
            
            Double_t dDeltaMeanX = dX - dTrkMeanPosX;
            Double_t dDeltaMeanY = dY - dTrkMeanPosY;
            Double_t dDeltaMeanZ = dZ - dTrkMeanPosZ;
            Double_t dDeltaMeanR = TMath::Sqrt(   dDeltaMeanX*dDeltaMeanX 
                                                + dDeltaMeanY*dDeltaMeanY 
                                                + dDeltaMeanZ*dDeltaMeanZ );
            fhMultiTrkHitMeanDeltaX->Fill( dDeltaMeanX );
            fhMultiTrkHitMeanDeltaY->Fill( dDeltaMeanY );
            fhMultiTrkHitMeanDeltaZ->Fill( dDeltaMeanZ );
            fhMultiTrkHitMeanDeltaR->Fill( dDeltaMeanR );
            fhMultiTrkHitMeanDeltaT->Fill( 1000.0*(pTofHit->GetTime() - dTrkMeanTime) );
            
            // Check Hit Quality relative to closest MC Tracks
               // Obtain Point position (Consider 1st Pnt of each Trk is approximate coord)
            pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iClosestTrkIdx);
            TVector3 vPntPosClo;
            pTofPoint->Position( vPntPosClo );
         
            Double_t dDeltaCloX = dX - vPntPosClo.X();
            Double_t dDeltaCloY = dY - vPntPosClo.Y();
            Double_t dDeltaCloZ = dZ - vPntPosClo.Z();
            Double_t dDeltaCloR = TMath::Sqrt(   dDeltaCloX*dDeltaCloX 
                                               + dDeltaCloY*dDeltaCloY 
                                               + dDeltaCloZ*dDeltaCloZ );
         
            fhMultiTrkHitClosestDeltaX->Fill( dDeltaCloX );
            fhMultiTrkHitClosestDeltaY->Fill( dDeltaCloY );
            fhMultiTrkHitClosestDeltaZ->Fill( dDeltaCloZ );
            fhMultiTrkHitClosestDeltaR->Fill( dDeltaCloR );
            fhMultiTrkHitClosestDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
            // Check Hit Quality relative to furthest MC Tracks
               // Obtain Point position (Consider 1st Pnt of each Trk is approximate coord)
            pTofPoint = (CbmTofPoint*) fTofPointsColl->At(iFurthestTrkIdx);
            TVector3 vPntPosFar;
            pTofPoint->Position( vPntPosFar );
         
            Double_t dDeltaFarX = dX - vPntPosFar.X();
            Double_t dDeltaFarY = dY - vPntPosFar.Y();
            Double_t dDeltaFarZ = dZ - vPntPosFar.Z();
            Double_t dDeltaFarR = TMath::Sqrt(   dDeltaFarX*dDeltaFarX 
                                               + dDeltaFarY*dDeltaFarY 
                                               + dDeltaFarZ*dDeltaFarZ );
         
            fhMultiTrkHitFurthestDeltaX->Fill( dDeltaFarX );
            fhMultiTrkHitFurthestDeltaY->Fill( dDeltaFarY );
            fhMultiTrkHitFurthestDeltaZ->Fill( dDeltaFarZ );
            fhMultiTrkHitFurthestDeltaR->Fill( dDeltaFarR );
            fhMultiTrkHitFurthestDeltaT->Fill( 1000.0*(pTofHit->GetTime() - pTofPoint->GetTime()) );
         } // if( 1 == vTofTracksId.size() )
      
      // clear storage of point and track IDs
      vTofPointsId.clear();
      vTofTracksId.clear();
   } // for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++ )
   
   return kTRUE;
}
// ------------------------------------------------------------------

Bool_t CbmTofHitFinderQa::SetHistoFileNameCartCoordNorm( TString sFilenameIn )
{
   fsHistoInNormCartFilename = sFilenameIn;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::SetHistoFileNameAngCoordNorm( TString sFilenameIn )
{
   fsHistoInNormAngFilename = sFilenameIn;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::SetHistoFileNameSphCoordNorm( TString sFilenameIn )
{
   fsHistoInNormSphFilename = sFilenameIn;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::NormalizeMapHistos()
{
   // TIP: Normalization files can (should?) be generated using the FairBoxGenerator
   //      to get uniform distributions with the right density in the desired coords.
   // Normalization of Cartesian coordinates maps 
   if( "" != fsHistoInNormCartFilename )
   {
      // Open file and set folders properly to avoid creating objects in uncontrolled places
      TDirectory * oldir = gDirectory;
      TFile *fHistNorm = new TFile( fsHistoInNormCartFilename,"READ");
      if( NULL == fHistNorm )
      {
         LOG(ERROR)<<"CbmTofHitFinderQa::NormalizeMapHistos => Could not open file"
                    <<" with Normalization histos of Cartesian coordinates maps with name "
                    << fsHistoInNormCartFilename
                    <<" => Stopping normalization here!"<<FairLogger::endl;
         return kFALSE;           
      } // if( NULL == fHistNorm )
      gROOT->cd();
      
      // Prepare pointers to access normalization histos
      TH2 * fhPointMapNormXY = NULL;
      TH2 * fhPointMapNormXZ = NULL;
      TH2 * fhPointMapNormYZ = NULL;
      TH2 * fhDigiMapNormXY  = NULL;
      TH2 * fhDigiMapNormXZ  = NULL;
      TH2 * fhDigiMapNormYZ  = NULL;
      TH2 * fhHitMapNormXY   = NULL;
      TH2 * fhHitMapNormXZ   = NULL;
      TH2 * fhHitMapNormYZ   = NULL;
      
      // Get pointers for norm histos from file if they exist
      fhPointMapNormXY = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapXY"));
      fhPointMapNormXZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapXZ"));
      fhPointMapNormYZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapYZ"));
      
      fhDigiMapNormXY = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapXY"));
      fhDigiMapNormXZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapXZ"));
      fhDigiMapNormYZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapYZ"));
      
      fhHitMapNormXY = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapXY"));
      fhHitMapNormXZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapXZ"));
      fhHitMapNormYZ = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapYZ"));
      
      // If norm histo found, normalize corresponding map
      if( NULL != fhPointMapNormXY )
         fhPointMapXY->Divide( fhPointMapNormXY );
      if( NULL != fhPointMapNormXZ )
         fhPointMapXZ->Divide( fhPointMapNormXZ );
      if( NULL != fhPointMapNormYZ )
         fhPointMapYZ->Divide( fhPointMapNormYZ );
      if( NULL != fhDigiMapNormXY  )
         fhDigiMapXY->Divide( fhDigiMapNormXY );
      if( NULL != fhDigiMapNormXZ  )
         fhDigiMapXZ->Divide( fhDigiMapNormXZ );
      if( NULL != fhDigiMapNormYZ  )
         fhDigiMapYZ->Divide( fhDigiMapNormYZ );
      if( NULL != fhHitMapNormXY   )
         fhHitMapXY->Divide( fhHitMapNormXY );
      if( NULL != fhHitMapNormXZ   )
         fhHitMapXZ->Divide( fhHitMapNormXZ );
      if( NULL != fhHitMapNormYZ   )
         fhHitMapYZ->Divide( fhHitMapNormYZ );
      
      // Go back to original folder, whereever it is
      gDirectory->cd( oldir->GetPath() );

      fHistNorm->Close();
   } // if( "" != fsHistoInNormCartFilename )
      else LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => Name of input file for"
                    <<" Normalization of Cartesian coordinates maps not given"
                    <<" => no normalization for these maps!"<<FairLogger::endl;
   // Normalization of Angular   coordinates maps 
   if( "" != fsHistoInNormAngFilename )
   {
      // Open file and set folders properly to avoid creating objects in uncontrolled places
      TDirectory * oldir = gDirectory;
      TFile *fHistNorm = new TFile( fsHistoInNormAngFilename,"READ");
      if( NULL == fHistNorm )
      {
         LOG(ERROR)<<"CbmTofHitFinderQa::NormalizeMapHistos => Could not open file"
                    <<" with Normalization histos of Angular   coordinates maps with name "
                    << fsHistoInNormAngFilename
                    <<" => Stopping normalization here!"<<FairLogger::endl;
         return kFALSE;           
      } // if( NULL == fHistNorm )
      gROOT->cd();
      
      // Prepare pointers to access normalization histos
      TH2 * fhPointMapNormAng = NULL;
      TH2 * fhDigiMapNormAng  = NULL;
      TH2 * fhHitMapNormAng   = NULL;
      
      // Get pointers for norm histos from file if they exist
      fhPointMapNormAng = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapAng"));
      fhDigiMapNormAng = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapAng"));
      fhHitMapNormAng = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapAng"));
      
      // If norm histo found, normalize corresponding map
      if( NULL != fhPointMapNormAng )
         fhPointMapAng->Divide( fhPointMapNormAng );
      if( NULL != fhDigiMapNormAng )
         fhDigiMapAng->Divide( fhDigiMapNormAng );
      if( NULL != fhHitMapNormAng )
         fhHitMapAng->Divide( fhHitMapNormAng );
      
      // Go back to original folder, whereever it is
      gDirectory->cd( oldir->GetPath() );

      fHistNorm->Close();
   } // if( "" != fsHistoInNormAngFilename )
      else LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => Name of input file for"
                    <<" Normalization of Angular   coordinates maps not given"
                    <<" => no normalization for these maps!"<<FairLogger::endl;
   // Normalization of Spherical coordinates maps 
   if( "" != fsHistoInNormSphFilename )
   {
      // Open file and set folders properly to avoid creating objects in uncontrolled places
      TDirectory * oldir = gDirectory;
      TFile *fHistNorm = new TFile( fsHistoInNormSphFilename,"READ");
      if( NULL == fHistNorm )
      {
         LOG(ERROR)<<"CbmTofHitFinderQa::NormalizeMapHistos => Could not open file"
                    <<" with Normalization histos of Spherical coordinates maps with name "
                    << fsHistoInNormSphFilename
                    <<" => Stopping normalization here!"<<FairLogger::endl;
         return kFALSE;           
      } // if( NULL == fHistNorm )
      gROOT->cd();
      
      // Prepare pointers to access normalization histos
      TH2 * fhPointMapNormSph = NULL;
      TH2 * fhDigiMapNormSph  = NULL;
      TH2 * fhHitMapNormSph   = NULL;
      
      // Get pointers for norm histos from file if they exist
      fhPointMapNormSph = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_PointsMapSph"));
      fhDigiMapNormSph = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_DigisMapSph"));
      fhHitMapNormSph = static_cast<TH2*> (fHistNorm->FindObjectAny("TofTests_HitsMapSph"));
      
      // If norm histo found, normalize corresponding map
      if( NULL != fhPointMapNormSph )
         fhPointMapSph->Divide( fhPointMapNormSph );
      if( NULL != fhDigiMapNormSph )
         fhDigiMapSph->Divide( fhDigiMapNormSph );
      if( NULL != fhHitMapNormSph )
         fhHitMapSph->Divide( fhHitMapNormSph );
      
      // Go back to original folder, whereever it is
      gDirectory->cd( oldir->GetPath() );

      fHistNorm->Close();
   } // if( "" != fsHistoInNormSphFilename )
      else LOG(INFO)<<"CbmTofHitFinderQa::NormalizeMapHistos => Name of input file for"
                    <<" Normalization of Spherical coordinates maps not given"
                    <<" => no normalization for these maps!"<<FairLogger::endl;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::NormalizeNormHistos()
{
   if( 0 < fhTrackMapXY->GetEntries())
   {
      fhPointMapXY->Divide( fhTrackMapXY );
      fhDigiMapXY ->Divide( fhTrackMapXY );
      fhHitMapXY  ->Divide( fhTrackMapXY );
   } // if( 0 < fhTrackMapXY->GetEntries())
   /*
   if( 0 < fhTrackMapXZ->GetEntries())
   {
      fhPointMapXZ->Divide( fhTrackMapXZ );
      fhDigiMapXZ ->Divide( fhTrackMapXZ );
      fhHitMapXZ  ->Divide( fhTrackMapXZ );
   } // if( 0 < fhTrackMapXZ->GetEntries())
   if( 0 < fhTrackMapYZ->GetEntries())
   {
      fhPointMapYZ->Divide( fhTrackMapYZ );
      fhDigiMapYZ ->Divide( fhTrackMapYZ );
      fhHitMapYZ  ->Divide( fhTrackMapYZ );
   } // if( 0 < fhTrackMapYZ->GetEntries())
   */
   if( 0 < fhTrackMapAng->GetEntries())
   {
      fhPointMapAng->Divide( fhTrackMapAng );
      fhDigiMapAng ->Divide( fhTrackMapAng );
      fhHitMapAng  ->Divide( fhTrackMapAng );
   } // if( 0 < fhTrackMapAng->GetEntries())
   if( 0 < fhTrackMapSph->GetEntries())
   {
      fhPointMapSph->Divide( fhTrackMapSph );
      fhDigiMapSph ->Divide( fhTrackMapSph );
      fhHitMapSph  ->Divide( fhTrackMapSph );
   } // if( 0 < fhTrackMapSph->GetEntries()) 

   return kTRUE;
}
// ------------------------------------------------------------------

Bool_t CbmTofHitFinderQa::SetHistoFileName( TString sFilenameIn )
{
   fsHistoOutFilename = sFilenameIn;
   return kTRUE;
}
Bool_t CbmTofHitFinderQa::WriteHistos()
{
   // TODO: add sub-folders ?

   // Write histogramms to the file
   TDirectory * oldir = gDirectory;
   TFile *fHist = new TFile( fsHistoOutFilename,"RECREATE");
   fHist->cd();
      // Mapping
   if( kTRUE == fbNormHistGenMode )
   {
      fhTrackMapXY->Write();
      fhTrackMapXZ->Write();
      fhTrackMapYZ->Write();
      fhTrackMapAng->Write();
      fhTrackMapSph->Write();
   } // if( kTRUE == fbNormHistGenMode )
   fhPointMapXY->Write();
   fhPointMapXZ->Write();
   fhPointMapYZ->Write();
   fhPointMapAng->Write();
   fhPointMapSph->Write();
   fhDigiMapXY->Write();
   fhDigiMapXZ->Write();
   fhDigiMapYZ->Write();
   fhDigiMapAng->Write();
   fhDigiMapSph->Write();
   fhHitMapXY->Write();
   fhHitMapXZ->Write();
   fhHitMapYZ->Write();
   fhHitMapAng->Write();
   fhHitMapSph->Write();
   
   if( kFALSE == fbNormHistGenMode )
   {
         // L/R digis missmatch
      fhLeftRightDigiMatch->Write();
         // Nb different MC Points and Tracks in Hit
      fhNbPointsInHit->Write();
      fhNbTracksInHit->Write();
         // Hit Quality for Hits coming from a single MC Point
      fhSinglePointHitDeltaX->Write();
      fhSinglePointHitDeltaY->Write();
      fhSinglePointHitDeltaZ->Write();
      fhSinglePointHitDeltaR->Write();
      fhSinglePointHitDeltaT->Write();
         // Hit Quality for Hits coming from a multiple MC Points
            // To Point closest to Hit
      fhMultiPntHitClosestDeltaX->Write();
      fhMultiPntHitClosestDeltaY->Write();
      fhMultiPntHitClosestDeltaZ->Write();
      fhMultiPntHitClosestDeltaR->Write();
      fhMultiPntHitClosestDeltaT->Write();
            // To Point furthest from Hit
      fhMultiPntHitFurthestDeltaX->Write();
      fhMultiPntHitFurthestDeltaY->Write();
      fhMultiPntHitFurthestDeltaZ->Write();
      fhMultiPntHitFurthestDeltaR->Write();
      fhMultiPntHitFurthestDeltaT->Write();
            // To mean Point position
      fhMultiPntHitMeanDeltaX->Write();
      fhMultiPntHitMeanDeltaY->Write();
      fhMultiPntHitMeanDeltaZ->Write();
      fhMultiPntHitMeanDeltaR->Write();
      fhMultiPntHitMeanDeltaT->Write();
         // Hit Quality for Hits coming from a single MC Track
      fhSingleTrackHitDeltaX->Write();
      fhSingleTrackHitDeltaY->Write();
      fhSingleTrackHitDeltaZ->Write();
      fhSingleTrackHitDeltaR->Write();
      fhSingleTrackHitDeltaT->Write();
         // Hit Quality for Hits coming from a multiple MC Points
            // To Track closest to Hit
      fhMultiTrkHitClosestDeltaX->Write();
      fhMultiTrkHitClosestDeltaY->Write();
      fhMultiTrkHitClosestDeltaZ->Write();
      fhMultiTrkHitClosestDeltaR->Write();
      fhMultiTrkHitClosestDeltaT->Write();
            // To Track furthest from Hit
      fhMultiTrkHitFurthestDeltaX->Write();
      fhMultiTrkHitFurthestDeltaY->Write();
      fhMultiTrkHitFurthestDeltaZ->Write();
      fhMultiTrkHitFurthestDeltaR->Write();
      fhMultiTrkHitFurthestDeltaT->Write();
            // To mean Track position
      fhMultiTrkHitMeanDeltaX->Write();
      fhMultiTrkHitMeanDeltaY->Write();
      fhMultiTrkHitMeanDeltaZ->Write();
      fhMultiTrkHitMeanDeltaR->Write();
      fhMultiTrkHitMeanDeltaT->Write();
   } // if( kFALSE == fbNormHistGenMode )

   gDirectory->cd( oldir->GetPath() );

   fHist->Close();

   return kTRUE;
}
Bool_t   CbmTofHitFinderQa::DeleteHistos()
{
   // Mapping
   if( kTRUE == fbNormHistGenMode )
   {
      delete fhTrackMapXY;
      delete fhTrackMapXZ;
      delete fhTrackMapYZ;
      delete fhTrackMapAng;
      delete fhTrackMapSph;
   } // if( kTRUE == fbNormHistGenMode )
   delete fhPointMapXY;
   delete fhPointMapXZ;
   delete fhPointMapYZ;
   delete fhPointMapAng;
   delete fhPointMapSph;
   delete fhDigiMapXY;
   delete fhDigiMapXZ;
   delete fhDigiMapYZ;
   delete fhDigiMapAng;
   delete fhDigiMapSph;
   delete fhHitMapXY;
   delete fhHitMapXZ;
   delete fhHitMapYZ;
   delete fhHitMapAng;
   delete fhHitMapSph;
      // L/R digis missmatch
   delete fhLeftRightDigiMatch;
      // Nb different MC Points and Tracks in Hit
   delete fhNbPointsInHit;
   delete fhNbTracksInHit;
      // Hit Quality for Hits coming from a single MC Point
   delete fhSinglePointHitDeltaX;
   delete fhSinglePointHitDeltaY;
   delete fhSinglePointHitDeltaZ;
   delete fhSinglePointHitDeltaR;
   delete fhSinglePointHitDeltaT;
      // Hit Quality for Hits coming from a multiple MC Points
         // To Point closest to Hit
   delete fhMultiPntHitClosestDeltaX;
   delete fhMultiPntHitClosestDeltaY;
   delete fhMultiPntHitClosestDeltaZ;
   delete fhMultiPntHitClosestDeltaR;
   delete fhMultiPntHitClosestDeltaT;
         // To Point furthest from Hit
   delete fhMultiPntHitFurthestDeltaX;
   delete fhMultiPntHitFurthestDeltaY;
   delete fhMultiPntHitFurthestDeltaZ;
   delete fhMultiPntHitFurthestDeltaR;
   delete fhMultiPntHitFurthestDeltaT;
         // To mean Point position
   delete fhMultiPntHitMeanDeltaX;
   delete fhMultiPntHitMeanDeltaY;
   delete fhMultiPntHitMeanDeltaZ;
   delete fhMultiPntHitMeanDeltaR;
   delete fhMultiPntHitMeanDeltaT;
      // Hit Quality for Hits coming from a single MC Track
   delete fhSingleTrackHitDeltaX;
   delete fhSingleTrackHitDeltaY;
   delete fhSingleTrackHitDeltaZ;
   delete fhSingleTrackHitDeltaR;
   delete fhSingleTrackHitDeltaT;
      // Hit Quality for Hits coming from a multiple MC Points
         // To Track closest to Hit
   delete fhMultiTrkHitClosestDeltaX;
   delete fhMultiTrkHitClosestDeltaY;
   delete fhMultiTrkHitClosestDeltaZ;
   delete fhMultiTrkHitClosestDeltaR;
   delete fhMultiTrkHitClosestDeltaT;
         // To Track furthest from Hit
   delete fhMultiTrkHitFurthestDeltaX;
   delete fhMultiTrkHitFurthestDeltaY;
   delete fhMultiTrkHitFurthestDeltaZ;
   delete fhMultiTrkHitFurthestDeltaR;
   delete fhMultiTrkHitFurthestDeltaT;
         // To mean Track position
   delete fhMultiTrkHitMeanDeltaX;
   delete fhMultiTrkHitMeanDeltaY;
   delete fhMultiTrkHitMeanDeltaZ;
   delete fhMultiTrkHitMeanDeltaR;
   delete fhMultiTrkHitMeanDeltaT;
   
   return kTRUE;
}


ClassImp(CbmTofHitFinderQa);

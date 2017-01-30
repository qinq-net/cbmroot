/** @file CbmTofTestBeamClusterizer.cxx 
 ** author nh  
 ** adopted from
 ** @file CbmTofSimpClusterizer.cxx 
 ** @author Pierre-Alain Loizeau <loizeau@physi.uni-heidelberg.de>
 ** @date 23.08.2013
 **/

#include "CbmTofTestBeamClusterizer.h"

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
#include "CbmMatch.h"

#include "TTrbHeader.h"

// CBMroot classes and includes
#include "CbmMCTrack.h"

// FAIR classes and includes
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"

// ROOT Classes and includes
#include "TClonesArray.h"
#include "TMath.h"
#include "TLine.h"
#include "TRandom3.h"
#include "TF1.h"
#include "TF2.h"
#include "TVector3.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "TGeoManager.h"

// Constants definitions
#include "CbmTofClusterizersDef.h"

// C++ Classes and includes 
// Globals 
#include <vector>

static Int_t    iIndexDut = 0;
static Double_t StartAnalysisTime = 0.;

/************************************************************************************/
CbmTofTestBeamClusterizer::CbmTofTestBeamClusterizer():
   FairTask("CbmTofTestBeamClusterizer"),
   fGeoHandler(new CbmTofGeoHandler()),
   fTofId(NULL),
   fDigiPar(NULL),
   fChannelInfo(NULL),
   fDigiBdfPar(NULL),
   fTrbHeader(NULL),
   fTofPointsColl(NULL),
   fMcTracksColl(NULL),
   fTofDigisColl(NULL),
   fbWriteHitsInOut(kTRUE),
   fbWriteDigisInOut(kTRUE),
   fTofCalDigisColl(NULL),
   fTofHitsColl(NULL),
   fTofDigiMatchColl(NULL),
   fiNbHits(0),
   fVerbose(1),
   fStorDigi(),
   fStorDigiExp(),
   fStorDigiInd(),
   fviClusterMul(),
   fviClusterSize(),
   fviTrkMul(),
   fvdX(),
   fvdY(),
   fvdDifX(),
   fvdDifY(),
   fvdDifCh(),
   fhClustBuildTime(NULL),
   fhHitsPerTracks(NULL),
   fhPtsPerHit(NULL),
   fhTimeResSingHits(NULL),
   fhTimeResSingHitsB(NULL),
   fhTimePtVsHits(NULL),
   fhClusterSize(NULL),
   fhClusterSizeType(NULL),
   fhTrackMul(NULL),
   fhClusterSizeMulti(NULL),
   fhTrk1MulPos(NULL),
   fhHiTrkMulPos(NULL),
   fhAllTrkMulPos(NULL),
   fhMultiTrkProbPos(NULL),
   fhDigSpacDifClust(NULL),
   fhDigTimeDifClust(NULL),
   fhDigDistClust(NULL),
   fhClustSizeDifX(NULL),
   fhClustSizeDifY(NULL),
   fhChDifDifX(NULL),
   fhChDifDifY(NULL),
   fhCluMulCorDutSel(NULL),
   fhRpcDigiCor(),
   fhRpcCluMul(),
   fhRpcCluRate(),
   fhRpcCluPosition(),
   fhRpcCluDelPos(),
   fhRpcCluDelMatPos(),
   fhRpcCluTOff(),
   fhRpcCluDelTOff(),
   fhRpcCluDelMatTOff(),
   fhRpcCluTrms(),
   fhRpcCluTot(),
   fhRpcCluSize(),
   fhRpcCluAvWalk(),
   fhRpcCluAvLnWalk(),
   fhRpcCluWalk(),
   fhSmCluPosition(),
   fhSmCluTOff(),
   fhSmCluSvel(),
   fhRpcDTLastHits(),
   fhTRpcCluMul(),
   fhTRpcCluPosition(),
   fhTRpcCluTOff(),
   fhTRpcCluTot(),
   fhTRpcCluSize(),
   fhTRpcCluAvWalk(),
   fhTRpcCluDelTof(),
   fhTRpcCludXdY(),
   fhTRpcCluWalk(),
   fhTSmCluPosition(),
   fhTSmCluTOff(),
   fhTSmCluTRun(),
   fhTRpcCluTOffDTLastHits(),
   fhTRpcCluTotDTLastHits(),
   fhTRpcCluSizeDTLastHits(),
   fhTRpcCluMemMulDTLastHits(),
   fhSeldT(),
   fvCPDelTof(),
   fvCPTOff(),
   fvCPTotGain(),
   fvCPTotOff(),
   fvCPWalk(),
   fvLastHits(),
   fiNbSameSide(0),
   fhNbSameSide(NULL),
   fhNbDigiPerChan(NULL),
   fStart(),
   fStop(),
   dTRef(0.),
   fdTRefMax(0.),
   fCalMode(0),
   fCalSel(0),
   fCalSmAddr(0),
   fdCaldXdYMax(0.),
   fiCluMulMax(0),
   fTRefMode(0),
   fTRefHits(0),
   fDutId(0),
   fDutSm(0),
   fDutRpc(0),
   fDutAddr(0),
   fSelId(0),
   fSelSm(0),
   fSelRpc(0),
   fSelAddr(0),
   fiBeamRefType(0),
   fiBeamRefSm(0),
   fiBeamRefDet(0),
   fiBeamRefAddr(0),
   fiBeamRefMulMax(1),
   fiBeamAddRefMul(0),
   fSel2Id(0),
   fSel2Sm(0),
   fSel2Rpc(0),
   fSel2Addr(0),
   fDetIdIndexMap(),
   fviDetId(),
   fPosYMaxScal(0.),
   fTRefDifMax(0.),
   fTotMax(0.),
   fTotMin(0.),
   fTotOff(0.),
   fTotMean(0.),
   fdDelTofMax(60000.),
   fTotPreRange(0.),
   fMaxTimeDist(0.),
   fdChannelDeadtime(0.),
   fdMemoryTime(0.),
   fEnableMatchPosScaling(kTRUE),
   fEnableAvWalk(kFALSE),
   fbPs2Ns(kFALSE),
   fCalParFileName(""),
   fOutHstFileName(""),
   fCalParFile(NULL),      
   fiNevtBuild(0),
   fiMsgCnt(100),
   fdTOTMax(5.E4),
   fdTOTMin(0.),
   fdTTotMean(5.E3), //2.E4),
   fdMaxTimeDist(0.),
   fdMaxSpaceDist(0.),
   fdEvent(0)
{

}

CbmTofTestBeamClusterizer::CbmTofTestBeamClusterizer(const char *name, Int_t verbose, Bool_t writeDataInOut):
   FairTask(TString(name),verbose),
   fGeoHandler(new CbmTofGeoHandler()),
   fTofId(NULL),
   fDigiPar(NULL),
   fChannelInfo(NULL),
   fDigiBdfPar(NULL),
   fTrbHeader(NULL),
   fTofPointsColl(NULL),
   fMcTracksColl(NULL),
   fTofDigisColl(NULL),
   fbWriteHitsInOut(writeDataInOut),
   fbWriteDigisInOut(writeDataInOut),
   fTofCalDigisColl(NULL),
   fTofHitsColl(NULL),
   fTofDigiMatchColl(NULL),
   fiNbHits(0),
   fVerbose(verbose),
   fStorDigi(),
   fStorDigiExp(),
   fStorDigiInd(),
   fviClusterMul(),
   fviClusterSize(),
   fviTrkMul(),
   fvdX(),
   fvdY(),
   fvdDifX(),
   fvdDifY(),
   fvdDifCh(),
   fhClustBuildTime(NULL),
   fhHitsPerTracks(NULL),
   fhPtsPerHit(NULL),
   fhTimeResSingHits(NULL),
   fhTimeResSingHitsB(NULL),
   fhTimePtVsHits(NULL),
   fhClusterSize(NULL),
   fhClusterSizeType(NULL),
   fhTrackMul(NULL),
   fhClusterSizeMulti(NULL),
   fhTrk1MulPos(NULL),
   fhHiTrkMulPos(NULL),
   fhAllTrkMulPos(NULL),
   fhMultiTrkProbPos(NULL),
   fhDigSpacDifClust(NULL),
   fhDigTimeDifClust(NULL),
   fhDigDistClust(NULL),
   fhClustSizeDifX(NULL),
   fhClustSizeDifY(NULL),
   fhChDifDifX(NULL),
   fhChDifDifY(NULL),
   fhCluMulCorDutSel(NULL),
   fhRpcDigiCor(),
   fhRpcCluMul(),
   fhRpcCluRate(),
   fhRpcCluPosition(),
   fhRpcCluDelPos(),
   fhRpcCluDelMatPos(),
   fhRpcCluTOff(),
   fhRpcCluDelTOff(),
   fhRpcCluDelMatTOff(),
   fhRpcCluTrms(),
   fhRpcCluTot(),
   fhRpcCluSize(),
   fhRpcCluAvWalk(),
   fhRpcCluAvLnWalk(),
   fhRpcCluWalk(),
   fhSmCluPosition(),
   fhSmCluTOff(),
   fhSmCluSvel(),
   fhRpcDTLastHits(),
   fhTRpcCluMul(),
   fhTRpcCluPosition(),
   fhTRpcCluTOff(),
   fhTRpcCluTot(),
   fhTRpcCluSize(),
   fhTRpcCluAvWalk(),
   fhTRpcCluDelTof(),
   fhTRpcCludXdY(),
   fhTRpcCluWalk(),
   fhTSmCluPosition(),
   fhTSmCluTOff(),
   fhTSmCluTRun(),
   fhTRpcCluTOffDTLastHits(),
   fhTRpcCluTotDTLastHits(),
   fhTRpcCluSizeDTLastHits(),
   fhTRpcCluMemMulDTLastHits(),
   fhSeldT(),
   fvCPDelTof(),
   fvCPTOff(),
   fvCPTotGain(),
   fvCPTotOff(),
   fvCPWalk(),
   fvLastHits(),
   fiNbSameSide(0),
   fhNbSameSide(NULL),
   fhNbDigiPerChan(NULL),
   fStart(),
   fStop(),
   dTRef(0.),
   fdTRefMax(0.),
   fCalMode(0),
   fCalSel(0),
   fCalSmAddr(0),
   fdCaldXdYMax(0.),
   fiCluMulMax(0),
   fTRefMode(0),
   fTRefHits(0),
   fDutId(0),
   fDutSm(0),
   fDutRpc(0),
   fDutAddr(0),
   fSelId(0),
   fSelSm(0),
   fSelRpc(0),
   fSelAddr(0),
   fiBeamRefType(0),
   fiBeamRefSm(0),
   fiBeamRefDet(0),
   fiBeamRefAddr(0),
   fiBeamRefMulMax(1),
   fiBeamAddRefMul(0),
   fSel2Id(0),
   fSel2Sm(0),
   fSel2Rpc(0),
   fSel2Addr(0),
   fDetIdIndexMap(),
   fviDetId(),
   fPosYMaxScal(0.),
   fTRefDifMax(0.),
   fTotMax(0.),
   fTotMin(0.),
   fTotOff(0.),
   fTotMean(0.),
   fdDelTofMax(60000.),
   fTotPreRange(0.),
   fMaxTimeDist(0.),
   fdChannelDeadtime(0.),
   fdMemoryTime(0.),
   fEnableMatchPosScaling(kTRUE),
   fEnableAvWalk(kFALSE),
   fbPs2Ns(kFALSE),
   fCalParFileName(""),
   fOutHstFileName(""),
   fCalParFile(NULL),      
   fiNevtBuild(0),
   fiMsgCnt(100),
   fdTOTMax(5.E4),
   fdTOTMin(0.),
   fdTTotMean(5.E3), //2.E4),
   fdMaxTimeDist(0.),
   fdMaxSpaceDist(0.),
   fdEvent(0)
{
}

CbmTofTestBeamClusterizer::~CbmTofTestBeamClusterizer()
{
   if( fGeoHandler )
      delete fGeoHandler;
//   DeleteHistos(); // <-- if needed  ?
}

/************************************************************************************/
// FairTasks inherited functions
InitStatus CbmTofTestBeamClusterizer::Init()
{
    LOG(INFO) << "CbmTofTestBeamClusterizer initializing... expect Digis in ns units! "
              <<FairLogger::endl;

   if( kFALSE == RegisterInputs() )
      return kFATAL;

   if( kFALSE == RegisterOutputs() )
      return kFATAL;

   if( kFALSE == InitParameters() )
      return kFATAL;

   if( kFALSE == LoadGeometry() )
      return kFATAL;

   if( kFALSE == InitCalibParameter() )
      return kFATAL;

   if( kFALSE == CreateHistos() )
      return kFATAL;

    fDutAddr =CbmTofAddress::GetUniqueAddress(fDutSm,fDutRpc,0,0,fDutId);
    fSelAddr =CbmTofAddress::GetUniqueAddress(fSelSm,fSelRpc,0,0,fSelId);
    fSel2Addr=CbmTofAddress::GetUniqueAddress(fSel2Sm,fSel2Rpc,0,0,fSel2Id);
    fiBeamRefAddr=CbmTofAddress::GetUniqueAddress(fiBeamRefSm,fiBeamRefDet,0,0,fiBeamRefType);
    iIndexDut=fDigiBdfPar->GetDetInd(fDutAddr);
    iIndexDut=fDigiBdfPar->GetDetInd(fSelAddr);

   return kSUCCESS;
}


void CbmTofTestBeamClusterizer::SetParContainers()
{
   LOG(INFO)<<"=> Get the digi parameters for tof"<<FairLogger::endl;
   LOG(WARNING)<<"Return without action"<<FairLogger::endl;
   return;
   // Get Base Container
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();

   fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));

   LOG(INFO)<<"found " 
            << fDigiPar->GetNrOfModules() << " cells " <<FairLogger::endl;
   fDigiBdfPar = (CbmTofDigiBdfPar*)
              (rtdb->getContainer("CbmTofDigiBdfPar"));
}


void CbmTofTestBeamClusterizer::Exec(Option_t* /*option*/)
{
   // Clear output arrays 
   fTofCalDigisColl->Clear("C");
   fTofHitsColl->Clear("C");
   //fTofHitsColl->Delete();  // Computationally costly!, but hopefully safe
   //for (Int_t i=0; i<fTofDigiMatchColl->GetEntries(); i++) ((CbmMatch *)(fTofDigiMatchColl->At(i)))->ClearLinks();  // FIXME, try to tamper memory leak (did not help)
   //fTofDigiMatchColl->Clear("C+L");  // leads to memory leak
   fTofDigiMatchColl->Delete();

   fiNbHits = 0;

   LOG(DEBUG)<<"CbmTofTestBeamClusterizer::Exec => New event"<<FairLogger::endl;
   fStart.Set();

   BuildClusters();

   MergeClusters();

   fStop.Set();
   
   fdEvent++;
   FillHistos();

   //   fTofDigisColl->RemoveAll();
}

/************************************************************************************/
void CbmTofTestBeamClusterizer::Finish()
{
   WriteHistos();
   // Prevent them from being sucked in by the CbmHadronAnalysis WriteHistograms method
   // DeleteHistos();
   if(fdMemoryTime>0.) CleanLHMemory();
}

void CbmTofTestBeamClusterizer::Finish(Double_t calMode)
{
   SetCalMode(calMode);
   WriteHistos();
}

/************************************************************************************/
// Functions common for all clusters approximations
Bool_t   CbmTofTestBeamClusterizer::RegisterInputs()
{
   FairRootManager *fManager = FairRootManager::Instance();

   if( NULL == fManager)
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::RegisterInputs => Could not find FairRootManager!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigisColl)

   /*
   fTofPointsColl  = (TClonesArray *) fManager->GetObject("TofPoint");
   if( NULL == fTofPointsColl)
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::RegisterInputs => Could not get the TofPoint TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofPointsColl)

   fMcTracksColl   = (TClonesArray *) fManager->GetObject("MCTrack");
   if( NULL == fMcTracksColl)
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::RegisterInputs => Could not get the MCTrack TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fMcTracksColl)
   */

   fTofDigisColl   = (TClonesArray *) fManager->GetObject("CbmTofDigiExp");

   if( NULL == fTofDigisColl)
      fTofDigisColl = (TClonesArray *) fManager->GetObject("CbmTofDigi");

   if( NULL == fTofDigisColl)
      fTofDigisColl = (TClonesArray *) fManager->GetObject("TofDigiExp");

   if( NULL == fTofDigisColl)
      fTofDigisColl = (TClonesArray *) fManager->GetObject("TofDigi");
  
   if( NULL == fTofDigisColl)
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::RegisterInputs => Could not get the CbmTofDigi TClonesArray!!!"<<FairLogger::endl;
      return kFALSE;
   } // if( NULL == fTofDigisColl)


   fTrbHeader = (TTrbHeader *)  fManager->GetObject("TofTrbHeader.");
   if( NULL == fTrbHeader)
   {
      LOG(INFO)<<"CbmTofTestBeamClusterizer::RegisterInputs => Could not get the TofTrbHeader Object"<<FairLogger::endl;
   }

   return kTRUE;
}
Bool_t   CbmTofTestBeamClusterizer::RegisterOutputs()
{
   FairRootManager* rootMgr = FairRootManager::Instance();

   fTofCalDigisColl = new TClonesArray("CbmTofDigiExp");

   // Flag check to control whether digis are written in output root file
   rootMgr->Register( "TofCalDigi","Tof", fTofCalDigisColl, fbWriteDigisInOut);


   fTofHitsColl = new TClonesArray("CbmTofHit");

   // Flag check to control whether digis are written in output root file
   rootMgr->Register( "TofHit","Tof", fTofHitsColl, fbWriteHitsInOut);

   fTofDigiMatchColl = new TClonesArray("CbmMatch",100);
   rootMgr->Register( "TofDigiMatch","Tof", fTofDigiMatchColl, fbWriteHitsInOut);

   return kTRUE;

}
Bool_t   CbmTofTestBeamClusterizer::InitParameters()
{

   // Initialize the TOF GeoHandler
   Bool_t isSimulation=kFALSE;
   LOG(INFO)<<"CbmTofTestBeamClusterizer::InitParameters - Geometry, Mapping, ...  ??"
            <<FairLogger::endl;

   // Get Base Container
   FairRun* ana = FairRun::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();   

   Int_t iGeoVersion = fGeoHandler->Init(isSimulation);
   if( k14a > iGeoVersion )
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitParameters => Only compatible with geometries after v14a !!!"
                <<FairLogger::endl;
      return kFALSE;
   }
   fTofId = new CbmTofDetectorId_v14a();
   
   fDigiPar = (CbmTofDigiPar*) (rtdb->getContainer("CbmTofDigiPar"));
   if( 0 == fDigiPar )
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitParameters => Could not obtain the CbmTofDigiPar "<<FairLogger::endl;
      return kFALSE; 
   }

   fDigiBdfPar = (CbmTofDigiBdfPar*) (rtdb->getContainer("CbmTofDigiBdfPar"));
   if( 0 == fDigiBdfPar )
   {
      LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitParameters => Could not obtain the CbmTofDigiBdfPar "<<FairLogger::endl;
      return kFALSE; 
   }

   rtdb->initContainers(  ana->GetRunId() );

   LOG(INFO)<<"CbmTofTestBeamClusterizer::InitParameter: currently " 
            << fDigiPar->GetNrOfModules() << " digi cells " <<FairLogger::endl;

   
   fdMaxTimeDist  = fDigiBdfPar->GetMaxTimeDist();        // in ns
   fdMaxSpaceDist = fDigiBdfPar->GetMaxDistAlongCh();     // in cm

   if(fMaxTimeDist!=fdMaxTimeDist) {
     fdMaxTimeDist=fMaxTimeDist;            // modify default
     fdMaxSpaceDist=fdMaxTimeDist*fDigiBdfPar->GetSignalSpeed()*0.5; // cut consistently on positions (with default signal velocity)
   }

   LOG(INFO)<<" BuildCluster with MaxTimeDist "<<fdMaxTimeDist<<", MaxSpaceDist "
             <<fdMaxSpaceDist
             <<FairLogger::endl;

   if(fiCluMulMax==0) fiCluMulMax=100;
   if (fOutHstFileName==""){
     fOutHstFileName="./tofTestBeamClust.hst.root";
   }

   LOG(INFO)<<" Hst Output filename = "<<fOutHstFileName
             <<FairLogger::endl;
   /*
   if(fiBeamRefAddr == 0) {  // initialize defaults of sep14
     fiBeamRefType  = 5;
     fiBeamRefSm    = 1;
     fiBeamRefDet   = 0;
     fiBeamAddRefMul= 0;
   }
   if(fSelId == 0) {  // initialize defaults of sep14
     fSelId=4;
   }
   */

   LOG(INFO)<<"<I>  BeamRefType = "<<fiBeamRefType<<", Sm "<<fiBeamRefSm<<", Det "<<fiBeamRefDet
	    <<", MulMax "<<fiBeamRefMulMax
            <<FairLogger::endl;

   return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofTestBeamClusterizer::InitCalibParameter()
{
  // dimension and initialize calib parameter
  Int_t iNbDet     = fDigiBdfPar->GetNbDet();
  Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();

  if (fTotMean !=0.) fdTTotMean=fTotMean;   // adjust target mean for TTT

  fvCPTOff.resize( iNbSmTypes );
  fvCPTotGain.resize( iNbSmTypes );
  fvCPTotOff.resize( iNbSmTypes );
  fvCPWalk.resize( iNbSmTypes );
  fvCPDelTof.resize( iNbSmTypes );
  for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
  {
      Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
      Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
      fvCPTOff[iSmType].resize( iNbSm*iNbRpc );
      fvCPTotGain[iSmType].resize( iNbSm*iNbRpc );
      fvCPTotOff[iSmType].resize( iNbSm*iNbRpc );
      fvCPWalk[iSmType].resize( iNbSm*iNbRpc );
      fvCPDelTof[iSmType].resize( iNbSm*iNbRpc );
      for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
      {
        for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
        {
          //          LOG(INFO)<<Form(" fvCPDelTof resize for SmT %d, R %d, B %d ",iSmType,iNbSm*iNbRpc,nbClDelTofBinX)
          //           <<FairLogger::endl;
          fvCPDelTof[iSmType][iSm*iNbRpc+iRpc].resize( nbClDelTofBinX );
          for(Int_t iBx=0; iBx<nbClDelTofBinX; iBx++){ 
            // LOG(INFO)<<Form(" fvCPDelTof for SmT %d, R %d, B %d",iSmType,iSm*iNbRpc+iRpc,iBx)<<FairLogger::endl;
              fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx].resize( iNSel );
            for(Int_t iSel=0; iSel<iNSel; iSel++)
                  fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel]=0.;  // initialize
          }

          Int_t iNbChan = fDigiBdfPar->GetNbChan( iSmType, iRpc );
          fvCPTOff[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
          fvCPTotGain[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
          fvCPTotOff[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
          fvCPWalk[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
          Int_t nbSide  =2 - fDigiBdfPar->GetChanType( iSmType, iRpc );
          for (Int_t iCh=0; iCh<iNbChan; iCh++)
          {
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh].resize( nbSide );
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh].resize( nbSide );
            fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh].resize( nbSide );
            fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh].resize( nbSide );          
            for(Int_t iSide=0; iSide<nbSide; iSide++){
              fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]=0.;      //initialize
              fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]=1.;   //initialize
              fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]=0.;   //initialize
              fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][iSide].resize( nbClWalkBinX );
              for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
                fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][iSide][iWx]=0.;
              }
            }    
          }
        }
      }
  }

  LOG(INFO)<<"CbmTofTestBeamClusterizer::InitCalibParameter: defaults set"
           <<FairLogger::endl;

  TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
  /*
  gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !
  */

  if(0<fCalMode){
    LOG(INFO) << "CbmTofTestBeamClusterizer::InitCalibParameter: read histos from "
                 << "file " << fCalParFileName << FairLogger::endl;

  // read parameter from histos
    if(fCalParFileName.IsNull()) return kTRUE;

    fCalParFile = new TFile(fCalParFileName,"");
    if(NULL == fCalParFile) {
      LOG(FATAL) << "CbmTofTestBeamClusterizer::InitCalibParameter: "
                 << "file " << fCalParFileName << " does not exist!" << FairLogger::endl;
      return kTRUE;
    }
    /*
    gDirectory->Print();
    fCalParFile->cd();
    fCalParFile->ls();
    */
    for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
    {
      Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType );
      Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType );
      TProfile *hSvel    =  (TProfile *) gDirectory->FindObjectAny(  Form("cl_SmT%01d_Svel", iSmType) );

      // copy Histo to memory
      TDirectory * curdir = gDirectory;
      if(NULL != hSvel) {
	gDirectory->cd( oldir->GetPath() );
	TProfile *hSvelmem = (TProfile *)hSvel->Clone();
	gDirectory->cd( curdir->GetPath() );
      }
      for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
        for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
        {
	  // update default parameter
	  if(NULL != hSvel){
	    Double_t Vscal=hSvel->GetBinContent(iSm*iNbRpc+iRpc+1);
	    if (Vscal==0.) Vscal=1.;
	    fDigiBdfPar->SetSigVel(iSmType,iSm,iRpc,fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)*Vscal);
	    LOG(INFO)<<"Modify for "<<iSmType<<iSm<<iRpc<<" Svel by "<<Vscal<<" to "<<fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)
		     << FairLogger::endl;
	  }
          TH2F *htempPos_pfx =(TH2F*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Pos_pfx",iSmType,iSm,iRpc));
          TH2F *htempTOff_pfx=(TH2F*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_TOff_pfx",iSmType,iSm,iRpc));
          TH1D *htempTot_Mean =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_Mean",iSmType,iSm,iRpc));
          TH1D *htempTot_Off  =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_Off",iSmType,iSm,iRpc));
          if(NULL != htempPos_pfx && NULL != htempTOff_pfx && NULL != htempTot_Mean && NULL != htempTot_Off)   
          {
            Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
            Int_t iNbinTot = htempTot_Mean->GetNbinsX();
            for( Int_t iCh = 0; iCh < iNbCh; iCh++ )
              {
                Double_t YMean=((TProfile *)htempPos_pfx)->GetBinContent(iCh+1);  //nh +1 empirical(?)
                Double_t TMean=((TProfile *)htempTOff_pfx)->GetBinContent(iCh+1);
                //Double_t dTYOff=YMean/fDigiBdfPar->GetSignalSpeed() ;
                Double_t dTYOff=YMean/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc) ;
                fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] += -dTYOff + TMean ;
                fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1] += +dTYOff + TMean ;
 
                for(Int_t iSide=0; iSide<2; iSide++){
                  Double_t TotMean=htempTot_Mean->GetBinContent(iCh*2+1+iSide);  //nh +1 empirical(?)
                  if(1<TotMean){
                    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] *= fdTTotMean / TotMean;
                  }
                  fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]=htempTot_Off->GetBinContent(iCh*2+1+iSide);
                }

                if(5 == iSmType || 8 == iSmType){
                  fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]=fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0];
                  fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1] = fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0];
                  fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]=fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][0];
                }

                LOG(DEBUG)<<"CbmTofTestBeamClusterizer::InitCalibParameter:" 
                           <<" SmT "<< iSmType<<" Sm "<<iSm<<" Rpc "<<iRpc<<" Ch "<<iCh
                         << Form(": YMean %f, TMean %f",YMean, TMean)
                         <<" -> " << Form(" %f, %f, %f, %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                    fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1],
                                    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]) 
                           <<", NbinTot "<< iNbinTot
                           <<FairLogger::endl;

                TH1D *htempWalk0=(TH1D*)gDirectory->FindObjectAny( 
                                 Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
                TH1D *htempWalk1=(TH1D*)gDirectory->FindObjectAny( 
                                 Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
                if(NULL != htempWalk0 && NULL != htempWalk1 ) { // reinitialize Walk array 
                  LOG(DEBUG)<<"Initialize Walk correction for "
                            <<Form(" SmT%01d_sm%03d_rpc%03d_Ch%03d",iSmType, iSm, iRpc, iCh)
                            <<FairLogger::endl;
                  if(htempWalk0->GetNbinsX() != nbClWalkBinX) 
                    LOG(ERROR)<<"CbmTofTestBeamClusterizer::InitCalibParameter: Inconsistent Walk histograms"
                              <<FairLogger::endl;
                   for( Int_t iBin = 0; iBin < nbClWalkBinX; iBin++ )
                   {
                     fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iBin]=htempWalk0->GetBinContent(iBin+1);
                     fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iBin]=htempWalk1->GetBinContent(iBin+1);
                     LOG(DEBUG1)<<Form(" SmT%01d_sm%03d_rpc%03d_Ch%03d bin %d walk %f ",iSmType, iSm, iRpc, iCh, iBin,
                                       fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iBin])
                              <<FairLogger::endl;
                     if(5 == iSmType || 8 == iSmType){  // Pad structure
                       fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iBin]=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iBin];
                     }
                   }
                }
            }
          }
          else {
            LOG(WARNING)<<" Calibration histos " << Form("cl_SmT%01d_sm%03d_rpc%03d_XXX", iSmType, iSm, iRpc) 
                      << " not found. "
                      <<FairLogger::endl;
          }
          for(Int_t iSel=0; iSel<iNSel; iSel++){
           TH1D *htmpDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
           if (NULL==htmpDelTof) {
            LOG(DEBUG)<<" Histos " << Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof", iSmType, iSm, iRpc, iSel) << " not found. "
                      <<FairLogger::endl;
            continue;
           }
           LOG(DEBUG)<<" Load DelTof from histos "<< Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<"."
                    <<FairLogger::endl;
           for(Int_t iBx=0; iBx<nbClDelTofBinX; iBx++){
            fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel] += htmpDelTof->GetBinContent(iBx+1);
           }

           // copy Histo to memory
           // TDirectory * curdir = gDirectory;
           gDirectory->cd( oldir->GetPath() );
           TH1D *h1DelTof=(TH1D *)htmpDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));

           LOG(DEBUG)<<" copy histo "
                      <<h1DelTof->GetName()
                      <<" to directory "
                     <<oldir->GetName()
                     <<FairLogger::endl;

           gDirectory->cd( curdir->GetPath() );
          }
        }
    }
  }
  //   fCalParFile->Delete();
  gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!
  LOG(INFO)<<"CbmTofTestBeamClusterizer::InitCalibParameter: initialization done"
           <<FairLogger::endl; 
  return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofTestBeamClusterizer::LoadGeometry()
{
   LOG(INFO)<<"CbmTofTestBeamClusterizer::LoadGeometry starting for  "
            <<fDigiBdfPar->GetNbDet() << " described detectors, "
            <<fDigiPar->GetNrOfModules() << " geometrically known modules "
            <<FairLogger::endl;

   //fTRefMode=0x00002016;  // use plastic 1 as time reference 

   Int_t iNrOfCells = fDigiPar->GetNrOfModules();
   LOG(INFO)<<"Digi Parameter container contains "<<iNrOfCells<<" cells."<<FairLogger::endl;
   for (Int_t icell = 0; icell < iNrOfCells; ++icell) {

     Int_t cellId = fDigiPar->GetCellId(icell); // cellId is assigned in CbmTofCreateDigiPar
     fChannelInfo = fDigiPar->GetCell(cellId);

     Int_t smtype  = fGeoHandler->GetSMType(cellId);
     Int_t smodule = fGeoHandler->GetSModule(cellId);
     Int_t module  = fGeoHandler->GetCounter(cellId);
     Int_t cell    = fGeoHandler->GetCell(cellId);

     Double_t x = fChannelInfo->GetX();
     Double_t y = fChannelInfo->GetY();
     Double_t z = fChannelInfo->GetZ();
     Double_t dx = fChannelInfo->GetSizex();
     Double_t dy = fChannelInfo->GetSizey();
     LOG(DEBUG)
       << "-I- InitPar "<<icell<<" Id: "<< Form("0x%08x",cellId)
            << " "<< cell << " tmcs: "<< smtype <<" "<<smodule<<" "<<module<<" "<<cell   
            << " x="<<Form("%6.2f",x)<<" y="<<Form("%6.2f",y)<<" z="<<Form("%6.2f",z)
            <<" dx="<<dx<<" dy="<<dy<<FairLogger::endl;

      TGeoNode *fNode=        // prepare local->global trafo
      gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());
      LOG(DEBUG2)<<Form(" Node at (%6.1f,%6.1f,%6.1f) : 0x%p",
                          fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ(),fNode)
                 <<FairLogger::endl;
      //      fNode->Print();        

   }

   Int_t iNbDet=fDigiBdfPar->GetNbDet();
   for(Int_t iDetIndx=0; iDetIndx<iNbDet; iDetIndx++){
       Int_t iUniqueId = fDigiBdfPar->GetDetUId( iDetIndx );
       Int_t iSmType   = CbmTofAddress::GetSmType( iUniqueId );
       Int_t iSmId     = CbmTofAddress::GetSmId( iUniqueId );
       Int_t iRpcId    = CbmTofAddress::GetRpcId( iUniqueId );
       LOG(INFO) << " DetIndx "<<iDetIndx<<"("<<iNbDet<<"), SmType "<<iSmType<<", SmId "<<iSmId
                 << ", RpcId "<<iRpcId<<" => UniqueId "<<Form("0x%08x ",iUniqueId)
                 << Form(" Svel %6.6f ",fDigiBdfPar->GetSigVel(iSmType,iSmId,iRpcId))
                 <<FairLogger::endl;
       Int_t iCell=-1; 
       while (kTRUE){
         Int_t iUCellId= CbmTofAddress::GetUniqueAddress(iSmId,iRpcId,++iCell,0,iSmType);
         fChannelInfo = fDigiPar->GetCell(iUCellId);
         if (NULL == fChannelInfo) break;
         LOG(DEBUG3) << " Cell " << iCell << Form(" 0x%08x ",iUCellId)
                   << Form(", fCh 0x%p ",fChannelInfo)
                   << ", x: " << fChannelInfo->GetX()
                   << ", y: " << fChannelInfo->GetY()
                   << ", z: " << fChannelInfo->GetZ()
                   << FairLogger::endl; 
         }

   }

   //   return kTRUE;

   Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();

   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      fStorDigiExp.resize( iNbSmTypes );
      fStorDigiInd.resize( iNbSmTypes );
      fviClusterSize.resize( iNbSmTypes );
      fviTrkMul.resize( iNbSmTypes );
      fvdX.resize( iNbSmTypes );
      fvdY.resize( iNbSmTypes );
      fvdDifX.resize( iNbSmTypes );
      fvdDifY.resize( iNbSmTypes );
      fvdDifCh.resize( iNbSmTypes );
      fviClusterMul.resize( iNbSmTypes );
      fvLastHits.resize( iNbSmTypes );

      for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      {
         Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
         Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
         fStorDigiExp[iSmType].resize( iNbSm*iNbRpc );
         fStorDigiInd[iSmType].resize( iNbSm*iNbRpc );
         fviClusterSize[iSmType].resize( iNbRpc );
         fviTrkMul[iSmType].resize( iNbRpc );
         fvdX[iSmType].resize( iNbRpc );
         fvdY[iSmType].resize( iNbRpc );
         fvdDifX[iSmType].resize( iNbRpc );
         fvdDifY[iSmType].resize( iNbRpc );
         fvdDifCh[iSmType].resize( iNbRpc );
         for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
         {
           fviClusterMul[iSmType].resize( iNbSm );
           fvLastHits[iSmType].resize( iNbSm );
           for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
            {
               fviClusterMul[iSmType][iSm].resize( iNbRpc );
               fvLastHits[iSmType][iSm].resize( iNbRpc );
               Int_t iNbChan = fDigiBdfPar->GetNbChan( iSmType, iRpc );
	       if(iNbChan == 0) {
               LOG(WARNING)<<"CbmTofTestBeamClusterizer::LoadGeometry: StoreDigi without channels "
                         << Form("SmTy %3d, Sm %3d, NbRpc %3d, Rpc, %3d ",iSmType,iSm,iNbRpc,iRpc)
                         << FairLogger::endl;
	       }
               LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::LoadGeometry: StoreDigi with "
                         << Form(" %3d %3d %3d %3d %5d ",iSmType,iSm,iNbRpc,iRpc,iNbChan)
                         << FairLogger::endl;
               fStorDigiExp[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
               fStorDigiInd[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
               fvLastHits[iSmType][iSm][iRpc].resize( iNbChan );
            } // for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
         } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
      } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      else
      {
         fStorDigi.resize( iNbSmTypes );
         fStorDigiInd.resize( iNbSmTypes );
         fviClusterSize.resize( iNbSmTypes );
         fviTrkMul.resize( iNbSmTypes );
         fvdX.resize( iNbSmTypes );
         fvdY.resize( iNbSmTypes );
         fvdDifX.resize( iNbSmTypes );
         fvdDifY.resize( iNbSmTypes );
         fvdDifCh.resize( iNbSmTypes );
         for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
         {
            Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
            Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
            fStorDigi[iSmType].resize( iNbSm*iNbRpc );
            fStorDigiInd[iSmType].resize( iNbSm*iNbRpc );
            fviClusterSize[iSmType].resize( iNbRpc );
            fviTrkMul[iSmType].resize( iNbRpc );
            fvdX[iSmType].resize( iNbRpc );
            fvdY[iSmType].resize( iNbRpc );
            fvdDifX[iSmType].resize( iNbRpc );
            fvdDifY[iSmType].resize( iNbRpc );
            fvdDifCh[iSmType].resize( iNbRpc );
            for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
            {
               for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
               {
                  Int_t iNbChan = fDigiBdfPar->GetNbChan( iSmType, iRpc ); 
                  fStorDigi[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
                  fStorDigiInd[iSmType][iSm*iNbRpc+iRpc].resize( iNbChan );
              } // for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
            } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
         } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   return kTRUE;
}
Bool_t   CbmTofTestBeamClusterizer::DeleteGeometry()
{
   LOG(INFO)<<"CbmTofTestBeamClusterizer::DeleteGeometry starting"
             <<FairLogger::endl;
   return kTRUE;
   Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      {
         Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
         Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
         for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
         {
           for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ ) {
               fStorDigiExp[iSmType][iSm*iNbRpc+iRpc].clear();
               fStorDigiInd[iSmType][iSm*iNbRpc+iRpc].clear();
           }
         } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
         fStorDigiExp[iSmType].clear();
         fStorDigiInd[iSmType].clear();
      } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      fStorDigiExp.clear();
      fStorDigiInd.clear();
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      else
      {
         for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
         {
            Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
            Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
            for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
            {
              for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ ){
                 fStorDigi[iSmType][iSm*iNbRpc+iRpc].clear();
                 fStorDigiInd[iSmType][iSm*iNbRpc+iRpc].clear();
              }
            } // for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
            fStorDigi[iSmType].clear();
            fStorDigiInd[iSmType].clear();
         } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
         fStorDigi.clear();
         fStorDigiInd.clear();
      } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   return kTRUE;
}
/************************************************************************************/
// Histogramming functions
Bool_t   CbmTofTestBeamClusterizer::CreateHistos()
{
   TDirectory * oldir = gDirectory; // <= To prevent histos from being sucked in by the param file of the TRootManager!
   gROOT->cd(); // <= To prevent histos from being sucked in by the param file of the TRootManager !
   fhClustBuildTime = new TH1I( "TofTestBeamClus_ClustBuildTime", "Time needed to build clusters in each event; Time [s]",4000, 0.0, 4.0 );

/*   TH2* hTemp = 0;*/

   // Sm related distributions 
   fhSmCluPosition.resize( fDigiBdfPar->GetNbSmTypes() );
   fhSmCluTOff.resize( fDigiBdfPar->GetNbSmTypes() );
   fhSmCluSvel.resize( fDigiBdfPar->GetNbSmTypes() );
   fhTSmCluPosition.resize( fDigiBdfPar->GetNbSmTypes() );
   fhTSmCluTOff.resize( fDigiBdfPar->GetNbSmTypes() );
   fhTSmCluTRun.resize( fDigiBdfPar->GetNbSmTypes() );

   for (Int_t iS=0; iS< fDigiBdfPar->GetNbSmTypes(); iS++){
     Double_t YSCAL=50.;
     if (fPosYMaxScal !=0.) YSCAL=fPosYMaxScal;
     Int_t iUCellId  = CbmTofAddress::GetUniqueAddress(0,0,0,0,iS);
     fChannelInfo = fDigiPar->GetCell(iUCellId);
     if(NULL == fChannelInfo){
         LOG(WARNING)<<"No DigiPar for SmType "
                   <<Form("%d, 0x%08x", iS, iUCellId)
                   <<FairLogger::endl;
         continue;     
     }
     Double_t YDMAX=TMath::Max(2.,fChannelInfo->GetSizey())*YSCAL;

     fhSmCluPosition[iS] =  new TH2F( 
          Form("cl_SmT%01d_Pos", iS),
          Form("Clu position of SmType %d; Sm+Rpc# []; ypos [cm]", iS ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          99, -YDMAX,YDMAX);
 
     Double_t TSumMax=1.E3;
     if (fTRefDifMax !=0.) TSumMax=fTRefDifMax;
     fhSmCluTOff[iS] =  new TH2F( 
          Form("cl_SmT%01d_TOff", iS),
          Form("Clu TimeZero in SmType %d; Sm+Rpc# []; TOff [ns]", iS ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          99, -TSumMax,TSumMax );  
  
     TProfile *hSvelcur =  (TProfile *) gDirectory->FindObjectAny(  Form("cl_SmT%01d_Svel", iS) );
     if(NULL == hSvelcur)
     fhSmCluSvel[iS] =  new TProfile( 
          Form("cl_SmT%01d_Svel", iS),
          Form("Clu Svel in SmType %d; Sm+Rpc# []; v/v_{nominal} ", iS ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          0.8,1.2 );  
     else
       fhSmCluSvel[iS]=(TProfile *) hSvelcur->Clone();

     fhTSmCluPosition[iS].resize( iNSel );
     fhTSmCluTOff[iS].resize( iNSel );
     fhTSmCluTRun[iS].resize( iNSel );
     for (Int_t iSel=0; iSel<iNSel; iSel++){  // Loop over selectors 
       fhTSmCluPosition[iS][iSel] =  new TH2F( 
          Form("cl_TSmT%01d_Sel%02d_Pos", iS, iSel),
          Form("Clu position of SmType %d under Selector %02d; Sm+Rpc# []; ypos [cm]", iS, iSel ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          99, -YDMAX,YDMAX); 
       fhTSmCluTOff[iS][iSel] =  new TH2F( 
          Form("cl_TSmT%01d_Sel%02d_TOff", iS, iSel),
          Form("Clu TimeZero in SmType %d under Selector %02d; Sm+Rpc# []; TOff [ns]", iS, iSel ),
          fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS), 0, fDigiBdfPar->GetNbSm(iS)*fDigiBdfPar->GetNbRpc(iS),
          99, -TSumMax,TSumMax );    
       fhTSmCluTRun[iS][iSel] =  new TH2F( 
          Form("cl_TSmT%01d_Sel%02d_TRun", iS, iSel),
          Form("Clu TimeZero in SmType %d under Selector %02d; Event# []; TMean [ns]", iS, iSel ),
          100, 0, MaxNbEvent,
          99, -TSumMax,TSumMax );    
     }
   }

   // RPC related distributions
   Int_t iNbDet=fDigiBdfPar->GetNbDet();
   LOG(INFO)<<" Define Clusterizer histos for "<<iNbDet<<" detectors "<<FairLogger::endl;

   fhRpcDigiCor.resize( iNbDet  );
   fhRpcCluMul.resize( iNbDet  );
   fhRpcCluRate.resize( iNbDet  );
   fhRpcCluPosition.resize( iNbDet  );
   fhRpcCluDelPos.resize( iNbDet  );
   fhRpcCluDelMatPos.resize( iNbDet  );
   fhRpcCluTOff.resize( iNbDet  );
   fhRpcCluDelTOff.resize( iNbDet  );
   fhRpcCluDelMatTOff.resize( iNbDet  );
   fhRpcCluTrms.resize( iNbDet  );
   fhRpcCluTot.resize( iNbDet  );
   fhRpcCluSize.resize( iNbDet  );
   fhRpcCluAvWalk.resize( iNbDet );
   fhRpcCluAvLnWalk.resize( iNbDet );
   fhRpcCluWalk.resize( iNbDet );
   fhRpcDTLastHits.resize( iNbDet  );
   fviDetId.resize(iNbDet);

   fDetIdIndexMap.clear();

   for(Int_t iDetIndx=0; iDetIndx<iNbDet; iDetIndx++){
       Int_t iUniqueId = fDigiBdfPar->GetDetUId( iDetIndx );
       fDetIdIndexMap[iUniqueId]=iDetIndx;
       fviDetId[iDetIndx]=iUniqueId;

       Int_t iSmType   = CbmTofAddress::GetSmType( iUniqueId );
       Int_t iSmId     = CbmTofAddress::GetSmId( iUniqueId );
       Int_t iRpcId    = CbmTofAddress::GetRpcId( iUniqueId );
       Int_t iUCellId  = CbmTofAddress::GetUniqueAddress(iSmId,iRpcId,0,0,iSmType);
       fChannelInfo = fDigiPar->GetCell(iUCellId);
       if (NULL==fChannelInfo) {
         LOG(WARNING)<<"No DigiPar for Det "
                     <<Form("0x%08x", iUCellId)
                     <<FairLogger::endl;
         continue;
       }      
       LOG(INFO) << "DetIndx "<<iDetIndx<<", SmType "<<iSmType<<", SmId "<<iSmId
                  << ", RpcId "<<iRpcId<<" => UniqueId "<<Form("(0x%08x, 0x%08x)",iUniqueId,iUCellId)
                 <<", dx "<<fChannelInfo->GetSizex()
                 <<", dy "<<fChannelInfo->GetSizey()
                 <<Form(" ChPoi: %p ",fChannelInfo)
                  <<", nbCh "<<fDigiBdfPar->GetNbChan( iSmType, 0 )
                 <<FairLogger::endl;

       // check access to all channel infos 
       for (Int_t iCh=0; iCh<fDigiBdfPar->GetNbChan( iSmType, iRpcId ); iCh++){
	 Int_t iCCellId  = CbmTofAddress::GetUniqueAddress(iSmId,iRpcId,iCh,0,iSmType);
	 fChannelInfo = fDigiPar->GetCell(iCCellId);
	 if(NULL == fChannelInfo)
           LOG(WARNING)<<Form("missing ChannelInfo for ch %d addr 0x%08x",iCh,iCCellId)<<FairLogger::endl;
       }

       fChannelInfo = fDigiPar->GetCell(iUCellId);
       fhRpcDigiCor[iDetIndx] =  new TH2F(
          Form("cl_SmT%01d_sm%03d_rpc%03d_DigiCor", iSmType, iSmId, iRpcId ),
          Form("Digi Correlation of Rpc #%03d in Sm %03d of type %d; digi 0; digi 1", iRpcId, iSmId, iSmType ),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId));

       fhRpcCluMul[iDetIndx] =  new TH1F(
          Form("cl_SmT%01d_sm%03d_rpc%03d_Mul", iSmType, iSmId, iRpcId ),
          Form("Clu multiplicity of Rpc #%03d in Sm %03d of type %d; M []; Cnts", iRpcId, iSmId, iSmType ),
	      fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId));

       fhRpcCluRate[iDetIndx] =  new TH1F(
          Form("cl_SmT%01d_sm%03d_rpc%03d_rate", iSmType, iSmId, iRpcId ),
          Form("Clu rate of Rpc #%03d in Sm %03d of type %d; Time (s); Rate (Hz)", iRpcId, iSmId, iSmType ),
	      3600.,0.,3600.); 

       fhRpcDTLastHits[iDetIndx] =  new TH1F(
          Form("cl_SmT%01d_sm%03d_rpc%03d_DTLastHits", iSmType, iSmId, iRpcId ),
          Form("Clu #DeltaT to last hits  of Rpc #%03d in Sm %03d of type %d; ln ( #DeltaT (ns)); counts", iRpcId, iSmId, iSmType ),
	      100.,0.,15.);
	      
       Double_t YSCAL=50.;
       if (fPosYMaxScal !=0.) YSCAL=fPosYMaxScal;
       Double_t YDMAX=TMath::Max(2.,fChannelInfo->GetSizey())*YSCAL;
       fhRpcCluPosition[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Pos", iSmType, iSmId, iRpcId ),
          Form("Clu position of Rpc #%03d in Sm %03d of type %d; Strip []; ypos [cm]", iRpcId, iSmId, iSmType ),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            99, -YDMAX,YDMAX); 

       fhRpcCluDelPos[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_DelPos", iSmType, iSmId, iRpcId ),
          Form("Clu position difference of Rpc #%03d in Sm %03d of type %d; Strip []; #Deltaypos(clu) [cm]", iRpcId, iSmId, iSmType ),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            99, -10.,10.); 

       fhRpcCluDelMatPos[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_DelMatPos", iSmType, iSmId, iRpcId ),
          Form("Matched Clu position difference of Rpc #%03d in Sm %03d of type %d; Strip []; #Deltaypos(mat) [cm]", iRpcId, iSmId, iSmType ),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            99, -5.,5.); 

       Double_t TSumMax=1.E3;
       if (fTRefDifMax !=0.) TSumMax=fTRefDifMax;
       fhRpcCluTOff[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_TOff", iSmType, iSmId, iRpcId ),
          Form("Clu TimeZero of Rpc #%03d in Sm %03d of type %d; Strip []; TOff [ns]", iRpcId, iSmId, iSmType ),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            99, -TSumMax,TSumMax ); 

       fhRpcCluDelTOff[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_DelTOff", iSmType, iSmId, iRpcId ),
          Form("Clu TimeZero Difference of Rpc #%03d in Sm %03d of type %d; Strip []; #DeltaTOff(clu) [ns]", iRpcId, iSmId, iSmType ),
             fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
             99, -0.6,0.6); 

       fhRpcCluDelMatTOff[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_DelMatTOff", iSmType, iSmId, iRpcId ),
          Form("Clu TimeZero Difference of Rpc #%03d in Sm %03d of type %d; Strip []; #DeltaTOff(mat) [ns]", iRpcId, iSmId, iSmType ),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            99, -0.6,0.6); 

       fhRpcCluTrms[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Trms", iSmType, iSmId, iRpcId ),
          Form("Clu Time RMS of Rpc #%03d in Sm %03d of type %d; Strip []; Trms [ns]", iRpcId, iSmId, iSmType ),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            99, 0., 0.5 ); 

       if (fTotMax !=0.) fdTOTMax=fTotMax; 
       if (fTotMin !=0.) fdTOTMin=fTotMin;
       fhRpcCluTot[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Tot", iSmType, iSmId, iRpcId ),
          Form("Clu Tot of Rpc #%03d in Sm %03d of type %d; StripSide []; TOT [ns]", iRpcId, iSmId, iSmType ),
            2*fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,2*fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            100, fdTOTMin, fdTOTMax);

       fhRpcCluSize[iDetIndx] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Size", iSmType, iSmId, iRpcId ),
          Form("Clu size of Rpc #%03d in Sm %03d of type %d; Strip []; size [strips]", iRpcId, iSmId, iSmType ),
            fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
            16, 0.5, 16.5); 

       // Walk histos 
       fhRpcCluAvWalk[iDetIndx] = new TH2F( 
                          Form("cl_SmT%01d_sm%03d_rpc%03d_AvWalk", iSmType, iSmId, iRpcId),
                          Form("Walk in SmT%01d_sm%03d_rpc%03d_AvWalk", iSmType, iSmId, iRpcId),
                          nbClWalkBinX,fdTOTMin,fdTOTMax,nbClWalkBinY,-TSumMax,TSumMax);
   
       fhRpcCluAvLnWalk[iDetIndx] = new TH2F( 
                          Form("cl_SmT%01d_sm%03d_rpc%03d_AvLnWalk", iSmType, iSmId, iRpcId),
                          Form("Walk in SmT%01d_sm%03d_rpc%03d_AvLnWalk", iSmType, iSmId, iRpcId),
                          nbClWalkBinX,TMath::Log(fdTOTMax/50.),TMath::Log(fdTOTMax),nbClWalkBinY,-TSumMax,TSumMax);

       fhRpcCluWalk[iDetIndx].resize( fDigiBdfPar->GetNbChan(iSmType,iRpcId) );
       for( Int_t iCh=0; iCh<fDigiBdfPar->GetNbChan(iSmType,iRpcId); iCh++){
         fhRpcCluWalk[iDetIndx][iCh].resize( 2 );
         for (Int_t iSide=0; iSide<2; iSide++)
         {
           fhRpcCluWalk[iDetIndx][iCh][iSide]= new TH2F( 
                          Form("cl_SmT%01d_sm%03d_rpc%03d_Ch%03d_S%01d_Walk", iSmType, iSmId, iRpcId, iCh, iSide ),
                          Form("Walk in SmT%01d_sm%03d_rpc%03d_Ch%03d_S%01d_Walk", iSmType, iSmId, iRpcId, iCh, iSide ),
                          nbClWalkBinX,fdTOTMin,fdTOTMax,nbClWalkBinY,-TSumMax,TSumMax );
         }
       /*
         (fhRpcCluWalk[iDetIndx]).push_back( hTemp );
       */
       }
   }

   // Clusterizing monitoring 

   LOG(INFO)<<" Define Clusterizer monitoring histos "<<FairLogger::endl;

   fhCluMulCorDutSel = new TH2F(Form("hCluMulCorDutSel"),
		       	 Form("Cluster Multiplicity Correlation of Dut %d%d%d with Sel %d%d%d; Mul(Sel); Mul(Dut)", 
			      fDutId, fDutSm, fDutRpc, fSelId, fSelSm, fSelRpc ),
			      32,0,32,32,0,32);
 
   fhDigSpacDifClust = new TH1I( "Clus_DigSpacDifClust",
         "Space difference along channel direction between Digi pairs on adjacent channels; PosCh i - Pos Ch i+1 [cm]",
         5000, -10.0, 10.0 );
   fhDigTimeDifClust = new TH1I( "Clus_DigTimeDifClust",
         "Time difference between Digi pairs on adjacent channels; 0.5*(tDigiA + tDigiA)chi - 0.5*(tDigiA + tDigiA)chi+1 [ns]",
         5000, -5.0, 5.0 );
   fhDigDistClust = new TH2I( "Clus_DigDistClust",
         "Distance between Digi pairs on adjacent channels; PosCh i - Pos Ch i+1 [cm along ch]; 0.5*(tDigiA + tDigiA)chi - 0.5*(tDigiA + tDigiA)chi+1 [ns]",
         5000, -10.0, 10.0,    2000, -5.0, 5.0 );
   fhClustSizeDifX = new TH2I( "Clus_ClustSizeDifX", "Position difference between Point and Hit as function of Cluster Size; Cluster Size [Strips]; dX [cm]",
         100, 0.5, 100.5,       500,-50,50);
   fhClustSizeDifY = new TH2I( "Clus_ClustSizeDifY", "Position difference between Point and Hit as function of Cluster Size; Cluster Size [Strips]; dY [cm]",
         100, 0.5, 100.5,       500,-50,50);
   fhChDifDifX = new TH2I( "Clus_ChDifDifX", "Position difference between Point and Hit as function of Channel dif; Ch Dif [Strips]; dX [cm]",
         101, -50.5, 50.5,      500,-50,50);
   fhChDifDifY = new TH2I( "Clus_ChDifDifY", "Position difference between Point and Hit as function of Channel Dif; Ch Dif [Strips]; dY [cm]",
         101, -50.5, 50.5,      500,-50,50);
   fhNbSameSide = new TH1I( "Clus_NbSameSide",
         "How many time per event the 2 digis on a channel were of the same side ; Counts/Event []",
         500, 0.0, 500.0 );
   fhNbDigiPerChan = new TH1I( "Clus_NbDigiPerChan",
         "Nb of Digis per channel; Nb Digis []",
         100, 0.0, 100.0 );

   // Trigger selected histograms 
   if (0<iNSel){
   
   fhSeldT.resize( iNSel );
   for (Int_t iSel=0; iSel<iNSel; iSel++){
       fhSeldT[iSel] =  new TH2F(  Form("cl_dt_Sel%02d", iSel ),
                                   Form("Selector time %02d; dT [ns]",iSel ),
                                   99, -fdDelTofMax*10., fdDelTofMax*10.,
				   16, -0.5, 15.5 ); 
   }
 
   fhTRpcCluMul.resize( iNbDet  );
   fhTRpcCluPosition.resize( iNbDet  );
   fhTRpcCluTOff.resize( iNbDet  );
   fhTRpcCluTot.resize( iNbDet  );
   fhTRpcCluSize.resize( iNbDet  );
   fhTRpcCluAvWalk.resize( iNbDet );
   fhTRpcCluDelTof.resize( iNbDet );
   fhTRpcCludXdY.resize( iNbDet );
   fhTRpcCluWalk.resize( iNbDet );
   fhTRpcCluTOffDTLastHits.resize( iNbDet  );
   fhTRpcCluTotDTLastHits.resize( iNbDet  );
   fhTRpcCluSizeDTLastHits.resize( iNbDet  );
   fhTRpcCluMemMulDTLastHits.resize( iNbDet  );

   for(Int_t iDetIndx=0; iDetIndx<iNbDet; iDetIndx++){
       Int_t iUniqueId = fDigiBdfPar->GetDetUId( iDetIndx );
       Int_t iSmType   = CbmTofAddress::GetSmType( iUniqueId );
       Int_t iSmId     = CbmTofAddress::GetSmId( iUniqueId );
       Int_t iRpcId    = CbmTofAddress::GetRpcId( iUniqueId );
       Int_t iUCellId  = CbmTofAddress::GetUniqueAddress(iSmId,iRpcId,0,0,iSmType);
       fChannelInfo = fDigiPar->GetCell(iUCellId);
       if (NULL==fChannelInfo) {
         LOG(WARNING)<<"No DigiPar for Det "
                     <<Form("0x%08x", iUCellId)
                 <<FairLogger::endl;
         continue;
       }      
       LOG(DEBUG1) << "DetIndx "<<iDetIndx<<", SmType "<<iSmType<<", SmId "<<iSmId
                  << ", RpcId "<<iRpcId<<" => UniqueId "<<Form("(0x%08x, 0x%08x)",iUniqueId,iUCellId)
                 <<", dx "<<fChannelInfo->GetSizex()
                 <<", dy "<<fChannelInfo->GetSizey()
                 <<Form(" poi: 0x%p ",fChannelInfo)
                  <<", nbCh "<<fDigiBdfPar->GetNbChan(iSmType,iRpcId)
                 <<FairLogger::endl;

       fhTRpcCluMul[iDetIndx].resize( iNSel  );
       fhTRpcCluPosition[iDetIndx].resize( iNSel  );
       fhTRpcCluTOff[iDetIndx].resize( iNSel  );
       fhTRpcCluTot[iDetIndx].resize( iNSel  );
       fhTRpcCluSize[iDetIndx].resize( iNSel  );
       fhTRpcCluAvWalk[iDetIndx].resize( iNSel  );
       fhTRpcCluDelTof[iDetIndx].resize( iNSel  );
       fhTRpcCludXdY[iDetIndx].resize( iNSel  );
       fhTRpcCluWalk[iDetIndx].resize( iNSel  );
       fhTRpcCluTOffDTLastHits[iDetIndx].resize( iNSel  );
       fhTRpcCluTotDTLastHits[iDetIndx].resize( iNSel  );
       fhTRpcCluSizeDTLastHits[iDetIndx].resize( iNSel  );
       fhTRpcCluMemMulDTLastHits[iDetIndx].resize( iNSel  );

       for (Int_t iSel=0; iSel<iNSel; iSel++){
       fhTRpcCluMul[iDetIndx][iSel] =  new TH1F(
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Mul", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu multiplicity of Rpc #%03d in Sm %03d of type %d under Selector %02d; M []; cnts", iRpcId, iSmId, iSmType, iSel ),
	  fDigiBdfPar->GetNbChan(iSmType,iRpcId),0.,fDigiBdfPar->GetNbChan(iSmType,iRpcId));
      
       if (NULL == fhTRpcCluMul[iDetIndx][iSel]) LOG(FATAL)<<" Histo not generated !"<<FairLogger::endl; 
       
       Double_t YSCAL=50.;
       if (fPosYMaxScal !=0.) YSCAL=fPosYMaxScal;
       Double_t YDMAX=TMath::Max(2.,fChannelInfo->GetSizey())*YSCAL;
       fhTRpcCluPosition[iDetIndx][iSel] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Pos", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu position of Rpc #%03d in Sm %03d of type %d under Selector %02d; Strip []; ypos [cm]", iRpcId, iSmId, iSmType, iSel ),
          fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
          100, -YDMAX,YDMAX ); 

       Double_t TSumMax=1.E4;
       if (fTRefDifMax !=0.) TSumMax=fTRefDifMax;
       fhTRpcCluTOff[iDetIndx][iSel] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_TOff", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu TimeZero of Rpc #%03d in Sm %03d of type %d under Selector %02d; Strip []; TOff [ns]", iRpcId, iSmId, iSmType, iSel ),
          fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
          99, -TSumMax,TSumMax ); 

       if (fTotMax !=0.) fdTOTMax=fTotMax;
       fhTRpcCluTot[iDetIndx][iSel]  =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Tot", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu Tot of Rpc #%03d in Sm %03d of type %d under Selector %02d; StripSide []; TOT [ns]", iRpcId, iSmId, iSmType, iSel ),
          fDigiBdfPar->GetNbChan(iSmType,iRpcId)*2, 0, fDigiBdfPar->GetNbChan(iSmType,iRpcId)*2,
          100, fdTOTMin, fdTOTMax);

       fhTRpcCluSize[iDetIndx][iSel]  =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Size", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu size of Rpc #%03d in Sm %03d of type %d under Selector %02d; Strip []; size [strips]", iRpcId, iSmId, iSmType, iSel ),
          fDigiBdfPar->GetNbChan(iSmType,iRpcId),0,fDigiBdfPar->GetNbChan(iSmType,iRpcId),
          16, 0.5, 16.5); 

       // Walk histos 
       fhTRpcCluAvWalk[iDetIndx][iSel]  = new TH2F( 
                          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_AvWalk", iSmType, iSmId, iRpcId, iSel),
                          Form("Walk in SmT%01d_sm%03d_rpc%03d_Sel%02d_AvWalk; TOT; T-TSel", iSmType, iSmId, iRpcId, iSel),
                          nbClWalkBinX,fdTOTMin,fdTOTMax,nbClWalkBinY,-TSumMax,TSumMax);

       // Tof Histos 
       fhTRpcCluDelTof[iDetIndx][iSel] = new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof", iSmType, iSmId, iRpcId, iSel),
          Form("SmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof; TRef-TSel; T-TSel", iSmType, iSmId, iRpcId, iSel),
          nbClDelTofBinX,-fdDelTofMax,fdDelTofMax,nbClDelTofBinY,-TSumMax,TSumMax);
      
       // Position deviation histos  
       fhTRpcCludXdY[iDetIndx][iSel] = new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_dXdY", iSmType, iSmId, iRpcId, iSel),
          Form("SmT%01d_sm%03d_rpc%03d_Sel%02d_dXdY; #Delta x [cm]; #Delta y [cm];", iSmType, iSmId, iRpcId, iSel),
          nbCldXdYBinX,-dXdYMax,dXdYMax,nbCldXdYBinY,-dXdYMax,dXdYMax);

       fhTRpcCluWalk[iDetIndx][iSel].resize( fDigiBdfPar->GetNbChan(iSmType,iRpcId) );
       for( Int_t iCh=0; iCh<fDigiBdfPar->GetNbChan(iSmType,iRpcId); iCh++){
         fhTRpcCluWalk[iDetIndx][iSel][iCh].resize( 2 );
         for (Int_t iSide=0; iSide<2; iSide++)
         {
           fhTRpcCluWalk[iDetIndx][iSel][iCh][iSide]= new TH2F( 
           Form("cl_SmT%01d_sm%03d_rpc%03d_Ch%03d_S%01d_Sel%02d_Walk", iSmType, iSmId, iRpcId, iCh, iSide, iSel ),
           Form("Walk in SmT%01d_sm%03d_rpc%03d_Ch%03d_S%01d_Sel%02d_Walk", iSmType, iSmId, iRpcId, iCh, iSide, iSel ),
           nbClWalkBinX,fdTOTMin,fdTOTMax,nbClWalkBinY,-TSumMax,TSumMax);
         }
       }

       fhTRpcCluTOffDTLastHits[iDetIndx][iSel] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_TOff_DTLH", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu TimeZero of Rpc #%03d in Sm %03d of type %d under Selector %02d; ln(#DeltaT (ns)); TOff [ns]", iRpcId, iSmId, iSmType, iSel ),
          100, 0., 18., 99, -TSumMax,TSumMax ); 

       fhTRpcCluTotDTLastHits[iDetIndx][iSel]  =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Tot_DTLH", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu Tot of Rpc #%03d in Sm %03d of type %d under Selector %02d; ln(#DeltaT (ns)); TOT [ns]", iRpcId, iSmId, iSmType, iSel ),
          100, 0., 18., 100, fdTOTMin, fdTOTMax);

       fhTRpcCluSizeDTLastHits[iDetIndx][iSel]  =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_Size_DTLH", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu size of Rpc #%03d in Sm %03d of type %d under Selector %02d; ln(#DeltaT (ns)); size [strips]", iRpcId, iSmId, iSmType, iSel ),
          100, 0., 18., 10, 0.5, 10.5); 

       fhTRpcCluMemMulDTLastHits[iDetIndx][iSel] =  new TH2F( 
          Form("cl_SmT%01d_sm%03d_rpc%03d_Sel%02d_MemMul_DTLH", iSmType, iSmId, iRpcId, iSel ),
          Form("Clu Memorized Multiplicity of Rpc #%03d in Sm %03d of type %d under Selector %02d; ln(#DeltaT (ns)); TOff [ns]", iRpcId, iSmId, iSmType, iSel ),
          100, 0., 18., 10, 0, 10 ); 
      }
     }
   }
   // MC reference 
   fhHitsPerTracks = new TH1I( "Clus_TofHitPerTrk", "Mean Number of TofHit per Mc Track; Nb TofHits/Nb MC Tracks []",
         2000, 0.0, 20.0 );
   if( kFALSE == fDigiBdfPar->ClustUseTrackId() )
      fhPtsPerHit = new TH1I( "Clus_TofPtsPerHit", "Distribution of the Number of MCPoints associated to each TofHit; Nb MCPoint []",
         20, 0.0, 20.0 );
   if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   {
      fhTimeResSingHits = new TH1I( "Clus_TofTimeResClust", "Time resolution for TofHits containing Digis from a single MC Track; t(1st Mc Point) -tTofHit [ns]",
            10000, -25.0, 25.0 );
      fhTimeResSingHitsB = new TH2I( "Clus_TofTimeResClustB", "Time resolution for TofHits containing Digis from a single MC Track; (1st Mc Point) -tTofHit [ns]",
            5000, -25.0, 25.0,
            6, 0, 6);
      fhTimePtVsHits = new TH2I( "Clus_TofTimePtVsHit", "Time resolution for TofHits containing Digis from a single MC Track; t(1st Mc Point) -tTofHit [ns]",
            2000, 0.0, 50.0,
            2000, 0.0, 50.0 );
   }
      else
      {
         fhTimeResSingHits = new TH1I( "Clus_TofTimeResClust", "Time resolution for TofHits containing Digis from a single TofPoint; tMcPoint -tTofHit [ns]",
               10000, -25.0, 25.0 );
         fhTimeResSingHitsB = new TH2I( "Clus_TofTimeResClustB", "Time resolution for TofHits containing Digis from a single TofPoint; tMcPoint -tTofHit [ns]",
               5000, -25.0, 25.0,
               6, 0, 6);
         fhTimePtVsHits = new TH2I( "Clus_TofTimePtVsHit", "Time resolution for TofHits containing Digis from a single TofPoint; tMcPoint -tTofHit [ps]",
               2000, 0.0, 50.0,
               2000, 0.0, 50.0 );
      } // else of if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   fhClusterSize = new TH1I( "Clus_ClusterSize", "Cluster Size distribution; Cluster Size [Strips]",
         100, 0.5, 100.5 );
   fhClusterSizeType = new TH2I( "Clus_ClusterSizeType", "Cluster Size distribution in each (SM type, Rpc) pair; Cluster Size [Strips]; 10*SM Type + Rpc Index []",
         100, 0.5, 100.5,
         40*fDigiBdfPar->GetNbSmTypes(), 0.0, 40*fDigiBdfPar->GetNbSmTypes() );
   if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   {
      fhTrackMul = new TH1I( "Clus_TrackMul", "Number of MC tracks generating the cluster; MC Tracks multiplicity []",
            100, 0.5, 100.5 );
      fhClusterSizeMulti = new TH2I( "Clus_ClusterSizeMulti",
            "Cluster Size distribution as function of Number of MC tracks generating the cluster; Cluster Size [Strips]; MC tracks mul. []",
            100, 0.5, 100.5,
            100, 0.5, 100.5 );
      fhTrk1MulPos = new TH2D( "Clus_Trk1MulPos",
            "Position of Clusters with only 1 MC tracks generating the cluster; X [cm]; Y [cm]",
            1500, -750, 750,
            1000, -500, 500 );
      fhHiTrkMulPos = new TH2D( "Clus_HiTrkMulPos",
            "Position of Clusters with >1 MC tracks generating the cluster; X [cm]; Y [cm]",
            1500, -750, 750,
            1000, -500, 500 );
      fhAllTrkMulPos = new TH2D( "Clus_AllTrkMulPos",
            "Position of all clusters generating the cluster; X [cm]; Y [cm]",
            1500, -750, 750,
            1000, -500, 500 );
      fhMultiTrkProbPos = new TH2D( "Clus_MultiTrkProbPos",
            "Probability of having a cluster with multiple tracks as function of position; X [cm]; Y [cm]; Prob. [%]",
            1500, -750, 750,
            1000, -500, 500 );
   } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   
   gDirectory->cd( oldir->GetPath() ); // <= To prevent histos from being sucked in by the param file of the TRootManager!

   return kTRUE;
}

Bool_t   CbmTofTestBeamClusterizer::FillHistos()
{
 fhClustBuildTime->Fill( fStop.GetSec() - fStart.GetSec()
                           + (fStop.GetNanoSec() - fStart.GetNanoSec())/1e9 );
 Int_t iNbTofHits  = fTofHitsColl->GetEntries();
 CbmTofHit  *pHit;

 if(0<iNbTofHits){
 /* Double_t    dCluMul=0.;*/
  Bool_t      BSel[iNSel];
  Double_t    dTTrig[iNSel];
  CbmTofHit  *pTrig[iNSel]; 
  Double_t ddXdZ[iNSel];
  Double_t ddYdZ[iNSel];
  Double_t dSel2dXdYMin[iNSel];

  if(0<iNSel){                  // check software triggers

    LOG(DEBUG) <<"CbmTofTestBeamClusterizer::FillHistos() for "<<iNSel<<" triggers"
               <<", Dut "<<fDutId<<", "<<fDutSm<<", "<<fDutRpc<<Form(", 0x%08x",fDutAddr) 
	       <<", Sel "<<fSelId<<", "<<fSelSm<<", "<<fSelRpc<<Form(", 0x%08x",fSelAddr)
	       <<", Sel2 "<<fSel2Id<<", "<<fSel2Sm<<", "<<fSel2Rpc<<Form(", 0x%08x",fSel2Addr) 
               <<FairLogger::endl;
    LOG(DEBUG) <<"CbmTofTestBeamClusterizer::FillHistos: Muls: "
                     <<fviClusterMul[fDutId][fDutSm][fDutRpc]
               <<", "<<fviClusterMul[fSelId][fSelSm][fSelRpc]
               <<", "<<fviClusterMul[5][0][0]
               <<FairLogger::endl;
   // monitor multiplicities 
   Int_t iNbDet=fDigiBdfPar->GetNbDet();
   for(Int_t iDetIndx=0; iDetIndx<iNbDet; iDetIndx++){
     Int_t iDetId  = fviDetId[iDetIndx];
     Int_t iSmType = CbmTofAddress::GetSmType( iDetId );
     Int_t iSm     = CbmTofAddress::GetSmId( iDetId );
     Int_t iRpc    = CbmTofAddress::GetRpcId( iDetId );
     //LOG(INFO) << Form(" indx %d, Id 0x%08x, TSR %d %d %d", iDetIndx, iDetId, iSmType, iSm, iRpc)
     //          <<FairLogger::endl;
     if(NULL != fhRpcCluMul[iDetIndx])
     fhRpcCluMul[iDetIndx]->Fill(fviClusterMul[iSmType][iSm][iRpc]); //
   }

   fhCluMulCorDutSel->Fill(fviClusterMul[fSelId][fSelSm][fSelRpc],fviClusterMul[fDutId][fDutSm][fDutRpc]);

   // do input distributions first
   for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
   {
     pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
     if(NULL == pHit) continue;
     if (StartAnalysisTime == 0.) {
       StartAnalysisTime = pHit->GetTime();
       LOG(INFO) << "StartAnalysisTime set to "<<StartAnalysisTime<<" ns. "<<FairLogger::endl;
     }
     Int_t iDetId = (pHit->GetAddress() & DetMask);

     std::map<UInt_t,UInt_t>::iterator it=fDetIdIndexMap.find(iDetId);
     if (it == fDetIdIndexMap.end()) continue; // continue for invalid detector index
     Int_t iDetIndx=it->second;                //fDetIdIndexMap[iDetId];

     Int_t iSmType = CbmTofAddress::GetSmType( iDetId );
     Int_t iSm     = CbmTofAddress::GetSmId( iDetId );
     Int_t iRpc    = CbmTofAddress::GetRpcId( iDetId );
     Int_t iCh     = CbmTofAddress::GetChannelId( pHit->GetAddress() );

     Double_t dTimeAna=(pHit->GetTime() - StartAnalysisTime)/1.E9;
     LOG(DEBUG)<<"TimeAna"<<StartAnalysisTime<<", "<< pHit->GetTime()<<", "<<dTimeAna<<FairLogger::endl;
     fhRpcCluRate[iDetIndx]->Fill(dTimeAna);       
     
     if(fdMemoryTime>0. && fvLastHits[iSmType][iSm][iRpc][iCh].size()==0)
       LOG(FATAL)<<Form(" <E> hit not stored in memory for TSRC %d%d%d%d",
			  iSmType,iSm,iRpc,iCh)
		   <<FairLogger::endl;

     CheckLHMemory();

     if(fvLastHits[iSmType][iSm][iRpc][iCh].size()>1) { // check for outdated hits 
       //std::list<CbmTofHit *>::iterator it0=fvLastHits[iSmType][iSm][iRpc][iCh].begin();
       //std::list<CbmTofHit *>::iterator itL=fvLastHits[iSmType][iSm][iRpc][iCh].end();
       //CbmTofHit* pH0 = *it0;
       //CbmTofHit* pHL = *(--itL);
       CbmTofHit* pH0 = fvLastHits[iSmType][iSm][iRpc][iCh].front();
       CbmTofHit* pHL = fvLastHits[iSmType][iSm][iRpc][iCh].back();
       if(pH0->GetTime() > pHL->GetTime())
	 LOG(WARNING)<<Form("Invalid time ordering in ev %8.0f in list of size %lu for TSRC %d%d%d%d: Delta t %f  ",
			  fdEvent,
			  fvLastHits[iSmType][iSm][iRpc][iCh].size(),
			  iSmType,iSm,iRpc,iCh,
			  pHL->GetTime() - pH0->GetTime()
			  )
		   <<FairLogger::endl;

       //       while( (*((std::list<CbmTofHit *>::iterator) fvLastHits[iSmType][iSm][iRpc][iCh].begin()))->GetTime()+fdMemoryTime < pHit->GetTime()
       while( //fvLastHits[iSmType][iSm][iRpc][iCh].size() > 2. ||
	      fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetTime()+fdMemoryTime < pHit->GetTime()
	     )
	      
       {
	 LOG(DEBUG) << " pop from list size "<< fvLastHits[iSmType][iSm][iRpc][iCh].size()
		    <<Form(" outdated hits for ev %8.0f in TSRC %d%d%d%d",fdEvent,iSmType,iSm,iRpc,iCh)
		    <<Form(" with tHit - tLast %f ", pHit->GetTime()-
			   fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetTime())
			   //(*((std::list<CbmTofHit *>::iterator) fvLastHits[iSmType][iSm][iRpc][iCh].begin()))->GetTime())
		    << FairLogger::endl;
	 if( fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetAddress() != pHit->GetAddress() )
	 LOG(FATAL)<<Form("Inconsistent address in list of size %lu for TSRC %d%d%d%d: 0x%08x, time  %f",
			  fvLastHits[iSmType][iSm][iRpc][iCh].size(),
			  iSmType,iSm,iRpc,iCh,
			  fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetAddress(),
			  fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetTime())
		   <<FairLogger::endl;
	 fvLastHits[iSmType][iSm][iRpc][iCh].front()->Delete();
	 fvLastHits[iSmType][iSm][iRpc][iCh].pop_front();
       }     
     }

 // plot remaining time difference to previous hits 
     if(fvLastHits[iSmType][iSm][iRpc][iCh].size()>1){     // check for previous hits in memory time interval
       std::list<CbmTofHit *>::iterator itL=fvLastHits[iSmType][iSm][iRpc][iCh].end(); 
       itL--; 
       for(Int_t iH=0; iH<fvLastHits[iSmType][iSm][iRpc][iCh].size()-1; iH++){
	 itL--; 
	 fhRpcDTLastHits[iDetIndx]->Fill(TMath::Log(pHit->GetTime()-(*itL)->GetTime()));
	}      
     }
   } // iHitInd loop end 

   // do reference first 
   dTRef = dDoubleMax;
   fTRefHits=0;
   Int_t iBeamRefMul=0;
   Int_t iBeamAddRefMul=0;
   for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
   {
      pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
      if (NULL==pHit) continue;
      Int_t iDetId = (pHit->GetAddress() & DetMask);

      if( fiBeamRefAddr == iDetId ){
	 if(fviClusterMul[fiBeamRefType][fiBeamRefSm][fiBeamRefDet]>fiBeamRefMulMax) break;
	 // Check Tot
         CbmMatch* digiMatch=(CbmMatch *)fTofDigiMatchColl->At(iHitInd);
         Double_t TotSum=0.;
         for (Int_t iLink=0; iLink<digiMatch->GetNofLinks(); iLink+=2){  // loop over digis
           CbmLink L0 = digiMatch->GetLink(iLink);   //vDigish.at(ivDigInd);
           Int_t iDigInd0=L0.GetIndex(); 
           if (iDigInd0 < fTofCalDigisColl->GetEntries()){
            CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofCalDigisColl->At(iDigInd0));
            TotSum += pDig0->GetTot();
           }
         }
	 TotSum /= (0.5 * digiMatch->GetNofLinks());
	 if( TotSum > fhRpcCluTot[iIndexDut]->GetYaxis()->GetXmax()) continue; 

          fTRefHits=1;
	  if(pHit->GetTime() < dTRef)  
          {
	    dTRef = pHit->GetTime();
          }
         iBeamRefMul++;
     }else{ //additional reference type multiplicity
	if(fiBeamRefType == CbmTofAddress::GetSmType( iDetId ) ) iBeamAddRefMul++;
     }
   }
   LOG(DEBUG) <<"CbmTofTestBeamClusterizer::FillHistos: BRefMul: "
              <<iBeamRefMul<<", "<<iBeamAddRefMul
              <<FairLogger::endl;

   if (iBeamRefMul == 0) return kFALSE;  // don't fill histos without reference time
   if (iBeamAddRefMul<fiBeamAddRefMul) return kFALSE;  // ask for confirmation by other beam counters

   for (Int_t iSel=0; iSel<iNSel; iSel++){
     BSel[iSel]=kFALSE;
     pTrig[iSel]=NULL;
     Int_t iDutMul=0;
     Int_t iRefMul=0;
     Int_t iR0=0;
     Int_t iRl=0;
     ddXdZ[iSel]=0.;
     ddYdZ[iSel]=0.;
     dSel2dXdYMin[iSel]=1.E300;

     switch(iSel) {
       case 0 :         //  Detector under Test (Dut) && Diamonds,BeamRef
	 iRl=fviClusterMul[fDutId][fDutSm].size();
	 if(fDutRpc>-1) {iR0=fDutRpc; iRl=fDutRpc+1;}
	 for(Int_t iRpc=iR0; iRpc<iRl; iRpc++) iDutMul += fviClusterMul[fDutId][fDutSm][iRpc];
          LOG(DEBUG)<<"CbmTofTestBeamClusterizer::FillHistos(): selector 0: DutMul "
		    <<fviClusterMul[fDutId][fDutSm][fDutRpc]<<", "<<iDutMul
		    <<", BRefMul "<<iBeamRefMul<<" TRef: "<<dTRef
		    <<", BeamAddRefMul "<<iBeamAddRefMul<<", "<<fiBeamAddRefMul
		    <<FairLogger::endl;
         if(   iDutMul>0  ) {                           
             LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos(): Found selector 0"<<FairLogger::endl;
             dTTrig[iSel]=dDoubleMax;
             for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
             {
              pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
              if(NULL == pHit) continue;

              Int_t iDetId = (pHit->GetAddress() & DetMask);
              //if( fDutId == CbmTofAddress::GetSmType( iDetId ))
              if( fDutAddr == iDetId)
              {
                 if(pHit->GetTime() < dTTrig[iSel])
                 {
                  dTTrig[iSel] = pHit->GetTime();
                  pTrig[iSel]  = pHit;
                  BSel[iSel]=kTRUE;
                 }         
              }
             }
         } 
         break; 
       case 1 :         // MRef & BRef 
	 iRl=fviClusterMul[fSelId][fSelSm].size();
	 if(fSelRpc>-1) {iR0=fSelRpc; iRl=fSelRpc+1;}
	 for(Int_t iRpc=iR0; iRpc<iRl; iRpc++) iRefMul += fviClusterMul[fSelId][fSelSm][iRpc];
          LOG(DEBUG)<<"CbmTofTestBeamClusterizer::FillHistos(): selector 1: RefMul "
		    <<fviClusterMul[fSelId][fSelSm][fSelRpc]<<", "<<iRefMul
		    <<", BRefMul "<<iBeamRefMul<<FairLogger::endl;
         if(   iRefMul>0  ) {                        
             LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos(): Found selector 1"<<FairLogger::endl;
             dTTrig[iSel]=dDoubleMax;
             for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
             {
              pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
              if(NULL == pHit) continue;

              Int_t iDetId = (pHit->GetAddress() & DetMask);
              if( fSelAddr == iDetId )
              {
                  if(pHit->GetTime() < dTTrig[iSel])
                  {
                    dTTrig[iSel] = pHit->GetTime();
                    pTrig[iSel]  = pHit;
                    BSel[iSel]=kTRUE;
                  }           
              }
             }
         }
         break;

       default :
       LOG(INFO)<<"CbmTofTestBeamClusterizer::FillHistos: selection not implemented "<<iSel<<FairLogger::endl;
         ;
     }  // switch end 
     if(fTRefMode>10){
       dTTrig[iSel]=dTRef;
     }
   } // iSel - loop end 

   if (fSel2Id > 0 ) {  // confirm selector by independent match 
     for (Int_t iSel=0; iSel<iNSel; iSel++){
       if (BSel[iSel]){
         BSel[iSel]=kFALSE;
         for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
         {
           pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
           if(NULL == pHit) continue;
           Int_t iDetId = (pHit->GetAddress() & DetMask);
           if( fSel2Addr == iDetId )
           {
	     Double_t dzscal=1.;
	     if(fEnableMatchPosScaling) dzscal=pHit->GetZ()/pTrig[iSel]->GetZ();
	     Double_t dSEl2dXdz=(pHit->GetX()-pTrig[iSel]->GetX())/(pHit->GetZ()-pTrig[iSel]->GetZ());
	     Double_t dSEl2dYdz=(pHit->GetY()-pTrig[iSel]->GetY())/(pHit->GetZ()-pTrig[iSel]->GetZ());

             if (TMath::Sqrt(TMath::Power(pHit->GetX()-dzscal*pTrig[iSel]->GetX(),2.)
                            +TMath::Power(pHit->GetY()-dzscal*pTrig[iSel]->GetY(),2.))<fdCaldXdYMax)
             {
               BSel[iSel]=kTRUE;
	       Double_t dX2Y2=TMath::Sqrt(dSEl2dXdz*dSEl2dXdz+dSEl2dYdz*dSEl2dYdz);
	       if(dX2Y2<dSel2dXdYMin[iSel]){
		 ddXdZ[iSel]=dSEl2dXdz;
		 ddYdZ[iSel]=dSEl2dYdz;
		 dSel2dXdYMin[iSel]=dX2Y2;
	       }
               break;
             }
           }
         }
       } // BSel condition end 
     }   // iSel lopp end 
   }     // Sel2Id condition end
/*
    // find the best dTRef
    fTRefHits=0;
    dTRef=0.;     // invalidate old value 
    Double_t dRefChi2=dDoubleMax;
    for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
    {
      pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
      if (NULL==pHit) continue;
      Int_t iDetId = (pHit->GetAddress() & DetMask);

      if( fiBeamRefType == CbmTofAddress::GetSmType( iDetId )){
       if(fiBeamRefSm   == CbmTofAddress::GetSmId( iDetId ))
       {
	 Double_t dDT2=0.;
	 Double_t dNT=0.;
	 for (Int_t iSel=0; iSel<iNSel; iSel++){
	   if(BSel[iSel]){
	     dDT2 += TMath::Power(pHit->GetTime()-dTTrig[iSel],2);
	     dNT++;
	   }
	 }
	 if( dNT > 0)
	 if( dDT2/dNT < dRefChi2 )  
         {
	    fTRefHits=1;
	    dTRef = pHit->GetTime();
	    dRefChi2 = dDT2/dNT;
         }
       }
      }
    }
*/
   UInt_t uTriggerPattern=1;
   if(NULL != fTrbHeader) uTriggerPattern=fTrbHeader->GetTriggerPattern();
   for (Int_t iSel=0; iSel<iNSel; iSel++){
     if(BSel[iSel]){
       if (dTRef!=0. && fTRefHits>0) {
	 for(UInt_t uChannel = 0; uChannel < 16; uChannel++)
	 {
	   if( uTriggerPattern & (0x1 << uChannel) )
	   {	  
	     fhSeldT[iSel]->Fill(dTTrig[iSel]-dTRef,uChannel);   
	   }
	 } 
       }
     }
   }
 }  // 0<iNSel software triffer check end 
 
 for( Int_t iHitInd = 0; iHitInd < iNbTofHits; iHitInd++)
 {
   pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
   if(NULL == pHit) continue;

   Int_t iDetId = (pHit->GetAddress() & DetMask);
   std::map<UInt_t,UInt_t>::iterator it=fDetIdIndexMap.find(iDetId);
   if (it == fDetIdIndexMap.end()) continue; // continue for invalid detector index
   Int_t iDetIndx=it->second;                //fDetIdIndexMap[iDetId];

   Int_t iSmType = CbmTofAddress::GetSmType( iDetId );
   Int_t iSm     = CbmTofAddress::GetSmId( iDetId );
   Int_t iRpc    = CbmTofAddress::GetRpcId( iDetId );
   Int_t iNbRpc  = fDigiBdfPar->GetNbRpc( iSmType );
   if(-1<fviClusterMul[iSmType][iSm][iRpc]){         
     for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
       Double_t w=fviClusterMul[iSmType][iSm][iRpc];
       if (w==0.) w=1.;
       else       w=1./w;
       fhTRpcCluMul[iDetIndx][iSel]->Fill(fviClusterMul[iSmType][iSm][iRpc],w);
     }         
   }

   if(fviClusterMul[iSmType][iSm][iRpc] > fiCluMulMax) break; // skip this event 

   Int_t iChId = pHit->GetAddress();
   fChannelInfo = fDigiPar->GetCell( iChId );
   Int_t iCh = CbmTofAddress::GetChannelId(iChId);
   if(NULL == fChannelInfo){
     LOG(ERROR) << "Invalid Channel Pointer for ChId "
                << Form(" 0x%08x ",iChId)<<", Ch "<<iCh
                << FairLogger::endl;
     continue;
   }
   /*TGeoNode *fNode=*/        // prepare global->local trafo
   gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());

   LOG(DEBUG1)<<"Hit info: "
              <<Form(" 0x%08x %d %f %f %f %f %f %d",iChId,iCh,
                pHit->GetX(),pHit->GetY(),pHit->GetTime(),fChannelInfo->GetX(),fChannelInfo->GetY(), iHitInd )
              <<FairLogger::endl;

   Double_t hitpos[3];
   hitpos[0]=pHit->GetX();
   hitpos[1]=pHit->GetY();
   hitpos[2]=pHit->GetZ();
   Double_t hitpos_local[3];
   TGeoNode* cNode= gGeoManager->GetCurrentNode();
   gGeoManager->MasterToLocal(hitpos, hitpos_local);
   LOG(DEBUG1)<< Form(" MasterToLocal for node %p: (%6.1f,%6.1f,%6.1f) ->(%6.1f,%6.1f,%6.1f)", 
                 cNode, hitpos[0], hitpos[1], hitpos[2], 
                 hitpos_local[0], hitpos_local[1], hitpos_local[2])
              <<FairLogger::endl;

   fhRpcCluPosition[iDetIndx]->Fill((Double_t)iCh,hitpos_local[1]); //pHit->GetY()-fChannelInfo->GetY());
   fhSmCluPosition[iSmType]->Fill((Double_t)(iSm*iNbRpc+iRpc),hitpos_local[1]);

   for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
     fhTRpcCluPosition[iDetIndx][iSel]->Fill((Double_t)iCh,hitpos_local[1]);  //pHit->GetY()-fChannelInfo->GetY());
     fhTSmCluPosition[iSmType][iSel]->Fill((Double_t)(iSm*iNbRpc+iRpc),hitpos_local[1]);
   }

   if(TMath::Abs(hitpos_local[1])>fChannelInfo->GetSizey()*fPosYMaxScal) continue;
     LOG(DEBUG1)<<" TofDigiMatchColl entries:"
                <<fTofDigiMatchColl->GetEntries()
                <<FairLogger::endl;

     if(iHitInd>fTofDigiMatchColl->GetEntries()){
       LOG(ERROR)<<" Inconsistent DigiMatches for Hitind "
                 <<iHitInd<<", TClonesArraySize: "<<fTofDigiMatchColl->GetEntries()
                <<FairLogger::endl;
     }

     CbmMatch* digiMatch=(CbmMatch *)fTofDigiMatchColl->At( iHitInd );
     LOG(DEBUG1)<<" got "
                <<digiMatch->GetNofLinks()<< " matches for iCh "<<iCh<<" at iHitInd "<<iHitInd
                <<FairLogger::endl;

     fhRpcCluSize[iDetIndx]->Fill((Double_t)iCh,digiMatch->GetNofLinks()/2.);

     for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
       fhTRpcCluSize[iDetIndx][iSel]->Fill((Double_t)iCh,digiMatch->GetNofLinks()/2.);
       if(fvLastHits[iSmType][iSm][iRpc][iCh].size()>1){     // check for previous hits in memory time interval 
	 std::list<CbmTofHit *>::iterator itL=fvLastHits[iSmType][iSm][iRpc][iCh].end(); 
	 itL--; 
	 for(Int_t iH=0; iH<fvLastHits[iSmType][iSm][iRpc][iCh].size()-1; iH++){
	   itL--; 
	   fhTRpcCluSizeDTLastHits[iDetIndx][iSel]->Fill(TMath::Log(pHit->GetTime()-(*itL)->GetTime()),
							 digiMatch->GetNofLinks()/2.);
	 }
       }
     }

     Double_t TotSum=0.;
     for (Int_t iLink=0; iLink<digiMatch->GetNofLinks(); iLink++){  // loop over digis
       CbmLink L0 = digiMatch->GetLink(iLink);   //vDigish.at(ivDigInd);
       Int_t iDigInd0=L0.GetIndex(); 
       if (iDigInd0 < fTofCalDigisColl->GetEntries()){
         CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofCalDigisColl->At(iDigInd0));
         TotSum += pDig0->GetTot();
       }
     }
     Double_t dMeanTimeSquared=0.;
     Double_t dNstrips=0.;
         
     Double_t dDelTof=0.;
     Double_t dTcor[iNSel];
     Double_t dTTcor[iNSel];        
     Double_t dzscal=1.;
     //Double_t dDist=0.;
    
     for (Int_t iLink=0; iLink<digiMatch->GetNofLinks(); iLink+=2) {  // loop over digis
       CbmLink L0 = digiMatch->GetLink(iLink);   //vDigish.at(ivDigInd);
       Int_t iDigInd0=L0.GetIndex(); 
       Int_t iDigInd1=(digiMatch->GetLink(iLink+1)).GetIndex(); //vDigish.at(ivDigInd+1);
       //LOG(DEBUG1)<<" " << iDigInd0<<", "<<iDigInd1<<FairLogger::endl;

       if (iDigInd0 < fTofCalDigisColl->GetEntries() && iDigInd1 < fTofCalDigisColl->GetEntries()){
         CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofCalDigisColl->At(iDigInd0));
         CbmTofDigiExp *pDig1 = (CbmTofDigiExp*) (fTofCalDigisColl->At(iDigInd1));
         if((Int_t)pDig0->GetType()!=iSmType){
           LOG(ERROR)<<Form(" Wrong Digi SmType for Tofhit %d in iDetIndx %d, Ch %d with %3.0f strips at Indx %d, %d",
                            iHitInd,iDetIndx,iCh,dNstrips,iDigInd0,iDigInd1)
                     << FairLogger::endl; 
         }
         LOG(DEBUG1)<<" fhRpcCluTot:  Digi 0 "<<iDigInd0<<": Ch "<<pDig0->GetChannel()<<", Side "<<pDig0->GetSide()
		    <<", StripSide "<<(Double_t)iCh*2.+pDig0->GetSide() 
                    <<" Digi 1 "<<iDigInd1<<": Ch "<<pDig1->GetChannel()<<", Side "<<pDig1->GetSide()
		    <<", StripSide "<<(Double_t)iCh*2.+pDig1->GetSide() 
		    <<", Tot0 " << pDig0->GetTot() <<", Tot1 "<<pDig1->GetTot()<<FairLogger::endl;

         fhRpcCluTot[iDetIndx]->Fill(pDig0->GetChannel()*2.+pDig0->GetSide(),pDig0->GetTot());
         fhRpcCluTot[iDetIndx]->Fill(pDig1->GetChannel()*2.+pDig1->GetSide(),pDig1->GetTot());

         Int_t iCh0=pDig0->GetChannel();
         Int_t iCh1=pDig1->GetChannel();
         Int_t iS0=pDig0->GetSide();
         Int_t iS1=pDig1->GetSide();
         if(iCh0 != iCh1 || iS0==iS1){
           LOG(ERROR)<<Form(" MT2 for Tofhit %d in iDetIndx %d, Ch %d from %3.0f strips: ",iHitInd,iDetIndx,iCh,dNstrips)
                     <<Form(" Dig0: Ch %d, Side %d, T: %6.1f ",iCh0,iS0,pDig0->GetTime())
                     <<Form(" Dig1: Ch %d, Side %d, T: %6.1f ",iCh1,iS1,pDig1->GetTime())
                     << FairLogger::endl; 
           continue;
         }
 
         if(0>iCh0 ||  fDigiBdfPar->GetNbChan( iSmType, iRpc )<=iCh0) {
           LOG(ERROR)<<Form(" Wrong Digi for Tofhit %d in iDetIndx %d, Ch %d at Indx %d, %d from %3.0f strips:  %d, %d, %d, %d",
                            iHitInd,iDetIndx,iCh,iDigInd0,iDigInd1,dNstrips,iCh0,iCh1,iS0,iS1)
                     << FairLogger::endl; 
           continue;
         }

         if (digiMatch->GetNofLinks()>2 ) //&& digiMatch->GetNofLinks()<8 ) // FIXME: hardwired limits on CluSize
         {
           dNstrips+=1.;
           dMeanTimeSquared += TMath::Power(0.5*(pDig0->GetTime()+pDig1->GetTime())-pHit->GetTime(),2);
	   //             fhRpcCluAvWalk[iDetIndx]->Fill(0.5*(pDig0->GetTot()+pDig1->GetTot()),
	   //                        0.5*(pDig0->GetTime()+pDig1->GetTime())-pHit->GetTime());

           fhRpcCluAvLnWalk[iDetIndx]->Fill(TMath::Log(0.5*(pDig0->GetTot()+pDig1->GetTot())),
                                       0.5*(pDig0->GetTime()+pDig1->GetTime())-pHit->GetTime());

           Double_t dTotWeigth=(pDig0->GetTot()+pDig1->GetTot())/TotSum;
           Double_t dCorWeigth=1.-dTotWeigth;

           fhRpcCluDelTOff[iDetIndx]->Fill(pDig0->GetChannel(),
                                           dCorWeigth*(0.5*(pDig0->GetTime()+pDig1->GetTime())-pHit->GetTime()));

           Double_t dDelPos=0.5*(pDig0->GetTime()-pDig1->GetTime())*fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc);
           if(0==pDig0->GetSide()) dDelPos *= -1.;
           fhRpcCluDelPos[iDetIndx]->Fill(pDig0->GetChannel(),dCorWeigth*(dDelPos-hitpos_local[1]));

           fhRpcCluWalk[iDetIndx][iCh0][iS0]->Fill(pDig0->GetTot(),pDig0->GetTime()-(pHit->GetTime()
			 -(1.-2.*pDig0->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)));
              
           fhRpcCluWalk[iDetIndx][iCh1][iS1]->Fill(pDig1->GetTot(),pDig1->GetTime()-(pHit->GetTime()
                         -(1.-2.*pDig1->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)));

           fhRpcCluAvWalk[iDetIndx]->Fill(pDig0->GetTot(),pDig0->GetTime()-(pHit->GetTime()
                         -(1.-2.*pDig0->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)));
	   fhRpcCluAvWalk[iDetIndx]->Fill(pDig1->GetTot(),pDig1->GetTime()-(pHit->GetTime()
                         -(1.-2.*pDig1->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)));
         }  // end of Clustersize > 1 condition 

         LOG(DEBUG1)<<" fhTRpcCluTot: Digi 0 "<<iDigInd0<<": Ch "<<pDig0->GetChannel()<<", Side "<<pDig0->GetSide()
		    <<", StripSide "<<(Double_t)iCh*2.+pDig0->GetSide() 
                    <<" Digi 1 "<<iDigInd1<<": Ch "<<pDig1->GetChannel()<<", Side "<<pDig1->GetSide()
		    <<", StripSide "<<(Double_t)iCh*2.+pDig1->GetSide() 
		    <<FairLogger::endl;

         for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]) {
           if (NULL == pHit  || NULL == pTrig[iSel]) {
             LOG(INFO)<<" invalid pHit, iSel "
                      <<iSel<<", iDetIndx "<< iDetIndx
                      <<FairLogger::endl;
             break;
           }
           fhTRpcCluTot[iDetIndx][iSel]->Fill(pDig0->GetChannel()*2.+pDig0->GetSide(),pDig0->GetTot());
           fhTRpcCluTot[iDetIndx][iSel]->Fill(pDig1->GetChannel()*2.+pDig1->GetSide(),pDig1->GetTot());
	   if(fvLastHits[iSmType][iSm][iRpc][iCh].size()>1){     // check for previous hits in memory time interval 
	     std::list<CbmTofHit *>::iterator itL=fvLastHits[iSmType][iSm][iRpc][iCh].end(); 
	     itL--; 
	     for(Int_t iH=0; iH<fvLastHits[iSmType][iSm][iRpc][iCh].size()-1; iH++){
	       itL--; 
	       fhTRpcCluTotDTLastHits[iDetIndx][iSel]->Fill(TMath::Log(pHit->GetTime()-(*itL)->GetTime()), 
		                                                pDig0->GetTot());
	       fhTRpcCluTotDTLastHits[iDetIndx][iSel]->Fill(TMath::Log(pHit->GetTime()-(*itL)->GetTime()), 
		                                                pDig1->GetTot());
	     }      
	   }
           if(iLink==0) {      // Fill histo only once (for 1. digi entry)
	     if(fEnableMatchPosScaling) dzscal=pHit->GetZ()/pTrig[iSel]->GetZ();
             fhTRpcCludXdY[iDetIndx][iSel]->Fill(pHit->GetX()-dzscal*pTrig[iSel]->GetX(),
						 pHit->GetY()-dzscal*pTrig[iSel]->GetY());
           }
	   //// look for geometrical match  with selector hit
           if(  iSmType==fiBeamRefType      // to get entries in diamond/BeamRef histos  
             || TMath::Sqrt(TMath::Power(pHit->GetX()-dzscal*pTrig[iSel]->GetX(),2.)
                           +TMath::Power(pHit->GetY()-dzscal*pTrig[iSel]->GetY(),2.))<fdCaldXdYMax)
           {
	     if ( !fEnableMatchPosScaling && dSel2dXdYMin[iSel]<1.E300)
               if( TMath::Sqrt(TMath::Power(pHit->GetX()-(pTrig[iSel]->GetX()+ddXdZ[iSel]*(pHit->GetZ()-(pTrig[iSel]->GetZ()))),2.)
			      +TMath::Power(pHit->GetY()-(pTrig[iSel]->GetY()+ddYdZ[iSel]*(pHit->GetZ()-(pTrig[iSel]->GetZ()))),2.))
		    > 0.5*fdCaldXdYMax) continue;  // refine position selection cut in cosmic measurement
	     dTcor[iSel]=0.;                       // precaution
             if (dTRef !=0. && TMath::Abs(dTRef-dTTrig[iSel])<fdDelTofMax) { // correct times for DelTof - velocity spread 
               if(iLink==0){   // do calculations only once (at 1. digi entry) // interpolate! 
		 // calculate spatial distance to trigger hit
		 /*
		 dDist=TMath::Sqrt(TMath::Power(pHit->GetX()-pTrig[iSel]->GetX(),2.)
                                  +TMath::Power(pHit->GetY()-pTrig[iSel]->GetY(),2.)
	       		          +TMath::Power(pHit->GetZ()-pTrig[iSel]->GetZ(),2.));
		 */
		 // determine correction value 
                 if(fiBeamRefAddr  != iDetId) // do not do this for reference counter itself
		 {
		   Double_t dTentry=dTRef-dTTrig[iSel]+fdDelTofMax;
		   Int_t iBx = dTentry/2./fdDelTofMax*nbClDelTofBinX;
		   if(iBx<0) iBx=0;
		   if(iBx>nbClDelTofBinX-1) iBx=nbClDelTofBinX-1;
		   Double_t dBinWidth=2.*fdDelTofMax/nbClDelTofBinX;
		   Double_t dDTentry=dTentry-((Double_t)iBx)*dBinWidth;
		   Int_t iBx1=0;
		   dDTentry < 0 ? iBx1=iBx-1 : iBx1=iBx+1;
		   Double_t w0=1.-TMath::Abs(dDTentry)/dBinWidth;
		   Double_t w1=1.-w0;
		   if(iBx1<0) iBx1=0;
		   if(iBx1>nbClDelTofBinX-1) iBx1=nbClDelTofBinX-1;
		   dDelTof=fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel]*w0 + fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx1][iSel]*w1;
		   //dDelTof *= dDist; // has to be consistent with fhTRpcCluDelTof filling
		   LOG(DEBUG1)<<Form(" DelTof for SmT %d, Sm %d, R %d, T %d, dTRef %6.1f, Bx %d, Bx1 %d, DTe %6.1f -> DelT %6.1f",
				     iSmType, iSm, iRpc, iSel, dTRef-dTTrig[iSel], iBx, iBx1, dDTentry, dDelTof)
			      <<FairLogger::endl;
		 }	
		 dTTcor[iSel]=dDelTof;
                 dTcor[iSel]=pHit->GetTime()-dDelTof-dTTrig[iSel];  
		 Double_t dAvTot=0.5*(pDig0->GetTot()+pDig1->GetTot());
	       } // if(iLink==0)

               LOG(DEBUG)<<Form(" TRpcCluWalk for Ev %d, Link %d(%d), Sel %d, TSR %d%d%d, Ch %d,%d, S %d,%d T %f, DelTof %6.1f, W-ent:  %6.0f,%6.0f",
			 fiNevtBuild, iLink,(Int_t)digiMatch->GetNofLinks(),iSel,iSmType,iSm,iRpc,
				iCh0, iCh1, iS0, iS1, dTTrig[iSel], dDelTof,
				fhTRpcCluWalk[iDetIndx][iSel][iCh0][iS0]->GetEntries(),
				fhTRpcCluWalk[iDetIndx][iSel][iCh1][iS1]->GetEntries())
                         <<FairLogger::endl;

	       if( fhTRpcCluWalk[iDetIndx][iSel][iCh0][iS0]->GetEntries() != 
		   fhTRpcCluWalk[iDetIndx][iSel][iCh1][iS1]->GetEntries() )
		 LOG(ERROR) << Form(" Inconsistent walk histograms -> debugging necessary ... for %d, %d, %d, %d, %d, %d, %d ",
				      fiNevtBuild, iDetIndx, iSel, iCh0,  iCh1, iS0, iS1)
			    << FairLogger::endl;

	       LOG(DEBUG1)<<Form(" TRpcCluWalk values side %d: %f, %f, side %d:  %f, %f ",
				 iS0,pDig0->GetTot(),pDig0->GetTime()
				 +((1.-2.*pDig0->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc))-dTTcor[iSel]-dTTrig[iSel],
				 iS1,pDig1->GetTot(),pDig1->GetTime()
				 +((1.-2.*pDig1->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc))-dTTcor[iSel]-dTTrig[iSel])
			  <<FairLogger::endl;

	       fhTRpcCluWalk[iDetIndx][iSel][iCh0][iS0]->Fill(pDig0->GetTot(),pDig0->GetTime()
		 +((1.-2.*pDig0->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc))-dTTcor[iSel]-dTTrig[iSel]);
	       fhTRpcCluWalk[iDetIndx][iSel][iCh1][iS1]->Fill(pDig1->GetTot(),pDig1->GetTime()
		 +((1.-2.*pDig1->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc))-dTTcor[iSel]-dTTrig[iSel]);

	       fhTRpcCluAvWalk[iDetIndx][iSel]->Fill(pDig0->GetTot(),pDig0->GetTime()
	         +((1.-2.*pDig0->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc))-dTTcor[iSel]-dTTrig[iSel]);
	       fhTRpcCluAvWalk[iDetIndx][iSel]->Fill(pDig1->GetTot(),pDig1->GetTime()
               	 +((1.-2.*pDig1->GetSide())*hitpos_local[1]/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc))-dTTcor[iSel]-dTTrig[iSel]);

	       if(iLink==0){          // Fill histo only once (for 1. digi entry)
		 //fhTRpcCluDelTof[iDetIndx][iSel]->Fill(dTRef-dTTrig[iSel],dTcor[iSel]/dDist);
		 fhTRpcCluDelTof[iDetIndx][iSel]->Fill(dTRef-dTTrig[iSel],dTcor[iSel]);
		 fhTSmCluTOff[iSmType][iSel]->Fill((Double_t)(iSm*iNbRpc+iRpc),dTcor[iSel]);
		 fhTSmCluTRun[iSmType][iSel]->Fill(fdEvent,dTcor[iSel]);
		 if( iDetId != (pTrig[iSel]->GetAddress() & DetMask) ){ // transform matched hit-pair back into detector frame
		   hitpos[0]=pHit->GetX()-dzscal*pTrig[iSel]->GetX() + fChannelInfo->GetX();
		   hitpos[1]=pHit->GetY()-dzscal*pTrig[iSel]->GetY() + fChannelInfo->GetY();
		   hitpos[2]=pHit->GetZ();
		   gGeoManager->MasterToLocal(hitpos, hitpos_local); //  transform into local frame
		   fhRpcCluDelMatPos[iDetIndx]->Fill((Double_t)iCh,hitpos_local[1]);
		   fhRpcCluDelMatTOff[iDetIndx]->Fill((Double_t)iCh,pHit->GetTime()-dTTcor[iSel]-dTTrig[iSel]);
		 }
	       }    // iLink==0 condition end
	    }      // position condition end  
          } // Match condition end
        }  // closing of selector loop
     } 
     else {
             LOG(ERROR)<<"CbmTofTestBeamClusterizer::FillHistos: invalid digi index "<<iDetIndx
                       <<" digi0,1"<<iDigInd0<<", "<<iDigInd1<<" - max:"
                       << fTofCalDigisColl->GetEntries() 
               //                       << " in event " << XXX 
                       << FairLogger::endl;  
       }
     } // iLink digi loop end;

     if (1<dNstrips){
       //           Double_t dVar=dMeanTimeSquared/dNstrips - TMath::Power(pHit->GetTime(),2);
       Double_t dVar=dMeanTimeSquared/(dNstrips-1);
       //if(dVar<0.) dVar=0.;
       Double_t dTrms=TMath::Sqrt(dVar);
       LOG(DEBUG)<<Form(" Trms for Tofhit %d in iDetIndx %d, Ch %d from %3.0f strips: %6.3f ns",
                             iHitInd,iDetIndx,iCh,dNstrips,dTrms)
                 << FairLogger::endl;  
       fhRpcCluTrms[iDetIndx]->Fill((Double_t)iCh,dTrms);
       pHit->SetTimeError(dTrms);
     }
	 
     LOG(DEBUG1)<<" Fill Time of iDetIndx "<<iDetIndx<<", hitAddr "
		<<Form(" %08x, y = %5.2f",pHit->GetAddress(),hitpos_local[1])
		<<" for |y| <"
                <<fhRpcCluPosition[iDetIndx]->GetYaxis()->GetXmax()
                <<FairLogger::endl;
     if(TMath::Abs(hitpos_local[1])< (fhRpcCluPosition[iDetIndx]->GetYaxis()->GetXmax())){
       if(dTRef !=0. && fTRefHits==1){ 
         fhRpcCluTOff[iDetIndx]->Fill((Double_t)iCh,pHit->GetTime()-dTRef);
         fhSmCluTOff[iSmType]->Fill((Double_t)(iSm*iNbRpc+iRpc),pHit->GetTime()-dTRef);

         for (Int_t iSel=0; iSel<iNSel; iSel++) if(BSel[iSel]){
           LOG(DEBUG1)<<" TRpcCluTOff "<< iDetIndx <<", Sel "<< iSel
                      <<Form(", Dt %7.3f, LHsize %lu ",
			     pHit->GetTime()-dTTrig[iSel],fvLastHits[iSmType][iSm][iRpc][iCh].size());
	   if(fvLastHits[iSmType][iSm][iRpc][iCh].size()>1) {     // check for previous hits in memory time interval 
	     std::list<CbmTofHit *>::iterator itL=fvLastHits[iSmType][iSm][iRpc][iCh].end(); 
	     itL--; 
	     for(Int_t iH=0; iH<fvLastHits[iSmType][iSm][iRpc][iCh].size()-1; iH++){
	       itL--; 	   
	       LOG(DEBUG1)<<Form(" %f,",pHit->GetTime()-(*itL)->GetTime());
	     }
	    }
           LOG(DEBUG1)<<FairLogger::endl;
	   // fill Time Offset histograms without velocity spread (DelTof) correction 
           fhTRpcCluTOff[iDetIndx][iSel]->Fill((Double_t)iCh,pHit->GetTime()-dTTrig[iSel]);// -dTTcor[iSel] only valid for matches
	   if(fvLastHits[iSmType][iSm][iRpc][iCh].size()>1){     // check for previous hits in memory time interval 
	     std::list<CbmTofHit *>::iterator itL=fvLastHits[iSmType][iSm][iRpc][iCh].end(); 
	     itL--; 
	     for(Int_t iH=0; iH<1; iH++){  // use only last hit
	       //  for(Int_t iH=0; iH<fvLastHits[iSmType][iSm][iRpc][iCh].size()-1; iH++){//fill for all memorized hits 
	       itL--; 	     
	       Double_t dTsinceLast = pHit->GetTime()-(*itL)->GetTime();
               if(dTsinceLast > fdMemoryTime)
		 LOG(FATAL)<<Form("Invalid Time since last hit on channel TSRC %d%d%d%d: %f > %f",
				  iSmType,iSm,iRpc,iCh,dTsinceLast,fdMemoryTime
				  )
			   << FairLogger::endl; 

	       fhTRpcCluTOffDTLastHits[iDetIndx][iSel]->Fill(TMath::Log(dTsinceLast),
		                                            pHit->GetTime()-dTTrig[iSel]);
	       fhTRpcCluMemMulDTLastHits[iDetIndx][iSel]->Fill(TMath::Log(dTsinceLast),
		                                             fvLastHits[iSmType][iSm][iRpc][iCh].size()-1);
	     }      
	   }
         }
       }
     }
   }

   for( Int_t iSmType = 0; iSmType < fDigiBdfPar->GetNbSmTypes(); iSmType++ ){
      for( Int_t iRpc = 0; iRpc < fDigiBdfPar->GetNbRpc( iSmType); iRpc++ )
      {
         LOG(DEBUG1)<<"CbmTofTestBeamClusterizer::FillHistos:  "
                    <<Form(" %3d %3d %3lu ",iSmType,iRpc,fviClusterSize[iSmType][iRpc].size())
                    <<FairLogger::endl;
    
         for( UInt_t uCluster = 0; uCluster < fviClusterSize[iSmType][iRpc].size(); uCluster++ )
         {
            LOG(DEBUG2)<<"CbmTofTestBeamClusterizer::FillHistos:  "
                       <<Form(" %3d %3d %3d ",iSmType,iRpc,uCluster)
                       <<FairLogger::endl;

            fhClusterSize->Fill( fviClusterSize[iSmType][iRpc][uCluster]);
            fhClusterSizeType->Fill( fviClusterSize[iSmType][iRpc][uCluster], 40*iSmType + iRpc ); //FIXME - hardwired constant
            if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
            {
               fhTrackMul->Fill( fviTrkMul[iSmType][iRpc][uCluster] );
               fhClusterSizeMulti->Fill( fviClusterSize[iSmType][iRpc][uCluster],
                                         fviTrkMul[iSmType][iRpc][uCluster] );
               if( 1 == fviTrkMul[iSmType][iRpc][uCluster] )
                  fhTrk1MulPos->Fill( fvdX[iSmType][iRpc][uCluster], fvdY[iSmType][iRpc][uCluster] );
               if( 1 < fviTrkMul[iSmType][iRpc][uCluster] )
                  fhHiTrkMulPos->Fill(fvdX[iSmType][iRpc][uCluster], fvdY[iSmType][iRpc][uCluster] );
               fhAllTrkMulPos->Fill(fvdX[iSmType][iRpc][uCluster], fvdY[iSmType][iRpc][uCluster] );
            } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
            if(kFALSE) //  1 == fviTrkMul[iSmType][iRpc][uCluster] )
            {
               fhClustSizeDifX->Fill( fviClusterSize[iSmType][iRpc][uCluster], fvdDifX[iSmType][iRpc][uCluster]);
               fhClustSizeDifY->Fill( fviClusterSize[iSmType][iRpc][uCluster], fvdDifY[iSmType][iRpc][uCluster]);
               if( 1 == fviClusterSize[iSmType][iRpc][uCluster] )
               {
                  fhChDifDifX->Fill( fvdDifCh[iSmType][iRpc][uCluster], fvdDifX[iSmType][iRpc][uCluster]);
                  fhChDifDifY->Fill( fvdDifCh[iSmType][iRpc][uCluster], fvdDifY[iSmType][iRpc][uCluster]);
               }
            }
         } // for( UInt_t uCluster = 0; uCluster < fviClusterSize[iSmType][iRpc].size(); uCluster++ )
         fviClusterSize[iSmType][iRpc].clear();
         fviTrkMul[iSmType][iRpc].clear();
         fvdX[iSmType][iRpc].clear();
         fvdY[iSmType][iRpc].clear();
         fvdDifX[iSmType][iRpc].clear();
         fvdDifY[iSmType][iRpc].clear();
         fvdDifCh[iSmType][iRpc].clear();
      } // for( Int_t iRpc = 0; iRpc < fDigiBdfPar->GetNbRpc( iSmType); iRpc++ )
   }
   fhNbSameSide->Fill(fiNbSameSide);
  }

  return kTRUE;
}

Bool_t   CbmTofTestBeamClusterizer::WriteHistos()
{
   TDirectory * oldir = gDirectory;
   TFile *fHist;
   fHist = new TFile(fOutHstFileName,"RECREATE");
   fHist->cd();
   fhClustBuildTime->Write();

   for (Int_t iS=0; iS< fDigiBdfPar->GetNbSmTypes(); iS++){
     if (NULL == fhSmCluPosition[iS]) continue; 
     fhSmCluPosition[iS]->Write();
     fhSmCluTOff[iS]->Write();
     fhSmCluSvel[iS]->Write();
     for (Int_t iSel=0; iSel<iNSel; iSel++){  // Loop over selectors 
       fhTSmCluPosition[iS][iSel]->Write();
       fhTSmCluTOff[iS][iSel]->Write();
       fhTSmCluTRun[iS][iSel]->Write();
     }
   }

   for(Int_t iDetIndx=0; iDetIndx< fDigiBdfPar->GetNbDet(); iDetIndx++){
     if(NULL == fhRpcCluMul[iDetIndx]) continue;
     fhRpcCluMul[iDetIndx]->Write();
     fhRpcCluRate[iDetIndx]->Write();
     fhRpcCluPosition[iDetIndx]->Write();
     fhRpcCluDelPos[iDetIndx]->Write();
     fhRpcCluTOff[iDetIndx]->Write();
     fhRpcCluDelTOff[iDetIndx]->Write();
     fhRpcCluTrms[iDetIndx]->Write();
     fhRpcCluTot[iDetIndx]->Write();
     fhRpcCluAvWalk[iDetIndx]->Write();
     fhRpcCluAvLnWalk[iDetIndx]->Write();
     fhRpcDTLastHits[iDetIndx]->Write();
     
     LOG(DEBUG)<<"Write triggered Histos for Det Ind "<<iDetIndx<<Form(", UID 0x%08x",fDigiBdfPar->GetDetUId(iDetIndx))<<FairLogger::endl;
     for (Int_t iSel=0; iSel<iNSel; iSel++){   // Save trigger selected histos 
       if(NULL == fhTRpcCluMul[iDetIndx][iSel]) continue;
       fhTRpcCluMul[iDetIndx][iSel]->Write();
       fhTRpcCluPosition[iDetIndx][iSel]->Write();
       fhTRpcCluTOff[iDetIndx][iSel]->Write();
       fhTRpcCluTot[iDetIndx][iSel]->Write();
       fhTRpcCluAvWalk[iDetIndx][iSel]->Write();
     }

     Int_t iUniqueId = fDigiBdfPar->GetDetUId( iDetIndx );
     Int_t iSmAddr   = iUniqueId & DetMask; 
     Int_t iSmType   = CbmTofAddress::GetSmType( iUniqueId );
     Int_t iSm       = CbmTofAddress::GetSmId( iUniqueId );
     Int_t iRpc      = CbmTofAddress::GetRpcId( iUniqueId );
     
     Int_t iNent =0;
     if(fCalSel > -1){ 
      if(NULL == fhTRpcCluAvWalk[iDetIndx][fCalSel]) continue;
      iNent = (Int_t) fhTRpcCluAvWalk[iDetIndx][fCalSel]->GetEntries();
     }
     else{
      if(NULL == fhRpcCluAvWalk[iDetIndx]) continue;
      iNent = (Int_t) fhRpcCluAvWalk[iDetIndx]->GetEntries();
     }
     if(0==iNent){
       LOG(INFO)<<"WriteHistos: No entries in Walk histos for " 
                 <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc 
                 <<FairLogger::endl;
       // continue;
     }
     
     //     if(-1<fCalSmAddr && fcalType != iSmAddr) continue;
     TH2      *htempPos      = NULL;
     TProfile *htempPos_pfx  = NULL;  
     TH1      *htempPos_py   = NULL;
     TProfile *htempTOff_pfx = NULL;
     TH1      *htempTOff_px  = NULL;
     TProfile *hAvPos_pfx    = NULL;  
     TProfile *hAvTOff_pfx   = NULL;
     TH2      *htempTOff     = NULL; // -> Comment to remove warning because set but never used
     TH2      *htempTot      = NULL;
     TProfile *htempTot_pfx  = NULL; 
     TH1      *htempTot_Mean = NULL; 
     TH1      *htempTot_Off  = NULL; 

     if(-1<fCalSel){
       htempPos      = fhRpcCluPosition[iDetIndx]; // use untriggered distributions for position  
       htempPos_pfx  = fhRpcCluPosition[iDetIndx]->ProfileX("_pfx",1,fhRpcCluPosition[iDetIndx]->GetNbinsY());
       //htempPos      = fhTRpcCluPosition[iDetIndx][fCalSel];
       //htempPos_pfx  = fhTRpcCluPosition[iDetIndx][fCalSel]->ProfileX("_pfx",1,fhTRpcCluPosition[iDetIndx][fCalSel]->GetNbinsY());
       htempTOff     = fhTRpcCluTOff[iDetIndx][fCalSel]; // -> Comment to remove warning because set but never used
       htempTOff_pfx = htempTOff->ProfileX("_pfx",1,fhTRpcCluTOff[iDetIndx][fCalSel]->GetNbinsY());
       htempTOff_px  = htempTOff->ProjectionX("_px",1,fhTRpcCluTOff[iDetIndx][fCalSel]->GetNbinsY());
       htempTot      = fhTRpcCluTot[iDetIndx][fCalSel]; 
       htempTot_pfx  = fhTRpcCluTot[iDetIndx][fCalSel]->ProfileX("_pfx",1,fhTRpcCluTot[iDetIndx][fCalSel]->GetNbinsY());
       hAvPos_pfx    = fhTSmCluPosition[iSmType][fCalSel]->ProfileX("_pfx",1,fhTSmCluPosition[iSmType][fCalSel]->GetNbinsY());
       hAvTOff_pfx   = fhTSmCluTOff[iSmType][fCalSel]->ProfileX("_pfx",1,fhTSmCluTOff[iSmType][fCalSel]->GetNbinsY(),"s");
     }else  // all triggers
     {
       htempPos      = fhRpcCluPosition[iDetIndx];
       htempTot      = fhRpcCluTot[iDetIndx];
       htempTot_pfx  = fhRpcCluTot[iDetIndx]->ProfileX("_pfx",1,fhRpcCluTot[iDetIndx]->GetNbinsY());
       hAvPos_pfx    = fhSmCluPosition[iSmType]->ProfileX("_pfx",1,fhSmCluPosition[iSmType]->GetNbinsY());
       hAvTOff_pfx   = fhSmCluTOff[iSmType]->ProfileX("_pfx",1,fhSmCluTOff[iSmType]->GetNbinsY());
       if(-1==fCalSel){    // take corrections from untriggered distributions 
         htempPos_pfx  = fhRpcCluPosition[iDetIndx]->ProfileX("_pfx",1,fhRpcCluPosition[iDetIndx]->GetNbinsY());
//       htempTOff     = fhRpcCluTOff[iDetIndx]; // -> Comment to remove warning because set but never used
         htempTOff_pfx = fhRpcCluTOff[iDetIndx]->ProfileX("_pfx",1,fhRpcCluTOff[iDetIndx]->GetNbinsY(),"s");
         htempTOff_px  = fhRpcCluTOff[iDetIndx]->ProjectionX("_px",1,fhRpcCluTOff[iDetIndx]->GetNbinsY());
       }else
         { if(-2==fCalSel){ //take corrections from Cluster deviations 
           htempPos_pfx  = fhRpcCluDelPos[iDetIndx]->ProfileX("_pfx",1,fhRpcCluDelPos[iDetIndx]->GetNbinsY());
//         htempTOff     = fhRpcCluDelTOff[iDetIndx]; // -> Comment to remove warning because set but never used
           htempTOff_pfx = fhRpcCluDelTOff[iDetIndx]->ProfileX("_pfx",1,fhRpcCluDelTOff[iDetIndx]->GetNbinsY());
           htempTOff_px  = fhRpcCluDelTOff[iDetIndx]->ProjectionX("_px",1,fhRpcCluDelTOff[iDetIndx]->GetNbinsY());
         }else
         {
           if(-3==fCalSel){  // take corrections from deviations to matched trigger hit
             htempPos_pfx  = fhRpcCluDelMatPos[iDetIndx]->ProfileX("_pfx",1,fhRpcCluDelMatPos[iDetIndx]->GetNbinsY());
//           htempTOff     = fhRpcCluDelMatTOff[iDetIndx]; // -> Comment to remove warning because set but never used
             htempTOff_pfx = fhRpcCluDelMatTOff[iDetIndx]->ProfileX("_pfx",1,fhRpcCluDelMatTOff[iDetIndx]->GetNbinsY());
             htempTOff_px  = fhRpcCluDelMatTOff[iDetIndx]->ProjectionX("_px",1,fhRpcCluDelMatTOff[iDetIndx]->GetNbinsY());
           }
         }
       }
     }

     if(NULL == htempPos_pfx) {
       LOG(INFO)<<"WriteHistos: Projections not available, continue " 
                << FairLogger::endl;
       continue;
     }

     htempTot_Mean=htempTot_pfx->ProjectionX("_Mean");
     htempTot_Off =htempTot_pfx->ProjectionX("_Off");

     htempPos_pfx->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Pos_pfx",iSmType,iSm,iRpc) );
     htempTOff_pfx->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_TOff_pfx",iSmType,iSm,iRpc) );
     htempTot_pfx->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_pfx",iSmType,iSm,iRpc) );
     htempTot_Mean->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_Mean",iSmType,iSm,iRpc) );
     htempTot_Off->SetName( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Tot_Off",iSmType,iSm,iRpc) );
     hAvPos_pfx->SetName( Form("cl_CorSmT%01d_Pos_pfx",iSmType) );
     hAvTOff_pfx->SetName( Form("cl_CorSmT%01d_TOff_pfx",iSmType) );

     switch(fCalMode%10){
     case 0 : {                // Initialize
       htempTot_Off->Reset();   // prepare TotOffset histo
       TH1 *hbins[200];
       Int_t nbins=htempTot->GetNbinsX();
       for (int i=0;i<nbins;i++) {
         hbins[i] = htempTot->ProjectionY(Form("bin%d",i+1),i+1,i+1);
/*         Double_t Ymax=hbins[i]->GetMaximum();*/
         Int_t iBmax=hbins[i]->GetMaximumBin();
         TAxis *xaxis = hbins[i]->GetXaxis();
         Double_t Xmax=xaxis->GetBinCenter(iBmax);
         Double_t XOff=Xmax-fTotPreRange;
         XOff = (Double_t)(Int_t)XOff;
         if(XOff<0) XOff=0;    
         htempTot_Off->SetBinContent(i+1,XOff);
         Double_t Xmean=htempTot_Mean->GetBinContent(i+1);
         if(Xmean<XOff) {
           LOG(WARNING)<<"Inconsistent  Tot numbers for "
                       <<Form("SmT%01d_sm%03d_rpc%03d bin%d: mean %f, Off %f",iSmType,iSm,iRpc,i,Xmean,XOff)
                       <<FairLogger::endl; 
         }
         htempTot_Mean->SetBinContent(i+1,(Xmean-XOff));
         if ( htempTot_Mean->GetBinContent(i+1) != (Xmean-XOff))
           LOG(WARNING)<<"Tot numbers not stored properly for "
                       <<Form("SmT%01d_sm%03d_rpc%03d bin%d: mean %f, target %f",iSmType,iSm,iRpc,i,htempTot_Mean->GetBinContent(i+1),Xmean-XOff)
                       <<FairLogger::endl; 
       }
       htempPos_pfx->Write();
       htempTOff_pfx->Write();
       htempTot_pfx->Write();
       htempTot_Mean->Write();
       htempTot_Off->Write();
     }
       break;
     case 1 :                  //save offsets, update walks 
     {
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
        Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
        LOG(INFO)<<"WriteHistos: restore Offsets and Gains and save Walk for "
                 <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc
		 <<" and calSmAddr = "<<Form(" 0x%08x ",TMath::Abs(fCalSmAddr)) 
                 <<FairLogger::endl;
        htempPos_pfx->Reset();    //reset to restore means of original histos
        htempTOff_pfx->Reset();
        htempTot_Mean->Reset();
        htempTot_Off->Reset();
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
        {
	 Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)*0.5
                           *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
         Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
         htempPos_pfx->Fill(iCh,YMean);
         if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! ch "<<iCh
                     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
                     <<","<<htempPos_pfx->GetBinContent(iCh+2)
                     <<", expected "<<YMean
                     <<FairLogger::endl;
         }
         htempTOff_pfx->Fill(iCh,TMean); 

         for(Int_t iSide=0; iSide<2; iSide++){
           htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );  //nh +1 empirical(?)
           htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
         }
        }

        LOG(DEBUG1)<<" Offset, gain restoring done ... "<<FairLogger::endl;
        htempPos_pfx->Write();
        htempTOff_pfx->Write();
        htempTot_pfx->Write();
        htempTot_Mean->Write();
        htempTot_Off->Write();

        for(Int_t iSel=0; iSel<iNSel; iSel++){
         // Store DelTof corrections 
         TDirectory * curdir = gDirectory;
         gROOT->cd(); //
         TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
         gDirectory->cd( curdir->GetPath() );
         if (NULL!=hCorDelTof) {
           TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
          hCorDelTofout->Write();
         }else {
         LOG(INFO)<<" No CorDelTof histo "
                  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
         }
        }

        if(   (fCalSmAddr < 0 && TMath::Abs(fCalSmAddr) != iSmAddr)
           || fCalSmAddr == iSmAddr)  // select detectors for determination of walk correction
	{

        LOG(INFO)<<"WriteHistos: restore Offsets and Gains and update Walk for "
                 <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc<<" with "<<fDigiBdfPar->GetNbChan(iSmType,iRpc)<<" channels"
                 <<FairLogger::endl;
        for( Int_t iCh=0; iCh< fDigiBdfPar->GetNbChan( iSmType, iRpc ); iCh++){
          TH2 *h2tmp0;
          TH2 *h2tmp1;
	  if(!fEnableAvWalk){
           if(-1<fCalSel){
            h2tmp0=fhTRpcCluWalk[iDetIndx][fCalSel][iCh][0];
            h2tmp1=fhTRpcCluWalk[iDetIndx][fCalSel][iCh][1];
           }else { // take correction from deviation within clusters 
            h2tmp0=fhRpcCluWalk[iDetIndx][iCh][0];
            h2tmp1=fhRpcCluWalk[iDetIndx][iCh][1];
           }
	  }else{  // go for averages (low statistics)
           if(-1<fCalSel){
            h2tmp0=fhTRpcCluAvWalk[iDetIndx][fCalSel];
            h2tmp1=fhTRpcCluAvWalk[iDetIndx][fCalSel];
           }else { // take correction from deviation within clusters 
            h2tmp0=fhRpcCluAvWalk[iDetIndx];
            h2tmp1=fhRpcCluAvWalk[iDetIndx];
           }	  }
          if(NULL == h2tmp0){
            LOG(INFO)<<Form("WriteHistos: Walk histo not available for SmT %d, Sm %d, Rpc %d, Ch %d",iSmType,iSm,iRpc,iCh)
                     <<FairLogger::endl;
            continue;
          }
          Int_t iNEntries=h2tmp0->GetEntries();
          if(iCh==0)  // condition to print message only once
          LOG(INFO)<<Form(" Update Walk correction for SmT %d, Sm %d, Rpc %d, Ch %d, Sel%d: Entries %d",
                          iSmType,iSm,iRpc,iCh,fCalSel,iNEntries)
                     <<FairLogger::endl;

         h2tmp0->Write();
         h2tmp1->Write();
         if(-1<iNEntries){  // always done 
          TProfile *htmp0 = h2tmp0->ProfileX("_pfx",1,h2tmp0->GetNbinsY());
          TProfile *htmp1 = h2tmp1->ProfileX("_pfx",1,h2tmp1->GetNbinsY());
          TH1D *h1tmp0    = h2tmp0->ProjectionX("_px",1,h2tmp0->GetNbinsY());
          TH1D *h1tmp1    = h2tmp1->ProjectionX("_px",1,h2tmp1->GetNbinsY());
          TH1D *h1ytmp0   = h2tmp0->ProjectionY("_py",1,nbClWalkBinX);  // preserve means
          TH1D *h1ytmp1   = h2tmp1->ProjectionY("_py",1,nbClWalkBinX);
          Double_t dWMean0 = h1ytmp0->GetMean();
          Double_t dWMean1 = h1ytmp1->GetMean();
          Double_t dWMean  = 0.5*(dWMean0+dWMean1);
          Int_t iWalkUpd=1;                        // Walk update mode flag
	  //if(5==iSmType || 8==iSmType || 2==iSmType) iWalkUpd=0; // keep both sides consistent for diamonds and pads 
          if(5==iSmType || 8==iSmType) iWalkUpd=0; // keep both sides consistent for diamonds and pads (Cern2016) 
          for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
           switch(iWalkUpd){
           case 0 :
             if(h1tmp0->GetBinContent(iWx+1)>WalkNHmin && h1tmp1->GetBinContent(iWx+1)>WalkNHmin ){
             // preserve y - position (difference) on average 
               Double_t dWcor=(((TProfile *)htmp0)->GetBinContent(iWx+1) + ((TProfile *)htmp1)->GetBinContent(iWx+1))*0.5;
               fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]+=dWcor-dWMean;
               fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]+=dWcor-dWMean;
             }
             break;
           case 1 :
             if(h1tmp0->GetBinContent(iWx+1)>WalkNHmin && h1tmp1->GetBinContent(iWx+1)>WalkNHmin) {
	       Double_t dWcor0 = ((TProfile *)htmp0)->GetBinContent(iWx+1)-dWMean0;
	       Double_t dWcor1 = ((TProfile *)htmp1)->GetBinContent(iWx+1)-dWMean1;
	       Double_t dWcor = 0.5*(dWcor0 + dWcor1);
               fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]+=dWcor-dWMean0;
               fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]+=dWcor-dWMean1;

	       if(iCh==10 && iSmType==9 && iSm==1 && h1tmp0->GetBinContent(iWx+1)>WalkNHmin)
		 LOG(INFO) <<"Update Walk Sm = "<<iSm<<"("<<iNbRpc<<"), Rpc "<< iRpc <<", Bin "<< iWx << ", "
		       <<h1tmp0->GetBinContent(iWx+1)<<" cts: "
                       <<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]<<" + "
		       <<((TProfile *)htmp0)->GetBinContent(iWx+1) << " - " << dWMean0
		       <<" -> "<<dWcor - dWMean0
		       <<", S1: "
                       <<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]<<" + "
		       <<((TProfile *)htmp1)->GetBinContent(iWx+1) << " - " << dWMean1
		       <<" -> "<<dWcor - dWMean1 
                       <<FairLogger::endl;
	     }
             break;

           default:
             ;
           }
          }
          h1tmp0->Reset();
          h1tmp1->Reset();
          for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
           h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
           h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
           LOG(ERROR)<<"WriteHistos: writing not successful! iWx "<<iWx
                     <<" got "<< h1tmp0->GetBinContent(iWx+1)
                     <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
                     <<FairLogger::endl;         }
          
           if( h1tmp1->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]) {
           LOG(ERROR)<<"WriteHistos: writing not successful! iWx "<<iWx
                     <<" got "<< h1tmp1->GetBinContent(iWx+1)
                     <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]
                     <<FairLogger::endl;         }
          }
          h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          h1tmp0->Smooth(iNWalkSmooth);
          htmp0->Write();
          h1tmp0->Write();
          h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          h1tmp1->Smooth(iNWalkSmooth);
          htmp1->Write();
          h1tmp1->Write();
         }
	}
        }else{  // preserve whatever is there for fCalSmAddr !
         for( Int_t iCh = 0; iCh < fDigiBdfPar->GetNbChan( iSmType, iRpc ); iCh++ ) // restore old values 
         {
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
          for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
           h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
           h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! iWx "<<iWx
                     <<" got "<< h1tmp0->GetBinContent(iWx+1)
                     <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
                     <<FairLogger::endl;         }
          }
          h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
          h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
         }
        }     
     }
     break;

     case 2 :         //update time offsets from positions and times with Sm averages, save walks and DELTOF 
     {    
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
        Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );

        if((fCalSmAddr < 0) || (fCalSmAddr != iSmAddr)){   // select detectors for updating offsets
         LOG(INFO)<<"WriteHistos: (case 2) update Offsets and keep Gains, Walk and DELTOF for "
                  <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc 
                  <<FairLogger::endl;
         Int_t iB=iSm*iNbRpc+iRpc;
	 Double_t dVscal=1.;
	 if(NULL != fhSmCluSvel[iSmType])
	    dVscal=fhSmCluSvel[iSmType]->GetBinContent(iSm*iNbRpc+iRpc+1);
	 if(dVscal==0.) dVscal=1.;

         Double_t YMean=((TProfile *)hAvPos_pfx)->GetBinContent(iB+1);  //nh +1 empirical(?)
	 htempPos_py=htempPos->ProjectionY(Form("%s_py",htempPos->GetName()),1,iNbCh);
	 const Double_t YFITMIN=500.;
	 if(htempPos_py->GetEntries() > YFITMIN) {
	   LOG(DEBUG1)<<Form("Determine YMean in %s by fit to %d entries",
			     htempPos->GetName(),(Int_t)htempPos_py->GetEntries()) 
		      <<FairLogger::endl;
	   CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, 0);
	   Int_t iChId = fTofId->SetDetectorInfo( xDetInfo );
	   fChannelInfo = fDigiPar->GetCell( iChId );
	   if(NULL == fChannelInfo){
	     LOG(WARNING)<<Form("invalid ChannelInfo for 0x%08x",iChId)<<FairLogger::endl;
	     continue;
	   }
           fit_ybox(htempPos_py,0.5*fChannelInfo->GetSizey());
	   TF1 *ff=htempPos_py->GetFunction("YBox");
	   if(NULL != ff){              
             LOG(INFO) << "FRes YBox "<<htempPos_py->GetEntries()<<" entries in TSR "<<iSmType<<iSm<<iRpc
		       <<", chi2 "<<ff->GetChisquare()
		       << Form(", striplen (%5.2f), %4.2f: %7.2f +/- %5.2f, pos res %5.2f +/- %5.2f at y_cen = %5.2f +/- %5.2f",
			       fChannelInfo->GetSizey(),dVscal,
			       2.*ff->GetParameter(1),2.*ff->GetParError(1),
			       ff->GetParameter(2),ff->GetParError(2),
			       ff->GetParameter(3),ff->GetParError(3))
		       << FairLogger::endl;	    

	     if(    TMath::Abs(fChannelInfo->GetSizey()-2.*ff->GetParameter(1))/fChannelInfo->GetSizey()<0.2 
	         && TMath::Abs(ff->GetParError(1)/ff->GetParameter(1))<0.2 ) 
	     //	 &&  ff->GetChisquare() < 500.)   //FIXME - constants!
	     {                  
	       if(TMath::Abs(ff->GetParameter(3)-YMean)<0.5*fChannelInfo->GetSizey()){
		 YMean=ff->GetParameter(3);
		 Double_t dV =dVscal*fChannelInfo->GetSizey()/(2.*ff->GetParameter(1)); 
		 fhSmCluSvel[iSmType]->Fill((Double_t)(iSm*iNbRpc+iRpc),dV);
	       }
	     }
	   }
	 }

         Double_t TMean=((TProfile *)hAvTOff_pfx)->GetBinContent(iB+1);
         Double_t TWidth=((TProfile *)hAvTOff_pfx)->GetBinError(iB+1);
         Double_t dTYOff=YMean/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc) ;


         if (fiBeamRefType == iSmType && fiBeamRefSm == iSm && fiBeamRefDet == iRpc) TMean=0.; // don't shift reference counter
         
         LOG(DEBUG)<<Form("<ICor> Correct %d %d %d by TMean=%8.2f, TYOff=%8.2f, TWidth=%8.2f, ",iSmType,iSm,iRpc,TMean,dTYOff,TWidth)
                  <<FairLogger::endl;
        
                  
         for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // update Offset and Gain 
         {
          fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] += -dTYOff + TMean ;
          fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1] += +dTYOff + TMean ;

          LOG(DEBUG)<<"FillCalHist:" 
                   <<" SmT "<< iSmType<<" Sm "<<iSm<<" Rpc "<<iRpc<<" Ch "<<iCh
                   <<": YMean "<<YMean<<", TMean "<< TMean
                   <<" -> " << Form(" %f, %f, %f, %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                    fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1],
                                    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                    fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1])
                   <<FairLogger::endl;
         } // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
        }
        htempPos_pfx->Reset();    //reset to store new values 
        htempTOff_pfx->Reset();
        htempTot_Mean->Reset();
        htempTot_Off->Reset();
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // store new values 
        {
          Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)*0.5
          *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
          Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
          htempPos_pfx->Fill(iCh,YMean);
          if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! ch "<<iCh
                     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
                     <<","<<htempPos_pfx->GetBinContent(iCh+2)
                     <<", expected "<<YMean
                     <<FairLogger::endl;
          }
          htempTOff_pfx->Fill(iCh,TMean); 

          for(Int_t iSide=0; iSide<2; iSide++){
           htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );  //nh +1 empirical(?)
           htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
          }
        } // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 

        LOG(DEBUG1)<<" Updating done ... write to file "<<FairLogger::endl;
        htempPos_pfx->Write();
        htempTOff_pfx->Write();
        htempTot_pfx->Write();
        htempTot_Mean->Write();
        htempTot_Off->Write();

        // store old DELTOF histos
        LOG(DEBUG)<<" Copy old DelTof histos from "<< gDirectory->GetName()<<" to file "<<FairLogger::endl;

        for(Int_t iSel=0; iSel<iNSel; iSel++){
         // Store DelTof corrections 
          TDirectory * curdir = gDirectory;
          gROOT->cd(); //
          TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
          gDirectory->cd( curdir->GetPath() );
          if (NULL!=hCorDelTof) {
             TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
             hCorDelTofout->Write();
          }else {
             LOG(DEBUG)<<" No CorDelTof histo "
                  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
          }
        }

        // store walk histos
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // store new values 
        {
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
          for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
           h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
           h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! iWx "<<iWx
                     <<" got "<< h1tmp0->GetBinContent(iWx+1)
                     <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
                     <<FairLogger::endl;         }
          }
          h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
          h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
        }
     }
     break;

     case 3 :    //update offsets, gains, save walks and DELTOF 
     {     
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc(  iSmType);
        Int_t iNbCh  = fDigiBdfPar->GetNbChan( iSmType, iRpc );
        if((fCalSmAddr < 0) || (fCalSmAddr != iSmAddr) ){     // select detectors for updating offsets
         LOG(INFO)<<"WriteHistos (calMode==3): update Offsets and Gains, keep Walk and DelTof for "
                  <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc<<" with " <<  iNbCh << " channels "
                   <<" using selector "<<fCalSel
                  <<FairLogger::endl;
         /*
         Double_t dTRefMean=0.;
         if (5 == iSmType && fTRefMode%10 == iSm){   // reference counter
           dTRefMean=htempTOff->GetMean(2);
         }
         */
	 Double_t dVscal=1.;
	 if(NULL != fhSmCluSvel[iSmType])
	    dVscal=fhSmCluSvel[iSmType]->GetBinContent(iSm*iNbRpc+iRpc+1);
	 if(dVscal==0.) dVscal=1.;

         for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // update Offset and Gain 
         {
          Double_t YMean=((TProfile *)htempPos_pfx)->GetBinContent(iCh+1);  //set default
	  htempPos_py=htempPos->ProjectionY(Form("%s_py%02d",htempPos->GetName(),iCh),iCh+1,iCh+1);
	  const Double_t YFITMIN=500.;
	  if(htempPos_py->GetEntries() > YFITMIN) {
	    LOG(DEBUG1)<<Form("Determine YMean in %s of channel %d by fit to %d entries",
			    htempPos->GetName(),iCh,(Int_t)htempPos_py->GetEntries()) 
		       <<FairLogger::endl;
	    CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh);
	    Int_t iChId = fTofId->SetDetectorInfo( xDetInfo );
	    fChannelInfo = fDigiPar->GetCell( iChId );
	    if(NULL == fChannelInfo){
	      LOG(WARNING)<<Form("invalid ChannelInfo for 0x%08x",iChId)<<FairLogger::endl;
	      continue;
	    }
            fit_ybox(htempPos_py,0.5*fChannelInfo->GetSizey());
	    TF1 *ff=htempPos_py->GetFunction("YBox");
	    if(NULL != ff){              
              LOG(DEBUG) << "FRes YBox "<<htempPos_py->GetEntries()<<" entries in "<<iSmType<<iSm<<iRpc<<iCh
			<<", chi2 "<<ff->GetChisquare()
			<< Form(", striplen (%5.2f), %4.2f: %7.2f +/- %5.2f, pos res %5.2f +/- %5.2f at y_cen = %5.2f +/- %5.2f",
				fChannelInfo->GetSizey(),dVscal,
				2.*ff->GetParameter(1),2.*ff->GetParError(1),
				ff->GetParameter(2),ff->GetParError(2),
				ff->GetParameter(3),ff->GetParError(3))
			<< FairLogger::endl;	    

	      if(    TMath::Abs(fChannelInfo->GetSizey()-2.*ff->GetParameter(1))/fChannelInfo->GetSizey()<0.2 
		  && TMath::Abs(ff->GetParError(1)/ff->GetParameter(1))<0.2 ) 
	      //	 &&  ff->GetChisquare() < 500.)   //FIXME - constants!
	      {                  
		if(TMath::Abs(ff->GetParameter(3)-YMean)<0.5*fChannelInfo->GetSizey()){
		  YMean=ff->GetParameter(3);
		  Double_t dV =dVscal*fChannelInfo->GetSizey()/(2.*ff->GetParameter(1)); 
		  fhSmCluSvel[iSmType]->Fill((Double_t)(iSm*iNbRpc+iRpc),dV);
		}
	      }
	    }
	  } 
                   
          Double_t TMean=((TProfile *)htempTOff_pfx)->GetBinContent(iCh+1);
          Double_t dTYOff=YMean/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc) ;

	  if (fiBeamRefType == iSmType && fiBeamRefSm == iSm && fiBeamRefDet == iRpc) {
	    // don't shift reference counter on average
            TMean-=((TProfile *)hAvTOff_pfx)->GetBinContent(iSm*iNbRpc+iRpc+1); 
          }

          if(htempTOff_px->GetBinContent(iCh+1)>WalkNHmin){
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] += -dTYOff + TMean;
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1] += +dTYOff + TMean;
          }
	  LOG(DEBUG3)<<Form("Calib: TSRC %d%d%d%d, hits %6.0f, new Off %8.0f,%8.0f ",
			  iSmType,iSm,iRpc,iCh,htempTOff_px->GetBinContent(iCh+1),
			  fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
			  fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1])
                    <<FairLogger::endl;

          /*
           Double_t TotMean=((TProfile *)htempTot_pfx)->GetBinContent(iCh+1);  //nh +1 empirical(!)
          if(1<TotMean){
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0] *= fdTTotMean / TotMean;
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1] *= fdTTotMean / TotMean;
          }
          */
          for(Int_t iSide=0; iSide<2; iSide++){
            Int_t ib=iCh*2+1+iSide;
            TH1 * hbin=htempTot->ProjectionY(Form("bin%d",ib),ib,ib);
            if(100>hbin->GetEntries()) continue;  //request min number of entries
/*            Double_t Ymax=hbin->GetMaximum();*/
            Int_t iBmax=hbin->GetMaximumBin();
            TAxis *xaxis = hbin->GetXaxis();
            Double_t Xmax=xaxis->GetBinCenter(iBmax)/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide];
            Double_t XOff=Xmax-fTotPreRange;
            if(0){//TMath::Abs(XOff - fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide])>100){
              LOG(WARNING)<<"XOff changed for "
                          <<Form("SmT%01d_sm%03d_rpc%03d_Side%d: XOff %f, old %f",iSmType,iSm,iRpc,iSide,XOff,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide])
                          <<FairLogger::endl; 
            }
            //            Double_t TotMean=htempTot_Mean->GetBinContent(ib);
            Double_t TotMean=hbin->GetMean();
            if(15==iSmType){
              LOG(WARNING)<<"Gain for "
                          <<Form("SmT%01d_sm%03d_rpc%03d_Side%d: TotMean %f, prof %f, gain %f, modg %f ",
                                 iSmType,iSm,iRpc,iSide,TotMean,htempTot_Mean->GetBinContent(ib),
                                 fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide],fdTTotMean / TotMean)
                          <<FairLogger::endl;             }
            if(1<TotMean){
              fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] *= fdTTotMean / TotMean;
            }
          }
          if(5==iSmType && 
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] != fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]) {// diamond 
            LOG(WARNING)<<"CbmTofTestBeamClusterizer::FillCalHist:" 
                        <<" SmT "<< iSmType<<" Sm "<<iSm<<" Rpc "<<iRpc<<" Ch "<<iCh
                        <<": YMean "<<YMean<<", TMean "<< TMean
                        <<" -> " 
                        <<Form(" %f %f %f %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                               fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1],
                                               fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                               fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1])
                        <<FairLogger::endl;
            Double_t dTOff=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]);
            Double_t dGain=0.5*(fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0]+fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]);        
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]   =dTOff;
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]   =dTOff;
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0]=dGain;
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]=dGain;
          }   // diamond warning end 
         }    // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
        }     // iSmType selection condition  

        htempPos_pfx->Reset();    //reset to store new values 
        htempTOff_pfx->Reset();
        htempTot_Mean->Reset();
        htempTot_Off->Reset();
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // store new values 
        {
	 Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)*0.5
          *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
         Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
         htempPos_pfx->Fill(iCh,YMean);
         if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! ch "<<iCh
                     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
                     <<","<<htempPos_pfx->GetBinContent(iCh+2)
                     <<", expected "<<YMean
                     <<FairLogger::endl;
         }
         htempTOff_pfx->Fill(iCh,TMean); 

          for(Int_t iSide=0; iSide<2; iSide++){
           htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );
           htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
          }
          //         htempTot_pfx->Fill(iCh,fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]);
        } // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 


        LOG(DEBUG1)<<" Updating done ... write to file "<<FairLogger::endl;
        htempPos_pfx->Write();
        htempTOff_pfx->Write();
        htempTot_pfx->Write();
        htempTot_Mean->Write();
        htempTot_Off->Write();

        // store old DELTOF histos
        LOG(DEBUG)<<" Copy old DelTof histos from "<< gDirectory->GetName()<<" to file "<<FairLogger::endl;

        for(Int_t iSel=0; iSel<iNSel; iSel++){
         // Store DelTof corrections 
         TDirectory * curdir = gDirectory;
         gROOT->cd(); //
         TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
         gDirectory->cd( curdir->GetPath() );
         if (NULL!=hCorDelTof) {
           TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
          hCorDelTofout->Write();
         }else {
         LOG(DEBUG)<<" No CorDelTof histo "
                  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
         }
        }

        LOG(DEBUG)<<" Store old walk histos to file "<<FairLogger::endl;
        // store walk histos
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // restore old values 
        {
	  if(NULL == fhRpcCluWalk[iDetIndx][iCh][0]) break;
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
          for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
           h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
           h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! iWx "<<iWx
                     <<" got "<< h1tmp0->GetBinContent(iWx+1)
                     <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
                     <<FairLogger::endl;         }
          }
          h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
          h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
        }
     }
     break;
     case 4 :   //update DelTof, save offsets, gains and walks 
     {    
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
        Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
        LOG(INFO)<<"WriteHistos: restore Offsets, Gains and Walk, save DelTof for "
                 <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc 
                 <<FairLogger::endl;
        htempPos_pfx->Reset();    //reset to restore mean of original histos
        htempTOff_pfx->Reset();
        htempTot_Mean->Reset();
        htempTot_Off->Reset();
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
        {
	 Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)*0.5
                           *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
         Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
         htempPos_pfx->Fill(iCh,YMean);
         if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! ch "<<iCh
                     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
                     <<","<<htempPos_pfx->GetBinContent(iCh+2)
                     <<", expected "<<YMean
                     <<FairLogger::endl;
         }
         htempTOff_pfx->Fill(iCh,TMean); 

         for(Int_t iSide=0; iSide<2; iSide++){
           htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );  //nh +1 empirical(?)
           htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
         }
        }

        LOG(DEBUG1)<<" Restoring of Offsets and Gains done ... "<<FairLogger::endl;
        htempPos_pfx->Write();
        htempTOff_pfx->Write();
        htempTot_pfx->Write();
        htempTot_Mean->Write();
        htempTot_Off->Write();

        // restore walk histos
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // restore old values 
        {
          if(NULL == fhRpcCluWalk[iDetIndx][iCh][0]) break;
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
          for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
           h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
           h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! iWx "<<iWx
                     <<" got "<< h1tmp0->GetBinContent(iWx+1)
                     <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
                     <<FairLogger::endl;         }
          }
          h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
          h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
        }

        // generate/update DelTof corrections 
        if((fCalSmAddr < 0 && -fCalSmAddr != iSmAddr) || (fCalSmAddr == iSmAddr))  // select detectors for determination of DelTof correction
        {
          // if( fiBeamRefType == iSmType ) continue;  // no DelTof correction for Diamonds

          for(Int_t iSel=0; iSel<iNSel; iSel++){
          TH2 *h2tmp=fhTRpcCluDelTof[iDetIndx][iSel];
          if(NULL == h2tmp){
           LOG(INFO)<<Form("WriteHistos:  histo not available for SmT %d, Sm %d, Rpc %d",
                          iSmType,iSm,iRpc)
                     <<FairLogger::endl;
            break;
          }
          Int_t iNEntries=h2tmp->GetEntries();

          h2tmp->Write();
          TProfile *htmp = h2tmp->ProfileX("_pfx",1,h2tmp->GetNbinsY());
          TH1D *h1tmp    = h2tmp->ProjectionX("_px",1,h2tmp->GetNbinsY());
/*          TH1D *h1ytmp   = h2tmp->ProjectionY("_py",1,h2tmp->GetNbinsX());*/
          Double_t dDelMean=0.;//h1ytmp->GetMean();// inspect normalization, interferes with mode 3 for diamonds, nh, 10.01.2015 (?) 
          for(Int_t iBx=0; iBx<nbClDelTofBinX; iBx++){
	    Double_t dNEntries = h1tmp->GetBinContent(iBx+1);
            if(dNEntries > WalkNHmin)  // modify, request sufficient # of entries  
            fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel]+=((TProfile *)htmp)->GetBinContent(iBx+1);
	    dDelMean += fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel];
          }
	  dDelMean /= (Double_t)nbClDelTofBinX;

          LOG(INFO)<<Form(" Update DelTof correction for SmT %d, Sm %d, Rpc %d, Sel%d: Entries %d, Mean shift %6.1f",
                          iSmType,iSm,iRpc,iSel,iNEntries,dDelMean)
                    <<FairLogger::endl;

          for(Int_t iBx=0; iBx<nbClDelTofBinX; iBx++){
            //h1tmp->SetBinContent(iBx+1,fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel]-dDelMean);
            h1tmp->SetBinContent(iBx+1,fvCPDelTof[iSmType][iSm*iNbRpc+iRpc][iBx][iSel]);
          }
          h1tmp->SetName(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof", iSmType, iSm, iRpc,iSel));
          h1tmp->Write();
         }
        }else{  // copy existing histograms  
         for(Int_t iSel=0; iSel<iNSel; iSel++){
          // Store DelTof corrections 
          TDirectory * curdir = gDirectory;
          gROOT->cd(); //
          TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
          gDirectory->cd( curdir->GetPath() );
          if (NULL!=hCorDelTof) {
           TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
           LOG(INFO)<<" Save existing CorDelTof histo "
                  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
           hCorDelTofout->Write();
          }else {
          LOG(DEBUG)<<" No CorDelTof histo "
                    <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
          }
         }
        }
     }
     break;
    case 5 :    //update offsets (from positions only), gains, save walks and DELTOF 
     {     
        Int_t iNbRpc = fDigiBdfPar->GetNbRpc(  iSmType);
        Int_t iNbCh  = fDigiBdfPar->GetNbChan( iSmType, iRpc );
        if((fCalSmAddr < 0) || (fCalSmAddr != iSmAddr) ){     // select detectors for updating offsets
         LOG(INFO)<<"WriteHistos (calMode==3): update Offsets and Gains, keep Walk and DelTof for "
                  <<"Smtype"<<iSmType<<", Sm "<<iSm<<", Rpc "<<iRpc<<" with " <<  iNbCh << " channels "
                   <<" using selector "<<fCalSel
                  <<FairLogger::endl;

         for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // update Offset and Gain 
         {
          Double_t YMean=((TProfile *)htempPos_pfx)->GetBinContent(iCh+1);  //nh +1 empirical(?)
          Double_t TMean=0.;
          Double_t dTYOff=YMean/fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc) ;


          if(htempTOff_px->GetBinContent(iCh+1)>WalkNHmin){
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] += -dTYOff + TMean;
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1] += +dTYOff + TMean;
          }
	  LOG(DEBUG3)<<Form("Calib: TSRC %d%d%d%d, hits %6.0f, new Off %8.0f,%8.0f ",
			  iSmType,iSm,iRpc,iCh,htempTOff_px->GetBinContent(iCh+1),
			  fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
			  fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1])
                    <<FairLogger::endl;

          /*
           Double_t TotMean=((TProfile *)htempTot_pfx)->GetBinContent(iCh+1);  //nh +1 empirical(!)
          if(1<TotMean){
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0] *= fdTTotMean / TotMean;
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1] *= fdTTotMean / TotMean;
          }
          */
          for(Int_t iSide=0; iSide<2; iSide++){
            Int_t ib=iCh*2+1+iSide;
            TH1 * hbin=htempTot->ProjectionY(Form("bin%d",ib),ib,ib);
            if(100>hbin->GetEntries()) continue;  //request min number of entries
/*            Double_t Ymax=hbin->GetMaximum();*/
            Int_t iBmax=hbin->GetMaximumBin();
            TAxis *xaxis = hbin->GetXaxis();
            Double_t Xmax=xaxis->GetBinCenter(iBmax)/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide];
            Double_t XOff=Xmax-fTotPreRange;
            if(0){//TMath::Abs(XOff - fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide])>100){
              LOG(WARNING)<<"XOff changed for "
                          <<Form("SmT%01d_sm%03d_rpc%03d_Side%d: XOff %f, old %f",iSmType,iSm,iRpc,iSide,XOff,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide])
                          <<FairLogger::endl; 
            }
            //            Double_t TotMean=htempTot_Mean->GetBinContent(ib);
            Double_t TotMean=hbin->GetMean();
            if(15==iSmType){
              LOG(WARNING)<<"Gain for "
                          <<Form("SmT%01d_sm%03d_rpc%03d_Side%d: TotMean %f, prof %f, gain %f, modg %f ",
                                 iSmType,iSm,iRpc,iSide,TotMean,htempTot_Mean->GetBinContent(ib),
                                 fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide],fdTTotMean / TotMean)
                          <<FairLogger::endl;             }
            if(1<TotMean){
              fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] *= fdTTotMean / TotMean;
            }
          }
          if(5==iSmType && 
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0] != fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]) {// diamond 
            LOG(WARNING)<<"CbmTofTestBeamClusterizer::FillCalHist:" 
                        <<" SmT "<< iSmType<<" Sm "<<iSm<<" Rpc "<<iRpc<<" Ch "<<iCh
                        <<": YMean "<<YMean<<", TMean "<< TMean
                        <<" -> " 
                        <<Form(" %f %f %f %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                               fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1],
                                               fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                               fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1])
                        <<FairLogger::endl;
            Double_t dTOff=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]);
            Double_t dGain=0.5*(fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0]+fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]);        
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]   =dTOff;
            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]   =dTOff;
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][0]=dGain;
            fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]=dGain;
          }   // diamond warning end 
         }    // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 
        }     // iSmType selection condition  

        htempPos_pfx->Reset();    //reset to store new values 
        htempTOff_pfx->Reset();
        htempTot_Mean->Reset();
        htempTot_Off->Reset();
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // store new values 
        {
	 Double_t YMean=fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc)*0.5
          *(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]-fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
         Double_t TMean=0.5*(fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1]+fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0]);
         htempPos_pfx->Fill(iCh,YMean);
         if( ((TProfile *)htempPos_pfx)->GetBinContent(iCh+1)!=YMean) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! ch "<<iCh
                     <<" got "<< htempPos_pfx->GetBinContent(iCh)<<","<<htempPos_pfx->GetBinContent(iCh+1)
                     <<","<<htempPos_pfx->GetBinContent(iCh+2)
                     <<", expected "<<YMean
                     <<FairLogger::endl;
         }
         htempTOff_pfx->Fill(iCh,TMean); 

          for(Int_t iSide=0; iSide<2; iSide++){
           htempTot_Mean->SetBinContent(iCh*2+1+iSide,
                          fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][iSide] );
           htempTot_Off->SetBinContent(iCh*2+1+iSide,fvCPTotOff[iSmType][iSm*iNbRpc+iRpc][iCh][iSide]);
          }
          //         htempTot_pfx->Fill(iCh,fdTTotMean/fvCPTotGain[iSmType][iSm*iNbRpc+iRpc][iCh][1]);
        } // for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) 


        LOG(DEBUG1)<<" Updating done ... write to file "<<FairLogger::endl;
        htempPos_pfx->Write();
        htempTOff_pfx->Write();
        htempTot_pfx->Write();
        htempTot_Mean->Write();
        htempTot_Off->Write();

        // store old DELTOF histos
        LOG(DEBUG)<<" Copy old DelTof histos from "<< gDirectory->GetName()<<" to file "<<FairLogger::endl;

        for(Int_t iSel=0; iSel<iNSel; iSel++){
         // Store DelTof corrections 
         TDirectory * curdir = gDirectory;
         gROOT->cd(); //
         TH1D *hCorDelTof =(TH1D*) gDirectory->FindObjectAny( Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
         gDirectory->cd( curdir->GetPath() );
         if (NULL!=hCorDelTof) {
           TH1D *hCorDelTofout=(TH1D*)hCorDelTof->Clone(Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel));
          hCorDelTofout->Write();
         }else {
         LOG(DEBUG)<<" No CorDelTof histo "
                  <<Form("cl_CorSmT%01d_sm%03d_rpc%03d_Sel%02d_DelTof",iSmType,iSm,iRpc,iSel)<<FairLogger::endl;
         }
        }

        LOG(DEBUG)<<" Store old walk histos to file "<<FairLogger::endl;
        // store walk histos
        for( Int_t iCh = 0; iCh < iNbCh; iCh++ ) // restore old values 
        {
	  if(NULL == fhRpcCluWalk[iDetIndx][iCh][0]) break;
          TProfile *htmp0 = fhRpcCluWalk[iDetIndx][iCh][0]->ProfileX("_pfx",1,nbClWalkBinY);
          TProfile *htmp1 = fhRpcCluWalk[iDetIndx][iCh][1]->ProfileX("_pfx",1,nbClWalkBinY);
          TH1D *h1tmp0    = fhRpcCluWalk[iDetIndx][iCh][0]->ProjectionX("_px",1,nbClWalkBinY);
          TH1D *h1tmp1    = fhRpcCluWalk[iDetIndx][iCh][1]->ProjectionX("_px",1,nbClWalkBinY);
          for(Int_t iWx=0; iWx<nbClWalkBinX; iWx++){
           h1tmp0->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]);
           h1tmp1->SetBinContent(iWx+1,fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][1][iWx]);
           if( h1tmp0->GetBinContent(iWx+1)!=fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]) {
           LOG(ERROR)<<"WriteHistos: restore unsuccessful! iWx "<<iWx
                     <<" got "<< h1tmp0->GetBinContent(iWx+1)
                     <<", expected "<<fvCPWalk[iSmType][iSm*iNbRpc+iRpc][iCh][0][iWx]
                     <<FairLogger::endl;         }
          }
          h1tmp0->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S0_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp0->Write();
          h1tmp0->Write();
          h1tmp1->SetName(Form("Cor_SmT%01d_sm%03d_rpc%03d_Ch%03d_S1_Walk_px", iSmType, iSm, iRpc, iCh ));
          htmp1->Write();
          h1tmp1->Write();
        }
     }
     break;


     default: 
         LOG(DEBUG)<<"WriteHistos: update mode "
                   <<fCalMode<<" not yet implemented"
                   <<FairLogger::endl;
     }
   }

   fhCluMulCorDutSel->Write();

   fhDigSpacDifClust->Write();
   fhDigTimeDifClust->Write();
   fhDigDistClust->Write();

   fhClustSizeDifX->Write();
   fhClustSizeDifY->Write();

   fhChDifDifX->Write();
   fhChDifDifY->Write();

   fhNbSameSide->Write();
   fhNbDigiPerChan->Write();


   fhHitsPerTracks->Write();
   if( kFALSE == fDigiBdfPar->ClustUseTrackId() )
      fhPtsPerHit->Write();
   fhTimeResSingHits->Write();
   fhTimeResSingHitsB->Write();
   fhTimePtVsHits->Write();
   fhClusterSize->Write();
   fhClusterSizeType->Write();
   if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   {
      fhTrackMul->Write();
      fhClusterSizeMulti->Write();
      fhTrk1MulPos->Write();
      fhHiTrkMulPos->Write();
      fhAllTrkMulPos->Write();
      fhMultiTrkProbPos->Divide( fhHiTrkMulPos, fhAllTrkMulPos);
      fhMultiTrkProbPos->Scale( 100.0 );
      fhMultiTrkProbPos->Write();
   } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )


   gDirectory->cd( oldir->GetPath() );

   fHist->Close();

   return kTRUE;
}
Bool_t   CbmTofTestBeamClusterizer::DeleteHistos()
{
   delete fhClustBuildTime;
   delete fhHitsPerTracks;
   delete fhPtsPerHit;
   delete fhTimeResSingHits;
   delete fhTimeResSingHitsB;
   delete fhTimePtVsHits;
   delete fhClusterSize;
   delete fhClusterSizeType;

   if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
   {
      delete fhTrackMul;
      delete fhClusterSizeMulti;
      delete fhTrk1MulPos;
      delete fhHiTrkMulPos;
      delete fhAllTrkMulPos;
      delete fhMultiTrkProbPos;
   }
   delete fhDigSpacDifClust;
   delete fhDigTimeDifClust;
   delete fhDigDistClust;

   delete fhClustSizeDifX;
   delete fhClustSizeDifY;

   delete fhChDifDifX;
   delete fhChDifDifY;

   delete fhNbSameSide;
   delete fhNbDigiPerChan;

   return kTRUE;
}
/************************************************************************************/
Bool_t   CbmTofTestBeamClusterizer::BuildClusters()
{
   /*
    * FIXME: maybe use the 2D distance (X/Y) as criteria instead of the distance long channel
    * direction
    */
  Int_t iMess =0;
   
  if(NULL == fTofDigisColl) {
    LOG(INFO) <<" No CalDigis defined ! Check! " << FairLogger::endl;
    return kFALSE;
  }
  fiNevtBuild++;
  LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters from "
            <<fTofDigisColl->GetEntries()<<" digis in event "<<fiNevtBuild<<FairLogger::endl;

   fTRefHits=0.;

   Int_t iNbTofDigi = fTofDigisColl->GetEntries();
   if( kTRUE )
   {
      for( Int_t iDigInd = 0; iDigInd < iNbTofDigi; iDigInd++ )
      {
       CbmTofDigiExp *pDigi = (CbmTofDigiExp*) fTofDigisColl->At( iDigInd );
       LOG(DEBUG1)<<iDigInd<<" "<<pDigi<<Form(" Address : 0x%08x ",pDigi->GetAddress())
                 <<" SmT " << pDigi->GetType()
                 <<" Sm " << pDigi->GetSm()
                 <<" Rpc "<< pDigi->GetRpc()
                 <<" Ch " << pDigi->GetChannel()
                 <<" S " << pDigi->GetSide()
                 <<" : " << pDigi->ToString()
         //         <<" Time "<<pDigi->GetTime()
         //         <<" Tot " <<pDigi->GetTot()
                 <<FairLogger::endl;

       Int_t iDetIndx= fDigiBdfPar->GetDetInd( pDigi->GetAddress() );
       
       if (fDigiBdfPar->GetNbDet()-1<iDetIndx || iDetIndx<0){
         LOG(DEBUG)<<Form(" Wrong DetIndx %d >< %d,0 ",iDetIndx,fDigiBdfPar->GetNbDet())
                   <<FairLogger::endl;
         break;
       }
       
       if (NULL == fhRpcDigiCor[iDetIndx] ) {
	 if ( 100<iMess++ ) 
         LOG(WARNING)<<Form(" DigiCor Histo for  DetIndx %d derived from 0x%08x not found",iDetIndx,pDigi->GetAddress())
                   <<FairLogger::endl;	 
	 continue; 
       } 
       
       Double_t dTDifMin=dDoubleMax;
       CbmTofDigiExp *pDigi2Min=NULL;
       //       for (Int_t iDigI2 =iDigInd+1; iDigI2<iNbTofDigi;iDigI2++){
       for (Int_t iDigI2 =0; iDigI2<iNbTofDigi;iDigI2++){
         CbmTofDigiExp *pDigi2 = (CbmTofDigiExp*) fTofDigisColl->At( iDigI2 );
         if( iDetIndx == fDigiBdfPar->GetDetInd( pDigi2->GetAddress() )){
           if(0.==pDigi->GetSide() && 1.==pDigi2->GetSide()){
             fhRpcDigiCor[iDetIndx]->Fill( pDigi->GetChannel(),  pDigi2->GetChannel() );
           }
           else 
           {
             if (1.==pDigi->GetSide() && 0.==pDigi2->GetSide()){
                 fhRpcDigiCor[iDetIndx]->Fill( pDigi2->GetChannel(),  pDigi->GetChannel() );
             }
           }
           if(   pDigi->GetChannel() == pDigi2->GetChannel()
              && pDigi->GetSide()    != pDigi2->GetSide()){
            Double_t dTDif=TMath::Abs(pDigi->GetTime()-pDigi2->GetTime());
            if(dTDif<dTDifMin){
             dTDifMin=dTDif;
             pDigi2Min=pDigi2;
            }
           }
         }
       }
       
       if( pDigi2Min !=NULL ){
         CbmTofDetectorInfo xDetInfo(kTOF, pDigi->GetType(), pDigi->GetSm(), pDigi->GetRpc(), 0, pDigi->GetChannel());
         Int_t iChId = fTofId->SetDetectorInfo( xDetInfo );
         fChannelInfo = fDigiPar->GetCell( iChId );
         if(NULL == fChannelInfo){
           LOG(WARNING)<<Form("BuildClusters: invalid ChannelInfo for 0x%08x",iChId)<<FairLogger::endl;
           continue;
         }
         if(  fDigiBdfPar->GetSigVel(pDigi->GetType(),pDigi->GetSm(),pDigi->GetRpc()) * dTDifMin * 0.5 
            < fPosYMaxScal*fChannelInfo->GetSizey()) {
         //check consistency
           if(8==pDigi->GetType() || 5==pDigi->GetType()){
             if(pDigi->GetTime() != pDigi2Min->GetTime()){
               if(fiMsgCnt-- >0){
                 LOG(WARNING)<<" BuildClusters: Inconsistent duplicated digis in event " << fiNevtBuild
                             <<", Ind: "<<iDigInd<< FairLogger::endl;
                 LOG(WARNING)<<"   "<<pDigi->ToString()<< FairLogger::endl;
                 LOG(WARNING)<<"   "<<pDigi2Min->ToString()<< FairLogger::endl;
               }
               pDigi2Min->SetTot(pDigi->GetTot());
               pDigi2Min->SetTime(pDigi->GetTime());
             }
         }

         // average ToTs! temporary fix, FIXME
         /*
         Double_t dAvTot=0.5*(pDigi->GetTot()+pDigi2Min->GetTot());
         pDigi->SetTot(dAvTot);
         pDigi2Min->SetTot(dAvTot);
         LOG(DEBUG)<<" BuildClusters: TDif "<<dTDifMin<<", Average Tot "<<dAvTot<<FairLogger::endl;
         LOG(DEBUG)<<"      "<<pDigi->ToString()<< FairLogger::endl;
         LOG(DEBUG)<<"      "<<pDigi2Min->ToString()<< FairLogger::endl;
         */
       }
      }
   }
   }

   // First Time order the Digis array
   // fTofDigisColl->Sort();

   // Then loop over the digis array and store the Digis in separate vectors for
   // each RPC modules

   // Calibrate RawDigis
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      CbmTofDigiExp *pDigi;
      CbmTofDigiExp *pCalDigi=NULL;
      Int_t iDigIndCal=-1;
      // channel deadtime map 
      std::map<Int_t, Double_t>mChannelDeadTime;
      
      for( Int_t iDigInd = 0; iDigInd < iNbTofDigi; iDigInd++ )
      {
         pDigi = (CbmTofDigiExp*) fTofDigisColl->At( iDigInd );
	 Int_t iAddr=pDigi->GetAddress();

         LOG(DEBUG1)<<"BC "  // Before Calibration
		    <<Form("0x%08x",pDigi->GetAddress())<<" TSRC "
                    <<pDigi->GetType()
                    <<pDigi->GetSm()
                    <<pDigi->GetRpc()
		    <<Form("%2d",(Int_t)pDigi->GetChannel())<<" "
                    <<pDigi->GetSide()<<" "
		    <<Form("%f",pDigi->GetTime())<<" "
                    <<pDigi->GetTot()
		    <<FairLogger::endl;
		   
         if(pDigi->GetType()==5 || pDigi->GetType() == 8)   // for Pad counters generate fake digi to mockup a strip
	   if(pDigi->GetSide()==1) continue;                // skip one side to avoid double entries
   
	 Bool_t bValid=kTRUE;
         std::map<Int_t, Double_t>::iterator it;
	 it = mChannelDeadTime.find(iAddr);
         if( it != mChannelDeadTime.end() ) {
	   LOG(DEBUG1)<<"CCT found valid ChannelDeadtime entry "<<mChannelDeadTime[iAddr]
		      <<", DeltaT "<<pDigi->GetTime()-mChannelDeadTime[iAddr]
		      <<FairLogger::endl;
	   if( (bValid = (pDigi->GetTime() > mChannelDeadTime[iAddr] + fdChannelDeadtime)) )
	     pCalDigi = new((*fTofCalDigisColl)[++iDigIndCal]) CbmTofDigiExp( *pDigi );
	 }else {
	     pCalDigi = new((*fTofCalDigisColl)[++iDigIndCal]) CbmTofDigiExp( *pDigi );
	 }
 	 mChannelDeadTime[iAddr]=pDigi->GetTime();
         if ( ! bValid ) continue;
	 	 
         LOG(DEBUG1)<<"DC "  // After deadtime check. before Calibration
		    <<Form("0x%08x",pDigi->GetAddress())<<" TSRC "
                    <<pDigi->GetType()
                    <<pDigi->GetSm()
                    <<pDigi->GetRpc()
		    <<Form("%2d",(Int_t)pDigi->GetChannel())<<" "
                    <<pDigi->GetSide()<<" "
		    <<Form("%f",pDigi->GetTime())<<" "
                    <<pDigi->GetTot()
		    <<FairLogger::endl;
		   
	 if(fbPs2Ns) {
	   pCalDigi->SetTime(pCalDigi->GetTime()/1000.);        // for backward compatibility
	   pCalDigi->SetTot(pCalDigi->GetTot()/1000.);          // for backward compatibility
	 }
         if(    fDigiBdfPar->GetNbSmTypes() > pDigi->GetType()  // prevent crash due to misconfiguration 
             && fDigiBdfPar->GetNbSm(  pDigi->GetType()) > pDigi->GetSm()
             && fDigiBdfPar->GetNbRpc( pDigi->GetType()) > pDigi->GetRpc()
             && fDigiBdfPar->GetNbChan(pDigi->GetType(),0) >pDigi->GetChannel() 
                )
         {

         LOG(DEBUG2)<<FairLogger::endl<<" CluCal-Init: "<<pDigi->ToString()<<FairLogger::endl;
         // apply calibration vectors 
         pCalDigi->SetTime(pCalDigi->GetTime()- // calibrate Digi Time 
                        fvCPTOff[pDigi->GetType()]
                       [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                       [pDigi->GetChannel()]
                       [pDigi->GetSide()]);
         LOG(DEBUG2)<<" CluCal-TOff: "<<pCalDigi->ToString()<<FairLogger::endl;

         Double_t dTot = pCalDigi->GetTot()-  // subtract Offset 
                       fvCPTotOff[pDigi->GetType()]
                       [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                       [pDigi->GetChannel()]
                       [pDigi->GetSide()];
         if (dTot<1.)  dTot=1;
         pCalDigi->SetTot(dTot *  // calibrate Digi ToT 
                       fvCPTotGain[pDigi->GetType()]
                       [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                       [pDigi->GetChannel()]
                       [pDigi->GetSide()]);

         // walk correction 
         Double_t dTotBinSize = (fdTOTMax-fdTOTMin)/ nbClWalkBinX;
         Int_t iWx = (Int_t)((pCalDigi->GetTot()-fdTOTMin)/dTotBinSize);
         if (0>iWx) iWx=0;
         if (iWx>nbClWalkBinX) iWx=nbClWalkBinX-1;        
         Double_t dDTot = (pCalDigi->GetTot()-fdTOTMin)/dTotBinSize-(Double_t)iWx-0.5;
         Double_t dWT  = fvCPWalk[pCalDigi->GetType()]
                         [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                         [pCalDigi->GetChannel()]
                         [pCalDigi->GetSide()]
                         [iWx];
         if(dDTot > 0) {    // linear interpolation to next bin
           dWT += dDTot * (fvCPWalk[pCalDigi->GetType()]
                          [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                          [pCalDigi->GetChannel()]
                          [pCalDigi->GetSide()]
                          [iWx+1]
                         -fvCPWalk[pCalDigi->GetType()]
                          [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                          [pCalDigi->GetChannel()]
                          [pCalDigi->GetSide()]
                          [iWx]); //memory leak???
         }else  // dDTot < 0,  linear interpolation to next bin
         {
           dWT -= dDTot * (fvCPWalk[pCalDigi->GetType()]
                          [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                          [pCalDigi->GetChannel()]
                          [pCalDigi->GetSide()]
                          [iWx-1]
                         -fvCPWalk[pCalDigi->GetType()]
                          [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                          [pCalDigi->GetChannel()]
                          [pCalDigi->GetSide()]
                          [iWx]); //memory leak???
         }
             
         pCalDigi->SetTime(pCalDigi->GetTime() - dWT); // calibrate Digi Time 
         LOG(DEBUG2)<<" CluCal-Walk: "<<pCalDigi->ToString()<<FairLogger::endl;

         if(0) {//pDigi->GetType()==7 && pDigi->GetSm()==0){
          LOG(INFO)<<"CbmTofTestBeamClusterizer::BuildClusters: CalDigi "
                    <<iDigIndCal<<",  T "
                    <<pCalDigi->GetType()<<", Sm "
                    <<pCalDigi->GetSm()<<", R "
                    <<pCalDigi->GetRpc()<<", Ch "
                    <<pCalDigi->GetChannel()<<", S "
                    <<pCalDigi->GetSide()<<", T "
                    <<pCalDigi->GetTime()<<", Tot "
                    <<pCalDigi->GetTot()
                    <<", TotGain "<<
                       fvCPTotGain[pCalDigi->GetType()]
                       [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                       [pCalDigi->GetChannel()]
                       [pCalDigi->GetSide()]
                    <<", TotOff "<<
                       fvCPTotOff[pCalDigi->GetType()]
                       [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                       [pCalDigi->GetChannel()]
                       [pCalDigi->GetSide()]
                    <<", Walk "<<iWx<<": "<<
                        fvCPWalk[pCalDigi->GetType()]
                        [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                        [pCalDigi->GetChannel()]
                        [pCalDigi->GetSide()]
                        [iWx]
                   <<FairLogger::endl;

          LOG(INFO)<<" dDTot "<<dDTot
                   <<" BinSize: "<<dTotBinSize
                   <<", CPWalk "<<fvCPWalk[pCalDigi->GetType()]
                        [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                        [pCalDigi->GetChannel()]
                        [pCalDigi->GetSide()]
                        [iWx-1]
                  <<", "<<fvCPWalk[pCalDigi->GetType()]
                        [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                        [pCalDigi->GetChannel()]
                        [pCalDigi->GetSide()]
                        [iWx]
                  <<", "<<fvCPWalk[pCalDigi->GetType()]
                        [pCalDigi->GetSm()*fDigiBdfPar->GetNbRpc( pCalDigi->GetType()) + pCalDigi->GetRpc()]
                        [pCalDigi->GetChannel()]
                        [pCalDigi->GetSide()]
                        [iWx+1]
                  <<" -> dWT = "<< dWT
                  <<FairLogger::endl;
          }
          } else 
           {
           LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Skip Digi "
                     <<" Type "<<pDigi->GetType()<<" "<< fDigiBdfPar->GetNbSmTypes()
                     <<" Sm "  <<pDigi->GetSm()<<" " << fDigiBdfPar->GetNbSm(pDigi->GetType())
                     <<" Rpc " <<pDigi->GetRpc()<<" "<< fDigiBdfPar->GetNbRpc(pDigi->GetType())
                     <<" Ch "  <<pDigi->GetChannel()<<" "<<fDigiBdfPar->GetNbChan(pDigi->GetType(),0)
                     <<FairLogger::endl;
           }
          if(pCalDigi->GetType()==5 || pCalDigi->GetType() == 8) {  // for Pad counters generate fake digi to mockup a strip
	     CbmTofDigiExp *pCalDigi2 = new((*fTofCalDigisColl)[++iDigIndCal]) CbmTofDigiExp( *pCalDigi );
             if(pCalDigi->GetSide()==0) pCalDigi2->SetAddress(pCalDigi->GetSm(),pCalDigi->GetRpc(),pCalDigi->GetChannel(),1,pCalDigi->GetType());
             else                       pCalDigi2->SetAddress(pCalDigi->GetSm(),pCalDigi->GetRpc(),pCalDigi->GetChannel(),0,pCalDigi->GetType());;
	  }
      } // for( Int_t iDigInd = 0; iDigInd < nTofDigi; iDigInd++ )
      
      iNbTofDigi = fTofCalDigisColl->GetEntries();  // update because of added duplicted digis
      if(fTofCalDigisColl->IsSortable())
	LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Sort "<<fTofCalDigisColl->GetEntries()<<" calibrated digis "
                   <<FairLogger::endl;
      if(iNbTofDigi>1){
	fTofCalDigisColl->Sort(iNbTofDigi); // Time order again, in case modified by the calibration 
	if(!fTofCalDigisColl->IsSorted()){
	  LOG(WARNING)<<"CbmTofTestBeamClusterizer::BuildClusters: Sorting not successful "
                     <<FairLogger::endl;
	}
      }

      // Store CalDigis in vectors
      for( Int_t iDigInd = 0; iDigInd < iNbTofDigi; iDigInd++ )
      {
         pDigi = (CbmTofDigiExp*) fTofCalDigisColl->At( iDigInd );
         LOG(DEBUG1)<<"AC " // After Calibration
		    <<Form("0x%08x",pDigi->GetAddress())<<" TSRC "
                    <<pDigi->GetType()
                    <<pDigi->GetSm()
                    <<pDigi->GetRpc()
		    <<Form("%2d",(Int_t)pDigi->GetChannel())<<" "
                    <<pDigi->GetSide()<<" "
		    <<Form("%f",pDigi->GetTime())<<" "
                    <<pDigi->GetTot()
		    <<FairLogger::endl;

         if(    fDigiBdfPar->GetNbSmTypes() > pDigi->GetType()  // prevent crash due to misconfiguration 
             && fDigiBdfPar->GetNbSm(  pDigi->GetType()) > pDigi->GetSm()
             && fDigiBdfPar->GetNbRpc( pDigi->GetType()) > pDigi->GetRpc()
             && fDigiBdfPar->GetNbChan(pDigi->GetType(),0) >pDigi->GetChannel() 
                )
         {
         fStorDigiExp[pDigi->GetType()]
                     [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                     [pDigi->GetChannel()].push_back(pDigi);
         fStorDigiInd[pDigi->GetType()]
                     [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                     [pDigi->GetChannel()].push_back(iDigInd);
          } else 
           {
           LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Skip2 Digi "
                     <<" Type "<<pDigi->GetType()<<" "<< fDigiBdfPar->GetNbSmTypes()
                     <<" Sm "  <<pDigi->GetSm()<<" " << fDigiBdfPar->GetNbSm(pDigi->GetType())
                     <<" Rpc " <<pDigi->GetRpc()<<" "<< fDigiBdfPar->GetNbRpc(pDigi->GetType())
                     <<" Ch "  <<pDigi->GetChannel()<<" "<<fDigiBdfPar->GetNbChan(pDigi->GetType(),0)
                     <<FairLogger::endl;
           }
      } // for( Int_t iDigInd = 0; iDigInd < nTofDigi; iDigInd++ )

   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
      else
      {
	return kFALSE; // not implemented properly yet
	/*
         CbmTofDigi *pDigi;
         for( Int_t iDigInd = 0; iDigInd < iNbTofDigi; iDigInd++ )
         {
            pDigi = (CbmTofDigi*) fTofDigisColl->At( iDigInd );
            fStorDigi[pDigi->GetType()]
                     [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                     [pDigi->GetChannel()].push_back(pDigi);
         fStorDigiInd[pDigi->GetType()]
                     [pDigi->GetSm()*fDigiBdfPar->GetNbRpc( pDigi->GetType()) + pDigi->GetRpc()]
                     [pDigi->GetChannel()].push_back(iDigInd);
         } // for( Int_t iDigInd = 0; iDigInd < nTofDigi; iDigInd++ )
	*/
      } // else of if( kTRUE == fDigiBdfPar->UseExpandedDigi() )


   // Then build clusters inside each RPC module
   // Assume only 0 or 1 Digi per channel/side in each event
   // Use simplest method possible, scan direction independent:
   // a) Loop over channels in the RPC starting from 0
   //   * If strips
   //     i) Loop over Digis to check if both ends of the channel have a Digi
   //    ii) Reconstruct a mean channel time and a mean position
   //     + If a Hit is currently filled & the mean position (space, time) is less than XXX from last channel position
   //   iii) Add the mean channel time and the mean position to the ones of the hit
   //     + else
   //   iii) Use nb of strips in cluster to cal. the hit mean time and pos (charge/tot weighting)
   //    iv) Save the hit
   //     v) Start a new hit with current channel
   //   * else (pads)
   //     i) Loop over Digis to find if this channel fired
   //    ii) FIXME: either scan all other channels to check for matching Digis or have more than 1 hit open
   Int_t iNbSmTypes = fDigiBdfPar->GetNbSmTypes();
   // Hit variables
   Double_t dWeightedTime = 0.0;
   Double_t dWeightedPosX = 0.0;
   Double_t dWeightedPosY = 0.0;
   Double_t dWeightedPosZ = 0.0;
   Double_t dWeightsSum   = 0.0;
   std::vector< CbmTofPoint* > vPtsRef;
   std::vector< Int_t > vDigiIndRef;
   CbmTofCell *fTrafoCell=NULL;
   Int_t iTrafoCell=-1;
   Int_t    iNbChanInHit  = 0;
   // Last Channel Temp variables
   Int_t    iLastChan = -1;
   Double_t dLastPosX = 0.0; // -> Comment to remove warning because set but never used
   Double_t dLastPosY = 0.0;
   Double_t dLastTime = 0.0;
   // Channel Temp variables
   Double_t dPosX = 0.0;
   Double_t dPosY = 0.0;
   Double_t dPosZ = 0.0;
   Double_t dTime = 0.0;
   Double_t dTimeDif = 0.0;
   Double_t dTotS = 0.0;
   fiNbSameSide = 0;
   if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   {
      for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
      {
         Int_t iNbSm  = fDigiBdfPar->GetNbSm(  iSmType);
         Int_t iNbRpc = fDigiBdfPar->GetNbRpc( iSmType);
         for( Int_t iSm = 0; iSm < iNbSm; iSm++ )
            for( Int_t iRpc = 0; iRpc < iNbRpc; iRpc++ )
            {
               Int_t iNbCh = fDigiBdfPar->GetNbChan( iSmType, iRpc );
               Int_t iChType = fDigiBdfPar->GetChanType( iSmType, iRpc );
               LOG(DEBUG2)<<"RPC - Loop  "
                         << Form(" %3d %3d %3d %3d ",iSmType,iSm,iRpc,iChType)
                         <<FairLogger::endl;
               fviClusterMul[iSmType][iSm][iRpc]=0; 
               Int_t  iChId = 0;
               if( 0 == iChType )
               {
                  // Don't spread clusters over RPCs!!!
                  dWeightedTime = 0.0;
                  dWeightedPosX = 0.0;
                  dWeightedPosY = 0.0;
                  dWeightedPosZ = 0.0;
                  dWeightsSum   = 0.0;
                  iNbChanInHit  = 0;
                  vPtsRef.clear();
                  // For safety reinitialize everything
                  iLastChan = -1;
//                  dLastPosX = 0.0; // -> Comment to remove warning because set but never used
                  dLastPosY = 0.0;
                  dLastTime = 0.0;
                  LOG(DEBUG2)<<"ChanOrient "
                             << Form(" %3d %3d %3d %3d %3d ",iSmType,iSm,iRpc,fDigiBdfPar->GetChanOrient( iSmType, iRpc ),iNbCh)
                             <<FairLogger::endl;

                  if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
                  {
                     // Horizontal strips => X comes from left right time difference
                  } // if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
                     else
                     {
                        // Vertical strips => Y comes from bottom top time difference
                        for( Int_t iCh = 0; iCh < iNbCh; iCh++ )
                        {
                          LOG(DEBUG3)<<"VDigisize "
                             << Form(" T %3d Sm %3d R %3d Ch %3d Size %3lu ",
                                iSmType,iSm,iRpc,iCh,fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size())
                             <<FairLogger::endl;
                	  if (0 == fStorDigiExp[iSmType][iSm*iNbRpc+iRpc].size()) continue;
                          if( 0 < fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() )
                              fhNbDigiPerChan->Fill( fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() );
                           while( 1 < fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() )
                           {

                              while( (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0])->GetSide() ==
                                      (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1])->GetSide() )
                              {
                                 // Not one Digi of each end!
                                 fiNbSameSide++;
				 if(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size()>2) {
				   LOG(DEBUG) << "SameSide Digis! on TSRC "
					     << iSmType<<iSm<<iRpc<<iCh<<", Times: "
					      <<Form("%f",(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0])->GetTime())
					      << ", "<<Form("%f",(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1])->GetTime())
					     <<", DeltaT " <<(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1])->GetTime() - 
                                                           (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0])->GetTime()
					     <<", array size: " <<  fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() 
					     <<FairLogger::endl;
				   if (     fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][2]->GetSide() 
					 == fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0]->GetSide() ) {
				     LOG(DEBUG) << "3 consecutive SameSide Digis! on TSRC "
					       << iSmType<<iSm<<iRpc<<iCh<<", Times: "
					       <<   (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0])->GetTime()
					       << ", "<<(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1])->GetTime()
					       <<", DeltaT " <<(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1])->GetTime() - 
                                                           (fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0])->GetTime()
					       <<", array size: " <<  fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() 
					       <<FairLogger::endl;
				     fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                     fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				   }else {
				     if( fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][2]->GetTime()
					-fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0]->GetTime() >
					 fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][2]->GetTime()
					-fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1]->GetTime())
				       {
					 fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
					 fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				       }
				     else 
				       {
					 LOG(WARNING) 
					   << Form("Ev %8.0f, digis not properly time ordered, TSRCS %d%d%d%d%d ",
						   fdEvent,iSmType,iSm,iRpc,iCh,(Int_t)fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0]->GetSide())
					   << FairLogger::endl;
				         fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
					 fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				       }
				   }
				 }else{
				   LOG(DEBUG2)<<"SameSide Erase fStor entries(d) "<<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
                                              <<FairLogger::endl;
				   fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				   fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
				 }
                                 if(2 > fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size()) break;
                                 continue;  
                              }

                              LOG(DEBUG2) << "digis processing for " 
                                          << Form(" SmT %3d Sm %3d Rpc %3d Ch %3d # %3lu ",iSmType,iSm,iRpc,iCh,
                                                  fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size())
                                            <<FairLogger::endl;
                              if(2 > fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size()) break;
/*                              Int_t iLastChId = iChId; // Save Last hit channel*/

                              // 2 Digis = both sides present
                              CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh);
                              iChId = fTofId->SetDetectorInfo( xDetInfo );
                              Int_t iUCellId=CbmTofAddress::GetUniqueAddress(iSm,iRpc,iCh,0,iSmType);
                              LOG(DEBUG1)<< Form(" TSRC %d%d%d%d size %3lu ",
                                                 iSmType,iSm,iRpc,iCh,fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size())
                                         << Form(" ChId: 0x%08x 0x%08x ",iChId,iUCellId)
                                         <<FairLogger::endl;
                              fChannelInfo = fDigiPar->GetCell( iChId );
                              if(NULL == fChannelInfo){
                              LOG(ERROR)<<"CbmTofTestBeamClusterizer::BuildClusters: no geometry info! "
                                        << Form(" %3d %3d %3d %3d 0x%08x 0x%08x ",iSmType, iSm, iRpc, iCh, iChId,iUCellId)
                                        <<FairLogger::endl;
                              break;
                              }

                              CbmTofDigiExp * xDigiA = fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][0];
                              CbmTofDigiExp * xDigiB = fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh][1];

                              LOG(DEBUG2) << "    "<<xDigiA->ToString()<<FairLogger::endl;
                              LOG(DEBUG2) << "    "<<xDigiB->ToString()<<FairLogger::endl;

                              // The "Strip" time is the mean time between each end
                              dTime    =0.5 * ( xDigiA->GetTime() + xDigiB->GetTime() ) ; 
                              dTimeDif =      ( xDigiA->GetTime() - xDigiB->GetTime() ) ; 
                              if(5==iSmType && dTimeDif !=0.) { 
                                // FIXME -> Overflow treatment in calib/tdc/TMbsCalibTdcTof.cxx
                                LOG(DEBUG)<<"CbmTofTestBeamClusterizer::BuildClusters: Diamond hit in "
                                          << iSm <<" with inconsistent digits " 
                                          <<  xDigiA->GetTime() << ", " << xDigiB->GetTime()
                                          << " -> "<<dTimeDif
                                    <<FairLogger::endl;
                                LOG(DEBUG) << "    "<<xDigiA->ToString()<<FairLogger::endl;
                                LOG(DEBUG) << "    "<<xDigiB->ToString()<<FairLogger::endl;
                              }

                              // Weight is the total charge => sum of both ends ToT
                              dTotS = xDigiA->GetTot() + xDigiB->GetTot();

                              TGeoNode *fNode=        // prepare local->global trafo
                              gGeoManager->FindNode(fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ());
                              LOG(DEBUG2)<<Form(" Node at (%6.1f,%6.1f,%6.1f) : %p",
                                               fChannelInfo->GetX(),fChannelInfo->GetY(),fChannelInfo->GetZ(),fNode)
                                        <<FairLogger::endl;
                              //          fNode->Print();                         
                              // switch to local coordinates, (0,0,0) is in the center of counter  ?
                              dPosX=((Double_t)(-iNbCh/2 + iCh)+0.5)*fChannelInfo->GetSizex();
                              dPosY=0.;
                              dPosZ=0.;

                              if( 1 == xDigiA->GetSide() )
                                 // 0 is the top side, 1 is the bottom side
                                dPosY += fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc) * dTimeDif * 0.5;
                              else                              
                                 // 0 is the bottom side, 1 is the top side
                                dPosY += -fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc) * dTimeDif * 0.5;

                              LOG(DEBUG1)
                                   <<"NbChanInHit  "
                                   << Form(" %3d %3d %3d %3d %3d 0x%p %1.0f Time %f PosX %f PosY %f Svel %f ",
                                           iNbChanInHit,iSmType,iRpc,iCh,iLastChan,xDigiA,xDigiA->GetSide(),
                                           dTime,dPosX,dPosY,fDigiBdfPar->GetSigVel(iSmType,iSm,iRpc))
                                //  << Form( ", Offs %f, %f ",fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][0],
                                //                            fvCPTOff[iSmType][iSm*iNbRpc+iRpc][iCh][1])
                                   <<FairLogger::endl;

                              // Now check if a hit/cluster is already started
                              if( 0 < iNbChanInHit) 
                              {
                                 if( iLastChan == iCh - 1 )
                                 {
                                    fhDigTimeDifClust->Fill( dTime - dLastTime );
                                    fhDigSpacDifClust->Fill( dPosY - dLastPosY );
                                    fhDigDistClust->Fill( dPosY - dLastPosY,
                                                          dTime - dLastTime );
                                 } 
                                 // if( iLastChan == iCh - 1 )
                                 // a cluster is already started => check distance in space/time
                                 // For simplicity, just check along strip direction for now
                                 // and break cluster when a not fired strip is found
                                 if( TMath::Abs( dTime - dLastTime) < fdMaxTimeDist &&
                                      iLastChan == iCh - 1 &&
                                      TMath::Abs( dPosY - dLastPosY) < fdMaxSpaceDist  )
                                 {
                                    // Add to cluster/hit
                                    dWeightedTime += dTime*dTotS;
                                    dWeightedPosX += dPosX*dTotS;
                                    dWeightedPosY += dPosY*dTotS;
                                    dWeightedPosZ += dPosZ*dTotS;
                                    dWeightsSum   += dTotS;
                                    iNbChanInHit  += 1;
                                    // if one of the side digi comes from a CbmTofPoint not already found
                                    // in this cluster, save its pointer
                                    // Bool_t bFoundA = kFALSE;
                                    // Bool_t bFoundB = kFALSE;
                                    if(NULL != fTofPointsColl){ // MC - FIXME 
                                    
                                    /** **Comment the full Block as not used anymore** **/
/*                                    
                                    if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
                                       for( UInt_t uPtRef = 0; uPtRef < vPtsRef.size(); uPtRef++)
                                       {
                                        //if( ((CbmTofPoint*)(vPtsRef[uPtRef]))->GetTrackID() == ((CbmTofPoint*)(xDigiA->GetLinks()))->GetTrackID() )
                                        //bFoundA = kTRUE;
                                        //if( ((CbmTofPoint*)(vPtsRef[uPtRef]))->GetTrackID() == ((CbmTofPoint*)(xDigiB->GetLinks()))->GetTrackID() )
                                        //bFoundB = kTRUE;
                                       } // for( Int
                                       else for( UInt_t uPtRef = 0; uPtRef < vPtsRef.size(); uPtRef++)
                                       {
                                         //if( vPtsRef[uPtRef] == (CbmTofPoint*)xDigiA->GetLinks() )
                                         //bFoundA = kTRUE;
                                         //if( vPtsRef[uPtRef] == (CbmTofPoint*)xDigiB->GetLinks() )
                                         //bFoundB = kTRUE;
                                       } // for( UInt_t uPtRef = 0; uPtRef < vPtsRef.size(); uPtRef++)
*/

                                    // CbmTofPoint pointer for 1st digi not found => save it
                                    //if( kFALSE == bFoundA )
                                      //                                       vPtsRef.push_back( (CbmTofPoint*)(xDigiA->GetLinks()) );
                                    // CbmTofPoint pointer for 2nd digi not found => save it
                                      //                                    if( kFALSE == bFoundB )
                                      //                                       vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                    } // MC end 
                                    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][0]));
                                    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][1]));

                                    LOG(DEBUG1)<<" Add Digi and erase fStor entries(a): NbChanInHit "<< iNbChanInHit<<", "
                                             <<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
                                             <<FairLogger::endl;
                
                                    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
 
                                } // if current Digis compatible with last fired chan
                                 else
                                 {
                                    // Save Hit
                                    dWeightedTime /= dWeightsSum;
                                    dWeightedPosX /= dWeightsSum;
                                    dWeightedPosY /= dWeightsSum;
                                    dWeightedPosZ /= dWeightsSum;
                                    //  TVector3 hitPosLocal(dWeightedPosX, dWeightedPosY, dWeightedPosZ);
                                    //TVector3 hitPos;
                                    Double_t hitpos_local[3];
                                    hitpos_local[0] = dWeightedPosX;
                                    hitpos_local[1] = dWeightedPosY;
                                    hitpos_local[2] = dWeightedPosZ;

                                    Double_t hitpos[3];
                                    TGeoNode*         cNode   = gGeoManager->GetCurrentNode();
                                    /*TGeoHMatrix* cMatrix =*/ gGeoManager->GetCurrentMatrix();
                                    //cNode->Print();
                                    //cMatrix->Print();

                                    gGeoManager->LocalToMaster(hitpos_local, hitpos);
                                    LOG(DEBUG1)<<
                                    Form(" LocalToMaster for node %p: (%6.1f,%6.1f,%6.1f) ->(%6.1f,%6.1f,%6.1f)", 
                                         cNode, hitpos_local[0], hitpos_local[1], hitpos_local[2], 
                                         hitpos[0], hitpos[1], hitpos[2])
                                              <<FairLogger::endl;

                                    TVector3 hitPos(hitpos[0],hitpos[1],hitpos[2]);

                                    // Simple errors, not properly done at all for now
                                    // Right way of doing it should take into account the weight distribution
                                    // and real system time resolution
                                    TVector3 hitPosErr(0.5,0.5,0.5);  // including positioning uncertainty
                                    /*
                                    TVector3 hitPosErr( fChannelInfo->GetSizex()/TMath::Sqrt(12.0),   // Single strips approximation
                                       0.5, // Use generic value 
                                       1.);

                                    */                                       // fDigiBdfPar->GetFeeTimeRes() * fDigiBdfPar->GetSigVel(iSmType,iRpc), // Use the electronics resolution
                                       //fDigiBdfPar->GetNbGaps( iSmType, iRpc)*
                                       //fDigiBdfPar->GetGapSize( iSmType, iRpc)/ //10.0 / // Change gap size in cm
                                       //TMath::Sqrt(12.0) ); // Use full RPC thickness as "Channel" Z size

                                    // Int_t iDetId = vPtsRef[0]->GetDetectorID();// detID = pt->GetDetectorID() <= from TofPoint
                                    // calc mean ch from dPosX=((Double_t)(-iNbCh/2 + iCh)+0.5)*fChannelInfo->GetSizex();

				    Int_t iChm=floor(dWeightedPosX/fChannelInfo->GetSizex())+iNbCh/2;
				    if(iChm<0)        iChm=0;
				    if(iChm >iNbCh-1) iChm=iNbCh-1;
                                    Int_t iDetId = CbmTofAddress::GetUniqueAddress(iSm,iRpc,iChm,0,iSmType);
                                    Int_t iRefId = 0; // Index of the correspondng TofPoint
                                    if(NULL != fTofPointsColl) {
                                      iRefId = fTofPointsColl->IndexOf( vPtsRef[0] );
                                    }
                                    LOG(DEBUG)<<"Save Hit  "
                                               << Form(" %3d %3d 0x%08x %3d %3d %3d %f %f",
                                                       fiNbHits,iNbChanInHit,iDetId,iChm,iLastChan,iRefId,
                                                       dWeightedTime,dWeightedPosY)
                                               <<", DigiSize: "<<vDigiIndRef.size()
                                               <<", DigiInds: ";

                                    fviClusterMul[iSmType][iSm][iRpc]++; 

                                    for (UInt_t i=0; i<vDigiIndRef.size();i++){
				      LOG(DEBUG)<<" "<<vDigiIndRef.at(i)<<"(M"<<fviClusterMul[iSmType][iSm][iRpc]<<")";
                                    }
                                    LOG(DEBUG)  <<FairLogger::endl;

                                    if(        vDigiIndRef.size() < 2 ){
                                      LOG(WARNING)<<"Digi refs for Hit "
                                                    << fiNbHits<<":        vDigiIndRef.size()"
                                                  <<FairLogger::endl;
                                    }                            
                                    if(fiNbHits>0){
                                      CbmTofHit *pHitL = (CbmTofHit*) fTofHitsColl->At(fiNbHits-1);
                                      if(iDetId == pHitL->GetAddress() && dWeightedTime==pHitL->GetTime()){
                                      LOG(DEBUG)<<"Store Hit twice? "
                                                  <<" fiNbHits "<<fiNbHits<<", "<<Form("0x%08x",iDetId)
                                                  <<FairLogger::endl;

                                       for (UInt_t i=0; i<vDigiIndRef.size();i++){
                                          CbmTofDigiExp *pDigiC = (CbmTofDigiExp*) fTofCalDigisColl->At(vDigiIndRef.at(i));
                                         LOG(DEBUG)<<" Digi  "<<pDigiC->ToString()<<FairLogger::endl;
                                       }
                                       CbmMatch* digiMatchL=(CbmMatch *)fTofDigiMatchColl->At(fiNbHits-1);
                                       for (Int_t i=0; i<digiMatchL->GetNofLinks();i++){
                                         CbmLink L0 = digiMatchL->GetLink(i);  
                                         Int_t iDigIndL=L0.GetIndex();
                                         CbmTofDigiExp *pDigiC = (CbmTofDigiExp*) fTofCalDigisColl->At(iDigIndL);
                                         LOG(DEBUG)<<" DigiL "<<pDigiC->ToString()<<FairLogger::endl;
                                       }
                                      }
                                    }
                                    CbmTofHit *pHit =  new CbmTofHit( iDetId,
                                                       hitPos, hitPosErr,  //local detector coordinates
                                                       fiNbHits,  // this number is used as reference!!
                                                       dWeightedTime,
                                                       vPtsRef.size(), // flag  = number of TofPoints generating the cluster
                                                       0) ; //channel
                                    //                vDigiIndRef);
				    // output hit
				    new((*fTofHitsColl)[fiNbHits]) CbmTofHit(*pHit);
				    // memorize hit 
				    if(fdMemoryTime > 0.) {
				      fvLastHits[iSmType][iSm][iRpc][iChm].push_back(pHit);
				      if(dWeightedTime >= fvLastHits[iSmType][iSm][iRpc][iChm].front()->GetTime()){
				        LOG(DEBUG)<<Form(" Store LH from Ev  %8.0f for TSRC %d%d%d%d, size %lu, addr 0x%08x, time %f",
						       fdEvent,
						       iSmType,iSm,iRpc,iChm,fvLastHits[iSmType][iSm][iRpc][iChm].size(),
						       iDetId,
						       dWeightedTime)
						  <<FairLogger::endl;
				      }else{
					Double_t deltaTime=dWeightedTime - fvLastHits[iSmType][iSm][iRpc][iChm].front()->GetTime();
					LOG(WARNING)<<Form(" insert LH from Ev  %8.0f for TSRC %d%d%d%d, size %lu, addr 0x%08x, delta time %f ",
						       fdEvent,
						       iSmType,iSm,iRpc,iChm,fvLastHits[iSmType][iSm][iRpc][iChm].size(),
						       iDetId,
						       deltaTime)
						    <<FairLogger::endl; 
					fvLastHits[iSmType][iSm][iRpc][iChm].pop_back();
					if(deltaTime==0.){
					  // remove hit, otherwise double entry?
					  pHit->Delete();
					}else{
					  fvLastHits[iSmType][iSm][iRpc][iChm].push_front(pHit);			 
					}
				      }
				    }else{
				      pHit->Delete();
				    }
				    /*
				    new((*fTofDigiMatchColl)[fiNbHits]) CbmMatch();
				    CbmMatch* digiMatch = (CbmMatch *)fTofDigiMatchColl->At(fiNbHits);
				    */
				    CbmMatch* digiMatch = new((*fTofDigiMatchColl)[fiNbHits]) CbmMatch();
				    for (Int_t i=0; i<vDigiIndRef.size();i++){
				      Double_t dTot = ((CbmTofDigiExp*) (fTofCalDigisColl->At(vDigiIndRef.at(i))))->GetTot();
				      digiMatch->AddLink(CbmLink(dTot,vDigiIndRef.at(i)));
				    }
				  				    
                                    fiNbHits++;
                                    // For Histogramming
                                    fviClusterSize[iSmType][iRpc].push_back(iNbChanInHit);
                                    fviTrkMul[iSmType][iRpc].push_back( vPtsRef.size() );
                                    fvdX[iSmType][iRpc].push_back(dWeightedPosX);
                                    fvdY[iSmType][iRpc].push_back(dWeightedPosY);
                                    /*  no TofPoint available for data!  
                                    fvdDifX[iSmType][iRpc].push_back( vPtsRef[0]->GetX() - dWeightedPosX);
                                    fvdDifY[iSmType][iRpc].push_back( vPtsRef[0]->GetY() - dWeightedPosY);
                                    fvdDifCh[iSmType][iRpc].push_back( fGeoHandler->GetCell( vPtsRef[0]->GetDetectorID() ) -1 -iLastChan );
                                    */
                                    vPtsRef.clear();
                                    vDigiIndRef.clear();

                                    // Start a new hit
                                    dWeightedTime = dTime*dTotS;
                                    dWeightedPosX = dPosX*dTotS;
                                    dWeightedPosY = dPosY*dTotS;
                                    dWeightedPosZ = dPosZ*dTotS;
                                    dWeightsSum   = dTotS;
                                    iNbChanInHit  = 1;
                                    // Save pointer on CbmTofPoint
                                    // vPtsRef.push_back( (CbmTofPoint*)(xDigiA->GetLinks()) );
                                    // Save next digi address
                                    LOG(DEBUG2)<<" Next fStor Digi "<<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
                                               <<", Dig0 "<<(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][0])
                                               <<", Dig1 "<<(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][1])
                                               <<FairLogger::endl;

                                    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][0]));
                                    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][1]));
                                    LOG(DEBUG2)<<" Erase fStor entries(b) "<<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
                                               <<FairLogger::endl;
                                    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());

                                    if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
                                    {
                                      //                                       if( ((CbmTofPoint*)(xDigiA->GetLinks()))->GetTrackID() !=
                                      //                       ((CbmTofPoint*)(xDigiB->GetLinks()))->GetTrackID() )
                                          // if other side come from a different Track,
                                          // also save the pointer on CbmTofPoint
                                      //  vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                    } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
                                    //else if( xDigiA->GetLinks() != xDigiB->GetLinks() )
                                          // if other side come from a different TofPoint,
                                          // also save the pointer on CbmTofPoint
                                    //    vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                 } // else of if current Digis compatible with last fired chan
                              } // if( 0 < iNbChanInHit)
                                 else
                                 {
                                     LOG(DEBUG1)<<Form("1.Hit on channel %d, time: %f",iCh,dTime) 
						<<FairLogger::endl;

                                    // first fired strip in this RPC
                                    dWeightedTime = dTime*dTotS;
                                    dWeightedPosX = dPosX*dTotS;
                                    dWeightedPosY = dPosY*dTotS;
                                    dWeightedPosZ = dPosZ*dTotS;
                                    dWeightsSum   = dTotS;
                                    iNbChanInHit  = 1;
                                    // Save pointer on CbmTofPoint
                                    //if(NULL != fTofPointsColl)
                                    //                                    vPtsRef.push_back( (CbmTofPoint*)(xDigiA->GetLinks()) );
                                    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][0]));
                                    vDigiIndRef.push_back( (Int_t )(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh][1]));

                                    LOG(DEBUG2)<<" Erase fStor entries(c) "<<iSmType<<", SR "<<iSm*iNbRpc+iRpc<<", Ch"<<iCh
                                               <<FairLogger::endl;
                                    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());
                                    fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].erase(fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].begin());

                                    if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
                                    {
                                      // if( ((CbmTofPoint*)(xDigiA->GetLinks()))->GetTrackID() !=
                                      //     ((CbmTofPoint*)(xDigiB->GetLinks()))->GetTrackID() )
                                      // if other side come from a different Track,
                                      // also save the pointer on CbmTofPoint
                                      // vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                    } // if( kTRUE == fDigiBdfPar->ClustUseTrackId() )
                                      // else if( xDigiA->GetLinks() != xDigiB->GetLinks() )
                                      // if other side come from a different TofPoint,
                                      // also save the pointer on CbmTofPoint
                                    //   vPtsRef.push_back( (CbmTofPoint*)(xDigiB->GetLinks()) );
                                 } // else of if( 0 < iNbChanInHit)
                              iLastChan = iCh;
//                              dLastPosX = dPosX; // -> Comment to remove warning because set but never used
                              dLastPosY = dPosY;
                              dLastTime = dTime;
                           } // if( 2 == fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].size() )
                           fStorDigiExp[iSmType][iSm*iNbRpc+iRpc][iCh].clear();
                           fStorDigiInd[iSmType][iSm*iNbRpc+iRpc][iCh].clear();
                        } // for( Int_t iCh = 0; iCh < iNbCh; iCh++ )
			LOG(DEBUG2)<<"finished V-RPC"
				   << Form(" %3d %3d %3d %d %f %fx",iSmType,iSm,iRpc,fTofHitsColl->GetEntries(),dLastPosX,dLastPosY)
				   <<FairLogger::endl;
                     } // else of if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
               } // if( 0 == iChType)
                  else
                  {
                     LOG(ERROR)<<"=> Cluster building "
                           <<"from digis to hits not implemented for pads, Sm type "
                           <<iSmType<<" Rpc "<<iRpc<<FairLogger::endl;
                     return kFALSE;
                  } // else of if( 0 == iChType)

               // Now check if another hit/cluster is started
               // and save it if it's the case
               if( 0 < iNbChanInHit)
               {
                  LOG(DEBUG1)<<"Process cluster " 
                             <<iNbChanInHit<<FairLogger::endl;

                  // Check orientation to properly assign errors
                  if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
                  {
                     LOG(DEBUG1)<<"H-Hit " <<FairLogger::endl;
                  } // if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
                  else
                  {
                     LOG(DEBUG2)<<"V-Hit " <<FairLogger::endl;
                     // Save Hit
                     dWeightedTime /= dWeightsSum;
                     dWeightedPosX /= dWeightsSum;
                     dWeightedPosY /= dWeightsSum;
                     dWeightedPosZ /= dWeightsSum;
                     //TVector3 hitPos(dWeightedPosX, dWeightedPosY, dWeightedPosZ);

                     Double_t hitpos_local[3];
                     hitpos_local[0] = dWeightedPosX;
                     hitpos_local[1] = dWeightedPosY;
                     hitpos_local[2] = dWeightedPosZ;

                     Double_t hitpos[3];
                     TGeoNode*        cNode= gGeoManager->GetCurrentNode();
                     /*TGeoHMatrix* cMatrix =*/ gGeoManager->GetCurrentMatrix();
                     //cNode->Print();
                     //cMatrix->Print();

                     gGeoManager->LocalToMaster(hitpos_local, hitpos);
                     LOG(DEBUG1)<<
                     Form(" LocalToMaster for V-node %p: (%6.1f,%6.1f,%6.1f) ->(%6.1f,%6.1f,%6.1f)", 
                         cNode, hitpos_local[0], hitpos_local[1], hitpos_local[2], 
                         hitpos[0], hitpos[1], hitpos[2])
                             <<FairLogger::endl;

                     TVector3 hitPos(hitpos[0],hitpos[1],hitpos[2]);
                     // TestBeam errors, not properly done at all for now
                     // Right way of doing it should take into account the weight distribution
                     // and real system time resolution
                      TVector3 hitPosErr(0.5,0.5,0.5);  // including positioning uncertainty
                     /*
                     TVector3 hitPosErr( fChannelInfo->GetSizex()/TMath::Sqrt(12.0),   // Single strips approximation
                                       0.5, // Use generic value 
                                       1.);
                     */
                     //                fDigiBdfPar->GetFeeTimeRes() * fDigiBdfPar->GetSigVel(iSmType,iRpc), // Use the electronics resolution
                     //                fDigiBdfPar->GetNbGaps( iSmType, iRpc)*
                     //                fDigiBdfPar->GetGapSize( iSmType, iRpc)/10.0 / // Change gap size in cm
                     //                TMath::Sqrt(12.0) ); // Use full RPC thickness as "Channel" Z size
//                     cout<<"a "<<vPtsRef.size()<<endl;
//                     cout<<"b "<<vPtsRef[0]<<endl;
//                     cout<<"c "<<vPtsRef[0]->GetDetectorID()<<endl;
//                     Int_t iDetId = vPtsRef[0]->GetDetectorID();// detID = pt->GetDetectorID() <= from TofPoint
//                     Int_t iDetId = iChId;
		     Int_t iChm=floor(dWeightedPosX/fChannelInfo->GetSizex())+iNbCh/2;
		     if(iChm<0)        iChm=0;
		     if(iChm >iNbCh-1) iChm=iNbCh-1;
                     Int_t iDetId = CbmTofAddress::GetUniqueAddress(iSm,iRpc,iChm,0,iSmType);
                     Int_t iRefId = 0; // Index of the correspondng TofPoint
                     if(NULL != fTofPointsColl) iRefId = fTofPointsColl->IndexOf( vPtsRef[0] );
                     LOG(DEBUG)<<"Save V-Hit  "
                     << Form(" %3d %3d 0x%08x %3d 0x%08x", // %3d %3d 
                             fiNbHits,iNbChanInHit,iDetId,iLastChan,iRefId) //vPtsRef.size(),vPtsRef[0])
                       //   dWeightedTime,dWeightedPosY)
                                <<", DigiSize: "<<vDigiIndRef.size();
                     LOG(DEBUG)<<", DigiInds: ";

                     fviClusterMul[iSmType][iSm][iRpc]++; 

                     for (UInt_t i=0; i<vDigiIndRef.size();i++){
                       LOG(DEBUG)<<" "<<vDigiIndRef.at(i)<<"(M"<<fviClusterMul[iSmType][iSm][iRpc]<<")";
                     }
                     LOG(DEBUG)  <<FairLogger::endl;
                     
                     if( vDigiIndRef.size() < 2 ){
                      LOG(WARNING)<<"Digi refs for Hit "
                                  << fiNbHits<<":        vDigiIndRef.size()"
                                  <<FairLogger::endl;
                     }        
                     if(fiNbHits>0){
                       CbmTofHit *pHitL = (CbmTofHit*) fTofHitsColl->At(fiNbHits-1);
                       if(iDetId == pHitL->GetAddress() && dWeightedTime==pHitL->GetTime())
                          LOG(DEBUG)<<"Store Hit twice? "
                                    <<" fiNbHits "<<fiNbHits<<", "<<Form("0x%08x",iDetId)
                                    <<FairLogger::endl;
                     }

                     CbmTofHit *pHit =  new CbmTofHit( iDetId,
                                                       hitPos, hitPosErr,  //local detector coordinates
                                                       fiNbHits,  // this number is used as reference!!
                                                       dWeightedTime,
                                                       vPtsRef.size(), // flag  = number of TofPoints generating the cluster
                                                       0) ; //channel
                     //                vDigiIndRef);
		     // output hit
		     new((*fTofHitsColl)[fiNbHits]) CbmTofHit(*pHit);
 		     // memorize hit 
		     if(fdMemoryTime > 0.) {
		       fvLastHits[iSmType][iSm][iRpc][iChm].push_back(pHit);
		       if(dWeightedTime >= fvLastHits[iSmType][iSm][iRpc][iChm].front()->GetTime()){
		         LOG(DEBUG)<<Form(" Store LH from Ev  %8.0f for TSRC %d%d%d%d, size %lu, addr 0x%08x, time %f",
				       fdEvent,
				       iSmType,iSm,iRpc,iChm,fvLastHits[iSmType][iSm][iRpc][iChm].size(),
				       iDetId,
				       dWeightedTime)
			  	   <<FairLogger::endl;
		       }else{
			 Double_t deltaTime=dWeightedTime - fvLastHits[iSmType][iSm][iRpc][iChm].front()->GetTime();
			 LOG(WARNING)<<Form(" insert LH from Ev  %8.0f for TSRC %d%d%d%d, size %lu, addr 0x%08x, delta time %f ",
						       fdEvent,
						       iSmType,iSm,iRpc,iChm,fvLastHits[iSmType][iSm][iRpc][iChm].size(),
						       iDetId,
						       deltaTime)
				     <<FairLogger::endl; 
			 fvLastHits[iSmType][iSm][iRpc][iChm].pop_back();
			 if(deltaTime==0.){
			   // remove hit, otherwise double entry?
			   pHit->Delete();
			 }else{
			   fvLastHits[iSmType][iSm][iRpc][iChm].push_front(pHit);			 
			 }
		       }		       
		     }else{
		       pHit->Delete();
		     }
		     /*
		     new((*fTofDigiMatchColl)[fiNbHits]) CbmMatch();
		     CbmMatch* digiMatch = (CbmMatch *)fTofDigiMatchColl->At(fiNbHits);
		     */
		     CbmMatch* digiMatch = new((*fTofDigiMatchColl)[fiNbHits]) CbmMatch();

                     for (Int_t i=0; i<vDigiIndRef.size();i++){
		       Double_t dTot = ((CbmTofDigiExp*) (fTofCalDigisColl->At(vDigiIndRef.at(i))))->GetTot();
		       digiMatch->AddLink(CbmLink(dTot,vDigiIndRef.at(i)));
		     }

                     fiNbHits++;
                     // For Histogramming
                     fviClusterSize[iSmType][iRpc].push_back(iNbChanInHit);
                     fviTrkMul[iSmType][iRpc].push_back( vPtsRef.size() );
                     fvdX[iSmType][iRpc].push_back(dWeightedPosX);
                     fvdY[iSmType][iRpc].push_back(dWeightedPosY);
                     /*
                     fvdDifX[iSmType][iRpc].push_back( vPtsRef[0]->GetX() - dWeightedPosX);
                     fvdDifY[iSmType][iRpc].push_back( vPtsRef[0]->GetY() - dWeightedPosY);
                     fvdDifCh[iSmType][iRpc].push_back( fGeoHandler->GetCell( vPtsRef[0]->GetDetectorID() ) -1 -iLastChan );
                     */
                     vPtsRef.clear();
                     vDigiIndRef.clear();
                  } // else of if( 1 == fDigiBdfPar->GetChanOrient( iSmType, iRpc ) )
               } // if( 0 < iNbChanInHit)
               LOG(DEBUG2)<<" Fini-A "<<Form(" %3d %3d %3d M%3d",iSmType, iSm, iRpc, fviClusterMul[iSmType][iSm][iRpc])<<FairLogger::endl;
            } // for each sm/rpc pair
               LOG(DEBUG2)<<" Fini-B "<<Form(" %3d ",iSmType)<<FairLogger::endl;
      } // for( Int_t iSmType = 0; iSmType < iNbSmTypes; iSmType++ )
   } // if( kTRUE == fDigiBdfPar->UseExpandedDigi() )
   else
   {
     LOG(ERROR)<<" Compressed Digis not implemented ... "<<FairLogger::endl;
   }
   return kTRUE;
}

Bool_t   CbmTofTestBeamClusterizer::MergeClusters()
{
  // Merge clusters from neigbouring Rpc within a (Super)Module
  if(NULL == fTofHitsColl) {
    LOG(INFO) <<" No Hits defined ! Check! " << FairLogger::endl;
    return kFALSE;
  }
  // inspect hits
  for( Int_t iHitInd = 0; iHitInd < fTofHitsColl->GetEntries(); iHitInd++)
  {
     CbmTofHit *pHit = (CbmTofHit*) fTofHitsColl->At( iHitInd );
     if(NULL == pHit) continue;

     Int_t iDetId = (pHit->GetAddress() & DetMask);
     Int_t iSmType = CbmTofAddress::GetSmType( iDetId );
     Int_t iNbRpc  = fDigiBdfPar->GetNbRpc( iSmType);
     if(iSmType != 5 && iSmType != 8 ) continue; // only merge diamonds and Pad 
     LOG(DEBUG)<<"MergeClusters: in SmT "<< iSmType << " for "<< iNbRpc << " Rpcs"
	       <<FairLogger::endl;

     if(iNbRpc>1) {  // check for possible mergers
       Int_t iSm     = CbmTofAddress::GetSmId( iDetId );
       Int_t iRpc    = CbmTofAddress::GetRpcId( iDetId );
       Int_t iChId = pHit->GetAddress();
       fChannelInfo = fDigiPar->GetCell( iChId );
       Int_t iCh = CbmTofAddress::GetChannelId(iChId);
       LOG(DEBUG)<<"MergeClusters: Check for mergers in "
                <<Form(" SmT %d, Sm %d, Rpc %d, Ch %d - hit %d",iSmType,iSm,iRpc,iCh,iHitInd)
                <<FairLogger::endl;
       for( Int_t iHitInd2 = iHitInd+1; iHitInd2 < fTofHitsColl->GetEntries(); iHitInd2++)
       {
	  CbmTofHit *pHit2 = (CbmTofHit*) fTofHitsColl->At( iHitInd2 );
	  if(NULL == pHit2) continue;
	  Int_t iDetId2    = (pHit2->GetAddress() & DetMask);
	  Int_t iSmType2   = CbmTofAddress::GetSmType( iDetId2 );
	  if(iSmType2 == iSmType) {
	    Int_t iSm2     = CbmTofAddress::GetSmId( iDetId2 );
	    if(iSm2 == iSm || iSmType==5 ){
	      Int_t iRpc2  = CbmTofAddress::GetRpcId( iDetId2 );
	      if(TMath::Abs(iRpc-iRpc2)==1 || iSm2!=iSm){  // Found neighbour 
		Int_t iChId2 = pHit2->GetAddress();
		CbmTofCell  *fChannelInfo2 = fDigiPar->GetCell( iChId2 );
		Int_t iCh2 = CbmTofAddress::GetChannelId(iChId2);
		Double_t xPos=pHit->GetX();
		Double_t yPos=pHit->GetY();
		Double_t tof=pHit->GetTime();
		Double_t xPos2=pHit2->GetX();
		Double_t yPos2=pHit2->GetY();
		Double_t tof2=pHit2->GetTime();		
		LOG(DEBUG)<<"MergeClusters: Found hit in neighbour "
		<<Form(" SmT %d, Sm %d, Rpc %d, Ch %d - hit %d",iSmType2,iSm2,iRpc2,iCh2,iHitInd2)
		<<Form(" DX %6.1f, DY %6.1f, DT %6.1f",xPos-xPos2,yPos-yPos2,tof-tof2)
		<<FairLogger::endl;


                if(      TMath::Abs(xPos-xPos2)<fdCaldXdYMax*2.
                      && TMath::Abs(yPos-yPos2)<fdCaldXdYMax*2.
                      && TMath::Abs(tof-tof2)<fMaxTimeDist ){

                  CbmMatch* digiMatch=(CbmMatch *)fTofDigiMatchColl->At(iHitInd);
                  Double_t dTot=0;
                  for (Int_t iLink=0; iLink<digiMatch->GetNofLinks(); iLink+=2){  // loop over digis
                    CbmLink L0 = digiMatch->GetLink(iLink);  
                    Int_t iDigInd0=L0.GetIndex(); 
                    Int_t iDigInd1=(digiMatch->GetLink(iLink+1)).GetIndex(); 
                    if (iDigInd0 < fTofCalDigisColl->GetEntries() && iDigInd1 < fTofCalDigisColl->GetEntries()){
                      CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofCalDigisColl->At(iDigInd0));
                      CbmTofDigiExp *pDig1 = (CbmTofDigiExp*) (fTofCalDigisColl->At(iDigInd1));
                      dTot += pDig0->GetTot(); 
                      dTot += pDig1->GetTot();
                    } 
                  }

                  CbmMatch* digiMatch2=(CbmMatch *)fTofDigiMatchColl->At(iHitInd2);
                  Double_t dTot2=0;
                  for (Int_t iLink=0; iLink<digiMatch2->GetNofLinks(); iLink+=2){  // loop over digis
                    CbmLink L0 = digiMatch2->GetLink(iLink);  
                    Int_t iDigInd0=L0.GetIndex(); 
                    Int_t iDigInd1=(digiMatch2->GetLink(iLink+1)).GetIndex(); 
                    if (iDigInd0 < fTofCalDigisColl->GetEntries() && iDigInd1 < fTofCalDigisColl->GetEntries()){
                      CbmTofDigiExp *pDig0 = (CbmTofDigiExp*) (fTofCalDigisColl->At(iDigInd0));
                      CbmTofDigiExp *pDig1 = (CbmTofDigiExp*) (fTofCalDigisColl->At(iDigInd1));
                      dTot2 += pDig0->GetTot(); 
                      dTot2 += pDig1->GetTot();
                      digiMatch->AddLink(CbmLink(pDig0->GetTot(),iDigInd0));
                      digiMatch->AddLink(CbmLink(pDig1->GetTot(),iDigInd1));
                    } 
                  }
                  LOG(DEBUG)<<"MergeClusters: Found merger in neighbour "
                            <<Form(" SmT %d, Sm %d, Rpc %d, Ch %d - hit %d(%d)",
                                   iSmType2,iSm2,iRpc2,iCh2,iHitInd2,fTofHitsColl->GetEntries())
                            <<Form(" DX %6.1f, DY %6.1f, DT %6.1f",xPos-xPos2,yPos-yPos2,tof-tof2)
                            <<Form(" Tots %6.1f - %6.1f", dTot, dTot2) 
                            <<FairLogger::endl;
                  Double_t dTotSum=dTot+dTot2;
                  Double_t dxPosM = (xPos*dTot + xPos2*dTot2)/dTotSum;
                  Double_t dyPosM = (yPos*dTot + yPos2*dTot2)/dTotSum;
                  Double_t dtofM  = (tof*dTot  + tof2*dTot2) /dTotSum;
                  pHit->SetX(dxPosM);
                  pHit->SetY(dyPosM);
                  pHit->SetTime(dtofM);

                  // remove merged hit at iHitInd2 and update digiMatch

                  fTofHitsColl->RemoveAt( iHitInd2 );
                  fTofDigiMatchColl->RemoveAt( iHitInd2 );
                  fTofDigiMatchColl->Compress();
                  fTofHitsColl->Compress();
                  LOG(DEBUG)<<"MergeClusters: Compress TClonesArrays to "
                            <<fTofHitsColl->GetEntries()<<", "
                            <<fTofDigiMatchColl->GetEntries()
                            <<FairLogger::endl;
		  /*
                  for(Int_t i=iHitInd2; i<fTofHitsColl->GetEntries(); i++){ // update RefLinks
                     CbmTofHit *pHiti = (CbmTofHit*) fTofHitsColl->At( i );
                    pHiti->SetRefId(i);
                  }
		  */
                  //check merged hit (cluster)
                  //pHit->Print();
                }
              }
            }
          }
       }
     }
  }
  return kTRUE;
}

static Double_t f1_xboxe(double *x, double *par){
  double xx    = x[0];
  double wx    = 1. - par[4]*TMath::Power(xx+par[5],2);
  double xboxe = par[0]*0.25*(1.+TMath::Erf(( xx+par[1]-par[3])/par[2]))
                            *(1.+TMath::Erf((-xx+par[1]+par[3])/par[2]));
  return xboxe*wx ;
}

void CbmTofTestBeamClusterizer::fit_ybox(const char *hname)
{
 TH1 *h1;
 h1=(TH1 *)gROOT->FindObjectAny(hname);
 if(NULL != h1){
   fit_ybox(h1,0.);
 }
}

void CbmTofTestBeamClusterizer::fit_ybox(TH1 *h1, Double_t ysize)
{
   TAxis *xaxis = h1->GetXaxis();
   Double_t Ymin= xaxis->GetXmin();
   Double_t Ymax= xaxis->GetXmax();
   TF1 *f1=new TF1("YBox",f1_xboxe,Ymin,Ymax,6);
   Double_t yini=(h1->GetMaximum()+h1->GetMinimum())*0.5;
   if (ysize==0.) ysize=Ymax*0.8;
   f1->SetParameters(yini,ysize,1.,0.,0.,0.);
   h1->Fit("YBox","Q");

   double res[10];
   double err[10];
   res[9]=f1->GetChisquare();

   for (int i=0; i<6; i++)
   {
     res[i]=f1->GetParameter(i);
     err[i]=f1->GetParError(i);
     //cout << " FPar "<< i << ": " << res[i] << ", " << err[i] << endl;  
   }
   LOG(INFO) << "YBox Fit of "<<h1->GetName()<<" ended with chi2 = "<<res[9]
	     << Form(", strip length %7.2f +/- %5.2f, position resolution %7.2f +/- %5.2f at y_cen = %7.2f +/- %5.2f",
	 	 2.*res[1],2.*err[1],res[2],err[2],res[3],err[3])
	      << FairLogger::endl;
}

void CbmTofTestBeamClusterizer::CheckLHMemory()
{
  if(fvLastHits.size() != fDigiBdfPar->GetNbSmTypes())
    LOG(FATAL)<<Form("Inconsistent LH Smtype size %lu, %d ",fvLastHits.size(),fDigiBdfPar->GetNbSmTypes())
	      <<FairLogger::endl;

  for (Int_t iSmType=0; iSmType<fDigiBdfPar->GetNbSmTypes(); iSmType++ ){
    if(fvLastHits[iSmType].size() != fDigiBdfPar->GetNbSm( iSmType ))
      LOG(FATAL)<<Form("Inconsistent LH Sm size %lu, %d T %d",fvLastHits[iSmType].size(),fDigiBdfPar->GetNbSm( iSmType ), iSmType)
		<<FairLogger::endl;
    for( Int_t iSm = 0; iSm < fDigiBdfPar->GetNbSm(  iSmType); iSm++ ){
      if(fvLastHits[iSmType][iSm].size() != fDigiBdfPar->GetNbRpc( iSmType ))
	LOG(FATAL)<<Form("Inconsistent LH Rpc size %lu, %d TS %d%d ",fvLastHits[iSmType][iSm].size(),fDigiBdfPar->GetNbRpc( iSmType ),iSmType,iSm)
		  <<FairLogger::endl;
      for( Int_t iRpc = 0; iRpc < fDigiBdfPar->GetNbRpc( iSmType); iRpc++ ){
	if(fvLastHits[iSmType][iSm][iRpc].size() != fDigiBdfPar->GetNbChan( iSmType, iRpc ))
	  LOG(FATAL)<<Form("Inconsistent LH RpcChannel size %lu, %d TSR %d%d%d ",fvLastHits[iSmType][iSm][iRpc].size(),
			   fDigiBdfPar->GetNbChan( iSmType, iRpc ),iSmType,iSm,iRpc)
		    <<FairLogger::endl;	
	for (Int_t iCh=0; iCh< fDigiBdfPar->GetNbChan( iSmType, iRpc ); iCh++ )
	  if(fvLastHits[iSmType][iSm][iRpc][iCh].size()>0){
	    CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh);
	    Int_t iAddr = fTofId->SetDetectorInfo( xDetInfo );	     
	    if( fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetAddress() != iAddr )
	      LOG(FATAL)<<Form("Inconsistent address for Ev %8.0f in list of size %lu for TSRC %d%d%d%d: 0x%08x, time  %f",
			       fdEvent,
			       fvLastHits[iSmType][iSm][iRpc][iCh].size(),
			       iSmType,iSm,iRpc,iCh,
			       fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetAddress(),
			       fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetTime())
		        <<FairLogger::endl;
	  }
      }
    }
  }
  LOG(DEBUG) << Form("LH check passed for event %8.0f",fdEvent)
	     <<FairLogger::endl;       
}

void CbmTofTestBeamClusterizer::CleanLHMemory()
{
  if(fvLastHits.size() != fDigiBdfPar->GetNbSmTypes())
    LOG(FATAL)<<Form("Inconsistent LH Smtype size %lu, %d ",fvLastHits.size(),fDigiBdfPar->GetNbSmTypes())
	      <<FairLogger::endl;
  for (Int_t iSmType=0; iSmType<fDigiBdfPar->GetNbSmTypes(); iSmType++ ){
    if(fvLastHits[iSmType].size() != fDigiBdfPar->GetNbSm( iSmType ))
      LOG(FATAL)<<Form("Inconsistent LH Sm size %lu, %d T %d",fvLastHits[iSmType].size(),fDigiBdfPar->GetNbSm( iSmType ), iSmType)
		<<FairLogger::endl;
    for( Int_t iSm = 0; iSm < fDigiBdfPar->GetNbSm(  iSmType); iSm++ ){
      if(fvLastHits[iSmType][iSm].size() != fDigiBdfPar->GetNbRpc( iSmType ))
	LOG(FATAL)<<Form("Inconsistent LH Rpc size %lu, %d TS %d%d ",fvLastHits[iSmType][iSm].size(),fDigiBdfPar->GetNbRpc( iSmType ),iSmType,iSm)
		  <<FairLogger::endl;
      for( Int_t iRpc = 0; iRpc < fDigiBdfPar->GetNbRpc( iSmType); iRpc++ ){
	if(fvLastHits[iSmType][iSm][iRpc].size() != fDigiBdfPar->GetNbChan( iSmType, iRpc ))
	  LOG(FATAL)<<Form("Inconsistent LH RpcChannel size %lu, %d TSR %d%d%d ",fvLastHits[iSmType][iSm][iRpc].size(),
			   fDigiBdfPar->GetNbChan( iSmType, iRpc ),iSmType,iSm,iRpc)
		    <<FairLogger::endl;	
	for (Int_t iCh=0; iCh< fDigiBdfPar->GetNbChan( iSmType, iRpc ); iCh++ )
	  while(fvLastHits[iSmType][iSm][iRpc][iCh].size()>0){
	    CbmTofDetectorInfo xDetInfo(kTOF, iSmType, iSm, iRpc, 0, iCh);
	    Int_t iAddr = fTofId->SetDetectorInfo( xDetInfo );	     
	    if( fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetAddress() != iAddr )
	      LOG(FATAL)<<Form("Inconsistent address for Ev %8.0f in list of size %lu for TSRC %d%d%d%d: 0x%08x, time  %f",
			       fdEvent,
			       fvLastHits[iSmType][iSm][iRpc][iCh].size(),
			       iSmType,iSm,iRpc,iCh,
			       fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetAddress(),
			       fvLastHits[iSmType][iSm][iRpc][iCh].front()->GetTime())
		        <<FairLogger::endl;
	    fvLastHits[iSmType][iSm][iRpc][iCh].front()->Delete();
	    fvLastHits[iSmType][iSm][iRpc][iCh].pop_front();
	  }
      }
    }
  }
  LOG(INFO) << Form("LH cleaning done after %8.0f events",fdEvent)
	    << FairLogger::endl;       
}

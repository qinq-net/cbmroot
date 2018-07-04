/** @file CbmTrdRecoQaTb.cxx
 ** @author Alex Bercuci <abercuci@niham.nipne.ro>
 ** @date 19.12.2017
 **/

#include <utility>
#include "CbmTrdRecoQaTb.h"
#include "CbmHistManager.h"
#include "FairRootManager.h"
#include "FairLogger.h"
#include "CbmTrdHit.h"
#include "CbmMatch.h"
#include "CbmTrdCluster.h"
#include "CbmTrdDigi.h"
#include "CbmTrdAddress.h"
#include "CbmMCDataManager.h"
#include "CbmMCBuffer.h"
#include "CbmSimulationReport.h"
//#include "CbmTrdRecoQaTbReport.h"
//#include "CbmTrdSetup.h"

#include "TClonesArray.h"
#include "TH1.h"
#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TProfile.h"
#include "TProfile2D.h"

using std::cout;
using std::vector;
using std::set;
using std::map;
using std::pair;
using std::string;

CbmTrdRecoQaTb::CbmTrdRecoQaTb():
    fHM(NULL)
  , fOutputDir(" ")
//  , fSetup(NULL)
  , fTimeSlice(NULL)
  , fTrdDigis(NULL)
  , fTrdClusters(NULL)
  , fTrdHits(NULL)
  , fTrdClusterMatches(NULL)
  , fTrdHitMatches(NULL)
  , fTrdPoints(NULL)
  , fTrdDigiData()
  , fDaq(kTRUE)
  , fMaxScale(0)
  , fMCinCell()
  , fHitsinCell()
  , fEffinCell()
{

}

CbmTrdRecoQaTb::~CbmTrdRecoQaTb()
{
  if ( fHM ) delete fHM;
}

InitStatus CbmTrdRecoQaTb::Init()
{
  fHM = new CbmHistManager();

  ReadDataBranches();

//  fSetup = CbmTrdSetup::Instance();

  string type;
  if ( fDaq )
    type = "TimeSlice";
  else
    type = "Event";

  CreateHistograms(type);

  return kSUCCESS;
}

void CbmTrdRecoQaTb::Exec(Option_t* /*opt*/)
{
  string type;
  if ( fDaq ) {
    type = "TimeSlice";
    ProcessDigisAndPoints(fTrdDigis, fTrdPoints, type);
  }
  else {
    type = "Event";
    ProcessDigisAndPoints(fTrdDigis, fTrdPoints, type);
  }
  ProcessClusters(fTrdClusters, fTrdClusterMatches, fTrdPoints, type);
  ProcessHits(fTrdHits, fTrdHitMatches, type);

  FillResidualAndPullHistograms(fTrdPoints, fTrdHits, fTrdHitMatches, type);

  fHM->H1("hen_EventNo_TimeBasedQa")->Fill(0.5);
}

void CbmTrdRecoQaTb::Finish()
{
  fHM->WriteToFile();
  //CbmSimulationReport* report = new CbmTrdRecoQaTbReport(fDaq);
  //report->Create(fHM, fOutputDir);
  //delete report;

  //test
  string type;
  if ( fDaq )
    type = "TimeSlice";
  else
    type = "Event";
  Double_t matchedHits = 100. * (Double_t) fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral() /
		  (Double_t) fHM->H1("hno_NofObjects_Hits_Station_" + type)->Integral();
  Double_t efficiency = 100 * (Double_t) fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral() /
		  (Double_t) fHM->H1("hno_NofObjects_Points_Station_" + type)->Integral();
  Double_t ghost = 100 * ((Double_t) fHM->H1("hno_NofObjects_Hits_Station_" + type)->Integral() -
		  (Double_t) fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral()) /
		  (Double_t) fHM->H1("hno_NofObjects_Points_Station_" + type)->Integral();
  Double_t fake = 100. * ((Double_t) fHM->H1("hno_NofObjects_Hits_Station_" + type)->Integral() -
		  (Double_t) fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral()) /
		  (Double_t) fHM->H1("hno_NofObjects_Hits_Station_" + type)->Integral();
  std::cout<<" -I- CbmTrdRecoQaTb: Hits: "<<fHM->H1("hno_NofObjects_Hits_Station_" + type)->Integral()
		  <<"\n -I- CbmTrdRecoQaTb: MatchedHits: "<<fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Integral()
		  <<"\n -I- CbmTrdRecoQaTb: MatchedHits: "<<matchedHits<<" %"
		  <<"\n -I- CbmTrdRecoQaTb: Efficiency : "<<efficiency<<" %"
		  <<"\n -I- CbmTrdRecoQaTb: Ghost      : "<<ghost<<" %"
		  <<"\n -I- CbmTrdRecoQaTb: Fake       : "<<fake<<" %";
}

void CbmTrdRecoQaTb::ReadDataBranches()
{
  FairRootManager* ioman = FairRootManager::Instance();
  if ( NULL == ioman )
    LOG(FATAL) << GetName() << ": No FairRootManager!" << FairLogger::endl;

  CbmMCDataManager* mcManager = (CbmMCDataManager*) ioman->GetObject("MCDataManager");
  if ( NULL == mcManager )
    LOG(FATAL) << GetName() << ": No CbmMCDataManager!" << FairLogger::endl;
  fTrdPoints = mcManager->InitBranch("StsPoint");

  if ( fDaq ) {
    fTimeSlice = (CbmTimeSlice*) ioman->GetObject("TimeSlice.");
    if ( NULL == fTimeSlice )
      LOG(FATAL) << GetName() << ": No TimeSlice data!" << FairLogger::endl;
  }
  else {
    fTrdDigis = (TClonesArray*) ioman->GetObject("StsDigi");
    if ( NULL == fTrdDigis )
      LOG(ERROR) << GetName() << ": No StsDigi array!" << FairLogger::endl;
  }

  fTrdClusters = (TClonesArray*) ioman->GetObject("StsCluster");
  if ( NULL == fTrdClusters )
    LOG(ERROR) << GetName() << ": No StsCluster array!" << FairLogger::endl;

  fTrdHits = (TClonesArray*) ioman->GetObject("StsHit");
  if ( NULL == fTrdHits )
    LOG(ERROR) << GetName() << ": No StsHit array!" << FairLogger::endl;

  fTrdClusterMatches = (TClonesArray*) ioman->GetObject("StsClusterMatch");
  if ( NULL == fTrdClusterMatches )
    LOG(ERROR) << GetName() << ": No StsClusterMatch array!" << FairLogger::endl;

  fTrdHitMatches = (TClonesArray*) ioman->GetObject("StsHitMatch");
  if ( NULL == fTrdHitMatches )
    LOG(ERROR) << GetName() << ": No StsHitMatch array!" << FairLogger::endl;
}

void CbmTrdRecoQaTb::CreateHistograms(const string& type)
{
  CreateNofObjectsHistograms(type);
  CreateHitParametersHistograms(type);
  Create2dHistograms(type);

  fHM->Create1<TH1F>("hen_EventNo_TimeBasedQa", "hen_EventNo_TimeBasedQa", 1, 0, 1.);
}

void CbmTrdRecoQaTb::CreateNofObjectsHistograms(const string& type)
{
  Int_t nofBins = 10000;
  Double_t minX = -0.5;
  Double_t maxX = 999999.5;
  string name = "hno_NofObjects_";
  fHM->Create1<TH1F>(name + "Points_" + type, name + "Points_" + type + ";Points per " + type + ";Counter", nofBins, minX, maxX);
  fHM->Create1<TH1F>(name + "Digis_" + type, name + "Digis_" + type + ";Digis per " + type + ";Counter", nofBins, minX, maxX);
  fHM->Create1<TH1F>(name + "Clusters_" + type, name + "Clusters_" + type + ";Clusters per " + type + ";Counter", nofBins, minX, maxX);
  fHM->Create1<TH1F>(name + "Hits_" + type, name + "Hits_" + type + ";Hits per " + type + ";Counter", nofBins, minX, maxX);
  fHM->Create1<TH1F>(name + "MatchedHits_" + type, name + "MatchedHits_" + type + ";MatchedHits per " + type + ";Counter", nofBins, minX, maxX);
  nofBins = 100;
  minX = -0.5;
  maxX = 99.5;
  fHM->Create1<TH1F>(name + "Points_Station_" + type, name + "Points_Station_" + type + ";Station number;Points per " + type, nofBins, minX, maxX);
  fHM->Create1<TH1F>(name + "Digis_Station_" + type, name + "Digis_Station_" + type + ";Station number;Digis per " + type, nofBins, minX, maxX);
  fHM->Create1<TH1F>(name + "Clusters_Station_" + type, name + "Clusters_Station_" + type + ";Station number;Clusters per " + type, nofBins, minX, maxX);
  fHM->Create1<TH1F>(name + "Hits_Station_" + type, name + "Hits_Station_" + type + ";Station number;Hits per " + type, nofBins, minX, maxX);
  fHM->Create1<TH1F>(name + "MatchedHits_Station_" + type, name + "MatchedHits_Station_" + type + ";Station number;MatchedHits per " + type, nofBins, minX, maxX);
}

void CbmTrdRecoQaTb::CreateHitParametersHistograms(const string& type)
{
  Int_t nofBins = 100;
  Double_t minX = -5.0;
  Double_t maxX = 5.0;
  fHM->Create1<TH1F>("hrp_Pull_X_" + type, "Pull_X_" + type + ";Pull;Yield", nofBins, minX, maxX);
  minX = -50.0;
  maxX = 50.0;
  fHM->Create1<TH1F>("hrp_Pull_Y_" + type, "Pull_Y_" + type + ";Pull;Yield", nofBins, minX, maxX);
  minX = -60.0;
  maxX = 60.0;
  fHM->Create1<TH1F>("hrp_Residual_X_" + type, "Residual_X_" + type + ";Residual [#mum];Yield", nofBins, minX, maxX);
  minX = -600.0;
  maxX = 600.0;
  fHM->Create1<TH1F>("hrp_Residual_Y_" + type, "Residual_Y_" + type + ";Residual [#mum];Yield", nofBins, minX, maxX);
  nofBins = 25;	//TODO test and reset this number
  minX = 0.5;
  maxX = nofBins + minX;
  fHM->Create1<TH1F>("hhp_PointsInHit_" + type, "PointsInHit;Number of Points;Yield"
		  , nofBins, minX, maxX);
  fHM->Create1<TH1F>("hhp_PointsInMatchedHit_" + type, "PointsInMatchedHit;Number of Points;Yield"
		  , nofBins, minX, maxX);
  fHM->Create1<TH1F>("hpa_PointsInCluster_" + type, "PointsInCluster;Number of Points;Yield"
		  , nofBins, minX, maxX);
  fHM->Create1<TH1F>("hpa_PointsInDigi_" + type, "PointsInDigi;Number of Points;Yield"
		  , nofBins, minX, maxX);
  fHM->Create1<TH1F>("hdo_DigisByPoint_" + type, "DigisByPoint;Number of Digis;Yield"
		  , 50, 0.5, 50.5);
  fHM->Create1<TH1F>("hdo_DigisInCluster_" + type, "DigisInCluster;Number of Digis;Yield"
		  , 50, 0.5, 50.5);
  fHM->Create1<TH1F>("hpa_ClusterSize_" + type, "Cluster size;Size of cluster;Yield"
  		  , 100, 0.5, 100.5);
  fHM->Create1<TH1F>("hce_PointsInCells", "PointInCells;Number of Points in time cell;Yield"
		  , 20, -0.5, 19.5);
  fHM->Create1<TH1F>("hce_HitsInCells", "HitsInCells;Number of Hits in time cell;Yield"
		  , 20, -0.5, 19.5);
  fHM->Create1<TH1F>("hce_EffInCells", "EffInCells;Hit density [x1000 Hits per 100 ns];Efficiency"
		  , 20, -0.5, 19.5);
}

void CbmTrdRecoQaTb::Create2dHistograms(const string& type)
{
  Int_t nofBins = 100;
  Int_t nofBinsClusterSize = 10;
  Int_t nofBinsA = 90;
  Double_t minX = -60.0;
  Double_t maxX = 60.0;
  Double_t minA = 0.;
  Double_t maxA = 90.;
  
  fHM->Create2<TH2F>("h2d_Residual_X_vs_ClusterSize_" + type, 
	  "Residual_X_vs_ClusterSize_" + type + ";Cluster Size;Residual [#mum];", 
	  nofBinsClusterSize, 0.5, nofBinsClusterSize+0.5, nofBins, minX, maxX);
  fHM->Create2<TH2F>("h2d_ClusterSize_vs_SlopeX_" + type, 
	  "ClusterSize_vs_SlopeX_" + type + ";Slope X [deg];Cluster Size;", 
	  nofBinsA, minA, maxA, nofBinsClusterSize, 0.5, nofBinsClusterSize+0.5);
  fHM->Create2<TH2F>("h2d_Residual_X_vs_SlopeX_" + type, 
	  "Residual_X_vs_SlopeX_" + type + ";Slope X [deg];Residual [#mum];", 
	  nofBinsA, minA, maxA, nofBins, minX, maxX);
}

//_______________________________________________________________________________
void CbmTrdRecoQaTb::ProcessDigisAndPoints(const vector<CbmTrdDigi> digis, CbmMCDataArray* /*points*/, const string& type)
{
  printf("ProcessDigisAndPoints : digis[%d] type[%s]\n", (Int_t)digis.size(), type.c_str());
  Int_t CellSize = 100;
  Int_t nCells = 1;
  if (fDaq) nCells = fTimeSlice->GetDuration() / CellSize;
  fMCinCell.resize(nCells);
  for ( Int_t i = 0; i < nCells; i++ ) fMCinCell[i] = 0;

  if ( digis.size() > 0 && fHM->Exists("hno_NofObjects_Digis_" + type) )
    fHM->H1("hno_NofObjects_Digis_" + type)->Fill(digis.size());
  std::set<Double_t> pointIndexes;
  std::map<Double_t, Int_t> stations;
  std::map<Double_t, Int_t> digisByPoint;
  std::map<Double_t, Int_t>::iterator map_it;
  pointIndexes.clear();
  for(UInt_t iDigi = 0; iDigi < digis.size(); iDigi++) {
    const CbmTrdDigi trdDigi = digis[iDigi];
    const CbmMatch* digiMatch = static_cast<const CbmMatch*>(trdDigi.GetMatch());
    Int_t stationId = 0;//fSetup->GetStationNumber(stsDigi.GetAddress());
    for(Int_t iLink = 0; iLink < digiMatch->GetNofLinks(); iLink++) {
      const CbmLink link = digiMatch->GetLink(iLink);
      Double_t index = (1000 * link.GetIndex()) + (link.GetFile()) + (0.0001 * link.GetEntry());

      if ( pointIndexes.count(index) == 0 ) {
//        CbmTrdPoint* point = (CbmTrdPoint*) points->Get(link.GetFile(), link.GetEntry() - 1, link.GetIndex());
        Double_t timeD = trdDigi.GetTime();
        Int_t iCell = 0;
        if (fDaq) iCell = (timeD - fTimeSlice->GetStartTime()) / CellSize;
        fMCinCell[iCell]++;
      }

      pointIndexes.insert(index);
      stations.insert(std::pair<Double_t, Int_t>(index, stationId));
      Int_t channel = 0;//trdDigi.GetChannel();
      Int_t side = channel < 1024 ? 0 : 1;
      map_it = digisByPoint.find(index + (side * 0.00001));
      if ( map_it != digisByPoint.end() ) {
        map_it->second++;
      }
      else {
        digisByPoint.insert(std::pair<Double_t, Int_t>(index + (side * 0.00001), 1));
      }
    }
    fHM->H1("hno_NofObjects_Digis_Station_" + type)->Fill(stationId);
    fHM->H1("hpa_PointsInDigi_" + type)->Fill(digiMatch->GetNofLinks());
  }
  fHM->H1("hno_NofObjects_Points_" + type)->Fill(pointIndexes.size());
  std::set<Double_t>::iterator set_it;
  for(set_it = pointIndexes.begin(); set_it != pointIndexes.end(); ++set_it) {
    fHM->H1("hno_NofObjects_Points_Station_" + type)->Fill(stations[*set_it]);
    fHM->H1("hdo_DigisByPoint_" + type)->Fill(digisByPoint[*set_it]);
    fHM->H1("hdo_DigisByPoint_" + type)->Fill(digisByPoint[*set_it + 0.00001]);
  }
  if ( pointIndexes.size() > static_cast<size_t>(fMaxScale) ) fMaxScale = pointIndexes.size();
}

void CbmTrdRecoQaTb::ProcessDigisAndPoints(const TClonesArray* digis, const CbmMCDataArray* /*points*/, const string& type)
{
  if ( NULL != digis && fHM->Exists("hno_NofObjects_Digis_" + type) )
    fHM->H1("hno_NofObjects_Digis_" + type)->Fill(digis->GetEntriesFast());
  std::set<Double_t> pointIndexes;
  std::map<Double_t, Int_t> stations;
  std::map<Double_t, Int_t> digisByPoint;
  std::map<Double_t, Int_t>::iterator map_it;
  pointIndexes.clear();
  for(Int_t iDigi = 0; iDigi < digis->GetEntriesFast(); iDigi++) {
    const CbmTrdDigi* stsDigi = static_cast<const CbmTrdDigi*>(digis->At(iDigi));
    const CbmMatch* digiMatch = static_cast<const CbmMatch*>(stsDigi->GetMatch());
    Int_t stationId = 0;//fSetup->GetStationNumber(stsDigi->GetAddress());
    for(Int_t iLink = 0; iLink < digiMatch->GetNofLinks(); iLink++) {
      const CbmLink link = digiMatch->GetLink(iLink);
      Double_t index = (1000 * link.GetIndex()) + (link.GetFile()) + (0.0001 * link.GetEntry());
      pointIndexes.insert(index);
      stations.insert(std::pair<Double_t, Int_t>(index, stationId));
      Int_t channel = 0;//stsDigi->GetChannel();
      Int_t side = channel < 1024 ? 0 : 1;
      map_it = digisByPoint.find(index + (side * 0.00001));
      if ( map_it != digisByPoint.end() ) {
        map_it->second++;
      }
      else {
        digisByPoint.insert(std::pair<Double_t, Int_t>(index + (side * 0.00001), 1));
      }
    }
    fHM->H1("hno_NofObjects_Digis_Station_" + type)->Fill(stationId);
    fHM->H1("hpa_PointsInDigi_" + type)->Fill(digiMatch->GetNofLinks());
  }
  fHM->H1("hno_NofObjects_Points_" + type)->Fill(pointIndexes.size());
  std::set<Double_t>::iterator set_it;
  for(set_it = pointIndexes.begin(); set_it != pointIndexes.end(); ++set_it) {
    fHM->H1("hno_NofObjects_Points_Station_" + type)->Fill(stations[*set_it]);
    fHM->H1("hdo_DigisByPoint_" + type)->Fill(digisByPoint[*set_it]);
    fHM->H1("hdo_DigisByPoint_" + type)->Fill(digisByPoint[*set_it + 0.00001]);
  }
  if ( pointIndexes.size() > static_cast<size_t>(fMaxScale) ) fMaxScale = pointIndexes.size();
}

void CbmTrdRecoQaTb::ProcessClusters(const TClonesArray* clusters, const TClonesArray* clusterMatches, CbmMCDataArray* points, const string& type)
{
  if ( NULL != clusters && fHM->Exists("hno_NofObjects_Clusters_" + type) )
    fHM->H1("hno_NofObjects_Clusters_" + type)->Fill(clusters->GetEntriesFast());
  for(Int_t iCluster = 0; iCluster < clusters->GetEntriesFast(); iCluster++) {
    const CbmTrdCluster* stsCluster = static_cast<const CbmTrdCluster*>(clusters->At(iCluster));
    const CbmMatch* stsClusterMatch = static_cast<const CbmMatch*>(clusterMatches->At(iCluster));
    Int_t stationId = 0;//fSetup->GetStationNumber(stsCluster->GetAddress());
    fHM->H1("hno_NofObjects_Clusters_Station_" + type)->Fill(stationId);
    if ( NULL != clusters && fHM->Exists("hdo_DigisInCluster_" + type) )
      fHM->H1("hdo_DigisInCluster_" + type)->Fill(stsCluster->GetNofDigis());
    if ( NULL != clusterMatches && fHM->Exists("hpa_PointsInCluster_" + type) )
      fHM->H1("hpa_PointsInCluster_" + type)->Fill(stsClusterMatch->GetNofLinks());
    if ( NULL != clusters && (fHM->Exists("hpa_ClusterSize_" + type) || fHM->Exists("h2d_ClusterSize_vs_SlopeX_" + type) )){
      fHM->H1("hpa_ClusterSize_" + type)->Fill(0/*stsCluster->GetSize()*/);
      Double_t slope = -1000.;
      for(Int_t iLink = 0; iLink < stsClusterMatch->GetNofLinks(); iLink++) {
          const CbmLink& link = static_cast<const CbmLink&>(stsClusterMatch->GetLink(iLink));
          CbmTrdPoint* point = (CbmTrdPoint*) points -> Get(link.GetFile(), link.GetEntry() - 1, link.GetIndex());
          slope = TMath::ATan(point -> GetPx() / point -> GetPz()) * 180. / 3.1416;
          fHM->H2("h2d_ClusterSize_vs_SlopeX_" + type)->Fill(slope, 0/*stsCluster->GetSize()*/);
      }
    }
  }
  if ( clusters->GetEntriesFast() > fMaxScale ) fMaxScale = clusters->GetEntriesFast();
}

void CbmTrdRecoQaTb::ProcessHits(const TClonesArray* hits, const TClonesArray* hitMatches, const string& type)
{	//TODO Add histogram for matched Points
  Int_t CellSize = 100;
  Int_t nCells = 1;
  if(fDaq) nCells = fTimeSlice->GetDuration() / CellSize;
  fHitsinCell.resize(nCells);
  fEffinCell.resize(nCells);
  for ( Int_t i = 0; i < nCells; i++ ) {
	  fHitsinCell[i] = 0;
	  fEffinCell[i] = 0;
  }

  if ( NULL != hits && fHM->Exists("hno_NofObjects_Hits_" + type) )
    fHM->H1("hno_NofObjects_Hits_" + type)->Fill(hits->GetEntriesFast());
  Int_t nofMatchedHits = 0;
  vector<CbmLink> links;
  links.clear();
//  Int_t nofDoubledHits = 0;
  for(Int_t iHit = 0; iHit < hits->GetEntriesFast(); iHit++) {
    const CbmTrdHit* hit = static_cast<CbmTrdHit*>(hits->At(iHit));
    const CbmMatch* hitMatch = static_cast<CbmMatch*>(hitMatches->At(iHit));
    Int_t stationId = 0;//fSetup->GetStationNumber(hit->GetAddress());
    fHM->H1("hno_NofObjects_Hits_Station_" + type)->Fill(stationId);
    fHM->H1("hhp_PointsInHit_" + type)->Fill(hitMatch->GetNofLinks());
    if ( hitMatch->GetNofLinks() == 1 ) {
      const CbmLink& link = static_cast<const CbmLink&>(hitMatch->GetLink(0));
      Bool_t d = kFALSE;
      for(UInt_t i = 0; i < links.size(); i++) {
    	  if ( link == links[i] ) {
    		  d = kTRUE;
    		  break;
    	  }
      }
      if ( !d ){
        links.push_back(link);
        fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Fill(stationId);
        fHM->H1("hhp_PointsInMatchedHit_" + type)->Fill(hitMatch->GetNofLinks());
        nofMatchedHits++;

        Int_t iCell = 0;
        if (fDaq) iCell = (hit->GetTime() - fTimeSlice->GetStartTime()) / CellSize;
        fHitsinCell[iCell]++;
      }
    }
    else {
      const CbmMatch* frontClusterMatch = static_cast<const CbmMatch*>(fTrdClusterMatches->At(0/*hit->GetFrontClusterId()*/));
      const CbmMatch* backClusterMatch = static_cast<const CbmMatch*>(fTrdClusterMatches->At(0/*hit->GetBackClusterId()*/));
      Bool_t matched = kFALSE;
      for(Int_t iFrontLink = 0; iFrontLink < frontClusterMatch->GetNofLinks(); iFrontLink++) {
        if ( matched ) break;
        const CbmLink& frontLink = frontClusterMatch->GetLink(iFrontLink);
        for(Int_t iBackLink = 0; iBackLink < backClusterMatch->GetNofLinks(); iBackLink++) {
          if ( matched ) break;
          const CbmLink backLink = backClusterMatch->GetLink(iBackLink);
          if ( frontLink == backLink ) {
            Bool_t d = kFALSE;
            for(UInt_t i = 0; i < links.size(); i++) {
          	  if ( frontLink == links[i] ) {
                d = kTRUE;
                break;
              }
            }
            if ( !d ) {
        	  links.push_back(frontLink);

        	  fHM->H1("hno_NofObjects_MatchedHits_Station_" + type)->Fill(stationId);
        	  fHM->H1("hhp_PointsInMatchedHit_" + type)->Fill(hitMatch->GetNofLinks());
        	  nofMatchedHits++;
        	  matched = kTRUE;

        	  Int_t iCell = 0;
        	  if (fDaq) iCell = (hit->GetTime() - fTimeSlice->GetStartTime()) / CellSize;
        	  fHitsinCell[iCell]++;
        	  break;
            }
          }
        }
      }
    }
  }
  fHM->H1("hno_NofObjects_MatchedHits_" + type)->Fill(nofMatchedHits);
  if ( nofMatchedHits > fMaxScale ) fMaxScale = nofMatchedHits;

  if (fDaq) {
    for ( Int_t i = nCells - 1; i > 0; i-- ) {
	  if ( fHitsinCell[i] > fMCinCell[i] ) {
        Int_t diff = fHitsinCell[i] - fMCinCell[i];
        fHitsinCell[i] = fMCinCell[i];
        fHitsinCell[i - 1] += diff;
	  }
    }
    for ( Int_t i = 0; i < nCells; i++ ) {
      if ( fMCinCell[i] != 0 ) fEffinCell[i] = 100. * (Float_t)fHitsinCell[i] / (Float_t)fMCinCell[i];
      else fEffinCell[i] = 0;
      fHM->H1("hce_PointsInCells")->Fill((Int_t)(fMCinCell[i] / 1000), fMCinCell[i]);
      fHM->H1("hce_HitsInCells")->Fill((Int_t)(fMCinCell[i] / 1000), fHitsinCell[i] * 100);
    }
  }
}

void CbmTrdRecoQaTb::FillResidualAndPullHistograms(
		CbmMCDataArray* points,
		const TClonesArray* hits,
		const TClonesArray* hitMatches,
		const string& type)
{
  if ( NULL == points || NULL == hits || NULL == hitMatches ) return;
  string nameResX = "hrp_Residual_X_" + type;
  string nameResY = "hrp_Residual_Y_" + type;
  string namePullX = "hrp_Pull_X_" + type;
  string namePullY = "hrp_Pull_Y_" + type;
  string nameResXvsClusterSize = "h2d_Residual_X_vs_ClusterSize_" + type;
  string nameResXvsSlope = "h2d_Residual_X_vs_SlopeX_" + type;
  if ( !fHM->Exists(nameResX) || !fHM->Exists(nameResY)
		  || !fHM->Exists(namePullX) || !fHM->Exists(namePullY) 
		  || !fHM->Exists(nameResXvsClusterSize) || !fHM->Exists(nameResXvsSlope)) return;
  Float_t residualX = -1000;
  Float_t residualY = -1000;
  Float_t slopeX = -1000;
  Int_t clusterSizeFront = -1;
  Int_t clusterSizeBack = -1;
  Int_t nofHits = hits->GetEntriesFast();
  for(Int_t iHit = 0; iHit < nofHits; iHit++) {
    const CbmTrdHit* hit = static_cast<const CbmTrdHit*>(hits->At(iHit));
    const CbmTrdCluster * frontCluster = static_cast<const CbmTrdCluster*>(fTrdClusters->At(0/*hit->GetFrontClusterId()*/));
    clusterSizeFront = 0;//frontCluster -> GetSize();
    const CbmTrdCluster * backCluster = static_cast<const CbmTrdCluster*>(fTrdClusters->At(0/*hit->GetBackClusterId()*/));
    clusterSizeBack = 0;//backCluster -> GetSize();
    const CbmMatch* hitMatch = static_cast<const CbmMatch*>(hitMatches->At(iHit));
    if ( hitMatch->GetNofLinks() == 1 ) {
    	const CbmLink link = hitMatch->GetLink(0);
//      CbmTrdPoint* point = (CbmTrdPoint*)points->Get(hitMatch->GetLink(0));
//      CbmTrdPoint* point = (CbmTrdPoint*) points->Get(link.GetFile(), link.GetEntry() - 1, link.GetIndex());	// TODO: link.GetEntry() - from 1 to N. Should be from 0 to N-1
//      residualX = point->GetX(hit->GetZ()) - hit->GetX();
//      residualY = point->GetY(hit->GetZ()) - hit->GetY();
//      slopeX = TMath::ATan(point -> GetPx() / point -> GetPz()) * 180. / 3.1416;
    }
    else {
      const CbmMatch* frontClusterMatch = static_cast<const CbmMatch*>(fTrdClusterMatches->At(0/*hit->GetFrontClusterId()*/));
      const CbmMatch* backClusterMatch = static_cast<const CbmMatch*>(fTrdClusterMatches->At(0/*hit->GetBackClusterId()*/));
      Bool_t matched = kFALSE;
      for(Int_t iFrontLink = 0; iFrontLink < frontClusterMatch->GetNofLinks(); iFrontLink++) {
        if ( matched ) break;
        const CbmLink& frontLink = static_cast<const CbmLink&>(frontClusterMatch->GetLink(iFrontLink));
        for(Int_t iBackLink = 0; iBackLink < backClusterMatch->GetNofLinks(); iBackLink++) {
          if ( matched ) break;
          const CbmLink& backLink = static_cast<const CbmLink&>(backClusterMatch->GetLink(iBackLink));
          if ( frontLink == backLink ) {
//            CbmTrdPoint* point = (CbmTrdPoint*)points->Get(frontLink);
//         	CbmTrdPoint* point = (CbmTrdPoint*) points->Get(frontLink.GetFile(), frontLink.GetEntry() - 1, frontLink.GetIndex());
//             residualX = ((point->GetXIn() + point->GetXOut()) / 2) - hit->GetX();
//             residualY = ((point->GetYIn() + point->GetYOut()) / 2) - hit->GetY();
// 	    slopeX = TMath::ATan(point -> GetPx() / point -> GetPz()) * 180. / 3.1416;
            matched = kTRUE;
            break;
          }
        }
      }
    }
    fHM->H1(nameResX)->Fill(residualX * 10000);
    fHM->H1(nameResY)->Fill(residualY * 10000);
//    std:cout.precision(10);
    fHM->H1(namePullX)->Fill(residualX / hit->GetDx());
    fHM->H1(namePullY)->Fill(residualY / hit->GetDy());
    fHM->H1(nameResXvsClusterSize)->Fill(clusterSizeFront, residualX * 10000);
    fHM->H1(nameResXvsSlope)->Fill(slopeX, residualX * 10000);
  }
}

ClassImp(CbmTrdRecoQaTb);

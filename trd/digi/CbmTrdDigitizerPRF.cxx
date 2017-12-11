#include "CbmTrdDigitizerPRF.h"

#include "CbmTrdDigiPar.h"
#include "CbmTrdPoint.h"
#include "CbmTrdDigi.h"
#include "CbmTrdModule.h"
#include "CbmTrdRadiator.h"
#include "CbmTrdTriangle.h"
#include "CbmTrdGeoHandler.h"

#include "CbmMCTrack.h"
#include "CbmMatch.h"
#include "CbmDaqBuffer.h"


// Includes from FairRoot
#include "FairEventHeader.h"
#include "FairMCEventHeader.h"
#include "FairRunAna.h"
#include "FairRunSim.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"

// Includes from Root
#include "TRandom.h"
#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include "TGeoManager.h"
#include "TStopwatch.h"

// Includes from C++
#include <iomanip>
#include <iostream>
#include <cmath>
#define VERBOSE_TRIANG 0

using std::cout;
using std::endl;
using std::make_pair;
using std::max;
using std::fabs;
using std::pair;

CbmTrdDigitizerPRF::CbmTrdDigitizerPRF(CbmTrdRadiator *radiator)
  :FairTask("CbmTrdDigitizerPRF"),
   fDebug(false),
   fStream(false),
   fNoiseDigis(true),
   fTrianglePads(false),
   fCbmLinkWeightDistance(false),
   fSigma_noise_keV(0.1),
   fNoise(NULL),
   fMinimumChargeTH(1.0e-06),
   fCurrentTime(-1.),
   fAddress(-1.),
   fLastEventTime(-1),
   fCollectTime(100),
   fnClusterConst(0),
   fnScanRowConst(0),
   fnScanColConst(0),
   fnRow(-1),
   fnCol(-1),
   fdtlow(0),
   fdthigh(0),
   fpoints(0),
   fLayerId(-1),
   fModuleId(-1),
   fBufferAddress(-1),
   fMCPointId(-1),
   fPoints(NULL),
   fDigis(NULL),
   fDigiMatches(NULL),
   fMCTracks(NULL),
   fDigiPar(NULL),
   fModuleInfo(NULL),
   fRadiator(radiator),
   fTriangleBinning(NULL),
   fDigiMap(),
   fAnalogBuffer(),
   fChargeBuffer(),
   fTimeBuffer()
{
}

CbmTrdDigitizerPRF::~CbmTrdDigitizerPRF()
{
  if(fTriangleBinning) delete fTriangleBinning;
   fDigis->Delete();
   delete fDigis;
   fDigiMatches->Delete();
   delete fDigiMatches;
}

InitStatus CbmTrdDigitizerPRF::Init()
{
  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) LOG(FATAL) << "CbmTrdDigitizerPRF::Init: No FairRootManager" << FairLogger::endl;

  fPoints = (TClonesArray*) ioman->GetObject("TrdPoint");
  if (fPoints == NULL) LOG(FATAL) << "CbmTrdDigitizerPRF::Init: No TrdPoint array" << FairLogger::endl;

  fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
  if (fMCTracks == NULL) LOG(FATAL) << "CbmTrdDigitizerPRF::Init: No MCTrack array" << FairLogger::endl;

  fDigis = new TClonesArray("CbmTrdDigi", 100);
  ioman->Register("TrdDigi", "TRD Digis", fDigis, IsOutputBranchPersistent("TrdDigi"));

  fDigiMatches = new TClonesArray("CbmMatch", 100);
  ioman->Register("TrdDigiMatch", "TRD Digis", fDigiMatches, IsOutputBranchPersistent("TrdDigiMatch"));
  if (fRadiator != NULL)
    fRadiator->Init();

  if (fSigma_noise_keV > 0.0){
    fNoise = new TRandom3();
  }

  // If the task CbmDaq is found, run in stream mode; else in event mode.
  FairTask* daq = FairRun::Instance()->GetTask("Daq");
  if ( daq ) {
    LOG(INFO) << GetName() << ": Using stream mode."
              << FairLogger::endl;
    fStream = true;
  }  //? stream mode
  else {
    LOG(INFO) << GetName() << ": Using event mode."
              << FairLogger::endl;
    fStream = false;
  }

  
  return kSUCCESS;
}

void CbmTrdDigitizerPRF::Exec(Option_t*)
{
  LOG(INFO) << "================ TRD PRF Digitizer ===============" << FairLogger::endl;
  LOG(INFO) << " Noise width:     " << fSigma_noise_keV << " keV"<< FairLogger::endl;
  fDigis->Delete();
  fDigiMatches->Delete();

  fDebug = false;
  TStopwatch timer;
  timer.Start();

  /// get event info (once per event, used later in the matching)
  GetEventInfo(fInputNr, fEventNr, fEventTime);

  Int_t nofLatticeHits = 0;
  Int_t nofElectrons = 0;
  Int_t nofBackwardTracks = 0;
  Int_t nofPointsAboveThreshold = 0;
  Int_t nofPoints = fPoints->GetEntries();
  gGeoManager->CdTop();

  for (Int_t iPoint = 0; iPoint < nofPoints ; iPoint++) {
    fpoints++;
    fMCPointId = iPoint;
    CbmTrdPoint* point = static_cast<CbmTrdPoint*>(fPoints->At(iPoint));
    if(NULL == point) continue;
    const CbmMCTrack* track = static_cast<const CbmMCTrack*>(fMCTracks->At(point->GetTrackID()));
    if(NULL == track) continue;

    Double_t dz = point->GetZOut() - point->GetZIn();
    if (dz < 0) {
      LOG(DEBUG2) << "CbmTrdDigitizerPRF::Exec: MC-track points towards target!" << FairLogger::endl;
      nofBackwardTracks++;
    }

    TVector3 mom;
    point->Momentum(mom);

    Double_t ELoss = 0.0;
    Double_t ELossTR = 0.0;
    Double_t ELossdEdX = 0.0;
    if (fRadiator != NULL)
      if (TMath::Abs(track->GetPdgCode()) == 11){
	nofElectrons++;
	if (fRadiator->LatticeHit(point)){  // electron has passed lattice grid (or frame material) befor reaching the gas volume -> TR-photons have been absorbed by the lattice grid
	  nofLatticeHits++;
	  ELossTR = 0.0;
	} else if (dz < 0){ //electron has not passed the radiator
	  ELossTR = 0.0;
	} else {
	  ELossTR = fRadiator->GetTR(mom);
	}
      }
    ELossdEdX = point->GetEnergyLoss();
    ELoss = ELossTR + ELossdEdX;
    if (ELoss > fMinimumChargeTH)  nofPointsAboveThreshold++;
    fCurrentTime =fEventTime + point->GetTime()+ AddDrifttime(gRandom->Integer(240))*1000;  //convert to ns;
    
    // Find node corresponding to the point in the center between entrance and exit MC-point coordinates
    Double_t meanX = (point->GetXOut() + point->GetXIn()) / 2.;
    Double_t meanY = (point->GetYOut() + point->GetYIn()) / 2.;
    Double_t meanZ = (point->GetZOut() + point->GetZIn()) / 2.;
    gGeoManager->FindNode(meanX, meanY, meanZ);

    if (!TString(gGeoManager->GetPath()).Contains("gas")){
      LOG(ERROR) << "CbmTrdDigitizerPRF::Exec: MC-track not in TRD! Node:" << TString(gGeoManager->GetPath()).Data() << " gGeoManager->MasterToLocal() failed!" << FairLogger::endl;
      continue;
    }

    fLayerId = CbmTrdAddress::GetLayerId(point->GetDetectorID());
    fModuleId = CbmTrdAddress::GetModuleId(point->GetDetectorID());
    
    fModuleInfo = fDigiPar->GetModule(point->GetDetectorID());
    fnCol = fModuleInfo->GetNofColumns();
    fnRow = fModuleInfo->GetNofRows();

    fTrianglePads=fModuleInfo->GetPadGeoTriangular();
    if(VERBOSE_TRIANG) printf("\ni[%3d] point[%7.2f %7.2f %7.2f] ly[%d] ModuleId[%d] cols[%2d] rows[%2d]\n", iPoint, point->GetXIn(), point->GetYIn(), point->GetZIn(), fLayerId, fModuleId, fnCol, fnRow);
    if(fTrianglePads) SplitTrackPathTriang(point, ELoss, ELossTR);
    else SplitTrackPath(point, ELoss, ELossTR);
  }

  // Fill data from internally used stl map into output TClonesArray
  if(!fStream){
    Int_t iDigi = 0;
    std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it;
    for (it = fDigiMap.begin() ; it != fDigiMap.end(); it++) {
      new ((*fDigis)[iDigi]) CbmTrdDigi(*(it->second.first));
      new ((*fDigiMatches)[iDigi]) CbmMatch(*(it->second.second));
      delete it->second.first;
      delete it->second.second;
      iDigi++;
    }
    fDigiMap.clear();
  }

  fLastEventTime=fEventTime;

  Double_t digisOverPoints = (nofPoints > 0) ? fDigis->GetEntriesFast() / nofPoints : 0;
  Double_t latticeHitsOverElectrons = (nofElectrons > 0) ? (Double_t) nofLatticeHits / (Double_t) nofElectrons : 0;
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec Points:               " << nofPoints << FairLogger::endl;
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec PointsAboveThreshold: " << nofPointsAboveThreshold << FairLogger::endl;
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec Digis:                " << fDigis->GetEntriesFast() << FairLogger::endl;
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec digis/points:         " << digisOverPoints << FairLogger::endl;
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec BackwardTracks:       " << nofBackwardTracks << FairLogger::endl;
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec LatticeHits:          " << nofLatticeHits  << FairLogger::endl;
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec Electrons:            " << nofElectrons << FairLogger::endl;
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec latticeHits/electrons:" << latticeHitsOverElectrons << FairLogger::endl;
  timer.Stop();
  LOG(INFO) << "CbmTrdDigitizerPRF::Exec real time=" << timer.RealTime()
            << " CPU time=" << timer.CpuTime() << FairLogger::endl;
}


void CbmTrdDigitizerPRF::ScanPadPlane(const Double_t* local_point, Double_t clusterELoss, Double_t clusterELossTR)
{
  Int_t sectorId(-1), columnId(-1), rowId(-1);
  fModuleInfo->GetPadInfo( local_point, sectorId, columnId, rowId);
  if (sectorId < 0 && columnId < 0 && rowId < 0) {
    return;
  }
  else {
    for (Int_t i = 0; i < sectorId; i++) {
      rowId += fModuleInfo->GetNofRowsInSector(i); // local -> global row
    }

    Double_t displacement_x(0), displacement_y(0);//mm
    Double_t h = fModuleInfo->GetAnodeWireToPadPlaneDistance();
    Double_t W(fModuleInfo->GetPadSizeX(sectorId)), H(fModuleInfo->GetPadSizeY(sectorId));
    fModuleInfo->TransformToLocalPad(local_point, displacement_x, displacement_y);

    Int_t maxCol(5/W+0.5), maxRow(6);//5/H+3);// 7 and 3 in orig. minimum 5 times 5 cm area has to be scanned
    if (fnScanRowConst > 0)
      maxRow = fnScanRowConst;
    if (fnScanColConst > 0)
      maxCol = fnScanColConst;

    Int_t startCol(columnId-maxCol/2), startRow(rowId-maxRow/2);
    Double_t sum = 0;
    Int_t secRow(-1), targCol(-1), targRow(-1), targSec(-1), address(-1);
    for (Int_t iRow = startRow; iRow <= rowId+maxRow/2; iRow++) {
      for (Int_t iCol = startCol; iCol <= columnId+maxCol/2; iCol++) {
	if (((iCol >= 0) && (iCol <= fnCol-1)) && ((iRow >= 0) && (iRow <= fnRow-1))){// real adress
	  targSec = fModuleInfo->GetSector(iRow, secRow);
	  address = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, iCol);
	  fAddress = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, iCol);
	}
	else {
	  targRow = iRow;
	  targCol = iCol;
	  if (iCol < 0) {
	    targCol = 0;
	  }
	  else if (iCol > fnCol-1) {
	    targCol = fnCol-1;
	  }
	  if (iRow < 0) {
 	    targRow = 0;
	  }
	  else if (iRow > fnRow-1) {
	    targRow = fnRow-1;
	  }
	  
	  targSec = fModuleInfo->GetSector(targRow, secRow);
	  address = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, targCol);
	}

	Double_t chargeFraction = 0;
	if (rowId == iRow && columnId == iCol) { // if pad in center of 7x3 arrayxs
	  chargeFraction = CalcPRF(displacement_x, W, h) * CalcPRF(displacement_y, H, h);
	}
	else {
	  chargeFraction = CalcPRF((iCol - columnId) * W - displacement_x, W, h) * CalcPRF((iRow - rowId) * H - displacement_y, H, h);
	}
	if (fnScanRowConst == 1 && fnScanColConst == 1)
	  chargeFraction = 1.0;

	sum += chargeFraction;
	
	if(!fStream)            AddDigi(fMCPointId, address, Double_t(chargeFraction * clusterELoss), Double_t(chargeFraction * clusterELossTR), fCurrentTime);
	if(fStream)             AddDigitoBuffer(fMCPointId, address, Double_t(chargeFraction * clusterELoss), Double_t(chargeFraction * clusterELossTR), fCurrentTime);
      } // for iCol

    } // for iRow
    if (sum < 0.99 || sum > 1.01){
      LOG(WARNING) << "CbmTrdDigitizerPRF::ScanPadPlane: Summed charge: " << std::setprecision(5) << sum << "  hit:(" << columnId << ", " << rowId <<")   max:(" << fnCol-1 << ", " << fnRow-1 << ")" << FairLogger::endl;
    }
  }
}

void CbmTrdDigitizerPRF::SplitTrackPath(const CbmTrdPoint* point, Double_t ELoss, Double_t ELossTR)
{
  const Double_t nClusterPerCm = 1.0;
  Double_t point_in[3] = {
    point->GetXIn(),
    point->GetYIn(),
    point->GetZIn()
  };
  Double_t point_out[3] = {
    point->GetXOut(),
    point->GetYOut(),
    point->GetZOut()
  };
  Double_t local_point_out[3];// exit point coordinates in local module cs
  Double_t local_point_in[3]; // entrace point coordinates in local module cs
  gGeoManager->MasterToLocal(point_in,  local_point_in);
  gGeoManager->MasterToLocal(point_out, local_point_out);

  Double_t cluster_pos[3];   // cluster position in local module coordinate system
  Double_t cluster_delta[3]; // vector pointing in MC-track direction with length of one path slice within chamber volume to creat n cluster

  Double_t trackLength = 0;
  
  for (Int_t i = 0; i < 3; i++) {
    cluster_delta[i] = (local_point_out[i] - local_point_in[i]);
    trackLength += cluster_delta[i] * cluster_delta[i];
  }
  trackLength = TMath::Sqrt(trackLength);
  Int_t nCluster = trackLength / nClusterPerCm + 0.9;// Track length threshold of minimum 0.1cm track length in gas volume
  if (fnClusterConst > 0){
    nCluster = fnClusterConst;   // Set number of cluster to constant value
  }

  if (nCluster < 1){
    return;
  }
  for (Int_t i = 0; i < 3; i++){
    cluster_delta[i] /= Double_t(nCluster);
  }
  Double_t clusterELoss = ELoss / Double_t(nCluster);
  Double_t clusterELossTR = ELossTR / Double_t(nCluster);

  for (Int_t iCluster = 0; iCluster < nCluster; iCluster++){
    for (Int_t i = 0; i < 3; i++){
      cluster_pos[i] = local_point_in[i] + (0.5 + iCluster) * cluster_delta[i];// move start position 0.5 step widths into track direction
    }

    if ( fModuleInfo->GetSizeX() < fabs(cluster_pos[0]) || fModuleInfo->GetSizeY() < fabs(cluster_pos[1])){
      printf("->    nC %i/%i x: %7.3f y: %7.3f \n",iCluster,nCluster-1,cluster_pos[0],cluster_pos[1]);
      for (Int_t i = 0; i < 3; i++)
	printf("  (%i) | in: %7.3f + delta: %7.3f * cluster: %i/%i = cluster_pos: %7.3f out: %7.3f g_in:%f g_out:%f\n",
	       i,local_point_in[i],cluster_delta[i],iCluster,(Int_t)nCluster,cluster_pos[i],local_point_out[i],point_in[i],point_out[i]);
    }


    //add noise digis between the actual and the last event
    if(!fTrianglePads && fNoiseDigis){
      Int_t noiserate=fNoise->Uniform(0,3); //still in development
      Double_t simtime=fCurrentTime;
      for(Int_t ndigi=0; ndigi<noiserate; ndigi++){
	NoiseTime();
	ScanPadPlane(cluster_pos, fNoise->Gaus(0, fSigma_noise_keV * 1.E-6), 0);
      }
      fCurrentTime=simtime;
    }

    
    fModuleInfo->ProjectPositionToNextAnodeWire(cluster_pos);
    if (!fTrianglePads)                     ScanPadPlane(cluster_pos, clusterELoss, clusterELossTR);
    else                                    ScanPadPlaneTriangle(cluster_pos, clusterELoss, clusterELossTR);
  }
}

void CbmTrdDigitizerPRF::AddDigi(Int_t pointId, Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t up)
{
  Double_t weighting = charge;

  if (fCbmLinkWeightDistance) {
    const CbmTrdPoint* point = static_cast<CbmTrdPoint*>(fPoints->At(pointId));
    Double_t ref_pos[3] = {point->GetXOut(), point->GetYOut(), point->GetZOut()};
    TVector3 padPos, padPosErr;
    fModuleInfo->GetPadPosition(address, padPos, padPosErr);
    Double_t distance = sqrt(pow(ref_pos[0] - padPos[0],2) + pow(ref_pos[1] - padPos[1],2));
    weighting = 1. / distance;
  }

  std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it = fDigiMap.find(address);
  if (it == fDigiMap.end()) { // Pixel not yet in map -> Add new pixel
    if(!fTrianglePads) AddNoise(charge);
    
    CbmMatch* digiMatch = new CbmMatch();
    digiMatch->AddLink(CbmLink(weighting, pointId, fEventNr, fInputNr));
    fDigiMap[address] = make_pair(new CbmTrdDigi(address, 0., time, fTrianglePads), digiMatch);
    it = fDigiMap.find(address);
    it->second.first->SetCharge(charge, up);
    it->second.first->SetChargeTR(chargeTR, up);
  }
  else { // Pixel already in map -> Add charge
    it->second.first->AddCharge(charge, up);
    it->second.first->AddChargeTR(chargeTR, up);
    it->second.first->SetTime(max(time, it->second.first->GetTime()));
    it->second.second->AddLink(CbmLink(weighting, pointId, fEventNr, fInputNr));
  }
  //if(fTrianglePads) cout<<it->second.first->ToString();
}

void CbmTrdDigitizerPRF::AddDigitoBuffer(Int_t pointId, Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t up)
{
  Double_t weighting = charge;

  if (fCbmLinkWeightDistance) {
    const CbmTrdPoint* point = static_cast<CbmTrdPoint*>(fPoints->At(pointId));
    Double_t ref_pos[3] = {point->GetXOut(), point->GetYOut(), point->GetZOut()};
    TVector3 padPos, padPosErr;
    fModuleInfo->GetPadPosition(address, padPos, padPosErr);
    Double_t distance = sqrt(pow(ref_pos[0] - padPos[0],2) + pow(ref_pos[1] - padPos[1],2));
    weighting = 1. / distance;
  }

  Bool_t process=CheckTime(address);

  if(process)                                                 ProcessBuffer(address,weighting);

  AddNoise(charge);
  
  std::map<Int_t, std::vector<pair<CbmTrdDigi*, CbmMatch*>>>::iterator it = fAnalogBuffer.find(address);

  CbmMatch* digiMatch = new CbmMatch();
  digiMatch->AddLink(CbmLink(weighting, pointId, fEventNr, fInputNr));
  CbmTrdDigi* digi= new CbmTrdDigi(address, charge, time, fTrianglePads);
  digi->SetCharge(charge, up);
  digi->SetChargeTR(chargeTR, up);
  digi->SetMatch(digiMatch);
  //CbmDaqBuffer::Instance()->InsertData(digi);
  fAnalogBuffer[address].push_back(make_pair(digi, digiMatch));
  fChargeBuffer[address].push_back(make_pair(charge,chargeTR));
}


// -------------------------------------------------------------------------
void CbmTrdDigitizerPRF::GetEventInfo(Int_t& inputNr, Int_t& eventNr,
				      Double_t& eventTime)
{
  // --- The event number is taken from the FairRootManager
  eventNr = FairRootManager::Instance()->GetEntryNr();

   // --- In a FairRunAna, take the information from FairEventHeader
  if ( FairRunAna::Instance() ) {
    FairEventHeader* event = FairRunAna::Instance()->GetEventHeader();
    inputNr   = event->GetInputFileId();
    eventTime = event->GetEventTime();
  }

  // --- In a FairRunSim, the input number and event time are always zero;
  // --- only the event number is retrieved.
  else {
    if ( ! FairRunSim::Instance() )
      LOG(FATAL) << GetName() << ": neither SIM nor ANA run." 
		 << FairLogger::endl;
    inputNr   = 0;
    eventTime = 0.;
  }
}

void CbmTrdDigitizerPRF::SetTriangularPads(Bool_t triangles)
{
  fTrianglePads = triangles;
}

void CbmTrdDigitizerPRF::SetNoiseLevel(Double_t sigma_keV)
{
	fSigma_noise_keV = sigma_keV;
}


void CbmTrdDigitizerPRF::SetNCluster(Int_t nCluster)
{
  fnClusterConst = nCluster;
}
void CbmTrdDigitizerPRF::SetCbmLinkWeightDistance(Bool_t dist)
{
  fCbmLinkWeightDistance = dist;
}
void CbmTrdDigitizerPRF::SetPadPlaneScanArea(Int_t column, Int_t row)
{
  if (row%2 == 0) row += 1;
  fnScanRowConst = row;
  if (column%2 == 0) column += 1;
  fnScanColConst = column;
}


void CbmTrdDigitizerPRF::SetParContainers()
{
	fDigiPar = (CbmTrdDigiPar*)(FairRun::Instance()->GetRuntimeDb()->getContainer("CbmTrdDigiPar"));
}


void CbmTrdDigitizerPRF::SetTriggerThreshold(Double_t minCharge){
  fMinimumChargeTH = minCharge;//  To be used for test beam data processing
}



Double_t CbmTrdDigitizerPRF::CalcPRF(Double_t x, Double_t W, Double_t h)
{
  Float_t K3 = 0.525; 
  Double_t SqrtK3 = sqrt(K3);

  return fabs(
	      -1. / (2. * atan(SqrtK3)) * (
					   atan(SqrtK3 * tanh(TMath::Pi() * (-2. + SqrtK3 ) * (W + 2.* x) / (8.* h) )) + 
					   atan(SqrtK3 * tanh(TMath::Pi() * (-2. + SqrtK3 ) * (W - 2.* x) / (8.* h) ))
					   )
	      );
}


void CbmTrdDigitizerPRF::ScanPadPlaneTriangle(const Double_t* local_point, Double_t clusterELoss, Double_t clusterELossTR)
{
  /*
  // In order to get this algorithm as perfromant as possible the pad plane in the digipar containes twice pad rows without further information on the pad geomatry itself.
  // The triangle is associated to a row by the position of the 90 deg angle.-> In even rows the smallest angle ist pointing upwards and in odd rows it is pointing downward per definition.
  // In a first step the full rectangular pad (2 rows) is calculated. This has the benifit that we need only to numericaly integrate one triangular pad which is very time consuming. 
  // The second triangle is given by the difference of the full rectangle and the first triangle.
  */
  Int_t sectorId(-1), columnId(-1), rowId(-1);//, preSecRows(0);
  fModuleInfo->GetPadInfo( local_point, sectorId, columnId, rowId);
  if (sectorId < 0 && columnId < 0 && rowId < 0) {
    printf("    x:%7.3f  y:%7.3f z:%7.3f\n",local_point[0],local_point[1],local_point[2]);
    return;
  } else {
    for (Int_t i = 0; i < sectorId; i++)
      rowId += fModuleInfo->GetNofRowsInSector(i); // local -> global row

    Double_t displacement_x(0), displacement_y(0);//cm
    Double_t h = fModuleInfo->GetAnodeWireToPadPlaneDistance();
    Double_t W(fModuleInfo->GetPadSizeX(sectorId)), H(fModuleInfo->GetPadSizeY(sectorId));
    fModuleInfo->TransformToLocalPad(local_point, displacement_x, displacement_y);  
     
    Int_t maxCol(5/W+0.5), maxRow(6);//5/H+3);// 7 and 3 in orig. minimum 5 times 5 cm area has to be scanned
    if (fnScanRowConst > 0)
      maxRow = fnScanRowConst;
    if (fnScanColConst > 0)
      maxCol = fnScanColConst;

    //Estimate starting column and row and limits due to chamber dimensions
    Int_t startCol(columnId-maxCol/2), stopCol(columnId+maxCol/2), startRow(rowId-maxRow/2), stopRow(rowId+maxRow/2+1); // startSec(0);
    if (startRow % 2 != 0){ // It does not make sence to start scanning in odd rows for triangluar pad geometry since the triangles are later combined to rectangles and parallelograms
      startRow -= 1;
      stopRow  -= 1;
    }
    Double_t sum = 0;
    Int_t secRow(-1), targCol(-1), targRow(-1), targSec(-1), addressEven(-1), addressOdd(-1);
    for (Int_t iCol = startCol; iCol <= stopCol; iCol++){
      Double_t chargeFractionRectangle(0.0), chargeFractionTriangle(0.0);
      for (Int_t iRow = startRow; iRow <= stopRow; iRow++){
	if (((iCol >= 0) && (iCol <= fnCol-1)) && ((iRow >= 0) && (iRow <= fnRow-1))){// real address	 
	  targSec = fModuleInfo->GetSector(iRow, secRow);
	  if (iRow % 2 == 0)
	    addressEven = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, iCol); 
	  else
	    addressOdd = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, iCol); 
	} else {
          targRow = iRow;
          targCol = iCol;
          if (iCol < 0)
	    targCol = 0;
	  else if (iCol > fnCol-1)
	    targCol = fnCol-1;
	  if (iRow < 0)
	    targRow = 0;
	  else if (iRow > fnRow-1)	    
	    targRow = fnRow-1;

	  targSec = fModuleInfo->GetSector(targRow, secRow);
	  if (iRow % 2 == 0)
	    addressEven = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, targCol);
	  else
	    addressOdd = CbmTrdAddress::GetAddress(fLayerId, fModuleId, targSec, secRow, targCol);
	  if (secRow > 11 && fModuleId == 5){
	  }
	}

	if (iRow % 2 == 0) { // to calculate the full rectangle one has to sum over one even and the following odd row
	  sum += chargeFractionRectangle;
	  chargeFractionRectangle = 0;	
          chargeFractionTriangle  = TriangleIntegration(true,  displacement_x - (iCol - columnId) * W, W, displacement_y - (iRow - rowId) * H, H, h);
	} 

	chargeFractionRectangle += CalcPRF((iCol - columnId) * W - displacement_x, W, h) * CalcPRF((iRow - rowId) * H - displacement_y, H, h);

	if (iRow % 2 != 0){
	  if ((chargeFractionRectangle - chargeFractionTriangle) < 0.0)
	    printf("-------------------------------\nc:%i r:%i even: %E  odd %E        R: %E   T:%E\n-------------------------------\n",iCol, iRow, chargeFractionTriangle, chargeFractionRectangle - chargeFractionTriangle, chargeFractionRectangle, chargeFractionTriangle);
	  AddDigi(fMCPointId, addressEven, Double_t(chargeFractionTriangle * clusterELoss),
Double_t(chargeFractionTriangle * clusterELossTR), fCurrentTime);
	  AddDigi(fMCPointId, addressOdd, Double_t((chargeFractionRectangle - chargeFractionTriangle) * clusterELoss), Double_t((chargeFractionRectangle - chargeFractionTriangle) * clusterELossTR), fCurrentTime);
	}     
      } // for iRow      
    } // for iCol
    if (sum < 0.99 || sum > 1.01){
      LOG(WARNING) << "CbmTrdDigitizerPRF::ScanPadPlane: Summed charge: " << std::setprecision(5) << sum << "  hit:(" << columnId << ", " << rowId <<")   max:(" << fnCol-1 << ", " << fnRow-1 << ")" << FairLogger::endl;
    }
  }  
}

void CbmTrdDigitizerPRF::ProcessBuffer(Int_t address,Double_t /*weighting*/){

  Double_t digicharge=0;
  Double_t digiTRcharge=0;
  std::map<Int_t,std::vector<pair<CbmTrdDigi*, CbmMatch*>>>:: iterator itana=fAnalogBuffer.find(address);
  std::map<Int_t,std::vector<pair<Double_t,Double_t>>>::      iterator itcharge=fChargeBuffer.find(address);
  std::vector<pair<Double_t,Double_t>>                        charge=itcharge->second;
  std::vector<pair<CbmTrdDigi*, CbmMatch*>>                   analog=itana->second;
  std::vector<pair<Double_t,Double_t>>::                      iterator it;

  //preliminary implementation 
  for (it=charge.begin() ; it != charge.end(); it++) {
    digicharge+=it->first;
    digiTRcharge+=it->second;
  }
  charge.clear();
  
  // Copy match object. Will be deleted in the digi destructor.
  // TODO: check if move of object is possible
  CbmMatch* digiMatch = new CbmMatch(*analog.begin()->second);
//  CbmMatch* digiMatch = analog.begin()->second;
  CbmTrdDigi* digi= new CbmTrdDigi(address, digicharge, fCurrentTime);
  digi->SetChargeTR(digiTRcharge);
  digi->SetMatch(digiMatch);
  CbmDaqBuffer::Instance()->InsertData(digi);

  analog.clear();
}


Double_t CbmTrdDigitizerPRF::AddNoise(Double_t charge){
  
  if (fSigma_noise_keV > 0.0){
    Double_t noise = fNoise->Gaus(0, fSigma_noise_keV * 1.E-6);// keV->GeV // add only once per digi and event noise !!!
    charge += noise; // resulting charge can be < 0 -> possible  problems with position reconstruction
  }

  return charge;
}


Double_t CbmTrdDigitizerPRF::CheckTime(Int_t address){
 
  std::map<Int_t,Double_t>::                                  iterator timebuffer = fTimeBuffer.find(address);
  Double_t dt=fCurrentTime-fTimeBuffer[address];
  if(fCurrentTime>fTimeBuffer[address] && dt>0.0000000)                      fTimeBuffer[address]=fCurrentTime;


  if(dt>fCollectTime && dt!=fCurrentTime)              return true;
  else                                                 return false;
}


void CbmTrdDigitizerPRF::NoiseTime(){

  //Double_t dEventTime=fEventTime-fLastEventTime;
  fCurrentTime=fNoise->Uniform(fLastEventTime,fEventTime);
  
}

Double_t CbmTrdDigitizerPRF::AddDrifttime(Double_t /*x*/){

  Double_t drifttime[240]={0.11829,0.11689,0.11549,0.11409,0.11268,0.11128,0.10988,0.10847,0.10707,0.10567,
			   0.10427,0.10287,0.10146,0.10006,0.09866,0.09726,0.095859,0.094459,0.09306,0.091661,
			   0.090262,0.088865,0.087467,0.086072,0.084677,0.083283,0.08189,0.080499,0.07911,0.077722,
			   0.076337,0.074954,0.073574,0.072197,0.070824,0.069455,0.06809,0.066731,0.065379,0.064035,
			   0.0627,0.061376,0.060063,0.058764,0.05748,0.056214,0.054967,0.053743,0.052544,0.051374,
			   0.05024,0.049149,0.048106,0.047119,0.046195,0.045345,0.044583,0.043925,0.043403,0.043043,
			   0.042872,0.042932,0.043291,0.044029,0.045101,0.04658,0.048452,0.050507,0.052293,0.053458,
			   0.054021,0.053378,0.052139,0.53458,0.050477,0.048788,0.047383,0.046341,0.045631,0.045178,
			   0.045022,0.045112,0.045395,0.045833,0.046402,0.047084,0.047865,0.048726,0.049651,0.050629,
			   0.051654,0.052718,0.053816,0.054944,0.056098,0.057274,0.058469,0.059682,0.060909,0.062149,
			   0.0634,0.064661,0.06593,0.067207,0.06849,0.069778,0.07107,0.072367,0.073666,0.074968,
			   0.076272,0.077577,0.078883,0.080189,0.081495,0.082801,0.084104,0.085407,0.086707,0.088004,
			   0.089297,0.090585,0.091867,0.093142,0.094408,0.095664,0.096907,0.098134,0.099336,0.10051,
			   0.10164,0.10273,0.10375,0.10468,0.10548,0.10611,0.10649,0.10655,0.10608,0.10566,
			   0.1072,0.10799,0.10875,0.11103,0.11491,0.11819,0.12051,0.12211,0.12339,0.12449,
			   0.12556,0.12663,0.12771,0.12881,0.12995,0.13111,0.13229,0.13348,0.13468,0.13589,
			   0.13711,0.13834,0.13957,0.1408,0.14204,0.14328,0.14452,0.14576,0.14701,0.14825,
			   0.1495,0.15075,0.152,0.15325,0.1545,0.15576,0.15701,0.15826,0.15952,0.16077,
			   0.16203,0.16328,0.16454,0.16579,0.16705,0.1683,0.16956,0.17082,0.17207,0.17333,
			   0.17458,0.17584,0.1771,0.17835,0.17961,0.18087,0.18212,0.18338,0.18464,0.18589,
			   0.18715,0.18841,0.18966,0.19092,0.19218,0.19343,0.19469,0.19595,0.19721,0.19846,
			   0.19972,0.20098,0.20223,0.20349,0.20475,0.20601,0.20726,0.20852,0.20978,0.21103,
			   0.21229,0.21355,0.2148,0.21606,0.21732,0.21857,0.21983,0.22109,0.22234,0.2236,
			   0.22486,0.22612,0.22737,0.22863,0.22989,0.23114,0.2324,0.23366,0.23491,0.23617};

  
  Int_t xindex=0;

  return drifttime[xindex];

  
}
  // --------------------------------------------------------------------
Double_t CbmTrdDigitizerPRF::TriangleIteration(Bool_t even, Int_t step, Double_t dis_x, Double_t W, Double_t dis_y, Double_t H, Double_t h){
  Double_t chargeFraction = 0.0;//W * H;
  
  chargeFraction = CalcPRF(dis_x, W, h) * CalcPRF(dis_y, H, h);
  if (step < 8) { // maximum 15 iterations -> 99.99847% accuracy
    // new size of the two smaller rectangles  
    W *= 0.5;
    H *= 0.5;
    
    if (even) {   // even = bottom triangle
      // left rectangle with a quarter of the area
      chargeFraction += TriangleIteration(even, step+1 , dis_x + 1.5 * W, W, dis_y + 0.5 * H, H, h); // bottom left
      // upper rectangle with a quarter of the area
      chargeFraction += TriangleIteration(even, step+1 , dis_x - 0.5 * W, W, dis_y - 1.5 * H, H, h); // top right
    } else {   // odd = top triangle
      // right rectangle with a quarter of the area
      chargeFraction += TriangleIteration(even, step+1 , dis_x - 1.5 * W, W, dis_y - 0.5 * H, H, h); // top right
      // lower rectangle with a quarter of the area
      chargeFraction += TriangleIteration(even, step+1 , dis_x + 0.5 * W, W, dis_y + 1.5 * H, H, h); // bottom left
    }
    
  } 
  
  return chargeFraction;
}
  // --------------------------------------------------------------------
Double_t CbmTrdDigitizerPRF::TriangleIntegration(Bool_t even, Double_t dis_x, Double_t W, Double_t dis_y, Double_t H, Double_t h)
{
  /*
  // The pad height has to be doubled since the digipar provides only pad height for rectangular pads. Therefore the dis has also to be corrected for triangular pad geometry.
  // The Triangular pad will be numerically approximated by a larger number of small subrectangels. 
  */
  Double_t chargeFraction = 0.0;
  //First itteration level; biggest rectangle lower left corner (W/2, H)  
  W *= 0.5;
  H *= 1.0;

  if (even){
    chargeFraction = TriangleIteration(even, 0 , dis_x - 0.5 * W, W, dis_y, H, h);
  }
  else {
    chargeFraction = TriangleIteration(even, 0 , dis_x + 0.5 * W, W, dis_y, H, h);
  }
  return chargeFraction;
  
}

//_______________________________________________________________________________________________
void CbmTrdDigitizerPRF::SplitTrackPathTriang(const CbmTrdPoint* point, Double_t ELoss, Double_t ELossTR)
{
/**  
  Steering routine for building digits out of the TRD hit for the triangular pad geometry. 
  1. Scan the amplification cells span by the track\n
  2. Build digits for each cell proportional with the projected energy on the cell\n
    2.1 Continuous distribution for ionisation\n
    2.2 Exponential decay for TR with constant \lambda
*/

  if(VERBOSE_TRIANG) printf("SplitTrackPathTriang ...\n");
  Double_t  gin[3] = {point->GetXIn(), point->GetYIn(), point->GetZIn()},
            gout[3]= {point->GetXOut(),point->GetYOut(),point->GetZOut()},
            lin[3],     // entrace point coordinates in local module cs
            lout[3],    // exit point coordinates in local module cs
            ain[3],     // entrace anode wire position
            aout[3],    // exit anode wire position
            dd[3];      // vec(lout)-vec(lin)
  gGeoManager->MasterToLocal(gin,  lin);
  gGeoManager->MasterToLocal(gout, lout);

  // compute track length in the gas volume
  Double_t trackLength(0.), txy(0.);
  for (Int_t i = 0; i < 3; i++) {
    dd[i] = (lout[i] - lin[i]);
    if(i==2) txy = trackLength;
    trackLength += dd[i] * dd[i];
  }
  if(trackLength) trackLength = TMath::Sqrt(trackLength);
  else{
    LOG(WARNING) << "CbmTrdDigitizerPRF::SplitTrackPathTriang: NULL track length for"
    " dEdx("<<std::setprecision(5)<<ELoss*1e6<<") keV " << FairLogger::endl;
    return;
  }
  if(txy) txy = TMath::Sqrt(txy);
  else{
    LOG(WARNING) << "CbmTrdDigitizerPRF::SplitTrackPathTriang: NULL xy track length projection for"
    " dEdx("<<std::setprecision(5)<<ELoss*1e6<<") keV " << FairLogger::endl;
    return;
  }

  // get anode wire for the entrance point
  memcpy(ain, lin, 3*sizeof(Double_t));
  fModuleInfo->ProjectPositionToNextAnodeWire(ain);
  // get anode wire for the exit point
  memcpy(aout, lout, 3*sizeof(Double_t));
  fModuleInfo->ProjectPositionToNextAnodeWire(aout);

  // estimate no of anode wires hit by the track
  Double_t dw(fModuleInfo->GetAnodeWireSpacing());
  Int_t ncls=TMath::Nint(TMath::Abs(aout[1]-ain[1])/dw+1.);
  if(VERBOSE_TRIANG) {
    printf("  Ncls[%d]\n", ncls);
    printf("  pin[%7.4f %7.4f %7.4f] pout[%7.4f %7.4f %7.4f]\n", lin[0], lin[1], lin[2], lout[0], lout[1], lout[2]);
    printf("  ain[%7.4f %7.4f %7.4f] aout[%7.4f %7.4f %7.4f]\n", ain[0], ain[1], ain[2], aout[0], aout[1], aout[2]);
  }
  
  // calculate track segmentation on the amplification cells distribution
  Int_t sgnx(1), sgny(1); 
  if(lout[0]<lin[0]) sgnx=-1;
  if(lout[1]<lin[1]) sgny=-1;
  Double_t  dy[] = {TMath::Min((ain[1]+0.5*sgny*dw-lin[1])*sgny, (lout[1]-lin[1])*sgny), 
                    TMath::Min((lout[1]-(aout[1]-0.5*sgny*dw))*sgny, (lout[1]-lin[1])*sgny)},
            dxw(TMath::Abs(dd[0]*dw/dd[1])),
            dx[] = {TMath::Abs(dy[0]*dd[0]/dd[1]), TMath::Abs(dy[1]*dd[0]/dd[1])};
  // check partition
  Double_t DX(dx[0]), DY(dy[0]);
  for(Int_t ic(1); ic<ncls-1; ic++){DX+=dxw; DY+=dw;}
  if(ncls>1){DX+=dx[1]; DY+=dy[1];}
  if(VERBOSE_TRIANG) {
    printf("  tdx[%7.4f] dx[%7.4f %7.4f %7.4f] check[%7.4f]\n"
         "  tdy[%7.4f] dy[%7.4f %7.4f %7.4f] check[%7.4f]\n", 
         dd[0], dx[0], dx[1], dxw, sgnx*DX, 
         dd[1], dy[0], dy[1], dw, sgny*DY);
  }
  
  Double_t pos[3]={ain[0], ain[1], ain[2]},
           ldx(0.), ldy(0.),
           dxy(0.), e(0.), etr(0.);
  for(Int_t icl(0); icl<ncls; icl++){
    if(!icl){ldx=dx[0]; ldy=dy[0];}  
    else if(icl==ncls-1){ldx=dx[1]; ldy=dy[1];}  
    else{ldx=dxw; ldy=dw;}
   
    dxy=ldx*ldx+ldy*ldy;
    if(dxy<=0){
      LOG(ERROR) << "CbmTrdDigitizerPRF::SplitTrackPathTriang: NULL projected track length in cluster "<<icl
        <<" for track length[cm] ("<<std::setprecision(5)<<ldx<<", "<<std::setprecision(2)<<ldy<<")."
        " dEdx("<<std::setprecision(5)<<ELoss*1e6<<") keV " << FairLogger::endl;
      continue;
    }
    dxy=TMath::Sqrt(dxy);
    if(VERBOSE_TRIANG) printf("    %d ldx[%7.4f] ldy[%7.4f] xy[%7.4f] frac=%7.2f%%\n", icl, ldx, ldy, dxy, 1.e2*dxy/txy);

    Double_t  dEdx(dxy/txy),
              dEdxTR(dxy/txy),
              cELoss(ELoss*dEdx),         // continuos energy deposit 
              cELossTR(ELossTR*dEdxTR);   // TODO decay distribution of energy 
    e+=cELoss; etr+=cELossTR;
    pos[0]+=0.5*ldx*sgnx; 
    ScanPadPlaneTriangleAB(pos, ldx, 1e6*cELoss, 1e6*cELossTR);
    pos[0]+=0.5*ldx*sgnx; 
    pos[1]+=dw*sgny;
  }
  if(TMath::Abs(lout[0]-pos[0])>1.e-9){
    LOG(WARNING) << "CbmTrdDigitizerPRF::SplitTrackPathTriang: Along wire coordinate error : x_sim="
      <<std::setprecision(5)<<lout[0]<<" x_calc="<<std::setprecision(5)<<pos[0] << FairLogger::endl;
  }
  if(TMath::Abs(ELoss-e)>1.e-9){
    LOG(WARNING) << "CbmTrdDigitizerPRF::SplitTrackPathTriang: dEdx partition to anode wires error : E[keV] = "
      <<std::setprecision(5)<<ELoss*1e6<<" Sum(Ei)[keV]="<<std::setprecision(5)<<e*1e6 << FairLogger::endl;
  }
  if(etr>ELossTR){
    LOG(WARNING) << "CbmTrdDigitizerPRF::SplitTrackPathTriang: TR energy partition to anode wires error : Etr[keV] = "
      <<std::setprecision(5)<<ELossTR*1e6<<" Sum(Ei)[keV]="<<std::setprecision(5)<<etr*1e6 << FairLogger::endl;
  }
}

//_______________________________________________________________________________________________
void CbmTrdDigitizerPRF::ScanPadPlaneTriangleAB(const Double_t* point, Double_t DX, Double_t ELoss, Double_t ELossTR)
{
/**  
  The hit is expressed in local chamber coordinates, localized as follows:
    - Along the wire in the middle of the track projection on the closest wire
    - Across the wire on the closest anode.
    
  The physical uncertainty along wires is given by the projection span (dx) and the energy from ionisation is proportional to the track projection length in the local chamber x-y plane. For the TR energy the proportionality to the total TR is given by the integral over the amplification cell span of a decay law with decay constant ...
  
  The class CbmTrdTriangle is used to navigate the pad plane outward from the hit position until a threshold wrt to center is reached. The pad-row cross clusters are considered. Finally all digits are registered via AddDigi() function. 
*/
  if(VERBOSE_TRIANG) printf("      xy[%7.4f %7.4f] D[%7.4f] E[keV]=%7.4f\n", point[0], point[1], DX, ELoss);

  Int_t sec(-1), col(-1), row(-1);
  fModuleInfo->GetPadInfo(point, sec, col, row);
  if (sec < 0 || col < 0 || row < 0) {
    LOG(ERROR) << "CbmTrdDigitizerPRF::ScanPadPlaneTriangleAB: Hit to pad matching failed for ["
      <<std::setprecision(5)<<point[0]<<", "
      <<std::setprecision(5)<<point[1]<<", "
      <<std::setprecision(5)<<point[2]<<"]." << FairLogger::endl;
    return;
  }
  for (Int_t is(0); is < sec; is++) row += fModuleInfo->GetNofRowsInSector(is);
  if(VERBOSE_TRIANG) printf("      Found pad @ col[%d] row[%d]\n", col, row);
  
  Double_t dx, dy;
  fModuleInfo->TransformToLocalPad(point, dx, dy);
  if(VERBOSE_TRIANG) printf("      pad x[%7.4f] y[%7.4f]\n", dx, dy);

  // build binning if called for the first time. Don't care about sector information as Bucharest has only 1 type of pads
  if(!fTriangleBinning) 
    fTriangleBinning=new CbmTrdTriangle(fModuleInfo->GetPadSizeX(1), fModuleInfo->GetPadSizeY(1));
  if(!fTriangleBinning->SetOrigin(dx, dy)){
    LOG(WARNING) << "CbmTrdDigitizerPRF::ScanPadPlaneTriangleAB: Hit outside integration limits ["
      <<std::setprecision(5)<<dx<<", "
      <<std::setprecision(5)<<dy<<"]." << FairLogger::endl;
    return;
  }
  
  // set minimum threshold for all channels [keV]
  // TODO should be stored/computed in CbmTrdModule via triangular/FASP digi param
  Double_t epsilon=1.e-4;

  // local storage for digits on a maximum area of 5x3 columns for up[1]/down[0] pads
  const Int_t nc=2*CbmTrdTriangle::NC+1;
  const Int_t nr=2*CbmTrdTriangle::NR+1;
  Double_t array[nc][nr][2] = {{{0.}}}, prf(0.);
  Int_t colOff, rowOff, up, bx, by;
  
  // look right
  do{
    // check if there is any contribution on this bin column
    if(fTriangleBinning->GetChargeFraction()<=epsilon) break;
    
    // look up
    do{
      prf = fTriangleBinning->GetChargeFraction();
      fTriangleBinning->GetCurrentPad(colOff, rowOff, up);
      if( colOff<0 || colOff>=nc ||
          rowOff<0 || rowOff>=nr){
        printf("CbmTrdDigitizerPRF::ScanPadPlaneTriangleAB: Bin outside mapped array : col[%d] row[%d]\n", colOff, rowOff);
        break;
      }
      //fTriangleBinning->GetCurrentBin(bx, by);
      //printf("      {ru} bin[%2d %2d] c[%d] r[%d] u[%2d] PRF[%f]\n", bx, by, colOff, rowOff, up, prf);
      if(up) array[colOff][rowOff][(up>0?1:0)] += prf;
      else{ 
        array[colOff][rowOff][0] += 0.5*prf;
        array[colOff][rowOff][1] += 0.5*prf;
      }
    } while(fTriangleBinning->NextBinY() && prf>=epsilon);
    fTriangleBinning->GoToOriginY();
      //printf("\n");
    
    // skip bin @ y0 which was calculated before
    if(!fTriangleBinning->PrevBinY()) continue;
    
    // look down
    do{
      prf = fTriangleBinning->GetChargeFraction();
      fTriangleBinning->GetCurrentPad(colOff, rowOff, up);
      if( colOff<0 || colOff>=nc ||
          rowOff<0 || rowOff>=nr){
        printf("CbmTrdDigitizerPRF::ScanPadPlaneTriangleAB: Bin outside mapped array : col[%d] row[%d]\n", colOff, rowOff);
        break;
      }
      //fTriangleBinning->GetCurrentBin(bx, by);
      //printf("      {rd} bin[%2d %2d] c[%d] r[%d] u[%2d] PRF[%f]\n", bx, by, colOff, rowOff, up, prf);
      if(up) array[colOff][rowOff][(up>0?1:0)] += prf;
      else{ 
        array[colOff][rowOff][0] += 0.5*prf;
        array[colOff][rowOff][1] += 0.5*prf;
      }
    } while(fTriangleBinning->PrevBinY() && prf>=epsilon);
    fTriangleBinning->GoToOriginY();
      //printf("\n");
    
  } while(fTriangleBinning->NextBinX());    
  fTriangleBinning->GoToOriginX();
  

  if(fTriangleBinning->PrevBinX()){ // skip bin @ x0 which was calculated before
    // look left
    do{
      // check if there is any contribution on this bin column
      if(fTriangleBinning->GetChargeFraction()<=epsilon) break;

      // look up
      do{
        prf = fTriangleBinning->GetChargeFraction();
        fTriangleBinning->GetCurrentPad(colOff, rowOff, up);
        if( colOff<0 || colOff>=nc ||
            rowOff<0 || rowOff>=nr){
          printf("CbmTrdDigitizerPRF::ScanPadPlaneTriangleAB: Bin outside mapped array : col[%d] row[%d]\n", colOff, rowOff);
          break;
        }
        //fTriangleBinning->GetCurrentBin(bx, by);
        //printf("      {lu} bin[%2d %2d] c[%d] r[%d] u[%2d] PRF[%f]\n", bx, by, colOff, rowOff, up, prf);
        if(up) array[colOff][rowOff][(up>0?1:0)] += prf;
        else{ 
          array[colOff][rowOff][0] += 0.5*prf;
          array[colOff][rowOff][1] += 0.5*prf;
        }
      } while(fTriangleBinning->NextBinY() && prf>=epsilon);
      fTriangleBinning->GoToOriginY();

      // skip bin @ y0 which was calculated before
      if(!fTriangleBinning->PrevBinY()) continue;
      
      // look down
      do{
        prf = fTriangleBinning->GetChargeFraction();
        fTriangleBinning->GetCurrentPad(colOff, rowOff, up);
        if( colOff<0 || colOff>=nc ||
            rowOff<0 || rowOff>=nr){
          printf("CbmTrdDigitizerPRF::ScanPadPlaneTriangleAB: Bin outside mapped array : col[%d] row[%d]\n", colOff, rowOff);
          break;
        }
        //fTriangleBinning->GetCurrentBin(bx, by);
        //printf("      {ld} bin[%2d %2d] c[%d] r[%d] u[%2d] PRF[%f]\n", bx, by, colOff, rowOff, up, prf);
        if(up) array[colOff][rowOff][(up>0?1:0)] += prf;
        else{ 
          array[colOff][rowOff][0] += 0.5*prf;
          array[colOff][rowOff][1] += 0.5*prf;
        }
      } while(fTriangleBinning->PrevBinY() && prf>=epsilon);
      fTriangleBinning->GoToOriginY();
      //printf("\n");
      
    } while(fTriangleBinning->PrevBinX());    
  }
  fTriangleBinning->GoToOriginX();
      //printf("\n");
  if(VERBOSE_TRIANG) {
    printf("        "); for(Int_t ic(0); ic<5; ic++) printf("%7d[u/d]  ", ic); printf("\n");
    for(Int_t ir(nr); ir--; ){
      printf("      r[%d] ", ir);
      for(Int_t ic(0); ic<nc; ic++) printf("%6.4f/%6.4f ", array[ic][ir][1], array[ic][ir][0]);
      printf("\n");
    }
  }
  
  // pair pads and convert to ADC
  // calibration ADC -> keV based on 55Fe measurements as presented @
  //https://indico.gsi.de/event/4760/session/6/contribution/58/material/slides/0.pdf on slide 14
  // TODO should be stored/computed in CbmTrdModule via triangular/FASP digi param
  const Float_t ECalib[]={-528./380., 1./380.};
  Double_t Emeasure(0.);
  for(Int_t ir(nr); ir--; ){
    for(Int_t ic(0); ic<nc; ic++){
      for(Int_t iup(0); iup<2; iup++){
        if(array[ic][ir][iup]<=epsilon) continue;
        array[ic][ir][iup]*=ELoss/fTriangleBinning->Norm();
        array[ic][ir][iup]*=ELoss;
        Emeasure+=array[ic][ir][iup];
        array[ic][ir][iup] = (array[ic][ir][iup]-ECalib[0])*380.;
      }
      if(ic>0) array[ic-1][ir][0]+=array[ic][ir][1];  // add top pad to previous tilt pair
      array[ic][ir][1] += array[ic][ir][0];           // add bottom pad to current rect pair
    }
  }
  if(VERBOSE_TRIANG) {
    printf("  Edep[keV]=%6.4f Emes[keV]=%6.4f\n", ELoss, Emeasure);
    printf("        "); for(Int_t ic(0); ic<5; ic++) printf("%7d[R/T]  ", ic); printf("\n");
    for(Int_t ir(nr); ir--; ){
      printf("      r[%d] ", ir);
      for(Int_t ic(0); ic<nc; ic++) printf("%6.1f/%6.1f ", array[ic][ir][1], array[ic][ir][0]);
      printf("\n");
    }
  }
  // register digitisation to container
  Int_t address(0);
  for(Int_t ir(0); ir<nr; ir++){
    for(Int_t ic(0); ic<nc; ic++){  
      address=0;
      for(Int_t iup(0); iup<2; iup++){
        // check if there are data available
        if(array[ic][ir][iup]<=epsilon) continue;

        // check if column is inside pad-plane        
        Int_t wcol(col+ic-CbmTrdTriangle::NC);
        if( wcol<0 || wcol>=fnCol ) continue;

        // check if row is inside pad-plane
        Int_t wrow(row+ir-CbmTrdTriangle::NR);
        if( wrow<0 || wrow>=fnRow) continue;

        // move row id to sector wise schema
        Int_t srow, isec=fModuleInfo->GetSectorRow(wrow, srow);
        // compute pixel address
        address = CbmTrdAddress::GetAddress(fLayerId, fModuleId, isec, srow, wcol);
        AddDigi(fMCPointId, address, 
              Double_t(array[ic][ir][iup]),
              Double_t(array[ic][ir][iup] * ELossTR/ELoss), fCurrentTime, iup);
      }
      if(address>0 && VERBOSE_TRIANG){
        std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it = fDigiMap.find(address);
        cout<<(it->second.first)->ToString();
      }
    }
  }
}

ClassImp(CbmTrdDigitizerPRF)
			 

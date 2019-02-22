#include "CbmTrdDigitizer.h"

#include "CbmTrdPads.h"
#include "CbmTrdRadiator.h"
#include "CbmTrdParSetAsic.h"
#include "CbmTrdParSetGas.h"
#include "CbmTrdParSetDigi.h"
#include "CbmTrdParSetGain.h"
#include "CbmTrdParSetGeo.h"
#include "CbmTrdParAsic.h"
#include "CbmTrdParModGeo.h"
#include "CbmTrdParModGas.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParModGain.h"
#include "CbmTrdPoint.h"
#include "CbmTrdDigi.h"
#include "CbmTrdGeoHandler.h"
#include "CbmTrdModuleSim.h"
#include "CbmTrdModuleSimT.h"
#include "CbmTrdModuleSimR.h"
#include "CbmTrdAddress.h"
#include "CbmMCTrack.h"
#include "CbmMatch.h"
#include "CbmDaqBuffer.h"

#include <FairRootManager.h>
#include <FairEventHeader.h>
#include <FairRunSim.h>
#include <FairRunAna.h>
#include <FairRuntimeDb.h>
#include <FairBaseParSet.h>
#include <FairLogger.h>

#include <TRandom.h>
#include <TVector3.h>
#include <TClonesArray.h>
#include <TStopwatch.h>

#include <iostream>
#include <iomanip>
#include <cmath>
using std::cout;
using std::endl;
using std::pair;
using std::make_pair;
using std::map;
using std::vector;
using std::max;
using namespace std;
Int_t CbmTrdDigitizer::fConfig = 0;

//________________________________________________________________________________________
CbmTrdDigitizer::CbmTrdDigitizer(CbmTrdRadiator* radiator)
  : CbmDigitize("TrdDigitize")
  ,fLastEventTime(0)
  ,fpoints(0)
  ,nofBackwardTracks(0)
  ,fEfficiency(1.)
  ,fPoints(NULL)
  ,fTracks(NULL)
  ,fDigis(NULL)
  ,fDigiMatches(NULL)
  ,fAsicPar(NULL)
  ,fGasPar(NULL)
  ,fDigiPar(NULL)
  ,fGainPar(NULL)
  ,fGeoPar(NULL)
  ,fRadiator(radiator)
//  ,fGeoHandler(new CbmTrdGeoHandler())
  ,fModuleMap()
  ,fDigiMap()
{
  if(fRadiator == NULL) fRadiator = new CbmTrdRadiator(kTRUE,"K++");
}

//________________________________________________________________________________________
CbmTrdDigitizer::~CbmTrdDigitizer()
{
  fDigis->Clear("C");
  delete fDigis;
  fDigiMatches->Clear("C");
  delete fDigiMatches;
  //delete fGeoHandler;

  for(map<Int_t, CbmTrdModuleSim*>::iterator imod=fModuleMap.begin(); imod!=fModuleMap.end(); imod++) delete imod->second;
  fModuleMap.clear();
}

//________________________________________________________________________________________
void CbmTrdDigitizer::SetParContainers()
{
  fAsicPar = static_cast<CbmTrdParSetAsic*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetAsic"));
  fGasPar = static_cast<CbmTrdParSetGas*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetGas"));
  fDigiPar = static_cast<CbmTrdParSetDigi*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetDigi"));
  fGainPar = static_cast<CbmTrdParSetGain*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetGain"));
  fGeoPar = new CbmTrdParSetGeo(); //fGeoPar->Print();
}

//________________________________________________________________________________________
InitStatus CbmTrdDigitizer::Init()
{
  FairRootManager* ioman = FairRootManager::Instance();
  if (!ioman) LOG(FATAL) << "CbmTrdDigitizer::Init: No FairRootManager" << FairLogger::endl;

  fPoints = (TClonesArray*) ioman->GetObject("TrdPoint");
  if (!fPoints) LOG(FATAL) << "CbmTrdDigitizer::Init(): No TrdPoint array!" << FairLogger::endl;

  fTracks = (TClonesArray*)ioman->GetObject("MCTrack");
  if (!fTracks) LOG(FATAL) << "CbmTrdDigitizer::Init(): No MCTrack array!" << FairLogger::endl;

  if (fRadiator)  fRadiator->Init();    

  // Set time-based mode if appropriate
  SetTimeBased(fEventMode ? kFALSE : kTRUE);
  
  fDigis = new TClonesArray("CbmTrdDigi", 100);
  ioman->Register("TrdDigi", "TRD", fDigis, IsOutputBranchPersistent("TrdDigi"));

  fDigiMatches = new TClonesArray("CbmMatch", 100);
  ioman->Register("TrdDigiMatch", "TRD", fDigiMatches, IsOutputBranchPersistent("TrdDigiMatch"));

  LOG(INFO) << "================ TRD Digitizer ===============" << FairLogger::endl;
  LOG(INFO) << " Free streaming    : " << (IsTimeBased()?"yes":"no")<< FairLogger::endl;
  LOG(INFO) << " Add Noise         : " << (AddNoise()?"yes":"no")<< FairLogger::endl;
  LOG(INFO) << " Weighted distance : " << (UseWeightedDist()?"yes":"no")<< FairLogger::endl;

  return kSUCCESS;
}

//________________________________________________________________________________________
void CbmTrdDigitizer::Exec(Option_t*)
{
//  fDigis->Delete();
//  fDigiMatches->Delete();

  // start timer
  TStopwatch timer; timer.Start();

  // get event info (once per event, used for matching)
  GetEventInfo();
  
  // reset private monitoring counters
  ResetCounters();
  
  // loop tracks in current event
  CbmTrdModuleSim *mod(NULL);
  Int_t nofPoints = fPoints->GetEntriesFast();
  gGeoManager->CdTop();
  for (Int_t iPoint = 0; iPoint < nofPoints ; iPoint++) {
    fpoints++;
    //fMCPointId = iPoint;
    
    CbmTrdPoint* point = static_cast<CbmTrdPoint*>(fPoints->At(iPoint));
    if(!point) continue;
    const CbmMCTrack* track = static_cast<const CbmMCTrack*>(fTracks->At(point->GetTrackID()));
    if(!track) continue;

    Double_t dz = point->GetZOut() - point->GetZIn();
    if (dz < 0) {
      LOG(DEBUG2) << GetName() <<"::Exec: MC-track points towards target!" << FairLogger::endl;
      nofBackwardTracks++;
    }
    
    // get link to the module working class
    map<Int_t, CbmTrdModuleSim*>::iterator imod = fModuleMap.find(point->GetDetectorID());
    if(imod==fModuleMap.end()){ 
      // Looking for gas node corresponding to current point in geo manager
      Double_t meanX = (point->GetXOut() + point->GetXIn()) / 2.;
      Double_t meanY = (point->GetYOut() + point->GetYIn()) / 2.;
      Double_t meanZ = (point->GetZOut() + point->GetZIn()) / 2.;
      gGeoManager->FindNode(meanX, meanY, meanZ);
      if (!TString(gGeoManager->GetPath()).Contains("gas")){
        LOG(ERROR) << GetName()<<"::Exec: MC-track not in TRD! Node:" << TString(gGeoManager->GetPath()).Data() << " gGeoManager->MasterToLocal() failed!" << FairLogger::endl;
        continue;
      }
      mod = AddModule(point->GetDetectorID());
    } else mod = imod->second;
    mod->SetLinkId(fCurrentInput, fCurrentMCEntry, iPoint);
    Double_t gamma = TMath::Sqrt(1+TMath::Power(track->GetP()/(3.e8*track->GetMass()),2));
    mod->SetGamma(gamma);
    mod->MakeDigi(point, fCurrentEventTime, TMath::Abs(track->GetPdgCode()) == 11);
  }

  // Fill data from internally used stl map into CbmDaqBuffer.
  // Calculate final event statistics
  Int_t nDigis(0), nofElectrons(0), nofLatticeHits(0), nofPointsAboveThreshold(0), n0, n1, n2;
  for(map<Int_t, CbmTrdModuleSim*>::iterator imod = fModuleMap.begin(); imod != fModuleMap.end(); imod++) {
    // in streaming mode flush buffers only up to a certain point in time wrt to current event time (allow for event pile-ups)
    //printf("Processing data for module %d\n", imod->first);
    if(IsTimeBased()) nDigis += imod->second->FlushBuffer(fCurrentEventTime);  
    // in event-by-event mode flush all buffers     
    else imod->second->FlushBuffer();                               
    imod->second->GetCounters(n0, n1, n2);
    nofElectrons+=n0; nofLatticeHits+=n1; nofPointsAboveThreshold+=n2;
    std::map<Int_t, std::pair<CbmTrdDigi*, CbmMatch*>> *digis = imod->second->GetDigiMap();
    //printf("  Digits[%d] %d\n", imod->first, digis->size());    
    for (std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it = digis->begin() ; it != digis->end(); it++) {
      (it->second.first)->SetMatch(it->second.second);
      SendDigi(it->second.first); 
      nDigis++;
    } //# modules
    digis->clear();
  } //# digis
  fLastEventTime=fCurrentEventTime;
    
  
  Double_t digisOverPoints = (nofPoints > 0) ? Double_t(nDigis) / Double_t(nofPoints) : 0;
  Double_t latticeHitsOverElectrons = (nofElectrons > 0) ? (Double_t) nofLatticeHits / (Double_t) nofElectrons : 0;
  LOG(DEBUG) << "CbmTrdDigitizer::Exec Points:               " << nofPoints << FairLogger::endl;
  LOG(DEBUG) << "CbmTrdDigitizer::Exec PointsAboveThreshold: " << nofPointsAboveThreshold << FairLogger::endl;
  LOG(DEBUG) << "CbmTrdDigitizer::Exec Digis:                " << nDigis << FairLogger::endl;
  LOG(DEBUG) << "CbmTrdDigitizer::Exec digis/points:         " << digisOverPoints << FairLogger::endl;
  LOG(DEBUG) << "CbmTrdDigitizer::Exec BackwardTracks:       " << nofBackwardTracks << FairLogger::endl;
  LOG(DEBUG) << "CbmTrdDigitizer::Exec LatticeHits:          " << nofLatticeHits  << FairLogger::endl;
  LOG(DEBUG) << "CbmTrdDigitizer::Exec Electrons:            " << nofElectrons << FairLogger::endl;
  LOG(DEBUG) << "CbmTrdDigitizer::Exec latticeHits/electrons:" << latticeHitsOverElectrons << FairLogger::endl;
  timer.Stop();
  LOG(DEBUG) << "CbmTrdDigitizer::Exec real time=" << timer.RealTime()
            << " CPU time=" << timer.CpuTime() << FairLogger::endl;

  // --- Event log
  LOG(INFO) << "+ " << setw(15) << GetName() << ": Event " << setw(6)
         << right << fCurrentEvent << " at " << fixed << setprecision(3)
         << fCurrentEventTime << " ns, points: " << nofPoints
         << ", digis: " << nDigis << ". Exec time " << setprecision(6)
         << timer.RealTime() << " s." << FairLogger::endl;

}

//________________________________________________________________________________________
void CbmTrdDigitizer::FlushBuffers()
{
  LOG(INFO) << GetName() << ": Processing analogue buffers"<< FairLogger::endl;
  Int_t nDigis(0);
  for(map<Int_t, CbmTrdModuleSim*>::iterator imod = fModuleMap.begin(); imod != fModuleMap.end(); imod++) {
    nDigis += imod->second->FlushBuffer();                               
    std::map<Int_t, std::pair<CbmTrdDigi*, CbmMatch*>> *digis = imod->second->GetDigiMap();
    for (std::map<Int_t, pair<CbmTrdDigi*, CbmMatch*> >::iterator it = digis->begin() ; it != digis->end(); it++) {
      (it->second.first)->SetMatch(it->second.second);
      SendDigi(it->second.first); 
      nDigis++;
    } //# modules
    digis->clear();
  } //# digis
  LOG(INFO) << GetName() << ": " << nDigis
        << ( nDigis == 1 ? " digi " :  " digis " )
        << "created and sent to DAQ "<< FairLogger::endl;
}

//________________________________________________________________________________________
void CbmTrdDigitizer::Finish()
{
  // flush buffers in streaming mode    
  LOG(INFO) << "=====================================" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Finish run" << FairLogger::endl;
  if(IsTimeBased()) FlushBuffers(); 
  LOG(INFO) << GetName() << ": Run summary " << FairLogger::endl;
  LOG(INFO) << "=====================================" << FairLogger::endl;
}

//________________________________________________________________________________________
CbmTrdModuleSim* CbmTrdDigitizer::AddModule(Int_t detId)
{
/**  The geometry structure is treelike with cave as
 * the top node. For the TRD there are keeping volume
 * trd_vXXy_1 which is only container for the different layers.
 * The trd layer is again only a container for all volumes of this layer.
 * Loop over all nodes below the top node (cave). If one of
 * the nodes contains a string trd it must be TRD detector.
 * Now loop over the layers and 
 * then over all modules of the layer to extract in the end
 * all active regions (gas) of the complete TRD. For each
 * of the gas volumes get the information about size and
 * position from the geomanager and the sizes of the sectors
 * and pads from the definitions in CbmTrdPads. This info
 * is then stored in a CbmTrdModule object for each of the TRD modules.
 **/ 

  const char *path=gGeoManager->GetPath();
  CbmTrdGeoHandler geoHandler;
  Int_t moduleAddress = geoHandler.GetModuleAddress(path),
        moduleType    = geoHandler.GetModuleType(path),
        orientation   = geoHandler.GetModuleOrientation(path),
        lyId          = CbmTrdAddress::GetLayerId(detId);
  if(moduleAddress != detId){
    LOG(ERROR) << "CbmTrdDigitizer::AddModule: MC module ID " << detId << " does not match geometry definition "<< moduleAddress << ". Module init failed!" << FairLogger::endl;
    return NULL;
  }
  LOG(DEBUG) << GetName() << "::AddModule("<<path<<" "<< (moduleType<=0?'T':'R')<<
    "] mod[" << moduleAddress <<
    "] ly["  << lyId <<
    "] det[" << detId <<"]"<<FairLogger::endl;
  CbmTrdModuleSim *module(NULL);
  if(moduleType<=0){
    module = fModuleMap[moduleAddress] = new CbmTrdModuleSimT(moduleAddress, lyId, orientation, UseFASP());
  } else {
    module = fModuleMap[moduleAddress] = new CbmTrdModuleSimR(moduleAddress, lyId, orientation);  
  }
  
  // try to load Geometry parameters for module
  const CbmTrdParModGeo *pGeo(NULL);
  if(!fGeoPar || !(pGeo = (const CbmTrdParModGeo *)fGeoPar->GetModulePar(detId))){
    LOG(DEBUG) << GetName() << "::AddModule : No Geo params for module @ "<< path <<". Using default."<< FairLogger::endl;
    module->SetGeoPar(new CbmTrdParModGeo(Form("TRD_%d", detId), path));
  } else module->SetGeoPar(pGeo);

  // try to load read-out parameters for module
  const CbmTrdParModDigi *pDigi(NULL);
  if(!fDigiPar || !(pDigi = (const CbmTrdParModDigi *)fDigiPar->GetModulePar(detId))){
    LOG(DEBUG) << GetName() << "::AddModule : No Read-Out params for module @ "<< path <<". Using default."<< FairLogger::endl;
  } else module->SetDigiPar(pDigi);

  // try to load ASIC parameters for module
  CbmTrdParSetAsic *pAsic(NULL);
  if(!fAsicPar || !(pAsic = (CbmTrdParSetAsic *)fAsicPar->GetModuleSet(detId))){
    LOG(DEBUG) << GetName() << "::AddModule : No ASIC params for module @ "<< path <<". Using default."<< FairLogger::endl;
    module->SetAsicPar(); // map ASIC channels to read-out channels - need ParModDigi already loaded
  } else module->SetAsicPar(pAsic);

  // try to load Chamber parameters for module
  const CbmTrdParModGas *pChmb(NULL);
  if(!fGasPar || !(pChmb = (const CbmTrdParModGas *)fGasPar->GetModulePar(detId))){
    LOG(DEBUG) << GetName() << "::AddModule : No Gas params for module @ "<< path <<". Using default."<< FairLogger::endl;
  } else module->SetChmbPar(pChmb);

  // try to load Gain parameters for module
  const CbmTrdParModGain *pGain(NULL);
  if(!fGainPar || !(pGain = (const CbmTrdParModGain *)fGainPar->GetModulePar(detId))){
    LOG(DEBUG) << GetName() << "::AddModule : No Gain params for module @ "<< path <<". Using default."<< FairLogger::endl;
  } else module->SetGainPar(pGain);

  if(fRadiator) module->SetRadiator(fRadiator);
  
  return module;
}

//________________________________________________________________________________________
void CbmTrdDigitizer::ResetCounters()
{
/** Loop over modules and calls ResetCounters on each
 */
  fpoints = 0;
  nofBackwardTracks = 0;
  for(std::map<Int_t, CbmTrdModuleSim*>::iterator imod=fModuleMap.begin(); imod!=fModuleMap.end(); imod++) imod->second->ResetCounters();
}

// -----   Reset output arrays   -------------------------------------------

void CbmTrdDigitizer::ResetArrays() {

  fDigis->Delete();

  fDigiMatches->Delete();

}

// -----   Write a digi to the output   ----------------------------------------

void CbmTrdDigitizer::WriteDigi(CbmDigi* digi) {

  CbmTrdDigi* trdDigi = dynamic_cast<CbmTrdDigi*>(digi);

  assert(trdDigi);

  Int_t index = fDigis->GetEntriesFast();

  if ( digi->GetMatch() ) new ((*fDigiMatches)[index]) CbmMatch(*(digi->GetMatch()));
  new ((*fDigis)[index]) CbmTrdDigi(*trdDigi);

}

// -----------------------------------------------------------------------------


ClassImp(CbmTrdDigitizer)

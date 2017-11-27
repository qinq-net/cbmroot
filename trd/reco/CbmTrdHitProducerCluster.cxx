#include "CbmTrdHitProducerCluster.h"
#include "CbmTrdDigiPar.h"
#include "CbmTrdDigi.h"
#include "CbmTrdPoint.h"
#include "CbmTrdCluster.h"
#include "CbmTrdHit.h"
#include "CbmTrdModule.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TGeoManager.h"
#include "TMath.h"
#include "TF1.h"
#include "TGraphErrors.h"

#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;

CbmTrdHitProducerCluster::CbmTrdHitProducerCluster()
  :FairTask("CbmTrdHitProducerCluster",1),
   fDigis(NULL),
   fClusters(NULL),
   fHits(NULL),
   fDigiPar(NULL),
   fModule(NULL),
   fModuleHits(NULL),
   fTriangularSignal(NULL),
   fTriangularPRF(NULL),
   fTrianglePads(false),
   fRecoTriangular(0)
{
}

CbmTrdHitProducerCluster::~CbmTrdHitProducerCluster()
{
   fHits->Delete();
   delete fHits;
   if(fModuleHits) delete fModuleHits; //????
   if(fTriangularPRF) delete fTriangularPRF;
   if(fTriangularSignal) delete fTriangularSignal;
}

void CbmTrdHitProducerCluster::SetParContainers()
{
   fDigiPar = (CbmTrdDigiPar*)(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdDigiPar"));
}

InitStatus CbmTrdHitProducerCluster::Init()
{
   FairRootManager *ioman = FairRootManager::Instance();

   fDigis = (TClonesArray *) ioman->GetObject("TrdDigi");
   if (fDigis == NULL) LOG(FATAL) << "CbmTrdHitProducerCluster::Init No TrdDigi array." << FairLogger::endl;

   fClusters = (TClonesArray *) ioman->GetObject("TrdCluster");
   if (fClusters == NULL) LOG(FATAL) << "CbmTrdHitProducerCluster::Init No TrdCluster array." << FairLogger::endl;

   fHits = new TClonesArray("CbmTrdHit", 100);
   ioman->Register("TrdHit","TRD Hit",fHits,IsOutputBranchPersistent("TrdHit"));

   return kSUCCESS;
}
void CbmTrdHitProducerCluster::SetTriangularPads(Bool_t triangles)
{
  fTrianglePads = triangles;
}
void CbmTrdHitProducerCluster::Exec(Option_t*)
{
  fHits->Delete();
  // reset status variable for triangular TRD
  fRecoTriangular=0; fModule=NULL;
  
  TStopwatch timer;
  timer.Start();
  cout << "================CbmTrdHitProducerCluster===============" << endl;
  //LOG(INFO) << "CbmTrdHitProducerCluster::Exec : Triangular Pads: " << (Bool_t)fTrianglePads << FairLogger::endl;
  Int_t nofCluster = fClusters->GetEntries(), clsTriangular(0);
  
  for (Int_t iCluster = 0; iCluster < nofCluster; iCluster++) {
    const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*>(fClusters->At(iCluster));

    if(cluster->HasTrianglePads()){ 
      clsTriangular++;
      TriangularPadReconstructionAB(iCluster);
    } else CenterOfGravity(iCluster);
  }
  if(fRecoTriangular && fModuleHits->size()) TriangularFinalizeModule();

  LOG(INFO) << "CbmTrdHitProducerCluster::Exec : Digis:          " << fDigis->GetEntriesFast()  << FairLogger::endl;
  LOG(INFO) << "CbmTrdHitProducerCluster::Exec : [R]clusters:    " << nofCluster-clsTriangular << FairLogger::endl;
  LOG(INFO) << "CbmTrdHitProducerCluster::Exec : [R]hits:        " << fHits->GetEntriesFast() - fRecoTriangular << FairLogger::endl;
  LOG(INFO) << "CbmTrdHitProducerCluster::Exec : [T]clusters:    " << clsTriangular << FairLogger::endl;
  LOG(INFO) << "CbmTrdHitProducerCluster::Exec : [T]hits:        " << fRecoTriangular << FairLogger::endl;

  timer.Stop();
  LOG(INFO) << "CbmTrdHitProducerCluster::Exec : real time=" << timer.RealTime()
	    << " CPU time=" << timer.CpuTime() << FairLogger::endl;
}
Double_t CbmTrdHitProducerCluster::CalcDisplacement(Double_t Q_left, Double_t Q_center, Double_t Q_right, Double_t padWidth){
  //printf("%E, %E, %E, pW:%f\n", Q_left, Q_center, Q_right, padWidth);
  if (Q_left > 0.0 && Q_center > 0.0 && Q_right > 0.0)
    return  0.5 * padWidth * TMath::Log(Q_right / Q_left) / TMath::Log((Q_center * Q_center) / (Q_left * Q_right));
  else
    return 0.0;
}
Double_t CbmTrdHitProducerCluster::CalcY(Double_t padWidth, Double_t padHeight, Double_t rect_Dis, Double_t para_Dis, Int_t digiRow){
  Double_t alpha = TMath::ATan(padWidth / (2*padHeight));
  //printf("alpha: %f H:%f  W:%f\n",alpha, padHeight, padWidth);
  if (digiRow%2 == 0)
    return (para_Dis - (rect_Dis - 0.5 * padWidth) * TMath::Cos(alpha)) / (-1. * TMath::Sin(alpha));
  else
    return (para_Dis - (rect_Dis + 0.5 * padWidth) * TMath::Cos(alpha)) / (-1. * TMath::Sin(alpha));
}
//Bool_t combiIdSorter(std::pair a, std::pair b)
//{return (a.first < b.first); }
//____________________________________________________________________________
UInt_t setTriangleCombId(Int_t row, Int_t col, Int_t ncol, Int_t tri)
{
//  printf("    setTriangleCombId(%2d %2d/%2d %d) -> %4d\n", row, col, ncol, tri, 2 * (row * (ncol+2) + col+1)+tri);
  return 2 * (row * (ncol+2) + col+1)+tri;
}

//____________________________________________________________________________
void getTriangleCombId(UInt_t comb, Int_t ncol, Int_t &row, Int_t &col, Int_t &tri)
{ 
/** Decrypt triangular pad id
 */
  tri = comb&1; 
  Int_t rc(comb>>1);
  row=rc/(ncol+2); 
  col=rc%(ncol+2)-1;
  //printf("    getTriangleCombId(%4d %2d) -> "
  //         " t[%d] r[%2d] c[%2d]\n", comb, ncol, tri, row, col);
}

//____________________________________________________________________________
void CbmTrdHitProducerCluster::TriangularPadReconstructionAB(Int_t clusterId)
{
  //printf("\nTriangularPadReconstructionAB(%d) ...\n", clusterId);
  const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*>(fClusters->At(clusterId));
 
  Double_t totalCharge = 0;
  Double_t totalChargeTR = 0;
  Int_t nofDigis = cluster->GetNofDigis();

  Int_t digiId = -1;
  Int_t combiId(-1), secId(-1), colId(-1), rowId(-1), rowOld(-1), triId(-1), nCol(0);

  Double_t padH(0.), padW(0.);
  
  // retrieve digits information
  std::map<UInt_t, std::pair<Double_t, UChar_t>> digiMap; // combiId, <DigiCharge, DigiTime>
  Int_t moduleAddress(0), digiAddress(-1);
  for (Int_t iDigi = 0; iDigi < nofDigis; iDigi++) {
    digiId = cluster->GetDigi(iDigi);
    const CbmTrdDigi* digi = static_cast<const CbmTrdDigi*>(fDigis->At(digiId));
    digiAddress = digi->GetAddress();
    secId = CbmTrdAddress::GetSectorId(digiAddress);
    colId = CbmTrdAddress::GetColumnId(digiAddress);
    rowId = CbmTrdAddress::GetRowId(digiAddress);
    triId = CbmTrdAddress::GetTriangleId(digiAddress);
    if(!moduleAddress){
      moduleAddress = CbmTrdAddress::GetModuleAddress(digiAddress);
      CbmTrdModule *module = fDigiPar->GetModule(moduleAddress);
      if (!module) {
        LOG(WARNING) << "CbmTrdHitProducerCluster::TriangularPadReconstructionAB No CbmTrdModule found digiAddress="
        << digi->GetAddress() << " moduleAddress=" << CbmTrdAddress::GetModuleAddress(digi->GetAddress()) << FairLogger::endl;
        return;
      }
      if(module!=fModule){ 
        if(fModule) TriangularFinalizeModule();
        fModule = module;
        gGeoManager->FindNode(fModule->GetX(), fModule->GetY(), fModule->GetZ());
        //printf("current node [%s]\n", gGeoManager->GetPath());
      }
      nCol = fModule->GetNofColumns();
//       padW = moduleInfo->GetPadSizeX(secId);
//       padH = moduleInfo->GetPadSizeY(secId);
    }
    rowId = fModule->GetModuleRow(secId, rowId);
    if(rowOld>=0 && rowId!=rowOld) TriangularProcessRowCluster(digiMap, clusterId);
      
    
    Double_t  charge(digi->GetCharge()),
              chargeTR(digi->GetChargeTR());
    //printf("  AB:: r[%2i] c[%2i]%c t[%d] q[%7.4f / %7.4f]\n", rowId, colId, (triId==1?'u':'d'), Int_t(digi->GetTime()), charge, chargeTR);
    // pack cluster data for further processing
    combiId = setTriangleCombId(rowId, colId, nCol, 0);
    if(triId==1){
      digiMap[combiId-1].first+= charge; digiMap[combiId-1].second= UChar_t(digi->GetTime());    
      digiMap[combiId].first  += charge; digiMap[combiId].second  = UChar_t(digi->GetTime());    
    } else {
      digiMap[combiId].first  += charge; digiMap[combiId].second  = UChar_t(digi->GetTime());    
      digiMap[combiId+1].first+= charge; digiMap[combiId+1].second= UChar_t(digi->GetTime());          
    }
    totalCharge     += charge;
    totalChargeTR   += chargeTR;
    rowOld          =  rowId;
  }  
  TriangularProcessRowCluster(digiMap, clusterId);
}

//____________________________________________________________________________
void CbmTrdHitProducerCluster::TriangularFinalizeModule()
{
  Int_t idx(0),  jdx(0), address(-1);
  std::vector<Bool_t> mask(fModuleHits->size());
  std::fill(mask.begin(), mask.end(), kFALSE);
  std::vector<Bool_t>::iterator im(mask.begin());
  for(std::vector<CbmTrdHit*>::iterator it=fModuleHits->begin(); it!=fModuleHits->end(); it++, im++, idx++){
    if((*im)) continue;
    if(address<0){
      address = (*it)->GetAddress();
      //printf("TriangularFinalizeModule(%4d ly[%d] id[%d]) hits[%2d]\n", address, CbmTrdAddress::GetLayerId(address), CbmTrdAddress::GetModuleId(address), fModuleHits->size());
    }
    //printf("%2d x[%7.3f] y[%7.3f] z[%7.3f]\n", idx, (*it)->GetX(), (*it)->GetY(), (*it)->GetZ());
    jdx=0;
    std::vector<Bool_t>::iterator jm(mask.begin());
    std::vector<CbmTrdHit*>::iterator jt=fModuleHits->begin();
    for(; jt!=fModuleHits->end(); jt++, jm++, jdx++){
      if(jt==it) continue;
      if((*jm)) continue;
      //printf("  ck %2d x[%7.3f] y[%7.3f] z[%7.3f]\n", jdx, (*jt)->GetX(), (*jt)->GetY(), (*jt)->GetZ());
      if(TMath::Abs((*it)->GetX() - (*jt)->GetX()) > 0.2) continue; //TODO use error parametrization
      //if(TMath::Abs((*it)->GetY() - (*jt)->GetY()) > 3.*fModule->GetAnodeWireSpacing()) continue; //TODO use error parametrization
      if(TMath::Abs((*it)->GetTime() - (*jt)->GetTime()) > 50) continue; //TODO use error parametrization
      TriangularMergeHits((*it), (*jt));
      (*im)=kTRUE;(*jm)=kTRUE;
      break;
    }
    if(jt==fModuleHits->end()){  // register hit
      //printf("register hit %d\n", idx);
      TVector3 pos, pose;
      (*it)->Position(pos);(*it)->PositionError(pose);
      CbmTrdHit *hit = new ((*fHits)[fHits->GetEntriesFast()]) 
        CbmTrdHit((*it)->GetAddress(), 
                  pos, 
                  pose, 
                  (*it)->GetDxy(), 
                  (*it)->GetRefId(), 
                  (*it)->GetELossTR(), 
                  (*it)->GetELossdEdX(), 
                  (*it)->GetELoss());
      hit->SetTime((*it)->GetTime(), (*it)->GetTimeError());
      (*im)=kTRUE;
    }
  } 
  
  fModuleHits->clear();
}

//____________________________________________________________________________
void CbmTrdHitProducerCluster::TriangularMergeHits(CbmTrdHit *h0, CbmTrdHit *h1)
{
/**
 * The parameters of the merged hit are wheighted with the dEdx value of the two hits - should be elaborated. The merged hit is registered in the fHits array
 */

  //printf("TriangularMergeHits()\n");
  TVector3 pos0, pos0e;h0->Position(pos0); h0->PositionError(pos0e);
  TVector3 pos1, pos1e;h1->Position(pos1); h1->PositionError(pos1e);
  Double_t e0(h0->GetELossdEdX()), e1(h1->GetELossdEdX()), e(e0+e1);
  e0/=e; e1/=e;
  pos0*=e0; pos1*=e1; pos0+=pos1;
  CbmTrdHit *hit = new ((*fHits)[fHits->GetEntriesFast()]) 
        CbmTrdHit(h0->GetAddress(), 
                  pos0, 
                  pos0e, 
                  (e0>e1?h0->GetDxy():h1->GetDxy()), 
                  h0->GetRefId(), 
                  h0->GetELossTR()+h1->GetELossTR(), 
                  e, 
                  h0->GetELoss()+h1->GetELoss());
  hit->SetTime(e0*h0->GetTime()+e1*h1->GetTime());
}

//____________________________________________________________________________
void CbmTrdHitProducerCluster::TriangularProcessRowCluster(std::map<UInt_t, std::pair<Double_t, UChar_t>> &digiMap, Int_t cid)
{
  //printf("  TriangularProcessRowCluster [%d]...\n", fRecoTriangular);
  
  Int_t maxSec(-1), maxRow(-1), maxCol(-1), maxTyp(-1), maxId(-1), maxTime(-1),
    ly(CbmTrdAddress::GetLayerId(fModule->GetModuleAddress())), 
    mid(CbmTrdAddress::GetModuleId(fModule->GetModuleAddress()));
  const Float_t MINS(1100.);
  const Float_t MAXS(3800.);
  struct PAD {
    UChar_t  stat;    // status map 
                      // 7 6 5 4 3 2 1 0
                      //               1  mask pad for further usage
                      //             1    pad not connected
    UChar_t  time;    // no of clocks from prompt hit
    Double_t signal;  // signal per pad
    PAD(Double_t s=0., UChar_t t=0):stat(0),time(t),signal(s){;}
    PAD& operator=(const PAD &lh) { 
      stat  = lh.stat;
      time  = lh.time;
      signal= lh.signal; 
      return *this;
    }
  };
  Int_t t,r,c;
  // store pads in current cluster
  // index of pad is idx = 2 * (row * nCol + col+1) + typ; typ = 0 [rectangular], 1 [tilted]
  // pad is described by struct PAD see above
  std::map<UInt_t, PAD> padMap;
  for(std::map<UInt_t, std::pair<Double_t, UChar_t>>::iterator id=digiMap.begin(); id!=digiMap.end(); ++id){ 
    PAD pad(id->second.first, id->second.second);
    //getTriangleCombId(id->first, fModule->GetNofColumns(), r, c, t); printf("\t%d[%c] r[%2d] c[%2d] s[%7.4f] t[%d]\n", id->first, (t?'T':'R'), r, c, id->second.first, id->second.second);
    padMap[id->first] =  pad;
  }
  Int_t np(0);
  Double_t maxS = 0;
  while(kTRUE){
    np=0; maxS=-1;  
    for(std::map<UInt_t, PAD>::iterator id=padMap.begin(); id!=padMap.end(); ++id){
      if(id->second.stat&1) continue;
      if (id->second.signal < maxS) continue;
      getTriangleCombId(id->first, fModule->GetNofColumns(), r, c, t);
      if(maxRow<0) maxSec=fModule->GetSectorRow(r, maxRow);
      maxS = id->second.signal;
      maxTime= id->second.time;
      maxTyp = t;
      maxCol = c;
      maxId  = id->first;
    }
    if(maxS<0) break;
    //printf("  MAX s[%d] r[%2d | %2d] c[%2d] q[%7.4f]\n", maxSec, maxRow, r, maxCol, maxS);

    np=1;
    std::map<UInt_t, PAD>::iterator ip=padMap.find(maxId), mp=ip, lp=ip, rp=ip;
    ip->second.stat|=1; // mask pad
    Double_t  lastS(ip->second.signal);
    UChar_t   lastT(ip->second.time), maxT(lastT);
    //printf("  mxPad[%4d] : mask[%c] q[%f] time[%d]\n", maxId, (ip->second.stat&1)?'y':'n', ip->second.signal, ip->second.time);

    // look to the left    
    for(--ip; kTRUE; --ip){
      //printf("  idPad[%4d] : mask[%c] q[%f] time[%d]\n", ip->first, (ip->second.stat&1)?'y':'n', ip->second.signal, ip->second.time);
      if(ip->second.stat&1) break;// detect other cluster
      Double_t sgn(ip->second.signal); UChar_t tt(ip->second.time);
      if( (sgn<MAXS && lastS<MAXS) && // detect hit stop only for signals which are not in saturation
        ((sgn>lastS) ||                         // detect increase in signal
        (sgn>MINS && TMath::Abs(lastT-tt)>1) || // detect time difference between adjacent channels larger than 2 clocks
        (sgn>MINS && TMath::Abs(maxT-tt)>3) || // detect larger than 3 clocks difference between prompt and tail signals
        (sgn>1500 && tt-maxT>2))){             // detect larger than 2 clocks difference between prompt and LARGE neighbour signals
        //printf("Detect TIME stop condition LEFT for clusters for S[%7.2f] T[%d] @ r[%d] c[%d] in c[%d] S[%7.2f] T[%d]\n", max, jtime, irow, jpadMax, p0, sgn, tt);
        break;      
      }
      ip->second.stat|=1; // mask pad
      lastS   = sgn;
      lastT   = tt;
      lp=ip;  // mark limit iterator
      np++;
      if(ip==padMap.begin()) break;
    }

    //printf("====================\n");  
    // look to the right
    lastS = maxS; lastT=maxT; mp++;
    for(ip=mp; ip!=padMap.end(); ip++){
      //printf("  idPad[%4d] : mask[%c] q[%f] time[%d]\n", ip->first, (ip->second.stat&1)?'y':'n', ip->second.signal, ip->second.time);
      if(ip->second.stat&1) break;// detect other cluster
      Double_t sgn(ip->second.signal); UChar_t tt(ip->second.time);
      if( (sgn<MAXS && lastS<MAXS) && // detect hit stop only for signals which are not in saturation
        ((sgn>lastS) ||                         // detect increase in signal
        (sgn>MINS && TMath::Abs(lastT-tt)>1) || // detect time difference between adjacent channels larger than 2 clocks
        (sgn>MINS && TMath::Abs(maxT-tt)>3) || // detect larger than 3 clocks difference between prompt and tail signals
        (sgn>1500 && tt-maxT>2))){             // detect larger than 2 clocks difference between prompt and LARGE neighbour signals
        //printf("Detect TIME stop condition LEFT for clusters for S[%7.2f] T[%d] @ r[%d] c[%d] in c[%d] S[%7.2f] T[%d]\n", max, jtime, irow, jpadMax, p0, sgn, tt);
        break;      
      }
      ip->second.stat|=1; // mask pad
      lastS   = sgn;
      lastT   = tt;
      rp=ip;  // mark limit iterator
      np++;
    }
  
    //printf("======FINAL========\n");  
    Double_t sgns[100]={0.}; Char_t toff[100]={0}; Int_t idx(0), imax(0);
    for(ip=lp; 1; ip++){
      if(ip==lp && ip->first/2) idx++; // add an empty entry if first pad is tilted
      sgns[idx] = ip->second.signal; 
      toff[idx] = ip->second.time-maxT;
      if(ip==mp) imax=idx;
      idx++;
      //printf("  pad[%4d] r[%2d] c[%2d] t[%d] : mask[%c] q[%f] time[%d]\n", ip->first, r, c, t, (ip->second.stat&1)?'y':'n', ip->second.signal, ip->second.time);
      if(ip==rp) break;
    }  
    
    // compute cluster energy and position
    // printf("  n[%2d] max[%d]\n", idx, imax);
    Double_t  x = TriangularGetX(idx, sgns, imax);
    //printf("  --> x0[%7.4f] ", x);
    Double_t y(x);
    Int_t   an = TriangularGetY(idx, sgns, y, toff);   
    x = TriangularGetX(idx, sgns, imax, an);
    Double_t edep = TriangularGetEdep(idx, sgns), etr(0.);
    //printf("x[%7.4f] y[%7.4f] an[%2d] edep[%7.4f]\n", x, y, an, edep);
    
    // get row/col position
    TVector3 padPos, padSize;
    fModule->GetPadPosition(CbmTrdAddress::GetAddress(ly, mid, maxSec, maxRow, maxCol), padPos, padSize);
    
    TVector3 loc_xyz; 
    loc_xyz.SetXYZ(padPos[0]+x,                                 // offset from column center 
                   padPos[1]+an*fModule->GetAnodeWireSpacing(), // offset from row center  
                   0.5*fModule->GetSizeZ()-fModule->GetAnodeWireToPadPlaneDistance()); // anode plane position
    // check and register hit 
    if (fabs(loc_xyz[0]) > fModule->GetSizeX() || fabs(loc_xyz[1]) > fModule->GetSizeY()){
      LOG(WARNING) << "CbmTrdHitProducerCluster::TriangularPadReconstructionAB : Hit outside module : x[" << 
      fabs(loc_xyz[0])<<" / "<< fModule->GetSizeX()<<"] y["<<
      fabs(loc_xyz[1])<<" / "<< fModule->GetSizeY()<<"]"<< FairLogger::endl;
      continue;
    }
  
    Double_t loc_hit[3], glb_hit[3];
    loc_xyz.GetXYZ(loc_hit);
    gGeoManager->LocalToMaster(loc_hit, glb_hit);
    TVector3 glb_xyz; glb_xyz.SetXYZ(glb_hit[0], glb_hit[1], glb_hit[2]); 

    // error parametrization
    TVector3 exyz; exyz.SetXYZ(y, 0.1, 0.1);
    Double_t covxy(0.);
    if(!fModuleHits) fModuleHits = new std::vector<CbmTrdHit*>;
    CbmTrdHit *hit = new CbmTrdHit(fModule->GetModuleAddress(), glb_xyz, exyz, covxy, cid, etr, edep, edep+etr);
    hit->SetTime(maxTime);
    fModuleHits->push_back(hit);

    //printf("\tUpdate reco[%d]\n", fRecoTriangular);
    fRecoTriangular++;
  }
  digiMap.clear();
}

//____________________________________________________________________________
Double_t CbmTrdHitProducerCluster::TriangularGetX(Int_t n, Double_t *sgn, Int_t max, Int_t an)
{
/**
 *  Fill signal distribution and fit it with PRF model (Gauss). The tilt pads are assigned the x0 position of the column boundary (col+0.5*pw). In case of anode wire information a further correction is applied:\n
 * x0 = col+0.5+an*dw/h\n
 * with dw the anode wire pitch and h the pad row height
 */
  if(!fTriangularPRF){ 
    fTriangularPRF = new TF1("TriangularPRF", "gaus", -3.5, 3.5);
    fTriangularSignal = new TGraphErrors();
  }
  fTriangularPRF->SetParameter(0, sgn[max]); fTriangularPRF->SetParLimits(0, 0.5*sgn[max], 2.5*sgn[max]);
  fTriangularPRF->SetParameter(1, 0.);       fTriangularPRF->SetParLimits(1, -0.75, 0.95);
  fTriangularPRF->SetParameter(2, 0.5);      fTriangularPRF->SetParLimits(2, 0.4, 0.7);
 
  Int_t jp(0), jpad(-(max>>1));
  if(max%2==0) jpad++; // take care of the case when the maximum is on a tilted pad
  Double_t xmin(jpad-0.5), xmax(0.);
  if(sgn[0]>0.){
    //printf("    %2d x[%5.2f] s[%f]\n", jp, xmin, 0.);
    fTriangularSignal->SetPoint(jp++, xmin, 0.);
  } else xmin+=0.5;
  for(Int_t ip(0); ip<n; ip++){
    if(ip%2){ // add tilted pads
      xmax = jpad+0.5;
      if(an>=-4) xmax+=an*0.3/2.7;//dw/h;
      fTriangularSignal->SetPoint(jp, xmax, sgn[ip]);
      fTriangularSignal->SetPointError(jp, 0., 0.001/*sgne[ip]*/);       
      jpad++;
    } else {  // add rectangular pads
      xmax = jpad;
      fTriangularSignal->SetPoint(jp, xmax, sgn[ip]);
      fTriangularSignal->SetPointError(jp, 0., 0.001/*sgne[ip]*/); 
    }
    //printf("    %2d x[%5.2f] s[%f]\n", jp, xmax, sgn[ip]);
    jp++;
  }
  xmax+=0.5;
  //printf("    %2d x[%5.2f] s[%f]\n", jp, xmax, 0.);
  fTriangularSignal->SetPoint(jp++, xmax, 0);
  //fTriangularSignal->Print();
  fTriangularSignal->Fit(fTriangularPRF, "QRB", "", xmin-0.1, xmax+0.1);
  return fTriangularPRF->GetParameter(1);
}

//____________________________________________________________________________
Double_t CbmTrdHitProducerCluster::TriangularGetY(Int_t n, Double_t *sgn, Double_t& x, Char_t */*dt*/) const
{
  Int_t idx(0);
  Double_t q0(0.), q1(0.), qt(0.);
  do{
    q0+=qt;
    qt=sgn[idx++]-qt; //printf("    qq[%2d] qt[%f] -> 1\n", idx-1, qt);
    q1+=qt;
    qt=sgn[idx++]-qt;  //printf("    qq[%2d] qt[%f] -> 0\n", idx-1, qt);
  } while (idx<=n);
  q0+=qt;
  x = (q0-q1)/(q0+q1);
  Int_t an(TMath::Abs(x)/x);
  if(TMath::Abs(x)<0.1) an=0;
  else if(TMath::Abs(x)<0.35) ;
  else if(TMath::Abs(x)<0.52) an*=2;
  else if(TMath::Abs(x)<0.7)  an*=3;
  else an*=4;

  return an;
}

//____________________________________________________________________________
Double_t CbmTrdHitProducerCluster::TriangularGetEdep(Int_t n, Double_t *sgn) const
{
  Double_t  eR(0.),  // energy reconstructed on rectangular pads
            eT(0.);  // energy reconstructed on tilted pads
  for(Int_t ip(0); ip<n; ip++){
    if(ip%2) eT+=sgn[ip];
    else eR+=sgn[ip];
  }
  //printf("    TriangularGetEdep(R[%7.2f] T[%7.2f])\n", eR, eT);
  return TMath::Max(eR, eT);
}

void CbmTrdHitProducerCluster::TriangularPadReconstruction(Int_t clusterId){
  const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*>(fClusters->At(clusterId));
 
  TVector3 hit_posV;
  TVector3 local_pad_posV;
  TVector3 local_pad_dposV;
  Double_t totalCharge = 0;
  Double_t totalChargeTR = 0;
  Int_t moduleAddress = 0;
  Int_t nofDigis = cluster->GetNofDigis();
//  Int_t digiMaxId(-1), digiMaxCombiId(-1), maxRow(-1), maxCol(-1), maxDigiAddress(-1);
  Int_t digiMaxCombiId(-1), maxRow(-1), maxCol(-1), maxDigiAddress(-1);
  Double_t maxCharge = 0;
  Int_t digiId = -1;
  Int_t combiId(-1), secId(-1), colId(-1), rowId(-1), nCol(0);
  Int_t digiAddress = -1;
  Double_t padH(0.), padW(0.);
  //std::list<std::pair<Int_t, Int_t> > digiList; // combiIds = iRow * (nCol +1) + iCol, DigiId
  std::map<Int_t, Double_t> digiMap; // combiId, DigiCharge
  CbmTrdModule *moduleInfo = NULL;
  for (Int_t iDigi = 0; iDigi < nofDigis; iDigi++) {
    digiId = cluster->GetDigi(iDigi);
    const CbmTrdDigi* digi = static_cast<const CbmTrdDigi*>(fDigis->At(digiId));
    digiAddress = digi->GetAddress();
    secId = CbmTrdAddress::GetSectorId(digiAddress);
    colId = CbmTrdAddress::GetColumnId(digiAddress);
    rowId = CbmTrdAddress::GetRowId(digiAddress);
    moduleInfo = fDigiPar->GetModule(CbmTrdAddress::GetModuleAddress(digiAddress));
    if (moduleInfo == NULL) {
      LOG(WARNING) << "CbmTrdHitProducerCluster::TriangularPadReconstruction No CbmTrdModule found digiAddress="
		   << digi->GetAddress() << " moduleAddress=" << CbmTrdAddress::GetModuleAddress(digi->GetAddress()) << FairLogger::endl;
      return;
    }
    rowId = moduleInfo->GetModuleRow(secId, rowId);
    nCol = moduleInfo->GetNofColumns();
    combiId = rowId * (nCol + 1) + colId;
    printf("r:%4i(r:%4i)  c:%4i   ->  %4i\n", rowId, CbmTrdAddress::GetRowId(digiAddress), colId, combiId);
    //digiList.push_back(make_pair(combiId,digiId));  
    moduleAddress = CbmTrdAddress::GetModuleAddress(digi->GetAddress());
    Double_t charge = digi->GetCharge();
    Double_t chargeTR = digi->GetChargeTR();
    digiMap[combiId] = charge;
    totalCharge += charge;
    totalChargeTR += chargeTR;
    if (charge > maxCharge){
      maxCharge = charge;
      maxRow = rowId;
      maxCol = colId;
      digiMaxCombiId = combiId;
      maxDigiAddress = digiAddress;
      padW = moduleInfo->GetPadSizeX(secId);
      padH = moduleInfo->GetPadSizeY(secId);
//      digiMaxId = cluster->GetDigi(iDigi);
    }
  }
  //digiList.sort(combiIdSorter);
  //std::map<Int_t, Int_t>::iterator it;
  moduleAddress = CbmTrdAddress::GetModuleAddress(maxDigiAddress);
  moduleInfo = fDigiPar->GetModule(moduleAddress);
  gGeoManager->FindNode(moduleInfo->GetX(), moduleInfo->GetY(), moduleInfo->GetZ());
  moduleInfo->GetPadPosition(maxDigiAddress, local_pad_posV, local_pad_dposV);

  if (maxRow % 2 == 0){
    if (digiMap.find(maxRow * (nCol + 1) + (maxCol - 1))     == digiMap.end() ) 
      printf("          left same row:            (%4i,%4i) -> %4i\n",maxRow,maxCol-1,maxRow * (nCol + 1) + (maxCol - 1));
    if (digiMap.find(maxRow * (nCol + 1) + (maxCol + 1))     == digiMap.end() ) 
      printf("          right same row:           (%4i,%4i) -> %4i\n",maxRow,maxCol+1,maxRow * (nCol + 1) + (maxCol + 1));
    if (digiMap.find((maxRow+1) * (nCol + 1) + (maxCol - 1)) == digiMap.end() ) 
      printf("          left upper row:           (%4i,%4i) -> %4i\n",(maxRow+1),maxCol - 1,(maxRow+1) * (nCol + 1) + (maxCol - 1));
    if (digiMap.find((maxRow+1) * (nCol + 1) + (maxCol))     == digiMap.end() ) 
      printf("          center upper row:         (%4i,%4i) -> %4i\n",(maxRow+1),maxCol,(maxRow+1) * (nCol + 1) + (maxCol));
    if (digiMap.find((maxRow+1) * (nCol + 1) + (maxCol + 1)) == digiMap.end() ) 
      printf("          right upper row:          (%4i,%4i) -> %4i\n",(maxRow+1),maxCol + 1,(maxRow+1) * (nCol + 1) + (maxCol + 1));
    if (digiMap.find((maxRow+1) * (nCol + 1) + (maxCol + 2)) == digiMap.end() ) 
      printf("          right of right upper row: (%4i,%4i) -> %4i\n",(maxRow+1),maxCol + 2,(maxRow+1) * (nCol + 1) + (maxCol + 2));
    if (digiMap.find(maxRow * (nCol + 1) + (maxCol - 1))     == digiMap.end() || // left same row
	digiMap.find(maxRow * (nCol + 1) + (maxCol + 1))     == digiMap.end() || // right same row
	digiMap.find((maxRow+1) * (nCol + 1) + (maxCol - 1)) == digiMap.end() || // left upper row
	digiMap.find((maxRow+1) * (nCol + 1) + (maxCol))     == digiMap.end() || // center upper row
	digiMap.find((maxRow+1) * (nCol + 1) + (maxCol + 1)) == digiMap.end() || // right upper row 
	digiMap.find((maxRow+1) * (nCol + 1) + (maxCol + 2)) == digiMap.end() ){ // right of right upper row
      CenterOfGravity(clusterId);
      LOG(WARNING) << "CbmTrdHitProducerCluster::TriangularPadReconstruction Row:" << maxRow << " Col:" << maxCol << " max:(" << moduleInfo->GetNofRows() << "," << nCol << ")" << FairLogger::endl;
      return;
    } else {
      fRecoTriangular++;
      //printf("parallel padW:%f rect padW:%f\n",padW * TMath::Cos(TMath::ATan(padW / padH)),padW);
      Double_t d_rec = CalcDisplacement(digiMap[digiMaxCombiId - 1] + digiMap[maxRow + 1 * (nCol + 1) + (maxCol - 1)], // left rectagular
					digiMap[digiMaxCombiId]     + digiMap[maxRow + 1 * (nCol + 1) + (maxCol)],     // central rectagular
					digiMap[digiMaxCombiId + 1] + digiMap[maxRow + 1 * (nCol + 1) + (maxCol + 1)], // right rectangular
					padW);
      Double_t d_par = CalcDisplacement(digiMap[digiMaxCombiId - 1] + digiMap[maxRow + 1 * (nCol + 1) + (maxCol)],     // left parallelogram
					digiMap[digiMaxCombiId]     + digiMap[maxRow + 1 * (nCol + 1) + (maxCol + 1)], // central parallelogram
					digiMap[digiMaxCombiId + 1] + digiMap[maxRow + 1 * (nCol + 1) + (maxCol + 2)], // right parallelogram
					padW * TMath::Cos(TMath::ATan(padW / padH)));
      Double_t dy = CalcY(padW, padH, d_rec, d_par, maxRow);
      local_pad_posV[0] += d_rec;
      local_pad_posV[1] += 0.5 * padH + dy;
      //printf("d_rec:%f  d_par:%f  dy:%f \n", d_rec, d_par, dy);
    }
  } else {
    if (digiMap.find(maxRow * (nCol + 1) + (maxCol - 1))     == digiMap.end() ) 
      printf("          left same row:          (%4i,%4i) -> %4i\n",maxRow,maxCol-1,maxRow * (nCol + 1) + (maxCol - 1));
    if (digiMap.find(maxRow * (nCol + 1) + (maxCol + 1))     == digiMap.end() ) 
      printf("          right same row:         (%4i,%4i) -> %4i\n",maxRow,maxCol+1,maxRow * (nCol + 1) + (maxCol + 1));
    if (digiMap.find((maxRow-1) * (nCol + 1) + (maxCol - 2)) == digiMap.end() ) 
      printf("          left of left lower row: (%4i,%4i) -> %4i\n",maxRow-1,maxCol-2,(maxRow-1) * (nCol + 1) + (maxCol - 2));
    if (digiMap.find((maxRow-1) * (nCol + 1) + (maxCol - 1)) == digiMap.end() ) 
      printf("          left lower row:         (%4i,%4i) -> %4i\n",maxRow-1,maxCol-1,(maxRow-1) * (nCol + 1) + (maxCol - 1));
    if (digiMap.find((maxRow-1) * (nCol + 1) + (maxCol))     == digiMap.end() ) 
      printf("          center lower row:       (%4i,%4i) -> %4i\n",maxRow-1,maxCol,(maxRow-1) * (nCol + 1) + (maxCol));
    if (digiMap.find((maxRow-1) * (nCol + 1) + (maxCol + 1)) == digiMap.end() ) 
      printf("          right lower row:        (%4i,%4i) -> %4i\n",maxRow-1,maxCol+1,(maxRow-1) * (nCol + 1) + (maxCol + 1));
    if (digiMap.find(digiMaxCombiId - 1)                     == digiMap.end() || // left same row
	digiMap.find(digiMaxCombiId + 1)                     == digiMap.end() || // right same row
	digiMap.find((maxRow-1) * (nCol + 1) + (maxCol - 2)) == digiMap.end() || // left of left lower row
	digiMap.find((maxRow-1) * (nCol + 1) + (maxCol - 1)) == digiMap.end() || // left lower row
	digiMap.find((maxRow-1) * (nCol + 1) + (maxCol))     == digiMap.end() || // center lower row
	digiMap.find((maxRow-1) * (nCol + 1) + (maxCol + 1)) == digiMap.end() ){ // right lower row
      CenterOfGravity(clusterId);
      LOG(WARNING) << "CbmTrdHitProducerCluster::TriangularPadReconstruction Row:" << maxRow << " Col:" << maxCol <<  " max:(" << moduleInfo->GetNofRows() << "," << nCol << ")" << FairLogger::endl;
      return;
    } else {
      fRecoTriangular++;
      //printf("parallel padW:%f rect padW:%f\n",padW * TMath::Cos(TMath::ATan(padW / padH)), padW);
      Double_t d_rec = CalcDisplacement(digiMap[digiMaxCombiId - 1] + digiMap[maxRow - 1 * (nCol + 1) + (maxCol - 1)],// left rectagular
					digiMap[digiMaxCombiId]     + digiMap[maxRow - 1 * (nCol + 1) + (maxCol)],    // central rectagular
					digiMap[digiMaxCombiId + 1] + digiMap[maxRow - 1 * (nCol + 1) + (maxCol + 1)],// right rectangular
					padW);
      Double_t d_par = CalcDisplacement(digiMap[digiMaxCombiId - 1] + digiMap[maxRow - 1 * (nCol + 1) + (maxCol - 2)],// left parallelogram
					digiMap[digiMaxCombiId]     + digiMap[maxRow - 1 * (nCol + 1) + (maxCol - 1)],// central parallelogram
					digiMap[digiMaxCombiId + 1] + digiMap[maxRow - 1 * (nCol + 1) + (maxCol + 1)],// right parallelogram
					padW * TMath::Cos(TMath::ATan(padW / padH)));
      Double_t dy = CalcY(padW, padH, d_rec, d_par, maxRow);
      local_pad_posV[0] += d_rec;
      local_pad_posV[1] += -0.5 * padH + dy;
      //printf("d_rec:%f  d_par:%f  dy:%f \n", d_rec, d_par, dy);
    }
  }
  digiMap.clear();

  if (fabs(local_pad_posV[0]) > moduleInfo->GetSizeX() || fabs(local_pad_posV[1]) > moduleInfo->GetSizeY()) return;
  Double_t hit_pos[3];
  for (Int_t iDim = 0; iDim < 3; iDim++) {
    hit_posV[iDim] = local_pad_posV[iDim];
    //hit_posV[iDim] /= totalCharge;
    hit_pos[iDim] = hit_posV[iDim];
  }
  Double_t global_hit[3];
  gGeoManager->LocalToMaster(hit_pos, global_hit);

  for (Int_t iDim = 0; iDim < 3; iDim++){
    hit_posV[iDim] = global_hit[iDim];
  }
 
  Int_t nofHits = fHits->GetEntriesFast();
  new ((*fHits)[nofHits]) CbmTrdHit(moduleAddress, hit_posV, local_pad_dposV, 0, clusterId, 
				    totalChargeTR, totalCharge-totalChargeTR, totalCharge);
}


void CbmTrdHitProducerCluster::CenterOfGravity(Int_t clusterId)
{
  const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*>(fClusters->At(clusterId));

  TVector3 hit_posV;
  TVector3 local_pad_posV;
  TVector3 local_pad_dposV;
  for (Int_t iDim = 0; iDim < 3; iDim++) {
    hit_posV[iDim] = 0.0;
    local_pad_posV[iDim] = 0.0;
    local_pad_dposV[iDim] = 0.0;
  }
  Double_t xVar = 0;
  Double_t yVar = 0;
  Double_t totalCharge = 0;
  Double_t totalChargeTR = 0;
  Int_t moduleAddress = 0;
  Int_t nofDigis = cluster->GetNofDigis();
  for (Int_t iDigi = 0; iDigi < nofDigis; iDigi++) {
    const CbmTrdDigi* digi = static_cast<const CbmTrdDigi*>(fDigis->At(cluster->GetDigi(iDigi)));
    
    Double_t digiCharge = digi->GetCharge();
    
    if (digiCharge <= 0)
       continue;

    moduleAddress = CbmTrdAddress::GetModuleAddress(digi->GetAddress());
    CbmTrdModule* moduleInfo = fDigiPar->GetModule(moduleAddress);
    gGeoManager->FindNode(moduleInfo->GetX(), moduleInfo->GetY(), moduleInfo->GetZ());

    if (moduleInfo == NULL) {
      LOG(WARNING) << "CbmTrdHitProducerCluster::CenterOfCharge No CbmTrdModule found digiAddress="
		   << digi->GetAddress() << " moduleAddress=" << CbmTrdAddress::GetModuleAddress(digi->GetAddress()) << FairLogger::endl;
      return;
    }
    totalCharge += digi->GetCharge();
    totalChargeTR += digi->GetChargeTR();
    //printf("DigiAddress:%i ModuleAddress:%i\n",digi->GetAddress(), CbmTrdAddress::GetModuleAddress(digi->GetAddress()));
    moduleInfo->GetPadPosition(digi->GetAddress(), local_pad_posV, local_pad_dposV);//local_pad_pos[0], local_pad_pos[1], local_pad_pos[2]);
    if (fTrianglePads){
      Double_t local_pad_pos[3];
      for (Int_t i = 0; i < 3; i++) {
	local_pad_pos[i] = local_pad_posV[i];
      }
      Int_t secId(-1), colId(-1), rowId(-1);
      moduleInfo->GetPadInfo(local_pad_pos, secId, colId, rowId);
      if (rowId%2 == 0) { // Calculate center of gravity for triangular pads 
	local_pad_posV[0] = 1./3. * ((local_pad_pos[0] - 0.5 * moduleInfo->GetPadSizeX(secId)) + 
				     (local_pad_pos[0] + 0.5 * moduleInfo->GetPadSizeX(secId)) + 
				     (local_pad_pos[0] + 0.5 * moduleInfo->GetPadSizeX(secId)));
	local_pad_posV[1] = 1./3. * ((local_pad_pos[1] - 0.5 * moduleInfo->GetPadSizeX(secId)) + 
				     (local_pad_pos[1] - 0.5 * moduleInfo->GetPadSizeX(secId)) + 
				     (local_pad_pos[1] + 0.5 * moduleInfo->GetPadSizeX(moduleInfo->GetSector(rowId+1, secId))));
      } else {
	local_pad_posV[0] = 1./3. * ((local_pad_pos[0] - 0.5 * moduleInfo->GetPadSizeX(secId)) + 
				     (local_pad_pos[0] + 0.5 * moduleInfo->GetPadSizeX(secId)) + 
				     (local_pad_pos[0] - 0.5 * moduleInfo->GetPadSizeX(secId)));
	local_pad_posV[1] = 1./3. * ((local_pad_pos[1] + 0.5 * moduleInfo->GetPadSizeX(secId)) + 
				     (local_pad_pos[1] + 0.5 * moduleInfo->GetPadSizeX(secId)) + 
				     (local_pad_pos[1] - 0.5 * moduleInfo->GetPadSizeX(moduleInfo->GetSector(rowId-1, secId))));
      }
    }
    
    Double_t xMin = local_pad_posV[0] - local_pad_dposV[0];
    Double_t xMax = local_pad_posV[0] + local_pad_dposV[0];
    xVar += (xMax * xMax + xMax * xMin + xMin * xMin) * digiCharge;
    
    Double_t yMin = local_pad_posV[1] - local_pad_dposV[1];
    Double_t yMax = local_pad_posV[1] + local_pad_dposV[1];
    yVar += (yMax * yMax + yMax * yMin + yMin * yMin) * digiCharge;
    
    //moduleInfo->TransformHitError(local_pad_dposV);

    for (Int_t iDim = 0; iDim < 3; iDim++) {
       hit_posV[iDim] += local_pad_posV[iDim] * digiCharge;
    }
    
    
  }
  
  if (totalCharge <= 0)
     return;
  
  Double_t hit_pos[3];
  for (Int_t iDim = 0; iDim < 3; iDim++) {
    hit_posV[iDim] /= totalCharge;
    hit_pos[iDim] = hit_posV[iDim];
  }
  
  xVar /= totalCharge;
  xVar /= 3;
  xVar -= hit_pos[0] * hit_pos[0];  
  yVar /= totalCharge;
  yVar /= 3;
  yVar -= hit_pos[1] * hit_pos[1];
  
  Double_t global_hit[3];
  gGeoManager->LocalToMaster(hit_pos, global_hit);

  for (Int_t iDim = 0; iDim < 3; iDim++){
    hit_posV[iDim] = global_hit[iDim];
  }
  
  TVector3 cluster_pad_dposV(sqrt(xVar), sqrt(yVar), 0);
  CbmTrdModule* moduleInfo = fDigiPar->GetModule(moduleAddress);
  moduleInfo->TransformHitError(cluster_pad_dposV);

  Int_t nofHits = fHits->GetEntriesFast();
  new ((*fHits)[nofHits]) CbmTrdHit(moduleAddress, hit_posV, cluster_pad_dposV, 0, clusterId,
				    totalChargeTR, totalCharge-totalChargeTR, totalCharge);
}
    ClassImp(CbmTrdHitProducerCluster)

#include "CbmTrdModuleRecR.h"

#include "CbmTrdClusterFinder.h"
#include "CbmTrdAddress.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParSetDigi.h"
#include "CbmTrdHit.h"
#include "CbmTrdDigi.h"
#include "CbmTrdCluster.h"

#include <TClonesArray.h>
#include <TVector3.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TImage.h>
#include "TGeoMatrix.h"

#include <FairLogger.h>

using std::cout;
using std::endl;
using std::pair;
using std::map;
using std::vector;
using std::deque;
using std::make_pair;
using std::tuple;
using std::make_tuple;
using std::get;
using std::list;


//_______________________________________________________________________________
CbmTrdModuleRecR::CbmTrdModuleRecR()
  : CbmTrdModuleRec()
  ,fDigiCounter(0)
  ,fDigiMap()
  ,fClusterMap()
{
  SetNameTitle("TrdModuleRecR", "Reconstructor for rectangular pad TRD module");    
}
  
//_______________________________________________________________________________
CbmTrdModuleRecR::CbmTrdModuleRecR(Int_t mod, Int_t ly, Int_t rot)
  : CbmTrdModuleRec(mod, ly, rot)
  ,fDigiCounter(0)
  ,fDigiMap()
  ,fClusterMap()
{
  SetNameTitle(Form("TrdModuleRecR%02d", mod), "Reconstructor for rectangular pad TRD module");
}
    
//_______________________________________________________________________________
CbmTrdModuleRecR::~CbmTrdModuleRecR()
{
}
      
//_______________________________________________________________________________
Bool_t CbmTrdModuleRecR::AddDigi(CbmTrdDigi *digi, Int_t id)
{


  //fill the digimap
  fDigiMap.push_back(make_tuple(id, false, digi));
  fDigiCounter++;
  return kTRUE;
}

//_______________________________________________________________________________
void CbmTrdModuleRecR::Clear(Option_t *opt)
{
  if(strcmp(opt, "cls")==0){
    fDigiMap.erase(fDigiMap.begin(),fDigiMap.end());
    fClusterMap.erase(fClusterMap.begin(),fClusterMap.end());
    fDigiCounter = 0;
  }
  CbmTrdModuleRec::Clear(opt);
}

//_______________________________________________________________________________
Int_t CbmTrdModuleRecR::FindClusters()
{
  
  deque<tuple<Int_t, Bool_t, CbmTrdDigi*>>::                   iterator mainit; //subiterator for the deques in each module; searches for main-trigger to then add the neighbors
  deque<tuple<Int_t, Bool_t, CbmTrdDigi*>>::                   iterator FNit; //last iterator to find the FN digis which correspond to the main trigger or the adjacent main triggers
  deque<tuple<Int_t, Bool_t, CbmTrdDigi*>>::                   iterator start; //marker to erase already processed entries from the map to reduce the complexity of the algorithm
  deque<tuple<Int_t, Bool_t, CbmTrdDigi*>>::                   iterator stop; //marker to erase already processed entries from the map to reduce the complexity of the algorithm

  //reset time information; used to erase processed digis from the map
  Double_t time=0;
  Double_t lasttime=0;
  Double_t timediff=-1000;

  Int_t Clustercount=0;
  Double_t interval=CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC);
  Bool_t print=false;
  
  // iterator for the main trigger; searches for an unprocessed main triggered digi and then starts a subloop to directly construct the cluster
  //  while(!fDigiMap.empty()){
  //  std::cout<<fDigiMap.size()<<std::endl;
  if(print){
    std::cout<<fDigiMap.size()<<std::endl;
    for (mainit=fDigiMap.begin() ; mainit != fDigiMap.end(); mainit++) {
      CbmTrdDigi *digi =       (CbmTrdDigi*) get<2>(*mainit);
      Double_t ptime =                   digi->GetTime();
      Int_t digiAddress  =     digi->GetAddress();
      Float_t Charge =         digi->GetCharge(); 
      Int_t digiId =           get<0>(*mainit);
      Int_t channel =          digi->GetAddressChannel();
      Int_t ncols=             fDigiPar->GetNofColumns();
      Int_t triggerId =        digi->GetTriggerType();
    
      std::cout<<" module: " << fModAddress<<"   time: " << ptime<<"   charge: " << Charge<<"   col: " << channel % ncols<<"   row: " << channel / ncols<<"   trigger: " << triggerId<<"  ncols: " << ncols<<std::endl;
    }
  }

  start = fDigiMap.begin();
  for (mainit=fDigiMap.begin() ; mainit != fDigiMap.end(); mainit++) {
      
      //block to erase processed entries
      CbmTrdDigi *digi =       (CbmTrdDigi*) get<2>(*mainit);

      // --- VF: Had to introduce this condition to make the code running. Problem is that
      // --- the container is modified (by erase) within the loop. This also changes
      // --- fDigiMap.end(). A crash happens when after erasing, a single digi remains
      // --- in fDigiMap. Mainit then points at the next element, which is not defined.
      // --- The break condition mainit != fDigiMap.end() seems not to be fulfilled in that case.
      // --- I honestly do not know what happens in all other cases when erase is used.
      // --- The code surely needs being revised.
      if ( ! digi ) break;

      time =                   digi->GetTime();
      if(lasttime > 0)         timediff=time-lasttime;
      lasttime=                time;
      //      if(timediff < -interval)      start=mainit;
      if(timediff > interval && lasttime > 0)       {start=mainit;}
      //if(timediff > interval)       {start=mainit;stop=mainit;break;}
      if(timediff > interval)       {fDigiMap.erase(fDigiMap.begin(),stop+1);start=mainit;stop=mainit;}
      if(timediff < interval)       stop=mainit;



      Int_t triggerId =        digi->GetTriggerType();
      Bool_t marked    =       get<1>(*mainit);
      if(triggerId != CbmTrdDigi::kSelf || marked)       continue;
      

      //variety of neccessary address information; uses the "combiId" for the comparison of digi positions
      Int_t digiAddress  =     digi->GetAddress();
      Float_t Charge =         digi->GetCharge(); 
      Int_t digiId =           get<0>(*mainit);
      Int_t channel =          digi->GetAddressChannel();
      Int_t ncols=             fDigiPar->GetNofColumns();

      //some logic information which is used to process and find the clusters
      Int_t lowcol =           channel;
      Int_t highcol =          channel;
      Int_t lowrow =           channel;
      Int_t highrow =          channel;
      
      //counter which is used to easily break clusters which are at the edge and therefore do not fullfill the classical look
      Int_t dmain=1;

      //information buffer to handle neighbor rows and cluster over two rows; the identification of adjacent rows is done by comparing their center of gravity
      Int_t counterrow=1;
      Int_t countertop=0;
      Int_t counterbot=0;
      Double_t buffertop[3]={0,0,0};
      Double_t bufferbot[3]={0,0,0};
      Double_t bufferrow[3]={Charge,0,0};
      // vector<Double_t> buffertop;
      // vector<Double_t> bufferbot;
      // vector<Double_t> bufferrow;
      Double_t CoGtop;
      Double_t CoGbot;
      Double_t CoGrow; 
      tuple<CbmTrdDigi*,CbmTrdDigi*,CbmTrdDigi*> topdigi;// used to store the necassary digis for the CoG calculation without the need to revisit those digis
      tuple<CbmTrdDigi*,CbmTrdDigi*,CbmTrdDigi*> botdigi;

      // //some logical flags to reject unnecessary steps
      Bool_t finished=false;    //is turned true either if the implemented trigger logic is fullfilled or if there are no more adjacend pads due to edges,etc.
      Bool_t sealtopcol=false;  //the "seal" bools register when the logical end of the cluster was found
      Bool_t sealbotcol=false;
      Bool_t sealtoprow=false;
      Bool_t sealbotrow=false;
      Bool_t rowchange=false;   //flags that there is a possible two row cluster
      Bool_t addtop=false;      //adds the buffered information of the second row
      Bool_t addbot=false;
    
      // //deque which contains the actual cluster
      deque<std::pair<Int_t,  CbmTrdDigi*>>   cluster;
      cluster.push_back(make_pair(digiId, digi));
      if(print)    std::cout<<" module: " << fModAddress<<"   time: " << time<<"   charge: " << Charge<<"   col: " << channel % ncols<<"   row: " << channel / ncols<<"   trigger: " << triggerId<<"  ncols: " << ncols<<std::endl;
      //    std::cout<<" module: " << fModAddress<<"   time: " << time<<"   charge: " << Charge<<"   col: " << channel % ncols<<"   trigger: " << triggerId<<"  ncols: " << ncols<<std::endl;
      get<1>(*mainit)= true;
    
      //      Bool_t mergerow=CbmTrdClusterFinder::HasRowMerger();
      Bool_t mergerow=true;
      //loop to find the other pads corresponding to the main trigger
      while(!finished){
	dmain=0;

	//for (FNit=fDigiMap.begin() ; FNit != fDigiMap.end();FNit++) {
	for (FNit=start ; FNit != fDigiMap.end();FNit++) {

	  //some information to serparate the time space and to skip processed digis
	  //	continue;

	  CbmTrdDigi *d =          (CbmTrdDigi*) get<2>(*FNit);
	  Double_t newtime=        d->GetTime();
	  Double_t dt =            newtime-time;
	  Bool_t filled =          get<1>(*FNit);
	  if(filled)    continue;
	  if(dt<-interval)   continue;
	  if(dt>interval)    break;
	
	  //position information of the possible neighbor digis
	  Double_t charge =        d->GetCharge();
	  digiAddress  =           d->GetAddress();
	  Int_t digiid    =        get<0>(*FNit);
	  Int_t ch =              d->GetAddressChannel();
	  Int_t col =              ch % ncols;
	  Int_t trigger =          d->GetTriggerType();

	  if(mergerow){
	    //multiple row processing
	    //first buffering

	    if(ch == channel-ncols && !rowchange && trigger==CbmTrdDigi::kSelf && !get<1>(*FNit))     {
	      rowchange=true;
	      bufferbot[0]=charge;
	      counterbot++;
	      get<0>(botdigi)=d;
	    }
	    if(ch == (channel-ncols)-1 && rowchange && !get<1>(*FNit))                {
	      bufferbot[1]=charge;
	      counterbot++;
	      get<1>(botdigi)=d;
	    }
	    if(ch == (channel-ncols)+1 && rowchange && !get<1>(*FNit))                {
	      bufferbot[2]=charge;
	      counterbot++;
	      get<2>(botdigi)=d;
	    }     
	    if(ch == channel+ncols && !rowchange && trigger==CbmTrdDigi::kSelf && !get<1>(*FNit))     {
	      rowchange=true;
	      buffertop[0]=charge;
	      countertop++;
	      get<0>(topdigi)=d;
	    }
	    if(ch == (channel+ncols)-1 && rowchange && !get<1>(*FNit))                {
	      buffertop[1]=charge;
	      countertop++;
	      get<1>(topdigi)=d;
	    }
	    if(ch == (channel+ncols)+1 && rowchange && !get<1>(*FNit))                {
	      buffertop[2]=charge;
	      countertop++;
	      get<2>(topdigi)=d;
	    }

	    if(ch == channel-1)                {
	      bufferrow[1]=charge;
	      counterrow++;
	      get<1>(topdigi)=d;
	    }
	    if(ch == channel+1)                {
	      bufferrow[2]=charge;
	      counterrow++;
	      get<2>(topdigi)=d;
	    }

	    //then the calculation of the center of gravity with the identification of common CoGs
	    if(countertop==3)                                                               {CoGtop=(buffertop[2]/buffertop[0])-(buffertop[1]/buffertop[0]);}
	    if(counterbot==3)                                                               {CoGbot=(bufferbot[2]/bufferbot[0])-(bufferbot[1]/bufferbot[0]);}
	    if(counterrow==3)                                                               {CoGrow=(bufferrow[2]/bufferrow[0])-(bufferrow[1]/bufferrow[0]);}
	    if(countertop==3 && counterrow==3 && !addtop && TMath::Abs((CoGtop-CoGrow))< 0.25*CoGrow)    {addtop=true;}
	    if(counterbot==3 && counterrow==3 && !addbot && TMath::Abs((CoGbot-CoGrow))< 0.25*CoGrow)    {addbot=true;}
	  }
	
	  //logical implementation of the trigger logic in the same row as the main trigger
	  if(ch == lowcol-1 && trigger==CbmTrdDigi::kSelf && !get<1>(*FNit))                      {
	    cluster.push_back(make_pair(digiid, d));
	    lowcol=ch;
	    dmain++;get<1>(*FNit)= true;
	    if(print)	  std::cout<<" time: " << newtime<<" charge: " << charge<<"   col: " << col<<"   row: " << ch / ncols<<"   trigger: " << trigger<<std::endl;
	  }
	  if(ch == highcol+1 && trigger==CbmTrdDigi::kSelf && !get<1>(*FNit))                     {
	    cluster.push_back(make_pair(digiid, d));
	    highcol=ch;
	    dmain++;
	    get<1>(*FNit)= true;
	    if(print)	  std::cout<<" time: " << newtime<<" charge: " << charge<<"   col: " << col<<"   row: " << ch / ncols<<"   trigger: " << trigger<<std::endl;
	  }
	  if(ch == highcol+1 && trigger==CbmTrdDigi::kNeighbor && !get<1>(*FNit) && !sealtopcol ) {
	    cluster.push_back(make_pair(digiid, d));
	    sealtopcol=true;
	    dmain++;
	    get<1>(*FNit)= true;
	    if(print)       std::cout<<" time: " << newtime<<" charge: " << charge<<"   col: " << col<<"   row: " << ch / ncols<<"   trigger: " << trigger<<std::endl;
	  }
	  if(ch == lowcol-1  && trigger==CbmTrdDigi::kNeighbor && !get<1>(*FNit) && !sealbotcol ) {
	    cluster.push_back(make_pair(digiid, d));
	    sealbotcol=true;
	    dmain++;
	    get<1>(*FNit)= true;
	    if(print)       std::cout<<" time: " << newtime<<" charge: " << charge<<"   col: " << col<<"   row: " << ch / ncols<<"   trigger: " << trigger<<std::endl;
	  }
	  if(col == ncols)          {sealtopcol=true;}
	  if(col == 0)              {sealbotcol=true;}

	  if(mergerow){	   
	    //adding of the neighboring row
	    if(ch == channel-ncols && addbot && !get<1>(*FNit))                                  {
	      cluster.push_back(make_pair(digiid, d));
	      lowrow=ch;
	      highrow=ch;
	      dmain++;
	      get<1>(*FNit)= true;
	    }
	    if(ch == channel+ncols && addtop && !get<1>(*FNit))                                  {
	      cluster.push_back(make_pair(digiid, d));
	      lowrow=ch;
	      highrow=ch;
	      dmain++;
	      get<1>(*FNit)= true;
	    }
	    if(rowchange && ch == lowrow-1 && lowrow!=channel && trigger==CbmTrdDigi::kSelf && !get<1>(*FNit))      {
	      cluster.push_back(make_pair(digiid, d));
	      lowrow=ch;
	      dmain++;
	      get<1>(*FNit)= true;
	    }
	    if(rowchange && ch == highrow+1 && highrow!=channel && trigger==CbmTrdDigi::kSelf && !get<1>(*FNit))    {
	      cluster.push_back(make_pair(digiid, d));
	      highrow=ch;
	      dmain++;
	      get<1>(*FNit)= true;
	    }
	    if(rowchange && ch == highrow+1 && highrow!=channel &&trigger==CbmTrdDigi::kNeighbor && !get<1>(*FNit) && !sealtoprow)   {
	      cluster.push_back(make_pair(digiid, d));
	      sealtoprow=true;
	      dmain++;
	      get<1>(*FNit)= true;
	    }
	    if(rowchange && ch == lowrow-1  && lowrow!=channel &&trigger==CbmTrdDigi::kNeighbor && !get<1>(*FNit) && !sealbotrow)    {
	      cluster.push_back(make_pair(digiid, d));
	      sealbotrow=true;
	      dmain++;
	      get<1>(*FNit)= true;
	    }
	  }
	
	}
      
	//some finish criteria
	if(((sealbotcol && sealtopcol) && !rowchange) || dmain==0 )                  finished=true;
	if((sealbotcol && sealtopcol && sealtoprow && sealbotrow) || dmain==0)       finished=true;
	//      finished=true;
	if(print)    cout<<dmain<<endl;
      }// end of cluster completion
      if(print)    cout<<dmain<<endl;
      if(print)    cout<<endl;
      //    fClusterMap.push_back(cluster);
      Clustercount++;
      addClusters(cluster);
  }//end of main trigger loop
  //  fDigiMap.erase(fDigiMap.begin(),fDigiMap.end());
    //  }

  
  //add map with deques of clusters

  return Clustercount;
}


//_____________________________________________________________________
void CbmTrdModuleRecR::addClusters(deque<std::pair<Int_t,  CbmTrdDigi*>> cluster)
{
  //create vector for indice matching
  vector<Int_t> digiIndices(cluster.size());
  Int_t idigi = 0;
  Int_t add=0;
    
  for (std::deque<std::pair<Int_t, CbmTrdDigi*>>::iterator iDigi = cluster.begin(); iDigi != cluster.end(); iDigi++) {
    //add digi id to vector
    digiIndices[idigi] = iDigi->first;
    idigi++;
    add = iDigi->second->GetAddressModule();
  }
  
  //add the clusters to the Array
  //    const CbmDigi* digi = static_cast<const CbmDigi*>(fDigis->At(digiIndices.front()));
  Int_t size = fClusters->GetEntriesFast();
  CbmTrdCluster* newcluster = new ((*fClusters)[size]) CbmTrdCluster();

  newcluster->SetAddress(fModAddress);
  newcluster->SetDigis(digiIndices);

  //  BuildChannelMap(cluster);
}

//_______________________________________________________________________________
Bool_t CbmTrdModuleRecR::MakeHits()
{
  return kTRUE;
}

//_______________________________________________________________________________
CbmTrdHit* CbmTrdModuleRecR::MakeHit(Int_t clusterId, const CbmTrdCluster *cluster, std::vector<const CbmTrdDigi*> *digis)
{

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
  Double_t momentum = 0.;
  Int_t moduleAddress = 0;
  Double_t time = 0.;
  for(std::vector<const CbmTrdDigi*>::iterator id = digis->begin(); id!=digis->end(); id++){
    const CbmTrdDigi* digi = (*id);
    if(!digi) {continue;std::cout<<" no digi " << std::endl;}

    Double_t digiCharge = digi->GetCharge();
    
    if (digiCharge <= 0)     {std::cout<<" charge 0 " << std::endl;continue;}

    Int_t ncols= fDigiPar->GetNofColumns();
    Int_t nrows= fDigiPar->GetNofRows();
    Int_t row= digi->GetAddressChannel()/ncols;
    Int_t col= digi->GetAddressChannel()%ncols; 
    Int_t srow, sector= fDigiPar->GetSectorRow(row, srow);
    time += digi->GetTime();
    
    totalCharge += digi->GetCharge();
    //    fDigiPar->GetPadPosition(digi->GetAddress(), local_pad_posV, local_pad_dposV);
    fDigiPar->GetPadPosition(sector, col, srow, local_pad_posV, local_pad_dposV);
    
    Double_t xMin = local_pad_posV[0] - local_pad_dposV[0];
    Double_t xMax = local_pad_posV[0] + local_pad_dposV[0];
    xVar += (xMax * xMax + xMax * xMin + xMin * xMin) * digiCharge;

    Double_t yMin = local_pad_posV[1] - local_pad_dposV[1];
    Double_t yMax = local_pad_posV[1] + local_pad_dposV[1];
    yVar += (yMax * yMax + yMax * yMin + yMin * yMin) * digiCharge;

    for (Int_t iDim = 0; iDim < 3; iDim++) {
      hit_posV[iDim] += local_pad_posV[iDim] * digiCharge;
    }
    
    
  }
  time /= digis->size();
  
  if (totalCharge <= 0)   return NULL;
  
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

  Double_t PadSizeX = fDigiPar->GetPadSizeX(1);
  Double_t PadSizeY = fDigiPar->GetPadSizeY(1);
  TVector3 cluster_pad_dposV(sqrt(xVar), sqrt(yVar), 0);
	
  // --- If a TGeoNode is attached, transform into global coordinate system
  Double_t global[3];
  LocalToMaster(hit_pos, global);

  fDigiPar->TransformHitError(cluster_pad_dposV);

  //TODO: get momentum for more exact spacial error
  if ((fDigiPar->GetOrientation() == 1) || (fDigiPar->GetOrientation() == 3)){
    cluster_pad_dposV[0] = fDigiPar->GetPadSizeY(1);
    //    cluster_pad_dposV[1] = GetSpaceResolution();
  }
  else{
    cluster_pad_dposV[1] = fDigiPar->GetPadSizeY(1);
    //    cluster_pad_dposV[0] = GetSpaceResolution();
  }
      
  Int_t nofHits = fHits->GetEntriesFast();
  //return new ((*fHits)[nofHits]) CbmTrdHit(fModAddress, global, cluster_pad_dposV, 0, clusterId, 0, 0, totalCharge);

  //  return bla;
  return new ((*fHits)[nofHits]) CbmTrdHit(fModAddress, global, cluster_pad_dposV, 0, clusterId,0, 0, totalCharge,time,Double_t(CbmTrdDigi::Clk(CbmTrdDigi::kSPADIC)));
  
}


Double_t CbmTrdModuleRecR::GetSpaceResolution(Double_t val){

  std::pair<Double_t,Double_t> res[12] = {
    make_pair(0.5,0.4),make_pair(1,0.35),make_pair(2,0.3),make_pair(2.5,0.3),make_pair(3.5,0.28),
    make_pair(4.5,0.26),make_pair(5.5,0.26),make_pair(6.5,0.26),make_pair(7.5,0.26),make_pair(8.5,0.26),
    make_pair(8.5,0.26),make_pair(9.5,0.26)
  };

  Double_t selval = 0.;
  
  for(Int_t n=0;n<12;n++){
    if(val < res[0].first)             selval = res[0].second;
    if(n == 11)                        {selval = res[11].second;break;}
    if(val >= res[n].first && val <= res[n+1].first){
      Double_t dx = res[n+1].first - res[n].first;
      Double_t dy = res[n+1].second - res[n].second;
      Double_t slope = dy / dx ;
      selval = (val - res[n].first) * slope + res[n].second;
      break;
    }
  }

  return selval;
  

}

ClassImp(CbmTrdModuleRecR)

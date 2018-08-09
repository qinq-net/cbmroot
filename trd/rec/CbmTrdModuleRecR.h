#ifndef CBMTRDMODULERECR_H
#define CBMTRDMODULERECR_H

#include "CbmTrdModuleRec.h"
#include <map>
#include <list>
#include <vector>
#include <deque>

using std::pair;
using std::tuple;

/**
  * \brief Rectangular pad module; Cluster finding and hit reconstruction algorithms
  **/
class CbmTrdModuleRecR : public CbmTrdModuleRec
{
public:
  /**
   * \brief Default constructor.
   **/
  CbmTrdModuleRecR();
  /**
  * \brief Constructor with placement
  **/
  CbmTrdModuleRecR(Int_t mod, TGeoPhysicalNode* node=nullptr, 
                  Int_t ly=-1, Int_t rot=0, 
                  Double_t x=0, Double_t y=0, Double_t z=0, 
                  Double_t dx=0, Double_t dy=0, Double_t dz=0);
  virtual ~CbmTrdModuleRecR();

  virtual Bool_t  AddDigi(CbmTrdDigi *d, Int_t id);

  /**
   * \brief Clear local storage.
   * \sa CbmTrdModuleRec::Clear()
   **/
  virtual void    Clear(Option_t *opt="");
  /**
   * \brief Steering routine for finding digits clusters
   **/
  virtual Int_t   FindClusters();
  
  Int_t           GetOverThreshold() const  { return fDigiCounter;}
  /**
   * \brief Steering routine for building hits
   **/
  virtual Bool_t      MakeHits();
  /**
   * \brief Steering routine for converting cluster to hit
   **/
  virtual CbmTrdHit*  MakeHit(Int_t cId, const CbmTrdCluster *c, std::vector<const CbmTrdDigi*> *digis);

protected:
  

private:
  CbmTrdModuleRecR(const CbmTrdModuleRecR &ref);
  const CbmTrdModuleRecR& operator=(const CbmTrdModuleRecR &ref);
  
  void       addClusters(std::deque<std::pair<Int_t,  CbmTrdDigi*>> cluster);
  Int_t      fDigiCounter;       // digits over threshold
  
  std::deque<std::tuple<Int_t, Bool_t, CbmTrdDigi*>>   fDigiMap; //map to sort all digis from the Array into a deque; different module are separated; the tuple contains the digi indice, a bool to flag processed digis and the digi itself
  std::deque<std::deque<std::pair<Int_t, CbmTrdDigi*>>> fClusterMap; //map to store the clusters and the digi indices for later matching
  
  ClassDef(CbmTrdModuleRecR, 1)  // Rectangular pad module; Cluster finding and hit reconstruction algorithms
};
#endif

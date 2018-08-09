#ifndef CBMTRDMODULERECT_H
#define CBMTRDMODULERECT_H

#include "CbmTrdModuleRec.h"
#include <map>

/**
  * \brief Triangular pad module; Cluster finding and hit reconstruction algorithms
  **/
class CbmTrdModuleRecT : public CbmTrdModuleRec
{
public:
  /**
   * \brief Default constructor.
   **/
  CbmTrdModuleRecT();
  /**
  * \brief Constructor with placement
  **/
  CbmTrdModuleRecT(Int_t mod, TGeoPhysicalNode* node=nullptr, 
                  Int_t ly=-1, Int_t rot=0, 
                  Double_t x=0, Double_t y=0, Double_t z=0, 
                  Double_t dx=0, Double_t dy=0, Double_t dz=0);
  
  virtual ~CbmTrdModuleRecT();


  /**
   * \brief Steering routine for finding digits clusters
   **/
  virtual Int_t  FindClusters();
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
  CbmTrdModuleRecT(const CbmTrdModuleRecT &ref);
  const CbmTrdModuleRecT& operator=(const CbmTrdModuleRecT &ref);

  ClassDef(CbmTrdModuleRecT, 1)  // Triangular pad module; Cluster finding and hit reconstruction algorithms
};
#endif

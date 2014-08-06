/**
 * \file CbmFiberHodoCluster.h
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \brief Data Container for TRD clusters.
 **/

#ifndef CBMFIBERHODOCLUSTER_H
#define CBMFIBERHODOCLUSTER_H 

#include "CbmCluster.h"

class CbmFiberHodoCluster : public CbmCluster
{
public:

  /**
   * \brief Default constructor.
   */
  CbmFiberHodoCluster();
  CbmFiberHodoCluster(Int_t index){AddDigi(index);}

  /**
   * \brief Destructor.
   */
  virtual ~CbmFiberHodoCluster();
  
  void SetTime(Double_t time)
  {
    fTime = time;
  }
  void SetMean(Double_t mean)
  {
    fMean = mean;
  }
  void SetMeanError(Double_t meanError)
  {
    fMeanError = meanError;
  }
  Double_t GetTime() const
  {
    return fTime;
  }
  Double_t GetMean() const
  {
    return fMean;
  }
  Double_t GetMeanError() const
  {
    return fMeanError;
  }

 private:
  CbmFiberHodoCluster(const CbmFiberHodoCluster&);
  CbmFiberHodoCluster& operator=(const CbmFiberHodoCluster&);
  
  Double_t fTime; // FIXME: Modify clustering algorithm and remove this parameter.
  Double_t fMean; // FIXME: Modify clustering algorithm and remove this parameter.
  Double_t fMeanError; // FIXME: Modify clustering algorithm and remove this parameter.

  ClassDef(CbmFiberHodoCluster, 2)


};
#endif

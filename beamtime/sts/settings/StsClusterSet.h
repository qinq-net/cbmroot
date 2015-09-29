/** StsClusterSet.h
 *@author Anna Senger <a.senger@gsi.de>
 **
 ** Task to calculate time differences for the cluster building.
 **/

#ifndef STSCLUSTERSET_H
#define STSCLUSTERSET_H 

#include "FairTask.h"
#include "CbmStsAddress.h"
#include "CbmStsDigi.h"

#include "TH1F.h"
#include "TH2F.h"

#include <map>
#include <set>

#include "TClonesArray.h"



struct classcomp2 {
  bool operator() (const CbmStsDigi* lhs, const CbmStsDigi* rhs) const
    {return CbmStsAddress::GetElementId(lhs->GetAddress(),kStsChannel) < CbmStsAddress::GetElementId(rhs->GetAddress(),kStsChannel);}
};
    
class StsClusterSet : public FairTask
{
  
 public:
  
  /**
   * Default constructor.
   */
  StsClusterSet();
  
  /**
   * Default destructor.
   */
  ~StsClusterSet();
 
  /** Initialisation **/
  virtual InitStatus ReInit();
  virtual InitStatus Init();
  virtual void SetParContainers();
  
  /** Executed task **/
  virtual void Exec(Option_t * option);
  
  /** Selection of the strips with signal charge **/  
  void SetChargeLimitsStrip(Double_t min[3], Double_t max[3]) { for(int i=0; i<3; i++){fChargeMinStrip[i] = min[i]; fChargeMaxStrip[i] = max[i];} }

  /** Finish task **/
  virtual void Finish();

 private:

  TClonesArray*     fDigis;       /** Input array of CbmStsDigi **/

  TH1F *time_diff_strips[3];
  TH1F* cluster_adc[3][2];
  
  
  Double_t fChargeMinStrip[3];
  Double_t fChargeMaxStrip[3];

  std::map<Int_t, std::set<CbmStsDigi*, classcomp2> > fDigiMap;   /** digis per hodo layer **/ 

  StsClusterSet(const StsClusterSet&);
  StsClusterSet& operator=(const StsClusterSet&);
  Int_t fEvent;

  ClassDef(StsClusterSet,1);
  
};
#endif

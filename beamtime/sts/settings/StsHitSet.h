/** StsHitSet.h
 *@author Anna Senger <a.senger@gsi.de>
 **
 ** Task to calculate sets for the hit building.
 **/

#ifndef STSHITSET_H
#define STSHITSET_H 

#include "FairTask.h"
#include "CbmStsAddress.h"
#include "CbmStsDigi.h"

#include "TH1F.h"
#include "TH2F.h"

#include <map>
#include <set>

//don't know why forward declaration doesn't work here
//class TClonesArray;
#include "TClonesArray.h"



class StsHitSet : public FairTask
{
  
 public:
  
  /**
   * Default constructor.
   */
  StsHitSet();
  
  /**
   * Default destructor.
   */
  ~StsHitSet();
 
  /** Initialisation **/
  virtual InitStatus ReInit();
  virtual InitStatus Init();
  virtual void SetParContainers();
  
  /** Executed task **/
  virtual void Exec(Option_t * option);
  Int_t GetEntries () { return fChain->GetEntries();}
  
  /** Finish task **/
  virtual void Finish();
 
 private:

  TClonesArray*     stsClusters;    /** Input array of CbmStsCluster **/
  
  TChain *fChain;           

  TH1F *sts_time_diff[3];
  TH1F *cluster_time_diff[3];
 
  StsHitSet(const StsHitSet&);
  StsHitSet& operator=(const StsHitSet&);
  Int_t fEvent;

  ClassDef(StsHitSet,1);
  
};
#endif

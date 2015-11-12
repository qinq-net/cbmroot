/** CbmRunAna.h
 *@author Volker Friese v.friese@gsi.de
 *@since 10.12.2009
 *@version 1.0
 **
 ** Run manager for reconstruction and analysis in CBM.
 ** Derives from FairRunAna, but overrides the Run() method in order to
 ** enable asynchroneous output (fill output tree on demand and not after
 ** processing one input event).
 **/


#ifndef CBMRUNANA_H
#define CBMRUNANA_H 1


#include "FairRootManager.h"
#include "FairRunAna.h"


class CbmRunAna : public FairRunAna
{

 public:

  /**   Constructor   **/
  CbmRunAna();


  /**   Destructor   **/
  virtual ~CbmRunAna();


  /**   Mark tree for filling after tasks' execution  **/
  void MarkFill(Bool_t mark = kTRUE) { fMarkFill = mark; }


  /**   Set asynchronous output mode  **/
  void SetAsync(Bool_t async = kTRUE) { fAsync = async; }


  /**   Run all events in input file  **/
  void Run() { ExecRun(0, fRootManager->GetInChain()->GetEntries() - 1); }


  /**   Run n events 
   **@param nEvents   Number of events to be processed  **/
  void Run(Int_t nEvents) { ExecRun(0, nEvents - 1); }


  /**   Run from event iStart to event iStop
   **@param iStart   Number of first event
   **@param iStop    Number of last event
   **/
  void Run(Int_t iStart, Int_t iStop) { ExecRun(iStart, iStop); }



 private:

  Bool_t fAsync;       /** Flag for asynchronous output mode **/
  Bool_t fMarkFill;    /** Flag for filling output tree at end of event **/
  FairRunInfo fRunInfo;
  // TODO: fRunInfo shadows FairRunAna::fRunInfo, because that obe is declared private.


  /**   Run execution
   **@param iStart  Number of first event
   **@param iStop   Number of last event
   **/
  void ExecRun(Int_t iStart, Int_t iStop);


  /** Fill output tree
   **
   ** FairRootManager::Fill() will be called in asynchronous mode only
   ** when fMarkFill was set to kTRUE. In normal mode, it will be called
   ** anyway.
   **/
  void Fill();


  ClassDef(CbmRunAna,2);

};

#endif



  




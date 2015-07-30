/** CbmRunOnline.h
 *@author Pierre-Alain Loizeau p.-a.loizeau@gsi.de
 *@since 28.04.2015
 *@version 1.0
 **
 ** Run manager for unpacking, reconstruction and analysis in CBM with not root sources.
 ** Derives from FairRunOnline, but overrides the Run() and EventLoop() methods in order to
 ** enable asynchroneous output (fill output tree on demand and not after
 ** processing one input event).
 **/


#ifndef CBMRUNONLINE_H
#define CBMRUNONLINE_H 1


#include "FairRootManager.h"
#include "FairRunOnline.h"


class CbmRunOnline : public FairRunOnline
{

 public:

  /**   Constructor   **/
  CbmRunOnline();


  /**   Destructor   **/
  virtual ~CbmRunOnline();


  /**   Mark tree for filling after tasks' execution  **/
  void MarkFill(Bool_t mark = kTRUE) { fMarkFill = mark; }


  /**   Set asynchroneous output mode  **/
  void SetAsync(Bool_t async = kTRUE) { fAsync = async; }


  /**   Run all events in input file
   * => not working with RunOnline as some sources do not provide the input number of events **/
//  void Run() { ExecRun(0, fRootManager->GetInChain()->GetEntries()); }

  /** Run for the given number of events
   ** @param nEvents   Number of events to be processed  **/
  void Run(Int_t nEvents) { ExecRun(nEvents); }

  /**   Run from event iStart to event iStop
   **@param iNbEvt   Number of input events to process
   **@param dummy    Dummy parameter for compatibility with RunAna
   **/
  void Run(Int_t iNbEvt, Int_t dummy) { ExecRun(iNbEvt); }



 private:

  /**   Run execution
   **@param iNbEvt   Number of input events to process
   **/
  void ExecRun(Int_t iNbEvt);

  /** Main Event loop **/
  Int_t EventLoop();


  Bool_t fAsync;       /** Flag for asynchroneous output mode **/
  Bool_t fMarkFill;    /** Flag for filling output tree at end of event
         ** Only relevant in asynchroneous mode **/



  ClassDef(CbmRunOnline,1);

};

#endif








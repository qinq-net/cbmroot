/** @file CbmDigitizationSource.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 09.11.2018
 **/

#ifndef CBMDIGITIZATIONSOURCE_H
#define CBMDIGITIZATIONSOURCE_H 1

#include <map>
#include <set>
#include "TObject.h"
#include "TString.h"
#include "FairEventHeader.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "FairSource.h"
#include "CbmInputChain.h"

class FairEventHeader;

/** @class CbmDigitizationSource
 ** @brief Source class for the input to digitization in CBM
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 9 November 2018
 **
 ** This class serves as input source for CBM digitization runs.
 ** It allows to mix an arbitrary number of different inputs
 ** (results of transport simulations) with different event rates.
 ** For each input chain, a sequence of event times is calculated.
 ** The respective next event is delivered to FairRootManager.
 **
 ** The API is the method AddInput, specifying a TChain with an
 ** event rate. The source object has to be registered to the run.
 ** The large number of non-API public methods is due to the
 ** implementation of the base class FairSource.
 **
 ** The inputIds are given sequentially in the order of added inputs.
 **
 ** The class is based on FairMixedSource by M. Al-Turany, stripping
 ** unneeded functionality and introducing a different concept of
 ** input mixing.
 **/
class CbmDigitizationSource : public FairSource
{

  public:

    /** @brief Constructor **/
    CbmDigitizationSource();


    /** @brief Destructor **/
    virtual ~CbmDigitizationSource();


    /** @brief Activate a branch and set its address
     ** @param object  Pointer to pointer to branch class
     ** @param branchName  Name of branch
     **
     ** Inherited from FairSource. This method will be called from
     ** FairRootManager::ActivateBranch(branchName) when an input
     ** branch is demanded by a task.
     **
     ** All input trees have to be connected to the argument object.
     **/
    virtual Bool_t ActivateObject(TObject** object, const char* branchName);


    /** @brief Add an transport input
     ** @param inputId   Input number (identifier)
     ** @param fileName  Input file name
     ** @param rate      Input event rate (1/s)
     **/
    void AddInput(UInt_t inputId, TChain* chain, Double_t rate,
                  Cbm::ETreeAccess mode = Cbm::kRegular);


    /** @brief Maximal entry number the source can run to
     ** @param lastEntry  Last entry as specified by FairRunAna
     ** @value Last entry possible with this source
     **
     ** Inherited from FairSource. Since there can be several inputs
     ** being randomly mixed together, a maximal entry number cannot
     ** be specified. Thus, a practically infinite value is returned.
     ** The run is then terminated by ReadEvent() returning non-zero.
     ** If lastEntry is specified by FairRunAna (i.e., by the user),
     ** this is the return value.
     **/
    virtual Int_t CheckMaxEventNo(Int_t lastEntry = 0);


    /** @brief Abstract in base class. No implementation here.
     **
     ** Is actually not called at all from FairRunAna.
     **/
    virtual void Close() {
    }


    /** @brief Fill the output event header
     ** @param event Pointer to event header
     **
     ** Fills run ID, input ID, entry ID and event time.
     **/
    virtual void FillEventHeader(FairEventHeader* event);


    /** @brief List of branch names
     ** @value Reference to set of branch names
     **/
    const std::set<TString>& GetBranchList() const {
      return fBranches;
    }


    /** @brief Input
     ** @param inputId  Input identifier
     ** @value Pointer to CbmInputChain object
     **/
    CbmInputChain* GetInput(UInt_t inputId);


    /** @brief Source type is kFILE **/
    virtual Source_Type GetSourceType() {
      LOG(FATAL) << "GetSourceTpye" << FairLogger::endl;

      return kFILE;
    }


    /** @brief Abstract in base class. No implementation here. **/
    virtual Bool_t Init();


    /** @brief Abstract in base class. No implementation here.
     **
     ** Is actually not called at all from FairRunAna.
     **/
    virtual Bool_t InitUnpackers() {
      return kTRUE;
    }


    /** @brief Provide one tree entry
     ** @param event Event number. Has no effect here.
     ** @value 0 for success, 1 is end of tree is reached.
     **
     ** The input with the smallest next event time is chosen.
     ** From each input, the entries are provided consecutively.
     ** If the end of the respective input tree is reached, such
     ** that no more entries can be read, 1 is returned, causing
     ** FairRunAna to stop the run.
     **/
    virtual Int_t ReadEvent(UInt_t event = 0);


    /** @brief Abstract in base class. No implementation here.
     **
     ** Is actually not called at all from FairRunAna.
     **/
    virtual Bool_t ReInitUnpackers() {
      return kTRUE;
    }


    /** @brief Abstract in base class. No implementation here. **/
    virtual void Reset() {
    }


    /** @brief Set event-by-event mode
     ** @value choice  kTRUE if event-by-event mode
     **
     ** In the event-by-event mode, only the first input is processed.
     ** No event start time is generated; the event time is always zero.
     **/
    void SetEventMode(Bool_t choice = kTRUE) {
      fEventMode = choice;
    }


    /** @brief Abstract in base class. No implementation here.
     **
     ** Is actually not called at all from FairRunAna.
     **/
    virtual void SetParUnpackers() {
    }


  private:

    std::map<UInt_t, CbmInputChain*> fInputs; //! Key is inputId
    std::map<Double_t, UInt_t> fNextEvent;    //! Key is time, value is inputId
    FairMCEventHeader* fMCEventHeader;
    TObjArray* fListOfFolders;
    std::set<TString> fBranches;              // List of branches names
    Double_t fCurrentTime;
    Int_t fCurrentEntryId;
    Int_t fCurrentInputId;
    Int_t fCurrentRunId;
    Bool_t fFirstCall;
    Bool_t fEventMode;


    /** @brief Compare an input branch list with the global branch list
     ** @param input Input chain
     ** @value kTRUE if the branch list of the input is compatible
     **
     ** The branch list of the input is considered compatible if all branches
     ** of the global list are present in the input. Additional branches
     ** in the input are not considered harmful. The global branch list
     ** is defined by the first input.
     **/
    Bool_t CheckBranchList(CbmInputChain* input);



    ClassDef(CbmDigitizationSource, 1);

};

#endif /* CBMDIGITIZATIONSOURCE_H */

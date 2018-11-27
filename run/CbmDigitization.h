/** @file CbmDigitization.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 22.05.2018
 **/

#ifndef CBMDIGITIZATION_H
#define CBMDIGITIZATION_H 1

#include <map>
#include <vector>
#include "TList.h"
#include "TNamed.h"
#include "TString.h"
#include "TSystem.h"
#include "FairLogger.h"
#include "CbmDaq.h"
#include "CbmDefs.h"
#include "CbmDigitizationSource.h"
#include "CbmDigitizeInfo.h"
//#include "CbmInputChain.h"

class TGeoManager;
class CbmDigitize;


class CbmDigitization : public TNamed
{

  public:

    /** @brief Constructor **/
    CbmDigitization();


    /** @brief Destructor  **/
    virtual ~CbmDigitization();


    /** @brief Add an input file
     ** @param inputId   Unique input identifier
     ** @param fileName  Name of input file (MC)
     ** @param eventRate Rate for events from input file [1/s]
     ** @param mode      Tree access mode (kRegular / kRepeat / kRandom)
     **/
    void AddInput(UInt_t inputId, TString fileName, Double_t eventRate = -1.,
                  Cbm::ETreeAccess mode = Cbm::kRegular);


    /** @brief Add an input file
     ** @param fileName  Name of input file (MC)
     ** @param eventRate Rate for events from input file [1/s]
     ** @param mode      Tree access mode (kRegular / kRepeat / kRandom)
     **
     ** Shortcut for legacy reasons, when only one input file is used.
     ** This will set the inputId to zero. Repeated use will lead to abort.
     **/
    void AddInput(TString fileName, Double_t eventRate = -1.,
                  Cbm::ETreeAccess mode = Cbm::kRegular) {
      AddInput(0, fileName, eventRate, mode);
    }

    /** @brief Add an ASCII parameter file
     ** @param fileName  Name of parameter file
     ** @value kTRUE is file is found
     **
     ** All ASCII parameter files will be concatenated and used
     ** as second input to the runtime database.
     */
    Bool_t AddParameterAsciiFile(TString fileName);


    /** @brief Embed an input file into another one
     ** @param inputId   Unique input identifier
     ** @param fileName  Name of input file (MC)
     ** @param targetInputId ID of the input to be embedded into
     ** @param mode      Tree access mode (kRegular / kRepeat / kRandom)
     **/
    void EmbedInput(UInt_t inputId, TString fileName, UInt_t targetInputId,
                    Cbm::ETreeAccess mode = Cbm::kRegular);


    /** @brief Enable resource monitoring (default is kTRUE)
     ** @param choice If kTRUE, resources will be monitored
     **/
    void EnableMonitor(Bool_t choice = kTRUE) {
      fMonitor = choice;
    }


    /** @brief Write run info (default is kTRUE)
     ** @param choice  If kTRUE, run info will be written
     **
     ** The run info comprises CPU and memory consumption for each event.
     ** It will be written to a separate ROOT file.
     */
    void GenerateRunInfo(Bool_t choice = kTRUE) {
      fGenerateRunInfo = choice;
    }


    /** @brief Deactivate a system for digitisation
     ** @param system System ID (ECbmModuleId)
     **
     ** The digitiser for this system will not be run even if
     ** the MCPoint branch is present in the input tree.
     **/
    void Deactivate(Int_t system);


    /** @brief Process all events from input **/
    void Run() { Run(-1, -1); }


    /** @brief Process nEvents from input, starting with the first event
     ** @param nEvents  Number of events to process
     **/
    void Run(Int_t nEvents) { Run(0, nEvents); }


    /** @brief Process input from event1 to event2
     ** @param event1  First event to process
     ** @param event2  Last event to process
     **/
    void Run(Int_t event1, Int_t event2);


    /** @brief Set a digitizer explicitly
     ** @param system System ID (ECbmModuleId)
     ** @param digitizer Pointer to digitizer instance
     ** @param branch Input branch name (MCPoint)
     ** @param persistent if kTRUE, the output (digis) will be persistent
     **
     ** This method has to be called from the macro level, if a digitizer
     ** different from the default one or with different settings than the
     ** default ones is to be used. The respective digitizer has to be
     ** instantiated before. The ownership is passed to this class.
     **/
    void SetDigitizer(Int_t system, CbmDigitize* digitizer,
                      TString branch = "", Bool_t persistent = kTRUE);


    /** @brief Set event-by-event mode
     ** @param choice  If kTRUE, digitisation will be done event-by-event
     **
     ** In the event-by-event mode, one time slice will be created for
     ** each input event. There will be no interference between events.
     **/
    void SetEventMode(Bool_t choice = kTRUE) {
      fDaq->SetEventMode(choice);
    }


    /** @brief Set the output file name
     ** @param path  Name of output file
     ** @param overwrite Overwrite output file if already existing
     **
     ** If the directory of the file does not exist, it will be created.
     **/
    void SetOutputFile(TString fileName, Bool_t overwrite = kFALSE);


    /** @brief Set the parameter file name
     ** @param fileName  Name of output file
     **/
    void SetParameterRootFile(TString fileName);


    /** @brief Set length of the time-slices
     ** @param length  Length of time-slices [ns]
     **
     ** The raw data will be sorted into time-slices of the specified
     ** length.
     **
     ** If the time-slice length is negative (default), one time-slice
     ** for all input data will be created.
     **/
     void SetTimeSliceLength(Double_t length) {
       fDaq->SetTimeSliceLength(length);
     }


     /** @brief Store all time-slices
      ** @param choice If kTRUE; also empty slices will be stored.
      **
      ** By default, only time slices containing data are filled into the tree.
      **/
     void StoreAllTimeSlices(Bool_t choice = kTRUE) {
       fDaq->StoreAllTimeSlices(choice);
     }



  private:

    std::map<Int_t, CbmDigitizeInfo*> fDigitizers;
    CbmDaq* fDaq;
    CbmDigitizationSource* fSource;
    TString fOutFile;
    TString fParRootFile;
    TList fParAsciiFiles;
    Bool_t fOverwriteOutput;
    Bool_t fGenerateRunInfo;
    Bool_t fMonitor;
    Int_t fRun;


    /** @brief Copy constructor forbidden **/
    CbmDigitization(const CbmDigitization&) = delete;


    /** @brief Assignment operator forbidden **/
    CbmDigitization operator=(const CbmDigitization&) = delete;


    /** @brief Check the presence of input branches.
     ** @value Number of required branches (MCPoint) found in the tree.
     **
     ** The branch list is searched for the input branches (MCPoint)
     ** required by the digitizers. For each present input branch,
     ** the corresponding digitizer will be instantiated. In addition,
     ** the run number is extracted from the MCEventHeader.
     **/
    Int_t CheckInput();


    /** @brief Instantiate the default digitisers for the active systems
     ** @value Number of instantiated digitisers
     **
     ** For systems not having explicitly defined their digitizers,
     ** the default digitisers are instantiated.
     **/
    Int_t CreateDefaultDigitizers();


    /** @brief Get the geometry tag of a system from a TGeoManager
     ** @param system  Detector system (ECbmModuleId)
     ** @param geo     Pointer to TGeoManager
     ** @value Geometry tag
     **
     ** The implementation assumes that the top-level volume name of each
     ** system in the geometry contains the geometry tag in the form of
     ** e.g., sts_v16g
     **/
    TString GetGeoTag(Int_t system, TGeoManager* geo);


    /** @brief Default settings for digitizers **/
    void SetDefaultBranches();


    ClassDef(CbmDigitization, 1);
};

#endif /* CBMDIGITIZATION_H */

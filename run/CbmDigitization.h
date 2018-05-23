/** @file CbmDigitization.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 22.05.2018
 **/

#ifndef CBMDIGITIZATION_H
#define CBMDIGITIZATION_H 1

#include <map>
#include <vector>
#include "TNamed.h"
#include "CbmDefs.h"
#include "CbmDigitizeInfo.h"



class CbmDigitization : public TNamed
{

  public:

    /** @brief Constructor **/
    CbmDigitization();


    /** @brief Destructor  **/
    virtual ~CbmDigitization();


    /** @brief Add an input file
     ** @param fileName  Name of input file (MC)
     ** @param eventRate Rate for events from input file [1/s]
     **/
    Int_t AddInput(TString fileName, Double_t eventRate = -1.);


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
    void Run(Int_t nEvents) { Run(0, nEvents-1); }


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
     ** default ones is to be used. The respective digitizer has to be instantiated
     ** before. The ownership is passed to this class.
     **/
    void SetDigitizer(Int_t system, FairTask* digitizer, TString branch = "",
                      Bool_t persistent = kTRUE);


    /** @brief Set the output file name
     ** @param fileName  Name of output file
     **/
    void SetOutputFile(TString fileName) { fOutFile = fileName; }


    /** @brief Set the parameter file name
     ** @param fileName  Name of output file
     **/
    void SetParameterFile(TString fileName) { fParFile = fileName; }


  private:

    std::map<Int_t, CbmDigitizeInfo*> fDigitizers;
    std::vector<TString> fInputFiles;
    std::vector<Double_t> fEventRates;
    TString fOutFile;
    TString fParFile;
    Bool_t fEventMode;


    /** @brief Check the presence of input arrays (MCPoint) in the tree.
     ** @value Number of MCPoint arrays found in the tree.
     **
     ** For each present input array, the corresponding digitizer will be
     ** instantiated.
     **/
    Int_t CheckInputArrays();

    /** @brief Check the presence of input arrays (MCPoint) in the tree.
     ** @value Number of MCPoint arrays found in the tree.
     **
     ** For each present input array, the corresponding digitizer will be
     ** instantiated.
     **/
    Int_t CheckInputFile();


    /** @brief Instantiate the digitisers for the active systems
     ** @value Number of instantiated digitisers
     **
     ** For systems not having explicitly defined their digitizers,
     ** the default digitisers are instantiated.
     **/
    Int_t CreateDigitizers();


    /** @brief Set the input array names
     **
     ** The names are fixed by convention and hard-coded here.
     **/
    void SetArrayNames();


    /** @brief Default settings for digitizers **/
    void SetDefaults();


    ClassDef(CbmDigitization, 1);
};

#endif /* CBMDIGITIZATION_H */

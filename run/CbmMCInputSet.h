/** @file CbmMCInputSet.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.11.2018
 **/

#ifndef CBMMCINPUTSET_H
#define CBMMCINPUTSET_H 1

#include <map>
#include <set>
#include <utility>
#include "TF1.h"
#include "TObject.h"
#include "CbmInputChain.h"



/** @class CbmMCInputSet
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23 November 2018
 ** @brief A MC transport input to digitisation in CBM
 **
 ** CbmMCInputSet represents one MC input line to digitization.
 ** It is characterised by an event rate, allowing to sample the time
 ** difference to the previous event using the method GetDeltaT().
 ** Assuming a Poisson process, deltaT is obtained from an exponential
 ** distribution, the first moment of which being the inverse rate.
 **
 ** The input set consists of one or several single inputs (class CbmMCInput).
 ** Subsequent calls to GetNextEntry() will subsequently call
 ** CbmMCInput::GetNextEntry(), until all inputs were used once, after
 ** which GetNextEntry() will return kTRUE.
 **/
class CbmMCInputSet : public TObject
{

  public:

    /** @brief Default constructor **/
    CbmMCInputSet();


    /** @brief Constructor
     ** @param rate   Event rate [1/s]. Must be positive.
     **/
    CbmMCInputSet(Double_t rate);


    /** @brief Destructor **/
    virtual ~CbmMCInputSet();


    /** @brief Add an input to the set
     ** @param inputId  Unique input identifier
     ** @param input    Pointer to CbmMCInput object
     **/
    void AddInput(UInt_t inputId, CbmInputChain* input);


    /** @brief List of branches
     ** @value Reference to branch list
     **/
    const std::set<TString>& GetBranchList() const {
      return fBranches;
    }


    /** @brief Time difference to next event
     ** @value Time difference to next event [ns]
     **
     ** This method samples from the probability distribution for the time
     ** difference between two subsequent events, assuming a Poisson process
     ** (exponential distribution).
     ** The return value is zero if the rate was specified to be non-positive.
     **/
    Double_t GetDeltaT();


    /** @brief Get the next entry from the inputs
     ** @value Status tuple
     **
     ** The return tuple consists of:
     ** Flag whether the used input is the last one in the set
     ** ID of the used input
     ** Entry number from the used input. If -1, the end of this input
     ** was reached.
     **/
    std::tuple<Bool_t, UInt_t, Int_t> GetNextEntry();



    /** @brief Event rate
     ** @value Event rate [1/s]
     **/
    Double_t GetRate() const {
      return fRate;
    }


  private:

    Double_t fRate;                // Event rate [1/s]
    std::map<UInt_t, CbmInputChain*> fInputs; // Key is input ID
    std::map<UInt_t, CbmInputChain*>::iterator fInputHandle; // Handle for inputs
    std::set<TString> fBranches;   // List of branch names
    TF1* fDeltaDist;               // Probability distribution for delta(t)


    /** @brief Compare an input branch list with the reference branch list
     ** @param input Pointer to CbmMCInput object
     ** @value kTRUE if the branch list of the input is compatible
     **
     ** The branch list of the input is considered compatible if all branches
     ** of the global list are present in the input. Additional branches
     ** in the input are not considered harmful. The referece branch list
     ** is defined by the first input.
     **/
    Bool_t CheckBranchList(CbmInputChain* input);



    ClassDef(CbmMCInputSet, 1);

};

#endif /* CBMMCINPUT_H */

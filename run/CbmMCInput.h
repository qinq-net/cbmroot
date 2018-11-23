/** @file CbmMCInput.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 09.11.2018
 **/

#ifndef CBMMCINPUT_H
#define CBMMCINPUT_H 1

#include <set>
#include "TChain.h"
#include "TF1.h"
#include "TObject.h"
#include "CbmDefs.h"


/** @class CbmMCInput
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 9 November 2018
 ** @brief An MC (transport) input to digitisation in CBM
 **
 ** CbmMCInput gives access to the entries of a TChain according to the
 ** specified access mode through the mathod GetNextEntry(). The access to
 ** the tree entries can be sequential with stopping at the end
 ** (mode = kRegular), sequential round-the-corner (mode kRepeat) or random
 ** (mode = kRandom).
 **/
class CbmMCInput : public TObject
{

  public:

    /** @brief Default constructor **/
    CbmMCInput();


    /** @brief Constructor
     ** @param chain  Pointer to input file chain
     ** @param rate   Event rate [1/s]. Must be positive.
     ** @param mode   Access mode (see EAccessMode)
     **/
    CbmMCInput(TChain* chain, Double_t rate,
                  Cbm::ETreeAccess mode = Cbm::kRegular);


    /** @brief Destructor **/
    virtual ~CbmMCInput();


    /** @brief List of branches
     ** @value Reference to branch list
     **/
    std::set<TString>& GetBranchList();


    /** @brief Pointer to chain
     ** @value Pointer to TChain object
     */
    TChain* GetChain() const {
      return fChain;
    }


    /** @brief Time difference to next event
     ** @value Time difference to next event [ns]
     **
     ** This method samples from the probability distribution for the time
     ** difference between two subsequent events, assuming a Poisson process
     ** (exponential distribution).
     **/
    Double_t GetDeltaT();


    /** @brief Get the next unused entry from the chain
     ** @value Id of tree entry.
     **
     ** The method returns -1 if the maximum number of entries is exceeded.
     **/
    Int_t GetNextEntry();


    /** @brief Number of entries
     ** @value Number of entries in this input chain.
     **/
    Int_t GetNofEntries() const {
      return fChain->GetEntries();
    }


    /** @brief Number of used entries
     ** @value Number of successful calls to GetNextEvent()
     **/
    UInt_t GetNofUsedEntries() const {
      return fNofUsedEntries;
    }


    /** @brief Event rate
     ** @value Event rate [1/s]
     **/
    Double_t GetRate() const {
      return fRate;
    }


  private:

    TChain* fChain;                //! Input chain
    Double_t fRate;                // Event rate [1/s]
    Cbm::ETreeAccess fMode;        // Access mode to tree
    std::set<TString> fBranches;   // List of branch names
    UInt_t fLastUsedEntry;         // Index of last used entry
    UInt_t fNofUsedEntries;        // Number of used entries
    TF1* fDeltaDist;               // Probability distribution for delta(t)


    /** @brief Read list of branches from file
     ** @value Number of branches
     **
     ** The list of branches is stored as TList in each file.
     **/
    UInt_t ReadBranches();


    ClassDef(CbmMCInput, 1);

};

#endif /* CBMMCINPUT_H */

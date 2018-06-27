/** @file CbmDigitize.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 01.06.2018
 **/

#ifndef CBMDIGITIZE_H
#define CBMDIGITIZE_H 1

#include "FairTask.h"
#include "CbmDaqBuffer.h"

class CbmDigi;


/** @class CbmDigitize
 ** @brief Abstract base class for CBM digitisation tasks
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 01 June 2018
 **
 ** Derived classes have to implement the abstract method WriteDigi.
 **/
class CbmDigitize : public FairTask
{

  public:

    /** @brief Constructor **/
    CbmDigitize();


    /** @brief Constructor with name
     ** @param name Task name
     **/
    CbmDigitize(const char* name);


    /** @brief Destructor **/
    virtual ~CbmDigitize();


    /** @brief Fill custom data into time slice
     ** @param fillTime Time until data can be filled
     ** @param limit If kTRUE, only data up to fillTime will be treated; otherwise, all.
     **
     ** This method allows the digitizer to implement additional functionality
     ** than writing digis and match objects. It will be called from CbmDaq.
     **/
    virtual void FillCustomData(Double_t fillTime, Bool_t limit = kTRUE) {
    }


    /** @brief Get event information **/
    void GetEventInfo();


    /** @brief Current event time
     ** @value Start time of current event [ns]
     **/
    Double_t GetEventTime() const {
      return fCurrentEventTime;
    }


    /** @brief Send a digi object to the DAQ
     ** @param digi  Pointer to digi object
     **/
    void SendDigi(CbmDigi* digi);


    /** @brief Set the DAQ buffer instance **/
    void SetDaqBuffer(CbmDaqBuffer* buffer) {
      fDaqBuffer = buffer;
    }


    /** @brief Set event-by-event mode
     ** @param Choice If kTRUE, the digitizer will run in event-by-event mode
     **/
    void SetEventMode(Bool_t choice = kTRUE) { fEventMode = choice; }


    /** @brief Reset the output arrays
     **
     ** This method is called from CbmDaq after a tree->Fill is triggered.
     ** To be implemented by the derived class.
     **/
    virtual void ResetArrays() = 0;


    /** @brief Write a digi object to the output array
     ** @param digi Pointer to digi object
     **
     ** A copy of the digi object has to be created and inserted into
     ** the output array registered to the FairRootManager in the Init method.
     ** The corresponding match object has to be treated accordingly.
     ** This method will be called from CbmDaq.
     **/
    virtual void WriteDigi(CbmDigi* digi) = 0;


  protected:

    Bool_t fEventMode;
    Int_t fCurrentInput;
    Int_t fCurrentEvent;
    Double_t fCurrentEventTime;
    CbmDaqBuffer* fDaqBuffer;

  private:


    /** @brief Copy constructor forbidden **/
    CbmDigitize(const CbmDigitize&) = delete;


    /** @brief Assignment operator forbidden **/
    void operator=(const CbmDigitize&) = delete;


    ClassDef(CbmDigitize, 1);
};

#endif /* CBMDIGITIZE_H */

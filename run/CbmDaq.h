/** @file CbmDaq.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 20 July 2012
 **
 **/


#ifndef CBMDAQ_H
#define CBMDAQ_H 1

#include "TStopwatch.h"
#include "FairTask.h"
#include "CbmMCEventList.h"
#include "CbmTimeSlice.h"
#include "CbmDigitization.h"


class TClonesArray;
class CbmDaqBuffer;
class CbmDigi;


/** @class CbmDaq
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 20 July 2012
 ** @brief CBM task class for filling digis into time slices
 **
 ** The CbmDaq collects raw data (digis) from various input sources
 ** (detectors), sorts them w.r.t. time and fills time slices.
 ** The digis in one time slice are written to TCLonesArrays as branches of
 ** the output tree. One tree entry corresponds to one time slice (interval),
 ** the duration of which can be adjusted.
 **/
class CbmDaq : public FairTask
{

  public:

    /** @brief Constructor
     ** @param interval  Duration of a time-slice [ns]
     **
     ** By default, the DAQ will run in time-based mode. The event-based mode
     ** can be selected by the method SetEventMode.
     ** If the time-slice interval is negative (default), all data will be
     ** written into one time-slice. Otherwise, time-slice of equal intervals
     ** will be created.
     **/
    CbmDaq(Double_t interval = -1.);


   /** @brief Destructor   **/
    ~CbmDaq();


    /** @brief Task execution **/
    virtual void Exec(Option_t* opt);


    /** @brief Initialisation **/
    virtual InitStatus Init();


    /** @brief Set the time-slice interval
     ** @param interval  Duration of a time-slice [ns]
     **/
    void SetTimeSliceInterval(Double_t interval) {
      fDuration = interval;
    }


  private:

    Bool_t fEventMode;  ///< Flag for event-by-event mode
    Double_t fSystemTime;          ///< Current system time [ns]
    Double_t fCurrentStartTime;    ///< Start time of current time slice [ns]

    /** Duration of time slice [ns] **/
    Double_t fDuration;

    Double_t fBufferTime; ///< Maximal time disorder of input data

    /** Flag whether empty time-slices should be filled to the tree **/
    Bool_t fStoreEmptySlices;

    /** Timer **/
    TStopwatch fTimer;

    /** Counters **/
    Int_t fNofSteps;             ///< Number of execute steps
    Int_t fNofDigis;             ///< Total number of processed digis
    Int_t fNofTimeSlices;        ///< Number of time slices
    Int_t fNofTimeSlicesEmpty;   ///< Number of empty time slices
    Double_t fTimeDigiFirst;     ///< Time of first digi
    Double_t fTimeDigiLast;      ///< Time of last digi
    Double_t fTimeSliceFirst;    ///< Start time of first time slice
    Double_t fTimeSliceLast;     ///< Stop time of last time slice

    /** Output array (digis) **/
    std::map<Int_t, TClonesArray*> fDigis;

    /** Pointer to current time slice **/
    CbmTimeSlice* fTimeSlice;

    /** Pointer to CbmDaqBuffer instance  **/
    CbmDaqBuffer* fBuffer;
    
    /** MC event list (all events) **/
    CbmMCEventList fEventList;      //!  MC event list (all)
    CbmMCEventList* fEventsCurrent; //! MC events for current time slice

    /** First and last event in current time slice for each input **/
    std::map<Int_t, std::pair<Int_t, Int_t>> fEventRange; //!


    /** Close the current time slice
     ** The current slice is filled to the tree. It is then reset
     ** to the next time slice interval.
     */
    void CloseTimeSlice();


    /** Copy the MC events contributing to the current time slice
     ** to the output array.
     **
     ** @value  Number of MC events for this time slice
     **/
    Int_t CopyEventList();


    /** Copy data (digi) from the DaqBuffer into the output array
     ** @param digi  Pointer to digi object
     **/
    void FillData(CbmDigi* digi);


    /** Fill data from the buffer into the current time slice
     ** @param fillTime Maximum time for digis to be filled
     ** @return Number of digis filled into the time slice
     **
     ** If fillTime is negative (default), all data from the buffer
     ** are copied to the time slice.
     **/
    Int_t FillTimeSlice(Double_t fillTime = -1.);


    /** Screen log of the range of MC events contributing to the
     ** current time slice
     **/
    void PrintCurrentEventRange() const;


    /** Register input and entry number of a MC event contributing data
     ** to the current time slice.
     **
     ** CbmDaqNew stores the first and last event for each input such that
     ** it can be copied from the event list after the time slice is
     ** closed and filled to the tree.
     **/
    void RegisterEvent(CbmDigi* digi);


    /** At end of run: Process the remaining data in the CbmDaqBuffer  **/
    virtual void Finish();

    /** Copy constructor and assignment operator are not allowed. **/
    CbmDaq(const CbmDaq&) = delete;
    CbmDaq& operator=(const CbmDaq&) = delete;
    
    ClassDef(CbmDaq,1);

};

#endif /* CBMDAQ_H */

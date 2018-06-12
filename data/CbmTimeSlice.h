/** @file CbmTimeSlice.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17 July 2012
 **/

#ifndef CBMTIMESLICE_H
#define CBMTIMESLICE_H 1

#include <map>
#include <string>
#include "TNamed.h"
#include "CbmMatch.h"


#ifndef __CINT__ // for BOOST serialization
// boost
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#endif 


/** @class CbmTimeSlice
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17 July 2012
 ** @brief Bookkeeping of time-slice content
 **/
class CbmTimeSlice : public TNamed
{

  public:

    /** @brief Constructor for a flexible time-slice
     **
     ** A time-slice with flexible limits is used in the
     ** event-by-event mode or when all data are put
     ** into one time-slice.
     **/
    CbmTimeSlice();


    /** Standard constructor for a fixed-length time-slice
     ** @param start    Start time of time slice [ns]
     ** @param duration Duration of time slice [ns]
     */
    CbmTimeSlice(Double_t start, Double_t duration);


    /** Destructor **/
    ~CbmTimeSlice();


    /** @brief Add data to time-slice
     ** @param detector  System ID (ECbmModuleId)
     **
     ** The respective counter will be incremented.
     **/
    void AddData(Int_t detector) {
      fNofData[detector]++;
      fIsEmpty = kFALSE;
    }

    /** @brief Add data with time to time-slice
     ** @param detector  System ID (ECbmModuleId)
     ** @param time      Data time [ns]
     ** @value kFLASE if time is out of time-slice bounds; else kTRUE
     **
     ** The respective counter will be incremented.
     ** Time of data is checked with time-slice bounds.
     ** Time of first and last data are updated.
     **/
    Bool_t AddData(Int_t detector, Double_t time);


    /** @brief Get size of raw data container for given detector
     ** @param detector  system ID (ECbmModuleId)
     ** @value Size of raw data container (number of digis)
     **/
    Int_t GetNofData(Int_t detector) const;


    /** Duration of time slice
     **
     ** @return duration [ns]
     **/
    Double_t GetLength() const { return fLength; }


    /** Get match object
     ** @return Match object
     **/
    const CbmMatch& GetMatch() const { return fMatch; }


    /** Start time of time slice
     ** @return start time [ns]
     **/
     Double_t GetStartTime() const { return fStartTime; }


    /** End time of time slice
     ** @return end time [ns]
     **/
    Double_t GetEndTime() const { return fStartTime + fLength; }


    /** Check whether time slice contains data
     ** @return kTRUE if time slice contains data
     **/
    Bool_t IsEmpty() const { return fNofData.empty(); }


    /** @brief Reset the time slice
     ** @param start    New start time [ns]
     ** @param length   New lengt [ns]
     **
     ** Reset start time, length and counters
     **/
    void Reset(Double_t start, Double_t length);


    /** @brief Set start time
     ** @param time Start time [ns]
     **/
    void SetStartTime(Double_t time) { fStartTime = time; }


    /** Status to string **/
    std::string ToString() const;

    

    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& fLength;
        ar& fIsEmpty;
    }
    
  private:

    Double_t fStartTime;                 ///< Start time [ns]
    Double_t fLength  ;                  ///< Length of tim-slice [ns]
    Bool_t   fIsFlexible;                ///< Flag for flexible time limits
    Bool_t   fIsEmpty;                   ///< Flag for containing no data
    std::map<Int_t, Int_t> fNofData;     ///< systemId -> Number of digis
    Double_t fTimeDataFirst;             ///< Time of first data object
    Double_t fTimeDataLast;              ///< Time of last data object
    CbmMatch fMatch;                     ///< link time slice to events

    

    #ifndef __CINT__ // for BOOST serialization
    friend class boost::serialization::access;
    #endif // for BOOST serialization

    ClassDef(CbmTimeSlice, 5)
};

#endif /* CBMTIMESLICE_H */

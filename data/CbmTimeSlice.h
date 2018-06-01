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
 ** @brief Container class for CBM raw data in a given time interval
 **/
class CbmTimeSlice : public TNamed
{

  public:

    /** Default constructor **/
    CbmTimeSlice();


    /** Standard constructor
     ** @param start    Start time of time slice [ns]
     ** @param duration Duration of time slice [ns]
     */
    CbmTimeSlice(Double_t start, Double_t duration);


    /** Destructor **/
    ~CbmTimeSlice();


    /** @brief Add data to time-slice
     ** @param detector  system ID (ECbmModuleId)
     **
     ** Just for bookkeeping. The respective counter will be incremented.
     **/
    void AddData(Int_t detector) {
      fNofData[detector]++;
      fIsEmpty = kFALSE;
    }


    /** @brief Get size of raw data container for given detector
     ** @param detector  system ID (ECbmModuleId)
     ** @value Size of raw data container (number of digis)
     **/
    Int_t GetNofData(Int_t detector) const;


    /** Duration of time slice
     **
     ** @return duration [ns]
     **/
    Double_t GetDuration() const { return fDuration; }


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
    Double_t GetEndTime() const { return fStartTime + fDuration; }


    /** Check whether time slice contains data
     ** @return kTRUE if time slice contains data
     **/
    Bool_t IsEmpty() const { return fNofData.empty(); }


    /** Reset the time slice
     **
     ** A new start time is set.
     **
     ** @param start    New start time [ns]
     ** @param duration New duration [ns]
     */
    void Reset(Double_t start, Double_t duration);


    /** @brief Set start time
     ** @param time Start time [ns]
     **/
    void SetStartTime(Double_t time) { fStartTime = time; }


    /** Status to string **/
    std::string ToString() const;

    

    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& fDuration;
        ar& fIsEmpty;
    }
    
  private:

    Double_t fStartTime;                 ///< start time [ns]
    Double_t fDuration;                  ///< duration [ns]
    Bool_t   fIsEmpty;                   ///< Flag for containing no data
    std::map<Int_t, Int_t> fNofData;     ///< systemId -> Number of digis
    CbmMatch fMatch;                     ///< link time slice to events

    

    #ifndef __CINT__ // for BOOST serialization
    friend class boost::serialization::access;
    #endif // for BOOST serialization

    ClassDef(CbmTimeSlice, 4)
};

#endif /* CBMTIMESLICE_H */

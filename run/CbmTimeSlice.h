/** @file CbmTimeSlice.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17 July 2012
 **/

#ifndef CBMTIMESLICE_H
#define CBMTIMESLICE_H 1

#include <string>
#include <vector>

#include "TNamed.h"
#include "CbmStsDigi.h"
#include "CbmMuchDigi.h"
#include "CbmTofDigiExp.h"
#include "CbmDigi.h"

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

    /** Access to data
     ** @param iDet   detector type
     ** @param index  index of data object in array
     ** @return pointer to data object
     **/
    CbmDigi* GetData(DetectorId iDet, UInt_t index);


    /** Get size of raw data container for given detector
     **
     ** @param iDet   detector type
     ** @return size of raw data container (number of digis)
     */
    Int_t GetDataSize(DetectorId iDet) const;


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


    /** Copy data into the time slice
     **
     ** Data will only be copied if their time fits into the time slice
     **
     ** @param digi  pointer to data object
     **/
    void InsertData(CbmDigi* digi);


    /** Check whether timeslice contains data
     ** @return kTRUE if timeslice contains data
     **/
    Bool_t IsEmpty() const { return fIsEmpty; }


    /** Reset the time slice
     **
     ** The data vectors will be cleared and a new start time is set.
     **
     ** @param start    New start time [ns]
     ** @param duration New duration [ns]
     */
    void Reset(Double_t start, Double_t duration);


    /** Set the flag whether the time slice is empty
     ** @param isEmpty  kTRUE is timeslice contains no data
     **/
    void SetEmpty(Bool_t isEmpty = kTRUE) { fIsEmpty = isEmpty; }


    /** Consistency check
     ** Tests data to be in defined time interval and ordered w.r.t. time.
     ** @return kTRUE if OK, else kFALSE
     **/
    Bool_t SelfTest();


    /** Status to string **/
    std::string ToString() const;


    /** Get vector of much digis
     **
     ** @return MUCH raw data container (vector of digis) 
     */
    std::vector<CbmMuchDigi> GetMuchData() {return fMuchData; } 
    std::vector<CbmStsDigi>  GetStsData()  {return fStsData; }
    std::vector<CbmTofDigiExp>  GetTofData()  {return fTofData; }
    

    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& fStsData;
        ar& fMuchData;
        ar& fStartTime;
        ar& fDuration;
        ar& fIsEmpty;
    }
    
  private:

    Double_t fStartTime;                 ///< start time [ns]
    Double_t fDuration;                  ///< duration [ns]
    Bool_t   fIsEmpty;                   ///< Flag for containing no data
    std::vector<CbmStsDigi> fStsData;    ///< raw data container for STS
    std::vector<CbmMuchDigi> fMuchData;  ///< raw data container for MUCH
    std::vector<CbmTofDigiExp> fTofData; ///< raw data container for TOF
    CbmMatch fMatch;                     ///< link time slice to events

    

    #ifndef __CINT__ // for BOOST serialization
    friend class boost::serialization::access;
    #endif // for BOOST serialization

    ClassDef(CbmTimeSlice,3)
};

#endif /* CBMTIMESLICE_H */

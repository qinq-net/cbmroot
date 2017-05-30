/** @file CbmMCPointBuffer.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 13 February 2012
 **/

#ifndef CBMMCPOINTBUFFER_H
#define CBMMCPOINTBUFFER_H 1


#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>

#include "TClonesArray.h"

#include "FairMCPoint.h"


/** @class IsBefore
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 10 February 2012
 ** @brief Template comparison class for objects with time information.
 **
 ** This template class provides a weak ordering for objects having
 ** implemented the method GetTime(), which the comparison is based on.
  **/
template <class Point> class IsBefore {
 public:
  Bool_t operator() (Point point1, Point point2) const {
    return ( point1.GetTime() < point2.GetTime() ); 
  }
};




/** @class CbmMCPointBuffer
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 13 February 2012
 ** @brief Buffer class for the MCPoints of a detector
 **
 ** MCPoints are read in from a TClonesArray and stored in a time-ordered
 ** manner using the STL multiset container. They can be sequentially accessed
 ** by the method GetNextPoint, which will deliver pointers to the MCPoints
 ** with absolute times up to a specified time. The method Clear deletes all
 ** points accessed by GetNextPoint so far.
 **
 ** The template class can be used for any class having the method GetTime().
 **/
template <class T> class CbmMCPointBuffer
{


 public:

  /**   Default constructor  **/
  CbmMCPointBuffer(const char* name = "") 
   : fName(name),
     fBuffer(),
     fBufferIt(fBuffer.begin())
  {
  };


  /**   Destructor  **/
  ~CbmMCPointBuffer() {
    fBuffer.clear();
  };


  /**   Removes all points from the beginning to the current position of the
   **   iterator (all those accessed by GetNextPoint before).
   **/
  void Clear() {
    fBuffer.erase(fBuffer.begin(), fBufferIt);
    fBufferIt = fBuffer.begin();
  };


  /**   Fill the buffer with the content of a TClonesArray
   ** @param array      Pointer to TClonesArray
   ** @param eventTime  Time to be added to the MCPoint time to get the absolute time
   **/
  Int_t Fill(TClonesArray* array, Double_t eventTime, Int_t eventNr) {
    if ( ! array ) return 0;
    Int_t nPoints = array->GetEntriesFast();
    for (Int_t iPoint = 0; iPoint < nPoints; iPoint++) {
      T* point = (T*) array->At(iPoint);
      point->SetEventID(eventNr);
      point->SetTime(eventTime + point->GetTime());
      // 2 lines added by EK to propagate point index in TClonesArray
      // TODO add functionality in the buffer to return FairLink to the point
      FairLink l = point->GetLink(0);
      point->AddLink(FairLink(l.GetFile(), eventNr, -1, iPoint));
      fBuffer.insert(*point);
    }
    fBufferIt = fBuffer.begin();
    return nPoints;
  };


  /** Get the time of last data in the buffer **/
  Double_t GetMaxTime() const {
    if ( fBuffer.empty() ) return -1.;
    return fBuffer.rbegin()->GetTime();
  }


  /** Get the time of first data in the buffer **/
  Double_t GetMinTime() const {
    if ( fBuffer.empty() ) return -1.;
    return fBuffer.begin()->GetTime();
  }


  /** Get the number of entries in the buffer **/
  Int_t GetNofEntries() const {
    return fBuffer.size();
  }


  /**   Get the size of the buffer in MB
   ** @value  Buffer size [MB]
   **/
  Double_t GetSize() const { 
    return 1.e-6 * sizeof(T) * Double_t(fBuffer.size()); 
  }

  /**   Get the next (w.r.t. abolute time) MCPoint from the buffer.
   **   If there is none left, the method returns NULL.
   **   The point will stay in memory, but is marked for deletion by Clear()\
   **   through the iterator position.
   ** @value Pointer to the FairMCPoint. NULL if there are no points left.
   **/
  const FairMCPoint* GetNextPoint() {
    const FairMCPoint* point = NULL;
    if ( fBufferIt != fBuffer.end() ) {
      point = &(*fBufferIt);
      fBufferIt++;
    }
    return point;
  };


  /**   Get the next (w.r.t. abolute time) MCPoint from the buffer.
   **   Only points up to the specified time are delivered. If there is none
   **   left, the method returns NULL.
   **   The point will stay in memory, but is marked for deletion by Clear()\
   **   through the iterator position.
   ** @param time  Time up to which the points will be delivered
   ** @value Pointer to the FairMCPoint. NULL if there are no points left.
   **/
  const FairMCPoint* GetNextPoint(Double_t time) {
    const FairMCPoint* point = NULL;
    if ( fBufferIt != fBuffer.end() ) {
      if ( (*fBufferIt).GetTime() < time ) {
        point = &(*fBufferIt);
        fBufferIt++;
      }
    }
    return point;
  };


  /**  Print the content of the buffer  **/
  void PrintContent() const {
    typename std::multiset<T, IsBefore<T> >::iterator iter;
    for (iter = fBuffer.begin(); iter != fBuffer.end(); iter++) {
      std::cout << "Point: x = " << iter->GetXIn() << ", y = " << (*iter).GetYIn()
           << ", z = " << (*iter).GetZ() << ", t = " << (*iter).GetTime();
      std::cout << std::endl;
    }
  }


  /** Status string output **/
  std::string ToString() const {
    std::stringstream ss;
    ss << std::setw(4) << std::left << fName << " Buffer: " << std::setw(8) << std::right
       << fBuffer.size() << " points ("
       << std::setw(8) << std::fixed << std::setprecision(3) << GetSize() << " MB) from "
       << std::setw(10) << GetMinTime() << " ns to "
       << std::setw(10) << GetMaxTime() << " ns";
     return ss.str();
  }


 private:

  const char* fName;
  std::multiset<T, IsBefore<T> > fBuffer;
  typename std::multiset<T, IsBefore<T> >::iterator fBufferIt;


  CbmMCPointBuffer<T>(const CbmMCPointBuffer<T>&);  
  CbmMCPointBuffer<T>& operator=(const CbmMCPointBuffer<T>&);  

};


#endif

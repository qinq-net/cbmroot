/** @file CbmMCEventList.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 24.11.2015
 **/

#ifndef CBMMCEVENTLIST_H
#define CBMMCEVENTLIST_H 1

#include <map>
#include <utility>

#include <TObject.h>

using std::map;
using std::pair;

/** @class CbmMCEventList
 ** @brief Container class for MC events with number, file and start time
 **
 ** Implemented as a simple ROOT wrapper to an STL map, in order to make
 ** it usable in a TTree.
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 24.11.2015
 **/
class CbmMCEventList: public TObject
{

  public:

    /** Standard constructor **/
    CbmMCEventList();

    /** Destructor **/
    virtual ~CbmMCEventList();

    /** Delete all event entries **/
    virtual void Clear(Option_t* opt = "") {
      fEvents.clear();
    }

    /** Event start time
     ** @param event  MC event number
     ** @param file   MC input file number
     ** @value MC event start time [ns]
     **/
    Double_t GetEventTime(Int_t event, Int_t file = 0) {
      if (fEvents.find(file) == fEvents.end()) return -1.;
      if (fEvents[file].find(event) == fEvents[file].end()) return -1.;
      return fEvents[file][event];
    }

    /** Insert an event with its start time into the event list
     ** @param event  MC event number
     ** @param file   MC input file number
     ** @param time   MC event start time [ns]
     */
    void Insert(Int_t event, Int_t file, Double_t time) {
      fEvents[file][event] = time;
    }

    /** Screen info **/
    virtual void Print(Option_t* opt = "") const;

  private:

    /** Event container. Implemented as std::map:
     ** Input file number -> ( event number -> event time )
     **/
    map<Int_t, map<Int_t, Double_t>> fEvents;

  ClassDef(CbmMCEventList, 1)
    ;
};

#endif /* CBMMCEVENTLIST_H */

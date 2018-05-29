/** @file CbmMCEventList.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 24.11.2015
 **/

#ifndef CBMMCEVENTLIST_H
#define CBMMCEVENTLIST_H 1

#include <string>
#include <map>
#include <utility>

#include <TNamed.h>

/** @class CbmMCEventList
 ** @brief Container class for MC events with number, file and start time
 **
 ** Implemented as a simple ROOT wrapper to an STL map, in order to make
 ** it usable in a TTree.
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 24.11.2015
 **/
class CbmMCEventList: public TNamed
{

  public:

    /** @brief Standard constructor **/
    CbmMCEventList();


    /** @brief Destructor **/
    virtual ~CbmMCEventList();


    /** @brief Delete all event entries **/
    virtual void Clear(Option_t*) {
      fEvents.clear();
    }


    /** @brief Event start time
     ** @param event  MC event number
     ** @param file   MC input file number
     ** @value MC event start time [ns]
     **
     ** Returns -1. if the event is not present in the list.
     **/
    Double_t GetEventTime(Int_t event, Int_t file = 0) const;


    /** @brief Number of events in the list
     ** @value Number of events
     **/
    Int_t GetNofEvents() const;


    /** Insert an event with its start time into the event list
     ** @param event  MC event number
     ** @param file   MC input file number
     ** @param time   MC event start time [ns]
     */
    void Insert(Int_t event, Int_t file, Double_t time) {
      fEvents[file][event] = time;
    }


    /** Status to string **/
    std::string ToString() const;


  private:

    /** Event container. Implemented as std::map:
     ** Input file number -> ( event number -> event time )
     **/
    std::map<Int_t, std::map<Int_t, Double_t>> fEvents;


    ClassDef(CbmMCEventList, 2)
    ;
};

#endif /* CBMMCEVENTLIST_H */

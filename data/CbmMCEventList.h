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

class CbmMCEventList: public TObject
{

  public:

    CbmMCEventList();

    virtual ~CbmMCEventList();

    virtual void Clear(Option_t* opt = "") {
      fEvents.clear();
    }

    Double_t GetEventTime(Int_t event, Int_t file) {
      pair<Int_t, Int_t> a(event, file);
      if ( fEvents.find(a) == fEvents.end() ) return -1.;
      return fEvents[a];
    }

    void Insert(Int_t event, Int_t file, Double_t time) {
      pair<Int_t, Int_t> a(event, file);
      fEvents[a] = time;
    }

  private:

    map<pair<Int_t, Int_t>, Double_t > fEvents;

    ClassDef(CbmMCEventList, 1);
};

#endif /* CBMMCEVENTLIST_H */

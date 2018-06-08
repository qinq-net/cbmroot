/**
 * @file
 * @author Christian Simon <csimon@physi.uni-heidelberg.de>
 * @since 2018-05-31
 */

#ifndef CBMTOFBUILDDIGIEVENTS_H
#define CBMTOFBUILDDIGIEVENTS_H 1


#include "FairTask.h"

#include <tuple>
#include <map>
#include <set>
#include <vector>


class FairFileSource;
class CbmTimeSlice;
class TClonesArray;
class CbmTofDigiExp;

/**
 * @brief ...
 * @author Christian Simon <csimon@physi.uni-heidelberg.de>
 * @since 2018-05-31
 * @version 1.0
 * @details ...
 */
class CbmTofBuildDigiEvents: public FairTask
{
	public:

		CbmTofBuildDigiEvents();

		virtual ~CbmTofBuildDigiEvents();

    virtual void Exec(Option_t *option);

//    virtual void FinishEvent();

    void SetEventWindow(Double_t dWindow) {fdEventWindow = dWindow;}
    void SetTriggerCounter(Int_t iModuleType, Int_t iModuleIndex, Int_t iCounterIndex, Int_t iNCounterSides);
    void SetTriggerMultiplicity(Int_t iMultiplicity) {fiTriggerMultiplicity = iMultiplicity;}
    void SetPreserveMCBacklinks(Bool_t bPreserve) {fbPreserveMCBacklinks = bPreserve;}


  protected:

    virtual InitStatus Init();

//    virtual void SetParContainers();

    virtual void Finish();


	private:

    CbmTofBuildDigiEvents(const CbmTofBuildDigiEvents&);

    CbmTofBuildDigiEvents& operator=(const CbmTofBuildDigiEvents&);

    void ProcessIdealEvents(Double_t dProcessingTime);

    FairFileSource* fFileSource;
    CbmTimeSlice* fTimeSliceHeader;
		TClonesArray* fTofTimeSliceDigis;
    TClonesArray* fTofEventDigis;
    Double_t fdEventWindow;
    std::map<std::tuple<Int_t, Int_t, Int_t>, UChar_t> fNominalTriggerCounterMultiplicity;
    Int_t fiTriggerMultiplicity;
    Bool_t fbPreserveMCBacklinks;
    Bool_t fbMCEventBuilding;
    Double_t fdEventStartTime;
    std::map<std::tuple<Int_t, Int_t, Int_t>, UChar_t> fCounterMultiplicity;
    Double_t fdIdealEventWindow;
    std::set<std::pair<Int_t, Int_t>> fProcessedIdealEvents;
    std::map<std::pair<Int_t, Int_t>, Double_t> fIdealEventStartTimes;
    std::map<std::pair<Int_t, Int_t>, std::vector<CbmTofDigiExp*>> fIdealEventDigis;


		ClassDef(CbmTofBuildDigiEvents, 0);
};

#endif

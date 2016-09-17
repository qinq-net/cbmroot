/** @file CbmBuildEventsIdeal.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.09.2016
 **/
#ifndef CBMBUILDEVENTSIDEAL_H_
#define CBMBUILDEVENTSIDEAL_H 1


#include <FairTask.h>

class TClonesArray;

/** @class CbmStsBuildEventsIdeal
 ** @brief Task class for associating digis to events
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 17.09.2016
 ** @version 1.0
 **
 **/
class CbmBuildEventsIdeal: public FairTask {

	public:
		CbmBuildEventsIdeal();
		virtual ~CbmBuildEventsIdeal();

		virtual void Exec(Option_t* opt);


	private:


		TClonesArray* fStsDigis;   ///< Input array (class CbmStsDigi)
		TClonesArray* fEvents;     ///< Output array (class CbmEvent)

		Int_t fNofEntries;         ///< Number of processed entries


		virtual InitStatus Init();

		ClassDef(CbmBuildEventsIdeal, 1);

};

#endif /* CBMBUILDEVENTSIDEAL_H */

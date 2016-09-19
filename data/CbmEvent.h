/** @file CbmEvent.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.09.2016
 **/


#ifndef CBMEVENT_H
#define CBMEVENT_H 1

#include <map>
#include <string>
#include <vector>
#include <TObject.h>
#include "CbmDetectorList.h"



/** @class CbmEvent
 ** @brief Class characterising one event by a collection of links (indices)
 ** to data objects,
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 **/

class CbmEvent: public TObject {

	public:

		/** Default constructor **/
		CbmEvent() :
			TObject(), fNumber(-1), fTime(0.), fNofData(0), fIndexMap() {
		}


		/** Constructor with event number and time
		 ** @param number  Event number
		 ** @param time    Event start time [ns]
		 **/
		CbmEvent(Int_t number, Double_t time = 0.) :
			TObject(), fNumber(number), fTime(time) , fNofData(0), fIndexMap() {
		}


		/** Destructor **/
		virtual ~CbmEvent() {
		}


		/** Add a data object to the index map
		 ** @param DataType  Type of data (for values see CbmDetectorList.h)
		 ** @param Index     Index of the data object in its TClonesArray
		 */
		void AddData(Cbm::DataType type, Int_t index);


		/** Get the index of a data object in its TClonesArray
		 ** @param DataType  Type of data (for values see CbmDetectorList.h)
		 ** @param iData     Running number of data object in event
		 ** @value Index of data object in its TClonesArray
		 **/
		Int_t GetIndex(Cbm::DataType type, UInt_t iData);


		/** Get total number of data (of all types) in the event **/
		Int_t GetNofData() { return fNofData; }


		/** Get number of data objects of a given type in this event
		 ** @param DataType  Type of data (for values see CbmDetectorList.h)
		 ** @value Number of objects of type DataType in the event.
		 ** -1 is data type is not registered.
		 **/
		Int_t GetNofData(Cbm::DataType type);


	  /** String output **/
	  std::string ToString() const;



	private:

		/** Event metadata **/
		Int_t fNumber;   ///< Event number
		Double_t fTime;  ///< Event time [ns]
		Int_t fNofData;  ///< Number of data objects in the event

		/** Arrays of indices to data types **/
		std::map<Cbm::DataType, std::vector<UShort_t>> fIndexMap;


		ClassDef(CbmEvent, 1);
};

#endif /* CBMEVENT_H_ */

/** @file CbmStsStation_new.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 4.03.2015
 **/

#ifndef CBMSTSSTATION_NEW_H
#define CBMSTSSTATION_NEW_H 1


/** @class CbmStsStation_new
 ** @brief Class representing a station of the StsSystem.
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** The StsStation class represents one level in the STS element hierarchy.
 ** It holds station-wide parameters like z position, sensor thickness,
 ** and material budget for use in reconstruction tasks.
 **/

class CbmStsStation_new : public CbmStsElement {

	public:

		/** Default constructor **/
		CbmStsStation_new();


		/** Standard constructor
		 ** @param name   Station name
		 ** @param title  Station type
		 ** @param node   Pointer to corresponding TGeoPhysicalNode
		 */
		CbmStsStation_new(const char* name, const char* title,
						TGeoPhysicalNode* node = NULL);


		/** Destructor **/
		virtual ~CbmStsStation_new();


		/** Station z position in global c.s. **/
		Double_t GetZ() const { return fZ; }


	 /** @brief Initialise the station parameters **/
		void Init();




	private:

		Double_t fZ;  ///< z position of station (mean of min. and. max. z of sensors)

		/** @brief Initialise the z position
		 ** The z position of the station is determined as the mean of the minimum
		 ** and maximum z positions of all sensors within the station.
		 **/
		void InitPosition();

		ClassDef(CbmStsStation_new, 1);
};

#endif /* CBMSTSSTATION_NEW_H_ */

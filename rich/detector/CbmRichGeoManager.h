/*
 * CbmRichGeoManager.h
 *
 *  Created on: Dec 16, 2015
 *      Author: slebedev
 */

#include "CbmRichRecGeoPar.h"
#include "TVector3.h"

#ifndef RICH_CBMRICHGEOMANAGER_H_
#define RICH_CBMRICHGEOMANAGER_H_

class CbmRichGeoManager {

private:


public:
	CbmRichRecGeoPar* fGP;

public:
	/**
	 * Return Instance of CbmRichGeoManager.
	 */
	static CbmRichGeoManager& GetInstance() {
		static CbmRichGeoManager fInstance;
		return fInstance;
	}

	/**
	* Rotate points by
	* -theta, -phi for x>0, y>0
	* theta, -phi for x>0, y<0
	* theta,  phi for x<0, y<0
	* -theta,  phi for x<0, y>0
	* and shift x position in order to avoid overlap.
	* \param[in] inPos points position to be tilted.
	* \param[out] outPos point position after tilting.
	* \param[in] noTilting If you do not want to make tilting, needed for convenience.
	*/
	void RotatePoint(
		 TVector3 *inPos,
		 TVector3 *outPos,
		 Bool_t noTilting = false);

private:

	/**
	 * private Constructor.
	 */
	CbmRichGeoManager();

	/**
	 * Init geometry.
	 */
	void InitGeometry();

	/**
	* \brief Copy constructor.
	*/
	CbmRichGeoManager(const CbmRichGeoManager&);

	/**
	* \brief Assignment operator.
	*/
	CbmRichGeoManager& operator=(const CbmRichGeoManager&);
};

#endif /* RICH_CBMRICHGEOMANAGER_H_ */

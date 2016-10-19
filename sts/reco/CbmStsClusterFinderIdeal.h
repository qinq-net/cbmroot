/** @file CbmStsClusterFinderIdeal.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.10.2016
 **/

#ifndef CBMSTSCLUSTERFINDERIDEAL_H
#define CBMSTSCLUSTERFINDERIDEAL_H 1

#include "CbmStsClusterFinder.h"

class TClonesArray;
class CbmStsModule;


/** @class CbmStsClusterFinderIdeal
 ** @brief MC-based cluster finding in STS
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** The ideal cluster finder groups digis belonging to the same MCPoint
 ** obtained from the digi match object into clusters, separately for the
 ** front and back side of the module/sensor.
 ** For digis with multiple links to MCPoints, the link with the largest
 ** weight is taken.
 **/
class CbmStsClusterFinderIdeal: public CbmStsClusterFinder {

	public:

		/** Constructor
		 ** @param clusterçrray  Output array for CbmStsCluster objects
		 **/
		CbmStsClusterFinderIdeal(TClonesArray* clusterArray = NULL);


		/** Destructor **/
		virtual ~CbmStsClusterFinderIdeal() { };


		/** Algorithm implementation
		 ** @param module  Pointer to CbmStsModule to be operated on
		 */
		virtual Int_t FindClusters(CbmStsModule* module);


		ClassDef(CbmStsClusterFinderIdeal, 1);

};

#endif /* CBMSTSCLUSTERFINDERIDEAL_H */

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
 **
 ** The ideal cluster finder gives a lower number of clusters compared
 ** to the old implementation in CbmStsClusterFinderSimple, because it
 ** also clusters strips "around the corner" on the stereo side.
 ** For time-based input, the number of clusters is slightly higher than
 ** for event-based input. This is due to the fact that double-hits can be
 ** resolved by time in the time-based mode.
 **
 ** TODO: In principle, double hits can be restored using the MC information
 ** in the digi match object.
 **/
class CbmStsClusterFinderIdeal: public CbmStsClusterFinder {

	public:

		/** Constructor
		 ** @param clusterArray  Output array for CbmStsCluster objects
		 **/
		CbmStsClusterFinderIdeal(TClonesArray* clusterArray = NULL);


		/** Destructor **/
		virtual ~CbmStsClusterFinderIdeal() { };


		/** Algorithm implementation
		 ** @param module  Pointer to CbmStsModule to be operated on
		 ** @param event   Pointer to current event
		 */
		virtual Int_t FindClusters(CbmStsModule* module, CbmEvent* event = NULL);


		ClassDef(CbmStsClusterFinderIdeal, 1);

};

#endif /* CBMSTSCLUSTERFINDERIDEAL_H */

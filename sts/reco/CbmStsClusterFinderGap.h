/** @file CbmStsClusterFinderGap.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 19.10.2016
 **/

#ifndef CBMSTSCLUSTERFINDERGAP_H
#define CBMSTSCLUSTERFINDERGAP_H 1

#include "CbmStsClusterFinder.h"

class TClonesArray;
class CbmStsModule;


/** @class CbmStsClusterFinderGap
 ** @brief Cluster finding in STS with gaps for inactive channels
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** The gap cluster finder groups digis belonging to neighbouring
 ** channels within a module into a cluster. Two channels are also
 ** considered neighbouring if they are separated by an inactive channel.
 **/
class CbmStsClusterFinderGap: public CbmStsClusterFinder {

	public:

		/** Constructor
		 ** @param clusterArray  Output array for CbmStsCluster objects
		 **/
		CbmStsClusterFinderGap(TClonesArray* clusterArray = NULL);


		/** Destructor **/
		virtual ~CbmStsClusterFinderGap() { };


		/** Algorithm implementation
		 ** @param module  Pointer to CbmStsModule to be operated on
		 ** @param event   Pointer to current event
		 */
		virtual Int_t FindClusters(CbmStsModule* module, CbmEvent* event = NULL);

	private:

		/** Create a cluster in the output array
		 ** @param clusterStart  First channel of cluster
		 ** @param clusterEnd    Last channel of cluster
		 ** @param module        Pointer to CbmStsModule
		 ** @param event         Pointer to current event
		 **/
		void CreateCluster(Int_t clusterStart, Int_t clusterEnd,
				               CbmStsModule* module, CbmEvent* event = NULL);


		ClassDef(CbmStsClusterFinderGap, 1);

};

#endif /* CBMSTSCLUSTERFINDERREAL_H */

/** @file CbmStsClusterFinderReal.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 19.10.2016
 **/

#ifndef CBMSTSCLUSTERFINDERREAL_H
#define CBMSTSCLUSTERFINDERREAL_H 1

#include "CbmStsClusterFinder.h"

class TClonesArray;
class CbmStsModule;


/** @class CbmStsClusterFinderReal
 ** @brief Simple cluster finding in STS
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** The real cluster finder groups digis belonging to neighbouring
 ** channels within a module into a cluster,
 **/
class CbmStsClusterFinderReal: public CbmStsClusterFinder {

	public:

		/** Constructor
		 ** @param clusterçrray  Output array for CbmStsCluster objects
		 **/
		CbmStsClusterFinderReal(TClonesArray* clusterArray = NULL);


		/** Destructor **/
		virtual ~CbmStsClusterFinderReal() { };


		/** Algorithm implementation
		 ** @param module  Pointer to CbmStsModule to be operated on
		 */
		virtual Int_t FindClusters(CbmStsModule* module);

	private:

		/** Create a cluster in the output array
		 ** @param clusterStart  First channel of cluster
		 ** @param clusterEnd    Last channel of cluster
		 ** @param module        Pointer to CbmStsModule
		 **/
		void CreateCluster(Int_t clusterStart, Int_t clusterEnd,
				               CbmStsModule* module);


		ClassDef(CbmStsClusterFinderReal, 1);

};

#endif /* CBMSTSCLUSTERFINDERREAL_H */

/** @file CbmStsClusterFinder.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.10.2016
 **/

#ifndef CBMSTSCLUSTERFINDER_H
#define CBMSTSCLUSTERFINDER_H 1

#include "TNamed.h"

class TClonesArray;
class CbmStsCluster;
class CbmEvent;
class CbmStsModule;


/** @class CbmStsClusterFinder
 ** @brief Abstract base class for STS cluster finding algorithms
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** Cluster finding means grouping digis supposedly originating from the
 ** same charged track (CbmStsPoint). In the STS modules, this means
 ** clusters are made of digis from neighbouring channels.
 **
 ** A concrete algorithm has to implement the abstract method FindClusters.
 ** This method operates on a CbmStsModule, which holds a list of its digis.
 ** The algorithm has to create objects of type CbmStsCluster and write them
 ** to the output TClonesArray.
 **/
class CbmStsClusterFinder: public TNamed {

	public:

		/** Constructor
		 ** @param clusterArray  Output array for CbmStsCluster objects
		 **/
		CbmStsClusterFinder(TClonesArray* clusterArray = NULL);


                CbmStsClusterFinder(const CbmStsClusterFinder&) = delete;
                CbmStsClusterFinder& operator=(const CbmStsClusterFinder&) = delete;

		/** Destructor **/
		virtual ~CbmStsClusterFinder() { };


		/** Algorithm implementation
		 ** @param module  Pointer to CbmStsModule to be operated on
		 ** @param event   Pointer to current event
		 */
		virtual Int_t FindClusters(CbmStsModule* module, CbmEvent* = NULL) = 0;


	protected:

		TClonesArray* fClusters;  //!  Output array for clusters


		ClassDef(CbmStsClusterFinder, 1);

};

#endif /* CBMSTSCLUSTERFINDER_H */

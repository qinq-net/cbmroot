/** @file CbmStsClusterFinder.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.10.2016
 **/

#include "CbmStsClusterFinder.h"

#include "CbmStsCluster.h"

// -----   Constructor   ----------------------------------------------------
CbmStsClusterFinder::CbmStsClusterFinder(TClonesArray* clusterArray)
	: TNamed("StsClusterFinder", "Base"), fClusters(clusterArray) {
}
// --------------------------------------------------------------------------



ClassImp(CbmStsClusterFinder)

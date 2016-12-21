/**
 * \file CbmCluster.cxx
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2012
 */

#include "CbmCluster.h"
#include <sstream>

using namespace std;

CbmCluster::CbmCluster()
 : TObject(),
   fDigis(),
   fAddress(0),
   fMatch(NULL)
{

}
CbmCluster::CbmCluster(const std::vector<Int_t>& indices, Int_t address)
 : TObject(),
   fDigis(),
   fAddress(address),
   fMatch(NULL)
{
 fDigis.assign(indices.begin(), indices.end());
}

CbmCluster::~CbmCluster()
{

}

void CbmCluster::SetMatch(CbmMatch* match) {
	 if ( fMatch ) delete fMatch;
	 fMatch = match;
}


string CbmCluster::ToString() const
{
   stringstream ss;
   ss << "CbmCluster: ";
   Int_t nofDigis = GetNofDigis();
   ss << "nofDigis=" << nofDigis << " | ";
   for (Int_t i = 0; i < nofDigis; i++) {
      ss << fDigis[i] << " ";
   }
   ss << " | address=" << fAddress << endl;
   return ss.str();
}

ClassImp(CbmCluster);

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "GeoReader.h"

CbmBinnedGeoReader* CbmBinnedGeoReader::fInstance = 0;

CbmBinnedGeoReader* CbmBinnedGeoReader::Instance()
{
   if (0 != fInstance)
      return fInstance;
   
   FairRootManager* ioman = FairRootManager::Instance();
   
   if (0 == ioman)
      return 0;
   
   fInstance = new CbmBinnedGeoReader;
   fInstance->fIoman = ioman;
   return fInstance;
}

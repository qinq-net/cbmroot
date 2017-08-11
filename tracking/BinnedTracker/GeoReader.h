/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GeoReader.h
 * Author: tablyaz
 *
 * Created on August 11, 2017, 5:34 PM
 */

#ifndef GEOREADER_H
#define GEOREADER_H

#include "FairRootManager.h"


class CbmBinnedGeoReader
{
public:
    static CbmBinnedGeoReader* Instance();
    
public:
    
private:
    static CbmBinnedGeoReader* fInstance;
    
private:
    FairRootManager* fIoman;
};

#endif /* GEOREADER_H */


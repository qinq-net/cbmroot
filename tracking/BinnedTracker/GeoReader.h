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

#include <list>
#include <map>
#include <string>
#include "TGeoManager.h"
#include "FairRootManager.h"

class CbmBinnedGeoReader
{
public:
    static CbmBinnedGeoReader* Instance();
    
public:
    CbmBinnedGeoReader();
    CbmBinnedGeoReader(const CbmBinnedGeoReader&) = delete;
    CbmBinnedGeoReader& operator=(const CbmBinnedGeoReader&) = delete;
    void FindGeoChild(TGeoNode* node, const char* name, std::list<TGeoNode*>& results);
    void Read();
    
private:
    void ReadDetector(const char* name);
    void SearchStation(TGeoNode* node, std::list<const char*>::const_iterator stationPath, std::list<const char*>::const_iterator stationPathEnd,
        const std::list<const char*>& geoPath);
    void HandleStation(TGeoNode* node, std::list<const char*>::const_iterator geoPath, std::list<const char*>::const_iterator geoPathEnd);
    void HandleActive(TGeoNode* node, Double_t& left, Double_t& right, Double_t& top, Double_t& bottom);
    void ReadSts();
    void ReadRich();
    void ReadMuch();
    void ReadTrd();
    void ReadTof();
    
private:
    static CbmBinnedGeoReader* fInstance;
    
private:
    FairRootManager* fIoman;
    TGeoNavigator* fNavigator;
    std::map<std::string, void (CbmBinnedGeoReader::*)()> fDetectorReaders;
};

#endif /* GEOREADER_H */


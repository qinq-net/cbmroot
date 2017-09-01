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
#include "Tracker.h"
#include "HitReader.h"

class CbmBinnedTracker;

class CbmBinnedGeoReader
{
public:
    static CbmBinnedGeoReader* Instance();
    
public:
    CbmBinnedGeoReader(FairRootManager* ioman, CbmBinnedTracker* tracker);
    CbmBinnedGeoReader(const CbmBinnedGeoReader&) = delete;
    CbmBinnedGeoReader& operator=(const CbmBinnedGeoReader&) = delete;
    void FindGeoChild(TGeoNode* node, const char* name, std::list<TGeoNode*>& results);
    void Read();
    
private:
    void ReadDetector(const char* name);
    void SearchStation(TGeoNode* node, CbmBinnedHitReader* hitReader, std::list<const char*>::const_iterator stationPath, std::list<const char*>::const_iterator stationPathEnd,
        const std::list<const char*>& geoPath, bool is4d = false);
    void HandleStation(TGeoNode* node, std::list<const char*>::const_iterator geoPath, std::list<const char*>::const_iterator geoPathEnd,
        Double_t& left, Double_t& right, Double_t& top, Double_t& bottom, Double_t& front, Double_t& back);
    void HandleActive(TGeoNode* node, Double_t& left, Double_t& right, Double_t& top, Double_t& bottom, Double_t& front, Double_t& back);
    void ReadSts();
    void ReadRich();
    void ReadMuch();
    void ReadTrd();
    void ReadTof();
    void ReadTarget();
    void FindNode(std::list<const char*> nodePath, std::list<TGeoNode*>& results, TGeoNode* rootNode = 0);
    
private:
    FairRootManager* fIoman;
    TGeoNavigator* fNavigator;
    std::map<std::string, void (CbmBinnedGeoReader::*)()> fDetectorReaders;
    CbmBinnedTracker* fTracker;
};

#endif /* GEOREADER_H */


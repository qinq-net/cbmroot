/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CbmBinnedTrackerQA.h
 * Author: tablyaz
 *
 * Created on August 24, 2017, 6:21 PM
 */

#ifndef CBMBINNEDTRACKERQA_H
#define CBMBINNEDTRACKERQA_H

#include "FairTask.h"

class CbmBinnedTrackerQA : public FairTask
{
public:
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
private:
    ClassDef(CbmBinnedTrackerQA, 1)
};

#endif /* CBMBINNEDTRACKERQA_H */


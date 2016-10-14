/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LxTBMLTask.h
 * Author: tablyaz
 *
 * Created on October 14, 2016, 11:35 AM
 */

#ifndef LXTBMLTASK_H
#define LXTBMLTASK_H

#include "FairTask.h"

class LxTBMLFinder : public FairTask
{
public:
    LxTBMLFinder();
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
private:
    ClassDef(LxTBMLFinder, 1)
};

#endif /* LXTBMLTASK_H */


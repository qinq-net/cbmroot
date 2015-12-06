
/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

/*
 * File:   CbmClusterFinderTask.h
 * Author: winckler
 *
 * Created on November 12, 2015, 1:31 PM
 */

#ifndef CBMCLUSTERFINDERTASK_H
#define	CBMCLUSTERFINDERTASK_H

#include "CbmTimeSlice.h"
#include "BaseProcessorTaskPolicy.h"
#include "FairMQLogger.h"

template<typename T>
class CbmClusterFinderTask : public BaseProcessorTaskPolicy<CbmClusterFinderTask<T> >
{
  protected: 

    typedef T cluster_finder;
    typedef T* cluster_finder_ptr;

  public:
    CbmClusterFinderTask() : 
        BaseProcessorTaskPolicy<CbmClusterFinderTask<T> >(), 
        fClusterFinder(nullptr),
        fContainer(nullptr)
    {
        fClusterFinder = new cluster_finder();
    }

    CbmClusterFinderTask(const CbmClusterFinderTask&);
    CbmClusterFinderTask& operator=(const CbmClusterFinderTask&);

    virtual ~CbmClusterFinderTask()
    {
        if(fClusterFinder)
        {
            delete fClusterFinder;
            fClusterFinder=nullptr;
        }
    }

    void InitClusterFinderTask(const std::string& filename)
    {
        fClusterFinder->UseTbClusterFinder();
        fClusterFinder->UseDaq();
        fClusterFinder->InitMQ(filename);
    }

  protected:


    void ExecuteTask(CbmTimeSlice& ts)
    {
        fClusterFinder->SetTimeSlice(&ts);
        fClusterFinder->ExecMQ();
        fContainer = fClusterFinder->GetClusters();
        MQLOG(TRACE)<<"ExecuteTask : TClonesarray size = " <<fContainer->GetEntries();
    }

    TClonesArray* GetOutputData()
    {
        return fContainer;
    }


    cluster_finder_ptr fClusterFinder;// task
    TClonesArray* fContainer;// output

};


#endif	/* CBMCLUSTERFINDERTASK_H */


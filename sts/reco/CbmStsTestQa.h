/*
 * CbmStsTestQa.h
 *
 *  Created on: 07.11.2016
 *      Author: vfriese
 */

#ifndef CBMSTSTESTQA_H_
#define CBMSTSTESTQA_H 1

#include <FairTask.h>

class CbmEvent;
class CbmVertex;
class CbmHistManager;
class TClonesArray;
class fstream;

class CbmStsTestQa: public FairTask {

public:
	CbmStsTestQa();

	virtual ~CbmStsTestQa();


    /** Task execution **/
    virtual void Exec(Option_t* opt);


    /** End-of-run action **/
    virtual void Finish();


    /** End-of-event action **/
    virtual void FinishEvent() { };


    /** Initialisation **/
    virtual InitStatus Init();


private:

    TClonesArray* fEvents;   //!
    TClonesArray* fClusters;  //!
    TClonesArray* fHits;  //!
    TClonesArray* fTracks;  //!
    CbmVertex* fVertex; //!

    CbmHistManager* fHistMan;
    ofstream* fFileClusters; //!
    ofstream* fFileHits; //!
    ofstream* fFileTracks; //!
    ofstream* fFileVertices; //!

    void ProcessEvent(CbmEvent* event = NULL);

    ClassDef(CbmStsTestQa, 1);


};

#endif /* CBMSTSTESTQA_H */

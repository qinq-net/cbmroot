#include "CbmTutorial2DetContFact.h"

#include "CbmTutorial2DetGeoPar.h"
#include "CbmTutorial2DetDigiPar.h"

#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(CbmTutorial2DetContFact)

static CbmTutorial2DetContFact gCbmTutorial2DetContFact;

CbmTutorial2DetContFact::CbmTutorial2DetContFact() {
  /** Constructor (called when the library is loaded) */
  fName="CbmTutorial2DetContFact";
  fTitle="Factory for parameter containers in libTutorial21";
  setAllContainers();
  FairRuntimeDb::instance()->addContFactory(this);
}

void CbmTutorial2DetContFact::setAllContainers() {
  /** Creates the Container objects with all accepted 
      contexts and adds them to
      the list of containers for the Tutorial21 library.
  */

  FairContainer* p1= new FairContainer("CbmTutorial2DetGeoPar",
				     "CbmTutorial2Det Geometry Parameters",
				     "TestDefaultContext");
  p1->addContext("TestNonDefaultContext");
  
  containers->Add(p1);

  FairContainer* p2= new FairContainer("CbmTutorial2DetDigiPar",
				     "Tutorial2 Det Digi Parameters",
				     "TestDefaultContext");
  p2->addContext("TestNonDefaultContext");

  containers->Add(p2);
}

FairParSet* CbmTutorial2DetContFact::createContainer(FairContainer* c) {
  /** Calls the constructor of the corresponding parameter container.
      For an actual context, which is not an empty string and not 
      the default context
      of this container, the name is concatinated with the context. 
  */
  const char* name=c->GetName();
  FairParSet* p=NULL;
  if (strcmp(name,"CbmTutorial2DetGeoPar")==0) {
    p=new CbmTutorial2DetGeoPar(c->getConcatName().Data(),
			       c->GetTitle(),c->getContext());
  }
  if (strcmp(name,"CbmTutorial2DetDigiPar")==0) {
    p=new CbmTutorial2DetDigiPar(c->getConcatName().Data(),
			       c->GetTitle(),c->getContext());
  }
  return p;
}

#ifndef ROC_FILEINPUT_H
#define ROC_FILEINPUT_H

/* *** PAL edit: Run offline with LMD file and without DABC *** 
#include "roc/UdpBoard.h"
   ***************** */
#include "roc/Board.h"

#ifndef MBS_LmdInput
#include "mbs/LmdInput.h"
#endif

#ifndef MBS_MbsTypeDefs
#include "mbs/MbsTypeDefs.h"
#endif

namespace roc {

   class FileInput : public Board {
      protected:
         mbs::LmdInput* fInput;

         mbs::EventHeader* fHdr;
         mbs::SubeventHeader* fSubev;

         int fSelectRoc;

      public:
         FileInput(mbs::LmdInput* inp, int selectroc = -1);

         virtual ~FileInput();

         virtual int getTransportKind() const;
         
         virtual void setFlushTimeout(double) {}

         virtual int operGen(OperList&, double = 0.) { return 0; }

         // daq interface part of roc::Board
         virtual bool startDaq() { return true; }
         virtual bool suspendDaq()  { return true; }
         virtual bool stopDaq()  { return true; }

         virtual bool getNextBuffer(void* &buf, unsigned& len, double tmout = 1.);

         virtual MessageFormat getMsgFormat() const;
         
      private:
         FileInput(const FileInput&);
         FileInput& operator=(const FileInput&);
   };
}


#endif

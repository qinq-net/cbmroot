// C/C++ standard library headers
#include <sstream>
#include <stdint.h>
#include <vector>
#include <ctime>

// FLES control LIB main header
#include "fles/ctrl/control/ControlProtocol.hpp"


// Generic ROC + FLIB registers: GPIO
#include "fles/nxyter_ctrl/defines_gpio.h"

// NX + ROC registers: NX I2C, NX SPI
//#include "roc/roclib/roc/defines_i2c.h"
//#include "fles/nxyter_ctrl/defines_spi.h"

// GET4 v1.x ROC registers: generic ROC, generic OPTICS, GET4
#include "fles/get4_ctrl/defines_roc_get4v1x.h"

// GET4 v1.x internal registers address
#include "fles/get4_ctrl/defines_get4v1x.h"

// GET4 v1.x ROC message tool functions => maybe already loaded from libCbmFlibReader.so?
#include "fles/reader/unpacker/CbmGet4v1xHackDef.h"

// Symbols from ControlClient.hpp are in libcbmnetcntlclientroot.so, loaded in rootlogon.C
// This is to let your IDE know about CbmNet::ControlClient class, and thus make
// autocompletion possible
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "fles/ctrl/control/libclient/ControlClient.hpp"
#endif

using std::ostringstream;

const long klReceiveMaskLsbsRoc1 = 0xFFFFFFFF;
const long klReceiveMaskMsbsRoc1 = 0x00000000;

/*
 * ROC Generator lab 2015 initial configuration
 * int mode = readout mode to configure
 *            * 0 => default 24b mode settings
 *            * 1 => default 32b mode settings
 *            * 2 => tbd, read config from a text/xml file
 * uint uNbMess = nb of messages printed from control interface for each ROC
 */
void config_rocgen( int mode, uint32_t uNbMess = 800  )
{
   // Custom settings:
   const uint32_t kNodeId  = 0;
   const uint32_t kuNbRocs = 2;
//uRoc = number of Rocs in use
   for( uint32_t uRoc = 0; uRoc < kuNbRocs; uRoc++ )
   {
      CbmNet::ControlClient conn;
      ostringstream dpath;
      int FlibLink = uRoc;

      dpath << "tcp://" << "localhost" << ":" << CbmNet::kPortControl + FlibLink;
      conn.Connect(dpath.str());

      // Printout header
      printf("++++++++++++++++++++++++++++++++++++++++++\n" );
      printf("Now configuring ROC #%05d\n", uRoc );

      // Check board and firmware info
      uint32_t ret;
      conn.Read( kNodeId, ROC_TYPE, ret );
      printf("Firmware type    = FE %5d TS %5d\n", (ret>>16)& 0xFFFF,  (ret)& 0xFFFF);
      conn.Read( kNodeId, ROC_HWV, ret );
      printf("Firmware Version = %10d \n", ret);
      conn.Read( kNodeId, ROC_FPGA_TYPE, ret );
      printf("FPGA type        = %10d \n", ret);
      conn.Read( kNodeId, ROC_SVN_REVISION, ret );
      printf("svn revision     = %10d \n", ret);
      if(0 == ret )
      {
        printf("Invalid svn revision, link or ROC is probably inactive, stopping there\n");
        return;
      } // if(0 == ret )

      conn.Read( kNodeId, ROC_BUILD_TIME, ret );
      printf("build time       = %10u \n", ret);
      time_t rawtime = ret;
      struct tm * timeinfo;
      char buffer [20];
      timeinfo = localtime( &rawtime);
      strftime( buffer,20,"%F %T",timeinfo);
      printf("build time       = %s \n", buffer);
      printf("\n");
      printf("Setting ROCID to = %d \n", uRoc);
      conn.Write( kNodeId, ROC_ROCID, uRoc );
      SetRocDef( conn, kNodeId );

      switch( mode )
      {
        case 0:
          switch( uRoc )
          {
             case 0:
//                ConfigRoc0_24b( conn, kNodeId );
//                ConfigChannelsRoc0( conn, kNodeId );
//                ConfigCoreDelayRoc0( conn, kNodeId );
                break;
             case 1:
//                ConfigRoc1_24b( conn, kNodeId );
//                ConfigChannelsRoc1( conn, kNodeId );
//                ConfigCoreDelayRoc1( conn, kNodeId );
                break;
             default:
//                Set24bDef( conn, kNodeId );
//                SetRocGet4ChanEna( conn, kNodeId);
//                SetRocGet4CoreDelay( conn, kNodeId);
                break;
          } // switch( uRoc )
          break;
        case 1:
           switch( uRoc )
           {
              case 0:
//                 ConfigRoc0_32b( conn, kNodeId );
//                 ConfigChannelsRoc0( conn, kNodeId );
//                 ConfigCoreDelayRoc0( conn, kNodeId );
                 break;
              case 1:
//                 ConfigRoc1_32b( conn, kNodeId );
//                 ConfigChannelsRoc1( conn, kNodeId );
//                 ConfigCoreDelayRoc1( conn, kNodeId );
                 break;
              default:
//                 Set32bDef( conn, kNodeId );
//                 SetRocGet4ChanEna( conn, kNodeId);
//                 SetRocGet4CoreDelay( conn, kNodeId);
                 break;
           } // switch( uRoc )
          break;
        case 2:
        {
          std::cout<<" Configuration mode 2 not implemented yet ! "<<std::endl;
   CbmNet::ListSeq initList;

// ROC_EPOCH250_EN => Enable/Disable the production of 250MHz (ROC)
//                    epochs
   initList.AddWrite(ROC_EPOCH250_EN,                    0);

//# ROC_GET4_READOUT_MODE => 24 bits mode selection
   initList.AddWrite(ROC_GET4_READOUT_MODE,               0);

// ROC_GET4_RECEIVE_MASK_LSBS & ROC_GET4_RECEIVE_MASK_MSBS
//  => Lab Jun 2015: 3 FEE per ROC
//   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x00FFFFFF);
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, klReceiveMaskLsbsRoc1 );
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_MSBS, klReceiveMaskMsbsRoc1 );

// ROC_GET4_SUPRESS_EPOCHS_LSBS & ROC_GET4_SUPRESS_EPOCHS_MSBS
//  => Enable the suppression of the 156.25 MHz epoch without data messages
//  => Chip granularity setting, maybe interesting to keep at least one per ROC
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_LSBS, 0x00000000);
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_MSBS, 0x00000000);

/*
 * 32 bit, read/write.
 * The 32 bit register is used to unmask the outputs of the data generator.
 * If this register holds the value 0x 0000 0001, data will be available on date generator output 0.
 * A value of 0x FFFF FFFF can be used to generate data on all 24 outputs.
 * If ROC GET4 DATAGEN EN is set, these outputs will be connected to the data receivers instead of
 * data from the GET4s.
 */
   initList.AddWrite(ROC_GET4_DATAGEN_MASK, 0xFFFFFFFF);

/*
 * 32 bit, read/write.
 * The 32 bit register can be used to shape the data that is generated by the data generator.
 * When the data generator is enabled, the register is shifted left and the MSB is used as LSB (circular
 * shift register). So every 32 serial data clock cycles the pattern is repeated. The MSB is connected
 * to the (unmasked) outputs of the data generator.
 */
   initList.AddWrite(ROC_GET4_DATAGEN_INIT, 0xF000000F);

/*
 * 1 bit, writeonly.
 * This enables/disables the data generator.
 * When the data generator is enabled, ROC GET4 DATAGEN INIT and ROC GET4 DATAGEN MASK cannot be updated
 * but the old values are used until the data generator is disabled.
 */
   initList.AddWrite(ROC_GET4_DATAGEN_EN, 0x00000001);

// Define CMD-list 2 (Reset + Start-DAQ)
// ROC_STOP_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x00, ROC_CMD_LST_PUT + ROC_STOP_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x04, 0);
// ROC_GET4_TS156_RESET => Resets timestamp in ROC for 156.25 MHz clock
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x10, ROC_CMD_LST_PUT + ROC_GET4_TS156_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x14, 1);
// ROC_TS_RESET => Resets timestamp in ROC for 250.00 MHz clock
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x18, ROC_CMD_LST_PUT + ROC_TS_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x1C, 1);
// ROC_FIFO_RESET (active high?)
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x20, ROC_CMD_LST_PUT + ROC_FIFO_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x24, 1);
// ROC_FIFO_RESET
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x28, ROC_CMD_LST_PUT + ROC_FIFO_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x2C, 0);
//# ROC_ADDSYSMSG
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x30, ROC_CMD_LST_PUT + ROC_ADDSYSMSG);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x34, 0x00000010);
//# ROC_START_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x38, ROC_CMD_LST_PUT + ROC_START_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x3C, 0);
//# ROC_CMD_LST_ACTIVEGet4:0x13
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x40, ROC_CMD_LST_PUT + ROC_CMD_LST_ACTIVE );
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x44, 0);

   conn.DoListSeq(kNodeId, initList);
          }
          break;
        default:
          std::cout<<" Unknown configuration mode "<<std::endl;
          break;
      } // switch( mode )


      // StartDAQ => Use the command list 2 in order to also to make a clean start
      conn.Write( kNodeId, ROC_CMD_LST_NR, 2);

      // Check link status
      conn.Read( kNodeId, ROC_OPTICS_LINK_STATUS, ret );
      printf("ROC_OPTICS_LINK_STATUS  = %d\n", ret);

      // Read 300 messages for quick cross check of setting
      ReadMessages( conn, kNodeId, uNbMess );

      // Close connection
      conn.Close();

      // Printout header
      printf("Done configuring ROC #%05d\n", uRoc );
      printf("------------------------------------------\n" );
   } // for( uint32_t uRoc = 0; uRoc < kuNbRocs; uRoc++ )
}





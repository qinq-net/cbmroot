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


// Symbols from ControlClient.hpp are in libcbmnetcntlclientroot.so, loaded in rootlogon.C
// This is to let your IDE know about CbmNet::ControlClient class, and thus make
// autocompletion possible
#if !defined(__CINT__) || defined(__MAKECINT__)
#include "fles/ctrl/control/libclient/ControlClient.hpp"
#endif

using std::ostringstream;

int SendSpi(CbmNet::ControlClient & conn, uint32_t nodeid, uint32_t nbData, uint32_t* data)
{
   CbmNet::ListSeq initList;
   // Send GET4 command Config SPI
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SPI_CONF + 0x000000 );

   // Send SPI START
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SPI_START );

   // Send SPI DATA by blocks of 24 bits
   for( int iData = 0; iData < nbData; iData++ )
      initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SPI_DATA + ((data[iData]) & 0xFFFFFF) );

   // Send SPI STOP
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SPI_STOP );

   return conn.DoListSeq(nodeid, initList);
}

int SetRocDef(CbmNet::ControlClient & conn, uint32_t nodeid)
{
   CbmNet::ListSeq initList;
   // rocutil> ssm 2
   initList.AddWrite(base::GPIO_SYNCM_SCALEDOWN,        2);
   initList.AddWrite(base::GPIO_SYNCM_BAUD_START,       1);
   initList.AddWrite(base::GPIO_SYNCM_BAUD1,            3);
   initList.AddWrite(base::GPIO_SYNCM_BAUD2,            3);

   // rocutil> setsyncs 0,1 loop=1
   initList.AddWrite(base::GPIO_CONFIG,               900);
   initList.AddWrite(base::GPIO_SYNCS0_BAUD_START,      1);
   initList.AddWrite(base::GPIO_SYNCS0_BAUD1,           3);
   initList.AddWrite(base::GPIO_SYNCS0_BAUD2,           3);

   // rocutil> setaux 2,re=1,fe=0
   initList.AddWrite(base::GPIO_CONFIG,           5000900);
   return conn.DoListSeq(nodeid, initList);
}

int Set24bDef(CbmNet::ControlClient & conn, uint32_t nodeid)
{
   CbmNet::ListSeq initList;

//# ROC_GET4_RECEIVE_CLK_CFG => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_RECEIVE_CLK_CFG,           3);

//# ROC_GET4_RECEIVE_MASK_LSBS & ROC_GET4_RECEIVE_MASK_MSBS
//  => Activate only the 4 first chips
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x0000000F);
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_MSBS, 0x00000000);

//# ROC_GET4_SAMPLE_FALLING_EDGE_LSBS & ROC_GET4_SAMPLE_FALLING_EDGE_MSBS
//  => Change the edge on which the data from the GET4 are sampled
//  => Can be necessary with some ROC v2 systems 
//   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_LSBS, 0x00000000);
//   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_MSBS, 0x00000000);

//# ROC_GET4_SUPRESS_EPOCHS_LSBS & ROC_GET4_SUPRESS_EPOCHS_MSBS
//  => Enable the suppression of the 156.25 MHz epoch without data messages
//  => Chip granularity setting, maybe interesting to keep at least one per ROC 
//   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_LSBS, 0x00000000);
//   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_MSBS, 0x00000000);

//# ROC_GET4_READOUT_MODE => 24 bits mode selection
   initList.AddWrite(ROC_GET4_READOUT_MODE,               0);

//# Define CMD-list 2 (Reset + Start-DAQ)
//# ROC_STOP_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x00, ROC_CMD_LST_PUT + ROC_STOP_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x04, 0);
//# ROC_GET4_CMD_TO_GET4 => Initialize epoch counter to 0
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x08, ROC_CMD_LST_PUT + ROC_GET4_CMD_TO_GET4);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x0C, GET4V1X_EP_SYNC_INIT + 0x000000 );
//# ROC_GET4_TS156_RESET => Resets timestamp in ROC for 156.25 MHz clock
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x10, ROC_CMD_LST_PUT + ROC_GET4_TS156_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x14, 1);
//# ROC_TS_RESET => Resets timestamp in ROC for 250.00 MHz clock
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x18, ROC_CMD_LST_PUT + ROC_TS_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x1C, 1);
//# ROC_FIFO_RESET (active high?)
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x20, ROC_CMD_LST_PUT + ROC_FIFO_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x24, 1);
//# ROC_FIFO_RESET
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x28, ROC_CMD_LST_PUT + ROC_FIFO_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x2C, 0);
//# ROC_START_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x30, ROC_CMD_LST_PUT + ROC_START_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x34, 0);
//# ROC_CMD_LST_ACTIVE
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x38, ROC_CMD_LST_PUT + ROC_CMD_LST_ACTIVE );
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x3C, 0);

//# GET4 configuration
   //# ROC_GET4_CMD_TO_GET4 => Disable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK          + 0x00000F );

   //# ROC_GET4_CMD_TO_GET4 => Reset Readout and configuration
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_ROCONF_RST );

   //# ROC_GET4_CMD_TO_GET4 => Enable DLL Auto Reset
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_DLL_RST_AUTO +      0x1 );

   //# ROC_GET4_CMD_TO_GET4 => Enable 24b readout mode, disable LostEventError messages
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_RO_CONF_BASIC     +      0x0 );

   //# ROC_GET4_CMD_TO_GET4 => Select Readout 1 diagnostic for channel 1 Rising edge
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DIAG_SELECT       +     0x23 );

   //# ROC_GET4_CMD_TO_GET4 => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_24B_RO_CONF       +      0x7 );

   //# ROC_GET4_CMD_TO_GET4 => Re-Initialize the readout unit state machine
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_RO_INIT );

   //# ROC_GET4_CMD_TO_GET4 => Enable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK          + 0x000000 );

   return conn.DoListSeq(nodeid, initList);
}

int Set32bDef(CbmNet::ControlClient & conn, uint32_t nodeid)
{
   CbmNet::ListSeq initList;


// ROC_GET4_RECEIVE_CLK_CFG => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_RECEIVE_CLK_CFG,           3);

// ROC_GET4_RECEIVE_MASK_LSBS & ROC_GET4_RECEIVE_MASK_MSBS
//  => Activate only the 4 first chips
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x0000000F);
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_MSBS, 0x00000000);

// ROC_GET4_SAMPLE_FALLING_EDGE_LSBS & ROC_GET4_SAMPLE_FALLING_EDGE_MSBS
//  => Change the edge on which the data from the GET4 are sampled
//  => Can be necessary with some ROC v2 systems 
//   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_LSBS, 0x00000000);
//   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_MSBS, 0x00000000);

// ROC_GET4_SUPRESS_EPOCHS_LSBS & ROC_GET4_SUPRESS_EPOCHS_MSBS
//  => Enable the suppression of the 156.25 MHz epoch without data messages
//  => Chip granularity setting, maybe interesting to keep at least one per ROC 
//   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_LSBS, 0x00000000);
//   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_MSBS, 0x00000000);

//# ROC_GET4_READOUT_MODE => 32 bits mode selection
   initList.AddWrite(ROC_GET4_READOUT_MODE,               1);

// Define CMD-list 2 (Reset + Start-DAQ)
// ROC_STOP_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x00, ROC_CMD_LST_PUT + ROC_STOP_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x04, 0);
// ROC_GET4_CMD_TO_GET4 => Initialize epoch counter to 0
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x08, ROC_CMD_LST_PUT + ROC_GET4_CMD_TO_GET4);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x0C, GET4V1X_EP_SYNC_INIT + 0x000000 );
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
// ROC_START_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x30, ROC_CMD_LST_PUT + ROC_START_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x34, 0);
// ROC_CMD_LST_ACTIVE
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x38, ROC_CMD_LST_PUT + ROC_CMD_LST_ACTIVE );
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x3C, 0);

// GET4 configuration
   // ROC_GET4_CMD_TO_GET4 => Disable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK          + 0x00000F );

   // ROC_GET4_CMD_TO_GET4 => Reset Readout and configuration
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_ROCONF_RST );

   // ROC_GET4_CMD_TO_GET4 => Enable DLL Auto Reset
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_DLL_RST_AUTO +      0x1 );

   // ROC_GET4_CMD_TO_GET4 => Enable 32b readout mode, disable LostEventError messages
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_RO_CONF_BASIC     +      0x1 );

   // ROC_GET4_CMD_TO_GET4 => Select Readout 1 diagnostic for channel 1 Rising edge
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DIAG_SELECT       +     0x23 );

   // ROC_GET4_CMD_TO_GET4 => Time over Threshold Configuration: 
   // => DI ON, AutoHeal ON, 2 bins resolution (100ps)
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_TOT   +      0x5 );

   // ROC_GET4_CMD_TO_GET4 => Time over Threshold Range: 
   // Bits maximum = 8 bits output (FF) * n bits resolution = FF (1 bin) to 7FF (8 bins)
   // Eventually can be set to higher than payload capacity => overflow (saturation?)
   // => Set here to to 1FF
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_TOT_MAX +  0x1FF );

   // ROC_GET4_CMD_TO_GET4 => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_LNK_RATE +   0x7 );

   // ROC_GET4_CMD_TO_GET4 => Lower DLL lock threshold
   // Maybe needed if DLL flag off while the lock can be observed at low levels on scope
//   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MIN       +      0x7 );

   // ROC_GET4_CMD_TO_GET4 => Upper DLL lock threshold
   // Maybe needed if DLL flag off while the lock can be observed at high levels on scope
//   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MAX       +      0x7 );

   // ROC_GET4_CMD_TO_GET4 => Re-Initialize the readout unit state machine
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_RO_INIT );

   // ROC_GET4_CMD_TO_GET4 => Enable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK          + 0x000000 );

   return conn.DoListSeq(nodeid, initList);
}

/*
 * GET4 v1.x (>1.2) initial configuration
 * int link = FLIB link number
 * int mode = readout mode to configure
 *            * 0 => default 24b mode settings
 *            * 1 => default 32b mode settings
 *            * 2 => tbd, read config from a text/xml file
 */
void config_get4v1x( int link, int mode, const uint32_t kRocId = 0xC2  )
{
	// Custom settings:
	int FlibLink = link;
	const uint32_t kNodeId = 0;

	// Needed ?
	const uint32_t kNxPort = 0; // 0 if nX is connected to CON19 connector; 1 for CON20 connector.
	if( kNxPort != 0 && kNxPort != 1 ) { printf("Error! invalid value kNxPort = %d\n", kNxPort ); return; }

	CbmNet::ControlClient conn;
	ostringstream dpath;

	dpath << "tcp://" << "localhost" << ":" << CbmNet::kPortControl + FlibLink;
	conn.Connect(dpath.str());

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
	printf("Setting ROCID to = %d \n", kRocId);
   conn.Write( kNodeId, ROC_ROCID, kRocId );
	SetRocDef( conn, kNodeId );

	switch( mode )
	{
	   case 0:
	      Set24bDef( conn, kNodeId );
	      break;
	   case 1:
	      Set32bDef( conn, kNodeId );
         break;
      case 2:
         std::cout<<" Configuration mode 2 not implemented yet ! "<<std::endl;
         break;
      default:
         std::cout<<" Unknown configuration mode "<<std::endl;
         break;
	} // switch( mode )


	// StartDAQ => Use the command list 2 in order to also 
	conn.Write( kNodeId, ROC_CMD_LST_NR, 2);

   // Check link status
	conn.Read( kNodeId, ROC_OPTICS_LINK_STATUS, ret );
	printf("ROC_OPTICS_LINK_STATUS  = %d\n", ret);

	ReadMessages( conn, kNodeId, 300 );

	// Close connection
	conn.Close();
}

void spiCmd_get4v1x( int link, int mode, uint32_t uSpiWord = 0xA5  )
{
   // Custom settings:
   int FlibLink = link;
   const uint32_t kNodeId = 0;

   // Needed ?
   const uint32_t kNxPort = 0; // 0 if nX is connected to CON19 connector; 1 for CON20 connector.
   if( kNxPort != 0 && kNxPort != 1 ) { printf("Error! invalid value kNxPort = %d\n", kNxPort ); return; }

   CbmNet::ControlClient conn;
   ostringstream dpath;

   dpath << "tcp://" << "localhost" << ":" << CbmNet::kPortControl + FlibLink;
   conn.Connect(dpath.str());

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
   time_t rawtime = ret;
   struct tm * timeinfo;
   char buffer [20];
   timeinfo = localtime( &rawtime);
   strftime( buffer,20,"%F %T",timeinfo);
   printf("build time       = %s \n", buffer);
   printf("\n");

   printf("Sending SPI command through GET4 => %d \n", uSpiWord);
   SendSpi( conn, kNodeId, 1, &uSpiWord)

   // Close connection
   conn.Close();
}

/*
 * GET4 v1.x (>1.2) + ROC + FLIB: direct readout of ROC messages
 * from ROC buffer through the registers interface
 */
int ReadMessages(CbmNet::ControlClient & conn, uint32_t nodeid, uint32_t nbMess = 1)
{
   CbmNet::ListSeq messList;
   // rocutil> ssm 2
   messList.AddRead(ROC_BURST1);
   messList.AddRead(ROC_BURST2);
   messList.AddRead(ROC_BURST3);

   int nSuccTot = 0;
   for( int iMess = 0; iMess < nbMess; iMess++ )
   {
      int nSucc = conn.DoListSeq(nodeid, messList);

      if( nSucc != messList.Size() ) {
         printf("Error in ReadMessages: errors during r/w SysCore registers. Number of successfully executed commands is %d out of %d\n",
               nSucc, messList.Size() );
         return -1;
      } // if( nSucc != list.Size() )
      printf("Message on ROC %04X %02X:%02X:%02X:%02X:%02X:%02X \n",
             nodeid,
             ((messList[2].value) >> 8)& 0xFF, (messList[2].value) & 0xFF,
             ((messList[1].value) >> 8)& 0xFF, (messList[1].value) & 0xFF,
             ((messList[0].value) >> 8)& 0xFF, (messList[0].value) & 0xFF);
      nSuccTot += nSucc;
   } // for( int iMess = 0; iMess < ; iMess++ )

   return nSuccTot;
}






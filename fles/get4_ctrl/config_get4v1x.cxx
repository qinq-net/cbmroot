#include <sstream>
#include <stdint.h>
#include <vector>
#include "fles/ctrl/control/ControlProtocol.hpp"
// Generic ROC registers
#include "roc/roclib/roc/defines_roc.h"
#include "roc/roclib/roc/defines_i2c.h"
#include "roc/roclib/roc/defines_optic.h"
// Generic ROC + FLIB registers
#include "fles/nxyter_ctrl/defines_gpio.h"
#include "fles/nxyter_ctrl/defines_spi.h"
// GET4 v1.x ROC registers
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

//# ROC_GET4_RECEIVE_MASK_LSBS => ROC_GET4_RECEIVE_MASK on ROC v2?
//  => Activate only the 4 first chips
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x0000000F);

//# ROC_GET4_READOUT_MODE => 24 bits mode selection
   initList.AddWrite(ROC_GET4_READOUT_MODE,               0);

//# Define CMD-list 2 (Reset + Start-DAQ)
//# ROC_GET4_CMD_TO_GET4 => Initialize epoch counter to 0
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x00, ROC_CMD_LST_PUT + ROC_GET4_CMD_TO_GET4);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x04, GET4V1X_EP_SYNC_INIT + 0x000000 );
//# ROC_GET4_TS156_RESET => ROC_GET4_TS_RESET in ROC v2 as resets both TS
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x08, ROC_CMD_LST_PUT + ROC_GET4_TS_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x0C, 1);
//# ROC_FIFO_RESET (active high?)
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x10, ROC_CMD_LST_PUT + ROC_GET4_FIFO_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x14, 1);
//# ROC_FIFO_RESET
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x18, ROC_CMD_LST_PUT + ROC_GET4_FIFO_RESET);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x1C, 0);
//# ROC_STOP_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x20, ROC_CMD_LST_PUT + ROC_OPTICS_STOP_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x24, 0);
//# ROC_CMD_LST_ACTIVE
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x28, ROC_CMD_LST_PUT + ROC_CMD_LST_ACTIVE );
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x2C, 0);

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
   /*
   initList.AddWrite(ROC_NX_SR_INIT,       0xfc03);
   */
   return conn.DoListSeq(nodeid, initList);
}

/*
 * GET$ v1.x (>1.2) initial configuration
 * int link = FLIB link number
 * int mode = readout mode to configure
 *            * 0 => default 24b mode settings
 *            * 1 => default 32b mode settings
 *            * 2 => tbd, read config from a text/xml file
 */
void config_get4v1x( int link, int mode, const uint32_t kRocId = C2  )
{
	//TODO: All !


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


	// StartDAQ
	conn.Write( kNodeId, ROC_NX_FIFO_RESET, 1);
	conn.Write( kNodeId, ROC_NX_FIFO_RESET, 0);
	conn.Write( kNodeId, ROC_OPTICS_START_DAQ, 1);

	uint32_t ret;
	const uint32_t ROC_OPTICS_LINK_STATUS = 0x201008;
	conn.Read( kNodeId, ROC_OPTICS_LINK_STATUS, ret );
	printf("ROC_OPTICS_LINK_STATUS  = %d\n", ret);


}






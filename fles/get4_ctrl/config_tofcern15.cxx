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

int ConfigRoc0_24b(CbmNet::ControlClient & conn, uint32_t nodeid)
{
   CbmNet::ListSeq initList;

//# ROC_GET4_RECEIVE_CLK_CFG => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_RECEIVE_CLK_CFG,           3);

// ROC_EPOCH250_EN => Enable/Disable the production of 250MHz (ROC)
//                    epochs
   initList.AddWrite(ROC_EPOCH250_EN,                    0);

//# ROC_GET4_RECEIVE_MASK_LSBS & ROC_GET4_RECEIVE_MASK_MSBS
//  => CERN Feb 2015: 3 FEE per ROC
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x00FFFFFF);
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_MSBS, 0x00000000);


//# ROC_GET4_SAMPLE_FALLING_EDGE_LSBS & ROC_GET4_SAMPLE_FALLING_EDGE_MSBS
//  => Change the edge on which the data from the GET4 are sampled
//  => Flat band cable dependent
   // ROC #1
   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_LSBS, 0x00000000);
   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_MSBS, 0x00000000);

//# ROC_GET4_SUPRESS_EPOCHS_LSBS & ROC_GET4_SUPRESS_EPOCHS_MSBS
//  => Enable the suppression of the 156.25 MHz epoch without data messages
//  => Chip granularity setting, maybe interesting to keep at least one per ROC
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_LSBS, 0x00000000);
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_MSBS, 0x00000000);

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
//# ROC_ADDSYSMSG
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x30, ROC_CMD_LST_PUT + ROC_ADDSYSMSG);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x34, 0x00000010);
//# ROC_START_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x38, ROC_CMD_LST_PUT + ROC_START_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x3C, 0);
//# ROC_CMD_LST_ACTIVE
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x40, ROC_CMD_LST_PUT + ROC_CMD_LST_ACTIVE );
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x44, 0);

//# GET4 configuration
   //# GET4V1X_TDC_CORE_TIMING_CONF => DLL TDC core timing
   //   (bit 4 enable, bits3..0 : phase shift in 400ps steps)
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_TDC_CORE_TIMING_CONF + 0x000012 );

   //# GET4V1X_DLL_DAC_MIN => lower DLL monitoring threshold 450 mV
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MIN          + 0x000100 );

   //# GET4V1X_DLL_DAC_MAX => upper DLL monitoring threshold 1000 mV
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MAX          + 0x000238 );

   //# ROC_GET4_CMD_TO_GET4 => Disable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK             + 0x00000F );

   //# GET4V1X_SERIAL_FLOW_CONTR => disable serial interface
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SERIAL_FLOW_CONTR    + 0x000001 );

   //# ROC_GET4_CMD_TO_GET4 => Reset Readout and configuration
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_ROCONF_RST );

   //# ROC_GET4_CMD_TO_GET4 => Enable DLL Auto Reset
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_DLL_RST_AUTO    +      0x1 );

   //# ROC_GET4_CMD_TO_GET4 => Enable 24b readout mode, disable LostEventError messages
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_RO_CONF_BASIC        +      0x0 );

   //# ROC_GET4_CMD_TO_GET4 => Select Readout 1 diagnostic for channel 1 Rising edge
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DIAG_SELECT          +     0x23 );

   //# ROC_GET4_CMD_TO_GET4 => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_24B_RO_CONF          +      0x7 );

// ROC_GET4_RECEIVE_CLK_CFG => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_RECEIVE_CLK_CFG,           0x3);

   //# ROC_GET4_CMD_TO_GET4 => Re-Initialize the readout unit state machine
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_RO_INIT );

   //# GET4V1X_SERIAL_FLOW_CONTR => enable serial interface
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SERIAL_FLOW_CONTR    + 0x000000 );

   //# ROC_GET4_CMD_TO_GET4 => Enable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK             + 0x000000 );

   return conn.DoListSeq(nodeid, initList);
}

int ConfigRoc1_24b(CbmNet::ControlClient & conn, uint32_t nodeid)
{
   CbmNet::ListSeq initList;

//# ROC_GET4_RECEIVE_CLK_CFG => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_RECEIVE_CLK_CFG,           3);

// ROC_EPOCH250_EN => Enable/Disable the production of 250MHz (ROC)
//                    epochs
   initList.AddWrite(ROC_EPOCH250_EN,                    0);

//# ROC_GET4_RECEIVE_MASK_LSBS & ROC_GET4_RECEIVE_MASK_MSBS
//  => CERN Feb 2015: 3 FEE per ROC
//   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x00FFFFFF);
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x0000FFFF); // USTC out as oscillating
//   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x000000FF); // USTC out as oscillating, HD REF Out
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_MSBS, 0x00000000);


//# ROC_GET4_SAMPLE_FALLING_EDGE_LSBS & ROC_GET4_SAMPLE_FALLING_EDGE_MSBS
//  => Change the edge on which the data from the GET4 are sampled
//  => Flat band cable dependent
   // ROC #2
   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_LSBS, 0x00880000);
   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_MSBS, 0x00000000);

//# ROC_GET4_SUPRESS_EPOCHS_LSBS & ROC_GET4_SUPRESS_EPOCHS_MSBS
//  => Enable the suppression of the 156.25 MHz epoch without data messages
//  => Chip granularity setting, maybe interesting to keep at least one per ROC
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_LSBS, 0x00000000);
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_MSBS, 0x00000000);

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
//# ROC_ADDSYSMSG
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x30, ROC_CMD_LST_PUT + ROC_ADDSYSMSG);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x34, 0x00000010);
//# ROC_START_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x38, ROC_CMD_LST_PUT + ROC_START_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x3C, 0);
//# ROC_CMD_LST_ACTIVE
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x40, ROC_CMD_LST_PUT + ROC_CMD_LST_ACTIVE );
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x44, 0);

//# GET4 configuration
   //# GET4V1X_TDC_CORE_TIMING_CONF => DLL TDC core timing
   //   (bit 4 enable, bits3..0 : phase shift in 400ps steps)
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_TDC_CORE_TIMING_CONF + 0x000012 );

   //# GET4V1X_DLL_DAC_MIN => lower DLL monitoring threshold 450 mV
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MIN          + 0x000100 );

   //# GET4V1X_DLL_DAC_MAX => upper DLL monitoring threshold 1000 mV
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MAX          + 0x000238 );

   //# ROC_GET4_CMD_TO_GET4 => Disable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK             + 0x00000F );

   //# GET4V1X_SERIAL_FLOW_CONTR => disable serial interface
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SERIAL_FLOW_CONTR    + 0x000001 );

   //# ROC_GET4_CMD_TO_GET4 => Reset Readout and configuration
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_ROCONF_RST );

   //# ROC_GET4_CMD_TO_GET4 => Enable DLL Auto Reset
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_DLL_RST_AUTO    +      0x1 );

   //# ROC_GET4_CMD_TO_GET4 => Enable 24b readout mode, disable LostEventError messages
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_RO_CONF_BASIC        +      0x0 );

   //# ROC_GET4_CMD_TO_GET4 => Select Readout 1 diagnostic for channel 1 Rising edge
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DIAG_SELECT          +     0x23 );

   //# ROC_GET4_CMD_TO_GET4 => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_24B_RO_CONF          +      0x7 );

// ROC_GET4_RECEIVE_CLK_CFG => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_RECEIVE_CLK_CFG,           0x3);

   //# ROC_GET4_CMD_TO_GET4 => Re-Initialize the readout unit state machine
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_RO_INIT );

   //# GET4V1X_SERIAL_FLOW_CONTR => enable serial interface
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SERIAL_FLOW_CONTR    + 0x000000 );

   //# ROC_GET4_CMD_TO_GET4 => Enable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK             + 0x000000 );

   return conn.DoListSeq(nodeid, initList);
}


int ConfigRoc0_32b(CbmNet::ControlClient & conn, uint32_t nodeid)
{
   CbmNet::ListSeq initList;

// ROC_EPOCH250_EN => Enable/Disable the production of 250MHz (ROC)
//                    epochs
   initList.AddWrite(ROC_EPOCH250_EN,                    0);

// ROC_GET4_RECEIVE_MASK_LSBS & ROC_GET4_RECEIVE_MASK_MSBS
//  => CERN Feb 2015: 3 FEE per ROC
//   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x0000FFFF);
//   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x00000000);
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x00FFFFFF);
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_MSBS, 0x00000000);

// ROC_GET4_SAMPLE_FALLING_EDGE_LSBS & ROC_GET4_SAMPLE_FALLING_EDGE_MSBS
//  => Change the edge on which the data from the GET4 are sampled
//  => Flat band cable dependent
   // ROC #1
   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_LSBS, 0x00000000);
   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_MSBS, 0x00000000);

// ROC_GET4_SUPRESS_EPOCHS_LSBS & ROC_GET4_SUPRESS_EPOCHS_MSBS
//  => Enable the suppression of the 156.25 MHz epoch without data messages
//  => Chip granularity setting, maybe interesting to keep at least one per ROC
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_LSBS, 0x00000000);
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_MSBS, 0x00000000);

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
//# ROC_ADDSYSMSG
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x30, ROC_CMD_LST_PUT + ROC_ADDSYSMSG);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x34, 0x00000010);
//# ROC_START_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x38, ROC_CMD_LST_PUT + ROC_START_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x3C, 0);
//# ROC_CMD_LST_ACTIVE
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x40, ROC_CMD_LST_PUT + ROC_CMD_LST_ACTIVE );
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x44, 0);

// GET4 configuration
   //# GET4V1X_TDC_CORE_TIMING_CONF => DLL TDC core timing
   //   (bit 4 enable, bits3..0 : phase shift in 400ps steps)
         // Nominal
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_TDC_CORE_TIMING_CONF + 0x000012 );

   //# GET4V1X_DLL_DAC_MIN => lower DLL monitoring threshold 450 mV
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MIN          + 0x000100 );

   //# GET4V1X_DLL_DAC_MAX => upper DLL monitoring threshold 1000 mV
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MAX          + 0x000238 );

   // ROC_GET4_CMD_TO_GET4 => Disable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK             + 0x00000F );

   //# GET4V1X_SERIAL_FLOW_CONTR => disable serial interface  
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SERIAL_FLOW_CONTR    + 0x000001 );

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
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_TOT   +      0xD );

   // ROC_GET4_CMD_TO_GET4 => Time over Threshold Range: 
   // Bits maximum = 8 bits output (FF) * n bits resolution = FF (1 bin) to 7FF (8 bins)
   // Eventually can be set to higher than payload capacity => overflow (saturation?)
   // => Set here to to 1FF
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_TOT_MAX +  0x1FF );


   // ROC_GET4_CMD_TO_GET4 => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_LNK_RATE +   0x6 );

// ROC_GET4_RECEIVE_CLK_CFG => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_RECEIVE_CLK_CFG,           0x3);

   // ROC_GET4_CMD_TO_GET4 => Re-Initialize the readout unit state machine
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_RO_INIT );

   //# GET4V1X_SERIAL_FLOW_CONTR => enable serial interface
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SERIAL_FLOW_CONTR    + 0x000000 );

   // ROC_GET4_CMD_TO_GET4 => Enable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK          + 0x000000 );

//# ROC_GET4_READOUT_MODE => 32 bits mode selection
   initList.AddWrite(ROC_GET4_READOUT_MODE,               1);

   return conn.DoListSeq(nodeid, initList);
}

int ConfigRoc1_32b(CbmNet::ControlClient & conn, uint32_t nodeid)
{
   CbmNet::ListSeq initList;

// ROC_EPOCH250_EN => Enable/Disable the production of 250MHz (ROC)
//                    epochs
   initList.AddWrite(ROC_EPOCH250_EN,                    0);

// ROC_GET4_RECEIVE_MASK_LSBS & ROC_GET4_RECEIVE_MASK_MSBS
//  => CERN Feb 2015: 3 FEE per ROC
//   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x00FFFFFF);
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x0000FFFF); // USTC out as oscillating
//   initList.AddWrite(ROC_GET4_RECEIVE_MASK_LSBS, 0x000000FF); // USTC out as oscillating, HD REF Out
   initList.AddWrite(ROC_GET4_RECEIVE_MASK_MSBS, 0x00000000);

// ROC_GET4_SAMPLE_FALLING_EDGE_LSBS & ROC_GET4_SAMPLE_FALLING_EDGE_MSBS
//  => Change the edge on which the data from the GET4 are sampled
//  => Flat band cable dependent
   // ROC #2
   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_LSBS, 0x00FF0000);
   initList.AddWrite(ROC_GET4_SAMPLE_FALLING_EDGE_MSBS, 0x00000000);

// ROC_GET4_SUPRESS_EPOCHS_LSBS & ROC_GET4_SUPRESS_EPOCHS_MSBS
//  => Enable the suppression of the 156.25 MHz epoch without data messages
//  => Chip granularity setting, maybe interesting to keep at least one per ROC
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_LSBS, 0x00000000);
   initList.AddWrite(ROC_GET4_SUPRESS_EPOCHS_MSBS, 0x00000000);

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
//# ROC_ADDSYSMSG
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x30, ROC_CMD_LST_PUT + ROC_ADDSYSMSG);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x34, 0x00000010);
//# ROC_START_DAQ
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x38, ROC_CMD_LST_PUT + ROC_START_DAQ);
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x3C, 0);
//# ROC_CMD_LST_ACTIVEGet4:0x13
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x40, ROC_CMD_LST_PUT + ROC_CMD_LST_ACTIVE );
   initList.AddWrite(ROC_CMD_LST_MEM + 0x200 + 0x44, 0);

// GET4 configuration
   //# GET4V1X_TDC_CORE_TIMING_CONF => DLL TDC core timing
   //   (bit 4 enable, bits3..0 : phase shift in 400ps steps)
         // Nominal
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_TDC_CORE_TIMING_CONF + 0x000012 );

   //# GET4V1X_DLL_DAC_MIN => lower DLL monitoring threshold 450 mV
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MIN          + 0x000100 );

   //# GET4V1X_DLL_DAC_MAX => upper DLL monitoring threshold 1000 mV
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_DLL_DAC_MAX          + 0x000238 );

   // ROC_GET4_CMD_TO_GET4 => Disable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK             + 0x00000F );

   //# GET4V1X_SERIAL_FLOW_CONTR => disable serial interface
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SERIAL_FLOW_CONTR    + 0x000001 );

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
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_TOT   +      0xD );

   // ROC_GET4_CMD_TO_GET4 => Time over Threshold Range:
   // Bits maximum = 8 bits output (FF) * n bits resolution = FF (1 bin) to 7FF (8 bins)
   // Eventually can be set to higher than payload capacity => overflow (saturation?)
   // => Set here to to 1FF
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_TOT_MAX +  0x1FF );

   // ROC_GET4_CMD_TO_GET4 => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_32B_RO_CONF_LNK_RATE +   0x6 );

// ROC_GET4_RECEIVE_CLK_CFG => Set link speed to 156.25 MBit/s
   initList.AddWrite(ROC_GET4_RECEIVE_CLK_CFG,           0x3);

   // ROC_GET4_CMD_TO_GET4 => Re-Initialize the readout unit state machine
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_INIT_RO_INIT );

   //# GET4V1X_SERIAL_FLOW_CONTR => enable serial interface
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_SERIAL_FLOW_CONTR    + 0x000000 );

   // ROC_GET4_CMD_TO_GET4 => Enable all channels
   initList.AddWrite(ROC_GET4_CMD_TO_GET4, GET4V1X_HIT_MASK          + 0x000000 );

//# ROC_GET4_READOUT_MODE => 32 bits mode selection
   initList.AddWrite(ROC_GET4_READOUT_MODE,               1);

   return conn.DoListSeq(nodeid, initList);
}

int ConfigChannelsRoc0(CbmNet::ControlClient & conn, uint32_t nodeid)
{
   return SetRocGet4ChanEna( conn, nodeid,
         0x7FFFFFFE,
         0xFFFE7FFF,
         0xFFBFFDFF,
         0x00000000 );

   /*
   return SetRocGet4ChanEna( conn, nodeid,
         0x00000000,
         0x00000000,
         0x00000000,
         0x00000000 );
   */
}

int ConfigChannelsRoc1(CbmNet::ControlClient & conn, uint32_t nodeid)
{

   return SetRocGet4ChanEna( conn, nodeid,
         0xFFFFFFFF,
         0xFFFBFFFE,
         0x7FFEFFFE,
         0x00000000 );

   /*
   return SetRocGet4ChanEna( conn, nodeid,
      0x00000000,
      0x00000000,
      0x00000000,
      0x00000000 );
   */
}

/*
 * CERN Feb 2015 initial configuration
 * int mode = readout mode to configure
 *            * 0 => default 24b mode settings
 *            * 1 => default 32b mode settings
 *            * 2 => tbd, read config from a text/xml file
 * uint uNbMess = nb of messages printed from control interface for each ROC
 */
void config_tofcern15( int mode, uint32_t uNbMess = 800  )
{
   // Custom settings:
   const uint32_t kNodeId  = 0;
   const uint32_t kuNbRocs = 2;

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
                ConfigRoc0_24b( conn, kNodeId );
                ConfigChannelsRoc0( conn, kNodeId );
                break;
             case 1:
                ConfigRoc1_24b( conn, kNodeId );
                ConfigChannelsRoc1( conn, kNodeId );
                break;
             default:
                Set24bDef( conn, kNodeId );
                SetRocGet4ChanEna( conn, kNodeId);
                break;
          } // switch( uRoc )
          break;
        case 1:
           switch( uRoc )
           {
              case 0:
                 ConfigRoc0_32b( conn, kNodeId );
                 ConfigChannelsRoc0( conn, kNodeId );
                 break;
              case 1:
                 ConfigRoc1_32b( conn, kNodeId );
                 ConfigChannelsRoc1( conn, kNodeId );
                 break;
              default:
                 Set32bDef( conn, kNodeId );
                 SetRocGet4ChanEna( conn, kNodeId);
                 break;
           } // switch( uRoc )
          break;
        case 2:
          std::cout<<" Configuration mode 2 not implemented yet ! "<<std::endl;
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



#ifndef PVC_PLATFORM_SPITCAN_H
#define PVC_PLATFORM_SPITCAN_H

/* @brief: Spit(o)can is the SPI to CAN-bus API for project PVC. Spitcan is only
 *         available for the MCP2515 Transceiver module (See references), and
 *         should not be used with other CAN-bus modules. This codebase has been
 *         created with help of the MCP2515 manual and should not be edited nor
 *         viewed without this manual at hand.
 *
 *         References:
 *           -  https://ww1.microchip.com/downloads/aemDocuments/documents/APID/
 *              ProductDocuments/DataSheets/ MCP2515-Family-Data-Sheet-
 *              DS20001801K.pdf
 *
 * @authors: Rene Huiberts
 * @date: 09 - 12 - 2023
 */

#include "driver/gpio.h"

#include "driver/spi_master.h"
#include "driver/spi_slave.h"

#ifndef PVC_SPITCAN_DEBUG
#  define PVC_SPITCAN_DEBUG 0x0
#endif

//= rhjr: mcp2515 types

typedef enum pvc_spitcan_message_priority pvc_spitcan_message_priority;
enum pvc_spitcan_message_priority
{
  LOW_PRIORITY       = 0x00,
  LOW_INTM_PRIORITY,
  HIGH_INTM_PRIORITY, 
  HIGH_PRIORITY,
  MAX_PRIORITY
};

typedef enum mcp_instruction mcp_instruction;
enum mcp_instruction
{
  INSTRUCTION_WRITE  = 0x02, 
  INSTRUCTION_READ   = 0x03,  
  INSTRUCTION_BITMOD = 0x05,
  INSTRUCTION_RESET  = 0xC0
};

typedef enum pvc_mcp_register pvc_mcp_register;
enum pvc_mcp_register
{
  REGISTER_CANCTRL  = 0x0F,
  REGISTER_CANSTAT  = 0x0E,

  REGISTER_TXB0CTRL = 0x30, 
  REGISTER_TXB0SIDH = 0x31, 
  REGISTER_TXB0SIDL = 0x32, 
  REGISTER_TXB0DLC  = 0x35, 
  REGISTER_TXB0DM   = 0x36, 
  // rhjr: REGISTER_TXB0DM + 7

  REGISTER_RXB0CTRL = 0x60, 
  REGISTER_RXB0SIDH = 0x61, 
  REGISTER_RXB0SIDL = 0x62, 
  REGISTER_RXB0DLC  = 0x65, 
  REGISTER_RXB0DM   = 0x66,
  // rhjr: REGISTER_RXB0DM + 7

  REGISTER_CANINTE  = 0x2B,
  REGISTER_CANINTF  = 0x2C,

  REGISTER_CANF3    = 0x28,
  REGISTER_CANF2    = 0x29,
  REGISTER_CANF1    = 0x2A,

  // rhjr: error flags
  REGISTER_EFLG     = 0x2D,
  REGISTER_TEC      = 0x1C,
  REGISTER_REC      = 0x1D
};

typedef enum pvc_mcp_mode pvc_mcp_mode;
enum pvc_mcp_mode
{
  MODE_NORMAL = 0x00,
  MODE_SLEEP,
  MODE_LOOPB,
  MODE_LISTEN,
  MODE_CONFIG,
  MODE_MAX    
};

#define PVC_SPITCAN_TXnCTRL_SIZE 14

typedef enum pvc_mcp_register_TXnCTRL pvc_mcp_register_TXnCTRL;
enum pvc_mcp_register_TXnCTRL
{
  TXB_ABTF   = 0x40,
  TXB_MLOA   = 0x20,
  TXB_TXERR  = 0x10,
  TXB_TXREQ  = 0x08,
  TXB_UNUSED = 0x04,
  TXB_TXP    = 0x03
};

//= rhjr: spitcan data 

// rhjr: masks
#define PVC_CAN_SFF_MASK 0x000007FFUL /* rhjr: standard frame format (SFF) */
#define PVC_CAN_DLC_MASK 0x0F

typedef struct pvc_spitcan_message pvc_spitcan_message;
struct pvc_spitcan_message
{
  uint32_t identifier;
  uint8_t  length_in_bytes;
  uint32_t data;
};

//= rhjr: spitcan interface 

internal esp_err_t pvc_spitcan_initalize ();

//- rhjr: device helpers 

internal esp_err_t pvc_spitcan_reset_device (
  spi_device_handle_t device);

internal esp_err_t pvc_spitcan_add_device (
  spi_host_device_t host_device, spi_device_handle_t *device);

internal esp_err_t pvc_spitcan_device_set_mode (
  spi_device_handle_t device, pvc_mcp_mode mode);

//- rhjr: spitcan register manipulation

// rhjr: set
internal esp_err_t pvc_spitcan_set_register (
  pvc_mcp_register _register, uint8_t value);

internal esp_err_t pvc_spitcan_set_registers (
  pvc_mcp_register _register, uint8_t *data, uint8_t length_in_bytes);

// rhjr: read
internal esp_err_t pvc_spitcan_read_register (
  pvc_mcp_register _register, uint8_t *data);

internal esp_err_t pvc_spitcan_read_registers (
  pvc_mcp_register _register, uint8_t *data, uint8_t length_in_bytes);

// rhjr: modify
internal esp_err_t pvc_spitcan_modify_register(
  pvc_mcp_register _register, uint8_t mask, uint8_t data);

//- rhjr: writing spitcan message + helpers

internal void pvc_spitcan_set_message_identification (
  uint8_t *sidn_buffer, pvc_spitcan_message *frame);

internal esp_err_t pvc_spitcan_write_message (
  pvc_spitcan_message *message, pvc_spitcan_message_priority priority);

//- rhjr: reading spitcan message + helpers

internal pvc_spitcan_message * pvc_spitcan_read_message (
  pvc_arena *arena);

internal bool pvc_spitcan_received_new_message ();

#endif // PVC_PLATFORM_SPITCAN_H
// spitcan.h ends here.

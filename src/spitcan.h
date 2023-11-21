#ifndef PVC_SPITCAN_H
#define PVC_SPITCAN_H

//= rhjr: spitcan helpers

#define BYTES(x) ((x * 8)) 
#define MHZ(x) ((x * 1000000))

//= rhjr: serial peripheral interface

#define PVC_SPI_SCK_FREQ MHZ(8)  

#define PVC_SPI_PIN      HSPI_HOST   /* rhjr: SPI1_HOST is reserved.          */
#define PVC_SPI_PIN_SCK  GPIO_NUM_14
#define PVC_SPI_PIN_SDO  GPIO_NUM_13 /* MOSI                                  */
#define PVC_SPI_PIN_SDI  GPIO_NUM_12 /* MISO                                  */
#define PVC_SPI_PIN_CS0  GPIO_NUM_15 /* rhjr: Reserved for first slave.       */

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

  REGISTER_TXB0CTRL = 0x30, 
  REGISTER_TXB0SIDH = 0x31, 
  REGISTER_TXB0SIDL = 0x32, 
  REGISTER_TXB0DLC  = 0x35, 
  REGISTER_TXB0DM   = 0x36, 

  REGISTER_RXB0CTRL = 0x60, 
  REGISTER_RXB0SIDH = 0x61, 
  REGISTER_RXB0SIDL = 0x62, 
  REGISTER_RXB0DLC  = 0x65, 
  REGISTER_RXB0DM   = 0x66,

  REGISTER_CANINTE  = 0x2B,
  REGISTER_CANINTF  = 0x2C
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

//- rhjr: CAN

#define PVC_CAN_SFF_MASK 0x000007FFUL /* rhjr: standard frame format (SFF) */

typedef struct pvc_spitcan_message pvc_spitcan_message;
struct pvc_spitcan_message
{
  uint32_t identifier;
  uint8_t length_in_bytes;
  uint8_t *data;
};

//= rhjr: spitcan interface

internal esp_err_t pvc_spitcan_initalize (spi_host_device_t host_device);

internal esp_err_t pvc_spitcan_add_device (
  spi_host_device_t host_device, spi_device_handle_t *device);
internal esp_err_t pvc_spitcan_reset_device (
  spi_device_handle_t device);

//- rhjr: spitcan state

internal esp_err_t pvc_spitcan_device_set_mode (
  spi_device_handle_t device, pvc_mcp_mode mode);

//- rhjr: spitcan register manipulation

internal esp_err_t pvc_spitcan_set_register (
  const pvc_mcp_register _register, const uint8_t value);

internal esp_err_t pvc_spitcan_set_registers (
  const pvc_mcp_register _register, const uint8_t *data,
  const uint8_t length_in_bytes);

internal esp_err_t pvc_spitcan_read_register (
  const pvc_mcp_register _register, uint8_t *data);

internal esp_err_t pvc_spitcan_read_registers (
  const pvc_mcp_register _register, uint8_t *data, uint8_t length_in_bytes);

// rhjr: Note not all registers can be modified, see which are available in the
//       MCP2515-manual.
internal esp_err_t pvc_spitcan_modify_register(
  const pvc_mcp_register _register, const uint8_t mask, uint8_t data);

//- rhjr: spitcan message frame

internal void pvc_spitcan_set_message_identification (
  uint8_t *sidn_buffer, pvc_spitcan_message *frame);

internal esp_err_t pvc_spitcan_write_message (spi_device_handle_t *device,
  pvc_spitcan_message *frame, pvc_spitcan_message_priority priority);

#endif /* PVC_SPITCAN_H */
// spitcan.h ends here.

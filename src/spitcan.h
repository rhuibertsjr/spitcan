#ifndef PVC_SPITCAN_H
#define PVC_SPITCAN_H

//= rhjr: serial peripheral interface

#define PVC_SPI_MODE      0x0
#define PVC_SPI_SCK_FREQ  SPI_MASTER_FREQ_8M 
#define PVC_SPI_QUEUE_LEN 0x7

#define PVC_SPI_PIN     HSPI_HOST /* rhjr: SPI1_HOST is reserved.             */
#define PVC_SPI_PIN_SCK 0x14 
#define PVC_SPI_PIN_SDO 0x13 /* MOSI                                          */
#define PVC_SPI_PIN_SDI 0x12 /* MISO                                          */
#define PVC_SPI_PIN_CS0 0x15 /* rhjr: Reserved for first slave.               */

//= rhjr: mcp2515 types

typedef enum mcp_instruction mcp_instruction;
enum mcp_instruction
{
  INSTRUCTION_WRITE  = 0x02, 
  INSTRUCTION_READ,  
  INSTRUCTION_BITMOD = 0x05,
  INSTRUCTION_RESET  = 0xC0
};

typedef enum mcp_register mcp_register;
enum mcp_register
{
  REGISTER_CANCTRL = 0x0F,
};

typedef enum mcp_mode mcp_mode;
enum mcp_mode
{
  MODE_NORMAL = 0x00,
  MODE_SLEEP,
  MODE_LOOPB,
  MODE_LISTEN,
  MODE_CONFIG,
  MODE_MAX    
};

//= rhjr: mcp2515 interface

internal esp_err_t pvc_mcp2515_reset    (spi_device_handle_t *device);
internal esp_err_t pvc_mcp2515_set_mode (mcp_mode mode);

//= rhjr: spitcan interface

internal void pvc_spitcan_initalize  (spi_host_device_t   host);
internal esp_err_t pvc_spitcan_add_device (
  spi_host_device_t host, spi_device_handle_t *device);

// rhjr: TODO writing and reading from MCP2515
//internal void pvc_spitcan_u32_read  (spi_device_handle_t device);
//internal void pvc_spitcan_u32_write (spi_device_handle_t device);

#endif /* PVC_SPITCAN_H */
// spitcan.h ends here.

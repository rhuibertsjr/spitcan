global_variable spi_device_handle_t mcp2515;

//= rhjr: mcp2515 interface

internal esp_err_t
pvc_mcp2515_reset (spi_device_handle_t *device)
{
  esp_err_t result;
  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  transaction.flags      = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.tx_data[0] = INSTRUCTION_RESET;
  transaction.length     = 8;
  //- rhjr: transmission
  result = // rhjr: blocks until transaction is completed.
    spi_device_polling_transmit(*device, &transaction);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  return result;
}

internal esp_err_t
pvc_mcp2515_set_mode (mcp_mode mode)
{
  ASSERT(mode < MODE_MAX, "Mode %u is not available.", mode);

  esp_err_t result;
  //- rhjr: message 
  mcp_register canctrl       = REGISTER_CANCTRL;
  uint8_t canctrl_reqop_mask = 0xE0;  
  //- rhjr: transaction
  spi_transaction_t transaction = {};
  transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.tx_data[0] = INSTRUCTION_WRITE;
  transaction.tx_data[1] = canctrl;
  transaction.tx_data[2] = canctrl_reqop_mask;
  transaction.tx_data[3] = mode;
  transaction.length = 32;
  //- rhjr: transmission
  result = // rhjr: blocks until transaction is completed.
    spi_device_polling_transmit(mcp2515, &transaction);

  // rhjr: TODO check if the register is actually set.

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  return result;
}

//= rhjr: spitcan

internal esp_err_t
pvc_spitcan_add_device (spi_host_device_t spi_host, spi_device_handle_t *device)
{
  ASSERT(spi_host < SPI_HOST_MAX, "Invalid SPI host");
  ASSERT(PVC_SPI_SCK_FREQ < SPI_MASTER_FREQ_20M, 
    "PVC_SPI_PIN_CLOCK_SPEED exceeds the allowed clock of MCP2515.");

  esp_err_t result;
  spi_device_interface_config_t spi_device_config = {0};  

  spi_device_config.mode           = PVC_SPI_MODE;
  spi_device_config.clock_speed_hz = PVC_SPI_SCK_FREQ;
  spi_device_config.spics_io_num   = PVC_SPI_PIN_CS0;
  spi_device_config.queue_size     = PVC_SPI_QUEUE_LEN;
  spi_device_config.duty_cycle_pos = 128; // 50% 

  result =
    spi_bus_add_device(spi_host, &spi_device_config, device);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);

  result =
    spi_device_acquire_bus(*device, portMAX_DELAY);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  return result;
}

internal void 
pvc_spitcan_initalize (spi_host_device_t spi_host)
{
  LOG(TAG_SPI, INFO, "Initializing Spitcan...");
  ASSERT(spi_host < SPI_HOST_MAX, "Invalid SPI host.");

  esp_err_t result;
  //- rhjr: message
  spi_bus_config_t spi_bus_config =
    {
      .sclk_io_num = PVC_SPI_PIN_SCK,
      .mosi_io_num = PVC_SPI_PIN_SDO,
      .miso_io_num = PVC_SPI_PIN_SDI,
      .quadwp_io_num = -1, // UNUSED
      .quadhd_io_num = -1, // UNUSED
    };

  //- rhjr: transmission
  result = spi_bus_initialize(spi_host, &spi_bus_config, SPI_DMA_DISABLED);
  
  //- rhjr: initializing mcp2515
  LOG(TAG_SPI, INFO, "  - Initializing MCP2515...");
  LOG(TAG_SPI, INFO, "    + Adding device to SPI bus");

  pvc_spitcan_add_device(spi_host, &mcp2515);

  LOG(TAG_SPI, INFO, "    + Configuring MCP2515");

  pvc_mcp2515_reset(&mcp2515);
  pvc_mcp2515_set_mode(MODE_NORMAL);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  return;
}

//internal esp_err_t
//pvc_spitcan_write (
//  spi_device_handle_t device, const uint8_t *data, uint32_t length_in_bytes)
//{
//  ASSERT(length_in_bytes > 0, "expected at least a length of 1");
//
//  esp_err_t result;
//
//  //- rhjr: SPI transaction
//  spi_transaction_t transaction = {0};
//  transaction.tx_buffer = data;
//  transaction.length    = length_in_bytes * 8; 
//
//  result = // rhjr: blocks until transaction is completed.
//    spi_device_transmit(device, &transaction);
//
//  ASSERT(result == ESP_OK, "Unexpected result");
//  LOG(TAG_SPI, INFO, "%s", "Message succesfully send");
//  return result;
//}
//
//esp_err_t pvc_spitcan_read (
//  spi_device_handle_t device, uint8_t *data, uint32_t length_in_bytes)
//{
//  ASSERT(length_in_bytes > 0, "expected at least a length of 1");
//
//  esp_err_t result;
//
//  //- rhjr: SPI transaction
//  spi_transaction_t transaction = {0};
//  transaction.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
//
//  // rhjr: MCP2515-manual Figure 12-2 p. 68
//  transaction.length     =   24;
//  transaction.tx_data[0] = 0x03;   
//  transaction.tx_data[1] = 0x66;
//  transaction.tx_data[2] = 0x00;
//  transaction.rx_buffer  = data;
//
//  result = // rhjr: blocks until transaction is completed.
//    spi_device_polling_transmit(device, &transaction);
//
//  ASSERT(result == ESP_OK, "Unexpected result");
//  LOG(TAG_SPI, INFO, "Message succesfully received");
//  return result;
//}

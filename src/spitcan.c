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
  pvc_mcp_register canctrl   = REGISTER_CANCTRL;
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
  LOG(TAG_SPI, INFO, "    + Adding MCP2515 to the SPI bus");

  pvc_spitcan_add_device(spi_host, &mcp2515);

  LOG(TAG_SPI, INFO, "    + Configuring MCP2515");

  pvc_mcp2515_reset(&mcp2515);
  pvc_mcp2515_set_mode(MODE_NORMAL);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  LOG(TAG_SPI, INFO, "Spitcan succesfully initialized.");
  return;
}

internal void
pvc_spitcan_message_set_identification (
  uint8_t *sidn_buffer, pvc_spitcan_message *frame)
{
  // rhjr: The frame identifier is a 12-bit field. Where there is a 11-bit field
  //       for the identification and a 1-bit field for an unused flag.
  uint16_t identifier = (uint16_t)(frame->id & 0x0FFFF);

  // rhjr: Use standard frame format for the CAN-protocol.
  identifier &= PVC_CAN_SFF_MASK;

  *sidn_buffer++ = identifier >> 3;
  *sidn_buffer = (identifier & 0x07) << 5;

  LOG(TAG_SPI, INFO, "SIDL register dump: %u", *sidn_buffer-- );
  LOG(TAG_SPI, INFO, "SIDH register dump: %u", *sidn_buffer);
}

internal esp_err_t
pvc_spitcan_read_from_registers (
  const pvc_mcp_register _register, uint8_t *data, uint8_t length_in_bits)
{
  //ASSERT(length_in_bits <= 8,
  //"Length in bytes exceeds the registers limit. Currently: %u",
  //length_in_bits );

  // rhjr: TODO: fix the length in bytes/bits for this function.

  esp_err_t result;
  uint8_t result_data[length_in_bits + 2];

  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  transaction.flags  = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.length = 8;

  transaction.tx_data[0] = INSTRUCTION_READ;
  transaction.tx_data[1] = _register;

  transaction.rx_buffer  = result_data;

  //- rhjr: transmission
  result = spi_device_transmit(mcp2515, &transaction);

  if (result != ESP_OK)
  {
    //LOG(TAG_SPI, ERROR, "Transmission failed, error code: %u", result); 
  }
  else
  {
    for (uint8_t index = 0; index < length_in_bits; index += 1)
    {
      *data++ = result_data[index];
    }
  }

  return result;
}

internal esp_err_t
pvc_spitcan_read_from_register(const pvc_mcp_register _register, uint8_t *data)
{
  return pvc_spitcan_read_from_registers(_register, data, 1);
}

internal esp_err_t pvc_spitcan_write_to_register (
  const pvc_mcp_register _register, const uint8_t mask, const uint8_t data)
{
  esp_err_t result;

  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  transaction.flags     = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.length    = 32; 

  transaction.tx_data[0] = INSTRUCTION_BITMOD;
  transaction.tx_data[1] = _register;
  transaction.tx_data[2] = mask;
  transaction.tx_data[3] = data;

  result = 
    spi_device_transmit(mcp2515, &transaction);

  if (result != ESP_OK)
  LOG(TAG_SPI, ERROR, "Writing message to MCP2515 failed");

  return result;
}

internal esp_err_t pvc_spitcan_write_to_registers (
  const pvc_mcp_register _register, const uint8_t *data,
  const uint8_t length_in_bytes)
{
  esp_err_t result;

  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  transaction.flags     = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.length    = 32; 

  transaction.tx_data[0] = INSTRUCTION_BITMOD;
  transaction.tx_data[1] = _register;

  for (uint8_t index = 0; index < length_in_bytes; index += 1)
  {
    transaction.tx_data[index + 2] = *data++;
  }

  //- rhjr: transmission
  result = spi_device_transmit(mcp2515, &transaction);

  if (result != ESP_OK)
  LOG(TAG_SPI, ERROR, "Writing message to MCP2515 failed");

  return result;
}

internal esp_err_t pvc_spitcan_write_message (
  spi_device_handle_t *device, pvc_spitcan_message *frame,
  pvc_spitcan_message_priority priority)
{
  ASSERT(frame->length_in_bytes <= 0x04,
    "Spitcan message exceeds the 4 byte limit.");
  ASSERT(priority < MAX_PRIORITY, "Unkown spitcan priority: %d", priority);

  esp_err_t result = ESP_FAIL; // assumes failed transmission. 

  //- rhjr: message information
  // MCP2515-manual: MESSAGE TRANSMISSION pg. 15

  // rhjr: message transmit request
  pvc_spitcan_write_to_register(REGISTER_TXB0CTRL, TXB_TXREQ, TXB_TXREQ);

  // rhjr: message priority -> low priority
  pvc_spitcan_write_to_register(REGISTER_TXB0CTRL, TXB_TXP, 0x01);

  //- rhjr: message identification
  uint8_t txb_sidn[2] = {0}; 
  pvc_spitcan_message_set_identification(txb_sidn, frame);

  pvc_spitcan_write_to_register(REGISTER_TXB0SIDH, 0xFF, txb_sidn[0]);
  pvc_spitcan_write_to_register(REGISTER_TXB0SIDL, 0xFF, txb_sidn[1]);

  //- rhjr: message data
  //pvc_spitcan_write_to_registers( // rhjr: 5 byte offset from identification.  
  //REGISTER_TXB0SIDH, 69, frame->length_in_bytes + 5); 
  
  //- rhjr: message status confirmation
  uint8_t status_txb_ctrl;
  pvc_spitcan_read_from_register(REGISTER_TXB0CTRL, &status_txb_ctrl);

  // rhjr: check if error bits are set.
  if ((status_txb_ctrl & (TXB_ABTF | TXB_MLOA | TXB_TXERR)) != 0)
  {
    LOG(TAG_SPI, ERROR,
      "Writing message to MCP2515 failed, register dump: %d", status_txb_ctrl);
    result = ESP_ERR_INVALID_RESPONSE;
  }
  else
  {
    LOG(TAG_SPI, INFO, "Message succesfully send...");
  }
  
  return result;
}

internal esp_err_t
pvc_spitcan_read_message (
  pvc_spitcan_message *destination, spi_device_handle_t source)
{
  esp_err_t result;

  uint32_t identifier;
  uint8_t amount_of_received_bytes;

  //- rhjr: identifier reconstruction
  uint8_t identifier_fragments[2];
  pvc_spitcan_read_from_registers(REGISTER_RXB0SIDH, identifier_fragments, 2);

  identifier = (identifier_fragments[0] << 3) + (identifier_fragments[1] >> 5);

  //- rhjr: amount of bytes received
  const uint8_t dlc_mask = 0x0F;
  pvc_spitcan_read_from_register(REGISTER_RXB0DLC, &amount_of_received_bytes);

  amount_of_received_bytes &= dlc_mask;

  if (amount_of_received_bytes == 0)
  {
    result = ESP_ERR_INVALID_SIZE;
    return result;
  }

  //- rhjr: received data
  pvc_spitcan_read_from_registers(
    REGISTER_RXB0DM, destination->data, amount_of_received_bytes);

  //- rhjr: can-frame reconstruction
  destination->id = identifier;
  destination->length_in_bytes = amount_of_received_bytes;

  // rhjr: clear the RXBn, to allow new messages.  
  pvc_spitcan_write_to_register(REGISTER_CANINTF, REGISTER_CANINTF, 0);

  return result;
}

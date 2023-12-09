global_variable spi_device_handle_t mcp2515;

//= rhjr: spitcan device interfacing

internal esp_err_t 
pvc_spitcan_initalize ()
{
  LOG(TAG_SPITCAN, INFO, "Initializing Spitcan...");

  esp_err_t result;

  //- rhjr: transaction
  spi_bus_config_t spi_bus_config =
    {
      .sclk_io_num = PVC_SPITCAN_PIN_SCK,
      .mosi_io_num = PVC_SPITCAN_PIN_SDO,
      .miso_io_num = PVC_SPITCAN_PIN_SDI,
      .quadwp_io_num = -1, // UNUSED
      .quadhd_io_num = -1, // UNUSED
    };

  //- rhjr: transmission
  result =
    spi_bus_initialize(PVC_SPITCAN_PIN, &spi_bus_config, SPI_DMA_DISABLED);
  ASSERT(result == ESP_OK, "Initializing the SPI bus failed.");
  
  //- rhjr: initializing mcp2515
  LOG(TAG_SPITCAN, INFO, "  - Initializing MCP2515...");
  LOG(TAG_SPITCAN, INFO, "    + Adding MCP2515 to the SPI bus");

  result = pvc_spitcan_add_device(PVC_SPITCAN_PIN, &mcp2515);

  ASSERT(result == ESP_OK, "Adding the MCP2515 to the SPI bus failed.");

  LOG(TAG_SPITCAN, INFO, "    + Configuring MCP2515");

  pvc_spitcan_device_set_mode(mcp2515, MODE_CONFIG);
  pvc_spitcan_reset_device(mcp2515);
  
  // rhjr: set MCP2515 transmission speed.
  pvc_spitcan_set_register(REGISTER_CANF1, 0x01);
  pvc_spitcan_set_register(REGISTER_CANF2, 0xBF);
  pvc_spitcan_set_register(REGISTER_CANF3, 0x87);

  pvc_spitcan_device_set_mode(mcp2515, MODE_NORMAL);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  LOG(TAG_SPITCAN, INFO, "Spitcan succesfully initialized.");
  return result;
}

internal esp_err_t
pvc_spitcan_add_device (spi_host_device_t spi_host, spi_device_handle_t *device)
{
  ASSERT(spi_host < SPI_HOST_MAX, "Invalid SPI host");
  ASSERT(PVC_SPITCAN_SCK_FREQ < SPI_MASTER_FREQ_20M, 
    "PVC_SPI_PIN_CLOCK_SPEED exceeds the allowed clock of MCP2515.");

  esp_err_t result;
  spi_device_interface_config_t spi_device_config = {0};  

  spi_device_config.mode           = 0;
  spi_device_config.clock_speed_hz = PVC_SPITCAN_SCK_FREQ;
  spi_device_config.spics_io_num   = PVC_SPITCAN_PIN_CS0;
  spi_device_config.queue_size     = 7;
  spi_device_config.duty_cycle_pos = 128; // rhjr: 50% 

  result =
    spi_bus_add_device(spi_host, &spi_device_config, device);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);

  result =
    spi_device_acquire_bus(*device, portMAX_DELAY);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  return result;
}

internal esp_err_t
pvc_spitcan_reset_device (spi_device_handle_t device)
{
  esp_err_t result;
  
  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  transaction.flags      = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.tx_data[0] = INSTRUCTION_RESET;
  transaction.length     = BYTES(1);
  
  //- rhjr: transmission
  result = // rhjr: blocks until transaction is completed.
    spi_device_polling_transmit(device, &transaction);

  //- rhjr: clearing register
  uint8_t cleared_bits[PVC_SPITCAN_TXnCTRL_SIZE];
  memset(cleared_bits, 0, sizeof(cleared_bits));

  pvc_spitcan_set_registers(
    REGISTER_TXB0CTRL, cleared_bits, PVC_SPITCAN_TXnCTRL_SIZE);

  // rhjr: clear control register
  pvc_spitcan_set_register(REGISTER_RXB0CTRL, 0);

  // rhjr: clear interupts flags
  pvc_spitcan_set_register(REGISTER_CANINTE, 0);

  // rhjr: 
  const uint8_t turn_off_filters = 96;
  pvc_spitcan_modify_register(
    REGISTER_RXB0CTRL, turn_off_filters, turn_off_filters);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  return result;
}

internal esp_err_t
pvc_spitcan_device_set_mode (spi_device_handle_t host, pvc_mcp_mode mode)
{
  ASSERT(mode < MODE_MAX, "Mode %u is not available.", mode);

  esp_err_t result;
  uint8_t canctrl_reqop_mask = 0xE0;  

  result = pvc_spitcan_modify_register(
    REGISTER_CANCTRL, canctrl_reqop_mask, mode);

  ASSERT(result == ESP_OK, "Unexpected result, error code: %d", result);
  return result;
}

//= rhjr: spitcan register manipulation

internal esp_err_t
pvc_spitcan_read_register(const pvc_mcp_register _register, uint8_t *data)
{
  esp_err_t result;

  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  transaction.flags  = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.length = BYTES(3);

  transaction.tx_data[0] = INSTRUCTION_READ;
  transaction.tx_data[1] = _register;
  transaction.tx_data[2] = 0x00;

  //- rhjr: transmission
  result = spi_device_polling_transmit(mcp2515, &transaction);
  memcpy(data, &transaction.rx_data[2], sizeof(uint8_t));

  ASSERT(result == ESP_OK, "Transmission failed, error code: %u", result); 
  return result;
}

internal esp_err_t
pvc_spitcan_read_registers (
  const pvc_mcp_register _register, uint8_t *data, uint8_t length_in_bytes)
{
  ASSERT(length_in_bytes <= 14, "Spitcan message exceeds the 14 byte limit.");
  esp_err_t result;

  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  uint8_t tx_message[16] = {0}; // rhjr: TODO expand 
  uint8_t rx_message[16] = {0}; // rhjr: TODO expand

  transaction.flags  = 0; // rhjr: tx_data is limited to 32-bits.
  transaction.length =
    BYTES(2 + (size_t) length_in_bytes); // rhjr: inst. + register

  tx_message[0] = INSTRUCTION_READ;
  tx_message[1] = _register;

  transaction.tx_buffer = tx_message;
  transaction.rx_buffer = rx_message;

  //- rhjr: transmission
  uint8_t *data_temp = data;

  result = spi_device_polling_transmit(mcp2515, &transaction);

  for (uint8_t index = 0; index < length_in_bytes; index += 1)
    memcpy(data_temp++, &rx_message[index + 2], sizeof(uint8_t));

  ASSERT(result == ESP_OK, "Transmission failed, error code: %u", result); 
  return result;
}

internal esp_err_t
pvc_spitcan_set_register(const pvc_mcp_register _register, const uint8_t value)
{
  esp_err_t result;

  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  transaction.flags     = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.length    = BYTES(3); 

  transaction.tx_data[0] = INSTRUCTION_WRITE;
  transaction.tx_data[1] = _register;
  transaction.tx_data[2] = value;

  //- rhjr: transmission
  result = spi_device_transmit(mcp2515, &transaction);

  ASSERT(result == ESP_OK, "Transmission failed, error code: %u", result); 
  return result;
}

internal esp_err_t
pvc_spitcan_set_registers(
  const pvc_mcp_register _register, const uint8_t *data,
  const uint8_t length_in_bytes)
{
  ASSERT(length_in_bytes <= 14, "Spitcan message exceeds the 14 byte limit.");
  esp_err_t result;

  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  uint8_t message[16] = {0}; // rhjr: TODO expand

  transaction.flags  = 0; // rhjr: tx_data is limited to 32-bits.
  transaction.length = BYTES(2 + (size_t) length_in_bytes); // rhjr: inst. + register

  message[0] = INSTRUCTION_WRITE;
  message[1] = _register;

  for (uint8_t index = 0; index < length_in_bytes; index += 1)
  message[index + 2] = *data++;

  transaction.tx_buffer = message;

  //- rhjr: transmission
  result = spi_device_polling_transmit(mcp2515, &transaction);

  ASSERT(result == ESP_OK, "Transmission failed, error code: %u", result); 
  return result;
}

internal esp_err_t
pvc_spitcan_modify_register(
  const pvc_mcp_register _register, const uint8_t mask, uint8_t data)
{
  esp_err_t result;

  //- rhjr: transaction
  spi_transaction_t transaction = {0};
  transaction.flags     = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
  transaction.length    = BYTES(4); 

  transaction.tx_data[0] = INSTRUCTION_BITMOD;
  transaction.tx_data[1] = _register;
  transaction.tx_data[2] = mask;
  transaction.tx_data[3] = data;

  //- rhjr: transmission
  result = spi_device_polling_transmit(mcp2515, &transaction);

  ASSERT(result == ESP_OK, "Transmission failed, error code: %u", result); 
  return result;
}

//= rhjr: spitcan message frame

internal void
pvc_spitcan_set_message_identification (
  uint8_t *sidn_buffer, pvc_spitcan_message *frame)
{
  // rhjr: The frame identifier is a 12-bit field. Where there is a 11-bit field
  //       for the identification and a 1-bit field for an unused flag.
  uint16_t identifier = (uint16_t)(frame->identifier & 0x0FFFF);

  // rhjr: Use standard frame format for the CAN-protocol.
  identifier &= PVC_CAN_SFF_MASK;

  *sidn_buffer++ = identifier >> 3;
  *sidn_buffer = (identifier & 0x07) << 5;

  #if 0
  LOG(TAG_SPITCAN, INFO, "SIDL register dump: %u", *sidn_buffer-- );
  LOG(TAG_SPITCAN, INFO, "SIDH register dump: %u", *sidn_buffer);
#endif
}

internal esp_err_t
pvc_spitcan_write_message (
  pvc_spitcan_message *message, pvc_spitcan_message_priority priority)
{
  ASSERT(priority < MAX_PRIORITY, "Unkown spitcan priority: %d", priority);

  esp_err_t result;

  //- rhjr: check transmission buffer
  uint8_t received_message = 0;
  pvc_spitcan_read_register(REGISTER_TXB0CTRL, &received_message);

  if ((received_message & TXB_TXREQ) == TXB_TXREQ)
  {
    LOG(TAG_SPITCAN, WARNING, "Message queue is full.");
    return ESP_ERR_INVALID_SIZE;
  }

  //- rhjr: message information
  // MCP2515-manual: MESSAGE TRANSMISSION pg. 15

  // rhjr: set message identification
  uint8_t txb_sidn_buffer[2] = {0}; 
  pvc_spitcan_set_message_identification(txb_sidn_buffer, message);

  pvc_spitcan_set_register(REGISTER_TXB0SIDH, txb_sidn_buffer[0]);
  pvc_spitcan_set_register(REGISTER_TXB0SIDL, txb_sidn_buffer[1]);

  // rhjr: fill message with data
  uint8_t data = 0;
  memcpy(&data, &message->data, message->length_in_bytes);

  pvc_spitcan_set_register(REGISTER_TXB0DM, data);
  pvc_spitcan_modify_register(REGISTER_TXB0DLC, 0x4F, message->length_in_bytes);

  // rhjr: request for transmission 
  pvc_spitcan_modify_register(REGISTER_TXB0CTRL, TXB_TXP,   priority);
  pvc_spitcan_modify_register(REGISTER_TXB0CTRL, TXB_TXREQ, TXB_TXREQ);
  
  // rhjr: clear interrupt
  pvc_spitcan_set_register(REGISTER_CANINTE, 0x0);

  //- rhjr: message status confirmation
  uint8_t status_txb_ctrl;
  pvc_spitcan_read_register(REGISTER_TXB0CTRL, &status_txb_ctrl);

  // rhjr: check if error bits are set.
  if ((status_txb_ctrl & (TXB_ABTF | TXB_MLOA | TXB_TXERR)) != 0)
  {
    uint8_t error_flags;
    pvc_spitcan_read_register(REGISTER_EFLG, &error_flags);

    LOG(TAG_SPITCAN, ERROR, "Writing message failed");
    LOG(TAG_SPITCAN, ERROR,
      "  /-> Status register dump -> %u", status_txb_ctrl);
    LOG(TAG_SPITCAN, ERROR,
      "  /-> Error flags register dump -> %u", error_flags);

    result = ESP_ERR_INVALID_RESPONSE;
    return result;
  }

  result = ESP_OK;
  return result;
}

internal pvc_spitcan_message *  
pvc_spitcan_read_message(pvc_arena *arena)
{
  pvc_spitcan_message *msg =
    pvc_memory_push(arena, pvc_spitcan_message, 1);

  //- rhjr: identifier reconstruction

  uint8_t identifier_fragments[2];
  pvc_spitcan_read_register(REGISTER_RXB0SIDH, &identifier_fragments[0]);
  pvc_spitcan_read_register(REGISTER_RXB0SIDL, &identifier_fragments[1]);

  msg->identifier =
    (identifier_fragments[0] << 3) + (identifier_fragments[1] >> 5);

  //- rhjr: data length code

  pvc_spitcan_read_register(REGISTER_RXB0DLC, &msg->length_in_bytes);
  msg->length_in_bytes &= PVC_CAN_DLC_MASK;

  if (msg->length_in_bytes == 0)
    return NULL;

  //- rhjr: data buffer

  // rhjr: TODO pvc_spitcan_read_registers() should be used for 32-bit support.
  pvc_spitcan_read_register(REGISTER_RXB0DM, &msg->data);

  // rhjr: clear the RXBn, to allow new messages.  
  pvc_spitcan_set_register(REGISTER_CANINTF, 0);

  return msg;
}

internal bool
pvc_spitcan_received_new_message ()
{
  uint8_t result = 0;
  uint8_t rxb0if_mask = 0x01;

  pvc_spitcan_read_register(REGISTER_CANINTF, &result);

  if ((result & rxb0if_mask) == 1)
    return true;
  return false;
}

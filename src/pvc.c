#include "pvc.h"

//= rhjr: logging

internal void
_pvc_monitor_stdout_log(
  pvc_monitor_tag tag, pvc_monitor_type type, const char* format, ...)
{
  const uint8_t message_size = 128;
  char message[message_size];
  va_list args_list;

  va_start(args_list, format);
  vsnprintf(message, message_size, format, args_list);
  va_end(args_list);

  fprintf(stdout, "[DEBUG][%s] (%s) %s\n",
    _pvc_monitor_type_table[type], _pvc_monitor_tag_table[tag], message);
}

//= rhjr: abort & assertion

internal _Noreturn uintptr_t
_pvc_monitor_assert (
  const char* condition, const char* file, uint32_t line,
  const char *format, ...)
{
  const uint8_t message_size = 128;
  char message[message_size];
  va_list args_list;

  va_start(args_list, format);
  vsnprintf(message, message_size, format, args_list);
  va_end(args_list);

  fprintf(stderr, "[DEBUG][%s] (%s) \"%s\" at %s::%ld: %s.\n",
    _pvc_monitor_type_table[ERROR], _pvc_monitor_tag_table[TAG_ASSERT],
    condition, file, line, message);

  fflush(stdout);
  abort();
}

//= rhjr: can-bus

esp_err_t
pvc_spitcan_master_initialize (spi_host_device_t spi_host)
{
  ASSERT(spi_host < SPI_HOST_MAX, "Invalid SPI host");
  LOG(TAG_SPI, INFO, "Initializing SPI-master");

  esp_err_t result;
  spi_bus_config_t spi_bus_config = {
    .sclk_io_num = PVC_SPI_PIN_SCK,
    .mosi_io_num = PVC_SPI_PIN_SDO,
    .miso_io_num = PVC_SPI_PIN_SDI
  };

  result =
    spi_bus_initialize(spi_host, &spi_bus_config, SPI_DMA_CH_AUTO);

  ASSERT(result == ESP_OK, "Unexpected result");
  return result;
}

//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
void my_post_setup_cb(spi_slave_transaction_t *trans)
{
  gpio_set_level(PVC_SPI_PIN_INT, 1);
}

//Called after transaction is sent/received. We use this to set the handshake line low.
void my_post_trans_cb(spi_slave_transaction_t *trans)
{
  gpio_set_level(PVC_SPI_PIN_INT, 0);
}

esp_err_t
pvc_spitcan_slave_initialize (spi_host_device_t spi_host)
{
  ASSERT(spi_host < SPI_HOST_MAX, "Invalid SPI host");
  LOG(TAG_SPI, INFO, "Initializing SPI-slave");

  esp_err_t result;
  spi_bus_config_t spi_bus_config = {
    .sclk_io_num = PVC_SPI_PIN_SCK,
    .mosi_io_num = PVC_SPI_PIN_SDO,
    .miso_io_num = PVC_SPI_PIN_SDI,
    .quadwp_io_num = -1, // UNUSED
    .quadhd_io_num = -1, // UNUSED
  };

  spi_slave_interface_config_t spi_slave_config = {
    .mode         = PVC_SPI_MODE,
    .spics_io_num = PVC_SPI_PIN_CSB,
    .queue_size   = PVC_SPI_QUEUE_LEN,
    .post_setup_cb = my_post_setup_cb,
    .post_trans_cb = my_post_trans_cb
  };

  gpio_config_t spi_handshake_config = {
    .mode         = GPIO_MODE_OUTPUT,
    .intr_type    = GPIO_INTR_DISABLE,
    .pin_bit_mask = BIT64(PVC_SPI_PIN_INT),
  };

  gpio_config(&spi_handshake_config);

  // rhjr: enable pull-ups, so no rogue pulses are created, when the handshake
  //       isn't completed.
  gpio_set_pull_mode(PVC_SPI_PIN_SDO, GPIO_PULLUP_ONLY); 
  gpio_set_pull_mode(PVC_SPI_PIN_SDI, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(PVC_SPI_PIN_CSB, GPIO_PULLUP_ONLY);

  result =
    spi_slave_initialize(
      spi_host, &spi_bus_config, &spi_slave_config, SPI_DMA_CH_AUTO);

  ASSERT(result == ESP_OK, "Unexpected result");
  return result;
}

esp_err_t
pvc_spitcan_add_device (spi_host_device_t spi_host, spi_device_handle_t *device)
{
  // rhjr: Esp32 allows a maximum of three devices (slaves).
  ASSERT(spi_host < SPI_HOST_MAX, "Invalid SPI host");
  ASSERT(PVC_SPI_SCK_FREQ < SPI_MASTER_FREQ_20M, 
         "PVC_SPI_PIN_CLOCK_SPEED exceeds the allowed clock of MCP2515.");

  esp_err_t result;
  
  // rhjr: c-style null initialization?
  spi_device_interface_config_t spi_device_config = {0};  
  spi_device_config.clock_speed_hz = PVC_SPI_SCK_FREQ;
  spi_device_config.spics_io_num   = PVC_SPI_PIN_CS0;
  spi_device_config.mode           = PVC_SPI_MODE;
  spi_device_config.queue_size     = PVC_SPI_QUEUE_LEN;

  result =
    spi_bus_add_device(spi_host, &spi_device_config , device);

  LOG(TAG_SPI, INFO, "  - Added one device.");

  ASSERT(result == ESP_OK, "Unexpected result");
  return result;
}

esp_err_t
pvc_spitcan_transmit (
  spi_device_handle_t device, const uint8_t *data, uint32_t length_in_bytes)
{
  ASSERT(length_in_bytes > 0, "expected at least a length of 1");

  esp_err_t result;
  spi_transaction_t transaction = {0};

  transaction.length = length_in_bytes * 8; 
  transaction.tx_buffer = data;
  transaction.user = (void*) 1;

  result = // rhjr: blocks until transaction is completed.
    spi_device_polling_transmit(device, &transaction);

  LOG(TAG_SPI, INFO, "%s",
    (result == ESP_OK) ? "Message succesfully send" : "Sending message failed");

  ASSERT(result == ESP_OK, "Unexpected result");
  return result;
}

//= rhjr: application

void
app_main (void)
{
  LOG(TAG_PLATFORM, INFO, "Initializing");

  //- rhjr: serial communication
  spi_device_handle_t spi_device;
  pvc_spitcan_initialize(PVC_SPI_PIN);

  //pvc_spitcan_add_device(PVC_SPI_PIN, &spi_device);

  //uint8_t data = 69;

  //pvc_spitcan_transmit(spi_device, &data, 1);

  //- rhjr: application loop
  char buffer[128];
  memset(&buffer, 0, 128);

  spi_slave_transaction_t t = {0};

  for (;;)
  {
    memset(&buffer, 0, 128);
    t.length = 128 * 8;
    t.rx_buffer = buffer;

    spi_slave_transmit(PVC_SPI_PIN, &t, portMAX_DELAY);

    printf("Received: %s\n", buffer);
  }

}

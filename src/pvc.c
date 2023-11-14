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
spitcan_initialize (spi_host_device_t spi_host)
{
  ASSERT(spi_host < SPI_HOST_MAX, "Invalid SPI host");
  LOG(INFO, TAG_SPI, "Initializing");

  esp_err_t result;
  spi_bus_config_t spi_bus_config = {
    .sclk_io_num = PVC_SPI_PIN_CLOCK,
    .miso_io_num = PVC_SPI_PIN_MISO,
    .mosi_io_num = PVC_SPI_PIN_MOSI
  };

  result =
    spi_bus_initialize(spi_host, &spi_bus_config, SPI_DMA_CH_AUTO);

  ASSERT(result == ESP_OK, "Unexpected result");
  return result;
}

esp_err_t
spitcan_add_device (spi_host_device_t spi_host, spi_device_handle_t *device)
{
  // rhjr: Esp32 allows a maximum of three devices (slaves).
  ASSERT(spi_host < SPI_HOST_MAX, "Invalid SPI host");
  ASSERT(PVC_SPI_PIN_CLOCK_SPEED < SPI_MASTER_FREQ_20M, 
         "PVC_SPI_PIN_CLOCK_SPEED exceeds the allowed clock of MCP2515.");

  esp_err_t result;
  
  // rhjr: c-style null initialization?
  spi_device_interface_config_t spi_device_config = {0};  
  spi_device_config.clock_speed_hz = SPI_CLK_SRC_DEFAULT;
  spi_device_config.spics_io_num = PVC_SPI_PIN_CS0;
  spi_device_config.mode = PVC_SPI_MODE;
  spi_device_config.queue_size = 7;

  result =
    spi_bus_add_device(spi_host, &spi_device_config , device);

  LOG(INFO, TAG_SPI, "  - Added one device.");

  ASSERT(result == ESP_OK, "Unexpected result");
  return result;
}

esp_err_t
spitcan_transmit (
  spi_device_handle_t device, const uint8_t *data, uint32_t length_in_bytes)
{
  ASSERT(length_in_bytes > 0, "expected at least a length of 1");

  esp_err_t result;
  spi_transaction_t transaction = {0};

  transaction.length = length_in_bytes * 8; 
  transaction.tx_buffer = data;
  transaction.user = (void*) 1;

  // rhjr: TODO add polling.
  result =
    spi_device_transmit(device, &transaction);

  ASSERT(result == ESP_OK, "Unexpected result");
  return result;
}

//= rhjr: application

void
app_main (void)
{
  LOG(INFO, TAG_PLATFORM, "Initializing");

  //- rhjr: serial communication
  spi_device_handle_t spi_device;
  spitcan_initialize(PVC_SPI_PIN);

  spitcan_add_device(PVC_SPI_PIN, &spi_device);

  uint8_t data = 69;

  spitcan_transmit(spi_device, &data, 1);

  //- rhjr: application loop
  for (;;)
  {
    
  }

}

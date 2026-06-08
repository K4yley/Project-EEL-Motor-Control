#include <Arduino.h>

// ESP32 Hardware Serial 2 pins
#define RXD2 16
#define TXD2 17

void setup() {
  // Setup UART buffered IO with event queue
  const int uart_buffer_size = 64;
  QueueHandle_t uart_queue;
  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

  const uart_port_t uart_num = UART_NUM_2;
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
      .rx_flow_ctrl_thresh = 122,
  };
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));


  // Start computer monitor serial
  Serial.begin(115200);
  
  // Start UART2 for Pico (Baud rate: 115200)
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  
  Serial.println("ESP32 UART Initialized. Waiting for Pico...");
}

void loop() {
  // Send data to Pico
  Serial2.println("Hello Pico!");
  Serial.println("Sent: Hello Pico!");

  // Wait briefly for a response
  delay(500);

  // Check if Pico sent anything back
  while (Serial2.available() > 0) {
    String response = Serial2.readStringUntil('\n');
    Serial.print("Received from Pico: ");
    Serial.println(response);
  }

  // Delay before sending the next message
  delay(1500);
}

#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

void setup() {
    a2dp_sink.set_stream_reader(read_data_stream);
    a2dp_sink.set_on_data_received(data_received_callback);
    a2dp_sink.start("ESP32");
}

void read_data_stream(const uint8_t *data, uint32_t length) {
  ESP_LOGI(BT_AV_TAG, "Data stream received, data: %d ", data[0]);
}

void data_received_callback() {
  //ESP_LOGI(BT_AV_TAG, "Data packet received");
}

void loop() {
}

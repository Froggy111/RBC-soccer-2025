#include <chrono>
#include <iostream>
#include <vector>
#include <libusb-1.0/libusb.h>
#include <algorithm>
#include <numeric>

struct LatencyPacket {
  uint32_t start_timestamp;
  uint32_t sequence;
};

class LatencyTest {
private:
  libusb_device_handle* dev_handle;
  const int ENDPOINT_IN = 0x81;
  const int ENDPOINT_OUT = 0x01;
  const int PACKET_SIZE = sizeof(LatencyPacket);
  const int NUM_ITERATIONS = 1000;

public:
  LatencyTest() {
    libusb_init(NULL);
    dev_handle = libusb_open_device_with_vid_pid(NULL, 0x2E8A, 0x000A);
    if (!dev_handle) {
      throw std::runtime_error("Could not open device");
    }
  }

  void runLatencyTest() {
    std::vector<uint32_t> latencies;

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
      LatencyPacket tx_packet{}, rx_packet{};

      // First ping - send start timestamp
      tx_packet.sequence = i;
      tx_packet.start_timestamp = std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();

      int transferred;

      // Send first ping
      libusb_bulk_transfer(dev_handle, ENDPOINT_OUT, 
                           reinterpret_cast<unsigned char*>(&tx_packet), PACKET_SIZE, 
                           &transferred, 0);

      // Send second ping to trigger round-trip calculation
      libusb_bulk_transfer(dev_handle, ENDPOINT_OUT, 
                           reinterpret_cast<unsigned char*>(&tx_packet), PACKET_SIZE, 
                           &transferred, 0);

      // Receive round-trip time from Pico
      libusb_bulk_transfer(dev_handle, ENDPOINT_IN, 
                           reinterpret_cast<unsigned char*>(&rx_packet), PACKET_SIZE, 
                           &transferred, 0);

      latencies.push_back(rx_packet.start_timestamp);
    }

    // Calculate statistics
    std::sort(latencies.begin(), latencies.end());

    double avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    double median_latency = latencies[latencies.size() / 2];
    uint32_t min_latency = latencies.front();
    uint32_t max_latency = latencies.back();

    std::cout << "Latency Statistics (microseconds):" << std::endl;
    std::cout << "Average: " << avg_latency << std::endl;
    std::cout << "Median: " << median_latency << std::endl;
    std::cout << "Min: " << min_latency << std::endl;
    std::cout << "Max: " << max_latency << std::endl;
  }

  ~LatencyTest() {
    libusb_close(dev_handle);
    libusb_exit(NULL);
  }
};

int main() {
  LatencyTest test;
  test.runLatencyTest();
  return 0;
}

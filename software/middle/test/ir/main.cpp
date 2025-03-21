#include "comms.hpp"
#include "types.hpp"
#include "TSSP4038.hpp"
#include "hardware/gpio.h"
#include "hardware/timer.h"

using namespace types;

const u8 LED_PIN = 25;
IRSensor ir_sensor = IRSensor(100);

void ir_sensor_poll_task(void *args) { 
  comms::USB_CDC.printf("Initialising IR sensors...\n");

  // initialise each IR pin as input 
  for (int i = 0; i < num_ir_sensors; i++) {
      gpio_init(ir_pins[i]); // initialise the GPIO pin
      gpio_set_dir(ir_pins[i], GPIO_IN); // set the dir to input
      ir_sensor.ir_samples[i] = new IRSensor(samples_per_window);  

      //combined callback:
      gpio_set_irq_enabled_with_callback(
        ir_pins[i],
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        (gpio_irq_callback_t)IRSensor::gpio_callback
        );
      
      comms::USB_CDC.printf("IR Sensor %d initialized at GPIO pin %d\n", i + 1, ir_pins[i]);
      }
      //start mod timer
      add_repeating_timer_ms(833, IRSensor::modulation_timer_callback, NULL, &modulation_timer); 

      while (true){ 
          uint64_t start_time = time_us_64();

          for (int i = 0; i < num_ir_sensors; i++) {
              bool state = gpio_get(ir_pins[i]); 
              ir_sensor.ir_samples[i]->add(state); 
              
              // Output the state and average for each IR sensor
              comms::USB_CDC.printf(">IR%d: s: %d\n", i + 1, state);
              comms::USB_CDC.printf(">IR%d: avg: %.2f\n", i + 1, ir_sensor.ir_samples[i]->average());
          }
          while (time_us_64() - start_time < period) { 
              continue; 
          }
      }

} 

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  comms::USB_CDC.init();

  xTaskCreate(ir_sensor_poll_task, "ir_sensor_poll_task", 1024, NULL, 10,
              NULL);

  vTaskStartScheduler();
  return 0;
}

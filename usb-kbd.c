#include <stdio.h>
#include <string.h>

#include "boards/seeed_xiao_rp2040.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#include "pio_usb.h"

#define LED_R 17
#define LED_G 16
#define LED_B 25 
// LED is OFF when Pin is HIGH

const uint LED_PINS[] = {LED_R, LED_G, LED_B};
static usb_device_t *usb_device = NULL;

void core1_main() {
  //sleep_ms(10);

  // To run USB SOF interrupt in core1, create alarm pool in core1.  
  static pio_usb_configuration_t config = PIO_USB_DEFAULT_CONFIG;
  //config.pinout = PIO_USB_PINOUT_DMDP;
  config.alarm_pool = (void*)alarm_pool_create(2, 1);
  usb_device = pio_usb_host_init(&config);

  //// Call pio_usb_host_add_port to use multi port
  // const uint8_t pin_dp2 = 8;
  // pio_usb_host_add_port(pin_dp2);

  while (true) {
    pio_usb_host_task();
  }
}

static void flash_LED(unsigned int pin,unsigned int length_ms) {
  gpio_put(pin,0);
  sleep_ms(length_ms);
  gpio_put(pin,1);
  sleep_ms(length_ms);
  return;
}

void parse_ascii(uint8_t byte) {
  uint8_t inp = 0;
  inp = byte+93;
  if (inp>122 || inp < 97) {
    printf("input: <invalid>\n");
    return;
  }
  printf("input: %c\n",inp);
}

int main() {
  // default 125MHz is not appropreate. Sysclock should be multiple of 12MHz.
  set_sys_clock_khz(120000, true);
  sleep_ms(3000);
  stdio_init_all();
  printf("hello!");

  multicore_reset_core1();
  // all USB task run in core1
  multicore_launch_core1(core1_main);
  for (unsigned short int i = 0; i < 3; i++) {
    gpio_init(LED_PINS[i]);
    gpio_set_dir(LED_PINS[i], GPIO_OUT);
    gpio_put(LED_PINS[i],1); // turn all lights off
  }


  while (true) {
    if (usb_device != NULL) {
      for (int dev_idx = 0; dev_idx < PIO_USB_DEVICE_CNT; dev_idx++) {
        usb_device_t *device = &usb_device[dev_idx];
        if (!device->connected) {
          //flash_LED(LED_R,30);
          //printf("dev not connected\n");
          continue;
        }

        // Print received packet to EPs
        for (int ep_idx = 0; ep_idx < PIO_USB_DEV_EP_CNT; ep_idx++) {
          endpoint_t *ep = pio_usb_get_endpoint(device, ep_idx);

          if (ep == NULL) {
            break;
          }

          uint8_t temp[64];
          int len = pio_usb_get_in_data(ep, temp, sizeof(temp));

          if (len > 0) {
            flash_LED(LED_G, 20);
            printf("%04x:%04x EP 0x%02x:\t", device->vid, device->pid,
                   ep->ep_num);
            for (int i = 0; i < len; i++) {
              printf("%02x ", temp[i]);
            }
            
            printf("\n");
            //parse_ascii(temp[2]);
          }
          else {
            flash_LED(LED_B,1);
          }
        }
      } 
    } else {
      printf("USB DEV NULL\n");
    }
    stdio_flush();
  }
}
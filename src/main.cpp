// Auto generated code by esphome
// ========== AUTO GENERATED INCLUDE BLOCK BEGIN ===========
#include "esphome.h"
using namespace esphome;
using namespace time;
using namespace switch_;
using namespace text_sensor;
logger::Logger *logger_logger;
web_server_base::WebServerBase *web_server_base_webserverbase;
captive_portal::CaptivePortal *captive_portal_captiveportal;
wifi::WiFiComponent *wifi_wificomponent;
ota::OTAComponent *ota_otacomponent;
api::APIServer *api_apiserver;
using namespace api;
sntp::SNTPComponent *sntp_time;
restart::RestartSwitch *restart_restartswitch;
gpio::GPIOSwitch *out_a;
gpio::GPIOSwitch *out_b;
gpio::GPIOSwitch *out_usb;
text_sensor::TextSensor *uptime;
#include "smartaps.h"
// ========== AUTO GENERATED INCLUDE BLOCK END ==========="

void setup() {
  // ===== DO NOT EDIT ANYTHING BELOW THIS LINE =====
  // ========== AUTO GENERATED CODE BEGIN ===========
  // async_tcp:
  // esphome:
  //   name: smartaps
  //   platform: ESP32
  //   board: esp32dev
  //   includes:
  //   - smartaps_files/smartaps.h
  //   platformio_options: {}
  //   arduino_version: espressif32@1.11.0
  //   build_path: smartaps
  //   libraries: []
  App.pre_setup("smartaps", __DATE__ ", " __TIME__);
  // time:
  // switch:
  // text_sensor:
  // logger:
  //   id: logger_logger
  //   tx_buffer_size: 512
  //   level: DEBUG
  //   logs: {}
  //   hardware_uart: UART0
  //   baud_rate: 115200
  logger_logger = new logger::Logger(115200, 512, logger::UART_SELECTION_UART0);
  logger_logger->pre_setup();
  App.register_component(logger_logger);
  // web_server_base:
  //   id: web_server_base_webserverbase
  web_server_base_webserverbase = new web_server_base::WebServerBase();
  App.register_component(web_server_base_webserverbase);
  // captive_portal:
  //   id: captive_portal_captiveportal
  //   web_server_base_id: web_server_base_webserverbase
  captive_portal_captiveportal = new captive_portal::CaptivePortal(web_server_base_webserverbase);
  App.register_component(captive_portal_captiveportal);
  // wifi:
  //   manual_ip:
  //     static_ip: 192.168.1.23
  //     gateway: 192.168.1.1
  //     subnet: 255.255.255.0
  //     dns1: 192.168.1.1
  //     dns2: 8.8.8.8
  //   ap:
  //     ssid: Smartaps Fallback Hotspot
  //     password: Cjy08060
  //     id: wifi_wifiap
  //     ap_timeout: 1min
  //   power_save_mode: LIGHT
  //   id: wifi_wificomponent
  //   fast_connect: false
  //   reboot_timeout: 15min
  //   domain: .local
  //   networks:
  //   - ssid: West Princess
  //     password: yiyangde
  //     priority: 0.0
  //     id: wifi_wifiap_2
  //   use_address: 192.168.1.23
  wifi_wificomponent = new wifi::WiFiComponent();
  wifi_wificomponent->set_use_address("192.168.1.23");
  wifi::WiFiAP wifi_wifiap_2 = wifi::WiFiAP();
  wifi_wifiap_2.set_ssid("West Princess");
  wifi_wifiap_2.set_password("yiyangde");
  wifi_wifiap_2.set_manual_ip(wifi::ManualIP{
      .static_ip = IPAddress(192, 168, 1, 23),
      .gateway = IPAddress(192, 168, 1, 1),
      .subnet = IPAddress(255, 255, 255, 0),
      .dns1 = IPAddress(192, 168, 1, 1),
      .dns2 = IPAddress(8, 8, 8, 8),
  });
  wifi_wifiap_2.set_priority(0.0f);
  wifi_wificomponent->add_sta(wifi_wifiap_2);
  wifi::WiFiAP wifi_wifiap = wifi::WiFiAP();
  wifi_wifiap.set_ssid("Smartaps Fallback Hotspot");
  wifi_wifiap.set_password("Cjy08060");
  wifi_wifiap.set_manual_ip(wifi::ManualIP{
      .static_ip = IPAddress(192, 168, 1, 23),
      .gateway = IPAddress(192, 168, 1, 1),
      .subnet = IPAddress(255, 255, 255, 0),
      .dns1 = IPAddress(192, 168, 1, 1),
      .dns2 = IPAddress(8, 8, 8, 8),
  });
  wifi_wificomponent->set_ap(wifi_wifiap);
  wifi_wificomponent->set_ap_timeout(60000);
  wifi_wificomponent->set_reboot_timeout(900000);
  wifi_wificomponent->set_power_save_mode(wifi::WIFI_POWER_SAVE_LIGHT);
  wifi_wificomponent->set_fast_connect(false);
  App.register_component(wifi_wificomponent);
  // ota:
  //   id: ota_otacomponent
  //   port: 3232
  //   safe_mode: true
  //   password: ''
  ota_otacomponent = new ota::OTAComponent();
  ota_otacomponent->set_port(3232);
  ota_otacomponent->set_auth_password("");
  App.register_component(ota_otacomponent);
  ota_otacomponent->start_safe_mode();
  // api:
  //   id: api_apiserver
  //   port: 6053
  //   reboot_timeout: 15min
  //   password: ''
  api_apiserver = new api::APIServer();
  App.register_component(api_apiserver);
  api_apiserver->set_port(6053);
  api_apiserver->set_password("");
  api_apiserver->set_reboot_timeout(900000);
  // time.sntp:
  //   platform: sntp
  //   id: sntp_time
  //   timezone: TZ-8
  //   servers:
  //   - 0.pool.ntp.org
  //   - 1.pool.ntp.org
  //   - 2.pool.ntp.org
  sntp_time = new sntp::SNTPComponent();
  sntp_time->set_servers("0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
  App.register_component(sntp_time);
  // switch.restart:
  //   platform: restart
  //   name: Smart APS Restart
  //   id: restart_restartswitch
  //   icon: mdi:restart
  restart_restartswitch = new restart::RestartSwitch();
  App.register_component(restart_restartswitch);
  // switch.gpio:
  //   platform: gpio
  //   name: 12V Output A
  //   pin:
  //     number: 25
  //     mode: OUTPUT
  //     inverted: false
  //   restore_mode: ALWAYS_OFF
  //   id: out_a
  //   interlock_wait_time: 0ms
  out_a = new gpio::GPIOSwitch();
  App.register_component(out_a);
  // switch.gpio:
  //   platform: gpio
  //   name: 12V Output B
  //   pin:
  //     number: 32
  //     mode: OUTPUT
  //     inverted: false
  //   restore_mode: ALWAYS_OFF
  //   id: out_b
  //   interlock_wait_time: 0ms
  out_b = new gpio::GPIOSwitch();
  App.register_component(out_b);
  // switch.gpio:
  //   platform: gpio
  //   name: USB Output
  //   pin:
  //     number: 16
  //     mode: OUTPUT
  //     inverted: false
  //   restore_mode: ALWAYS_ON
  //   id: out_usb
  //   interlock_wait_time: 0ms
  out_usb = new gpio::GPIOSwitch();
  App.register_component(out_usb);
  // text_sensor.custom:
  //   platform: custom
  //   id: smartaps
  //   lambda: !lambda |-
  //     auto sa = new SmartAPS();
  //     App.register_component(sa);
  //     return {sa};
  //   text_sensors:
  //   - id: uptime
  //     icon: mdi:heart-pulse
  //     name: Uptime
  sntp_time->set_timezone("TZ-8");
  App.register_switch(restart_restartswitch);
  restart_restartswitch->set_name("Smart APS Restart");
  restart_restartswitch->set_icon("mdi:restart");
  App.register_switch(out_a);
  out_a->set_name("12V Output A");
  App.register_switch(out_b);
  out_b->set_name("12V Output B");
  App.register_switch(out_usb);
  out_usb->set_name("USB Output");
  custom::CustomTextSensorConstructor smartaps = custom::CustomTextSensorConstructor([=]() -> std::vector<text_sensor::TextSensor *> {
      auto sa = new SmartAPS();
      sa->register_sensors();
      App.register_component(sa);
      return {sa};
  });
  uptime = smartaps.get_text_sensor(0);
  App.register_text_sensor(uptime);
  uptime->set_name("Uptime");
  uptime->set_icon("mdi:heart-pulse");
  out_a->set_pin(new GPIOPin(25, OUTPUT, false));
  out_a->set_restore_mode(gpio::GPIO_SWITCH_ALWAYS_OFF);
  out_b->set_pin(new GPIOPin(32, OUTPUT, false));
  out_b->set_restore_mode(gpio::GPIO_SWITCH_ALWAYS_OFF);
  out_usb->set_pin(new GPIOPin(16, OUTPUT, false));
  out_usb->set_restore_mode(gpio::GPIO_SWITCH_ALWAYS_ON);
  // =========== AUTO GENERATED CODE END ============
  // ========= YOU CAN EDIT AFTER THIS LINE =========
  App.setup();
}

void loop() {
  App.loop();
}

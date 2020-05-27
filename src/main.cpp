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
using namespace sensor;
using namespace api;
esp32_ble_tracker::ESP32BLETracker *bletracker;
xiaomi_lywsdcgq::XiaomiLYWSDCGQ *xiaomi_sensor1;
xiaomi_lywsdcgq::XiaomiLYWSDCGQ *studyroom_sensor;
xiaomi_lywsdcgq::XiaomiLYWSDCGQ *filament_sensor;
sntp::SNTPComponent *sntp_time;
restart::RestartSwitch *restart_restartswitch;
gpio::GPIOSwitch *out_port_a;
gpio::GPIOSwitch *out_port_b;
gpio::GPIOSwitch *out_usb;
xiaomi_ble::XiaomiListener *xiaomi_ble_xiaomilistener;
text_sensor::TextSensor *uptime;
sensor::Sensor *workshop_temperature;
sensor::Sensor *studyroom_temperature;
sensor::Sensor *filament_humidity;
sensor::Sensor *workshop_humidity;
sensor::Sensor *studyroom_humidity;
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
  //   libraries: []
  //   build_path: smartaps
  //   platformio_options: {}
  //   arduino_version: espressif32@1.11.0
  App.pre_setup("smartaps", __DATE__ ", " __TIME__);
  // time:
  // switch:
  // text_sensor:
  // logger:
  //   tx_buffer_size: 512
  //   hardware_uart: UART0
  //   baud_rate: 115200
  //   id: logger_logger
  //   level: DEBUG
  //   logs: {}
  logger_logger = new logger::Logger(115200, 512, logger::UART_SELECTION_UART0);
  logger_logger->pre_setup();
  App.register_component(logger_logger);
  // web_server_base:
  //   id: web_server_base_webserverbase
  web_server_base_webserverbase = new web_server_base::WebServerBase();
  App.register_component(web_server_base_webserverbase);
  // captive_portal:
  //   web_server_base_id: web_server_base_webserverbase
  //   id: captive_portal_captiveportal
  captive_portal_captiveportal = new captive_portal::CaptivePortal(web_server_base_webserverbase);
  App.register_component(captive_portal_captiveportal);
  // wifi:
  //   ap:
  //     ssid: Smartaps Fallback Hotspot
  //     password: Cjy08060
  //     ap_timeout: 1min
  //     id: wifi_wifiap
  //   reboot_timeout: 15min
  //   fast_connect: false
  //   power_save_mode: LIGHT
  //   domain: .local
  //   id: wifi_wificomponent
  //   networks:
  //   - ssid: West Princess
  //     password: yiyangde
  //     id: wifi_wifiap_2
  //     priority: 0.0
  //   use_address: smartaps.local
  wifi_wificomponent = new wifi::WiFiComponent();
  wifi_wificomponent->set_use_address("smartaps.local");
  wifi::WiFiAP wifi_wifiap_2 = wifi::WiFiAP();
  wifi_wifiap_2.set_ssid("West Princess");
  wifi_wifiap_2.set_password("yiyangde");
  wifi_wifiap_2.set_priority(0.0f);
  wifi_wificomponent->add_sta(wifi_wifiap_2);
  wifi::WiFiAP wifi_wifiap = wifi::WiFiAP();
  wifi_wifiap.set_ssid("Smartaps Fallback Hotspot");
  wifi_wifiap.set_password("Cjy08060");
  wifi_wificomponent->set_ap(wifi_wifiap);
  wifi_wificomponent->set_ap_timeout(60000);
  wifi_wificomponent->set_reboot_timeout(900000);
  wifi_wificomponent->set_power_save_mode(wifi::WIFI_POWER_SAVE_LIGHT);
  wifi_wificomponent->set_fast_connect(false);
  App.register_component(wifi_wificomponent);
  // ota:
  //   safe_mode: true
  //   password: ''
  //   port: 3232
  //   id: ota_otacomponent
  ota_otacomponent = new ota::OTAComponent();
  ota_otacomponent->set_port(3232);
  ota_otacomponent->set_auth_password("");
  App.register_component(ota_otacomponent);
  ota_otacomponent->start_safe_mode();
  // api:
  //   reboot_timeout: 15min
  //   password: ''
  //   port: 6053
  //   id: api_apiserver
  api_apiserver = new api::APIServer();
  App.register_component(api_apiserver);
  // sensor:
  api_apiserver->set_port(6053);
  api_apiserver->set_password("");
  api_apiserver->set_reboot_timeout(900000);
  // esp32_ble_tracker:
  //   id: bletracker
  //   scan_parameters:
  //     window: 30ms
  //     interval: 320ms
  //     active: true
  //     duration: 5min
  bletracker = new esp32_ble_tracker::ESP32BLETracker();
  App.register_component(bletracker);
  // sensor.xiaomi_lywsdcgq:
  //   platform: xiaomi_lywsdcgq
  //   id: xiaomi_sensor1
  //   mac_address: 4C:65:A8:DD:6F:59
  //   temperature:
  //     id: workshop_temperature
  //     name: Workshop Temperature
  //     accuracy_decimals: 1
  //     force_update: false
  //     icon: mdi:thermometer
  //     unit_of_measurement: °C
  //   humidity:
  //     id: workshop_humidity
  //     name: Workshop Humidity
  //     accuracy_decimals: 1
  //     force_update: false
  //     icon: mdi:water-percent
  //     unit_of_measurement: '%'
  //   esp32_ble_id: bletracker
  xiaomi_sensor1 = new xiaomi_lywsdcgq::XiaomiLYWSDCGQ();
  App.register_component(xiaomi_sensor1);
  // sensor.xiaomi_lywsdcgq:
  //   platform: xiaomi_lywsdcgq
  //   id: studyroom_sensor
  //   mac_address: 4C:65:A8:DD:2C:84
  //   temperature:
  //     id: studyroom_temperature
  //     name: Study Room Temperature
  //     accuracy_decimals: 1
  //     force_update: false
  //     icon: mdi:thermometer
  //     unit_of_measurement: °C
  //   humidity:
  //     id: studyroom_humidity
  //     name: Study Room Humidity
  //     accuracy_decimals: 1
  //     force_update: false
  //     icon: mdi:water-percent
  //     unit_of_measurement: '%'
  //   esp32_ble_id: bletracker
  studyroom_sensor = new xiaomi_lywsdcgq::XiaomiLYWSDCGQ();
  App.register_component(studyroom_sensor);
  // sensor.xiaomi_lywsdcgq:
  //   platform: xiaomi_lywsdcgq
  //   id: filament_sensor
  //   mac_address: 4C:65:A8:DD:59:FE
  //   humidity:
  //     id: filament_humidity
  //     name: Filament Humidity
  //     accuracy_decimals: 1
  //     force_update: false
  //     icon: mdi:water-percent
  //     unit_of_measurement: '%'
  //   esp32_ble_id: bletracker
  filament_sensor = new xiaomi_lywsdcgq::XiaomiLYWSDCGQ();
  App.register_component(filament_sensor);
  // time.sntp:
  //   platform: sntp
  //   id: sntp_time
  //   servers:
  //   - 0.pool.ntp.org
  //   - 1.pool.ntp.org
  //   - 2.pool.ntp.org
  //   timezone: TZ-8
  sntp_time = new sntp::SNTPComponent();
  sntp_time->set_servers("0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
  App.register_component(sntp_time);
  // switch.restart:
  //   platform: restart
  //   name: Smart APS Restart
  //   icon: mdi:restart
  //   id: restart_restartswitch
  restart_restartswitch = new restart::RestartSwitch();
  App.register_component(restart_restartswitch);
  // switch.gpio:
  //   platform: gpio
  //   name: Port A
  //   pin:
  //     number: 26
  //     mode: OUTPUT
  //     inverted: false
  //   restore_mode: RESTORE_DEFAULT_OFF
  //   id: out_port_a
  //   interlock_wait_time: 0ms
  out_port_a = new gpio::GPIOSwitch();
  App.register_component(out_port_a);
  // switch.gpio:
  //   platform: gpio
  //   name: Port B
  //   pin:
  //     number: 32
  //     mode: OUTPUT
  //     inverted: false
  //   restore_mode: RESTORE_DEFAULT_OFF
  //   id: out_port_b
  //   interlock_wait_time: 0ms
  out_port_b = new gpio::GPIOSwitch();
  App.register_component(out_port_b);
  // switch.gpio:
  //   platform: gpio
  //   name: USB
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
  //     sa->register_sensors();
  //     App.register_component(sa);
  //     return {sa};
  //   text_sensors:
  //   - id: uptime
  //     icon: mdi:heart-pulse
  //     name: Uptime
  // xiaomi_ble:
  //   id: xiaomi_ble_xiaomilistener
  //   esp32_ble_id: bletracker
  xiaomi_ble_xiaomilistener = new xiaomi_ble::XiaomiListener();
  bletracker->set_scan_duration(300);
  bletracker->set_scan_interval(512);
  bletracker->set_scan_window(48);
  bletracker->set_scan_active(true);
  sntp_time->set_timezone("TZ-8");
  App.register_switch(restart_restartswitch);
  restart_restartswitch->set_name("Smart APS Restart");
  restart_restartswitch->set_icon("mdi:restart");
  App.register_switch(out_port_a);
  out_port_a->set_name("Port A");
  App.register_switch(out_port_b);
  out_port_b->set_name("Port B");
  App.register_switch(out_usb);
  out_usb->set_name("USB");
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
  bletracker->register_listener(xiaomi_ble_xiaomilistener);
  bletracker->register_listener(xiaomi_sensor1);
  bletracker->register_listener(studyroom_sensor);
  bletracker->register_listener(filament_sensor);
  out_port_a->set_pin(new GPIOPin(26, OUTPUT, false));
  out_port_a->set_restore_mode(gpio::GPIO_SWITCH_RESTORE_DEFAULT_OFF);
  out_port_b->set_pin(new GPIOPin(32, OUTPUT, false));
  out_port_b->set_restore_mode(gpio::GPIO_SWITCH_RESTORE_DEFAULT_OFF);
  out_usb->set_pin(new GPIOPin(16, OUTPUT, false));
  out_usb->set_restore_mode(gpio::GPIO_SWITCH_ALWAYS_ON);
  xiaomi_sensor1->set_address(0x4C65A8DD6F59ULL);
  workshop_temperature = new sensor::Sensor();
  App.register_sensor(workshop_temperature);
  workshop_temperature->set_name("Workshop Temperature");
  workshop_temperature->set_unit_of_measurement("\302\260C");
  workshop_temperature->set_icon("mdi:thermometer");
  workshop_temperature->set_accuracy_decimals(1);
  workshop_temperature->set_force_update(false);
  studyroom_sensor->set_address(0x4C65A8DD2C84ULL);
  studyroom_temperature = new sensor::Sensor();
  App.register_sensor(studyroom_temperature);
  studyroom_temperature->set_name("Study Room Temperature");
  studyroom_temperature->set_unit_of_measurement("\302\260C");
  studyroom_temperature->set_icon("mdi:thermometer");
  studyroom_temperature->set_accuracy_decimals(1);
  studyroom_temperature->set_force_update(false);
  filament_sensor->set_address(0x4C65A8DD59FEULL);
  filament_humidity = new sensor::Sensor();
  App.register_sensor(filament_humidity);
  filament_humidity->set_name("Filament Humidity");
  filament_humidity->set_unit_of_measurement("%");
  filament_humidity->set_icon("mdi:water-percent");
  filament_humidity->set_accuracy_decimals(1);
  filament_humidity->set_force_update(false);
  xiaomi_sensor1->set_temperature(workshop_temperature);
  workshop_humidity = new sensor::Sensor();
  App.register_sensor(workshop_humidity);
  workshop_humidity->set_name("Workshop Humidity");
  workshop_humidity->set_unit_of_measurement("%");
  workshop_humidity->set_icon("mdi:water-percent");
  workshop_humidity->set_accuracy_decimals(1);
  workshop_humidity->set_force_update(false);
  studyroom_sensor->set_temperature(studyroom_temperature);
  studyroom_humidity = new sensor::Sensor();
  App.register_sensor(studyroom_humidity);
  studyroom_humidity->set_name("Study Room Humidity");
  studyroom_humidity->set_unit_of_measurement("%");
  studyroom_humidity->set_icon("mdi:water-percent");
  studyroom_humidity->set_accuracy_decimals(1);
  studyroom_humidity->set_force_update(false);
  filament_sensor->set_humidity(filament_humidity);
  xiaomi_sensor1->set_humidity(workshop_humidity);
  studyroom_sensor->set_humidity(studyroom_humidity);
  // =========== AUTO GENERATED CODE END ============
  // ========= YOU CAN EDIT AFTER THIS LINE =========
  App.setup();
}

void loop() {
  App.loop();
}

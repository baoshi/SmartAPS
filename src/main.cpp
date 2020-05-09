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
esp32_ble_tracker::ESP32BLETracker *esp32_ble_tracker_esp32bletracker;
xiaomi_lywsdcgq::XiaomiLYWSDCGQ *xiaomi_lywsdcgq_xiaomilywsdcgq;
xiaomi_lywsdcgq::XiaomiLYWSDCGQ *xiaomi_lywsdcgq_xiaomilywsdcgq_2;
xiaomi_lywsdcgq::XiaomiLYWSDCGQ *xiaomi_lywsdcgq_xiaomilywsdcgq_3;
sntp::SNTPComponent *sntp_time;
restart::RestartSwitch *restart_restartswitch;
gpio::GPIOSwitch *out_a;
gpio::GPIOSwitch *out_b;
gpio::GPIOSwitch *out_usb;
xiaomi_ble::XiaomiListener *xiaomi_ble_xiaomilistener;
text_sensor::TextSensor *uptime;
sensor::Sensor *study_temp;
sensor::Sensor *workshop_temp;
sensor::Sensor *filament_temp;
sensor::Sensor *study_humi;
sensor::Sensor *workshop_humi;
sensor::Sensor *filament_humi;
sensor::Sensor *study_batt;
sensor::Sensor *workshop_batt;
sensor::Sensor *filament_batt;
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
  //   libraries: []
  //   build_path: smartaps
  App.pre_setup("smartaps", __DATE__ ", " __TIME__);
  // time:
  // switch:
  // text_sensor:
  // logger:
  //   baud_rate: 115200
  //   level: DEBUG
  //   id: logger_logger
  //   logs: {}
  //   hardware_uart: UART0
  //   tx_buffer_size: 512
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
  //   ap:
  //     ssid: Smartaps Fallback Hotspot
  //     password: Cjy08060
  //     ap_timeout: 1min
  //     id: wifi_wifiap
  //   reboot_timeout: 15min
  //   domain: .local
  //   fast_connect: false
  //   id: wifi_wificomponent
  //   power_save_mode: LIGHT
  //   networks:
  //   - ssid: West Princess
  //     password: yiyangde
  //     priority: 0.0
  //     id: wifi_wifiap_2
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
  //   password: ''
  //   port: 3232
  //   safe_mode: true
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
  //   scan_parameters:
  //     interval: 320ms
  //     active: true
  //     duration: 5min
  //     window: 30ms
  //   id: esp32_ble_tracker_esp32bletracker
  esp32_ble_tracker_esp32bletracker = new esp32_ble_tracker::ESP32BLETracker();
  App.register_component(esp32_ble_tracker_esp32bletracker);
  // sensor.xiaomi_lywsdcgq:
  //   platform: xiaomi_lywsdcgq
  //   mac_address: 4C:65:A8:DD:2C:84
  //   temperature:
  //     id: study_temp
  //     name: Study Room Temperature
  //     icon: mdi:thermometer
  //     accuracy_decimals: 1
  //     force_update: false
  //     unit_of_measurement: °C
  //   humidity:
  //     id: study_humi
  //     name: Study Room Humidity
  //     icon: mdi:water-percent
  //     accuracy_decimals: 1
  //     force_update: false
  //     unit_of_measurement: '%'
  //   battery_level:
  //     id: study_batt
  //     name: Study Room MiJia Battery
  //     icon: mdi:battery
  //     accuracy_decimals: 0
  //     force_update: false
  //     unit_of_measurement: '%'
  //   esp32_ble_id: esp32_ble_tracker_esp32bletracker
  //   id: xiaomi_lywsdcgq_xiaomilywsdcgq
  xiaomi_lywsdcgq_xiaomilywsdcgq = new xiaomi_lywsdcgq::XiaomiLYWSDCGQ();
  App.register_component(xiaomi_lywsdcgq_xiaomilywsdcgq);
  // sensor.xiaomi_lywsdcgq:
  //   platform: xiaomi_lywsdcgq
  //   mac_address: 4C:65:A8:DD:6F:59
  //   temperature:
  //     id: workshop_temp
  //     name: Workshop Temperature
  //     icon: mdi:thermometer
  //     accuracy_decimals: 1
  //     force_update: false
  //     unit_of_measurement: °C
  //   humidity:
  //     id: workshop_humi
  //     name: Workshop Humidity
  //     icon: mdi:water-percent
  //     accuracy_decimals: 1
  //     force_update: false
  //     unit_of_measurement: '%'
  //   battery_level:
  //     id: workshop_batt
  //     name: Workshop MiJia Battery
  //     icon: mdi:battery
  //     accuracy_decimals: 0
  //     force_update: false
  //     unit_of_measurement: '%'
  //   esp32_ble_id: esp32_ble_tracker_esp32bletracker
  //   id: xiaomi_lywsdcgq_xiaomilywsdcgq_2
  xiaomi_lywsdcgq_xiaomilywsdcgq_2 = new xiaomi_lywsdcgq::XiaomiLYWSDCGQ();
  App.register_component(xiaomi_lywsdcgq_xiaomilywsdcgq_2);
  // sensor.xiaomi_lywsdcgq:
  //   platform: xiaomi_lywsdcgq
  //   mac_address: 4C:65:A8:DD:59:FE
  //   temperature:
  //     id: filament_temp
  //     name: Filament Temperature
  //     icon: mdi:thermometer
  //     accuracy_decimals: 1
  //     force_update: false
  //     unit_of_measurement: °C
  //   humidity:
  //     id: filament_humi
  //     name: Filament Humidity
  //     icon: mdi:water-percent
  //     accuracy_decimals: 1
  //     force_update: false
  //     unit_of_measurement: '%'
  //   battery_level:
  //     id: filament_batt
  //     name: Filament MiJia Battery
  //     icon: mdi:battery
  //     accuracy_decimals: 0
  //     force_update: false
  //     unit_of_measurement: '%'
  //   esp32_ble_id: esp32_ble_tracker_esp32bletracker
  //   id: xiaomi_lywsdcgq_xiaomilywsdcgq_3
  xiaomi_lywsdcgq_xiaomilywsdcgq_3 = new xiaomi_lywsdcgq::XiaomiLYWSDCGQ();
  App.register_component(xiaomi_lywsdcgq_xiaomilywsdcgq_3);
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
  //   icon: mdi:restart
  //   id: restart_restartswitch
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
  //     sa->register_sensors();
  //     App.register_component(sa);
  //     return {sa};
  //   text_sensors:
  //   - id: uptime
  //     icon: mdi:heart-pulse
  //     name: Uptime
  // xiaomi_ble:
  //   esp32_ble_id: esp32_ble_tracker_esp32bletracker
  //   id: xiaomi_ble_xiaomilistener
  xiaomi_ble_xiaomilistener = new xiaomi_ble::XiaomiListener();
  esp32_ble_tracker_esp32bletracker->set_scan_duration(300);
  esp32_ble_tracker_esp32bletracker->set_scan_interval(512);
  esp32_ble_tracker_esp32bletracker->set_scan_window(48);
  esp32_ble_tracker_esp32bletracker->set_scan_active(true);
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
  esp32_ble_tracker_esp32bletracker->register_listener(xiaomi_ble_xiaomilistener);
  esp32_ble_tracker_esp32bletracker->register_listener(xiaomi_lywsdcgq_xiaomilywsdcgq);
  esp32_ble_tracker_esp32bletracker->register_listener(xiaomi_lywsdcgq_xiaomilywsdcgq_2);
  esp32_ble_tracker_esp32bletracker->register_listener(xiaomi_lywsdcgq_xiaomilywsdcgq_3);
  out_a->set_pin(new GPIOPin(25, OUTPUT, false));
  out_a->set_restore_mode(gpio::GPIO_SWITCH_ALWAYS_OFF);
  out_b->set_pin(new GPIOPin(32, OUTPUT, false));
  out_b->set_restore_mode(gpio::GPIO_SWITCH_ALWAYS_OFF);
  out_usb->set_pin(new GPIOPin(16, OUTPUT, false));
  out_usb->set_restore_mode(gpio::GPIO_SWITCH_ALWAYS_ON);
  xiaomi_lywsdcgq_xiaomilywsdcgq->set_address(0x4C65A8DD2C84ULL);
  study_temp = new sensor::Sensor();
  App.register_sensor(study_temp);
  study_temp->set_name("Study Room Temperature");
  study_temp->set_unit_of_measurement("\302\260C");
  study_temp->set_icon("mdi:thermometer");
  study_temp->set_accuracy_decimals(1);
  study_temp->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq_2->set_address(0x4C65A8DD6F59ULL);
  workshop_temp = new sensor::Sensor();
  App.register_sensor(workshop_temp);
  workshop_temp->set_name("Workshop Temperature");
  workshop_temp->set_unit_of_measurement("\302\260C");
  workshop_temp->set_icon("mdi:thermometer");
  workshop_temp->set_accuracy_decimals(1);
  workshop_temp->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq_3->set_address(0x4C65A8DD59FEULL);
  filament_temp = new sensor::Sensor();
  App.register_sensor(filament_temp);
  filament_temp->set_name("Filament Temperature");
  filament_temp->set_unit_of_measurement("\302\260C");
  filament_temp->set_icon("mdi:thermometer");
  filament_temp->set_accuracy_decimals(1);
  filament_temp->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq->set_temperature(study_temp);
  study_humi = new sensor::Sensor();
  App.register_sensor(study_humi);
  study_humi->set_name("Study Room Humidity");
  study_humi->set_unit_of_measurement("%");
  study_humi->set_icon("mdi:water-percent");
  study_humi->set_accuracy_decimals(1);
  study_humi->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq_2->set_temperature(workshop_temp);
  workshop_humi = new sensor::Sensor();
  App.register_sensor(workshop_humi);
  workshop_humi->set_name("Workshop Humidity");
  workshop_humi->set_unit_of_measurement("%");
  workshop_humi->set_icon("mdi:water-percent");
  workshop_humi->set_accuracy_decimals(1);
  workshop_humi->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq_3->set_temperature(filament_temp);
  filament_humi = new sensor::Sensor();
  App.register_sensor(filament_humi);
  filament_humi->set_name("Filament Humidity");
  filament_humi->set_unit_of_measurement("%");
  filament_humi->set_icon("mdi:water-percent");
  filament_humi->set_accuracy_decimals(1);
  filament_humi->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq->set_humidity(study_humi);
  study_batt = new sensor::Sensor();
  App.register_sensor(study_batt);
  study_batt->set_name("Study Room MiJia Battery");
  study_batt->set_unit_of_measurement("%");
  study_batt->set_icon("mdi:battery");
  study_batt->set_accuracy_decimals(0);
  study_batt->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq_2->set_humidity(workshop_humi);
  workshop_batt = new sensor::Sensor();
  App.register_sensor(workshop_batt);
  workshop_batt->set_name("Workshop MiJia Battery");
  workshop_batt->set_unit_of_measurement("%");
  workshop_batt->set_icon("mdi:battery");
  workshop_batt->set_accuracy_decimals(0);
  workshop_batt->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq_3->set_humidity(filament_humi);
  filament_batt = new sensor::Sensor();
  App.register_sensor(filament_batt);
  filament_batt->set_name("Filament MiJia Battery");
  filament_batt->set_unit_of_measurement("%");
  filament_batt->set_icon("mdi:battery");
  filament_batt->set_accuracy_decimals(0);
  filament_batt->set_force_update(false);
  xiaomi_lywsdcgq_xiaomilywsdcgq->set_battery_level(study_batt);
  xiaomi_lywsdcgq_xiaomilywsdcgq_2->set_battery_level(workshop_batt);
  xiaomi_lywsdcgq_xiaomilywsdcgq_3->set_battery_level(filament_batt);
  // =========== AUTO GENERATED CODE END ============
  // ========= YOU CAN EDIT AFTER THIS LINE =========
  App.setup();
}

void loop() {
  App.loop();
}

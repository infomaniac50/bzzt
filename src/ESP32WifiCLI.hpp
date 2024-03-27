#ifndef ESP32WIFICLI_HPP
#define ESP32WIFICLI_HPP

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <SimpleCLI.h>
#include <Stream.h>

#define RW_MODE false
#define RO_MODE true

#define ESP32WIFICLI_VERSION "0.2.2"
#define ESP32WIFICLI_REVISION 043

class ESP32WifiCLICallbacks;

class ESP32WifiCLI {
 public:
  Preferences cfg;

  // Create CLI Object
  SimpleCLI cli;

  // Commands
  Command cmdHelp;
  Command cmdSetSSID;
  Command cmdSetPASW;
  Command cmdConnect;
  Command cmdList;
  Command cmdSelect;
  Command cmdMode;
  Command cmdScan;
  Command cmdStatus;
  Command cmdDisconnect;
  Command cmdDelete;

  WiFiMulti wifiMulti;
  const uint32_t connectTimeoutMs = 10000;
  bool silent = false;
  bool connectInBoot = true;

  ESP32WifiCLI(Stream* io);
  void begin(long baudrate = 0, String app_name = "wifi_cli_prefs");
  void loop();
  void printHelp();
  void printWifiStatus();
  void scan();
  void setSSID(String ssid);
  void setPASW(String pasw);
  void connect();
  void status();
  void list();
  void selectAP(int net);
  void disconnect();
  void reconnect();
  void multiWiFiConnect();
  void setMode(String mode);
  void wifiAPConnect(bool save);
  bool wifiValidation();
  bool loadAP(int net);
  void deleteNetwork(String ssid);
  void loadSavedNetworks(bool addAP = true);
  bool isSSIDSaved(String ssid);
  bool isConfigured();
  void saveNetwork(String ssid, String pasw);
  void setInt(String key, int value);
  int32_t getInt(String key, int defaultValue);
  void setString(String key, String value);
  String getString(String key, String defaultValue);
  void setSilentMode(bool enable);
  void disableConnectInBoot();
  String getMode();
  int getDefaultAP();
  void clearSettings();

  void setCallback(ESP32WifiCLICallbacks* pcb);

 private:
  String app_name;
  String temp_ssid = "";
  String temp_pasw = "";
  Stream* io;

  String getNetKeyName(int net);
  static void _scanNetworks(ESP32WifiCLI* wcli, Command cmd);
  static void _printHelp(ESP32WifiCLI* wcli, Command cmd);
  static void _setSSID(ESP32WifiCLI* wcli, Command cmd);
  static void _setPASW(ESP32WifiCLI* wcli, Command cmd);
  static void _connect(ESP32WifiCLI* wcli, Command cmd);
  static void _disconnect(ESP32WifiCLI* wcli, Command cmd);
  static void _listNetworks(ESP32WifiCLI* wcli, Command cmd);
  static void _wifiStatus(ESP32WifiCLI* wcli, Command cmd);
  static void _deleteNetwork(ESP32WifiCLI* wcli, Command cmd);
  static void _selectAP(ESP32WifiCLI* wcli, Command cmd);
  static void _setMode(ESP32WifiCLI* wcli, Command cmd);

  ESP32WifiCLICallbacks* cb = nullptr;
};

class ESP32WifiCLICallbacks {
public:
    virtual ~ESP32WifiCLICallbacks() {};
    virtual void onWifiStatus(bool isConnected);
    virtual void onHelpShow();
    virtual void onNewWifi(String ssid, String passw);
};

#endif

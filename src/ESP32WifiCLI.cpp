#include <ESP32WifiCLI.hpp>
#include <c/arg.h>

ESP32WifiCLI::ESP32WifiCLI(Stream* _io = &Serial) {
  io = _io;
}

void ESP32WifiCLI::printWifiStatus() {
  io->print("\nWiFi SSID \t: [");
  io->println(WiFi.SSID() + "]");  // Output Network name.
  io->print("IP address  \t: ");
  io->println(WiFi.localIP());  // Output IP Address.
  io->print("RSSI signal \t: ");
  io->println(WiFi.RSSI());  // Output signal strength.
  io->print("MAC Address\t: ");
  io->println(WiFi.macAddress());  // Output MAC address.
  io->print("Hostname \t: ");
  io->println(WiFi.getHostname());  // Output hostname.
  io->println("");
}

void ESP32WifiCLI::printHelp() {
  io->println("\nESP32WifiCLI Usage:\n");
  io->println("setSSID \"YOUR SSID\"\tset the SSID into quotes");
  io->println("setPASW \"YOUR PASW\"\tset the password into quotes");
  io->println("connect  \t\tsave and connect to the network");
  io->println("list \t\t\tlist all saved networks");
  io->println("select <number>   \tselect the default AP (default: last saved)");
  io->println("mode <single/multi>\tconnection mode. Multi AP is a little slow");
  io->println("scan \t\t\tscan for available networks");
  io->println("status \t\t\tprint the current WiFi status");
  io->println("disconnect \t\tdisconnect from the network");
  io->println("delete \"SSID\"\t\tremove saved network");
  io->println("help \t\t\tprint this help");
  if (cb != nullptr) cb->onHelpShow();
}

void ESP32WifiCLI::scan() {
  int n = WiFi.scanNetworks();
  io->print("\nscan done: ");
  if (n == 0) {
    io->println("no networks found");
  } else {
    io->print(n);
    io->println(" networks found\n");
    for (int i = 0; i < n; ++i) {
      String enc = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "[O]" : "[*]";
      io->printf("%02d %s[%i][%s]\r\n", i + 1, enc.c_str(), WiFi.RSSI(i), WiFi.SSID(i).c_str());
      delay(10);
    }
  }
}

void ESP32WifiCLI::status() {
  if (WiFi.status() == WL_CONNECTED) {
    printWifiStatus();
  } else {
    io->println("\nWiFi is not connected");
  }
}

String ESP32WifiCLI::getNetKeyName(int net) {
  if (net > 99) return "";
  char key[11];
  sprintf(key, "key_net%02d", net);
  return String(key);
}

void ESP32WifiCLI::loadSavedNetworks(bool addAP) {
  cfg.begin(app_name.c_str(), RO_MODE);
  int net = 1;
  int default_net = cfg.getInt("default_net", 1);
  if (!addAP) io->println("\nSaved networks:\n");
  while (cfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net);
    String ssid = cfg.getString(String(key + "_ssid").c_str(), "");
    String pasw = cfg.getString(String(key + "_pasw").c_str(), "");
    String dfl = (net == default_net) ? "*" : " ";
    if (!addAP) io->printf("(%s) %d: [%s]\r\n", dfl.c_str(), net, ssid.c_str());
    if (addAP) wifiMulti.addAP(ssid.c_str(), pasw.c_str());
    net++;
  }
  if(!addAP)io->println("");
  cfg.end();
}

void ESP32WifiCLI::list() {
  loadSavedNetworks(false);
}

bool ESP32WifiCLI::isSSIDSaved(String ssid) {
  cfg.begin(app_name.c_str(), RO_MODE);
  bool isSaved = false;
  int net = 1;
  while (cfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net++);
    String ssid_ = cfg.getString(String(key + "_ssid").c_str(), "");
    if (ssid_.equals(ssid)) {
      isSaved = true;
      break;
    }
  }
  cfg.end();
  return isSaved;
}

void ESP32WifiCLI::deleteNetwork(String ssid) {
  if (ssid.length() == 0) {
    io->println("\nSSID is empty, please set a valid SSID into quotes");
    return;
  }
  int net = 1;
  bool dropped = false;
  cfg.begin(app_name.c_str(), RW_MODE);
  while (cfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net++);
    String ssid_ = cfg.getString(String(key + "_ssid").c_str(), "");
    if (!dropped && ssid_.equals(ssid)) {
      io->printf("\nDeleting network [%s][%s]\r\n", key.c_str(), ssid.c_str());
      cfg.remove(String(key + "_ssid").c_str());
      cfg.remove(String(key + "_pasw").c_str());
      dropped = true;
      int net_count = cfg.getInt("net_count", 0);
      cfg.putInt("net_count", net_count - 1);
      int default_net = cfg.getInt("default_net", 1);
      if (net - 1 == default_net && net_count > 1 ) cfg.putInt("default_net", net_count - 1);
      continue;
    }
    if (dropped) {
      String ssid_drop = cfg.getString(String(key + "_ssid").c_str(), "");
      String pasw_drop = cfg.getString(String(key + "_pasw").c_str(), "");
      String key_drop = getNetKeyName(net - 2);
      // io->printf("ssid drop: [%s][%d][%s]\r\n",ssid_drop.c_str(), net - 2, key_drop.c_str());
      cfg.putString(String(key_drop + "_ssid").c_str(), ssid_drop);
      cfg.putString(String(key_drop + "_pasw").c_str(), pasw_drop);
      // io->printf("remove key: [%d][%s]\r\n",net - 1, key.c_str());
      int default_net = cfg.getInt("default_net", 1);
      if (net - 1 == default_net) cfg.putInt("default_net", net - 2);
      cfg.remove(String(key + "_ssid").c_str());
      cfg.remove(String(key + "_pasw").c_str());
    }
  }
  cfg.end();
}

void ESP32WifiCLI::saveNetwork(String ssid, String pasw) {
  cfg.begin(app_name.c_str(), RW_MODE);
  int net = cfg.getInt("net_count", 0);
  String key = getNetKeyName(net + 1);
  if (!silent) io->printf("Saving network: [%s][%s]\r\n", ssid.c_str(), pasw.c_str());
  cfg.putString(String(key + "_ssid").c_str(), ssid);
  cfg.putString(String(key + "_pasw").c_str(), pasw);
  cfg.putInt("net_count", net + 1);
  cfg.putInt("default_net", net + 1);
  cfg.end();
}

void ESP32WifiCLI::setSSID(String ssid) {
  temp_ssid = ssid;
  if (temp_ssid.length() == 0) {
    io->println("\nSSID is empty, please set a valid SSID into quotes");
  } else {
    io->println("\nset ssid to: " + temp_ssid);
  }
}

void ESP32WifiCLI::setPASW(String pasw) {
  temp_pasw = pasw;
  io->println("\nset password to: " + temp_pasw);
}

void ESP32WifiCLI::disconnect() {
  io->println("\nDisconnecting...");
  WiFi.disconnect();
}

bool ESP32WifiCLI::wifiValidation() {
  if (WiFi.status() == WL_CONNECTED) {
    io->println("connected!");
    if(!silent) status();
    return true;
  } else {
    io->println("connection failed!");
    return false;
  }
}

void ESP32WifiCLI::wifiAPConnect(bool save) {
  if (temp_ssid.length() == 0) {
    io->println("\nSSID is empty, please set a valid SSID into quotes\n");
    return;
  }
  io->print("\nConnecting to " + temp_ssid + "...");
  if (save) {
    wifiMulti.addAP(temp_ssid.c_str(), temp_pasw.c_str());
  }
  int retry = 0;
  WiFi.begin(temp_ssid.c_str(), temp_pasw.c_str());

  #ifdef FAMILY
  if (FAMILY == "ESP32-C3") WiFi.setTxPower(WIFI_POWER_8_5dBm);  // TODO: uggly workaround for some C3 devices
  #endif

  while (WiFi.status() != WL_CONNECTED && retry++ < 20) {  // M5Atom will connect automatically
    delay(1000);
    io->print(".");
  }
  delay(100);
  if (wifiValidation() && save) {
    saveNetwork(temp_ssid, temp_pasw);
    if(cb != nullptr) cb->onNewWifi(temp_ssid, temp_pasw);
  }
}

bool ESP32WifiCLI::isConfigured() {
  cfg.begin("wifi_cli_prefs", RO_MODE);
  String key = getNetKeyName(1);
  bool isConfigured = cfg.isKey(String(key + "_ssid").c_str());
  cfg.end();
  return isConfigured;
}

bool ESP32WifiCLI::loadAP(int net) {
  cfg.begin(app_name.c_str(), RO_MODE);
  String key = getNetKeyName(net);
  if (!cfg.isKey(String(key + "_ssid").c_str())) {
    cfg.end();
    return false;
  }
  // io->printf("\nDefault AP: %i: [%s]\r\n", net, cfg.getString(String(key + "_ssid").c_str(), "").c_str());
  temp_ssid = cfg.getString(String(key + "_ssid").c_str(), "");
  temp_pasw = cfg.getString(String(key + "_pasw").c_str(), "");
  cfg.end();
  return true;
}

void ESP32WifiCLI::selectAP(int net) {
  if (!loadAP(net)) {
    io->println("\nNetwork not found");
    return;
  }
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.putInt("default_net", net);
  cfg.end();
  list();
}

int ESP32WifiCLI::getDefaultAP() {
  cfg.begin(app_name.c_str(), RO_MODE);
  int net = cfg.getInt("default_net", 1);
  cfg.end();
  return net;
}

String ESP32WifiCLI::getMode() {
  cfg.begin(app_name.c_str(), RO_MODE);
  String mode = cfg.getString("mode", "single");
  cfg.end();
  return mode;
}

void ESP32WifiCLI::setMode(String mode) {
  cfg.begin(app_name.c_str(), RW_MODE);
  if (mode.equals("single")) {
    cfg.putString("mode", "single");
  } else if (mode.equals("multi")) {
    cfg.putString("mode", "multi");
  } else if (mode.equals("")) {
    io->printf("\nCurrent mode: %s\r\n", cfg.getString("mode", "single").c_str());
  } else {
    io->println("\nInvalid mode, please use single/multi parameter");
  }
  cfg.end();
}

void ESP32WifiCLI::multiWiFiConnect() {
  int retry = 0;
  io->print("\nConnecting in MultiAP mode..");
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED && retry++ < 10) {
    delay(500);
    io->print(".");
  }
  wifiValidation();
}

void ESP32WifiCLI::reconnect() {
  if (WiFi.status() != WL_CONNECTED && getMode().equals("single")) {
    wifiAPConnect(false);
  } else {
    multiWiFiConnect();
  }
}

void ESP32WifiCLI::connect() {
  if (WiFi.status() == WL_CONNECTED && temp_ssid == WiFi.SSID()) {
    io->println("\nWiFi is already connected");
    return;
  } else if (WiFi.status() == WL_CONNECTED) {
    disconnect();
    delay(1000);
  }
  if (getMode().equals("single")) {
    if (temp_ssid.length() == 0) {
      io->println("\nSSID is empty, please set a valid SSID into quotes\n");
      return;
    }
    if (isSSIDSaved(temp_ssid)) {
      wifiAPConnect(false);
      return;
    } else {
      wifiAPConnect(true);
    }
  } else {
    multiWiFiConnect();
  }
}

void ESP32WifiCLI::loop() {
  // term->loop();
  if (io->available()) {
    String input = io->readStringUntil('\n');
    
    if (input.length() > 0) {
        cli.parse(input);
    }
    
    if (cli.available()) { 
      Command c = cli.getCmd();

      if (cmdHelp == c) {
        ESP32WifiCLI::_printHelp(this, c);
      }
      if (cmdSetSSID == c) {
        ESP32WifiCLI::_setSSID(this, c);
      }
      if (cmdSetPASW == c) {
        ESP32WifiCLI::_setPASW(this, c);
      }
      if (cmdConnect == c) {
        ESP32WifiCLI::_connect(this, c);
      }
      if (cmdList == c) {
        ESP32WifiCLI::_listNetworks(this, c);
      }
      if (cmdSelect == c) {
        ESP32WifiCLI::_selectAP(this, c);
      }
      if (cmdMode == c) {
        ESP32WifiCLI::_setMode(this, c);
      }
      if (cmdScan == c) {
        ESP32WifiCLI::_scanNetworks(this, c);
      }
      if (cmdStatus == c) {
        ESP32WifiCLI::_wifiStatus(this, c);
      }
      if (cmdDisconnect == c) {
        ESP32WifiCLI::_disconnect(this, c);
      }
      if (cmdDelete == c) {
        ESP32WifiCLI::_deleteNetwork(this, c);
      }
    }
  }
  static uint_least64_t wifiTimeStamp = 0;
  if (millis() - wifiTimeStamp > 1000) {
    wifiTimeStamp = millis();
    if(cb != nullptr) cb->onWifiStatus(WiFi.status() == WL_CONNECTED); 
  }
}

void ESP32WifiCLI::setCallback(ESP32WifiCLICallbacks* pcb) {
  this->cb = pcb;
}

void ESP32WifiCLI::setInt(String key, int value) {
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.putInt(key.c_str(), value);
  cfg.end();
}

int32_t ESP32WifiCLI::getInt(String key, int defaultValue) {
  cfg.begin(app_name.c_str(), RO_MODE);
  int32_t out = cfg.getInt(key.c_str(), defaultValue);
  cfg.end();
  return out;
}

void ESP32WifiCLI::setString(String key, String value) {
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.putString(key.c_str(), value);
  cfg.end();
}

String ESP32WifiCLI::getString(String key, String defaultValue) {
  cfg.begin(app_name.c_str(), RO_MODE);
  String out = cfg.getString(key.c_str(), defaultValue);
  cfg.end();
  return out;
}

void ESP32WifiCLI::clearSettings() {
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.clear();
  cfg.end();
  if (!silent) io->println("Settings cleared!");
}

void ESP32WifiCLI::setSilentMode(bool silent) {
  this->silent = silent;
}

void ESP32WifiCLI::disableConnectInBoot(){
  this->connectInBoot = false;
}

void ESP32WifiCLI::_scanNetworks(ESP32WifiCLI* wcli, Command cmd) {
  wcli->scan();
}

void ESP32WifiCLI::_printHelp(ESP32WifiCLI* wcli, Command cmd) {
  wcli->printHelp();
}

void ESP32WifiCLI::_setSSID(ESP32WifiCLI* wcli, Command cmd) {
  // Get first (and only) Argument
  Argument arg = cmd.getArgument(0);

  // Get value of argument
  String argVal = arg.getValue();
  argVal.trim();
  
  if (argVal.length() == 0) {
    wcli->io->println("Missing argument <ssid>");
    return;
  }
  
  wcli->setSSID(argVal);
}

void ESP32WifiCLI::_setPASW(ESP32WifiCLI* wcli, Command cmd) {

  // Get first (and only) Argument
  Argument arg = cmd.getArgument(0);

  // Get value of argument
  String argVal = arg.getValue();
  argVal.trim();
  
  if (argVal.length() == 0) {
    wcli->io->println("Missing argument <password>");
  }

  wcli->setPASW(argVal);
}

void ESP32WifiCLI::_connect(ESP32WifiCLI* wcli, Command cmd) {
  wcli->connect();
}

void ESP32WifiCLI::_disconnect(ESP32WifiCLI* wcli, Command cmd) {
  wcli->disconnect();
}

void ESP32WifiCLI::_listNetworks(ESP32WifiCLI* wcli, Command cmd) {
  wcli->loadSavedNetworks(false);
}

void ESP32WifiCLI::_wifiStatus(ESP32WifiCLI* wcli, Command cmd) {
  wcli->status();
}

void ESP32WifiCLI::_deleteNetwork(ESP32WifiCLI* wcli, Command cmd) {
  // Get first (and only) Argument
  Argument arg = cmd.getArgument(0);

  // Get value of argument
  String argVal = arg.getValue();
  argVal.trim();
  
  if (argVal.length() == 0) {
    wcli->io->println("Missing argument <ssid>");
    return;
  }
  
  wcli->deleteNetwork(argVal);
}

void ESP32WifiCLI::_selectAP(ESP32WifiCLI* wcli, Command cmd) {
  // Get first (and only) Argument
  Argument arg = cmd.getArgument(0);

  // Get value of argument
  String argVal = arg.getValue();
  argVal.trim();
  
  if (argVal.length() == 0) {
    wcli->io->println("Missing argument <index>");
    return;
  }
  
  int net = argVal.toInt();
  wcli->selectAP(net);
}

void ESP32WifiCLI::_setMode(ESP32WifiCLI* wcli, Command cmd) {
  // Get first (and only) Argument
  Argument arg = cmd.getArgument(0);

  // Get value of argument
  String argVal = arg.getValue();
  argVal.trim();
  
  if (argVal.length() == 0) {
    wcli->io->println("Missing argument <mode>");
    return;
  }
  argVal.toLowerCase();

  if (argVal.equals("single") == 0 || argVal.equals("multi") == 0) {
    wcli->setMode(argVal);
  } else {
    wcli->io->println("Invalid argument <mode>: The value must be single or multi");
  }
}

void ESP32WifiCLI::begin(long baudrate, String app) {
  app_name = app.length() == 0 ? "wifi_cli_prefs" : app;
  WiFi.mode(WIFI_STA);
  io->flush();
  delay(10);
  io->println("");
  loadSavedNetworks();
  loadAP(getDefaultAP());
  if (connectInBoot) {
    reconnect();
    delay(10);
  }

  cmdHelp = cli.addCommand("help");
  cmdHelp.setDescription("show detail usage information");
  cmdSetSSID = cli.addSingleArgumentCommand("setSSID");
  cmdSetSSID.setDescription("set the Wifi SSID");
  cmdSetPASW = cli.addSingleArgumentCommand("setPASW");
  cmdSetPASW.setDescription("set the WiFi password  ");
  cmdConnect = cli.addCommand("connect");
  cmdConnect.setDescription("save and connect to WiFi network");
  cmdList = cli.addCommand("list");
  cmdList.setDescription("list saved WiFi networks");
  cmdSelect = cli.addSingleArgumentCommand("select");
  cmdSelect.setDescription("select the default AP (default: last)");
  cmdMode = cli.addSingleArgumentCommand("mode");
  cmdMode.setDescription("set the default operation single/multi AP (slow)");
  cmdScan = cli.addCommand("scan");
  cmdScan.setDescription("scan WiFi networks");
  cmdStatus = cli.addCommand("status");
  cmdStatus.setDescription("WiFi status information  ");
  cmdDisconnect = cli.addCommand("disconnect");
  cmdDisconnect.setDescription("WiFi disconnect");
  cmdDelete = cli.addCommand("delete");
  cmdDelete.setDescription("remove saved WiFi network by SSID");
}

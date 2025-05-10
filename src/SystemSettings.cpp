#include "SystemSettings.h"

void SystemPreferences::save(SystemSettings* settings) {
  preferences.begin(DEFAULT_SYSTEM_SETTING_APP_NAME, false);
  preferences.putString("bhm", settings->brokerHostname);
  preferences.putString("shm", settings->systemHostname);
  preferences.putString("tm1", settings->timeServer1);
  preferences.putString("tm2", settings->timeServer2);
  preferences.putString("tm3", settings->timeServer3);
  preferences.end();
}

void SystemPreferences::load(SystemSettings* settings) {
  preferences.begin(DEFAULT_SYSTEM_SETTING_APP_NAME, true);
  settings->brokerHostname = preferences.getString("bhm", DEFAULT_SYSTEM_SETTING_BROKER_HOSTNAME);
  settings->systemHostname = preferences.getString("shm", DEFAULT_SYSTEM_SETTING_SYSTEM_HOSTNAME);
  settings->timeServer1 = preferences.getString("tm1", DEFAULT_SYSTEM_SETTING_TIME_SERVER_1);
  settings->timeServer2 = preferences.getString("tm2", DEFAULT_SYSTEM_SETTING_TIME_SERVER_2);
  settings->timeServer3 = preferences.getString("tm3", DEFAULT_SYSTEM_SETTING_TIME_SERVER_3);
  preferences.end();
}

bool SystemPreferences::clear() {
  preferences.begin(DEFAULT_SYSTEM_SETTING_APP_NAME, false);
  bool status = preferences.clear();
  preferences.end();
  return status;
}
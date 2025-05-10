#ifndef _SYSTEM_SETTINGS_H_
#define _SYSTEM_SETTINGS_H_

#include <cstdint>
#include <Preferences.h>

#define DEFAULT_SYSTEM_SETTING_BROKER_HOSTNAME ""
#define DEFAULT_SYSTEM_SETTING_SYSTEM_HOSTNAME "bzzt"
#define DEFAULT_SYSTEM_SETTING_TIME_SERVER_1 "192.168.0.3" // Local GPS disciplined timeserver
#define DEFAULT_SYSTEM_SETTING_TIME_SERVER_2 "0.us.pool.ntp.org"
#define DEFAULT_SYSTEM_SETTING_TIME_SERVER_3 "1.us.pool.ntp.org"
#define DEFAULT_SYSTEM_SETTING_APP_NAME "bzzt_system"

struct SystemSettings
{
  String brokerHostname = DEFAULT_SYSTEM_SETTING_BROKER_HOSTNAME;
  String systemHostname = DEFAULT_SYSTEM_SETTING_SYSTEM_HOSTNAME;
  String timeServer1 = DEFAULT_SYSTEM_SETTING_TIME_SERVER_1;
  String timeServer2 = DEFAULT_SYSTEM_SETTING_TIME_SERVER_2;
  String timeServer3 = DEFAULT_SYSTEM_SETTING_TIME_SERVER_3;
};

class SystemPreferences {
  private:
    Preferences preferences;

  public:
    void save(SystemSettings* settings);
    void load(SystemSettings* settings);
    bool clear();
};
#endif

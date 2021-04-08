#ifndef P1_Override_Settings
#define P1_Override_Settings
	#ifdef TEST_MODE
	// Note t1 connects at (dhcp) 192.168.1.125 mac 68:C6:3A:B8:38:C1   11 Mbit/s WPA2 
	const char *ssid = "Test SSID";                        //  T E S T M O D E setting
	const char *password = "YourWifiTestPassword";
	const char *hostName = "nodemcu"        P1_VERSION_TYPE;	// our (OTA) hostname in the network, should be unique
	const char *mqttServer = "192.168.1.154";	// mqtt server for test, for example your local/desktop
	const char *mqttClientName = "nodemcu-" P1_VERSION_TYPE; // Note our ClientId for Mqtt should be unique
	const char *mqttTopic = "/energy/"      P1_VERSION_TYPE;	// mqtt topic production for the JSON fielded data
	const char *mqttLogTopic = "/log/"      P1_VERSION_TYPE;	// mqtt logtopic (activated by command 'L'
	const char *mqttErrorTopic = "/error/"  P1_VERSION_TYPE;	// mqtt topic to receive errors
	const char *mqttPower = "/energy/"      P1_VERSION_TYPE "power"; // mqtt topic to receive PowerUsage as numbered State
	const int   mqttPort = 1883;								// mqtt port (1883)
	#else
	// Note p1 connects at (dhcp) 192.168.1.125 mac 18:FE:34:D6:7E:25   11 Mbit/s WPA2
	const char *ssid = "Production SSID";    // "Pafo SSID4"    //  P R O D U C T I O N  setting
	const char *password = "YourWifiTestPassword";
	const char *hostName = "nodemcu"        P1_VERSION_TYPE;	// our (OTA) hostname in the network, should be unique
	const char *mqttServer = "192.168.1.254";	// mqtt server for production
	const char *mqttClientName = "nodemcu-" P1_VERSION_TYPE; // Note our ClientId for Mqtt should be unique
	const char *mqttTopic = "/energy/"      P1_VERSION_TYPE;	// mqtt topic production for the JSON fielded data
	const char *mqttLogTopic = "/log/"      P1_VERSION_TYPE;	// mqtt logtopic (activated by command 'L'
	const char *mqttErrorTopic = "/error/"  P1_VERSION_TYPE;	// mqtt topic to receive errors
	const char *mqttPower = "/energy/"      P1_VERSION_TYPE "power"; // mqtt topic to receive PowerUsage as numbered State
	const int   mqttPort = 1883;								// mqtt port (1883)
	#endif
#endif

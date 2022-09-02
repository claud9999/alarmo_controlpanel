#ifndef alarmo_cpsettings.hpp
#define alarmo_cpsettings.hpp

// replace with your wifi's ssid/password
const char *ssid = "YOURSSID"; const char *password = "YOURWIFIPWD";

// replace with a URL pointing at a jpeg that is 1024x768
const char *img_url = "YOURIMGURL";

// replace with your MQTT host, port, username, key
#define MQTT_HOST "YOURMQTTBROKER"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_KEY ""

#define IDLE_DELAY 120 // 2 minutes
#define DEEPSLEEP_DELAY 300 // 5 minutes

#endif /* alarmo_cpsettings.hpp */

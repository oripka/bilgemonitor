#define WIFI_SSID "MyWifi"
#define WIFI_PASSWORD "MyPassword"

#define AWS_TOPIC_NAME "bilgemonitor"
#define AWS_ENDPOINT "xxxxx.iot.us-east-1.amazonaws.com"
#define AWS_CLIENT_ID "MyClientID"

#define TIME_TO_SLEEP  60       /* Time ESP32 will go to sleep (in seconds) */
#define MIN_DISTANCE_TO_SEND 20.0

/* force send even every X wakeups.
 * This depends on the TIME_TO_SLEEP
 *default every hour if TIME_TO_SLEEP is 60 seconds
 */
#define FORCE_SEND_EVERY_X_WAKEUPS 60

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define MAX_SOUND_ID 2
#define MAX_ANIMATION_ID 1

// WIFI mode AP
const char* hotspot_ssid = "pinball_hotspot";
const char* hotspot_password = "tototiti";
// WIFI mode connected
const char* wifi_ssid = "Livebox-A860";
const char* wifi_password = "2Rjurfn54bz5fmP5aJ";

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

int score = 0;
int sound = -1;
int animation = -1;

char message[64];

const int ledPin = 2;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 

const char index_html[] PROGMEM = R"rawliteral(
'empty'
)rawliteral";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients() {
  score = random(10, 1000);
  sprintf(message, "%d %d %d", score, sound, animation);
  Serial.println("> sent score");
  Serial.println(message);
  ws.textAll(String(message));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "get_score") == 0) {
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var) {
  Serial.println(var);
  return String();
}

void initWifi(bool hotspot) {
  if (hotspot) {
    Serial.println("WIFI initialization...");
    WiFi.softAP(hotspot_ssid, hotspot_password);
    Serial.println("WIFI configuration...");
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
  
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }
  else {
    Serial.println("Connect to WIFI...");
    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("connected");

    // Print ESP Local IP Address
    Serial.print("WIFI local IP address:");
    Serial.println(WiFi.localIP());
  }
}

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  // animation simulation
  sound++;
  if (sound > MAX_SOUND_ID)
    sound = -1;
  animation++;
  if (animation > MAX_ANIMATION_ID)
    animation = -1;
//  Serial.print("ids= ");
//  Serial.print(sound);
//  Serial.print(" ");
//  Serial.println(animation);
  portEXIT_CRITICAL_ISR(&timerMux);
 
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  Serial.println("> setup is starting...");

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // timer setup
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 2000000, true);
  timerAlarmEnable(timer);

  // WIFI setup
  initWifi(false);

  // websocket setup
  initWebSocket();
  
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });


  // Start server
  server.begin();
  Serial.println("< setup done");
}

void loop() {
  ws.cleanupClients();
}

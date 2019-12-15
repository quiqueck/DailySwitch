#include "DailyBluetoothSwitch.h"
#include <HTTPClient.h>
static HTTPClient http;

#if USE_BT==0
#include "WeatherKey.h"
const char* WiFiSSID     = WIFI_NAME;
const char* WiFiPassword = WIFI_ACC;

#if DEBUG_WIFI_TARGET==1
static const char* host = "192.168.55.51";
static const int port = 8081;
static const char* target = "/thedaily/undefined/event";
#else
static const char* host = "192.168.55.21";
static const int port = 8081;
static const char* target = "/thedaily/main/event";
#endif

#define SECOND(__s__) (__s__*1000)
#define MINUTE(__m__) (__m__*1000*60)

bool trippedWiFiTimer = false;
hw_timer_t * wWiFiTimer = NULL;
portMUX_TYPE wWiFiTimerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onConnectTimer() {
  portENTER_CRITICAL_ISR(&wWiFiTimerMux);
  trippedWiFiTimer = true;  
  portEXIT_CRITICAL_ISR(&wWiFiTimerMux);
}

DailyBluetoothSwitchServer::DailyBluetoothSwitchServer(std::string name){
    whenConnected = NULL;
    baseName = String("AmberSmart.Switch") + String(name.c_str()); 
    Console.printf("Initializing WiFi Service %s\n", baseName.c_str());
    state = SwitchUpdateState::INIT;  
    lastUpdateCall = millis();

    Console.println("Attaching Hardware Timer");
       
    wWiFiTimer = timerBegin(2, 80, true);
    timerAttachInterrupt(wWiFiTimer, &onConnectTimer, true);
    Console.println("Set Timeout");
    timerAlarmWrite(wWiFiTimer, MINUTE(1)*1000, true);
    Console.println("Enable Timer");
    timerAlarmEnable(wWiFiTimer); 
}

void DailyBluetoothSwitchServer::startAdvertising() {
    if (state == SwitchUpdateState::IDLE || state == SwitchUpdateState::INIT) {
        if (WiFi.status() == WL_CONNECTED){
            return;
        }
        Console.println("Connecting WIFI...");
        state = SwitchUpdateState::CONNECTING;  
        wifiRetries = 500;

        WiFi.mode(WIFI_STA);
        WiFi.setSleep(true);
        WiFi.begin(WiFiSSID, WiFiPassword); 
    }
}
static void postJSON(String name, uint16_t id, DailyBluetoothSwitchServer::DBSNotificationStates state){
  String s = "{\"id\": " + String(id)  + ", \"state\":" + String(state) + ", \"name\":\"" + name + "\"}";  
  Console.printf("Sending %s to %s:%i/%s\n", s.c_str(), host, port, target);
  http.begin(host, port, String(target));
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(s);
  Console.printf("Result: %i\n", httpCode);
  http.end();
}

void DailyBluetoothSwitchServer::sendNotification(uint16_t id, DBSNotificationStates state){
    postJSON(baseName, id, state);
}

void DailyBluetoothSwitchServer::onConnect(BLEServer* pServer) {
    Console.printf("Connected %x\n", pServer);
    Console.println(WiFi.localIP());
    BLEClientConnected = true;

    if (whenConnected){
        whenConnected(true);
    }
};

void DailyBluetoothSwitchServer::onDisconnect(BLEServer* pServer) {
    Console.printf("Disconnected %x\n", pServer);
    BLEClientConnected = false;

    if (whenConnected){
        whenConnected(false);
    }
}

static void _stopWiFi(){    
    Console.println("Turnin off WiFi."); 
    //WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);
    WiFi.disconnect();    
    WiFi.setSleep(true);
    WiFi.mode(WIFI_OFF);
}

void DailyBluetoothSwitchServer::tick(){
    portENTER_CRITICAL_ISR(&wWiFiTimerMux);
    bool didTrip = trippedWiFiTimer;
    trippedWiFiTimer = false;  
    portEXIT_CRITICAL_ISR(&wWiFiTimerMux);
    
    //Console.printf("Tick with state=%i, didTrip=%i\n", state, didTrip);
    if (didTrip){
        if (state != SwitchUpdateState::CONNECTING && WiFi.status() != WL_CONNECTED){
            Console.printf("Found disconnected WiFi.");
            _stopWiFi();
            state = SwitchUpdateState::INIT;            
            onDisconnect(NULL);
        }
    }  else if (state != SwitchUpdateState::IDLE && state != SwitchUpdateState::CONNECTED) {
        Console.printf("waited %lu>%lu", millis() - lastUpdateCall, SECOND(1))  ;     
        
        if (millis() - lastUpdateCall > SECOND(1)){
            update();
        }
    }
}

void DailyBluetoothSwitchServer::update(){
    Console.printf("Update with state=%i\n", state);
    lastUpdateCall = millis();
    if (state == SwitchUpdateState::IDLE || state == SwitchUpdateState::INIT) {
        Console.println("Waiting for WiFi...\n");        
        startAdvertising();                                
    } if (state == SwitchUpdateState::CONNECTING){
        if (wifiRetries > 0) {
            wifiRetries--;            
            Console.print("*");
            if (WiFi.status() == WL_CONNECTED){        
               state = SwitchUpdateState::CONNECTED;
               onConnect(NULL);
            }
        } else {
            Console.println("");
            Console.println("  WiFi connection failed");
            _stopWiFi();
            state = SwitchUpdateState::IDLE;
        }
    } 
}


#endif
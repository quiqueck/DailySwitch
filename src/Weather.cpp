#include "Weather.h"
#include "SwitchUI.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <soc/rtc.h>

const char* ssid     = WIFI_NAME;
const char* password = WIFI_ACC;

const char* host = "api.openweathermap.org";
const char* basePath = "data/2.5";
/* use 
openssl s_client -showcerts -connect api.openweathermap.org:443 </dev/null 
to get this certificate */
const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFdDCCBFygAwIBAgIQJ2buVutJ846r13Ci/ITeIjANBgkqhkiG9w0BAQwFADBv\n" \
"MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFk\n" \
"ZFRydXN0IEV4dGVybmFsIFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBF\n" \
"eHRlcm5hbCBDQSBSb290MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFow\n" \
"gYUxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\n" \
"BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMSswKQYD\n" \
"VQQDEyJDT01PRE8gUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIICIjANBgkq\n" \
"hkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAkehUktIKVrGsDSTdxc9EZ3SZKzejfSNw\n" \
"AHG8U9/E+ioSj0t/EFa9n3Byt2F/yUsPF6c947AEYe7/EZfH9IY+Cvo+XPmT5jR6\n" \
"2RRr55yzhaCCenavcZDX7P0N+pxs+t+wgvQUfvm+xKYvT3+Zf7X8Z0NyvQwA1onr\n" \
"ayzT7Y+YHBSrfuXjbvzYqOSSJNpDa2K4Vf3qwbxstovzDo2a5JtsaZn4eEgwRdWt\n" \
"4Q08RWD8MpZRJ7xnw8outmvqRsfHIKCxH2XeSAi6pE6p8oNGN4Tr6MyBSENnTnIq\n" \
"m1y9TBsoilwie7SrmNnu4FGDwwlGTm0+mfqVF9p8M1dBPI1R7Qu2XK8sYxrfV8g/\n" \
"vOldxJuvRZnio1oktLqpVj3Pb6r/SVi+8Kj/9Lit6Tf7urj0Czr56ENCHonYhMsT\n" \
"8dm74YlguIwoVqwUHZwK53Hrzw7dPamWoUi9PPevtQ0iTMARgexWO/bTouJbt7IE\n" \
"IlKVgJNp6I5MZfGRAy1wdALqi2cVKWlSArvX31BqVUa/oKMoYX9w0MOiqiwhqkfO\n" \
"KJwGRXa/ghgntNWutMtQ5mv0TIZxMOmm3xaG4Nj/QN370EKIf6MzOi5cHkERgWPO\n" \
"GHFrK+ymircxXDpqR+DDeVnWIBqv8mqYqnK8V0rSS527EPywTEHl7R09XiidnMy/\n" \
"s1Hap0flhFMCAwEAAaOB9DCB8TAfBgNVHSMEGDAWgBStvZh6NLQm9/rEJlTvA73g\n" \
"JMtUGjAdBgNVHQ4EFgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQD\n" \
"AgGGMA8GA1UdEwEB/wQFMAMBAf8wEQYDVR0gBAowCDAGBgRVHSAAMEQGA1UdHwQ9\n" \
"MDswOaA3oDWGM2h0dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9BZGRUcnVzdEV4dGVy\n" \
"bmFsQ0FSb290LmNybDA1BggrBgEFBQcBAQQpMCcwJQYIKwYBBQUHMAGGGWh0dHA6\n" \
"Ly9vY3NwLnVzZXJ0cnVzdC5jb20wDQYJKoZIhvcNAQEMBQADggEBAGS/g/FfmoXQ\n" \
"zbihKVcN6Fr30ek+8nYEbvFScLsePP9NDXRqzIGCJdPDoCpdTPW6i6FtxFQJdcfj\n" \
"Jw5dhHk3QBN39bSsHNA7qxcS1u80GH4r6XnTq1dFDK8o+tDb5VCViLvfhVdpfZLY\n" \
"Uspzgb8c8+a4bmYRBbMelC1/kZWSWfFMzqORcUx8Rww7Cxn2obFshj5cqsQugsv5\n" \
"B5a6SE2Q8pTIqXOi6wZ7I53eovNNVZ96YUWYGGjHXkBrI/V5eu+MtWuLt29G9Hvx\n" \
"PUsE2JOAWVrgQSQdso8VYFhH2+9uRv0V9dlfmrPb2LjkQLPNlzmuhbsdjrzch5vR\n" \
"pu/xO28QOG8=\n" \
"-----END CERTIFICATE-----";

#define SECOND(__s__) (__s__*1000*1000)
#define MINUTE(__m__) (__m__*1000*1000*60)

bool trippedTimer = false;
hw_timer_t * wtimer = NULL;
portMUX_TYPE wtimerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onWeatherTimer() {
  portENTER_CRITICAL_ISR(&wtimerMux);
  trippedTimer = true;  
  portEXIT_CRITICAL_ISR(&wtimerMux);
}

Weather* Weather::_global = NULL;
StaticJsonDocument<1000> doc;

Weather::Weather(std::string keyIn, SwitchUI* uiIn){
    key = keyIn;
    ui = uiIn;
    hasData = false;
    state = WeatherUpdateState::INIT;    
    wtimer = timerBegin(1, 80, true);

    timerAttachInterrupt(wtimer, &onWeatherTimer, true);
    timerAlarmWrite(wtimer, MINUTE(30), true);
    timerAlarmEnable(wtimer);

    lastUpdateCall = millis();
}

void Weather::startWiFi(){  
    if (state != WeatherUpdateState::IDLE && state != WeatherUpdateState::INIT)
        return;

    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);     
}

void Weather::stopWiFi(){
    Serial.println("Turnin off WiFi.");    
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
}

float Weather::temperature() const{
    return doc["main"]["temp"];
}
float Weather::pressure() const{
    return doc["main"]["pressure"];
}
float Weather::humidity() const{
    return doc["main"]["humidity"];
}
std::string Weather::icon() const{
    const int wid = doc["weather"][0]["id"];
    const char* icon = doc["weather"][0]["icon"];

    if (wid==781) {
        bool day = true;
        if (icon && icon[2] == 'n') day = false;
        char buffer[20];
        snprintf(buffer, 20, "/%d%s.IST", wid, day?"d":"n");
        return std::string(buffer);
    }

    return std::string("/") + icon + ".IST";                
}

HTTPClient http;
void Weather::readData(){
    bool hasNewData = false;
    //https://samples.openweathermap.org/data/2.5/weather?lat=35&lon=139&appid=b6907d289e10d714a6e88b30761fae22 

    Serial.printf("\nStarting connection to %s ...\n", host);
    //Serial.print("freeMemory()="); Serial.print(ESP.getFreeHeap()); Serial.print(" "); Serial.println(ESP.getFreePsram());
    const uint16_t startTime = millis();
    //std::string request = std::string("http://") + host + "/" + basePath + "/weather?lat=" + LAT + "&lon=" + LON + "&appid=" + key + "&units=metric";
    std::string request = std::string("http://192.168.55.51:8080/data.json");
    //Serial.printf("Requesting %s\n", request.c_str());
    
    http.begin(request.c_str()); //Specify the URL
    int httpCode = http.GET();                                        //Make the request
 
    if (httpCode > 0) { //Check for the returning code 
        // Serial.println(httpCode);
        if (httpCode == 200){
            String payload = http.getString();

            // Deserialize the JSON document
            DeserializationError error = deserializeJson(doc, payload);

            // Test if parsing succeeds.
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());                
            } else {
                hasNewData = true;
                hasData = true;                                
            }
            //Serial.println(payload);
        } else {
           Serial.printf("HTTP request returned %d\n", httpCode); 
        }
    } else {
      Serial.println(F("Error on HTTP request"));
    }
 
    Serial.printf("Finished in %lu ms\n", millis() - startTime);
    http.end(); //Free the resources
    //Serial.print("freeMemory()="); Serial.print(ESP.getFreeHeap()); Serial.print(" "); Serial.println(ESP.getFreePsram());

    if (hasNewData){
        ui->weatherChanged(this);
    }
}

void Weather::update(){
    lastUpdateCall = millis();
    if (state == WeatherUpdateState::IDLE || state == WeatherUpdateState::INIT) {
        Serial.println("Updating Weather Info...\n");
        startWiFi();  
        state = WeatherUpdateState::CONNECTING;  
        wifiRetries = 100;                      
    } if (state == WeatherUpdateState::CONNECTING){
        if (wifiRetries > 0) {
            wifiRetries--;            
            Serial.print(".");
            if (WiFi.status() == WL_CONNECTED){        
               state = WeatherUpdateState::CONNECTED;
            }
        } else {
            Serial.println("");
            Serial.println("  WiFi connection failed");
            stopWiFi();
            state = WeatherUpdateState::IDLE;
        }
    } if (state == WeatherUpdateState::CONNECTED){
        Serial.println("");
        Serial.print("  WiFi connected as ");        
        Serial.println(WiFi.localIP());
        state = WeatherUpdateState::LOADING;
        readData();
        state = WeatherUpdateState::IDLE;
        Serial.print("  Weather Updated finished\n");  
        stopWiFi();
    }
}

void Weather::tick(){
    portENTER_CRITICAL_ISR(&wtimerMux);
    bool didTrip = trippedTimer;
    trippedTimer = false;  
    portEXIT_CRITICAL_ISR(&wtimerMux);
    
    if (didTrip){
        update();
    }  else if (state != WeatherUpdateState::IDLE) {
        //delay about 500ms
         if (millis() - lastUpdateCall > 500){
            update();
         }
    }
}
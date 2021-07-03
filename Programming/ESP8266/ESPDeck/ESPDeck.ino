//
// Einfacher Webserver der Dateien aus dem SPIFFS (Filesystem) ausliefert
// CC by cooper @ makesmart.net
// mehr Infos unter: https://my.makesmart.net/topic/206/
//
 
void setupServerRoutes();
void setupFilePaths();
void onWsEvent();
void setupWebsockets();
void handleWebSocketMessage();
void notifyClients();

void initLed();
bool readPairing();

String readConfig();
String writeConfig();
String getFSFiles();
 
// WLAN
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
 
// Webserver 
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Wifi Manager
#include <ESPAsyncWiFiManager.h>

// OTA Update
#include <AsyncElegantOTA.h>
 
// Filesystem
#include <FS.h>

// JSON
#include <ArduinoJson.h>
 
// Standard HTTP-Port 80
AsyncWebServer server(80);
// Websockets
AsyncWebSocket ws("/ws");
// DNS for Wifi-Portal
DNSServer dns;


// WLAN-Zugangsdaten
const char* espHostname = "espdeck";

bool isPaired = true;

String deckConfig;
String fsFiles;


 
// Was passiert, wenn eine Seite nicht gefunden werden kann
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/html", "<center><h1>404 - error</h1></center>");
}


String processor(const String& var){
  if(var == "deckJSON"){
    return deckConfig;
    return String();
  } 
}
 
void setup() {
    
    initLed();
    delay(500);
    Serial.begin(115200);
    Serial.println();
    delay(500);
 
   
    // Wifi Portal
    AsyncWiFiManager wifiManager(&server,&dns);
    wifiManager.autoConnect("cooperESPDeck", "cooperESPDeck");

    while(!isPaired){
      Serial.println("pair:StreaMark");
      if(readPairing()){
        isPaired = true;
        Serial.println("Pairing gefunden");
        digitalWrite(2, LOW);
      }
      delay(100);
      yield();
    }
 
    // Dateisystem initialisieren
    if(SPIFFS.begin()){
      Serial.println("Dateisystem: initialisiert");
    }else{
      Serial.println("Dateisystem: Fehler beim initialisieren");
    }


    deckConfig = readConfig();
    Serial.println(deckConfig);

    // Starten des mDNS-Servers
    if (!MDNS.begin(espHostname)) {             
      Serial.println("Fehler beim Staren des mDNS-Servers!");
    }
 
    // Server Routen werden in einer eigenen Funktion definiert
    setupServerRoutes();
    // Datei Routen werden in einer eigenen Funktion definiert
    setupFilePaths();
    // Für die Websockets
    setupWebsockets();

    // OTA
    AsyncElegantOTA.begin(&server, "cooperESPDeck", "cooperESPDeck");

    // Server
    server.begin();

    delay(500);
    Serial.print("ip-adress:"); 
    Serial.print(WiFi.localIP());
    Serial.println(":started");
}
 
void loop() {
  // MDNS
  MDNS.update();
  // Websockets
  ws.cleanupClients();  
 
  static unsigned long nextPull;
  static unsigned int intervall = 250;
  if(millis() > nextPull){
    
    Serial.println("pair:StreaMark");
    
    Serial.print("ip-adress:"); 
    Serial.print(WiFi.localIP());
    Serial.println(":started");  
    
    nextPull = millis() + intervall;
  }
  
}
 
 
void setupServerRoutes(){
  // Als Document-Root wird der Ordner "data"
  // Alle Pfade müssen relativ vom Ordner "data" definiert werden
 
  // Datei index.html wird ausgeliefert
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

    fsFiles = getFSFiles();
    if(fsFiles.indexOf("index.html") > 0){
      request->send(SPIFFS, "/index.html");
    }else{
      request->redirect("/update");
    }
        
    
  });


  // Datei edit.html wird ausgeliefert
  server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/edit.html", String(), false, processor);
  });


  // API Endpoint
  server.on("/api/post", HTTP_POST, [](AsyncWebServerRequest * request){
        String message;
        int params = request->params();
        for (int i = 0; i < params; i++)
        {
            AsyncWebParameter *p = request->getParam(i);
            if (p->isPost()){
                String postData = p->value().c_str();
                Serial.println(postData);
                deckConfig = writeConfig(postData);
            }
        }
        request->send(200, "text/plain", "true");
    });
 
  server.onNotFound(notFound);
  
}
 
void setupFilePaths(){
  // In der index.html Datei wird eine .js und eine .css Datei verlinkt
  // diese können ohne die folgenden Zeilen nicht gefunden werden
 
  // Der  erste Pfad bezieht sich auf die .html-Datei
  // dort ist als Pfad zur CSS-Datei "assets/css/style.css" hinterlegt
  server.on("/assets/css/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    // also soll auch die Datei aus dem SPIFFS dementsprechend ausgeliefert werden
    request->send(SPIFFS, "/assets/css/style.css", "text/css");
  });
 
  server.on("/assets/js/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/assets/js/script.js", "text/javascript");
  });
 
  // Gleiches kann man auch mit Bildern anderen Dateien machen
  
}
 
 
void setupWebsockets(){
  ws.onEvent(onWebsocketsEvent);
  server.addHandler(&ws);  
}
 
void onWebsocketsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){
    //  Eine neue Websockets Verbindung wurde hergestellt
    //  Serial.println("Client Verbinung hergestellt!");

    client->text(deckConfig);

    
  }
  
  if(type == WS_EVT_DISCONNECT){
    //  Eine Websockets verbindung wurde getrennt
    //  Serial.println("Client Verbindung beendet!");
  }
 
  if(type == WS_EVT_DATA){
    //  Eine Websocket-Nachricht wurde empfangen
    //  Serial.println("onWebsocketsEvent();");
    handleWebSocketMessage(client, arg, data, len);  
  }
}
 
void notifyClients(String msg){
  ws.textAll(msg);  
}
 
 
//  Die zwei wichtigsten Funktionen um auf eine Nachricht zu antworten:
//  notifyClients("Nachricht an alle Clients");
//  client->text("Nachricht an den Client von dem die Nachricht kommt");
 
void handleWebSocketMessage(AsyncWebSocketClient * client, void *arg, uint8_t *data, size_t len) {
  //Serial.println("handleWebSocketMessage();");
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    
    //     message = Inhalt der Nachricht vom Websocket-CLient
    String message = String((char*)data);
    Serial.println(message);
 
    if(message.indexOf("connection:new") >= 0){
      //Serial.println("Ein neuer Client hat sich verbunden");  
      //notifyClients("user:new");
    }
    
    
  }
}

bool readPairing(){
  
  if (Serial.available()){
    String input = Serial.readString();
    input.trim();
    
    if(input == "pair:StreaMark"){
      return true;
    }
  }
    
}

void initLed(){
  pinMode(2, OUTPUT);  
  digitalWrite(2, HIGH);
}

String readConfig(){
  const char* datei = "config.json";
  File file = SPIFFS.open(datei, "r");
  if(!file){
    // Die Datei ist nicht vorhanden
    Serial.println("Die Datei " + String(datei) + " existiert nicht!");
    // Datei mit Schreibrechten öffnen, wird erstellt wenn nicht vorhanden
    File file = SPIFFS.open(datei, "w");
    Serial.println("Datei " + String(datei) + " wurde erstellt!");
    // Daten in die Datei schreiben
    file.print("{\"buttons\": [{\"hotkey\": \"strg+shift+b\", \"icon\": \"desktop\", \"name\": \"Screen 2\", \"type\": \"switch\", \"group\": 1}]}");
    // Schließen der Datei
    file.close();
    return "{\"buttons\": [{\"hotkey\": \"strg+shift+b\", \"icon\": \"desktop\", \"name\": \"Screen 2\", \"type\": \"switch\", \"group\": 1}]}";
  }else{
     // Die Datei ist vorhanden
     Serial.println("Die Datei " + String(datei) + " wurde geöffnet!");
     // Inhalt ausgeben oder verarbeiten
     String fileContent;
     for(int i=0;i<file.size();i++){
      fileContent += (char)file.read();
     }
     file.close();
     return fileContent;
  }
}

String writeConfig(String data){
  const char* datei = "config.json";
  File file = SPIFFS.open(datei, "w");
  if(!file){
    // Die Datei ist nicht vorhanden
    Serial.println("Die Datei " + String(datei) + " existiert nicht!");
    // Datei mit Schreibrechten öffnen, wird erstellt wenn nicht vorhanden
    File file = SPIFFS.open(datei, "w");
    Serial.println("Datei " + String(datei) + " wurde erstellt!");
    // Daten in die Datei schreiben
    file.print(data);
    // Schließen der Datei
    file.close();
    return data;
  }else{
     // Die Datei ist vorhanden
     Serial.println("Die Datei " + String(datei) + " wurde geöffnet!");
     // Inhalt ausgeben oder verarbeiten
     file.print(data);
     file.close();
     return data;
  }
}


String getFSFiles(){
  String content;
  Dir dir = SPIFFS.openDir("");
  while (dir.next()){
    content += dir.fileName() + ":";
  }
  return content;
}

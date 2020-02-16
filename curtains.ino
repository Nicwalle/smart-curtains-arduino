#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server;
const int stepPin = 13; 
const int dirPin = 12; 
const int enPin = 14;

const int openButton = 16;
const int closeButton = 5;

const int relayPin = 2;

char *ssid = "Nicwalle-perso";
char *password = "0561124438410343";

int openState = 0;
int closedState = 4200;

int state = 0;
int objective = 0;

void setup(){
  initPins();
  configureWifi();

  server.on("/reset", resetState);
  server.on("/set-bounds", resetState);
  server.on("/set-state", resetState);
  server.on("/get-state", getState);
  server.on("/goto", setObjective);
  server.on("/open", setObjective);
  server.on("/close", setObjective);
  server.on("/stop", setObjective);
  
  server.begin();
}

void loop(){
  if (digitalRead(openButton)== HIGH) {
    objective = objective + 1;
  } else if (digitalRead(closeButton)== HIGH) {
    objective = objective - 1;
  } 
  
  if (state != objective) {
    rotate();  
  } else {
    digitalWrite(relayPin, LOW);
  }
  server.handleClient();
  
}

void step() {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(1000); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(1000); 
}

void rotate() {
  if (state < objective) {
    digitalWrite(relayPin, HIGH);
    digitalWrite(dirPin,LOW);
    step();
    state = state + 1;
  } else if (state > objective) {
    digitalWrite(relayPin, HIGH);
    digitalWrite(dirPin,HIGH);
    step();
    state = state - 1;
  }
}

void resetState(){
  if (server.hasArg("open")) {
    openState = server.arg("open").toInt();
  }

  if (server.hasArg("closed")) {
    closedState = server.arg("closed").toInt();
  }

  if (server.uri() == "/reset") {
    state = 0;
    objective = 0;
  }
  if (server.uri() == "/set-state") {
    if (server.hasArg("state")) {
      state = server.arg("state").toInt();
      objective = server.arg("state").toInt();
    }
  }
  getState();
}

void getState(){
  char response[100];
  sprintf(response, "{\"state\":%d,\"objective\":%d,\"openState\":%d,\"closedState\":%d}", state, objective, openState, closedState);
  server.send(200, "application/json", response);
}

void setObjective() {
  char response[150];
  String uri = server.uri();
  if (uri == "/stop") {
    objective = state;
    getState();
  } else if (uri == "/goto" && server.hasArg("goal")) {
    int goal = server.arg("goal").toInt();
    if (closedState < goal || goal < openState){
      sprintf(response, "{\"status\":\"BAD REQUEST\", \"message\":\"Goal parameter must be in interval [%d;%d]\",\"state\":%d,\"objective\":%d,\"openState\":%d,\"closedState\":%d}", openState, closedState, state, objective, openState, closedState);
      server.send(400, "application/json", response);
    } else {
      objective = goal;
      getState();
    }
  } else if (uri == "/open") {
    objective = openState;
    getState();
  } else if (uri == "/close") {
    objective = closedState;
    getState();
  } else {
    sprintf(response, "{\"status\":\"BAD REQUEST\", \"message\":\"Insert goal parameter\",\"state\":%d,\"objective\":%d,\"openState\":%d,\"closedState\":%d}", state, objective, openState, closedState);
    server.send(400, "application/json", response);
  }
}

/*********************
 *                   *
 *       SETUP       *
 *                   *
 *********************/

void initPins() {
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  pinMode(enPin,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(openButton, INPUT);
  pinMode(closeButton, INPUT);

  pinMode(relayPin,OUTPUT);
  
  digitalWrite(enPin,LOW);
  digitalWrite(LED_BUILTIN,HIGH);
  digitalWrite(relayPin,LOW);
}

void configureWifi() {
  WiFi.begin(ssid, password);
  Serial.begin(115200);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.print("IP address:");
  Serial.println(WiFi.localIP());
}

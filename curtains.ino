#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server;
const int stepPin = 13; 
const int dirPin = 12; 
const int enPin = 14;

const int openButton = 16;
const int closeButton = 5;

const int relayPin = 4;

char *ssid = "Nicwalle-perso";
char *password = "0561124438410343";

int openState = 0;
int closedState = 4200;

int state = 0;
int objective = 0;

/*
 * FSM : Finite state machine to define the starting/ending phases
 * FSM = 0 : Idle/Starting phase (delay between relay and first step)
 *     = 1 : Running phase
 *     = 2 : Stopping phase (delay before opening relay)
 */
int fsm = 0; 

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
  server.on("/left", setObjective);
  server.on("/right", setObjective);
  
  server.begin();
}

void loop(){
  /*
  if (digitalRead(openButton)== HIGH) {
    // Serial.println("Line 51. Open button high");
    objective = objective + 1;
  } else if (digitalRead(closeButton)== HIGH) {
    // Serial.println("Line 54. Close button high");
    objective = objective - 1;
  } 
  */
  
  if (state != objective) {
    if (fsm == 0) {
      digitalWrite(relayPin, HIGH);
      delay(150);
      fsm = 1;
    }
    rotate();  
  } else {
    if (fsm == 1) {
      delay(150);
      digitalWrite(relayPin, LOW);
      fsm = 0;
    }
  }
  server.handleClient();
  
}

void step() {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(800); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(800); 
}

void rotate() {
  if (state < objective) {
    digitalWrite(dirPin,LOW);
    step();
    state = state + 1;
  } else if (state > objective) {
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

/**
 * Returns the current state of the curtains:
 *   {
 *      state: int,
 *      objective: int,
 *      openState: int,
 *      closedState: int
 *   }
 **/
void getState(){
  char response[100];
  sprintf(response, "{\"state\":%d,\"objective\":%d,\"openState\":%d,\"closedState\":%d}", state, objective, openState, closedState);
  server.send(200, "application/json", response);
}

/**
 * Sets the position objective according to the requested uri:
 * Full API: https://github.com/Nicwalle/smart-curtains-arduino/blob/master/README.md
 **/
void setObjective() {
  char response[150];
  String uri = server.uri();
  if (uri == "/stop") { // STOP
    objective = state;
    getState();
  } else if (uri == "/goto" && server.hasArg("goal")) { // GOTO
    int goal = server.arg("goal").toInt();
    if (closedState < goal || goal < openState){
      sprintf(response, "{\"status\":\"BAD REQUEST\", \"message\":\"Goal parameter must be in interval [%d;%d]\",\"state\":%d,\"objective\":%d,\"openState\":%d,\"closedState\":%d}", openState, closedState, state, objective, openState, closedState);
      server.send(400, "application/json", response);
    } else {
      objective = goal;
      getState();
    }
  } else if (uri == "/open") { // OPEN
    objective = openState;
    getState();
  } else if (uri == "/close") { // CLOSE
    objective = closedState;
    getState();
  } else if  (uri == "/left") { // LEFT
    objective = state - 100;
    getState();
  } else if  (uri == "/right") { // RIGHT
    objective = state + 100;
    getState();
  } else { // BAD REQUEST
    sprintf(response, "{\"status\":\"BAD REQUEST\", \"message\":\"Insert goal parameter\",\"state\":%d,\"objective\":%d,\"openState\":%d,\"closedState\":%d}", state, objective, openState, closedState);
    server.send(400, "application/json", response);
  }

  if(objective < openState) {
    objective = openState;
  } else if (objective > closedState) {
    objective = closedState;  
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

  pinMode(openButton, INPUT);
  pinMode(closeButton, INPUT);

  pinMode(relayPin,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  
  digitalWrite(enPin,LOW);
  digitalWrite(relayPin,LOW);
  digitalWrite(LED_BUILTIN,HIGH);
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

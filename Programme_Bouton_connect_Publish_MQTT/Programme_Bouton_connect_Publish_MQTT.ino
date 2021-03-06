/*
 * Auteur: xTinox
 * Titre: Bouton sur ESP 8266, connecté au serveur MQTT
 * Version: 0.1
 * Date: 08/03/2020
 * Commentaire(s): Bouton connecté + publication de l'état du poste (variable: RESULTAT)
 * 
 * ATTENTION: Avant de téléverser veuillez aller dans Préférences/Ajouter URL et copier/coller l'URL suivant: http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *            Puis aller dans l'onglet Croquis de l'IDE Arduino /Inclure une bibliothèque/Gérer les bibliothèques/ puis rechercher et télécharger ESP8266Wifi.h ainsi que PubSubClient.h
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"fr.pool.ntp.org",7200);

const char* mqtt_server = "test.mosquitto.org";
int mqtt_port = 1883;

const int led_rouge = 13;         // pin D7 du ESP 8266
const int led_verte = 12;         // pin D6 du ESP 8266
const int led_bleue = 14;         // pin D5 du ESP 8266
const int pinBouton = 5;         // pin D1 du ESP 8266
int boucle=20;             // Variables:
int etatBouton;                 //
int tempo;                      //
int attente=4;                  //
int attente2=2;                 //
int compteur=6000;              //
int resultat=0;                 //  
int RESULTAT=0;                 //
int valEnregistree = 0;         //
int freqAllum = 4;
int duree = 10;

// Declaration "espClient" pour serveur MQTT
WiFiClient espClient;
PubSubClient client(espClient);

WiFiManager wifiManager;

// Fonction: Allumage de la led RGB
void led(int r, int v, int b){
  analogWrite(led_rouge, r);
  analogWrite(led_verte, v);
  analogWrite(led_bleue, b);
}

void connecter_au_Wifi(){
  for (int i=0; i<3; i++){ 
    for (int i=0; i<255; i++){
      led(i,i,0);
      delay(freqAllum-2);
    }
    for (int i=255; i>0; i--){
      led(i,i,0);
      delay(freqAllum-2);
    }
  }
}

void Reset_le_Wifi(){
  for (int i=0; i<5; i++){ 
    for (int i=0; i<255; i++){
      led(i,0,0);
      delay(freqAllum-3);
    }
    for (int i=255; i>0; i--){
      led(i,0,0);
      delay(freqAllum-3);
    }
  }
}

void Result(){
  if (resultat==0){
    for (int i=0; i<255; i++){
      led(i,i,i);
      delay(freqAllum+3);
    }
  }
  else if (resultat==1){
    for (int i=0; i<255; i++){
      led(0,0,i);
      delay(freqAllum+3);
    }
  }
  else if (resultat==2){
    for (int i=0; i<255; i++){
      led(0,i,0);
      delay(freqAllum+3);
    }
  }
  else if (resultat==3){
    for (int i=0; i<255; i++){
      led(i,0,0);
      delay(freqAllum+3);
    }
  }
  RESULTAT=resultat;
  affich();
}

// Boucle d'attente du mode En Panne
void EnPanne(){
  tempo=0;
  resultat=3;
  while (tempo<=attente2){  
    for (int i=0; i<255; i++){
      led(i,0,0);
      delay(freqAllum);
    }
    for (int i=255; i>0; i--){
      led(i,0,0);
      delay(freqAllum);
    }
    tempo=tempo+1;
    etatBouton = digitalRead(pinBouton);
    if (etatBouton==1){
      return DebutDeTache();
    }
  }
}

// Boucle d'attente du mode Fin de tâche
void FinDeTache(){
  tempo=0;
  resultat=2;
  while (tempo<=attente2){  
    for (int i=0; i<255; i++){
      led(0,i,0);
      delay(freqAllum);
    }
    for (int i=255; i>0; i--){
      led(0,i,0);
      delay(freqAllum);
    }
    tempo=tempo+1;
    etatBouton = digitalRead(pinBouton);
    if (etatBouton==1){
      return EnPanne();
    }
  }
}

// Boucle d'attente du mode Début de tâche
void DebutDeTache(){
  tempo=0;
  resultat=1;
  while (tempo<=attente2){  
    for (int i=0; i<255; i++){
      led(0,0,i);
      delay(freqAllum);
    }
    for (int i=255; i>0; i--){
      led(0,0,i);
      delay(freqAllum);
    }
    tempo=tempo+1;
    etatBouton = digitalRead(pinBouton);
    if (etatBouton==1){
      return FinDeTache();
    }
  }
}

// Boucle du mode Edition
void Edition(){
  tempo=0;
  while (tempo<=attente){  
    for (int i=0; i<255; i++){
      led(i,i,i);
      delay(freqAllum);
    }
    for (int i=255; i>0; i--){
      led(i,i,i);
      delay(freqAllum);
    }
    tempo=tempo+1;
    etatBouton = digitalRead(pinBouton);
    if (etatBouton==1){
      return DebutDeTache();
    }
  }
}

// Fonction de Rappel 
void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message arrivé [");
 Serial.print(topic);
 Serial.print("] ");
 for (int i = 0; i<length; i++) {
  Serial.print((char)payload[i]);
 }
 Serial.println();
}
 
void reconnect() {
 // Boucle jusqu'à être reconnecté
 while (!client.connected()) {
  Serial.print("Attente d'une connection MQTT...");
  // Attent d'être connecté
  if (client.connect("ESP8266Client")) {
    Serial.println("connecté");
  } 
  else {
    Serial.print("erreur, rc=");
    Serial.print(client.state());
    Serial.println(" nouvel essai dans 5 secondes");
    // Attente de 5 secondes avant de réessayer de se connecter
    delay(500);
  }
 }
}

void principale(){
  etatBouton = digitalRead(pinBouton);
  delay(20);
  if (etatBouton==1){
      compteur=0;
    }
  while (compteur<=boucle-1){  
    etatBouton = digitalRead(pinBouton);
    delay(20);
    
    if (etatBouton==1){
      compteur=compteur+1;
    }
    else{
      compteur=boucle+1;
    }
    Serial.println(compteur);
    if (compteur==boucle){  
    Edition();
    Result();
    }
    delay(10);
  }
  affich();
}

// Fonction d'affichage (PUBLISH) sur le serveur MQTT
void affich(){
  if (!client.connected()) {
    reconnect();
  }
  timeClient.update();
  client.publish("horizontel1", String(RESULTAT).c_str());
  client.publish("horizontel2", String(timeClient.getFormattedTime()).c_str());
  client.loop();
  delay(1000);
}

// Fonction nécessaire à l'installation (initialisation, déclaration des ports, connexion au Wifi)
void setup() {
  
    Serial.begin(9600);
    pinMode(led_rouge, OUTPUT);
    pinMode(led_verte, OUTPUT);
    pinMode(led_bleue, OUTPUT);

    etatBouton = digitalRead(pinBouton); //Lire état bouton, si ce dernier est appuyé (à l'instant du démarrage) la Wifi sera réinitialisée
    delay(20);
    if (etatBouton == 1){
      delay(20);
      wifiManager.resetSettings();
      Reset_le_Wifi();
    }
    // IP choisi
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    wifiManager.autoConnect("Bouton Horizon Telecom","horizontel"); //(Mode "Point d'Accès"),il devient un point d'accès Wifi. 
    //On configurera sa connection au réseau de l'entreprise grâce à un portail de connection 

    // Si l'ESP a réussi à se connecter sur la Wifi (Mode "Station")
    Serial.println("connecté :)");
    connecter_au_Wifi();
    // Démarrage du client NTP
    timeClient.begin();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
   
    Serial.println("Connecté");
    Serial.print("Serveur MQTT ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.println(String(mqtt_port)); 
    Serial.print("ESP8266 IP ");
    Serial.println(WiFi.localIP()); 
    Serial.println("Modbus RTU Master Online");
    etatBouton = digitalRead(pinBouton);
    delay(20);
    if (etatBouton == 1){
      delay(20);
      principale();
    }
    affich();
    //ESP.deepSleep(duree*1000000);
}


// Boucle infinie
void loop() {
 principale();
}

#include <SoftwareSerial.h>
#include "ddr2ascii.h"
#include "ascii2ddr.h"

#define PC_BAUD 9600

//#define DTD_PIN 14
#define RTS_PIN 23
#define ERIKA_RX 18
#define ERIKA_TX 19
#define ERIKA_BAUD 1200

SoftwareSerial erika(ERIKA_RX, ERIKA_TX); // RX, TX
bool wait = false;

#include <WiFi.h>             
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

const char* ssid     = "xxxxx"; // hier eigenen WLan_Namen eintragen 
const char* password = "xxxxx"; // hier eigenes WLan_Passwort eintragen 
String WiFi_name;
unsigned long previousMillis = 0;
unsigned long interval = 30000;
int WiFi_Count;

String token = "xxxxx";         // hier openAI API Key eintragen 
int max_tokens = 256;
//int max_tokens = 1024;
char Nachricht_array[5000];

String Eingabe = "";
String Ausgabe = "";
int Zeilenlaenge = 70;
String Feedback = "";
int Nachricht_len = 0;
//int merker_2 = 0;
//int more = 0;
int Stelle = 0;
int Sonderzeichen;

void setup()
{
  delay(1000);
  Serial.begin(115200);
  delay(100);
  Serial.println("geht los ...");
  pinMode(RTS_PIN, INPUT);
  erika.begin(ERIKA_BAUD);
  WiFi_Init();
  while (erika.read() > 0) Serial.println("Startup Zeichen leeren");
  erika.write(ascii2ddr['\n']);
}

void loop(){

  if (Nachricht_len > 0)
  {
    int rtsState = digitalRead(RTS_PIN);
    int result = 0;
    int result_2 = 0;

    if (rtsState == LOW && wait == false){
      char Zeichen = Nachricht_array[Stelle];
      //Serial.print("Stelle: "); Serial.print(Stelle);
      result = ascii2ddr[Zeichen];
      //Serial.print(" - result: "); Serial.print(result);
      char Zeichen_2 = Nachricht_array[Stelle + 1];
      result_2 = ascii2ddr[Zeichen_2];
      //Serial.print(" - result_2: "); Serial.println(result_2);
      if (result == 34 && result_2 == 41) {erika.write(0x47); Sonderzeichen = 1;} //ß
      else if (result == 34 && result_2 == 75) {erika.write(0x65); Sonderzeichen = 1;}//ä
      else if (result == 34 && result_2 == 82) {erika.write(0x66); Sonderzeichen = 1;}//ö
      else if (result == 34 && result_2 == 150) {erika.write(0x67); Sonderzeichen = 1;}//ü
      else if (result == 34 && result_2 == 32) {erika.write(0x3F); Sonderzeichen = 1;}//Ä
      else if (result == 34 && result_2 == 77) {erika.write(0x3C); Sonderzeichen = 1;}//Ö
      else if (result == 34 && result_2 == 71) {erika.write(0x3A); Sonderzeichen = 1;}//Ü
      else if (result == 33 && result_2 == 65) {erika.write(0x39); Sonderzeichen = 1;}//°
      else if (result == 33 && result_2 == 124) {erika.write(0x3D); Sonderzeichen = 1;}//§
      else if (Sonderzeichen == 1) Sonderzeichen = 0;
      else erika.write(result);
      Stelle = Stelle + 1;
      wait = true;
      delay(100);
    }
    else
    {
      //rts was high, which means erika is ready for the next byte, when rts goes low again
      wait = false;
    }
   if (Stelle == Nachricht_len) {
      Nachricht_len = 0;
      Stelle = 0;
      erika.write(ascii2ddr['\n']); erika.write(ascii2ddr['\n']);
      while (erika.read() > 0) Serial.println("Ende Zeichen leeren");
    }
  }

  if (erika.available())
  {
    int Zeichen_nummer = erika.read();
    //Serial.print(Zeichen_nummer);
    char Zeichen = ' ';
    char Umlaut = ' ';
    Zeichen = ddr2ascii[Zeichen_nummer];
    // Do nothing bei:
    if (Zeichen_nummer == 114); // Backtab
    if (Zeichen_nummer == 117); // Einzug
    if (Zeichen_nummer == 118); // Papier zurück
    if (Zeichen_nummer == 121); // Tab
    if (Zeichen_nummer == 252); // Korrektur

    if (Zeichen_nummer == 101) { Zeichen = 'a'; Umlaut = 'e'; }
    if (Zeichen_nummer == 102) { Zeichen = 'o'; Umlaut = 'e'; }
    if (Zeichen_nummer == 103) { Zeichen = 'u'; Umlaut = 'e'; }
    if (Zeichen_nummer == 71)  { Eingabe = Eingabe + 's'; Zeichen = 's'; }
    if (Umlaut == 'e')  { Eingabe = Eingabe + Umlaut; }
    if (Zeichen == 's') {Eingabe = Eingabe + Zeichen; }

    if (Zeichen != '\n') {
      Eingabe = Eingabe + Zeichen;
     }
    if (Zeichen == '\n') {
      Serial.println(openAI_text(Eingabe));
      Eingabe = "";
    }
  }
}
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

const char* ssid     = "xxxx"; // hier eigenen WLan_Namen eintragen 
const char* password = "xxxx"; // hier eigenes WLan_Passwort eintragen 
String WiFi_name;
unsigned long previousMillis = 0;
unsigned long interval = 30000;
int WiFi_Count;

String token = "xxxx";         // hier openAI API Key eintragen 
int max_tokens = 256;          // Für längere Antworten erhöhen
char Nachricht_array[4096];    // Bei größeren max_tokens das Nachricht_array auch vergrößern

String Eingabe = "";
String Ausgabe = "";
int Zeilenlaenge = 70;         // Auf der Erika ausgegebene Zeichen pro Zeile. Verringern falls Zeichen abgeschnitten werden
String Feedback = "";
int Nachricht_len = 0;
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
  // Mit dem WLan verbinden 
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi .."); 
  while (WiFi.status() != WL_CONNECTED) { 
    Serial.print('.'); 
    delay(1000); 
  } 
  Serial.println(WiFi.localIP());   
  while (erika.read() > 0) Serial.println("Startup Zeichen leeren");
  erika.write(ascii2ddr['\n']);
}

void loop(){

  // EINGABE ###############################################################################
  if (erika.available())
  {
    int Zeichen_nummer = erika.read();
    //Serial.print(Zeichen_nummer);
    char Zeichen = ' ';
    char Umlaut = ' ';
    Zeichen = ddr2ascii[Zeichen_nummer];

    switch (Zeichen_nummer) {
      case 71:  { Eingabe = Eingabe + "ss"; break; } // Eszett zu ss
      case 101: { Eingabe = Eingabe + "ae"; break; } // ä zu ae
      case 102: { Eingabe = Eingabe + "oe"; break; } // ö zu oe
      case 103: { Eingabe = Eingabe + "ue"; break; } // ü zu ue
      case 114: { break; }                           // Backtab ignorierenn
      case 117: { break; }                           // Einzug ignorieren
      case 118: { break; }                           // Papier zurück ignorieren
      case 119: { break; }                           // Newline hier ignorieren
      case 121: { break; }                           // Tab ignorieren
      default: {                                   // Druckbare Zeichen anhängen 
        if (Zeichen_nummer > 127) {} // do nothing
        else {Eingabe = Eingabe + Zeichen; }
        break;
      }
    }
    if (Zeichen == '\n') {
      Serial.println(Eingabe);
      
      String getResponse = "";
      int merker = 0;
    
      Serial.println("Sending API Request...");
    
      WiFiClientSecure client_tcp;
      client_tcp.setInsecure();   //run version 1.0.5 or above
    
      String request = "{\"model\":\"text-davinci-003\",\"prompt\":\"" + Eingabe + "\",\"temperature\":0.7,\"max_tokens\":" + String(max_tokens) + ",\"frequency_penalty\":0,\"presence_penalty\":0.6,\"top_p\":1.0}";
      Serial.print("request: "); Serial.println(request);
    
      if (client_tcp.connect("api.openai.com", 443)) {
        client_tcp.println("POST /v1/completions HTTP/1.1");
        client_tcp.println("Connection: close");
        client_tcp.println("Host: api.openai.com");
        client_tcp.println("Authorization: Bearer " + token);
        client_tcp.println("Content-Type: application/json; charset=utf-8");
        client_tcp.println("Content-Length: " + String(request.length()));
        client_tcp.println();
        client_tcp.println(request);
    
        boolean state = false;
        int waitTime = 40000;   // timeout 40 seconds
        long startTime = millis();
        while ((startTime + waitTime) > millis()) {
          Serial.print(".");
          erika.write(ascii2ddr['.']);
          delay(1000);
          // UPDATE 21.06.2023
          while (client_tcp.available()) {
              char c = client_tcp.read();
              if (state==true) {
                Feedback += String(c);
              // Anpassung wg. Chat-GPT Änderung:
              if (Feedback.indexOf("\"text\":")!=-1)
                  Feedback = ""; 
              // Anpassung wg. Chat-GPT Änderung:
              if (Feedback.indexOf("\"index\":")!=-1) {
                client_tcp.stop(); 
                Serial.println();
                // Anpassung wg. Chat-GPT Änderung:
                Feedback = Feedback.substring(6,Feedback.length()-17); 
                }
              }
              if (c == '\n') {
                if (getResponse.length()==0) state=true;
                getResponse = "";
              }
              else if (c != '\r')
                getResponse += String(c);
              startTime = millis();
            }
            if (getResponse.length()>0) break;
        }
        client_tcp.stop();
      }
      else Serial.println("Connection failed");
      // Ende ChatGPT
    
      erika.write(ascii2ddr['\n']);
      Nachricht_len = Feedback.length()+1;
      Serial.print("Nachricht_len: "); Serial.println(Nachricht_len);
      Feedback.toCharArray(Nachricht_array, Nachricht_len);
    
      for (int j=0; j<=Nachricht_len; j++){
        delay(1);
        if (Nachricht_array[j] == '\\' && Nachricht_array[j+1] == 'n' && Nachricht_array[j+2] == '\\' && Nachricht_array[j+3] == 'n')
        {
          Nachricht_array[j]   = ' ';
          Nachricht_array[j+1] = '\n';
          Nachricht_array[j+2] = ' ';
          Nachricht_array[j+3] = '\n';
        }
      }
    
      for (int j=0; j<=Nachricht_len; j++){
        delay(1);
        if (Nachricht_array[j] == '\\' && Nachricht_array[j+1] == 'n' && Nachricht_array[j+2] != '\\' && Nachricht_array[j+3] != 'n')
        {
          Nachricht_array[j]   = ' ';
          Nachricht_array[j+1] = '\n';
        }
      }
    
      // CR setzen bei erreichen Zeilenlaenge
      for (int i = 0; i <= Nachricht_len; i++){
        if (Nachricht_array[i] == ' ') {
          for (int j = i + 1; j <= Nachricht_len; j++){
            if (Nachricht_array[j] == ' '  || j == Nachricht_len) {
              if (j <= Zeilenlaenge + merker) i = j;
              if (j > Zeilenlaenge + merker) {
                Nachricht_array[i] = '\n';
                merker = i + 1;
              }
            }
            if (Nachricht_array[j] == '\n') {
              merker = i + 1;
              i = j;
            }
          }
        }
      }    
      Serial.println("Nachricht_array: "); Serial.println(Nachricht_array);
      Eingabe = "";
    }
  }
  // EINGABE ENDE ##########################################################################

  // AUSGABE ###############################################################################
  if (Nachricht_len > 0)
  {
    int rtsState = digitalRead(RTS_PIN);
    int result = 0;
    int result_2 = 0;

    if (rtsState == LOW && wait == false){
      char Zeichen = Nachricht_array[Stelle];
      result = ascii2ddr[Zeichen];
      char Zeichen_2 = Nachricht_array[Stelle + 1];
      result_2 = ascii2ddr[Zeichen_2];
      if (result == 34 && result_2 == 41) {erika.write(0x47); Sonderzeichen = 1;} //ß
      else if (result == 34 && result_2 == 75) {erika.write(0x65); Sonderzeichen = 1;}//ä
      else if (result == 34 && result_2 == 82) {erika.write(0x66); Sonderzeichen = 1;}//ö
      else if (result == 34 && result_2 == 150){erika.write(0x67); Sonderzeichen = 1;}//ü
      else if (result == 34 && result_2 == 32) {erika.write(0x3F); Sonderzeichen = 1;}//Ä
      else if (result == 34 && result_2 == 77) {erika.write(0x3C); Sonderzeichen = 1;}//Ö
      else if (result == 34 && result_2 == 71) {erika.write(0x3A); Sonderzeichen = 1;}//Ü
      else if (result == 33 && result_2 == 65) {erika.write(0x39); Sonderzeichen = 1;}//°
      else if (result == 33 && result_2 == 124){erika.write(0x3D); Sonderzeichen = 1;}//§
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
  // AUSGABE ENDE ##########################################################################  
  
}

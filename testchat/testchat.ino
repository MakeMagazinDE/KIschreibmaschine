#include <WiFi.h>             
#include <WiFiClientSecure.h> 
const char* ssid     = ""; // hier eigenen WLan_Namen eintragen 
const char* password = ""; // hier eigenes WLan_Passwort eintragen 
String token = "";         // hier openAI API Key eintragen 
int max_tokens = 256;      // Die Größe reicht für einfache Antworten. Für längere Antworten muss dieser Wert erhöht werden. Maximum ist 2.048 
String Request = ""; 
String Feedback = ""; 

void setup() { 
  Serial.begin(115200); 
  // Mit dem WLan verbinden 
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password); 
  Serial.print("Connecting to WiFi .."); 
  while (WiFi.status() != WL_CONNECTED) { 
    Serial.print('.'); 
    delay(1000); 
  } 
  Serial.println(WiFi.localIP()); 
} 

void loop() { 
  // Speichern der Eingabe im seriellen Monitor in der Variablen "Request" 
  while (Serial.available() > 0) { 
    // So lange lesen, bis return \n eingegeben wurde 
    Request = Serial.readStringUntil('\n'); 
    // Das letzte Zeichen ist return -> soll entfernt werden   
    Request = Request.substring(0, Request.length() - 1); 
    Serial.print("Request: "); Serial.println(Request); 
  }   
  // Eingabe ist abgeschlossen, jetzt erfolgt die Interaktion mit ChatGPT 
  if(Request != ""){ 
    String getResponse = ""; 
    WiFiClientSecure client_tcp; 
    // Dieser Eintrag ist notwendig, um mit https kommunizieren zu können 
    client_tcp.setInsecure(); 
    // Die Eingabewerte werden in der Variable "Request" zusammengestellt 
    Request = "{\"model\":\"text-davinci-003\",\"prompt\":\"" + Request + "\",\"temperature\":0.7,\"max_tokens\":" + String(max_tokens) + ",\"frequency_penalty\":0,\"presence_penalty\":0.6,\"top_p\":1.0}"; 
    // Der Aufruf wird nachfolgend an ChatGPT geschickt 
    if (client_tcp.connect("api.openai.com", 443)) { 
      client_tcp.println("POST /v1/completions HTTP/1.1"); 
      client_tcp.println("Connection: close"); 
      client_tcp.println("Host: api.openai.com"); 
      client_tcp.println("Authorization: Bearer " + token); 
      client_tcp.println("Content-Type: application/json; charset=utf-8"); 
      client_tcp.println("Content-Length: " + String(Request.length())); 
      client_tcp.println(); 
      client_tcp.println(Request); 
      boolean state = false; 
      int waitTime = 40000;   // Timeout bei 40 Sekunden 
      long startTime = millis(); 
      // Wir warten auf die Antwort 
      while ((startTime + waitTime) > millis()) { 
        Serial.print("."); 
        delay(200);       
          while (client_tcp.available()) {
              char c = client_tcp.read();
              if (state==true) {
                Feedback += String(c);
              // Update 21.06.2023
              // Anpassung wg. Chat-GPT:
              if (Feedback.indexOf("\"text\":")!=-1)
                  Feedback = ""; 
              // Anpassung wg. Chat-GPT:
              if (Feedback.indexOf("\"index\":")!=-1) {
                client_tcp.stop(); 
                Serial.println();
                // Anpassung wg. Chat-GPT:
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
    // Feedback wird im seriellen Monitor ausgegeben 
    Serial.println(Feedback); 
    Request = ""; 
  } 
} 

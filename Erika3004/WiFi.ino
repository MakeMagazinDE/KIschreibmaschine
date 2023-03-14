void WiFi_Init(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  wifiMulti.addAP("ssid_1", "password_1");
  wifiMulti.addAP("ssid_2", "password_2");
  Serial.print("Connecting to WiFi ");
  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.printf("Connected to %s\n", WiFi.SSID().c_str());
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
  }
}

void monitorWiFi(){
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
}

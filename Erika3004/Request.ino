String openAI_text(String request) {

  String getResponse = "";
  int merker = 0;

  Serial.println("Sending API Request...");

  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above

  request = "{\"model\":\"text-davinci-003\",\"prompt\":\"" + request + "\",\"temperature\":0.7,\"max_tokens\":" + String(max_tokens) + ",\"frequency_penalty\":0,\"presence_penalty\":0.6,\"top_p\":1.0}";
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
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (state==true) {
            Feedback += String(c);
            if (Feedback.indexOf("\"text\":\"\\n\\n")!=-1)
                Feedback = "";
            if (Feedback.indexOf("\",\"index\"")!=-1) {
              client_tcp.stop();
              Serial.println();
              Feedback = Feedback.substring(0,Feedback.length()-9);
              //Serial.print("Feedback: "); Serial.println(Feedback);
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
  /*
  for (int i = merker_2; i <= Nachricht_len - 1; i++){
    Ausgabe = Ausgabe + Nachricht_array[i];
    if (Nachricht_array[i] == '\n') {
      delay(10);
      Ausgabe = "";
    }
    if (i == Nachricht_len - 1){
      delay(10);
      Ausgabe = "";
      more = 0;
    }
  }
  */
  Serial.println("Nachricht_array: "); Serial.println(Nachricht_array);
}

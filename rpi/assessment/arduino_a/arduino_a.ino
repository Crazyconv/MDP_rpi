void setup{
  Serial.begin(115200);
}

void loop{
  if(Serial.available()){
    char newChar=Serial1.read();
    Serial.write(newChar+1);
  }
}

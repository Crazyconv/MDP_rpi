void setup(){
  Serial.begin(9600);
}

void loop(){
  String content = "";
  char character;
  if(Serial.available()){
    character = Serial.read();
    if(character == '\0'){
      Serial.println(content);
    }else{
      content.concat(character);
    }
  }
}

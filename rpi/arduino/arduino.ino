void setup(){
  Serial.begin(9600);
}

void loop(){
  char content[256] = "";
  char character;
  int index = 0;
  while(1){
    if(Serial.available()){
      character = Serial.read();
      if(character == '\\'){
        content[index] = character;
        Serial.print("Arduino receive your message: ");
        Serial.print(content);
        break;
      }else{
        content[index] = character;
        index ++;
      }
    }
  }
}

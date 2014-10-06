void setup(){
  Serial.begin(115200);
}

void loop(){
  //int feedback = irSensorFeedback(A0);
  //Serial.println(feedback);
  
  //int average = averageFeedback(7,4,A3);
  //Serial.println(average);
  
  int distance = calculateDistance(A0);
  Serial.println(distance);
  
  delay(500);
}

int calculateDistance(int pin){
  int average = averageFeedback(30,15,pin);
  int distance = average;
  return distance;
}

int irSensorFeedback(int pin){
  return analogRead(pin);
}

int averageFeedback(int in, int out, int pin){
  int x[in];
  int i;
  int sum = 0;
  int start = (in - out)/2;
  int average;
  for(i=0;i<in;i++){
    x[i] = analogRead(pin);
    Serial.print(x[i]);
    Serial.print(" ");
  }
  Serial.println();
  quickSort(x, 0, in-1);
  for(i = start; i < start+out; i++){
    sum = sum + x[i];
    Serial.print(x[i]);
    Serial.print(" ");
  }
  Serial.println();
  average = (sum+5)/out;
  return average;
}

void quickSort(int x[],int first,int last){
    int pivot,j,temp,i;

    if(first<last){
       pivot=first;
       i=first;
       j=last;

       while(i<j){
           while(x[i]<=x[pivot]&&i<last)
               i++;
           while(x[j]>x[pivot])
               j--;
           if(i<j){
               temp=x[i];
               x[i]=x[j];
               x[j]=temp;
           }
       }

       temp=x[pivot];
       x[pivot]=x[j];
       x[j]=temp;
       quickSort(x,first,j-1);
       quickSort(x,j+1,last);

   }
}

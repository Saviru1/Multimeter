//Variables
float input_volt = 0.0;
float temp=0.0;
float r1=700000.0;    
float r2=500000.0;      


void setup()
{
   Serial.begin(9600);     
}


void loop()
{
    int analogvalue = analogRead(A0);
    temp = (analogvalue * 5.0) / 1023.0;       //ADC conversion
    input_volt = temp / (r2/(r1+r2));          //Scaling

    Serial.print("V = ");                 
    Serial.println(input_volt);

    delay(300);

}
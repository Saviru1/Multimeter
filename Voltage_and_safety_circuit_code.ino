#include <LiquidCrystal.h>  // Library to control the LCD display

// Initialize the LCD with appropriate pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Variables for current measurement
float input_current = 0.0;
float temp_current = 0.0;
float r1_current = 33.0; // Resistor value for current measurement mA
float r2_current = 3300.0; // Resistor value for current measurement uA

// Variables for voltage measurement
float input_voltage = 0.0;
float temp_voltage = 0.0;
float r1_voltage = 700000.0; // Resistor divider value R1 (ohms)
float r2_voltage = 500000.0; // Resistor divider value R2 (ohms)
//////
float r3_voltage=1000000.0;

void setup() {
  Serial.begin(9600);      // Open serial port, set data rate to 9600 bps
  lcd.begin(16, 2);        // Set up the LCD's number of columns and rows
  lcd.print("MULTIMETER");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read analog values from the respective pins
  int analogvalue_voltage = analogRead(A0); // Read from A0
  int analogvalue_current_mA = analogRead(A1); // Read from A1
  int analogvalue_current_uA = analogRead(A2); // Read from A2
  //////
  int analogvalue_ckecked=analogRead(A3);//Read from A3

  /////
  if(analogvalue_ckecked>0){
    temp_voltage = (analogvalue_ckecked * 5.0) / 1023.0; // Convert to voltage
    input_voltage = temp_voltage / (r2_voltage / (2*(r3_voltage) + r2_voltage));

    // Display Voltage on LCD
    if(input_voltage<=12){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Voltage: ");
      lcd.print(input_voltage, 2); // Display with 2 decimal places
      lcd.print(" V");

      // Print Voltage to Serial Monitor
      Serial.print("Voltage: ");
      Serial.print(input_voltage, 2);
      Serial.println(" V");

    }
    else if(input_voltage >12){
      lcd.clear();
      lcd.setCursor(0, 0);


      lcd.print("Voltage is greater than our measuring value: ");

      Serial.print("Voltage: ");


    }


    


  }

////////////
   else if (analogvalue_voltage > 0) {
    temp_voltage = (analogvalue_voltage * 5.0) / 1023.0; // Convert to voltage
    input_voltage = temp_voltage / (r2_voltage / (r1_voltage + r2_voltage)); // Voltage divider formula

    // Display Voltage on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Voltage: ");
    lcd.print(input_voltage, 2); // Display with 2 decimal places
    lcd.print(" V");

    // Print Voltage to Serial Monitor
    Serial.print("Voltage: ");
    Serial.print(input_voltage, 2);
    Serial.println(" V");
  } 

  else if (analogvalue_current_mA > 0) {
    // Current Measurement in mA range
    temp_current = (analogvalue_current_mA * 5.0) / 1023.0;  // Convert to voltage
    input_current = temp_current / r1_current;  // Calculate current (I = V/R)

    if (input_current * 1000 < 1.0) {  // Check if the current is too small for mA range
      lcd.clear();
      lcd.print("Switch to uA");
      Serial.println("Switch to uA range");
    } else {
      // Display Current in mA range
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Current: ");
      lcd.print(input_current * 1000);  // Convert to mA
      lcd.print(" mA");

      // Print Current to Serial Monitor
      Serial.print("Current: ");
      Serial.print(input_current * 1000);
      Serial.println(" mA");
    }
  }
  else if (analogvalue_current_uA > 0) {
    // Current Measurement in uA range
    temp_current = (analogvalue_current_uA * 5.0) / 1023.0;  // Convert to voltage
    input_current = temp_current / r2_current;  // Calculate current (I = V/R)

    if (input_current * 1e6 > 1000.0) {  // Check if the current is too large for uA range
      lcd.clear();
      lcd.print("Switch to mA");
      Serial.println("Switch to mA range");
    } else {
      // Display Current in uA range
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Current: ");
      lcd.print(input_current * 1e6);  // Convert to uA
      lcd.print(" uA");

      // Print Current to Serial Monitor
      Serial.print("Current: ");
      Serial.print(input_current * 1e6);
      Serial.println(" uA");
    }
  } 



  else {
    // Handle out-of-range case
    lcd.clear();
    lcd.print("Out of range");
    Serial.println("Out of range");
  }

  delay(2000);  // Wait for 2 seconds before rechecking
}
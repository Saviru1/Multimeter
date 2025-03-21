# Multimeter
 Embedded Multimeter: A Smart Voltage & Current Measurement System

    // Variables for current measurement
    float input_current = 0.0;
    float temp_current = 0.0;
    float r1_current = 33.0; // Resistor value for current measurement mA
    float r2_current = 3300.0; // Resistor value for current measurement uA

    void setup() {
    Serial.begin(9600);      // Open serial port, set data rate to 9600 bps
    Serial.println("MULTIMETER");
    delay(2000);
    }

    void loop() {
    // Read analog values from the respective pins
    int analogvalue_current_mA = analogRead(A1); // Read from A1
    int analogvalue_current_uA = analogRead(A2); // Read from A2

    if (analogvalue_current_mA > 0) {
        // Current Measurement in mA range
        temp_current = (analogvalue_current_mA * 5.0) / 1023.0;  // Convert to voltage
        input_current = temp_current / r1_current;  // Calculate current (I = V/R)

        if (input_current * 1000 < 1.0) {  // Check if the current is too small for mA range
        Serial.println("Switch to uA range");
        } else {
        // Print Current to Serial Monitor
        Serial.print("Current: ");
        Serial.print(input_current * 1000);
        Serial.println(" mA");
        }
    } else if (analogvalue_current_uA > 0) {
        // Current Measurement in uA range
        temp_current = (analogvalue_current_uA * 5.0) / 1023.0;  // Convert to voltage
        input_current = temp_current / r2_current;  // Calculate current (I = V/R)

        if (input_current * 1e6 > 1000.0) {  // Check if the current is too large for uA range
        Serial.println("Switch to mA range");
        } else {
        // Print Current to Serial Monitor
        Serial.print("Current: ");
        Serial.print(input_current * 1e6);
        Serial.println(" uA");
        }
    } else {
        // Handle out-of-range case
        Serial.println("Out of range");
    }

    delay(2000);  // Wait for 2 seconds before rechecking
    }


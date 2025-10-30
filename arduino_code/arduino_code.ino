#include <DHT.h>

// DHT11 sensor
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Relay pins
#define RELAY1_PIN 7  // Fan Speed 1 (Low)
#define RELAY2_PIN 6  // Fan Speed 2 (Medium)
#define RELAY3_PIN 5  // Fan Speed 3 (High)
#define RELAY4_PIN 4  // Temperature control relay

// Temperature settings for auto relay
#define TEMP_ON_THRESHOLD 28.0   // Turn ON when temp exceeds this (°C)
#define TEMP_OFF_THRESHOLD 25.0  // Turn OFF when temp drops below this (°C)

// Current states
int currentSpeed = 0;
bool tempRelayState = false;

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  // Initialize relay pins
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  
  // Turn all relays OFF (HIGH for NO relays)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);
}

void loop() {
  delay(2000);
  
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Check if readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("T:0.0,H:0.0");
    return;
  }
  
  // Send sensor data
  Serial.print("T:");
  Serial.print(temperature, 1);
  Serial.print(",H:");
  Serial.println(humidity, 1);
  
  // Auto temperature control with hysteresis
  if (temperature > TEMP_ON_THRESHOLD && !tempRelayState) {
    digitalWrite(RELAY4_PIN, LOW);  // Turn ON
    tempRelayState = true;
    Serial.println("Temp relay: ON");
  }
  else if (temperature < TEMP_OFF_THRESHOLD && tempRelayState) {
    digitalWrite(RELAY4_PIN, HIGH);  // Turn OFF
    tempRelayState = false;
    Serial.println("Temp relay: OFF");
  }
  
  // Check for serial commands from ESP32 (0-3 for fan speed)
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    Serial.print("DEBUG: Received char: '");
    Serial.print(cmd);
    Serial.print("' (ASCII: ");
    Serial.print((int)cmd);
    Serial.println(")");
    
    if (cmd >= '0' && cmd <= '3') {
      Serial.print("DEBUG: Valid fan command, setting speed to: ");
      Serial.println(cmd - '0');
      setFanSpeed(cmd - '0');
    }
    else if (cmd == '\n' || cmd == '\r') {
      Serial.println("DEBUG: Newline/carriage return received");
    }
    // Manual relay testing
    else if (cmd == 'a') {
      digitalWrite(RELAY1_PIN, !digitalRead(RELAY1_PIN));
      Serial.println("Relay 1 toggled");
    }
    else if (cmd == 'b') {
      digitalWrite(RELAY2_PIN, !digitalRead(RELAY2_PIN));
      Serial.println("Relay 2 toggled");
    }
    else if (cmd == 'c') {
      digitalWrite(RELAY3_PIN, !digitalRead(RELAY3_PIN));
      Serial.println("Relay 3 toggled");
    }
    else if (cmd == 'd') {
      digitalWrite(RELAY4_PIN, !digitalRead(RELAY4_PIN));
      Serial.println("Relay 4 toggled");
    }
    else {
      Serial.print("DEBUG: Unknown command: '");
      Serial.print(cmd);
      Serial.println("'");
    }
  }
}

void setFanSpeed(int speed) {
  // Turn all fan relays OFF
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  
  // Turn on selected speed and relay 4
  switch(speed) {
    case 1:
      digitalWrite(RELAY1_PIN, LOW);
      digitalWrite(RELAY4_PIN, LOW);  // Turn on relay 4
      Serial.println("Fan: LOW");
      break;
    case 2:
      digitalWrite(RELAY2_PIN, LOW);
      digitalWrite(RELAY4_PIN, LOW);  // Turn on relay 4
      Serial.println("Fan: MEDIUM");
      break;
    case 3:
      digitalWrite(RELAY3_PIN, LOW);
      digitalWrite(RELAY4_PIN, LOW);  // Turn on relay 4
      Serial.println("Fan: HIGH");
      break;
    default:
      digitalWrite(RELAY4_PIN, HIGH);  // Turn off relay 4 when fan is off
      Serial.println("Fan: OFF");
      break;
  }
  
  currentSpeed = speed;
  
  // Update tempRelayState to reflect manual control
  tempRelayState = (speed > 0);
}
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_I2CDevice.h>

Adafruit_MPU6050 mpu;

// Relay pins
#define RELAY_RIGHT 3
#define RELAY_LEFT  2
#define RELAY_FRONT 4
#define RELAY_BACK  5

// Offset threshold in degrees
int OFFSET = 20;

// Calibration offsets
float basePitch = 0, baseRoll = 0;

void setup() {
  Serial.begin(115200);
 // while (!Serial) delay(10);

  Serial.println("MPU6050 Gesture Control Start");

  // Configure I2C0 pins (multiplexed)
  //Wire.setSDA(16);  // GP16 = SDA
  //Wire.setSCL(17);  // GP17 = SCL
  Wire.begin();

  if (!mpu.begin(0x68, &Wire)) { // default MPU6050 I2C addr = 0x68
    Serial.println("MPU6050 not found, check wiring!");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Relay setup
  pinMode(RELAY_RIGHT, OUTPUT);
  pinMode(RELAY_LEFT, OUTPUT);
  pinMode(RELAY_FRONT, OUTPUT);
  pinMode(RELAY_BACK, OUTPUT);

  digitalWrite(RELAY_RIGHT, HIGH);
  digitalWrite(RELAY_LEFT, HIGH);
  digitalWrite(RELAY_FRONT, HIGH);
  digitalWrite(RELAY_BACK, HIGH);

  // Initial calibration
  calibrateGyro();
}

void loop() {
  // Serial command for calibration
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("cal")) {
      calibrateGyro();
    }
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Calculate pitch and roll from accel
  float pitch = atan2(a.acceleration.y, sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.z * a.acceleration.z)) * 180 / PI;
  float roll  = atan2(-a.acceleration.x, a.acceleration.z) * 180 / PI;

  // Apply calibration offsets
  pitch -= basePitch;
  roll  -= baseRoll;

  Serial.print("Pitch: "); Serial.print(pitch);
  Serial.print(" | Roll: "); Serial.println(roll);



  // Check gestures with OFFSET
  if (roll > OFFSET) {
    Serial.println("RIGHT");
    digitalWrite(RELAY_RIGHT, LOW);
  } else if (roll < -OFFSET) {
    Serial.println("LEFT");
    digitalWrite(RELAY_LEFT, LOW);
  } else if (pitch > OFFSET) {
    Serial.println("FRONT");
    digitalWrite(RELAY_FRONT, LOW);
  } else if (pitch < -OFFSET) {
    Serial.println("BACK");
    digitalWrite(RELAY_BACK, LOW);
  }else{
  // Reset relays
  digitalWrite(RELAY_RIGHT, HIGH);
  digitalWrite(RELAY_LEFT, HIGH);
  digitalWrite(RELAY_FRONT, HIGH);
  digitalWrite(RELAY_BACK, HIGH);
  }

  delay(100);
}

// Calibration function
void calibrateGyro() {
  Serial.println("Calibrating... keep MPU6050 still");
  delay(2000);

  float pitchSum = 0, rollSum = 0;
  for (int i = 0; i < 200; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    pitchSum += atan2(a.acceleration.y, sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.z * a.acceleration.z)) * 180 / PI;
    rollSum  += atan2(-a.acceleration.x, a.acceleration.z) * 180 / PI;
    delay(5);
  }
  basePitch = pitchSum / 200.0;
  baseRoll  = rollSum / 200.0;

  Serial.println("Calibration done!");
}

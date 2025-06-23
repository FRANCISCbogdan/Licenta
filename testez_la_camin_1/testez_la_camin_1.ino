#define PWMA 3
#define AIN2 8
#define AIN1 9
#define STBY 10
#define BIN1 11
#define BIN2 12
#define PWMB 5
const int SENSOR_PINS[] = {A0, A1, A2, A3, A4, A5, A6, A7};
#define NUM_SENSORS 8
#define IR_ENABLE_PIN 7

unsigned int threshold = 750;
float Kp = 0.07;
int base_speed = 130;
int max_speed = 220;

unsigned int sensor_values[NUM_SENSORS];
int last_error = 0;

void setup() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  pinMode(IR_ENABLE_PIN, OUTPUT);
  digitalWrite(IR_ENABLE_PIN, HIGH);
  delay(1000);
}

void loop() {
  read_sensors();

  if (has_found_special_finish()) {
    stop_motors();
    while (true); 
  }

  long position = calculate_position();
  int error;

  if (position == -1) { 
    error = last_error;
  } else {
    error = position - 3500;
    last_error = error;
  }

  int motor_adjustment = Kp * error;
  int left_speed = base_speed - motor_adjustment;
  int right_speed = base_speed + motor_adjustment;

  left_speed = constrain(left_speed, -max_speed, max_speed);
  right_speed = constrain(right_speed, -max_speed, max_speed);

  set_motors(left_speed, right_speed);
}


void read_sensors() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensor_values[i] = analogRead(SENSOR_PINS[i]);
  }
}

bool has_found_special_finish() {
  bool outer_sensors_black = (sensor_values[0] > threshold) &&
                             (sensor_values[1] > threshold) &&
                             (sensor_values[6] > threshold) &&
                             (sensor_values[7] > threshold);

  bool inner_sensors_white = (sensor_values[3] < threshold) &&
                             (sensor_values[4] < threshold);

  return outer_sensors_black && inner_sensors_white;
}

long calculate_position() { 
  long sum = 0;
  long weighted_sum = 0;
  int sensors_on_line = 0;

  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensor_values[i] > threshold) {
      weighted_sum += (long)sensor_values[i] * (i * 1000);
      sum += sensor_values[i];
      sensors_on_line++;
    }
  }

  if (sensors_on_line == 0) {
    return -1;
  }

  return weighted_sum / sum;
}

void set_motors(int left_speed, int right_speed) {
  if (left_speed >= 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  }
  analogWrite(PWMA, abs(left_speed));

  if (right_speed >= 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  }
  analogWrite(PWMB, abs(right_speed));
}

void stop_motors() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, 0);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, 0);
}



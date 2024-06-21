// Simple sketch to read the analog flow meter values to serial monitor

const int flow_meter_Pin = 2; // Change this to whatever GPIO the flow meter is connected to
int value = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  value = analogRead(flow_meter_Pin);
  Serial.println(value);
  delay(500);
}

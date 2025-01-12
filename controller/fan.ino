
/*
$ python
> i = list(range(1,11))
> xs = [1.5**i for i in i]
> [int(y0 + (x - xs[0]) * (y1 - y0) / (xs[-1] - xs[0])) for x in xs]
*/
const int SPEEDS[] = {0, 30, 33, 37, 44, 54, 69, 92, 126, 177, 255};

void setupFanController()
{
  pinMode(CHIPSELECT, 10);
  digitalWrite(CHIPSELECT, HIGH);
  SPI.begin();
}


void setFanSpeed(bool intake, bool exhaust, int val) {
  val = constrain(val, 0, 10);
  Serial.printf("Fan Speed: %d (%d)", val, SPEEDS[val]);
  Serial.println();

  int command = 0b00010000;
  if (intake) {
    Serial.println("Setting intake fan speed");
    command = command | 0b00000001;
  }
  if (exhaust) {
    Serial.println("Setting exhaust fan speed");
    command = command | 0b00000010;
  }

  digitalWrite(CHIPSELECT, LOW);
  delay(10);
  SPI.transfer(command);
  SPI.transfer(SPEEDS[val]);
  delay(10);
  digitalWrite(CHIPSELECT, HIGH);
}
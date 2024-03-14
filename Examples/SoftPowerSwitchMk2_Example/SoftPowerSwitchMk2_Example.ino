/*
  Control the power to a system using the SparkFun Soft Power Switch Mk2
  By: Nathan Seidle
  Adapted by: Paul Clark
  SparkFun Electronics
  Date: March 14th, 2024
  License: MIT. See license file for more information

  This example assumes the OFF pin is connected.

  A simple tap of the power button will turn on the system. If the power button is not being pressed 
  (accidental tap) the system will turn off after ~20ms. If the system turns on and sees the power 
  button held for 0.5s, it will begin normal operation.

  During normal system operation, if user presses the power button for 2s, the system
  will shut down. This means the power button can also be used as a general 'select' button as
  long as user doesn't hold button for more than 2s.

  This example will output serial states. If you are powering your board over a USB connection then
  the power button can't do its job. We did our testing by connecting the TX and GND pins of a RedBoard to 
  an external USB to Serial adapter. That way the battery + Soft Power Switch could control power
  to the board.  
*/

//Hardware connections - GPIO pins (change these as required)
int _PUSH = 14; // Connect this GPIO to the PUSH breakout pad
int _OFF = 32; // Connect this GPIO to the OFF breakout pad

int STAT_LED = LED_BUILTIN;

unsigned long powerPressedStartTime = 0;

int debounceDelay = 20;

//Uncomment the following line to turn on shutdown time measurements
//#define PRINT_TIMER_OUTPUT

void setup()
{
  Serial.begin(115200);
  Serial.println("Soft Power Switch Mk2 example");

  pinMode(_PUSH, INPUT_PULLUP);

  //User has pressed the power button to turn on the system
  //Was it an accidental bump or do they really want to turn on?
  //Let's make sure they continue to press for two seconds
  Serial.print("Initial power on check");
  powerPressedStartTime = millis();
  while (digitalRead(_PUSH) == LOW)
  {
    //Wait for user to stop pressing button.
    //What if user has left something heavy pressing the power button?
    //The soft power switch will automatically turn off the system! Handy.
    delay(100);

    if ((millis() - powerPressedStartTime) > 500)
      break;
    Serial.print(".");
  }
  Serial.println();

  if ((millis() - powerPressedStartTime) < 500)
  {
    Serial.println("Power button tap. Returning to off state. Powering down");
    powerDown();
  }

  Serial.println("User wants to turn system on!");
  powerPressedStartTime = 0; //Reset var to return to normal 'on' state

  //Here we display something to user indicating system is on and running
  //For example an external display or LED turns on
  pinMode(STAT_LED, OUTPUT);
  digitalWrite(STAT_LED, HIGH);

  Serial.println("Press 'r' to enter infinite loop to test power-down override");
  Serial.println("Press 'z' to do a powerdown");
  Serial.println("Press and hold power button for 2s to do a powerdown");

  while (Serial.available()) // Flush the Serial RX buffer
    Serial.read();
}

void loop()
{
  if (Serial.available())
  {
    char incoming = Serial.read();

    if (incoming == 'z')
    {
      Serial.println("Power down");
      powerDown();
    }
    else if (incoming == 'r')
    {
      Serial.println("System locked. Now hold power button to force power down without using software");

      //Here we wait for user press button so we can time it
      while (digitalRead(_PUSH) == HIGH)
        delay(1);

      powerPressedStartTime = millis();
      Serial.println("Doing nothing, waiting for power override to kick in");
      while (1)
      {
#ifdef PRINT_TIMER_OUTPUT
        Serial.println(millis() - powerPressedStartTime);
#endif
        delay(1);

        if(digitalRead(_PUSH) == HIGH) break;
      }
      Serial.println("User released button before forced powered could complete. Try again, but hold power button for 7s");
      powerPressedStartTime = 0; //Reset var to return to normal 'on' state
    }
  }

  if (digitalRead(_PUSH) == LOW && powerPressedStartTime == 0)
  {
    //Debounce check
    delay(debounceDelay);
    if (digitalRead(_PUSH) == LOW)
    {
      Serial.println("User is pressing power button. Start timer.");
      powerPressedStartTime = millis();
    }
  }
  else if (digitalRead(_PUSH) == LOW && powerPressedStartTime > 0)
  {
    //Debounce check
    delay(debounceDelay);
    if (digitalRead(_PUSH) == LOW)
    {
      if ((millis() - powerPressedStartTime) > 2000) // Check if button has been held for >= 2 seconds
      {
        Serial.println("Time to power down!");
        powerDown();
      }
    }
  }
  else if (digitalRead(_PUSH) == HIGH && powerPressedStartTime > 0)
  {
    //Debounce check
    delay(debounceDelay);
    if (digitalRead(_PUSH) == HIGH)
    {
      Serial.print("Power button released after ms: ");
      Serial.println(millis() - powerPressedStartTime);
    }
    powerPressedStartTime = 0; //Reset var to return to normal 'on' state
  }
}

//Immediately power down
void powerDown()
{
  //Indicate to user we are shutting down
  digitalWrite(STAT_LED, LOW);

  Serial.println("Pulling OFF high");
  Serial.flush();

  powerPressedStartTime = millis();

  pinMode(_OFF, OUTPUT);
  digitalWrite(_OFF, HIGH); // Pull OFF high to turn the power off

  while (1)
  {
#ifdef PRINT_TIMER_OUTPUT
    Serial.println(millis() - powerPressedStartTime);
#endif
    delay(1);
  }
}

//Neopixel Driver and buttons

void initLEDs(){
  leds.begin();
  leds.setBrightness(128);
  setLEDs(whiteLED);
  leds.show();
}

void setUserLED(uint8_t red, uint8_t green, uint8_t blue){
  leds.setPixelColor(0, red, green, blue);
  leds.show();
}
void setLEDs(const uint8_t ledcol[]){
  leds.setPixelColor(0, ledcol[LRED], ledcol[LGREEN], ledcol[LBLUE]);
  leds.show();
}
  

void refreshUserControls(){
  
  userButtonDB.update();
  readUserSwitch();
  readUserButton();
}

void readUserSwitch(){
  device.oldUserSwitchState = device.userSwitchState;
  device.userSwitchState = !digitalRead(PIN_USER_SWITCH);
  if(device.oldUserSwitchState != device.userSwitchState) device.userSwitchChanged = true;
}

void readUserButton(){
  device.oldUserButtonState = device.userButtonState;
  device.userButtonState = !userButtonDB.read();
  if(device.oldUserButtonState != device.userButtonState) device.userButtonChanged = true;
  //device.userButtonState = !digitalRead(PIN_USER_BUTTON);
}

void cycleLED(uint8_t ledcol[]){
  for(int n = 0; n < 3; n++){
     ledcol[n] += 32;
  }
}
void randomiseLED(uint8_t ledcol[]){
  for(int n = 0; n < 3; n++){
     ledcol[n] = randomiseCol(ledcol[n]);
  }
}
uint8_t randomiseCol(uint8_t color){
  int16_t col;
  col = (int16_t)color + random(-16,16);
  if(col < 0) return (uint8_t)(255+col);
  else if(col > 255) return (uint8_t)( 0 + (col-255) );
  else return (uint8_t)col;
}

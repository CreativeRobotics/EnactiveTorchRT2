void initClock(){
  Clock.begin();
  // initialize RTC 24H format
  //hours, minutes, seconds
  //Clock.setTime(12, 40, 0);
  //day month year
  //Clock.setDate(04, 1, 19);
  setupClock();
}

void updateClock(){
  //Copy data to the DCU structure
  device.year = Clock.getYear();
  device.month = Clock.getMonth();
  device.day = Clock.getDay();
  device.hours = Clock.getHours();
  device.minutes = Clock.getMinutes();
  device.seconds = Clock.getSeconds();
}

void setupClock(){
  clockAlarm_isr(); //just call the alarm isr to set it up
  //Clock.setAlarmSeconds(1);
  Clock.enableAlarm(Clock.MATCH_SS);
  Clock.attachInterrupt(clockAlarm_isr);
}


void clockAlarm_isr(){
  //this should interrupt every second
  //set the next interrupt time
  if(Clock.getSeconds() < 59) Clock.setAlarmSeconds(Clock.getSeconds()+1);
  else Clock.setAlarmSeconds(0);
  device.clockLastMillis = millis();
}

unsigned long clockMillis(){
  return millis() - device.clockLastMillis;
}

String getFilenameTime(){
  //returns the date and time down to the nearest second
  String dateAndTime = String(Clock.getHours());
  dateAndTime += String(Clock.getMinutes());
  dateAndTime += String(Clock.getSeconds());
  return dateAndTime;
}


String getTimestamp(){
  //returns the exact time inclucing millis
  //Does not include date - this is in the filename
  String timeStamp = String(Clock.getHours());
  timeStamp += ':';
  timeStamp += String(Clock.getMinutes());
  timeStamp += ':';
  timeStamp += String(Clock.getSeconds());
  timeStamp += ':';
  timeStamp += String(clockMillis());
  return timeStamp;
}

String getTimeAndDate(){
  //returns the exact time inclucing millis
  //Does not include date - this is in the filename
  String timeStamp = "Date: ";
  timeStamp += String(Clock.getDay());
  timeStamp += ':';
  timeStamp += String(Clock.getMonth());
  timeStamp += ":20";
  timeStamp += String(Clock.getYear());
  timeStamp += " Time: ";
  timeStamp += String(Clock.getHours());
  timeStamp += ':';
  timeStamp += String(Clock.getMinutes());
  timeStamp += ':';
  timeStamp += String(Clock.getSeconds());
  timeStamp += ':';
  timeStamp += String(clockMillis());
  return timeStamp;
}

String getDate(){
  //returns the exact time inclucing millis
  //Does not include date - this is in the filename
  String timeStamp = String(Clock.getDay());
  timeStamp += ':';
  timeStamp += String(Clock.getMonth());
  timeStamp += ":20";
  timeStamp += String(Clock.getYear());
  return timeStamp;
}

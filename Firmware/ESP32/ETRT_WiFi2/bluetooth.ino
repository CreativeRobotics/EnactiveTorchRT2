void startBlueTooth(){
  //char bName[device.btName.length()+1];
  //device.btName.toCharArray(bName, device.btName.length()+1);
  SerialBlueTooth.begin(device.btName);
  device.btEnabled = true;
}
void stopBlueTooth(){
  //char bName[device.btName.length()+1];
  //device.btName.toCharArray(bName, device.btName.length()+1);
  SerialBlueTooth.end();
  device.btEnabled = false;
}

void serviceBT(){
  if(!device.btEnabled) {
    device.btEnabled = false;
    return;
  }
  if(SerialBlueTooth.hasClient() && device.btConnected == false){
    device.btConnected = true;
    Serial.println("ESP Status: BT Connected");
  }
  if(!SerialBlueTooth.hasClient() && device.btConnected == true){
    device.btConnected = false;
    Serial.println("ESP Status: BT Disconnected");
    return;
  }
  //if (Serial.available()) {
    //SerialBlueTooth.write(Serial.read());
  //}
  if (SerialBlueTooth.available()) {
    Serial.write(SerialBlueTooth.read());
  }
}
/*
void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("ESP Status: BT Connected");
    device.btConnected = true;
  }if(event == ESP_SPP_SRV_CLOSE_EVT){
    Serial.println("ESP Status: BT Disconnected");
    device.btConnected = false;
  }
}*/

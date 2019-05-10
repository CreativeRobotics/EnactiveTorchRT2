
void startUDP(){
  //Nothing needed here
  device.udpEnabled = true;
}
void stopUDP(){
  //Nothing needed here
  device.udpEnabled = false;
}
void sendUDP(String udpPacket){
  //Don't send unless connected
  if(!device.wifiConnected) return;
  char udpbuf[udpPacket.length()+1];
  char udpAddr[device.UDPAddress.length()+1];// = "192.168.0.255";
  device.UDPAddress.toCharArray(udpAddr, device.UDPAddress.length()+1);
  
  udpPacket.toCharArray(udpbuf, udpPacket.length()+1);
  
  if(connected){
    //Send a packet
    udp.beginPacket(udpAddr, device.udpPort);
    udp.printf(udpbuf);

    udp.endPacket();
  }
  if(DEBUGGING) debugPrintln(udpPacket);
}


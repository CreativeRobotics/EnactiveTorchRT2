void debugPrint(String message){
  String msg = String("DEBUG:" + message);
  Serial.print(msg);
}
void debugPrintln(String message){
  String msg = String("DEBUG:" + message);
  Serial.println(msg);
}
void debugWrite(char ch){
  Serial.write(ch);
}
void debugStartLine(){
  Serial.print("DEBUG:");
}
void debugEndLine(){
  Serial.println("");
}


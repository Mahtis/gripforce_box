const int button1 = 10;
const int button2 = 12;
const int button3 = 13;

void setup() {
  Serial.begin(9600);
  
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
}

void loop() {
  String msg = ((String)"Button 1: " + digitalRead(button1) + ", Button 2: " + digitalRead(button2) + ", Button 3: " + digitalRead(button3));
  Serial.println(msg);
}

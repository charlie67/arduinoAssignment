//#define DEBUG
//#define DEBUG_LED
//#define DEBUG_CAM_CONV
//#define DEBUG_INPUT
//#define DEBUG_PRINT
//#define DEBUG_MESSAGE_CHECK
#define RED_LED 5 //receive ascii, transmit CAM
#define BLUE_LED 11 //receive CAM, transmit ascii
#define AMBER_LED 6
#define YELLOW_LED 9
#define GREEN_LED 10
#define IR_IN 2
#define IR_LED A1
#define POTENT A0
/*camCodes_pe is all cam codes starting with % sign
   camCodes_eq is cam starting with =
   etc.
   eq =
   pc %
   at @
*/
String camCodes_eq[] = {"=", "==", "=%%", "=%", "=@", "==@", "==%", "=%@", "=@%", "===", " "}; //11
char letterCodes_eq[] = {'A', 'G', 'J', 'M', 'N', 'Q', 'V', 'W', 'X', 'Z', ' '};
String camCodes_at[] = {"@=", "@%", "@", "@@", "@%%=", "@%%@", "@%=%", "@%==", "@%=@", "@%@%", "@%@=", "@%@@", "@=%%", "@%%%", "@%%", "@%=", "@=%", "@==", "@=@", "@@%", "@@=", "@@@", "@%@", "@===@", " "}; //24
char letterCodes_at[] = {'O', 'P', 'T', 'U', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '.', ',', 39, '?', '!', '+', '-', '*', '/', '=', ' '};//39 is the ascii value of  a '
String camCodes_pc[] = {"%%=", "%=@", "%=", "%", "%=%", "%==", "%%", "%@", "%%@", "%@=", "%@%", "%@@", " "}; //13
char letterCodes_pc[] = {'B', 'C', 'D', 'E', 'F', 'H', 'I', 'K', 'L', 'R', 'S', 'Y', ' '};
char error[] = "Not a supported character.";
String ip = "";
String camIp = "";
String textIp = "";
String textInput = "";
int startChar;
int lastChar;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(AMBER_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(IR_LED, OUTPUT);
  pinMode(POTENT, INPUT);
  pinMode(IR_IN, INPUT);
}

void loop() {
  input();
  toPrint();
  ledDecision();
  messageResponse();
  clearStrings();//was having some weird issues so decided that clearing the strings might fix my problems, it did but https://xkcd.com/552/
}

bool checkLpNumbers(String input) {
  /* wanted to remove this from the main checkMessage() function just to make it a bit neater
      input will be of the form LPxxxxxxxxxxxx
      want to find if all x are numbers
      x range from input[2] to input[13]
  */
  String testNumbers;
  int lpNumber;
  for (int i = 2; i <= 13; i++) {
    testNumbers.concat(input[i]);
  }
  lpNumber = testNumbers.toInt();
  int numberCounter = 0;
  for (int i = 0; i < testNumbers.length(); i++) {
    if (isDigit(testNumbers[i])) {
      numberCounter++;//should result in 12 if LP if folowed by 12 digits
    }
  }
  if (numberCounter == 12) {
    return true;
  } else {
    return false;
  }
}

void checkMessage(String message) {
#ifdef DEBUG_MESSAGE_CHECK
  Serial.println(message.length());
#endif
  if (message[0] == 'L' && message[1] == 'P' && message.length() == 14 &&  checkLpNumbers(message) == true) {
#ifdef DEBUG_MESSAGE_CHECK
    Serial.println("LP 12 digit message detected");
#endif
    String redPwmStr;
    String amberPwmStr;
    String yellowPwmStr;
    String greenPwmStr;
    int redPwm;
    int amberPwm;
    int yellowPwm;
    int greenPwm;
    redPwmStr += message[2];//needs to be a string so you can concatonate it
    redPwmStr += message[3];
    redPwmStr += message[4];
    redPwm = redPwmStr.toInt();//then make it an int so it can be passed to sendAnalog()
    amberPwmStr += message[5];
    amberPwmStr += message[6];
    amberPwmStr += message[7];
    amberPwm = amberPwmStr.toInt();
    yellowPwmStr += message[8];
    yellowPwmStr += message[9];
    yellowPwmStr += message[10];
    yellowPwm = yellowPwmStr.toInt();
    greenPwmStr += message[11];
    greenPwmStr += message[12];
    greenPwmStr += message[13];
    greenPwm = greenPwmStr.toInt();
#ifdef DEBUG_MESSAGE_CHECK
    Serial.println(greenPwm);
    Serial.println(amberPwm);
    Serial.println(redPwm);
    Serial.println(yellowPwm);
    analogWrite(RED_LED, redPwm);
    analogWrite(AMBER_LED, amberPwm);
    analogWrite(YELLOW_LED, yellowPwm);
    analogWrite(GREEN_LED, greenPwm);
    delay(2000);
    analogWrite(RED_LED, 0);
    analogWrite(AMBER_LED, 0);
    analogWrite(YELLOW_LED, 0);
    analogWrite(GREEN_LED, 0);
#endif
  } else if (message == "IR*") {
#ifdef DEBUG_MESSAGE_CHECK
    Serial.println("IR* called");
#endif
    tone(IR_LED, 38000);
    delay(2000);
    noTone(IR_LED);
  } else if (message == "RXIR") {
    bool irState = digitalRead(IR_LED);
    if (irState == 0) {
      Serial.println("LOW");
    } else {
      Serial.println("HIGH");
    }
  } else if (message == "PV") {
    String potentReading;
    String potentReadingCam;
    potentReading = String(analogRead(POTENT));
#ifdef DEBUG_MESSAGE_CHECK
    Serial.println(potentReading);
#endif
    /*makes the potentreading 4 digits if it's any less
      because the potent reading needs to be transmitted as 4 CAM encoded digits
    */
    String dig4Potent = "0"; // wanted 4digPotent but couldn't create a variable with a number at the beggining
    int strLength = potentReading.length();
    if (strLength != 4) {
      if (strLength == 3) {
        dig4Potent.concat(potentReading);//now 000x
      } else if (strLength == 2) {
        dig4Potent.concat("0");//now 00
        dig4Potent.concat(potentReading);//now 00xx
      } else if (strLength == 1) {
        dig4Potent.concat("0"); //needs 3 added digits so becomes 00
        dig4Potent.concat("0"); //now 000
        dig4Potent.concat(potentReading);//now 000x
      }
    } else {
      //the potent reading must already be 4 digits so no new numbers need to be added
      dig4Potent = potentReading;
    }
    potentReadingCam = string2cam(dig4Potent);
    Serial.println(potentReadingCam);
  }
}

void messageResponse() {
  if (textInput == "cam") { // the message has to of been received as cam for an action to be performed
    checkMessage(textIp);
  }
}

void clearStrings() {
  //was getting some weird issues before I started doing this.
  ip = "";
  camIp = "";
  textIp = "";
  textInput = "";
  startChar = 0;
}

void toPrint() {
  if (textInput == "cam") {
    Serial.println(textIp);
  } else if (textInput == "ascii") {
    Serial.println(camIp);
  } else {
#ifdef DEBUG_PRINT
    Serial.print(toPrint);
    Serial.print(" toPrint is not cam or ascii");
    Serial.print('\n');
#endif
  }
}

void sendLed(int pin, String message) {
#ifdef DEBUG_LED
  Serial.println("message is " + message);
#endif
  float timeLength; //length of time of each character in ms
  int potentReading = analogRead(POTENT);
  /* the potent returns a value of 0 to 1023
    20ms timing needs to be at 0
    so this means that there are 480 other ms values to account for
    500/1023 is 0.488758553 which measn an increase of 1 by the potent is a 0.488758553 speed increase
    20/0.488758553 = 40.92 however the potent can only return whole numbers, so if potentReading <=41
  */
  if (potentReading <= 41) {
    timeLength = 20;
#ifdef DEBUG_LED
    Serial.println("potent Reading is <= 41");
    Serial.print("Potent Reading ");
    Serial.print(timeLength);
    Serial.print('\n');
#endif
  } else {
    timeLength = potentReading * 0.488758553;
#ifdef DEBUG_LED
    Serial.print("Potent Reading ");
    Serial.print(potentReading);
    Serial.print('\n');
    Serial.print("timeLength is: ");
    Serial.print(timeLength);
    Serial.print('\n');
#endif
  }//delays
  round(timeLength);//timeLength is a float and needs to go to an int so round it
  int percent = timeLength;
  int equal = 2 * timeLength;
  int at = 4 * timeLength;
  int pipe = 2 * timeLength; //3-1 for delay after character
  int wordDelay = 5 * timeLength; //6-1 "
#ifdef DEBUG_LED
  Serial.print("percent is: ");
  Serial.print(percent);
  Serial.print('\n');
  int time1 = millis();
#endif

  for (int i = 0; i < message.length(); i++) {
    if (message[i] == '%') {
      digitalWrite(pin, HIGH);
      delay(percent);
      digitalWrite(pin, LOW);
      delay(percent);
    } else if (message[i] == '=') {
      digitalWrite(pin, HIGH);
      delay(equal);
      digitalWrite(pin, LOW);
      delay(percent);
    } else if (message[i] == '@') {
      digitalWrite(pin, HIGH);
      delay(at);
      digitalWrite(pin, LOW);
      delay(percent);
    } else if (message[i] == '|') {
      delay(pipe);
    } else if (message[i] == ' ') {
      delay(wordDelay);
    }
  }
  #ifdef DEBUG_LED
  int time2 = millis();
  int diff = time2 - time1;
  Serial.println(diff);
  #endif
}

void ledDecision() { // decides whcih LED should flash
  /*if ip is cam then flash cam on BLUE_LED
     if ip is ascii flash cam on RED_LED
  */
  if (textInput == "cam") { //ip is a cam string
    sendLed(BLUE_LED, camIp);
  }
  else {// ip is ascii
    sendLed(RED_LED, camIp);
  }
}

void input() {
  ip = promptReadIn();
  ip.toUpperCase(); //cam is all defined as upper case characters
  if (ip[0] == ' ') {//there is at least one space at the start of the ip string need to find the first 'real' character
    #ifdef DEBUG_INPUT
    Serial.println("first char is a space");
    #endif
    int i = 0;
    while (ip[i] == ' ') {
      //count how many space characters there are at the beggining of the input string
      i++;
    }
    startChar = i;//first character that isnt a space
#ifdef DEBUG_INPUT
    int numberOfRealChar = ip.length() - startChar; //'real' means non space characters
    Serial.println(numberOfRealChar);
#endif
    if (ip[startChar] == '%' || ip[startChar] == '@' || ip[startChar] == '=') {
      //ip is a cam string
      textInput = "cam";
      camIp = ip;
      textIp = camString2string(ip);
    } else {
      textInput = "ascii";
      textIp = ip;
      camIp = string2cam(ip);
    }
    //ip[0] is not a space so can check for CAM code
  } else if (ip[0] == '%' || ip[0] == '@' || ip[0] == '=') {
    textInput = "cam";
    camIp = ip;
    textIp = camString2string(ip);
  } else {
    //so this has to be ascii
    textInput = "ascii";
    textIp = ip;
    camIp = string2cam(ip);
  }
}

String promptReadIn() {
  String iValue = "";
#ifdef DEBUG_INPUT
  Serial.println("Enter a message.");
#endif
  while (!Serial.available()) {
  }
  iValue = Serial.readString();
  return iValue;
}

void testForLastSpace() {
  //a test designed to find out how many spaces there are at the end of a cam string
  if (textInput == "cam") {
    /*
       need to check to see if there are any spaces at the end of the ip string
    */
#ifdef DEBUG_CAM_CONV
    Serial.println(ip);
#endif
    int ipLen = ip.length() - 1;//need to -1 for the array
    int i = ipLen;//i will become the end value of the String
    if (ip[i] == ' ') {
      while (ip[i] == ' ') {
#ifdef DEBUG_CAM_CONV
        Serial.println(i);
#endif
        i--;
      }
      lastChar = i;
      int spaceNumber = ipLen - i;
#ifdef DEBUG_CAM_CONV
      Serial.println(spaceNumber);
#endif
    } else {
      //no spaces at end of string
      lastChar = ip.length();
    }
  } else {
    lastChar = ip.length();
  }
}

String string2cam(String s) {// takes a ascii text string and converts it into a cam string
  String camString = "";
  int stringLength = s.length();
  for (int i = 0; i < stringLength; i++) {
    camString += char2cam(s.charAt(i));
    if (s.charAt(i) == ' ' || i == (stringLength - 1) || s.charAt(i + 1) == ' ') {

    }
    else {
      camString += '|';
    }
  }
  return camString;
}

String camString2string(String m) {//takes a cam string returns text string
  testForLastSpace();
#ifdef DEBUG_CAM_CONV
  Serial.print("startChar is: ");
  Serial.print(startChar);
  Serial.print('\n');
  Serial.print("lastChar is: ");
  Serial.print(lastChar);
  Serial.print('\n');
  Serial.print(m);
  Serial.print(" has been sent to camString2string");
  Serial.print('\n');
#endif
  String temp = "";
  String returnString = "";
  String spaceAtEnd = "";
  int stringLength = m.length();
  for (int i = 0; i < stringLength; i++) {
    /*
       need to iterate through the string checking the character
       if its anytrhing but a CAM character then the string can be sent to be translated
    */
    char c = m.charAt(i);
    if (c == ' ' && i < startChar) {//if there is a space at the beggining
#ifdef DEBUG_CAM_CONV
      Serial.println("space at start");
#endif
      returnString += ' ';
    } else if (c == '%' || c == '@' || c == '=') {
      temp += c;
#ifdef DEBUG_CAM_CONV
      Serial.print(temp);
      Serial.print(" is temp String");
      Serial.print('\n');
#endif
    } else if (c == '|') { //next letter translate the current letter
      returnString += cam2char(temp);
      temp = "";
    } else if (c == ' ' && i >= startChar && i < lastChar) {
      //this is a space seperating 2 words
#ifdef DEBUG_CAM_CONV
      Serial.println("space in middle");
#endif
      returnString += cam2char(temp);
      returnString += ' ';
      temp = "";
    } else if ( c == ' ' && i > lastChar) {
#ifdef DEBUG_CAM_CONV
      Serial.print("space at end: ");
      Serial.print(spaceAtEnd.length());
      Serial.print('\n');
#endif
      spaceAtEnd += ' ';
    }
  }
  if (temp.length() != 0) {
    returnString += cam2char(temp);
  }
  returnString.concat(spaceAtEnd);
  return returnString;
}

// ---CONVERSION CODE BELOW---
/*
   each array is ordered alphabetically and based on their start character
   the for loop will iterate through the array and find the matching character
   there is then an array for the corrosponding ascii character and this is returned
*/
char cam2char(String cam) {//takes the cam string and will output the corrosponding CAM char
#ifdef DEBUG_CAM_CONV
  Serial.print("cam2char(String cam) cam has been passed as: ");
  Serial.print(cam);
  Serial.print('\n');
#endif
  if (cam[0] == '=') {
    /*
       need to find if there are any ending spaces and then can work on the String to translate it
    */
#ifdef DEBUG_CAM_CONV
    Serial.println("= at start matched");
#endif
    for (int i = 0; i < 12; i++) {
      if (cam.equals(camCodes_eq[i])) { //will search throught he array trying to match the entire cam string
        return letterCodes_eq[i];//and then when found print the corrosponding letter it matches
        break;
      }
    }
  } else if (cam[0] == '@') {
#ifdef DEBUG_CAM_CONV
    Serial.println("@ at start matched");
#endif
    for (int i = 0; i < 26; i++) {
      if (cam.equals(camCodes_at[i])) {
        return letterCodes_at[i];
        break;
      }
    }
  } else if (cam[0] == '%') {
#ifdef DEBUG_CAM_CONV
    Serial.println("% at start matched");
#endif
    for (int i = 0; i < 14; i++) {
      if (cam.equals(camCodes_pc[i])) {
        return letterCodes_pc[i];
        break;
      }
    }
  } 
  else if (cam == " ") {
    #ifdef DEBUG_CAM_CONV
    Serial.println("cam is a space");
#endif
    return ' ';
  }
}

/*
    can iterate through the array containing ascii characters until
   the character needed is found then print that character out assigned each letter
*/
String char2cam(char text) {//will take in an ascii character and output the cam representation
  for (int i = 0; i < 49; i++) {
    if (text == letterCodes_eq[i]) {
      return camCodes_eq[i];
      break;
    } else  if (text == letterCodes_at[i]) {
      return camCodes_at[i];
      break;
    } else if (text == letterCodes_pc[i]) {
      return camCodes_pc[i];
      break;
    }
  }
}

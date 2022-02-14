/*
 *  This sketch sends a message to a TCP server
 *
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>

WiFiMulti WiFiMulti;
//********************Variabels**********************

const int inputHome=26;
const int inputchangeDriftMode=25;
const int analogInPin_1 = 36; //
const int analogInPin_2 = 39;


//*********MQTT***************************************
const char* mqtt_server = "raspFast";
//*********Variabels**********************************
long lastMsg = 0; //?
char msg[50]; //Mitt Meddelande
int value = 0; //För att räkna upp hur många meddelande den har skickat
int buttonState=0;

//***********Commands declarations********************
char crMoveRHcmd[80];
char crMoveRLcmd[80];
char crMoveLHcmd[80];
char crMoveLLcmd[80];
char crMoveStpCmd[80];
char crMoveHomeCmd[80];
const String cmdMvStop="B_MoveStop";
const String cmdMvR="Rmove";
const String cmdMvL="Lmove";
const String cmdHome="Home";
const String axis="axis";

//*************Positions******************************
const int JoyHighVal=3600;//1024-900=124=12%
const int JoyMidHVal=1960;//600 =60%
const int JoyMidLVal=1200;//200=20%

const int JoyHighValy=3600;//1024-900=124=12%
const int JoyMidHValy=1860;//600 =60%
const int JoyMidLValy=1000;//200=20%

//***********Send in progress*************************
boolean sendInPrgssR5=false;
boolean sendInPrgssR2=false;
boolean sendInPrgssL5=false;
boolean sendInPrgssL2=false;

boolean sendInPrgssU5=false;
boolean sendInPrgssU2=false;
boolean sendInPrgssD5=false;
boolean sendInPrgssD2=false;

boolean a_moveStop=false;
boolean b_moveStop=false;


//**********JoystickSpeeds**************************** 
String lowSpeed="4";
String highSpeed="8";
String stopSpeed="0";

//************Button**********************************

int buttonState_home=0;
int buttonState_drift=0;
boolean Auto=true;
//****************************************************
WiFiClient espClient; 
PubSubClient client(espClient); //MqqtClient ,->  WifiClient

void setup()
{
    Serial.begin(115200);
    delay(10);
    setup_wifi();

    //Lägg till server adress och port
    //Mqtt_server = raspFast.lan

    client.setServer(mqtt_server, 1883);
    //client.setCallback(callback);
    pinMode(inputHome, INPUT); //26
    pinMode(inputchangeDriftMode, INPUT);//25

}
//****************Funktioner************************

//Make All axis commands
String makeCommand(String cmdCmd,String spd){
  char tempCharArray[80];
  String command;
  snprintf(tempCharArray,sizeof(tempCharArray),"{\"action\":\"joystick\",\"cmd\":\"%s\", \"speed\":\"%s\"}",cmdCmd.c_str(),spd);
  command=tempCharArray;

  return command;
}
//Make Home commands
String makeHomeCommand(String cmdCmd,int axis){
  char tempCharArray[80];
  String command;
  snprintf(tempCharArray,sizeof(tempCharArray),"{\"action\":\"joystick\",\"cmd\":\"%s\", \"axis\":%d}",cmdCmd.c_str(),axis);
  command=tempCharArray;

  return command;
}
//Funktion för att starta och visa WifiAnslutning
void setup_wifi() {

  // We start by connecting to a WiFi network
    WiFiMulti.addAP("No_More_Lag", "");

    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);
}

//Funktion för att ta hand om svar MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

//***************Evaluate JoyStick***********
int eval_joystick(int joyVal,int iJoyHighVal,int iJoyMidHVal,int iJoyMidLVal,String cmdStop,String cmdRgtUp,String cmdLoDwn,boolean& sndPrgR2,boolean& sndPrgL2,boolean& sndPrgR5,boolean& sndPrgL5,boolean& mStop){
  
  Serial.print(":");
  Serial.print(joyVal);
  Serial.print(":");
  
  if(joyVal>iJoyHighVal){ // //900//3600
    Serial.print("R_Move speed 5");    
        if(client.connected()){ //Om mqtt connection
          if(!sndPrgR5){ //Om send in prgss
              sndPrgR5=true;
              sndPrgR2=false;  
              client.publish("camtorret/control/movement", makeCommand(cmdStop,stopSpeed).c_str());//crMoveStpCmd
              delay(500); 
              client.publish("camtorret/control/movement", makeCommand(cmdRgtUp,highSpeed).c_str());///crMoveRHcmd
              delay(500);           
              mStop=true;
              //return 1;
          }
              
                       
        }else{
            reconnect();
        }
        
  }else if(joyVal>iJoyMidHVal+200){//600//2500
    Serial.print("R_Move speed 2");  
        if(client.connected()){ //Om mqtt connection
          if(!sndPrgR2){ //Om send in prgss
              sndPrgR5=false;
              sndPrgR2=true;
              client.publish("camtorret/control/movement", makeCommand(cmdStop,stopSpeed).c_str());
              delay(500);           
              client.publish("camtorret/control/movement", makeCommand(cmdRgtUp,lowSpeed).c_str());//crMoveRLcmd
              mStop=true;
              //return 2;
          }
              
        }else{
          reconnect();
        } 
 
  }else if ((joyVal<iJoyMidHVal-250) &&(joyVal>iJoyMidLVal)){ //500 //200// 3500-1600
    Serial.print("L_Move speed 2"); 
      if(client.connected()){//Om mqtt connection
        if(!sndPrgL2){//Om send in prgss
            sndPrgL5=false;
            sndPrgL2=true;
            client.publish("camtorret/control/movement", makeCommand(cmdStop,stopSpeed).c_str());
            delay(500);           
            client.publish("camtorret/control/movement", makeCommand(cmdLoDwn,lowSpeed).c_str());//crMoveLHcmd
            mStop=true;
            //return 3;
        }
        
      }else{
        reconnect();
      }
  }else if(joyVal<iJoyMidLVal){//199
    
    Serial.print("L_Move speed 5");
    
      
      if(client.connected()){//Om mqtt connection
        if(!sndPrgL5){//Om send in prgss
            sndPrgL5=true;
            sndPrgL2=false;
            client.publish("camtorret/control/movement", makeCommand(cmdStop,stopSpeed).c_str());//crMoveStpCmd
            delay(500);           
            client.publish("camtorret/control/movement", makeCommand(cmdLoDwn,highSpeed).c_str());//crMoveLLcmd
            mStop=true;
            //return 4;
        }
        
    }else{
      reconnect();
    }
  }else if((joyVal > iJoyMidLVal) && (joyVal< iJoyMidHVal)) {//Inom tolerans
    Serial.print("I mitten");
    
    if(mStop){
      Serial.print("stop_move");
      if(client.connected()){//Om mqtt connection
        client.publish("camtorret/control/movement", makeCommand(cmdStop,stopSpeed).c_str());
        mStop=false;
        sndPrgR5=false;
        sndPrgR2=false;
        sndPrgL2=false;
        sndPrgL5=false;
        
      }else{
        reconnect();
      }
       return 10;  
    }
    
  }
  
}

void sendUnitToHome(){
  client.publish("camtorret/control/movement", makeHomeCommand(cmdHome,0).c_str()); //makeCommand(String cmdCmd,String spd)
  //
}

void changeDriftMode(String mode){
  Serial.println("DriftModeChange:");
  client.publish("camtorret/control/movement", makeCommand(mode,"0").c_str()); //makeCommand(String cmdCmd,String spd)
}
//*****************Återanslut MQTT*********************
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Huzzah")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("lumbergatan/iot/", "Hello MQTT , HUZZAH IS UP!");
      
      // ... and resubscribe
      //client.subscribe("camtorret/control/ack");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop()
{
  int analogReadJoyY=0;
  int analogReadJoyX=0;
 
  analogReadJoyX=analogRead(analogInPin_1);
  analogReadJoyY=analogRead(analogInPin_2);
  

    int tmp=eval_joystick(analogReadJoyX,JoyHighVal,JoyMidHVal,JoyMidLVal,"B_MoveStop","Rmove","Lmove",sendInPrgssR2,sendInPrgssL2,sendInPrgssR5,sendInPrgssL5,a_moveStop);

    int tmp2=eval_joystick(analogReadJoyY,JoyHighValy,JoyMidHValy,JoyMidLValy,"C_MoveStop","Umove","Dmove",sendInPrgssU2,sendInPrgssD2,sendInPrgssU5,sendInPrgssD5,b_moveStop);
    buttonState_home=digitalRead(inputHome);//26
    buttonState_drift=digitalRead(inputchangeDriftMode);

  Serial.println("Buttons:");
  
  
  if(buttonState_home==HIGH){
    Serial.println("Unit to home position "+ buttonState_home);
    
    sendUnitToHome();
    delay(5000); //Wait 2s
  }else if(buttonState_drift){
    Serial.println("Change driftMode"+ buttonState_drift);
    if(Auto){
      changeDriftMode("Man");
      Auto=false;
    }else{
      changeDriftMode("Auto");
      Auto=true;
    }
    
    
    delay(10000);
  }
  Serial.println(analogReadJoyX);
  Serial.println(analogReadJoyY);
  

  delay(1000); 
  if (!client.connected()) {  
      reconnect();
  }  
}

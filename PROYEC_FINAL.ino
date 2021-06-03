#include "ESP32_MailClient.h"
#include <Arduino.h>
#include <ThingSpeak.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include <FirebaseESP32.h>
SoftwareSerial conexion;
SMTPData datosSMTP;
Servo servomotor;
int led= 23;
bool bandera=false;
int val = 36;
int humedad=34;
int x;
String estado;
String clima;//PUERTA
String json;
int pos = 0;
int servoPin = 4;
int rele= 22;
int solenoide= 5;
# define SENSOR  2
# define CHANNEL_ID 1406892
# define CHANNEL_API_KEY "70MDO0JV6ZY92CBG"
# define FIREBASE_HOST "https://proyecto-iot-b5e84-default-rtdb.firebaseio.com/"
# define FIREBASE_AUTH "IflCqk03eivSxzhDKS8I2u9j7uE18K46U4HL8Zzl"
# define FIREBASE_SSID "Hogar"
# define FIREBASE_PASSWORD "195EDF8FD198D"
FirebaseData firebaseData;
String nodo= "/SENSORES";
bool iterar= true;
DHT dht(SENSOR,DHT11);
int T;
int H;
float ecuacion;
String GeneralPub;
String tanque;
String temperatura;
//**************************************
//*********** MQTT CONFIG **************
//**************************************
const char *mqtt_server = "ioticos.org";
const int mqtt_port = 1883;
const char *mqtt_user = "JbijC8Kc7JIoC4b";
const char *mqtt_pass = "8uTw7r1B26X2LHo";
const char *root_topic_subscribe = "VrAeLdmvqtf9RRe/in";
const char *root_topic_publish = "VrAeLdmvqtf9RRe/outgen";
const char *root_topic_publish_tanque = "VrAeLdmvqtf9RRe/tanque";
const char *root_topic_publish_hum = "VrAeLdmvqtf9RRe/tem";


//**************************************
//*********** WIFICONFIG ***************
//**************************************
const char* ssid = "Hogar";
const char* password =  "195EDF8FD198D";



//**************************************
//*********** GLOBALES   ***************
//**************************************
WiFiClient espClient;
WiFiClient Client;
PubSubClient client(espClient);
char msg[100];
char msg_tanque[25];
char msg_tem[25];


//************************
//** F U N C I O N E S ***
//************************
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void setup_wifi();

void setup() 
{
  ThingSpeak.begin(Client);
  analogReadResolution(10);
   estado="Cerrado";
   clima="NO Lluvia";
  servomotor.write(0);
  dht.begin();
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servomotor.setPeriodHertz(50);    // standard 50 hz servo
  servomotor.attach(servoPin, 500, 2400);
  pinMode(rele, OUTPUT);
  pinMode(solenoide, OUTPUT);
  pinMode(val, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(rele, 0);
  digitalWrite(led, 0);
  digitalWrite(solenoide, 1);
  Serial.begin(9600);
  conexion.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() 
{
  json="{\n \"PROYECTO\": IOT\n \"DOMOTICA\": CONTROL\n \"SENSORES\": [\n{\"TEMPERATURA\": "+String(T) +"}\n {\"HUMEDAD\": "+String(H) +"}\n {\"CLIMA\": "+String(val) +"}\n {\"HUMEDAD SUELO\": "+String(ecuacion) +"}\n]\n }";//JSON**************JSON
  Serial.println(json);
  
  if (!client.connected()) 
  {
    reconnect();
  }

  if (client.connected())
  {
   
    
  T= dht.readTemperature();
  H= dht.readHumidity();
  ThingSpeak.setField(1,T);
  ThingSpeak.setField(2,H);
  ThingSpeak.setField(3,val);
  ThingSpeak.setField(4,ecuacion);
  ThingSpeak.writeFields(CHANNEL_ID,CHANNEL_API_KEY);
  iterar= true;
   while(iterar)
  {
  
    /*
    //LEER DATOS
    Firebase.getInt(firebaseData, nodo + "/TEMPERATURA");
    Serial.println(firebaseData.intData());
    delay(250);
    Firebase.getInt(firebaseData, nodo + "/LLUVIA");
    Serial.println(firebaseData.intData());
    delay(250);
    */

    
 Firebase.setInt(firebaseData, nodo + "/Humedad-Ambiente: ", H);
    //Serial.println(firebaseData.intData());
 Firebase.setInt(firebaseData, nodo + "/Temperatura: ", T);
    //Serial.println(firebaseData.intData());
  Firebase.setInt(firebaseData, nodo + "/Clima: ", val);
    //Serial.println(firebaseData.intData());
    Firebase.setInt(firebaseData, nodo + "/Humedad-Suelo: ", ecuacion);
    iterar=false;
    //Firebase.end(firebaseData);
  }
  
  //H= dht.readHumidity();
  if(T>x && x>20)
  {
    correoVentilador();
    digitalWrite(rele, 1);
  }
  
  if(T<x && x>20)
  {
    digitalWrite(rele, 0);
  }


humedad = analogRead(34);//HUMEDAD SUELO
ecuacion=(humedad/1023.0)*100;
//Serial.println(ecuacion);
val = analogRead(36);//LLUVIA
  
  if(val<700)//LLUVIA*******************LLUVIA*************LLUVIA**********************
  {
    correolluvia();
    clima="Lluvia";
    servomotor.write(90);
    delay(1000);
  }
  
 else if(val>700)//NO LLUVIA*******************NO LLUVIA************* NO LLUVIA**********************
 {
  clima="NO Lluvia";
  servomotor.write(0);
  delay(1000);
 }

 if(ecuacion<=25 && bandera==1)//DATOS BASADOS EN PLANTA CON*************RIEGO**************RIEGO**************
 {
  digitalWrite(led, 0);
 }
 if(ecuacion>25 && bandera==1 )//SE ACTIVARA UNA BOMBA SIMULADA POR LED*******RIEGO**************RIEGO**************
 {
  digitalWrite(led, 1);
 }
    if(bandera==0)
    {
    delay(1000);  
    GeneralPub ="Temperatura: "+String(T)+"*****"+"Clima: "+String(clima);
    GeneralPub.toCharArray(msg,100);
    client.publish(root_topic_publish,msg);
    }
    if(bandera==1)
    {
    delay(1000);  
    GeneralPub ="Temperatura: "+String(T)+"*****"+"Clima: "+String(clima)+"*****"+"Sensor-Humedad: "+(ecuacion);
    GeneralPub.toCharArray(msg,100);
    client.publish(root_topic_publish,msg);
    }
    
    tanque ="Clima: "+String(clima);
    tanque.toCharArray(msg_tanque,25);
    client.publish(root_topic_publish_tanque,msg_tanque);

    temperatura ="Temperatura: "+String(T);
    temperatura.toCharArray(msg_tem,25);
    client.publish(root_topic_publish_hum,msg_tem);
  
  }
  client.loop();
  
}




//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi(){
  delay(10);
  // Nos conectamos a nuestra red Wifi
  Serial.println();
  Serial.print("Conectando a ssid: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a red WiFi!");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}



//*****************************
//***    CONEXION MQTT      ***
//*****************************

void reconnect() 
{

  while (!client.connected()) {
    Serial.print("Intentando conexión Mqtt...");
    // Creamos un cliente ID
    String clientId = "IOTICOS_H_W_";
    clientId += String(random(0xffff), HEX);
    // Intentamos conectar
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("Conectado!");
      // Nos suscribimos
      if(client.subscribe(root_topic_subscribe)){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("fallo Suscripciión");
      }
    } else {
      Serial.print("falló :( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");
      delay(5000);
    }
  }
}


//*****************************
//***       CALLBACK        ***
//*****************************

void callback(char* topic, byte* payload, unsigned int length)
{
  String incoming = "";
  Serial.print("Mensaje recibido desde -> ");
  Serial.print(topic);
  Serial.println("");
  for (int i = 0; i < length; i++) 
  {
    incoming += (char)payload[i];
  }
  incoming.trim();
  Serial.println("Mensaje -> " + incoming);
  
  x = incoming.toInt();

  
 
   if(incoming=="sueloD")
   {
    bandera=true;
    delay(1000);
   }
  
 if(incoming=="sueloU")
   {
    digitalWrite(led, 0);
    bandera=false;
    delay(1000);
   }
   
    
}
   

//*****************************
//***       CORREO ABRIR PUERTA       ***
//*****************************
void correolluvia()
{
//Configuración del servidor de correo electrónico SMTP, host, puerto, cuenta y contraseña
datosSMTP.setLogin("smtp.gmail.com", 465, "lab3iot@gmail.com", "IOT_PRU*.OBJ89_i");
// Establecer el nombre del remitente y el correo electrónico
datosSMTP.setSender("CENTROL DE CONTROL", "HOGAR");
// Establezca la prioridad o importancia del correo electrónico High, Normal, Low o 1 a 5 (1 es el más alto)
datosSMTP.setPriority("High");
// Establecer el asunto
datosSMTP.setSubject("ATENCION");
// Establece el mensaje de correo electrónico en formato de texto (sin formato)
datosSMTP.setMessage("LLENANDO TANQUE", false);
// Agregar destinatarios, se puede agregar más de un destinatario
datosSMTP.addRecipient("oscarivan@unisangil.edu.co");
 //Comience a enviar correo electrónico.
if (!MailClient.sendMail(datosSMTP))
Serial.println("Error enviando el correo, " + MailClient.smtpErrorReason());
//Borrar todos los datos del objeto datosSMTP para liberar memoria
datosSMTP.empty();
delay(7000);
}


//*****************************
//***       CORREO VENTILADOR       ***
//*****************************
void correoVentilador()
{
//Configuración del servidor de correo electrónico SMTP, host, puerto, cuenta y contraseña
datosSMTP.setLogin("smtp.gmail.com", 465, "lab3iot@gmail.com", "IOT_PRU*.OBJ89_i");
// Establecer el nombre del remitente y el correo electrónico
datosSMTP.setSender("CENTROL DE CONTROL", "lab3iot@gmail.com");
// Establezca la prioridad o importancia del correo electrónico High, Normal, Low o 1 a 5 (1 es el más alto)
datosSMTP.setPriority("High");
// Establecer el asunto
datosSMTP.setSubject("MONITOREO");
// Establece el mensaje de correo electrónico en formato de texto (sin formato)
datosSMTP.setMessage("VENTILADOR ENCENDIDO", false);
// Agregar destinatarios, se puede agregar más de un destinatario
datosSMTP.addRecipient("oscarivan@unisangil.edu.co");
 //Comience a enviar correo electrónico.
if (!MailClient.sendMail(datosSMTP))
Serial.println("Error enviando el correo, " + MailClient.smtpErrorReason());
//Borrar todos los datos del objeto datosSMTP para liberar memoria
datosSMTP.empty();
delay(5000);
}

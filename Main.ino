#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include "env.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266HTTPClient.h>

// Credenciales de la red WiFi
const char WIFI_SSID[] = "";      // Nombre de la red WiFi
const char WIFI_PASSWORD[] = "";  // Contraseña de la red WiFi

// Nombre del dispositivo en AWS
const char THINGNAME[] = "";

// Dirección del broker MQTT en AWS
const char MQTT_HOST[] = "xxxxxxxxxxx.iot.us-east-1.amazonaws.com";

// Tópicos MQTT para publicar y suscribirse
const char AWS_IOT_PUBLISH_TOPIC[] = "esp8266/pub";
const char AWS_IOT_SUBSCRIBE_TOPIC[] = "esp8266/sub";
const char AWS_IOT_JOKE_TOPIC[] = "esp8266/joke"; 

// Intervalo de publicación de datos (en milisegundos)
const long interval = 20000;

// Desfase horario desde UTC (en horas)
const int8_t TIME_ZONE = -3;

// Variable para almacenar el tiempo de la última publicación
unsigned long lastMillis = 0;

// Objeto WiFiClientSecure para la comunicación segura con AWS
WiFiClientSecure net;

// Certificados X.509 para autenticación con AWS IoT
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

// Instancia del cliente MQTT
PubSubClient client(net);

// Configuración del sensor DHT
uint8_t DHTPin = D8;         // Pin donde está conectado el sensor DHT
#define DHTTYPE DHT22        // Tipo de sensor (DHT22)
DHT dht(DHTPin, DHTTYPE);

// Variables para temperatura y humedad
float Temperature;
float Humidity;

// Configuración del LED
const uint8_t LEDPin = D6;
const float TempThreshold = 30.0; // Temperatura umbral para encender el LED

// Dimensiones de la pantalla OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Pines de conexión para la pantalla OLED
#define OLED_RESET -1 // Pin de reset (-1 si no se utiliza)
#define SDA_PIN D2
#define SCL_PIN D1

// Objeto para manejar la pantalla OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variable para mostrar la respuesta de la IA en la pantalla OLED
String aiResponse = "";
unsigned long displayTime = 0; // Variable para controlar el tiempo de visualización

// Función para conectar al servidor NTP y establecer la hora del sistema
void NTPConnect() {
  Serial.print("Estableciendo hora usando SNTP");
  configTime(TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 1510592825) {
    delay(500);
    Serial.print("...");
    now = time(nullptr);
  }
  Serial.println("¡Hora establecida!");
}

// Función para recibir mensajes MQTT (incluyendo el chiste)
void messageReceived(char *topic, byte *payload, unsigned int length) {
  Serial.print("Recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    aiResponse += (char)payload[i];  // Guardar el chiste recibido
  }
  Serial.println(aiResponse);

  // Mostrar el chiste en la pantalla OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Chiste:");
  display.println(aiResponse);  // Mostrar el chiste en la pantalla
  display.display();

  // Guardar el tiempo en el que se mostró el mensaje
  displayTime = millis();
}

// Función para conectar a AWS IoT Core utilizando MQTT
void connectAWS() {
  delay(3000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println(String("Intentando conectar a SSID: ") + String(WIFI_SSID));

  // Esperar hasta que el WiFi esté conectado
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("...");
    delay(1000);
  }

  // Conectar al servidor NTP para obtener la hora correcta
  NTPConnect();
  
  // Establecer certificados para la conexión segura
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);

  Serial.println("Conectando a AWS IoT");

  // Intentar conectar al cliente MQTT
  while (!client.connect(THINGNAME)) {
    Serial.print("...");
    delay(1000);
  }

  if (!client.connected()) {
    Serial.println("¡Tiempo de espera de AWS IoT!");
    return;
  }

  // Suscribirse a los tópicos MQTT
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  client.subscribe(AWS_IOT_JOKE_TOPIC);  // Suscripción al tópico del chiste
  Serial.println("¡Conectado a AWS IoT!");
}

// Función para publicar la temperatura y humedad en AWS IoT Core
void publishMessage() {
  // Leer la temperatura y humedad del sensor DHT
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();

  // Verificar si las lecturas son válidas
  if (isnan(Temperature) || isnan(Humidity)) {
    Serial.println("¡Error al leer del sensor DHT!");
    return;
  }

  // Controlar el LED basado en la temperatura
  if (Temperature > TempThreshold) {
    digitalWrite(LEDPin, HIGH);
  } else {
    digitalWrite(LEDPin, LOW);
  }

  // Mostrar valores de temperatura y humedad en el monitor serial
  Serial.print("Temperatura: ");
  Serial.print(Temperature);
  Serial.print(" °C, Humedad: ");
  Serial.print(Humidity);
  Serial.println(" %");

  // Crear documento JSON con los datos de la temperatura y humedad
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["temperature"] = Temperature;
  doc["humidity"] = Humidity;

  // Serializar el documento JSON en un buffer
  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);

  // Publicar el mensaje en el tópico MQTT
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(DHTPin, INPUT);
  pinMode(LEDPin, OUTPUT);

  // Inicializar el sensor DHT
  Serial.println("Inicializando sensor DHT...");
  dht.begin();

  // Inicializar la pantalla OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_PAGEADDR, 0x3C)) {
    Serial.println(F("Error al inicializar la pantalla OLED"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Pantalla OLED lista! by fernandosilvot"));
  display.display();
  delay(2000);

  // Conectar a AWS IoT
  connectAWS();
}

void loop() {
  if (millis() - lastMillis > interval) {
    lastMillis = millis();
    if (client.connected()) {
      publishMessage();  // Publicar los datos
    } else {
      connectAWS();  // Reconectar si el cliente MQTT se desconectó
    }
  }
  client.loop();  // Mantener la conexión MQTT activa

  // Verificar si han pasado más de 5 segundos desde que se mostró el mensaje de la IA
  if (millis() - displayTime > 10000 && displayTime != 0) {
    display.clearDisplay();  // Borrar el mensaje después de 5 segundos
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("Pantalla OLED lista! by fernandosilvot"));
    display.display();
    aiResponse = "";  // Limpiar la respuesta de la IA
    displayTime = 0;  // Resetear el temporizador
  }
}

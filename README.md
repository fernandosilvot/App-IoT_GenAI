# Blog Completo sobre AWS IoT y ESP8266

## Introducción
Proyecto de ejemplo para conectar un ESP8266 con AWS IoT Core, usar MQTT para comunicar datos de un sensor DHT22 y recibir mensajes desde Lambda, aprovechando Amazon Bedrock.

## Requisitos
- Módulo ESP8266 (con WiFi)  
- Sensor DHT22  
- LED, resistencias y cables  
- Pantalla OLED opcional (SSD1306)  
- Arduino IDE con librerías (PubSubClient, ArduinoJson, etc.)  
- Certificados y llaves provistos por AWS IoT  
- Cuenta en AWS con IoT Core y Lambda activados  

## Pasos Principales

1. **Crear Objeto en AWS IoT Core**  
   - Ir a la consola de AWS, elegir “AWS IoT Core”.  
   - En “Administrar → Todos los dispositivos → Objetos”, crear objeto.  
   - Generar certificados y política con acciones “iot:Connect”, “iot:Publish”, “iot:Receive” y “iot:Subscribe”.  
   - Guardar endpoint, Device Certificate y Private Key.

2. **Configurar Arduino IDE**  
   - Añadir soporte ESP8266 en el “Gestor de URLs adicionales”.  
   - Instalar librerías (Adafruit GFX, Adafruit SSD1306, DHT, PubSubClient, etc.).  
   - Crear un nuevo sketch con “Main.ino” y “env.h” que contengan el código del ESP8266 y los certificados.

3. **Instalar Drivers y Conectar ESP8266**  
   - Instalar CP210x o CH340 según corresponda.  
   - Subir el sketch al ESP8266 con la configuración de WiFi y broker MQTT.

4. **Crear Lambda e IAM Role**  
   - En la consola de AWS, crear función Lambda (Python 3.x, arquitectura arm64).  
   - Adjuntar rol con permisos (AmazonBedrockFullAccess, AWSIoTFullAccess).  
   - Añadir desencadenador “AWS IoT” con regla “SELECT temperature FROM 'esp8266/pub' WHERE temperature > 30.0” para filtrar datos.

5. **Incluir Lógica en el Código**  
   - “Main.ino” maneja WiFi, MQTT (publicación y suscripción).  
   - “lambda.py” usa Amazon Bedrock para generar chistes y los publica de vuelta por MQTT.  

## Funcionamiento
- ESP8266 lee temperatura/humedad con DHT22 y publica en “esp8266/pub”.  
- AWS IoT activa la Lambda si hay temperatura > 30.  
- Lambda genera y envía un chiste a “esp8266/joke”.  
- ESP8266 muestra el chiste en pantalla OLED y en el monitor serial.

## Conclusiones
Esta integración evita usar HTTP directo al ESP8266. Con MQTT y Lambda el flujo es fiable y económico. El aprendizaje clave radica en gestionar certificados, configurar el entorno y aprovechar AWS IoT y Amazon Bedrock en un solo proyecto.

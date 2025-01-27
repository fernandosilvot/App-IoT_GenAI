# Blog Completo sobre AWS IoT y ESP8266

## Introducción
Este proyecto muestra cómo conectar un ESP8266 con AWS IoT Core y usar MQTT para enviar datos de un sensor DHT22. Además, aprovecha Amazon Bedrock para generar chistes en respuesta a la temperatura. Para más detalles, consulta el blog en:
[Dev.to](https://dev.to/aws-espanol/integracion-iot-y-generative-ai-como-crear-una-app-que-cuenta-chistes-basados-en-la-temperatura-522)

## Primera Versión
https://github.com/user-attachments/assets/466705de-48ce-4d27-9cfa-25a68d369923

## Versión Final
https://github.com/user-attachments/assets/ccf6148a-5d03-41e3-996d-c1ffeac1d18d

## Requisitos
- Módulo ESP8266 (con WiFi)  
- Sensor DHT22  
- LED, resistencias y cables  
- Pantalla OLED opcional (SSD1306)  
- Arduino IDE con librerías (PubSubClient, ArduinoJson, etc.)  
- Certificados y llaves provistos por AWS IoT  
- Cuenta en AWS con IoT Core y Lambda activados  

## Pasos Principales
1. **Crear un objeto en AWS IoT Core**  
   - Crear y configurar el objeto.  
   - Generar certificados y política con acciones esenciales (Connect, Publish, Receive, Subscribe).  
   - Guardar endpoint, Device Certificate y Private Key.

2. **Configurar Arduino IDE**  
   - Añadir soporte para ESP8266 en el “Gestor de URLs adicionales”.  
   - Instalar librerías necesarias.  
   - Crear “Main.ino” y “env.h” con el código y certificados.

3. **Instalar drivers y subir el sketch**  
   - Instalar drivers CP210x o CH340.  
   - Subir el código al ESP8266 con la configuración de WiFi y MQTT.

4. **Crear Lambda e IAM Role**  
   - Crear función Lambda (Python 3.x).  
   - Asignar rol con permisos (AmazonBedrockFullAccess, AWSIoTFullAccess).  
   - Añadir disparador “AWS IoT” con regla para filtrar datos (p.ej., temperatura > 30).

5. **Incluir lógica en el código**  
   - “Main.ino” maneja publicación y suscripción MQTT.  
   - “lambda.py” usa Amazon Bedrock para generar chistes y enviarlos por MQTT.

## Funcionamiento
- El ESP8266 publica temperatura y humedad en “esp8266/pub”.  
- AWS IoT invoca Lambda si la temperatura supera el umbral.  
- Lambda genera un chiste y lo envía a “esp8266/joke”.  
- El ESP8266 muestra el chiste en la pantalla OLED o por monitor serial.

## Conclusiones
Con MQTT y Lambda, este flujo es confiable y rentable. La clave está en configurar certificados, aprovechar Amazon Bedrock y unir IoT con Generative AI en un solo proyecto.

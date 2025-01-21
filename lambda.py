import boto3
import json
import re

def lambda_handler(event, context):
    client = boto3.client('iot-data', region_name='us-east-1')
    bedrock = boto3.client(service_name='bedrock-runtime', region_name='us-east-1')
    modelId = 'meta.llama3-70b-instruct-v1:0'
    
    # Construcción del prompt
    topic = json.dumps(event)  
    prompt_base = (
        f"Eres un ingeniero IT, y tienes que dar un chiste corto de una frase "
        f"de 10 palabras en español sobre: {topic}."
    )
    body = {
        "prompt": prompt_base,
        "temperature": 0.7,
        "top_p": 0.9,
        "max_gen_len": 100
    }

    try:
        # Llamada al modelo
        response = bedrock.invoke_model(
            modelId=modelId,
            body=json.dumps(body), 
            accept='application/json',
            contentType='application/json'
        )
        
        # Leer y decodificar el contenido de la respuesta
        response_body = json.loads(response['body'].read().decode('utf-8'))
        raw_text = response_body.get('generation', 'No se pudo generar un chiste.')

        # Limpiar el texto para eliminar caracteres o patrones no deseados
        clean_text = re.sub(r"\\n|\\u[0-9a-fA-F]{4}", "", raw_text).strip()  # Elimina saltos de línea y unicodes
        clean_text = re.sub(r"\s+", " ", clean_text)  # Reemplaza múltiples espacios con un único espacio
        clean_text = clean_text.replace("\"", "")  # Elimina comillas si están presentes

        # Publicar en el tópico MQTT
        client.publish(
            topic='esp8266/joke',
            qos=0,
            payload=json.dumps({"joke": clean_text})
        )

        return {
            'statusCode': 200,
            'body': json.dumps({"joke": clean_text})
        }
    except Exception as e:
        print(f"Error: {str(e)}")
        return {
            'statusCode': 500,
            'headers': {
                'Content-Type': 'application/json',
                'Access-Control-Allow-Origin': '*'
            },
            'body': json.dumps({
                'error': str(e)
            })
        }
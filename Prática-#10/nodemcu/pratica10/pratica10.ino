/*-----------------------------------------------------------------*/
/**

  @file   pratica10.ino
  @author Eduardo Barros, Christian Oliveira e Lucas Pinheiro
  @brief  Implementa uma leitura de temperatura, umidade e luminosidade
          bem como um controle de motor de passo, com MQTT e Node-RED
 */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <AccelStepper.h>


/*-----------------------------------------------------------------
                            Definições
-----------------------------------------------------------------*/
// Configurações Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA";

// Configurações MQTT
const char* mqtt_server = "IP_DO_SEU_COMPUTADOR"; // IP do computador que executa o Node-RED
const int mqtt_port = 1883;

// Configurações DHT
#define DHTPIN D5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Configurações do Motor de Passo
#define STEP_PIN_1 D1
#define STEP_PIN_2 D2
#define STEP_PIN_3 D3
#define STEP_PIN_4 D4
AccelStepper stepper(AccelStepper::FULL4WIRE, STEP_PIN_1, STEP_PIN_3, STEP_PIN_2, STEP_PIN_4);

// Configuração do LED de Alarme
#define ALARM_LED_PIN D6

/*-----------------------------------------------------------------
                          Variáveis Globais
-----------------------------------------------------------------*/
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

// Variáveis para o Fotoresistor
int lightSensorPin = A0;

/*-----------------------------------------------------------------
                      Funções Implementadas
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief Função responsável por fazer a leitura das mensagens em um
          tópico do Broker.
   @param  char* topic Tópico em que a mensagem foi publicada.
   @param  byte* payload Conteúdo da mensagem em binário.
   @param  unsigned int length Tamanho do payload.
*/
/*-----------------------------------------------------------------*/
void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp; //variável para a mensagem recebida em string
  
  // Conversão do payload em string
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }

  // Controle do Motor de Passo
  if (String(topic) == "comando/motor") {
    // Converte a mensagem em int
    int position = messageTemp.toInt();
    // Move o motor para a posição
    stepper.moveTo(position);
  }

  // Controle do Alarme do LED
  if (String(topic) == "comando/alarme") {
    if (messageTemp == "ON") {
      // Liga o LED de alarme
      digitalWrite(ALARM_LED_PIN, HIGH);
    } else {
      // Desliga o LED de alarme
      digitalWrite(ALARM_LED_PIN, LOW);
    }
  }
}

/*-----------------------------------------------------------------*/
/**
   @brief Função responsável por fazer a conexão WI-FI.
*/
/*-----------------------------------------------------------------*/
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Aguarda a conexão ser estabelecida
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

/*-----------------------------------------------------------------*/
/**
   @brief Função responsável por refazer a conexão WI-FI.
*/
/*-----------------------------------------------------------------*/
void reconnect() {
  // Enquanto o cliente não estiver conectado...
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect("NodeMCUClient")) {
      Serial.println("conectado");
      // Inscreve-se nos tópicos utilizados
      client.subscribe("comando/motor");
      client.subscribe("comando/alarme");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state()); // código de retorno do cliente MQTT
      Serial.println(" tentaremos novamente em 5 segundos");
      delay(2000);
    }
  }
}

void setup() {

  // Seta o pino do LED de alarme como saída
  pinMode(ALARM_LED_PIN, OUTPUT);

  // Desliga o LED de alarme
  digitalWrite(ALARM_LED_PIN, LOW);

  // Inicialização do Serial
  Serial.begin(115200);

  setup_wifi();

  // Configuração do Broker MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Inicialização do sensor de temperatura e umidade
  dht.begin();

  // Configuração do Motor de Passo
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
}

void loop() {
  // Verifica se a conexão com o Broker MQTT está ativa
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Mantém a conexão com o Broker

  // Atualiza a posição do Motor de Passo
  stepper.run();

  // Verifica se é hora de enviar uma mensagem (a cada 2s)
  long now = millis(); // tempo atual
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // Leitura da Temperatura e Umidade
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Verifica se a leitura foi bem sucedida
    if (isnan(h) || isnan(t)) {
      Serial.println("Falha ao ler do sensor DHT");
      return;
    }

    // Publica Temperatura
    String tempStr = String(t);
    client.publish("sensor/temperatura", tempStr.c_str());

    // Publica Umidade
    String humStr = String(h);
    client.publish("sensor/umidade", humStr.c_str());

    // Leitura do Fotoresistor Selecionado
    int lightValue = analogRead(lightSensorPin);

    // Converter valor em estado
    String lightState;
    if (lightValue > 800) {
      lightState = "Iluminado";
    } else if (lightValue > 400) {
      lightState = "Meia Luz";
    } else {
      lightState = "Apagado";
    }

    // Publicar Estado do Fotoresistor
    client.publish("sensor/fotoresistor", lightState.c_str());
  }
}
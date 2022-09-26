//#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


//defines:
//defines de id mqtt e tópicos para publicação e subscribe
#define TOPICO_SUBSCRIBE "data/receive"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "data/post"    //tópico MQTT de envio de informações para Broker
 
#define ID_MQTT  "1"     //id mqtt (para identificação de sessão)
#define ID_USER  "MQTTGardenController"
#define ID_SENHA  "ce91b9e8-c99b-4527-b32e-993e9ddd0181"                                

#define DHTPIN 23     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);

// WIFI
const char* ssid = "CLARO_2G1DD51B"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "381DD51B"; // Senha da rede WI-FI que deseja se conectar
//const char* ssid = "moto g(7) 9225"; // SSID / nome da rede WI-FI que deseja se conectar
//const char* PASSWORD = "1234qwer"; // Senha da rede WI-FI que deseja se conectar
  
// MQTT
const char* BROKER_MQTT = "mqtt.tago.io"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

String Vteste = "";

char caractere;
int i;

unsigned long lastConnectionTime = 0;            // Ultimo instante que a comunicação foi feita
const unsigned long postingInterval = 60000; // Intervalo entre as comunicações, em milisegundos
 
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void initDHT();
String getTEMP();
String getUMID();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
 
/* 
 *  Implementações das funções
 */
void setup() 
{
    //inicializações:
    initSerial();
    initWiFi();
    initMQTT();
    initDHT();
}
  
//Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial 
//        o que está acontecendo.
//Parâmetros: nenhum
//Retorno: nenhum
void initSerial() 
{
    Serial.begin(115200);
}
 
//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(ssid);
    Serial.println("Aguarde");
     
    reconectWiFi();
}
  
//Função: inicializa parâmetros de conexão MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

//Função: inicializa DHT11 para monitoramento de temperatura e umidade
//Parâmetros: nenhum
//Retorno: nenhum
void initDHT() 
{
    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
    Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
    Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
    Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
    Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
    Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
}

//Função: puxa temperatura do DHT
//Parâmetros: nenhum
//Retorno: Temperatura - String
String getTEMP()
{
    // Get temperature event and print its value.
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    else {
      Serial.print(F("Temperature: "));
      Serial.print(event.temperature);
      Serial.println(F("°C"));
    }
    return String(event.temperature);
}

//Função: puxa temperatura do DHT
//Parâmetros: nenhum
//Retorno: Temperatura - String
String getUMID() 
{
    // Get temperature event and print its value.
    sensors_event_t event;
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    else {
      Serial.print(F("Humidity: "));
      Serial.print(event.relative_humidity);
      Serial.println(F("%"));
    }
    return String(event.relative_humidity);
}

//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: topic, payload e tamanho
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
   
    //Imprime mensagem recebida via MQTT
    Serial.println(msg);     
}
  
//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT, ID_USER, ID_SENHA)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(ssid, PASSWORD); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(ssid);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
 
//Função: envia ao Broker o estado atual do output 
//Parâmetros: variavel, valor e unidade
//Retorno: nenhum
void EnviaMQTT(String var, String val, String uni)
{
    //String PostData1 = String("{\n\t\t\"variable\": \"") + String(var) + String("\", \n\t\t\"unit\"    : \"") + String(uni) + String("\", \n\t\t\"value\"   : ") + String(val)+ String("\n}");
    String PostData1 = String("{\n\t\t\"variable\": \"") + String(var) + String("\", \n\t\t\"unit\" : \"") + String(uni) + String("\", \n\t\t\"value\" : ") + String(val)+ String("\n}");
    char vet[100];
    PostData1.toCharArray(vet,100);
    Serial.println(vet);

    if (MQTT.connect(ID_MQTT, ID_USER, ID_SENHA)) 
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.publish(TOPICO_PUBLISH, vet); 
    } 
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }

}
 
//programa principal
void loop() 
{   
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();
    
    /*if(Serial.available()>0){
      Vteste = "";
      
      while(Serial.available() > 0) {
        // Lê byte da serial
        caractere = Serial.read();
        Vteste.concat(caractere);
        // Aguarda buffer serial ler próximo caractere
        delay(10);
      }

      Serial.println(Vteste);
            
    }*/

    
    
    if (millis() - lastConnectionTime > postingInterval) {
        sensors_event_t event;

        EnviaMQTT("Umidade",getUMID()," %");
        EnviaMQTT("Temperatura",getTEMP()," °C");   
      
        lastConnectionTime = millis();
    }
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();
}

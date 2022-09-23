#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
 
//defines:
//defines de id mqtt e tópicos para publicação e subscribe
#define TOPICO_SUBSCRIBE "UNIONMAQUINALEDRECEBE"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "tago/data/post"    //tópico MQTT de envio de informações para Broker
 
#define ID_MQTT  "UNIONMAQUINAS"     //id mqtt (para identificação de sessão)
#define ID_USER  "UNIONMAQUINALED"
#define ID_SENHA  "f8e27f2c-bf04-435c-8405-ae63c18115c4"                                
 
//defines - mapeamento de pinos do NodeMCU
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1
 
// WIFI
const char* ssid = "ArlaFacil-MAQ-Petrobras"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "arla32faciliot"; // Senha da rede WI-FI que deseja se conectar
  
// MQTT
const char* BROKER_MQTT = "mqtt.tago.io"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

String vartotal = "";
char rele[17];
char STS[10];
String STSt = "";
char STSNV[9];
String STSNVt = "";
String STSRELEt = "";
String sinal = "";
String densidade = "";
String T1 = "";
String T2 = "";
String T3 = "";
String FREQ1 = "";
String FREQ2 = "";
String STSBLOQ = "";
String memoFlux = "";
String STSPRESSFLU = "";
String STSOSMOSE = "";
String STSAG = "";
String STSCABECOTE = "";
String STSTQA = "";
String STSARLA = "";

char vettrans[150];
int MAX;
char caractere;
int i;
int contvar;

unsigned long lastConnectionTime = 0;            // Ultimo instante que a comunicação foi feita
const unsigned long postingInterval = 1L * 60000L; // Intervalo entre as comunicações, em milisegundos
 
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);
 
/* 
 *  Implementações das funções
 */
void setup() 
{
    //inicializações:
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
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
            digitalWrite(D0, HIGH);
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            digitalWrite(D0, LOW);
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
        digitalWrite(D1, LOW);
    }
   
    Serial.println();
    digitalWrite(D1, HIGH);
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
    String PostData1 = String("{\n\t\t\"variable\": \"") + String(var) + String("\", \n\t\t\"unit\"    : \"") + String(uni) + String("\", \n\t\t\"value\"   : ") + String(val)+ String("\n}");
    char vet[100];
    PostData1.toCharArray(vet,100);
    Serial.println(vet);

    if (MQTT.connect(ID_MQTT, ID_USER, ID_SENHA)) 
    {
      digitalWrite(D0, HIGH);
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.publish(TOPICO_PUBLISH, vet); 
    } 
    else
    {
      digitalWrite(D0, LOW);
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }

}
 
//Função: inicializa o output em nível lógico baixo
//Parâmetros: nenhum
//Retorno: nenhum
void InitOutput(void)
{
    //Ascende LED em D0 para que MQTT OK
    pinMode(D0, OUTPUT);
    digitalWrite(D0, LOW);
    //Ascende LED em D1 para WIFI OK
    pinMode(D1, OUTPUT);
    digitalWrite(D1, LOW);
    //Ascende LED em D2 para SERIAL OK
    pinMode(D2, OUTPUT);
    digitalWrite(D2, LOW);
}
 
 
//programa principal
void loop() 
{   
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();

    if(Serial.available()>0){
      digitalWrite(D2, HIGH);
      vartotal = "";
      STSt = "";
      STSNV[9];
      STSNVt = "";
      STSRELEt = "";
      sinal = "";
      densidade = "";
      T1 = "";
      T2 = "";
      T3 = "";
      FREQ1 = "";
      FREQ2 = "";
      STSBLOQ = "";
      memoFlux = "";
      STSPRESSFLU = "";
      STSOSMOSE = "";
      STSAG = "";
      STSCABECOTE = "";
      STSTQA = "";
      STSARLA = "";
      
      MAX = 0;
      while(Serial.available() > 0) {
        // Lê byte da serial
        caractere = Serial.read();
        vartotal.concat(caractere);
        // Aguarda buffer serial ler próximo caractere
        delay(10);
        MAX++;
      }
      vartotal.toCharArray(vettrans, 150);

      Serial.println(vettrans);
      
      for(i=0, contvar=0; i<MAX; i++){
        if(vettrans[i] == '#'){
          contvar++;
        }
        
        if(vettrans[i] != '#' && contvar == 1){
          sinal.concat(vettrans[i]);
        }
        if(vettrans[i] != '#' && contvar == 2){
          densidade.concat(vettrans[i]);
        }
        if(vettrans[i] != '#' && contvar == 3){
          FREQ1.concat(vettrans[i]);
        }
        if(vettrans[i] != '#' && contvar == 4){
          FREQ2.concat(vettrans[i]);
        }
        if(vettrans[i] != '#' && contvar == 5){
          STSNVt.concat(vettrans[i]);
        }
        if(vettrans[i] != '#' && contvar == 6){
          STSRELEt.concat(vettrans[i]);
        }
        if(vettrans[i] != '#' && contvar == 7){
          STSARLA.concat(vettrans[i]);
        }
        if(vettrans[i] != '#' && contvar == 8){
          STSBLOQ.concat(vettrans[i]);
        }
      }
      
    }
    digitalWrite(D2, LOW);
    
    if (millis() - lastConnectionTime > postingInterval) {
      EnviaMQTT("Conexap","1"," -");
      EnviaMQTT("Concentracao",sinal," %");
      EnviaMQTT("Densidade",densidade," g/ml");
      EnviaMQTT("STSARLA",STSARLA," DG");
      EnviaMQTT("STSBLOQ",STSBLOQ," DG");

      STSRELEt.toCharArray(rele, 17);
      int j; 
      for(i=0, j=0; i<16; i++){
        j=i+1;
        EnviaMQTT(String(String("Rele") + String(j)),String(rele[i])," -");
      }
      STSNVt.toCharArray(STSNV, 9);
      for(i=0, j=0; i<7; i++){
        j=i+1;
        EnviaMQTT(String(String("STSNV") + String(j)),String(STSNV[i])," -");
      }
      EnviaMQTT("Frequencia1",FREQ1," %");
      EnviaMQTT("Frequencia2",FREQ2," %");
      
      
      lastConnectionTime = millis();
    } 
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();
}

#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
 

 
// Configurações do WiFi
const char* ssid     = "CLARO_2G1DD51B"; // Nome da rede WiFi
const char* password = "381DD51B"; // Senha da rede WiFi
 
 
// Configurações do Servidor NTP
const char* servidorNTP = "a.st1.ntp.br"; // Servidor NTP para pesquisar a hora
 
const int fusoHorario = -10800; // Fuso horário em segundos (-03h = -10800 seg)
const int taxaDeAtualizacao = 1800000; // Taxa de atualização do servidor NTP em milisegundos
 
WiFiUDP ntpUDP; // Declaração do Protocolo UDP
NTPClient timeClient(ntpUDP, servidorNTP, fusoHorario, 60000);
 

 
void setup()
{  
  // Declarar Serial para realizar debug do código
  Serial.begin(115200);
  delay(10);
 
 
  // Conectar ao WiFi
  WiFi.begin(ssid, password);
 
 
 
  // Aguardando conexão do WiFi
  while (WiFi.status() != WL_CONNECTED)
  {
 
      Serial.print(".");
      delay(500);
  }
   
  Serial.println("");
  Serial.print("WiFi conectado. Endereço IP: ");
  Serial.println(WiFi.localIP());
 
 
  // Iniciar cliente de aquisição do tempo
  timeClient.begin();
}
  
void loop()
{
  timeClient.update();
  Serial.println(timeClient.getFormattedDate());
  Serial.println(timeClient.getEpochTime());
  Serial.println(timeClient.getDay());
  Serial.println(timeClient.getHours());  
  Serial.println(timeClient.getMinutes());
 
  String horario = timeClient.getFormattedTime();
 
  Serial.println(horario);
 
  delay(800);
}

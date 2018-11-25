// --- Bibliotecas Auxiliares ---
#include <SPI.h> // Biblioteca para o barramento
#include <MFRC522.h> // Biblioteca do sensor RC522
#include <WiFi.h> // Biblioteca comunicação Wifi
#include <PubSubClient.h> // Biblioteca para publicar utilizando o protocolo MQTT

// --- Mapeamento de Hardware ---
#define SS_PIN 21 
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Cria instância com MFRC522
#define wifi_ssid "SSID"
#define wifi_password "PASSWORD"
#define mqtt_server "10.0.0.101"
//#define mqtt_user "admin"
//#define mqtt_password "admin"
#define your_topic "/esp32/rfidLeitura"
WiFiClient espClient;
PubSubClient client(espClient);

// --- Variáveis Globais --- 
char st[20];
String a;
String conteudo;
String rfidLeitura;
char data[20];

// --- Configurações Iniciais ---
void setup(){
  Serial.begin(115200);
  SPI.begin();          // Inicia comunicação SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("Aproxime o seu cartao do leitor...");
  Serial.println();
  
  
} //end setup

// --- Loop Infinito ---
void loop(){

  if(!client.connected()){
    reconnect();
  }
  client.loop();
  
  // Verifica novos cartões
  if ( ! mfrc522.PICC_IsNewCardPresent()){
    return;
  }
  // Seleciona um dos cartões
  if ( ! mfrc522.PICC_ReadCardSerial()){
    return;
  }

  varredura();
  
  conteudo.toUpperCase();
  a = conteudo.substring(1);
  Serial.println();
  Serial.println(a);

  if(a != conteudo.substring(1)){
    Serial.println("Um novo cartão foi detectado: ");
    Serial.println(a);
    Serial.println();
    varredura();
    
  } else{
      Serial.println("ID repetida");
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();

      rfidLeitura = "{ \"UID\": \"" + a + "\"}";
      rfidLeitura.toCharArray(data, (rfidLeitura.length() + 1));
      
      client.publish(your_topic, data, true);
      //client.publish(your_topic, String(a).c_str(), true);
    }
}

void varredura(){
  // Mostra UID na serial
  Serial.print("UID da tag :");
  conteudo= "";
  byte letra;
  
  for (byte i = 0; i < mfrc522.uid.size; i++){
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }  
}

void setup_wifi(){
  delay(10);
  //inicio da conexão com a rede wifi
  Serial.println();
  Serial.print("Conectando: ");
  Serial.println(wifi_ssid) ;

  WiFi.begin(wifi_ssid, wifi_password);

  //enquanto não estiver conectado imprime os pontinhos
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado ao Wifi");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect(){
  while(!client.connected()){
    Serial.print("Tentando conectar o MQTT...");
    if(client.connect("ESP32")){
      Serial.println();
      Serial.println("Conectado");
    } else{
        Serial.print("Falha, rc=");
        Serial.print(client.state());
        Serial.println("Próxima tentativa em 5 segundos");
        delay(5000);
      }
  }  
}

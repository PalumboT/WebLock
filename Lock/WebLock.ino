  /*
    *  ArduiLock v1.2
    *  Timothée Palumbo
  */
  
  #include <SPI.h>
  #include <Ethernet.h>
  #include <SoftwareSerial.h>
  
  /*
    *  Variable pour l'utilisation du RFID
  */
  SoftwareSerial   RFID(3, 2);
  char             inData[12];                                                   // Tableau qui contiendra la valeur de la clé
  char             inChar;                                                       // Variable qui contiendra la valeur entrante
  byte             index = 0;                                                    // Index qui stock le compte de caractère de la valeur de la clé
  int              i = 0;                                                        // Permettera de boucler pour récupérer les données du tableau
  String           chaine;                                                       // Contiendra l'entier de la clé après traitement
  
  /*
    * Variable pour le côté Ethernet du programme
  */
  
  byte             mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };               // Adresse MAC de la carte
  IPAddress        server(192,168,0,205);                                        // Adresse IP du serveur
  IPAddress        ip(192,168,0,206);                                            // Adresse IP de la carte
  IPAddress        ddns(192,168,0,1);                                            // Serveur DNS pour la carte
  EthernetClient   client;                                                       // Déclaration du client ethernet
  
  /*
    * Variable pour le status de l'application
  */
  
  bool             readyToSend = 0;                                              // Variable de status pour l'application
  
  /*
    * Définition des ports de sortie pour les leds
  */
  
  #define LED_GREEN 8
  #define LED_RED 7
  
  /*
    * Initialisation de l'application
  */
  
  void setup()
  {
    RFID.begin(9600);                                                            // Initialisation du lecteur RFID
    Serial.begin(9600);                                                          // Inistalisation de la connexion au PC
  
    pinMode(LED_GREEN, OUTPUT);                                                  // Configuration des ports leds en sorties
    pinMode(LED_RED, OUTPUT);                                                    //
    
    if (Ethernet.begin(mac) == 0) {                                              // Début de la connexion ethernet
      Serial.println("Failed to configure Ethernet using DHCP");                 // Affichage d'un message si la configuration par DHCP a échoué
      Ethernet.begin(mac, ip, ddns, ddns);                                       // Configuration d'IP statique 
    }
    delay(2000);                                                                 // Ajout d'un délais pour laisser un temps d'initialisation à la carte ethernet
    Serial.println("Ready !");                                                   // Affiche un message indiquant que la carte est prête
    Serial.println(Ethernet.localIP());                                          // Affiche l'adresse IP de la carte
  
  }
  
  /*
    * Boucle principal
  */
   
  void loop()
  {  
    if (readyToSend == 1)                                                        // Si les données sont prêtes à être envoyées au serveur pour validation
    {
      httpRequest();                                                             // Création et execution de la requète
      if (client.connected())                                                    // Si une connexion est établie avec le serveur
      {
        if(client.find("200"))                                                    // Si le serveur à retourner un "ok" pour valider la connexion
        {
          digitalWrite(LED_GREEN, HIGH);                                         // Allumage de la led verte
          Serial.println("Unlock");                                              // Affichage d'un message pour indiquer que la porte est ouverte
          delay(4000);
          resetProcess();
        }
        else                                                                     // Si la réponse ne contient pas le OK
        {
          digitalWrite(LED_RED, HIGH);                                           // Allumage de la led rouge
          Serial.println("Keep lock");                                           // Affichage d'un message pour indiquer que la porte restera fermée
          delay(4000);
          resetProcess();
        }
        
      }  
    }
    else                                                                         // Sinon, récupération des valeurs du lecteur RFID
    {
      readRFID();  
    }
  }
  
  /*
    * Fonction de récupération des données du lecteur RFID
  */
  
  void readRFID(){
  
    if (RFID.available() > 0)                                                    // Si le lecteur RFID reçoit des données
    {
      inChar = RFID.read();                                                      // Récupération de la valeur du lecteur 
      if(String(inChar, DEC) == "2")                                             // Si le caractère détecté est un début de valeur de clé (2 représente le début, et 3 la fin en décimal)
      {
        index = 0;                                                               // Cette partie permet de prévenir si la lecture d'une clé est interrompu
        chaine = "";                                                             // Si une nouvelle capture commence, il revient à zero et n'envoi pas de chaîne à moitier capturée
        Serial.println("Begin capture code"); 
      }
      else if (String(inChar, DEC) == "3")                                       // Si le caractère détecté est une fin de valeur de clé
      {
        for(i = 0; i < index; i++){                                              // Récupération des valeurs dans le tableau et création d'une chaîne
          chaine = chaine + String(inData[i]);                     
        }
        readyToSend = 1;                                                         // Changement du status de l'application pour l'envoi de la chaîne pour validation
        Serial.println("End capture code");                                      
      } 
      else {                                                                     // Si ce n'est ni un début ni une fin de chaîne
        if(index < sizeof(inData)){                                              // Protège l'application en cas de bug en évitant un dépassement dans le tableau
          inData[index] = inChar;                                                // Stockage de la valeur dans le tableau qui contiendra la chaîne de la clé
        }
        index++;                                                                 // Incrémentation du compteur de valeur
      } 
    }  
  }
  
  /*
    * Fonction de création et d'envoi de la requète HTTP
  */
  
  void httpRequest() {
    // Stop all the current connection if there remains
    client.stop();                                                               
    Serial.println("Connecting to server...");         
    // Connect to server. If the connection was successful    
    if (client.connect(server, 80)) {                                            
      Serial.println("Connected, send key " + chaine);                           // Affichage d'un message pour indiquer que la connexion est réussie
      client.println("GET /index.php?key=" + chaine + " HTTP/1.1");              // Création de la requète HTTP avec la valeur de la chaîne récupérée
      client.println("Host: 192.168.0.205");                                     // Configuration de l'hôte pour la requète
      client.println("User-Agent: arduino-ethernet");                            // Indique l'user-agent utilisé
      client.println("Connection: close");                                       // Fermeture de la connexion après réception de la réponse
      client.println();                                                           
    }
  }
  
  /*
    * Après avoir reçu une réponse du serveur,
    * cette fonction est appelé pour remettre l'application à son état initial
  */
  
  void resetProcess(){
    index = 0;                                                                   // Mise à zero du compteur
    readyToSend = 0;                                                             // Changement du status de l'application, qui n'est plus prête à renvoyer une requète
    client.stop();                                                               // Suppression de la connexion au serveur
    chaine = "";                                                                 // Remise de la chaine qui contient la valeur de la clé à zero
    delay(1000);                                                                 // Ajout d'un delais
    digitalWrite(LED_RED, LOW);                                                  // Extinction des leds
    digitalWrite(LED_GREEN, LOW);                                                // Extinction des leds
    RFID.flush();                                                                // Vide le buffer serial du lecteur RFID pour prévenir une rééxecution d'une requète HTTP 
  }

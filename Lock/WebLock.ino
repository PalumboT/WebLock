  
  /***************************************************************************
  *  Timothée Palumbo (https://github.com/PalumboT)                          *
  *                                                                          *
  *  DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE                             *
  *             Version 2, December 2004                                     *
  *  Everyone is permitted to copy and distribute verbatim or modified       *
  *  copies of this source code, and changing it is allowed.                 *
  *                                                                          *
  *  Based on a work at https://github.com/PalumboT/WebLock.                 *
  ****************************************************************************/
  
  #include <SPI.h>
  #include <Ethernet.h>
  #include <SD.h>
  #include <SoftwareSerial.h>
  #include <TextFinder.h>
  
  /***************************
   *  RFID variables         *
   ***************************/
 
  SoftwareSerial   RFID(2, 3);    // Set the pin of the RFID reader to emule a Serial
  char             RFIDKey[13];   // Table that contain the bytes of the key
  
  /***************************
   *  Ethernet variables     *
   ***************************/
  
  byte             mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };    // MAC address of the ethernet shield
  IPAddress        server(XXX,XXX,XXX,XXX);                           // IP address of the server
  //char           server[] = "www.exemple.com";                      // Address of the server (Domain name)
  IPAddress        ip(XXX,XXX,XXX,XXX);                               // IP address of the ethernet shield
  IPAddress        ddns(XXX,XXX,XXX,XXX);                             // IP address of the DNS server
  EthernetClient   client;                                            // Creating an ethernet client for queries
  TextFinder       finder(client, 1);                                 // Use to retrieve data when the server send a response
  
  /***************************
   *  Application status     *
   ***************************/
  
  boolean          readyToSend = 0;                                   // Will be used as variable application status
  byte             fileNumber = 0;                                    // Indicate with files to use for the offline mode
  
  /************************************************
   *  Define the port number for the components   *
   ***********************************************/
  
  #define         LED_GREEN   6                                       
  #define         LED_RED     5
  #define         BUZZER      9
  #define         STRIKE      8
  
  /***********************************************
   *  SD Card                                    *
   ***********************************************/
  
  File            file;                                               // Use to open differents files

  /***********************************************
   *  Shared                                     *
   ***********************************************/

  char            inChar;                                            // Hold the incoming byte from different sources (RFID, SDCard, etc.) 
  byte            i = 0;                                             // Use to loop in different place of the application
   
  /************************************************************************************
   *  Application activation part. Comment the line to disabling fonctionalities      *
   ************************************************************************************/

   #define        activeBip;                                         // You can active the song
   #define        activeOffline;                                     // You can active the offline mode
   #define        activeDebug;                                       // You can active debug on the serial
    
  /***********************************************
   *  For debug purpose                          *
   ***********************************************/
   
   #ifdef activeDebug
     #define        cleanDebug(str)  Serial.println(F(str))   
     #define        debugLn(str)     Serial.println(str)
     #define        debug(str)       Serial.print(str)
   #else
     #define        cleanDebug   
     #define        debugLn
     #define        debug
   #endif
  
  /***********************************************
   *  Initialisation of the application
   **********************************************/
  
  void setup()
  {
    
    ///////////////////////////////////
    // Init the connection to the PC //
    ///////////////////////////////////
    
    #ifdef debug
      Serial.begin(9600);              
    #endif
    
    ///////////////////////////////////////
    // Initialization of the RFID reader //
    ///////////////////////////////////////
    
    RFID.begin(9600);     
    
    //////////////////////////////////////////////////////
    // Configuring leds ports and strike port as output //
    //////////////////////////////////////////////////////
    
    pinMode(LED_GREEN, OUTPUT);        
    pinMode(LED_RED, OUTPUT);
    pinMode(STRIKE, OUTPUT);    

    //////////////////////////////////////
    // Start of the ethernet connection //
    //////////////////////////////////////

    Ethernet.begin(mac, ip, ddns); 
    
    ///////////////////////////////////
    // Initialisation of the SD Card //
    ///////////////////////////////////
    
    #ifdef activeOffline
      if (!SD.begin(4)) {                                         
        cleanDebug("initialization failed!");
      }
      else cleanDebug("initialization done.");
    #endif
    
  }
  
  /***********************************************
   *  Main loop
   **********************************************/
   
  void loop()
  {
    
    //////////////////////////////////
    // Update the offline keys file //
    //////////////////////////////////
    
    // If the number of millisconds since 
    // the Arduino began runningis a modulo 
    // of 1 hour (3600000 milliseconds), 
    // update the offline file
    
    #ifdef activeOffline
      if((millis() % 3600000) == 0){
        getOfflineFiles();
      }
    #endif
    
    ///////////////////
    // Process a key //
    ///////////////////
    
    if (readyToSend == 1)                                                        
    { 
      
      ///////////////////
      // Send request  //
      ///////////////////
      
      // Connect to server. If the connection was successful    
      if (client.connect(server, 80)) {     
        // Create the HTTP request with the value of the retrieved string
        client.print("GET /index.php?key=");      
        // Retrieve the values of the key in is buffer and write it in the client      
        for(i = 0; i < (sizeof(RFIDKey) - 1); i++){  
           client.print(RFIDKey[i]);                     
        }    
        client.println(" HTTP/1.1");        
        // Host configuration for the query  
        client.print("Host: ");    
        client.println(server);       
        // Specifies the user-agent used      
        client.println("User-Agent: arduino-ethernet");
        // Closing the connection after receiving the response
        client.println("Connection: close");
        client.println();    
      }
      
      ///////////////////////////////////////////////////////
      // Pass in the offline mode if host is not reachable //
      ///////////////////////////////////////////////////////

      #ifdef activeOffline
      else{
        offlineMode();
      }
      #endif
       
      ////////////////////////////////////////
      // Process the response of the server //
      ////////////////////////////////////////
      
      if (client.connected())                                                    
      {
        cleanDebug("Online Mode");
        // Wait the response from the server
        while(client.connected() && !client.available()) delay(1);
        //Go to the http code position
        finder.find("HTTP/1.1 ");
        // If the http code is 200, open the door
        if(finder.findUntil("200", " ")) {
          accessAllowed();
        } 
        // Else keep it close
        else{
          accessDenied();
        }
      } 
    }
    
    ////////////////////////////////////////
    // Read the data from the RFID reader //
    ////////////////////////////////////////
    
    else                                                                         
    {
      readRFID();  
    }
  }
  
  /********************************************************
   * Get data from the RFID reader                        *
   ********************************************************/
  
  void readRFID(){
 
    /////////////////////////////////////////////
    // Read the data if there is incoming data //
    /////////////////////////////////////////////
    
    if (RFID.available() > 0)                                                    
    {
      // Get the data 
      inChar = RFID.read();        
      // if the character detected is the beginning of a key (2 represents the beginning and 3 the end in decimal)     
      if(inChar == 2)                                             
      {
        // This part prevent against reading interruption
        i = 0;
      }
      // If the character is an end of key
      else if (inChar == 3)                                       
      {
        // Changing the status of the application to begin the process of sending the key to the server
        readyToSend = 1;
        // Do a little bip to tell the user that the key has been read
        #ifdef bip
          tone(BUZZER, 196,250);
          delay(325);
          noTone(BUZZER);    
        #endif    
      } 
      // If it is a simple character
      else {       
        // Protects the application by avoiding an overflow bug in the table       
        if(i < sizeof(RFIDKey)){
          // Storing the value in the buffer that contains the key
          RFIDKey[i] = inChar; 
        }else
        {
          i = 0;
        }
        i++; 
      } 
    } 
  }
  
 /******************************************************************************
  * Offline mode                                                              *
  *****************************************************************************/
  
  #ifdef activeOffline
  void offlineMode(){
    cleanDebug("Offline Mode");
    
    ///////////////////////
    // Open the key file //
    ///////////////////////
    
    if (SD.exists("KEYS2.TXT")) {
      file = SD.open("KEYS2.TXT");
    }
    else {
      file = SD.open("KEYS1.TXT");
    }
    // Create a text finder for the file with a 1 seconde timeout
    TextFinder finderSD(file, 1);
    
    ////////////////////////////////////
    // Find the key and is permission //
    ////////////////////////////////////
    
    if (file) {
      // If the key is found
      if (finderSD.find(RFIDKey)){
          // Get the permission number (the file.read() is use to jump the separator)
          // Key format in the keys file : IDKEY PermissionNumber. Exemple : 02005159B6C5 2
          file.read(); 
          inChar = file.peek();
      }
      // Close the file
      file.close();
      
      ////////////////////////////////////////////////////
      // Test the permission and open, or not, the door //
      ////////////////////////////////////////////////////

      if (inChar == '1') accessAllowed();
      else accessDenied();
    }else{
      accessDenied();
    }
  }
  #endif
  
 /**************************
  * Get the config file    *
  **************************/
  
  #ifdef activeOffline
  void getOfflineFiles(){
    
    cleanDebug("Update offline files");
    // Kill old client connection
    client.stop();
    
    ///////////////////////////////////////////////////////   
    // Open the file that contain the last modified date //
    ///////////////////////////////////////////////////////
    
    if (SD.exists("KEYS2.TXT")) {
      cleanDebug("Files 1");
      fileNumber = 1;
      file = SD.open("DATE2.TXT");
    }
    else {
      cleanDebug("Files 2");
      fileNumber = 2;
      file = SD.open("DATE1.TXT");
    }
    
    ///////////////////////////////////////////////////////   
    // Get the file with the last modified date          //
    ///////////////////////////////////////////////////////
    
    if (client.connect(server, 80)) {     
      // Create the HTTP request with the value of the retrieved string
      client.println("GET /files/keys.txt HTTP/1.1");      
      // Host configuration for the query  
      client.print("Host: ");    
      client.println(server);          
      // Specifies the user-agent used      
      client.println("User-Agent: arduino-ethernet");
      // Set the last modified date that is present in the OFFLDATE.TEXT file
      client.print("If-Modified-Since: ");
      // Set the date
      if(file){
        while(file.available()){
          inChar = file.read();
          client.print(inChar);
        }
        // Close the open date file
        file.close();
      }
      client.println("");
      // Closing the connection after receiving the response
      client.println("Connection: close");
      client.println();                                                           
    }  
    
    /////////////////////////////
    // Get and save the date   //
    /////////////////////////////
    
    // wait response from the server
    while(client.connected() && !client.available()) delay(1);
    //Go to the http code position
    finder.find("HTTP/1.1 ");
    // If the http code is 200, there is new key to save
    if(finder.findUntil("200", " ")){
      cleanDebug("Need update");
      // Go to the position of the data in the data stream
      finder.find("Last-Modified: ");
      // Open the correct file
      if(fileNumber == 1) { file = SD.open("DATE1.TXT", FILE_WRITE); }
      else { file = SD.open("DATE2.TXT", FILE_WRITE); }
      // Get and save the data in the file
      i = 0;
      if(file){
        while(client.available()) { 
          if(i < 30) inChar = client.read(); file.print(inChar);
          if(i == 30) break;
          i++;
        }
      }else{
        cleanDebug("Fail save date");
      }
      // Close the file
      file.close();
      
      /////////////////////////////
      // Get and save the keys   //
      /////////////////////////////
      
      // Go to content in the server response
      finder.find("\n\r");
      //Open the correct file
      if (fileNumber == 1){ file = SD.open("KEYS1.TXT", FILE_WRITE); }
      else { file = SD.open("KEYS2.TXT", FILE_WRITE); }
      // If file exist
      if (file) {
        // write until the end of the server response content
        while(client.available()) { 
          inChar = client.read();
          file.print(inChar);
        }
      // Else display an error message
      }else{
        cleanDebug("Fail save keys");
      }
      // Close the file
      file.close();
      
      //////////////////////
      // Delete old files //
      //////////////////////
      
      if(fileNumber == 1){
        cleanDebug("Deleting files 2");
        // Remove the old key file
        SD.remove("KEYS2.TXT");
        // Remove the old date file
        SD.remove("DATE2.TXT");
      }else{
        cleanDebug("Deleting files ");
        // Remove the old key file
        SD.remove("KEYS1.TXT");
        // Remove the old date file
        SD.remove("DATE1.TXT");
      }
    }else{
      cleanDebug("No update to do");
    }
    cleanDebug("End");
  }
  #endif
  
 /**************************
  * Open the door          *
  **************************/
  
  void accessAllowed(){
    // Power on the green led
    digitalWrite(LED_GREEN, HIGH); 
    // Open the door with the electrical strike
    digitalWrite(STRIKE, HIGH);   
    // Display a message in the serial port        
    cleanDebug("Unlock"); 
    // Do a long bip
    #ifdef activeBip
      tone(BUZZER, 196, 2000);
      delay(2600);
      noTone(BUZZER);
    #endif
    // Delay of 2 secondes          
    delay(2000);
    // Reset the application
    resetProcess();
  }
  
 /***************************
  * keep the door close     *
  ***************************/
  
  void accessDenied(){
    // Power on the red led
    digitalWrite(LED_RED, HIGH);
    // Display a message in the serial port        
    cleanDebug("Keep lock"); 
    // Do 5 error bips
    #ifdef activeBip
      for (int errorNote = 0; errorNote < 6; errorNote++) {
        tone(BUZZER, 196,250);
        delay(325);
        noTone(BUZZER);
      }
    #endif
    // Delay of 2 secondes            
    delay(2000);
    // Reset the application
    resetProcess();
  }
  
  /******************************************************************************
  * After receiving a response from the server,                               *
  * this function is called to restore the application to the initial state   *
  *****************************************************************************/
  
  void resetProcess(){ 
    // Change the status of the application, which is now not ready to send a request    
    readyToSend = 0;
    // Remove the server connection
    client.stop();
    // Adding a delay
    delay(1000);         
    // Power of the leds    
    digitalWrite(LED_RED, LOW);                                                 
    digitalWrite(LED_GREEN, LOW);  
    digitalWrite(STRIKE, LOW);
    // Empty the buffer of the RFID reader    
    RFID.flush();   
  }

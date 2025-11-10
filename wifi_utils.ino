#include <WiFi.h> // https://www.arduino.cc/en/Reference/WiFi
#include "wifi_utils.h"

 #define USE_SERIAL Serial

/*--------------------------------------------------------------------------*/
void wificonnect_multi(String hostname){
int nbtry = 0; // Nb of try to connect

 WiFiMulti wm; // Creates an instance of the WiFiMulti class
// Attention ! PAS arrivé à sortir l'instance de la fonction => heap error ! Why ????
 wm.addAP("OG", "ukva0653");
 wm.addAP("iPhone", "ibrahima");
 wm.addAP("GMAP", "vijx47050");
 wm.addAP("IOT","iotmiage");
 wm.addAP("Bbox-EA6814CF","Hvtx6bGfD4Sn7PCs2r");

 //WiFi.mode(WIFI_OFF);
 WiFi.mode(WIFI_STA); // Set WiFi to station mode
 // delete old config
 // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
 WiFi.disconnect(true); // Disconnect from an AP if it was previously connected
 // WiFi.persistent(false); // Avoid to store Wifi configuration in Flash

 // Define hostname => in C str ! not C++
 WiFi.setHostname(hostname.c_str());

 while(wm.run() != WL_CONNECTED && (nbtry < WiFiMaxTry)) {
 USE_SERIAL.printf("\nAttempting %d to connect AP", nbtry);
 delay(SaveDisconnectTime);
 USE_SERIAL.print(".");
 nbtry++;
 }

 if(wm.run() == WL_CONNECTED) {
    USE_SERIAL.printf("\nWifi Multi connected on %s !", WiFi.SSID());
    wifi_printstatus(0);
 }else{ 
    ESP.restart();
 }
}

 /*--------------------------------------------------------------------------*/
String translateEncryptionType(wifi_auth_mode_t encryptionType) {
 // encryptiontype to string
// cf https://www.arduino.cc/en/Reference/WiFiEncryptionType
 switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
         return "Open";
    case (WIFI_AUTH_WEP):
         return "WEP";
    case (WIFI_AUTH_WPA_PSK):
        return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
        return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
        return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
        return "WPA2_ENTERPRISE";
    }
}

/*--------------------------------------------------------------------------*/
void wifi_printstatus(int C){
/* print the status of the connected wifi in two ways ! */

if (C){
// Use Pure C => array of chars
   USE_SERIAL.printf("WiFi Status : \n");
   USE_SERIAL.printf("\tIP address : %s\n", WiFi.localIP().toString().c_str());
   USE_SERIAL.printf("\tMAC address : %s\n", WiFi.macAddress().c_str());
   USE_SERIAL.printf("\tSSID : %s\n", WiFi.SSID());
   USE_SERIAL.printf("\tReceived Signal Strength Indication : %ld dBm\n",WiFi.RSSI());
   USE_SERIAL.printf("\tReceived Signal Strength Indication : %ld %\n",
   constrain(2 * (WiFi.RSSI() + 100), 0, 100));
   USE_SERIAL.printf("\tBSSID : %s\n", WiFi.BSSIDstr().c_str());
   USE_SERIAL.printf("\tEncryption type : %s\n", translateEncryptionType(WiFi.encryptionType(0)));
}
   else {
   // Use of C++ => String !
   String s = "WiFi Status : \n";
   //s += "\t#" + String() + "\n";
   s += "\tIP address : " + WiFi.localIP().toString() + "\n";
   s += "\tMAC address : " + String(WiFi.macAddress()) + "\n";
   s += "\tSSID : " + String(WiFi.SSID()) + "\n";
   s += "\tReceived Sig Strength Indication : " + String(WiFi.RSSI()) + " dBm\n";
   s += "\tReceived Sig Strength Indication : " +
   String(constrain(2 * (WiFi.RSSI() + 100), 0, 100)) + " %\n";
   s += "\tBSSID : " + String(WiFi.BSSIDstr()) + "\n";
   s += "\tEncryption type : " + translateEncryptionType(WiFi.encryptionType(0))+ "\n";
   USE_SERIAL.print(s);
   }
}



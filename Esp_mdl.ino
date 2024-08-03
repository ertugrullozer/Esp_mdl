#include <SoftwareSerial.h>   
#include "DHT.h" // Bu örnekte Adafruit'in DHT11 kütüphanesini kullanacağız
                 // Arduino IDE ana menüsünden Araçlar > Kütüphaneleri Yönet... ekranını açın
                 // DHT sensor library yazıp aratın
                 // Adafruit tarafından hazırlanmış kütüphaneyi tüm gereklilikleri ile
                 // birlikte kurun
String agAdi = "Ağ Adı";                 //Ağımızın adını buraya yazıyoruz.    
String agSifresi = "Şifre";           //Ağımızın şifresini buraya yazıyoruz.

int rxPin = 10;                                               //ESP8266 RX pini
int txPin = 11;                                               //ESP8266 TX pini
int dht11Pin = 2;

String ip = "184.106.153.149";                                //Thingspeak ip adresi
float sicaklik, nem;

#define DHTTYPE DHT11     // Kütüphaneden sensör tipini seçiyoruz
DHT dht(dht11Pin, DHTTYPE); // Sensör pinimizi kütüphaneye tanıtıyoruz

SoftwareSerial esp(rxPin, txPin);                             //Seri haberleşme pin ayarlarını yapıyoruz.

void setup() {

dht.begin();            // Ölçüm başlasın
Serial.begin(9600);  //Seri port ile haberleşmemizi başlatıyoruz.
  Serial.println("Started");
  esp.begin(115200);                                          //ESP8266 ile seri haberleşmeyi başlatıyoruz.
  esp.println("AT");                                          //AT komutu ile modül kontrolünü yapıyoruz.
  Serial.println("AT Yollandı");
  while(!esp.find("OK")){                                     //Modül hazır olana kadar bekliyoruz.
    esp.println("AT");
    Serial.println("ESP8266 Bulunamadı.");
  }
  Serial.println("OK Komutu Alındı");
  esp.println("AT+CWMODE=1");                                 //ESP8266 modülünü client olarak ayarlıyoruz.
  while(!esp.find("OK")){                                     //Ayar yapılana kadar bekliyoruz.
    esp.println("AT+CWMODE=1");
    Serial.println("Ayar Yapılıyor....");
  }
  Serial.println("Client olarak ayarlandı");
  Serial.println("Aga Baglaniliyor...");
  esp.println("AT+CWJAP=\""+agAdi+"\",\""+agSifresi+"\"");    //Ağımıza bağlanıyoruz.
  while(!esp.find("OK"));                                     //Ağa bağlanana kadar bekliyoruz.
  Serial.println("Aga Baglandi.");
  delay(1000);


}

void loop() {
    esp.println("AT+CIPSTART=\"TCP\",\""+ip+"\",80");           //Thingspeak'e bağlanıyoruz.
  if(esp.find("Error")){                                      //Bağlantı hatası kontrolü yapıyoruz.
    Serial.println("AT+CIPSTART Error");
  }


  nem = dht.readHumidity();         // Nem ölçülüyor
  sicaklik = dht.readTemperature(); // Sıcaklık ölçülüyor

 String veri = "GET https://api.thingspeak.com/update?api_key=3TRG8D2UFN4VUDPS";   //Thingspeak komutu. Key kısmına kendi api keyimizi yazıyoruz.                                   //Göndereceğimiz sıcaklık değişkeni
  veri += "&field1=";
  veri += String(sicaklik);
  veri += "&field2=";
  veri += String(nem);                                        //Göndereceğimiz nem değişkeni
  veri += "\r\n\r\n"; 
  esp.print("AT+CIPSEND=");                                   //ESP'ye göndereceğimiz veri uzunluğunu veriyoruz.
  esp.println(veri.length()+2);
  delay(2000);
 if(esp.find(">")){                                          //ESP8266 hazır olduğunda içindeki komutlar çalışıyor.
    esp.print(veri);                                          //Veriyi gönderiyoruz.
    Serial.println(veri);
    Serial.println("Veri gonderildi.");
    delay(1000);
  }
  Serial.println("Baglantı Kapatildi.");
  esp.println("AT+CIPCLOSE");                                //Bağlantıyı kapatıyoruz
  delay(1000);                                               //Yeni veri gönderimi için 1 dakika bekliyoruz.

  if (isnan(nem) || isnan(sicaklik)) {    // Hata kontrolü yapalım, varsa uyaralım
    Serial.println("SENSÖR OKUMA HATASI! BAĞLANTILARI KONTROL EDİN");
    return;
  }
}
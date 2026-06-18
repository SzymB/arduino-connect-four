#include <FastLED.h>

#define NUM_COLS 7
#define NUM_ROWS 6
#define NUM_LEDS NUM_COLS*NUM_ROWS
#define SCREEN_PIN 3
#define RESET_PIN 11
#define PIN_AKT_GR 12
#define REMIS_PIN 14

CRGB leds[NUM_LEDS];
const int przPIN[] = {4,5,6,7,8,9,10};
const uint32_t kolory[] = {0xa0a0a0,0xd0d0d0,0xff0000,0x00ff00,0x0000ff,0xffff00,0xff00ff};
const uint32_t kolGraczy[] = {0xff0000,0x0000ff};
const int pinGrWygr[] = {13,2}; // piny do wygranej
const int limitRuchowRemis = 42;
const int dlugosc = 7;
//const int dlugosc = sizeof(przPIN) / sizeof(przPIN[0]);
int wsk = 0;
int aktGr = 0;
int czyGra = 1;
int wykRuchy = 0; // wykonane ruchy licznik
const uint32_t czarny = 0x000000;
int pola[NUM_COLS][NUM_ROWS];

void resetujLED(){
  for(int i=0; i < NUM_LEDS; i++){
    leds[i] = czarny;
  }
}
void resetujPola(){
  for(int x = 0; x < NUM_COLS; x++){
    for(int y=0; y < NUM_ROWS; y++){
      pola[x][y] = 2; // gracze to 0 i 1
    }
  }
}

void setup() {
  Serial.begin(9600);
  for(int i = 0; i < dlugosc; i++){
    pinMode(przPIN[i], INPUT);
  }
  for(int i=0; i < 2; i++){
    pinMode(pinGrWygr[i], OUTPUT);
  }
  pinMode(RESET_PIN, INPUT);
  pinMode(PIN_AKT_GR, OUTPUT);
  pinMode(REMIS_PIN, OUTPUT);

  FastLED.addLeds<WS2812B, SCREEN_PIN, GRB>(leds, NUM_LEDS);
  FastLED.show();
  resetujLED();
  resetujPola();

  //uint32_t ziel = 0x00ff00;
  //uint32_t nieb = 0x0000ff;
  //uint32_t czer = 0xff0000;
  //leds[0] = ziel;
  //leds[9] = nieb;
  //leds[41] = czer;
  FastLED.delay(60);
  //Serial.println("po conf");
}

void PiszPixel(int x, int y, uint32_t kolor){
  leds[41-(6-x)-7*y] = kolor;
}
int SprLicz(int x, int y, int kx, int ky, int gracz){
  // x,y - współrzędne
  // kx,ky - kierunek sprawdzenia: 0 lub 1 lub -1
  // zwraca: znal = liczbę kolejnych elementów danego gracza w tym kierunku
  int znal = 0;
  int aktx = x + kx; // aktualne
  int akty = y + ky;
  for(int i = 0; i < 7; i++){
    if(aktx < 0 || aktx >= NUM_COLS || akty < 0 || akty >= NUM_ROWS){
      return znal; //kończymy, jesteśmy poza mapą
    }
    if(pola[aktx][akty] == gracz){
      znal += 1;
    }
    else{
      return znal; //kończymy, jest to inne pole
    }
    aktx = aktx + kx; //następny element
    akty = akty + ky;
  }
  return znal;
}

void loop(){
  //digitalWrite(REMIS_PIN, HIGH);

  if(digitalRead(RESET_PIN) == HIGH){
    resetujLED();
    resetujPola(); //zmiennej
    aktGr = 0;
    digitalWrite(PIN_AKT_GR, LOW);
    digitalWrite(REMIS_PIN, LOW);
    for(int i=0; i < 2; i++){
      digitalWrite(pinGrWygr[i], LOW); // czyścimy lampki wygranych
    }
    czyGra = 1; // ustawiamy
    FastLED.delay(60); // odświeżanie
    wykRuchy = 0;
    //Serial.println("Reset pin");
    delay(500);
  }
  if(!czyGra){
    return; // nie badamy przycisków
  }
  int y_akt; // aktualna pozycja y dodanego elementu;
  for(int i = 0; i < dlugosc; i++){
    if(digitalRead(przPIN[i]) == HIGH){ //write next pixel on screen on button press
      int czyZmieniono = 0;
      for(int j=0; j < NUM_ROWS; j++){
        if(pola[i][j] == 2){ // jeżeli jest puste
          pola[i][j] = aktGr; // do zmiennej
          PiszPixel(i,j,kolGraczy[aktGr]); // na ekran
          czyZmieniono = 1;
          wykRuchy = wykRuchy + 1;
          y_akt = j; // zapisujemy
          break;
        }
      }
      if(!czyZmieniono){ // nie zmieniono = nie ma miejsca = nielegalny ruch
        czyGra = 0;
        if(aktGr == 0){
          //Serial.println("Gracz 0 przegral");
        }
        if(aktGr == 1){
          //Serial.println("Gracz 1 przegral");
        }
        digitalWrite(pinGrWygr[(aktGr+1)%2], HIGH); // wygrywa gracz przeciwny

        return; // kończymy tę loop()
      }
      if(1 + SprLicz(i,y_akt,-1,0,aktGr) + SprLicz(i,y_akt,1,0,aktGr) >= 4 ){
        // mamy 4ry poziome 
        digitalWrite(pinGrWygr[aktGr], HIGH);
        czyGra = 0; //kończymy
        FastLED.delay(60); // odświeżamy LED przed RETURN
        return;
      }
      if(1 + SprLicz(i,y_akt,0,-1,aktGr) + SprLicz(i,y_akt,0,1,aktGr) >= 4 ){
        // mamy 4ry pionowe 
        digitalWrite(pinGrWygr[aktGr], HIGH);
        czyGra = 0; //kończymy
        FastLED.delay(60); // odświeżamy LED przed RETURN
        return;
      }
      if(1 + SprLicz(i,y_akt,-1,1,aktGr) + SprLicz(i,y_akt,1,-1,aktGr) >= 4 ){
        // mamy 4ry skos malejący
        digitalWrite(pinGrWygr[aktGr], HIGH);
        czyGra = 0; //kończymy
        FastLED.delay(60); // odświeżamy LED przed RETURN
        return;
      }
      if(1 + SprLicz(i,y_akt,-1,-1,aktGr) + SprLicz(i,y_akt,1,1,aktGr) >= 4 ){
        // mamy 4ry skos rosnący
        digitalWrite(pinGrWygr[aktGr], HIGH);
        czyGra = 0; //kończymy
        FastLED.delay(60); // odświeżamy LED przed RETURN
        return;
      }

      
      //Serial.println("klik");
      
      FastLED.delay(60);
      if(wykRuchy == limitRuchowRemis ){
        digitalWrite(REMIS_PIN, HIGH);
        czyGra = 0;
        return;
      }
      aktGr = (aktGr + 1) % 2;
      digitalWrite(PIN_AKT_GR,aktGr);
      delay(500);
    }
  }
  


  //FastLED.delay(60);
  delay(10);

}

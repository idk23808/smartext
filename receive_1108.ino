#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//NRF24와 연결할 핀을 연결할 코드이다.
//9번 핀은 CE, 10번 핀은 CSN, 11번 핀은 MOSI, 12번 핀은 MISO, 13번 핀은 SCK
//NRF 모듈이 외부 간섭으로 부터 신호가 불안정할 경우 VCC, GND에 10uF 연결

#define CEPIN 9
#define CSNPIN 10

//ADDRESS는 NRF 모듈의 통신 주소이다. 송 수신 모두 동일한 값이어야함.
#define ADDRESS 0xF0F0F0E1LL

RF24 radio(CEPIN, CSNPIN); // CE, CSN 핀 설정

//화재 감지기로부터 화재 감지 여부를 전달받는 함수
bool RECEIVE_ALERT(float RECEIVE_FREQ, bool SPEAKER_SWITCH){
  bool firedetection = false;   //firedetection 값 초기화
  static unsigned long receive_prevmillis = 0;
  const unsigned long receive_delaymillis = RECEIVE_FREQ;
  static bool receive_state = 0;

  unsigned long receive_currmillis = millis();

  if(SPEAKER_SWITCH && radio.available()){
    if(receive_currmillis - receive_prevmillis >= receive_delaymillis){
      receive_prevmillis = receive_currmillis;
      radio.read(&firedetection, sizeof(firedetection));Serial.println(firedetection);return firedetection;}}else{}
}

//소화기 받침대의 스피커를 작동하는 함수(기본값은 테스트 레벨로)
void SPEAKER(float DELAY_MILIS_SPEAKER, bool ARAM_SWITCH, bool ISTEST = true){
  #define LOW_LEVEL 0
  #define TEST_LEVEL 128
  #define ALERT_LEVEL 255
  #define ARAM_OUTPUT_PIN 5

  static unsigned long previousMillis = 0;
  static int speakerState = LOW_LEVEL;
  const unsigned long delayMillis = DELAY_MILIS_SPEAKER;

  unsigned long currentMillis = millis();
  //테스트 버튼을 누를 경우 스피커 시험 출력
  if (ISTEST && ARAM_SWITCH) { 
    if (currentMillis - previousMillis >= delayMillis) {previousMillis = currentMillis;
      if (speakerState == LOW_LEVEL) {speakerState = TEST_LEVEL;} else {speakerState = LOW_LEVEL;}
      analogWrite(ARAM_OUTPUT_PIN, speakerState);}}
  //실제 상황이 발생할 경우 스피커 최대 출력
  else if (!ISTEST && ARAM_SWITCH){
    if(currentMillis - previousMillis >= delayMillis){
      previousMillis = currentMillis;
      if(speakerState == LOW_LEVEL) {speakerState = ALERT_LEVEL;} else {speakerState = LOW_LEVEL;}
      analogWrite(ARAM_OUTPUT_PIN, speakerState);}}
}


//소화기 받침대의 LED를 작동하는 함수
void BLINK_LED(long DELAY_MILIS_LED, bool LED_SWITCH){
  #define LED_OUTPUT_PIN 6

  const unsigned long led_interval = DELAY_MILIS_LED;
  static long prev_milis = 0;
  const unsigned long curr_milis = millis();
  if(LED_SWITCH == true){
    if ((curr_milis - prev_milis >= led_interval)) {prev_milis = curr_milis; digitalWrite(LED_OUTPUT_PIN, !digitalRead(LED_OUTPUT_PIN));}}else{}}

void setup(){

  //시리얼 모니터에 통신 값을 받아오는 비트레이트 처리 -> 시제품 때 삭제 예정.
  Serial.begin(9600);

  radio.begin();
  radio.openReadingPipe(0, ADDRESS);
  radio.setPALevel(RF24_PA_MIN);
  //모듈을 수신기 상태로 설정한다.
  radio.startListening(); 

  pinMode(ARAM_OUTPUT_PIN, OUTPUT);
  pinMode(LED_OUTPUT_PIN, OUTPUT);
}

void loop(){
  float FREQ = 1000;
  bool BIT_CHECKER = RECEIVE_ALERT(FREQ, true);
  if(BIT_CHECKER == true){BLINK_LED(1000, true);SPEAKER(500, true);}else{}


}

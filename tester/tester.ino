#include <PCD8544.h>
#include <math.h>
PCD8544 lcd;

// 以下是引脚定义，SMALL表示小电阻，BIG表示大电阻，READ表示模拟输入
const byte SMALL_1 = 8;
const byte BIG_1 = 9;
const byte READ_1 = A2;
const byte SMALL_2 = 10;
const byte BIG_2 = 11;
const byte READ_2 = A1;
const byte SMALL_3 = 12;
const byte BIG_3 = 13;
const byte READ_3 = A0;

// 把引脚写在一个数组里面，方便取用，比如说第一组的小电阻端口就是 PORT[0][SMALL]
const byte SMALL = 0;
const byte BIG = 1;
const byte READ = 2;
const byte PORT[3][3] = {
  {SMALL_1, BIG_1, READ_1},
  {SMALL_2, BIG_2, READ_2},
  {SMALL_3, BIG_3, READ_3},
};


const float VCC = 5;
const float R_BIG = 470e3;
const float R_SMALL = 680;

// 端口自身的内阻，似乎用不上
const float R_HIGH = 22;
const float R_LOW = 19;


const int ARRAY_LEN = 100; 


#define DEBUG

#ifdef DEBUG
#define printDebug(x) (printDebug_(x))
#else
#define printDebug(x) ((void)0)
#endif



float adcToVoltage(word adc) {
  return adc / 1023.0 * VCC;
}

float getVoltage(byte port) {
  return adcToVoltage(analogRead(PORT[port][READ]));
}

// 以 interval(单位：ms) 为间隔测量 times 次，然后取平均值
float getAvgVoltage(byte port, word times, word interval) {
  float s = 0;
  for (word i = 0; i < times; ++i) {
    s += getVoltage(port);
    delay(interval);
  }
  return s / times;
}

void printType(const char* str) {
  lcd.setCursor(0, 0);
  lcd.clearLine();
  lcd.print(str);
}

const byte NUM_PREFIXES_SMALL = 4;
const char* const PREFIXES_SMALL[NUM_PREFIXES_SMALL] = {"m", "u", "n", "p"};
const byte NUM_PREFIXES_BIG = 2;
const char* const PREFIXES_BIG[NUM_PREFIXES_BIG] = {"k", "M"};
int __abs(int x){
  return (x < 0 ? -x : x);
}
void printValue(float val, const char* unit) {
  lcd.setCursor(0, 1);
  lcd.clearLine();
  const char* prefix = "";
  if (val < 1) {
    val *= 1000;
    byte i = 0;
    while (val < 1) {
      val *= 1000;
      i += 1;
      if (i == NUM_PREFIXES_SMALL) {
        lcd.print("0 ");
        lcd.print(unit);
        return;
      }
    }
    prefix = PREFIXES_SMALL[i];
  } else if (val >= 1000) {
    val /= 1000;
    byte i = 0;
    while (val >= 1000) {
      val /= 1000;
      i += 1;
      if (i == NUM_PREFIXES_BIG) {
        lcd.print("Inf ");
        lcd.print(unit);
        return;
      }
    }
    prefix = PREFIXES_BIG[i];
  }
  lcd.print(val);
  lcd.print(' ');
  lcd.print(prefix);
  lcd.print(unit);
}

void printStatus(const char* str) {
  lcd.setCursor(0, 5);
  lcd.clearLine();
  lcd.print(str);
}

void printDebug_(const char* str) {
  lcd.setCursor(0, 4);
  lcd.clearLine();
  lcd.print(str);
}
void resetOther(byte port1, byte port2) {
  const byte TABLE[3][3] = {
    {-1, 2, 1},
    {2, -1, 0},
    {1, 0, -1},
  };
  byte port3 = TABLE[port1][port2];
  pinMode(PORT[port3][SMALL], INPUT);
  pinMode(PORT[port3][BIG], INPUT);
  pinMode(PORT[port3][READ], INPUT);
  digitalWrite(PORT[port3][SMALL], LOW);
  digitalWrite(PORT[port3][BIG], LOW);
  digitalWrite(PORT[port3][READ], LOW);
}


void switchToSmallResistor(byte port1, byte port2) {
  pinMode(PORT[port1][SMALL], OUTPUT);
  pinMode(PORT[port1][BIG], INPUT);
  pinMode(PORT[port1][READ], INPUT);
  pinMode(PORT[port2][SMALL], INPUT);
  pinMode(PORT[port2][BIG], INPUT);
  pinMode(PORT[port2][READ], OUTPUT);
  digitalWrite(PORT[port1][SMALL], LOW);
  digitalWrite(PORT[port1][BIG], LOW);
  digitalWrite(PORT[port1][READ], LOW);
  digitalWrite(PORT[port2][SMALL], LOW);
  digitalWrite(PORT[port2][BIG], LOW);
  digitalWrite(PORT[port2][READ], HIGH);
  resetOther(port1, port2);
}

//void switchToBigResistor(byte port1, byte port2) {
//  resetOther(port1, port2);
//  
//  pinMode(PORT[port1][SMALL], OUTPUT);
//  pinMode(PORT[port1][BIG], OUTPUT);
//  pinMode(PORT[port1][READ], OUTPUT);
//  pinMode(PORT[port2][SMALL], INPUT);
//  pinMode(PORT[port2][BIG], INPUT);
//  pinMode(PORT[port2][READ], INPUT);
//  digitalWrite(PORT[port1][SMALL], LOW);
//  digitalWrite(PORT[port1][BIG], LOW);
//  digitalWrite(PORT[port1][READ], LOW);
//  digitalWrite(PORT[port2][SMALL], LOW);
//  digitalWrite(PORT[port2][BIG], LOW);
//  digitalWrite(PORT[port2][READ], LOW);
//  
//  delay(100);
//  
//  pinMode(PORT[port1][SMALL], INPUT);
//  pinMode(PORT[port1][READ], INPUT);
//  pinMode(PORT[port2][READ], OUTPUT);
//  digitalWrite(PORT[port2][READ], HIGH);
//}

void switchToBigResistor(byte port1, byte port2) {
  resetOther(port1, port2);
  
  pinMode(PORT[port1][SMALL], INPUT);
  pinMode(PORT[port1][BIG], OUTPUT);
  pinMode(PORT[port1][READ], INPUT);
  pinMode(PORT[port2][SMALL], INPUT);
  pinMode(PORT[port2][BIG], INPUT);
  pinMode(PORT[port2][READ], OUTPUT);
  digitalWrite(PORT[port1][SMALL], LOW);
  digitalWrite(PORT[port1][BIG], LOW);
  digitalWrite(PORT[port1][READ], LOW);
  digitalWrite(PORT[port2][SMALL], LOW);
  digitalWrite(PORT[port2][BIG], LOW);
  digitalWrite(PORT[port2][READ], HIGH);
}

void dischargeModeBigR(byte port1, byte port2){ // 让电容放电(在大电阻上)
  pinMode(PORT[port1][SMALL], INPUT);
  pinMode(PORT[port1][BIG], OUTPUT);
  digitalWrite(PORT[port1][BIG], LOW);
  pinMode(PORT[port1][READ], INPUT);
  pinMode(PORT[port2][SMALL], INPUT);
  pinMode(PORT[port2][BIG], INPUT);
  pinMode(PORT[port2][READ], OUTPUT);
  digitalWrite(PORT[port2][READ], LOW);
}

void dischargeModeSmallR(byte port1, byte port2){ // 让电容放电(在小电阻上)
  pinMode(PORT[port1][SMALL], OUTPUT);
  pinMode(PORT[port1][BIG], INPUT);
  digitalWrite(PORT[port1][SMALL], LOW);
  pinMode(PORT[port1][READ], INPUT);
  pinMode(PORT[port2][SMALL], INPUT);
  pinMode(PORT[port2][BIG], INPUT);
  pinMode(PORT[port2][READ], OUTPUT);
  digitalWrite(PORT[port2][READ], LOW);
}

void dischargeCapacitorSmallR(byte port1, byte port2, int dischargeTime){
  dischargeModeSmallR(port1, port2);
  delay(dischargeTime);
}

void dischargeCapacitorBigR(byte port1, byte port2, int dischargeTime){
  dischargeModeBigR(port1, port2);
  delay(dischargeTime);
}

float getResistance(byte port1, byte port2, float r0) {
  float v = getAvgVoltage(port1, 10, 10);
  float r = r0 * (VCC - v) / v;
  return r;
}


bool recordVoltages(float *vArr, byte port1, int cnt, int interval){  // true: 电压下降值客观; false: 电压基本不变
  float THRESH = 0.5;
  for (int i = 0; i < cnt; ++i){
    vArr[i] = getVoltage(port1);
    delayMicroseconds(interval);
  }
  return (vArr[cnt - 1] - vArr[0] > THRESH);
}

float getTao(float *vArr, int pointCnt, int interval){
  const int THRESH = 0.1; // 电压为0的阈值
  float tao = -1;
  float sumTao = 0;
  int validCnt = 0;
  const float validThresh = 0.01; 
  // V = Vcc * e ** (-t / tao)
  for(int i = 1; i < pointCnt; ++i){ // 计算tao平均值
    float t = i * interval * 1e-6;
    float logDiff = log(VCC) - log(vArr[i]);
    if (__abs(logDiff) > validThresh){ // 如果电压接近0 就不计入
      sumTao += (t / logDiff);
      ++validCnt;;
    }
  }

  return sumTao / (float)(validCnt);
}

float getCapacitance(byte port1, byte port2, float r0){
  float vArr[ARRAY_LEN];
  int pointCnt = 50; // 取点数目
  int interval = 500; // 测量间隔(us)

  if(!recordVoltages(vArr, port1, pointCnt, interval)){
    return -1;
  }
  flaot tao = getTao(vArr, pointCnt, interval);
  return tao / r0;    // tao = RC
}

void testResistor(byte port1, byte port2) {
  // 这个阈值是随便取的，我有一个计算更好的值的思路，但是还没算
  const float THRESHOLD = 4.5;
  lcd.clear();
  printType("Resistor");
  printStatus("Wait...");
  float r0 = R_BIG;
  printDebug("Big");
  switchToBigResistor(port1, port2);
  delay(10);
  float v = getVoltage(port1);
  if (v > THRESHOLD) {
    r0 = R_SMALL;
    printDebug("Small");
    switchToSmallResistor(port1, port2);
    delay(10);
  }
  float r = getResistance(port1, port2, r0);
  printValue(r, "Ohm");
  printStatus("        Done!");
}

void testCapacitor(byte port1, byte port2){
  const int dischargeTime = 3000;
  lcd.clear();
  printType("Capacitor");
  printStatus("Trying Small F:");
  float cap;
  switchToBigResistor(port1, port2); // 开始充电
  cap = getCapacitance(port1, port2, R_BIG);
  dischargeCapacitorBigR(port1, port2, dischargeTime); //  电容放电

  if(cap < 0){
    printStatus("Failed, Trying big F"); 
    switchToSmallResistor(port1, port2); // 开始充电
    cap = getCapacitance(port1, port2, R_SMALL);
    dischargeCapacitorSmallR(port1, port2, dischargeTime); //  电容放电
  }

  printValue(cap, "F");
  printStatus("        Done!");
}


void setup() {
  lcd.begin(84, 48);
  Serial.begin(9600);
  switchToBigResistor(0, 1);
  float v = analogRead(0);
  delay(1000);
  switchToBigResistor(1, 0);
  v = analogRead(1);
  delay(1000);
//  pinMode(READ_1, OUTPUT);
//  pinMode(READ_2, OUTPUT);
//  digitalWrite(READ_1, HIGH);
//  digitalWrite(READ_2, LOW);
//  switchToSmallResistor(0, 1);
}

//void loop() {
//  testResistor(0, 1);
//  delay(3000);
//}

float v;

void loop() {
//  v = getVoltage(1);
//  lcd.setCursor(0, 1);
//  lcd.print(v);
//  delay(100);

  lcd.setCursor(0, 0);
  lcd.print((int)testConnectivity(0, 1));
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print((int)testConnectivity(1, 0));
  delay(1000);
//  lcd.setCursor(0, 2);
//  lcd.print((int)testConnectivity(1, 2));
//  delay(200);
//  lcd.setCursor(0, 3);
//  lcd.print((int)testConnectivity(2, 1));
//  delay(200);
//  lcd.setCursor(0, 4);
//  lcd.print((int)testConnectivity(0, 2));
//  delay(200);
//  lcd.setCursor(0, 5);
//  lcd.print((int)testConnectivity(2, 0));
//  delay(200);
}

//void loop() {
//  switchToSmallResistor(0, 1);
//  lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print(getVoltage(0));
//  lcd.setCursor(0, 1);
//  lcd.print(getVoltage(1));
//  delay(1000);
//}

#include <EnableInterrupt.h>
#include <math.h>
#include <TimerOne.h>

enum PlantMode
{
    loadMode = HIGH
  , drainMode = LOW
};

class Plant
{
private:
  PlantMode mode;
  double initLevelDrain, h;
  unsigned long initDrainTime;
  const double a, A, C;
  static const double Cc, Cv, D, d, g, fillGain;

  static Plant* instance;

  double nextH(double P, double nextT) const {
    return pow(sqrt(initLevelDrain) - P*C*a*sqrt(2*g/(pow(A, 2) - pow(a, 2)))*nextT/2, 2);
  }

  Plant() : mode(drainMode), initLevelDrain(Hmin), h(Hmin), initDrainTime(millis()), a(M_PI*pow(d, 2)/4), A(M_PI*pow(D, 2)/4), C(Cc*Cv) { }
  
public:
  static const double Hmax, Hmin;

  ~Plant() {
    if (instance != NULL) {
      delete instance;
    }  
  }

  static Plant* getInstance() {
    if (instance == NULL) {
      instance = new Plant;  
    }
    return instance;
  }

  void updateState(double P) {   
    if (mode == loadMode) {
      h = (h + fillGain > Hmax) ? Hmax : h + fillGain;
    }
    else if (mode == drainMode) {
      auto Time = millis();
      auto nextH = nextH(P, (Time - initDrainTime)/1000.0);
      h = (nextH < Hmin) ? Hmin : nextH;
      initDrainTime = Time;
      initLevelDrain = h;
    }
  }

  void setMode(PlantMode m) {
    if (m == drainMode && mode != drainMode) {
      initDrainTime = millis();
      initLevelDrain = h;
    }
    mode = m;
  }

  double getH() const {
    return h;  
  }

  
  unsigned long getInitDrainTime() const {
    return initDrainTime;
  }
};

const double Plant::Cc = 0.52;
const double Plant::Cv = 0.98;
const double Plant::D = 0.0165;
const double Plant::d = 0.001;
const double Plant::g = 9.81;
const double Plant::Hmax = 0.13;
const double Plant::Hmin = 0.005;
const double Plant::fillGain = 0.005;
Plant* Plant::instance = NULL;

class ES
{
private:
  static const double S_MIN, S_MAX, E_MIN, E_MAX, PIN_mode_Plant;

public:
  static double readC() {
    pinMode(PIN_mode_Plant, INPUT_PULLUP);
    return digitalRead(PIN_mode_Plant);
  }

  static byte converRange(double e) {
    return S_MIN + (e - E_MIN)*(S_MAX - S_MIN)/(E_MAX - E_MIN);
  }

  static void writeH(double h) {
    auto out = converRange(h);
    
    DDRB |= B00001111;
    DDRD |= B11110000;
    
    digitalWrite(11, out & B10000000);
    digitalWrite(10, out & B01000000);
    digitalWrite(9, out & B00100000);
    digitalWrite(8, out & B00010000);
    digitalWrite(7, out & B00001000);
    digitalWrite(6, out & B00000100);
    digitalWrite(5, out & B00000010);
    digitalWrite(4, out & B00000001);
  }
};

const double ES::S_MIN = 0;
const double ES::S_MAX = 255;
const double ES::E_MIN = Plant::Hmin;
const double ES::E_MAX = Plant::Hmax;
const double ES::PIN_mode_Plant = 3;

class GainMonitor
{
private:
  volatile int valPwm;
  volatile int previousTime;
  static const int MIN_PWM, MAX_PWM;
  static const double minGain, maxGain;

  static GainMonitor* instance;

  GainMonitor() : valPwm(0), previousTime(0) {
    pinMode(PIN, INPUT_PULLUP);
  }

public:
  static const uint8_t PIN;

  ~GainMonitor() {
    disableInterrupt(PIN);
    if (instance != NULL) {
      delete instance;  
    }
  }

  static GainMonitor* getInstance() {
    if (instance == NULL) {
      instance = new GainMonitor;  
    }
    return instance;
  }

  void goUp() {
    previousTime = micros();
  }

  void goDown() {
    valPwm = micros() - previousTime;
  }

  double getPropGain() const {
    auto propNotLimit = minGain + (valPwm - MIN_PWM)*(maxGain - minGain)/(MAX_PWM - MIN_PWM);
    auto propLimit = (propNotLimit < minGain)
      ? minGain
      : (propNotLimit > maxGain)
        ? maxGain
        : propNotLimit;
    //Serial.println(propLimit);
    return propLimit;
  }
};

const uint8_t GainMonitor::PIN = 2;
GainMonitor* GainMonitor::instance = NULL;
const double GainMonitor::minGain = 0.0;
const double GainMonitor::maxGain = 1.0;
const int GainMonitor::MIN_PWM = 548;
const int GainMonitor::MAX_PWM = 1480;

void up()
{
  enableInterrupt(GainMonitor::PIN, down, FALLING);
  GainMonitor::getInstance()->goUp();  
}

void down()
{
  enableInterrupt(GainMonitor::PIN, up, RISING);
  GainMonitor::getInstance()->goDown();
}

void itSamples()
{
  auto Plant = Plant::getInstance();
  Plant->setMode((PlantMode)ES::readC());
  auto G = GainMonitor::getInstance()->getPropGain() * 100;
  Plant->updateState(G);
  auto t = Plant->getInitDrainTime() / 1000;
  auto h = Plant->getH();
  ES::writeH(h);

  Serial.println(G, 3);
}

void setup()
{
  Serial.begin(9600);
  Timer1.initialize(10000);
  Timer1.attachInterrupt(itSamples);
  enableInterrupt(GainMonitor::PIN, up, RISING);
}

void loop() { }

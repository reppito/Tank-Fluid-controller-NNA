#include <ArduinoSTL.h>
#include <LiquidCrystal.h>
#include <Neurona.h>
#include <Servo.h>

using std::vector;

void updateSensors();

enum ButtonState
{
  PUSHING = LOW
  , NOT_PUSHING = HIGH
};

class ButtonRecieverDelegate
{
  public:
    virtual void ChangedButtonState(short IdButton, ButtonState StateNew) = 0;
};

class ButtonReciever
{
  private:
    int pin;
    ButtonState ActualState, PreviousState;
    vector<ButtonRecieverDelegate*> delegates;
    unsigned long LastRebound;
    static const unsigned long TimeDelay = 50;
    static short ID;
    const short id;

    void NotifyDelegates() const {
      for (auto delegate = delegates.begin(); delegate != delegates.end(); ++delegate) {
        (*delegate)->ChangedButtonState(id, ActualState);
      }
    }

  public:
    ButtonReciever(int pin) : pin(pin), ActualState(NOT_PUSHING), PreviousState(ActualState), LastRebound(0), id(ID++) {
      pinMode(pin, INPUT_PULLUP);
    }

    void updateState() {
      int read = digitalRead(pin);

      if (read != PreviousState) {
        LastRebound = millis();
      }

      if ((millis() - LastRebound) > TimeDelay) {
        if (read != ActualState) {
          ActualState = (ButtonState)read;
          NotifyDelegates();
        }
      }
      PreviousState = (ButtonState)read;
    }

    vector<ButtonRecieverDelegate*>* getDelegates() {
      return &delegates;
    }

    short getId() const {
      return id;
    }
};

short ButtonReciever::ID = 1;

class controlPanelReciever
{
  private:
    ButtonReciever
    DownLevelReciever
    , UpLevelReciver
    , trainingModeReciever
    , OperationModeReciever
    , stopSistemReciever;

  public:
    controlPanelReciever()
      : DownLevelReciever(ButtonReciever(PIN_B0))
      , UpLevelReciver(ButtonReciever(PIN_B1))
      , trainingModeReciever(ButtonReciever(PIN_B2))
      , OperationModeReciever(ButtonReciever(PIN_B3))
      , stopSistemReciever(ButtonReciever(PIN_B4)) { }

    void actRecievers() {
      DownLevelReciever.updateState();
      UpLevelReciver.updateState();
      trainingModeReciever.updateState();
      OperationModeReciever.updateState();
      stopSistemReciever.updateState();
    }

    ButtonReciever* getRecieverDown() {
      return &DownLevelReciever;
    }

    ButtonReciever* getUpReciever() {
      return &UpLevelReciver;
    }

    ButtonReciever* getOpTrainingReciever() {
      return &trainingModeReciever;
    }

    ButtonReciever* getTestTrainingReciever() {
      return &OperationModeReciever;
    }

    ButtonReciever* getStopReciever() {
      return &stopSistemReciever;
    }
};

class UltrasonicSensorReciever
{
  private:
    static const unsigned short heightSensor; // cm.
    static const int PIN_TRIGGER, PIN_ECHO;
    static const unsigned long TIMEOUT;
    double actualWaterLevel, previousWaterLevel, deltaWaterLevel;
    unsigned long TimeUpdate;

    double calculateWaterLevelProm() const {
      const int samplesNumber = 25;
      double levelSum = 0;

      for (int i = 0; i < samplesNumber; ++i) {
        delay(5);
        levelSum += calculateWaterLevel();  
      }

      return levelSum/samplesNumber;
    }

    double calculateWaterLevel() const {
      /*// simulation code
        pinMode(PIN_C7, INPUT);
        pinMode(PIN_C6, INPUT);
        pinMode(PIN_C5, INPUT);
        pinMode(PIN_C4, INPUT);
        pinMode(PIN_C3, INPUT);
        pinMode(PIN_C2, INPUT);
        pinMode(PIN_C1, INPUT);
        pinMode(PIN_C0, INPUT);

        byte lecturaPuerto = digitalRead(PIN_C7) << 7 | digitalRead(PIN_C6) << 6 | digitalRead(PIN_C5) << 5 | digitalRead(PIN_C4) << 4
                           | digitalRead(PIN_C3) << 3 | digitalRead(PIN_C2) << 2 | digitalRead(PIN_C1) << 1 | digitalRead(PIN_C0);

        return (0.005 + (lecturaPuerto) * (0.13 - 0.005) / (255)) * 100;
        // end*/

      digitalWrite(PIN_TRIGGER, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_TRIGGER, LOW);

      auto reboundDistance = pulseIn(PIN_ECHO, HIGH, TIMEOUT) / 58.0;

      if (reboundDistance == 0) {
        pinMode(PIN_ECHO, OUTPUT);
        digitalWrite(PIN_ECHO, LOW);
        delayMicroseconds(1);
        pinMode(PIN_ECHO, INPUT);
      }

      return heightSensor - ((reboundDistance > heightSensor)
                              ? heightSensor
                              : (reboundDistance == 0)
                              ? heightSensor
                              : reboundDistance);
    }

  public:
    UltrasonicSensorReciever() : actualWaterLevel(0), previousWaterLevel(actualWaterLevel), deltaWaterLevel(0), TimeUpdate(0) {
      pinMode(PIN_TRIGGER, OUTPUT);
      pinMode(PIN_ECHO, INPUT);
    }

    void UpdateWaterLevel() {
      auto updateTime = millis();

      previousWaterLevel = actualWaterLevel;
      actualWaterLevel = calculateWaterLevelProm();
      deltaWaterLevel = (actualWaterLevel - previousWaterLevel) / (updateTime - TimeUpdate) * 1000;
      TimeUpdate = updateTime;
    }

    double getWaterLevel() const {
      return actualWaterLevel;
    }

    double getWaterLevelDelta() const {
      return deltaWaterLevel;
    }
};

const unsigned short UltrasonicSensorReciever::heightSensor = 16;
const int UltrasonicSensorReciever::PIN_TRIGGER = PIN_B6;
const int UltrasonicSensorReciever::PIN_ECHO = PIN_B5;
const unsigned long UltrasonicSensorReciever::TIMEOUT = 3000;

enum OperationMode
{
  TrainingMode
  , TestingMode
};

class NeuronalController
{
  private:
    UltrasonicSensorReciever* sensor;
    double gainCalculated;

  public:
    NeuronalController(UltrasonicSensorReciever* sensor) : sensor(sensor), gainCalculated(0) { }

    void setgainCalculated(double pc) {
      gainCalculated = pc;
    }
  
    double calculateGainCalculated() {
      return gainCalculated;
    }
};

class triggerController
{
  private:
    double gain;
    NeuronalController* NeuronalController;
    bool automateControllerActive;
    
    double calculateGainServe() const {     
      return min_serve_ang + (gain - min_gain) * (max_serve_ang - min_serve_ang)
             / (max_gain - min_gain);
    }

  public:
    static const double min_gain, max_gain, min_serve_ang, max_serve_ang;
    Servo servo;

    triggerController(NeuronalController* NeuronalController) : gain(min_gain), NeuronalController(NeuronalController)
      , automateControllerActive(true) {
      servo.attach(PIN_D7);
      updateTrigger();
    }

    void setGain(double p) {
      automateControllerActive = false;
      gain = p;
    }

    double getGain() const {
      return gain;
    }

    void updateTrigger() {
      if (automateControllerActive) {
        gain = NeuronalController->calculateGainCalculated();
      }
      
      auto calcAngServ = calculateGainServe();
      auto AngServ = (calcAngServ < min_serve_ang)
                         ? min_serve_ang
                         : (calcAngServ > max_serve_ang)
                         ? max_serve_ang
                         : calcAngServ;
      servo.write(AngServ);
    }

    void activeAutomateController() {
      automateControllerActive = true;  
    }
};

const double triggerController::min_gain = 0;
const double triggerController::max_gain = 1;
const double triggerController::min_serve_ang = 9;
const double triggerController::max_serve_ang = 81;

class Led
{
  private:
    int pin;

  public:
    Led(int pin) : pin(pin) {
      pinMode(pin, OUTPUT);
    }

    void setState(bool StateNew) const {
      digitalWrite(pin, StateNew);
    }
};

class DisplayPanelController
{
  private:
    Led trainingModeLED, testingModeLED, upLED, downLED, TrainingEndLED;
    LiquidCrystal lcd;

  public:
    DisplayPanelController() : trainingModeLED(PIN_F0), testingModeLED(PIN_F1), upLED(PIN_F2)
      , downLED(PIN_F3), TrainingEndLED(PIN_F4), lcd(LiquidCrystal(PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5)) {
      lcd.begin(16, 2);
      lcd.print("h: ");
      lcd.setCursor(0, 1);
      lcd.print("G: ");
    }

    Led* getTrainingModeLED() {
      return &trainingModeLED;
    }

    Led* getTestModeLED() {
      return &testingModeLED;
    }

    Led* getUpLED() {
      return &upLED;
    }

    Led* getDownLED() {
      return &downLED;
    }

    Led* getTrainingEndLED() {
      return &TrainingEndLED;
    }

    void setWaterLevel(double waterLEvel) {
      lcd.setCursor(3, 0);
      lcd.print(waterLEvel);
      lcd.print("cm     ");
    }

    void setGain(double gain) {
      lcd.setCursor(3, 1);
      lcd.print(gain * 100);
      lcd.print("%      ");
    }
};

enum trainingState
{
  InitState
  , fillState
  , downState
  , tranState
  , endState
  , nullState
};

class trainingDelegate
{
  public:
    virtual void initTraining() = 0;
    virtual void whileGoingDown() = 0;
    virtual void trainingEnded() = 0;
};

class Training
{
  private:
    trainingState state;
    triggerController* triggerController;
    UltrasonicSensorReciever* UltrasonicSensorReciever;
    static const double maxWaterLevel, minWaterLevel, initRegisterLevel;
    static const unsigned long timeStates;
    unsigned long timeEndState;
    short itAct;
    static const short samplesNumbers;
    vector<trainingDelegate*> delegates;
    DisplayPanelController* DisplayPanelController;

  public:
    Training(triggerController* ca, UltrasonicSensorReciever* rsu, DisplayPanelController* cpv) : state(nullState), triggerController(ca)
      , UltrasonicSensorReciever(rsu), timeEndState(0), itAct(1), DisplayPanelController(cpv) { }

    void initTraining() {
      timeEndState = 0;
      itAct = 1;
      state = InitState;
      notInit();
    }

    void endTraining() {
      itAct = samplesNumbers;
      state = tranState;
    }

    vector<trainingDelegate*>* getDelegates() {
      return &delegates;
    }

    void notInit() const {
      for (auto it = delegates.begin(); it != delegates.end(); ++it) {
        (*it)->initTraining();
      }
    }

    void notiWhileGoingDown() const {
      for (auto it = delegates.begin(); it != delegates.end(); ++it) {
        (*it)->whileGoingDown();
      }
    }

    void notEnd() const {
      for (auto it = delegates.begin(); it != delegates.end(); ++it) {
        (*it)->trainingEnded();
      }
    }

    void updateState() {
      if (itAct <= samplesNumbers) {
        if (state == InitState) {
          DisplayPanelController->getDownLED()->setState(HIGH);
          DisplayPanelController->getUpLED()->setState(LOW);
          DisplayPanelController->getTrainingEndLED()->setState(LOW);

          if (timeEndState == 0) {
            timeEndState = millis() + timeStates;
            triggerController->setGain(triggerController::min_gain);
          }
          else if (millis() < timeEndState) {
            if (triggerController->getGain() > triggerController::min_gain) {
              triggerController->setGain(triggerController::min_gain);
            }
          }
          else {
            state = fillState;
          }
        }
        else if (state == fillState) {
          DisplayPanelController->getDownLED()->setState(LOW);
          DisplayPanelController->getUpLED()->setState(HIGH);

          if (UltrasonicSensorReciever->getWaterLevel() < maxWaterLevel) {
            timeEndState = 0;
            if (triggerController->getGain() > triggerController::min_gain) {
              triggerController->setGain(triggerController::min_gain);
            }
          }
          else if (timeEndState == 0) {
            timeEndState = millis() + timeStates;
          }
          else if (millis() >= timeEndState) {
            timeEndState = 0;
            state = downState;
          }
        }
        else if (state == downState) {
          DisplayPanelController->getDownLED()->setState(HIGH);
          DisplayPanelController->getUpLED()->setState(LOW);

          if (timeEndState == 0) {
            if (UltrasonicSensorReciever->getWaterLevel() > minWaterLevel) {
              if (UltrasonicSensorReciever->getWaterLevel() <= initRegisterLevel) {
                notiWhileGoingDown();
              }
              triggerController->setGain((triggerController::max_gain / samplesNumbers)*itAct);
            }
            else {
              timeEndState = millis() + timeStates;
            }
          }
          else if (millis() < timeEndState) {
            if (triggerController->getGain() > triggerController::min_gain) {
              triggerController->setGain(triggerController::min_gain);
            }
          }
          else {
            state = tranState;
          }
        }
        else if (state == tranState) {
          if (itAct == samplesNumbers) {
            DisplayPanelController->getDownLED()->setState(HIGH);
            DisplayPanelController->getUpLED()->setState(LOW);
            DisplayPanelController->getTrainingEndLED()->setState(HIGH);
            timeEndState = 0;
            state = endState;
          }
          else {
            itAct += 1;
            state = fillState;
          }
        }
        else if (state == endState) {
          if (timeEndState == 0) {
            timeEndState = millis() + timeStates;
          }
          else if (millis() >= timeEndState) {
            DisplayPanelController->getDownLED()->setState(LOW);
            DisplayPanelController->getTrainingEndLED()->setState(LOW);
            state = nullState;
            notEnd();
          }
        }
      }
    }
};

const double Training::maxWaterLevel = 10;
const double Training::initRegisterLevel = maxWaterLevel - 1;
const double Training::minWaterLevel = 4;
const unsigned long Training::timeStates = 5000;
const short Training::samplesNumbers = 3;

struct trainingelement
{
  double waterLEvel, deltaWaterLevel, gain;
};

class neuronalNetworkBuffer
{ 
  public: 
    static void sentInitSignal() {
      Serial.write('[');  
    }
  
    static void sentTrainingElement(trainingelement* elementoE) {
      Serial.print("{\"waterLEvel\":");
      Serial.print(elementoE->waterLEvel);
      Serial.print(",\"deltaWaterLevel\":");
      Serial.print(elementoE->deltaWaterLevel);
      Serial.print(",\"gain\":");
      Serial.print(elementoE->gain);
      Serial.write('}');
    }

    static void sentEndSignal() {
      Serial.write(']');
    }
};

class centralSystem : public ButtonRecieverDelegate, trainingDelegate
{
  private:
    controlPanelReciever controlPanelReciever;
    OperationMode OperationMode;
    UltrasonicSensorReciever UltrasonicSensorReciever;
    NeuronalController NeuronalController;
    triggerController triggerController;
    Training Training;
    DisplayPanelController DisplayPanelController;

    static centralSystem* instance;

    centralSystem() : controlPanelReciever(controlPanelReciever()), OperationMode(TestingMode)
      , NeuronalController(&UltrasonicSensorReciever)
      , triggerController(&NeuronalController)
      , Training(&triggerController, &UltrasonicSensorReciever, &DisplayPanelController) {
      Serial.begin(9600);
      controlPanelReciever.getUpReciever()->getDelegates()->push_back(this);
      controlPanelReciever.getRecieverDown()->getDelegates()->push_back(this);
      controlPanelReciever.getOpTrainingReciever()->getDelegates()->push_back(this);
      controlPanelReciever.getTestTrainingReciever()->getDelegates()->push_back(this);
      controlPanelReciever.getStopReciever()->getDelegates()->push_back(this);
      Training.getDelegates()->push_back(this);
    }

  public:
    virtual ~centralSystem() {
      if (instance != NULL) {
        delete instance;
      }
    }

    static centralSystem* getInstance() {
      if (instance == NULL) {
        instance = new centralSystem;
      }
      return instance;
    }

    void initTraining() {
      neuronalNetworkBuffer::sentInitSignal();
    }

    void whileGoingDown() {
      trainingelement elementoEnt;
      elementoEnt.waterLEvel = UltrasonicSensorReciever.getWaterLevel();
      elementoEnt.deltaWaterLevel = UltrasonicSensorReciever.getWaterLevelDelta();
      elementoEnt.gain = triggerController.getGain();
      neuronalNetworkBuffer::sentTrainingElement(&elementoEnt);
    }

    void trainingEnded() {
      neuronalNetworkBuffer::sentEndSignal();
      DisplayPanelController.getTrainingModeLED()->setState(LOW);
      DisplayPanelController.getTestModeLED()->setState(HIGH);
      OperationMode = TestingMode;
      triggerController.activeAutomateController();
    }

    void ChangedButtonState(short IdButton, ButtonState StateNew) {          
      if (OperationMode == TrainingMode) {        
        if (IdButton == controlPanelReciever.getTestTrainingReciever()->getId()) {
          Training.endTraining();
        }
      }
      else if (OperationMode == TestingMode) {       
        if (IdButton == controlPanelReciever.getUpReciever()->getId()) {
          if (StateNew == PUSHING) {
            DisplayPanelController.getUpLED()->setState(HIGH);
            triggerController.setGain(triggerController::min_gain);
          }
          else {
            DisplayPanelController.getUpLED()->setState(LOW);
            triggerController.activeAutomateController();
          }
        }
        else if (IdButton == controlPanelReciever.getRecieverDown()->getId()) {
          if (StateNew == PUSHING) {
            //DisplayPanelController.getUpLED()->setState(HIGH);        
            triggerController.setGain(triggerController::max_gain);
          }
          else {
            DisplayPanelController.getUpLED()->setState(LOW);
            triggerController.activeAutomateController();
          }
        }
        else if (IdButton == controlPanelReciever.getOpTrainingReciever()->getId()) {
          DisplayPanelController.getTrainingModeLED()->setState(HIGH);
          DisplayPanelController.getTestModeLED()->setState(LOW);
          OperationMode = TrainingMode;
          Training.initTraining();
        }
      }
    }

    controlPanelReciever* getControlPanelReciever() {
      return &controlPanelReciever;
    }

    OperationMode getOperationMode() const {
      return OperationMode;
    }

    void changeOperationMode(OperationMode newOperationMode) {
      OperationMode = newOperationMode;
    }

    UltrasonicSensorReciever* getUltrasonicReciever() {
      return &UltrasonicSensorReciever;
    }

    triggerController* getTriggerController() {
      return &triggerController;
    }

    Training* getTraining() {
      return &Training;
    }

    void actSensors() {
      UltrasonicSensorReciever.UpdateWaterLevel();
    }

    void actTriggers() {
      triggerController.updateTrigger();
      Training.updateState();
    }

    void actDisplayPanel() {

      if (OperationMode == TestingMode) {
        DisplayPanelController.getTrainingModeLED()->setState(LOW);
        DisplayPanelController.getTestModeLED()->setState(HIGH);
      }
      else if (OperationMode == TrainingMode) {
        DisplayPanelController.getTrainingModeLED()->setState(HIGH);
        DisplayPanelController.getTestModeLED()->setState(LOW);
      }
      
      controlPanelReciever.actRecievers();
      DisplayPanelController.setWaterLevel(UltrasonicSensorReciever.getWaterLevel());
      DisplayPanelController.setGain(triggerController.getGain());
    }

    void actNeuronalNetworkController() {
      if (OperationMode == TestingMode) {
        Serial.print("{\"waterLEvel\":");
        Serial.print(UltrasonicSensorReciever.getWaterLevel());
        Serial.print(",\"deltaWaterLevel\":");
        Serial.print(UltrasonicSensorReciever.getWaterLevelDelta());
        Serial.print("}");
        if (Serial.available() >= 7) {
          float gainCalculated = Serial.parseFloat();
          Serial.flush();
          NeuronalController.setgainCalculated(gainCalculated);
        }
      }

    }
};

centralSystem* centralSystem::instance = NULL;

void setup()
{
  centralSystem::getInstance();
}

void loop()
{
  centralSystem::getInstance()->actSensors();
  centralSystem::getInstance()->actDisplayPanel();
  centralSystem::getInstance()->actNeuronalNetworkController();
  centralSystem::getInstance()->actTriggers();
}

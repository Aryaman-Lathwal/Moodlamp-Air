#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const int redPin{25};
const int greenPin{26};
const int bluePin{27};
const int redPot{32};
const int greenPot{33};
const int bluePot{34};
const int modePin{35};

//enum to keep track of last mode being used (to smoothen transitioin)
enum Lastmode
{
  web,
  pot,
};
enum Lastmode lastmode{};

//declaring struct containing variables which hold the r, g and b values (0 to 255)
struct Colour
{
  int red{};
  int green{};
  int blue{};
  int brightness{};
};
typedef struct Colour Colour;

//global Current and Target Colour struct
Colour Current{0, 0, 0, 100};
Colour Target{0, 0, 0, 100};

//struct to remember colour before lightning
Colour LightningTarget{0, 0, 0, 100};
Colour BeforeLightning{0, 0, 0, 100};


//speed enumeration for better readability
enum Speed
{
  instant,
  vfast,
  fast,
  medium,
  slow,
  vslow,
};


//start of globals
int brightnessScale{100};

enum Preset
{
  none,
  oceanPreset,
  sunrise,
  sunset,
  fireplace,
  aurora,
  thunderstorm,
  airbus,
  boeing,
  rainbow,
  disco,
  party,
  police,
};
enum Preset CurrentPreset;


enum OceanState
{
  holding,
  transitioning,
};
enum OceanState oceanState;
unsigned long oceanStateStartTime{}; //global variable to keep a timestamp track for oceanstate.
unsigned long lastTransitionStep{}; //global variable which records timestamp when last move towards target colour happened


enum SunriseState
{
  sunriseholding,
  sunrisetransitioning,
};
enum SunriseState sunriseState;
unsigned long sunriseStateStartTime{}; //global variable to decide transition delays or holding delays


enum FireplaceState
{
  fireholding,
  firetransitioning,
};
enum FireplaceState fireplaceState;
unsigned long fireplaceStateStartTime{}; //global variable to timestamp the state of ocean for holding delay or transition speed


enum AuroraState
{
  auroraholding,
  auroratransitioning,
};
enum AuroraState auroraState;
unsigned long auroraStateStartTime{}; //keps timestamp for state of auraras for holding delay or transition speed


enum StormState
{
  stormholding,
  stormtransitioning,
};
enum StormState stormState;
unsigned long stormStateStartTime{}; //keeps time track for holding and transitioning storm colours


enum RippleState
{
  passive,
  active,
};
enum RippleState rippleState;
unsigned long rippleStateStartTime{}; //global variable to keep a timestamp track for ripplestate.
int requestedRippleDelay{};

enum FlickerState
{
  flickerpassive,
  flickeractive,
};
enum FlickerState flickerState;
unsigned long flickerStateStartTime{}; //global variable to keep a timestamp track for flickerstate.
unsigned long strobeStateStartTime{}; //global variable to keep track of delays between strobing in airbus/boeing effect.
int requestedFlickerDelay{};

enum UndulationState
{
  undulationpassive,
  undulationactive,
};
enum UndulationState undulationState;
unsigned long undulationStateStartTime{}; //global variable to keep track for undulations in auroras.
int requestedUndulationDelay{};

enum LightningState
{
  lightningpassive,
  lightningactive,
};
enum LightningState lightningState;
unsigned long lightningStateStartTime{}; //global variable to keep track of lightning in thunderstorm
int requestedLightningDelay{};

enum PartyMode
{
  smoothdrift,
  beatbuildup,
  basebeats,
  blackout,
  strobe,
  colourexplosion,
  frozencolours,
  modedecider,
};
enum PartyMode partyMode;
unsigned long partyStateStartTime{}; //to control the duration of party modes
int requestedPartyModeDelay{};

enum StrobeState
{
  on,
  off,
};
enum StrobeState strobeState;
int stroberate{};

enum BeatStage
{
  slowbeats,
  midbeats,
  fastbeats,
};
enum BeatStage beatStage;
int beatStateStartTime{};


struct Ripple
{
  int amplitude; //in values of percent. a value of 10 means beightness oscillates between 90 and 110%
  int oscillations; //number of oscillations we want in the ripple
  int rippleTime; //how long the entire ripple sequence should last (wont turn out accurate as toTarget() has its own time delay)
  float omega; //physics ((2 * n(oscillations) * PI) / t(rippleTime))
  float gamma; //physics again ((log(amplitude/3)) / t(rippleTime))
};
typedef struct Ripple Ripple;

struct Flicker
{
  int amplitude; //in values of percent. value of 10 would mean brightness can rise by 10 units.
  int flickers; //number of 'flickers' we want in the entire flicker sequence
  int flickerTime; //how long entire flicker sequence should last. picked randomly
  float omega; //physics lol ((2 * n(flickers) * PI)/ t(flickerTime))
};
typedef struct Flicker Flicker;

struct Undulation
{
  int amplitude; //same, in values of percent
  int undulations; //no. of oscillations
  int sign; //whether we want positive deviataion or negative devation in brightness
  int undulationTime; //how long the undulation should last
  float omega; //physics again xD ((2 * n(undulations) * PI) / t(undulationTime))
};
typedef struct Undulation Undulation;

struct Lightning
{
  int brightness; //how bright it should be. literal out of 100 (single flashes are brighter, more are dimmer)
  int strikes; //no. of lightnings (sits between 1 and 3)
  int lightningTime; //how long the entire lightning sequence lasts
  float omega; //physics. you know the rest
};
typedef struct Lightning Lightning;


int requestedHoldingDelay{2000}; //global colour holding delay variable so it works independent of any loops.
int requestedTransitionDelay{5000}; //global transition delay variable ''  ''.

Colour ocean[23]
{
// Bright tropical water
{0, 180, 255, 100},
{0, 200, 255, 100},
{20, 190, 255, 100},
{0, 150, 150, 100},

// Turquoise
{0, 200, 180, 100},
{0, 220, 180, 100},
{10, 90, 100, 100},
{30, 180, 220, 100},

// Lagoon water
{0, 220, 150, 100},
{20, 190, 130, 100},
{10, 140, 100, 100},

// Classic ocean blues
{0, 70, 110, 100},
{0, 120, 255, 100},
{0, 100, 220, 100},
{20, 120, 180, 100},

// Slightly deeper
{0, 90, 180, 100},
{0, 80, 160, 100},
{10, 90, 140, 100},

// Rare dark moments
{0, 70, 150, 100},
{0, 60, 120, 100},
{0, 50, 220, 100},
{0, 40, 120, 100},
{0, 20, 240, 100}
};


Ripple requestedRipple;
Flicker requestedFlicker;
Undulation requestedUndulation;
Lightning requestedLightning;

Colour fireplacecolours[24]
{
  // Deep embers (rare)
  {135, 28,  0, 80},
  {145, 35,  0, 80},
  {155, 42,  0, 80},

  // Dark orange
  {165, 55,  2, 80},
  {170, 62,  3, 80},
  {175, 68,  4, 80},

  // Rich orange (most common)
  {180, 75,  5, 80},
  {185, 82,  6, 80},
  {190, 88,  8, 80},
  {195, 95, 10, 80},
  {200, 102, 9, 80},
  {205, 108, 7, 80},
  {210, 115, 8, 80},
  {205, 105, 9, 80},
  {195, 92, 10, 80},

  // Warm amber
  {200, 125, 22, 80},
  {205, 135, 18, 80},
  {210, 145, 15, 80},
  {215, 150, 20, 80},

  // Golden flare (rare)
  {215, 165, 35, 80},
  {220, 175, 30, 80},
  {220, 185, 38, 80},

  // Very rare bright flame
  {220, 195, 55, 80},
  {220, 205, 50, 80}
};

Colour auroracolours[24]
{
  // Emerald greens
  {25, 220, 60, 100},
  {8, 205, 60, 100},
  {15, 190, 45, 100},
  {15, 180, 35, 100},
  {15, 165, 20, 100},
  {15, 210, 50, 100},
  {15, 185, 55, 100},
  {10, 170, 35, 100},
  {15, 155, 65, 100},
  {20, 215, 100, 100},

  // Cyan / Teal
  {20, 200, 140, 100},
  {20, 195, 160, 100},
  {15, 170, 210, 100},
  {20, 180, 150, 100},
  {20, 180, 120, 100},
  {5, 170, 190, 100},
  {10, 185, 105, 100},

  // Violet
  {90, 20, 180, 100},
  {110, 10, 210, 100},
  {80, 30, 170, 100},
  {125, 10, 230, 100},
  {100, 5, 190, 100},

  // Rare pink glow
  {180,40,180,100},
  {200,50,170,100}
};

Colour stormcolours[24] =
{
  // Slate blues
  {15, 18, 26, 100},
  {13, 16, 25, 100},
  {16, 20, 30, 100},
  {17, 22, 32, 100},

  // Blue-grey
  {18, 22, 28, 100},
  {20, 23, 30, 100},
  {22, 25, 32, 100},
  {23, 26, 30, 100},

  // Steel blue
  {16, 22, 33, 100},
  {15, 20, 32, 100},
  {18, 23, 35, 100},
  {20, 25, 36, 100},

  // Storm purple
  {22, 16, 32, 100},
  {23, 18, 35, 100},
  {20, 15, 30, 100},
  {25, 20, 38, 100},

  // Green-grey (that "storm is coming" look)
  {18, 23, 22, 100},
  {16, 22, 20, 100},
  {20, 25, 23, 100},
  {15, 20, 18, 100},

  // Heavy cloud
  {23, 23, 28, 100},
  {22, 22, 26, 100},
  {20, 20, 25, 100},
  {32, 32, 30, 100}
};

Colour lightningcolours[6]
{
    {210, 220, 255, 100},   // cold blue
    {220, 225, 255, 100},   // icy white
    {190, 180, 255, 100},   // violet
    {200, 210, 255, 100},   // cloud flash
    {180, 170, 240, 100},   // purple cloud
    {255, 255, 255, 100}    // rare direct strike
};

Colour rainbowcolours[14]
{
  {255,  0,  0, 100}, //RED
  {255, 20,  0, 100}, //red-orange
  {255,100,  0, 100}, //orange
  {255,220,  0, 100}, //yellow-orange
  {180,255,  0, 100}, //green-yellow
  { 60,255,  0, 100}, //parrot green
  {  0,255,  0, 100}, //GREEN
  {  0,255,120, 100}, //sea green
  {  0,255,255, 100}, //cyan
  {  0,120,255, 100}, //sky bluw
  {  0,  0,255, 100}, //deep blue
  {120,  0,255, 100}, //violet
  {255,  0,255, 100}, //magenta
  {255,  0,120, 100} //pinkish-red
};
int rainbowindex{};
int discoSpeedDecider{};
int policeMode{};

Colour sunrisecolours[24] =
{
  // Deep night (very dim navy)
  {4, 8 , 18, 100},
  {5, 10, 20, 100},
  {7, 13, 26, 100},

  // Nautical twilight
  {10, 21, 29, 100},
  {13, 24, 35, 100},
  {18, 23, 40, 100},

  // Purple hour
  {31, 22, 43, 100},
  {45, 24, 42, 100},
  {61, 25, 37, 100},

  // Pink dawn
  {64, 24, 35, 100},
  {81, 25, 25, 100},
  {96, 30, 15, 100},

  // First sunlight
  {109, 39, 10, 100},
  {118, 50, 8, 100},
  {127, 61, 8, 100},

  // Orange glow
  {132, 73, 10, 100},
  {140, 90, 14, 100},
  {150, 109, 21, 100},

  // Golden hour
  {165, 132, 34, 100},
  {180, 154, 52, 100},
  {200, 183, 77, 100},

  // Morning
  {225, 214, 90, 100},
  {232, 235, 100, 100},
  {255, 255, 115, 100}
};
int sunriseindex{};

bool wifiConnected{true};
int wifisearchtime{};
int beatBrightness{};
//end of globals


//defining a function which moves a few steps towards the target values of r, g and b
Colour toTarget(Colour current, Colour target, enum Speed transpeed)
{
  if(transpeed == instant)
  {
    current.red = target.red;
    current.green = target.green;
    current.blue = target.blue;
  }

  else
  {
    if(current.red < target.red) current.red++;
    else if(current.red > target.red) current.red--;

    if(current.green < target.green) current.green++;
    else if(current.green > target.green) current.green--;

    if(current.blue < target.blue) current.blue++;
    else if(current.blue > target.blue) current.blue--;
  }
  if(transpeed == vfast) delay (1);
  else if(transpeed == fast) delay(3);
  else if(transpeed == medium) delay(9);
  else if(transpeed == slow) delay(20);
  else if(transpeed == vslow) delay(40);
  
  return current;
}

bool compareColour(Colour a, Colour b)
{
  if(a.red != b.red) return false;
  if(a.green != b.green) return false;
  if(a.blue != b.blue) return false;
  return true;
}

//this is an overloaded setColour function and it takes a second argument (a bool value)
//the bool value being true means utilise the brightness functionality and set colour with that in consideration
void setColour(Colour desired)
{
  int redbrightness = (desired.red * desired.brightness) / 100;
  ledcWrite(redPin, constrain(redbrightness, 0, 255));
  int greenbrightness = (desired.green * desired.brightness) / 100;
  ledcWrite(greenPin, constrain(greenbrightness, 0, 255));
  int bluebrightness = (desired.blue * desired.brightness) / 100;
  ledcWrite(bluePin, constrain(bluebrightness, 0, 255));
}

void setup() {
  randomSeed(micros());
  Serial.begin(115200);
  //deploying the pwm hardware to the pins which operate r, g and b mosfets
  ledcAttach(redPin, 5000, 8);
  ledcAttach(greenPin, 5000, 8);
  ledcAttach(bluePin, 5000, 8);

  const char* homessid = "wifi_name";         //
  const char* homepassword = "wifi_password"; //
                                              //
  WiFi.begin(homessid, homepassword);         //
  Serial.print("Connecting");                 //
  wifisearchtime = millis();                  //
  while(WiFi.status() != WL_CONNECTED)        //
  {                                           //
      delay(500);                             //
      Serial.print(".");                      //
                                              //
      if(millis() - wifisearchtime > 10000)   //
      {                                       //     keep this code for home wifi but with backup self hosted wifi
        wifiConnected = false;                //     hash this if you want self hosted wifi
        break;                                //
      }                                       //
  }                                           //
  if(!wifiConnected)                          //
  {                                           //
    WiFi.softAP("Moodlamp Air", "password");  //
  }                                           //
  Serial.println();                           //
  Serial.println("Connected!");               //
  Serial.print("IP Address: ");               //
  Serial.println(WiFi.localIP());             //

//  WiFi.softAP("Moodlamp Air", "password");  //    keep this code instead if you just want self hosted wifi

  server.on("/", handleRoot);
  server.on("/colour", handleColour);
  server.on("/off", handleOff);
  server.on("/ocean", handleOcean);
  server.on("/night", handleNight);
  server.on("/white", handleWhite);
  server.on("/fireplace", handleFireplace);
  server.on("/sunrise", handleSunrise);
  server.on("/sunset", handleSunset);
  server.on("/airbus", handleAirbus);
  server.on("/boeing", handleBoeing);
  server.on("/brightness", handleBrightness);
  server.on("/aurora", handleAurora);
  server.on("/thunderstorm", handleThunderStorm);
  server.on("/rainbow", handleRainbow);
  server.on("/disco", handleDisco);
  server.on("/party", handleParty);
  server.on("/police", handlePolice);
  server.begin();

  //designing a startup animation
  const Colour AIRed{255, 14, 41, 100};
  const Colour AIGold{207, 150, 10, 100};
  const Colour AIPurple{110, 00, 210, 100};

  while(!compareColour(Current, AIRed))
  {
    Current = toTarget(Current, AIRed, medium);
    setColour(Current);
  }
  delay(1000);
  while(!compareColour(Current, AIGold))
  {
    Current = toTarget(Current, AIGold, medium);
    setColour(Current);
  }
  delay(1000);
  while(!compareColour(Current, AIPurple))
  {
    Current = toTarget(Current, AIPurple, medium);
    setColour(Current);
  }
  delay(1000);

  Colour input{analogRead(redPot)/16, analogRead(greenPot)/16, analogRead(bluePot)/16};
  while(!compareColour(Current, input))
  {
    Current = toTarget(Current, input, medium);
    setColour(Current);
  }
}

void loop()
{
  int webmode{analogRead(modePin)};

  //code to utilise website features
  if(webmode > 2000)
  {
    lastmode = web;
    server.handleClient();

    //flow to follow if ocean preset selected
    if(CurrentPreset == oceanPreset)
    {
      if(oceanState == transitioning)
      {
        if(millis() - lastTransitionStep > requestedTransitionDelay)
        {
          Current = toTarget(Current, Target);
          lastTransitionStep = millis();
        }
        if(compareColour(Current, Target))
        {
          oceanState = holding;
          requestedHoldingDelay = random(1, 100) < 70 ? random(200, 800) : random (1000, 4000);
          oceanStateStartTime = millis(); //we keep track of time to hold the colour a certain amount of time.
        }
      }
      else if(oceanState == holding)
      {
        if(millis() - oceanStateStartTime > requestedHoldingDelay)
        {
          Target = ocean[random(0,23)];
          requestedTransitionDelay = delayFinder(Current, Target, random(1500, 6000));
          lastTransitionStep = millis();
          oceanState = transitioning;
        }
      }

      if(rippleState == active)
      {
        int deltaBrightness{requestedRipple.amplitude * exp(-1 * requestedRipple.gamma * (millis() - rippleStateStartTime)) * 
                            sin(requestedRipple.omega * (millis() - rippleStateStartTime))};
        Current.brightness = ((100 + deltaBrightness) * brightnessScale) / 100;

        if((millis() - rippleStateStartTime) > requestedRipple.rippleTime)
        {
          rippleState = passive;
          Current.brightness = (100 * brightnessScale) / 100;
          rippleStateStartTime = millis();
          requestedRippleDelay = (random(1, 101) < 20 ? random(5000, 10000) : random(1000, 4000));
        }
      }
      else if(rippleState == passive)
      {
        Current.brightness = (100 * brightnessScale) / 100;
        if(millis() - rippleStateStartTime > requestedRippleDelay)
        {
          rippleStateStartTime = millis(); //we keep track of time to run the damped oscillation calculations.
          rippleState = active;
          requestedRipple = generateRipple();
        }
      }
      setColour(Current);
    }

    //flow to follow if fireplace preset is selected
    else if(CurrentPreset == fireplace)
    {
      if(fireplaceState == firetransitioning)
      {
        if(millis() - lastTransitionStep > requestedTransitionDelay)
        {
          Current = toTarget(Current, Target);
          lastTransitionStep = millis();
        }
        if(compareColour(Current, Target))
        {
          fireplaceState = fireholding;
          requestedHoldingDelay = random(100) < 35 ? random(2000, 5000) : random(500, 1500);
          fireplaceStateStartTime = millis(); //we keep track of time to hold the colour a certain amount of time.
        }
      }

      else if(fireplaceState == fireholding)
      {
        if(millis() - fireplaceStateStartTime > requestedHoldingDelay)
        {
          if(requestedFlicker.amplitude > 30)
          {
              // Strong flare
              Target = fireplacecolours[random(18,24)];
          }
          else if(requestedFlicker.amplitude < 20)
          {
              // Calm burning
              Target = fireplacecolours[random(10,15)];
          }
          else
          {
              // Normal flame
              Target = fireplacecolours[random(3,18)];
          }
          requestedTransitionDelay = delayFinder(Current, Target, random(1000, 5000));
          lastTransitionStep = millis();
          fireplaceState = firetransitioning;
        }
      }

      if(flickerState == flickeractive)
      {
        int deltaBrightness{requestedFlicker.amplitude * pow(sin(requestedFlicker.omega * (millis() - flickerStateStartTime)), 2)};
        Current.brightness = ((100 + deltaBrightness) * brightnessScale) / 100;

        if((millis() - flickerStateStartTime) > requestedFlicker.flickerTime)
        {
          flickerState = flickerpassive;
          Current.brightness = brightnessScale;
          flickerStateStartTime = millis();
          requestedFlickerDelay = random(1, 101) < 70 ? random(200, 300) : random(800, 1200);
        }
      }
      else if(flickerState == flickerpassive)
      {
        Current.brightness = brightnessScale;
        if(millis() - flickerStateStartTime > requestedFlickerDelay)
        {
          flickerStateStartTime = millis(); //we keep track of time to run the oscillation calculations.
          flickerState = flickeractive;
          requestedFlicker = generateFlicker();
        }
      }
      setColour(Current);
    }


    //flow to follow if sunrise preset is selected
    else if(CurrentPreset == sunrise)
    {
      if(sunriseState == sunrisetransitioning)
      {
        if(millis() - lastTransitionStep > requestedTransitionDelay)
        {
          Current = toTarget(Current, Target);
          lastTransitionStep = millis();
        }
        if(compareColour(Current, Target))
        {
          sunriseState = sunriseholding;
          requestedHoldingDelay = random(3000, 5000); //3 to 5 sec
          sunriseStateStartTime = millis(); //we keep track of time to hold the colour a certain amount of time.
        }
      }

      else if(sunriseState == sunriseholding)
      {
        if(millis() - sunriseStateStartTime > requestedHoldingDelay)
        {
          if(sunriseindex < 23) sunriseindex++;
          else if(sunriseindex == 23) sunriseindex = 0;
          Target = sunrisecolours[sunriseindex];
          requestedTransitionDelay = delayFinder(Current, Target, random(10000, 15000));
          lastTransitionStep = millis();
          sunriseState = sunrisetransitioning;
        }
      }

      if(flickerState == flickeractive)
      {
        int deltaBrightness{requestedFlicker.amplitude * sin(requestedFlicker.omega * (millis() - flickerStateStartTime))};
        Current.brightness = ((100 + deltaBrightness) * brightnessScale) / 100;

        if((millis() - flickerStateStartTime) > requestedFlicker.flickerTime)
        {
          flickerState = flickerpassive;
          Current.brightness = brightnessScale;
          flickerStateStartTime = millis();
          requestedFlickerDelay = random(1, 101) < 98 ? random(15000, 25000) : random(800, 1200);
        }
      }
      else if(flickerState == flickerpassive)
      {
        Current.brightness = brightnessScale;
        if(millis() - flickerStateStartTime > requestedFlickerDelay && sunriseindex > 11)
        {
          flickerStateStartTime = millis(); //we keep track of time to run the oscillation calculations.
          flickerState = flickeractive;
          requestedFlicker = {random(8, 15), 1, random(1, 100) < 5 ? random(250, 350) : random(2000, 3000), 0};
          requestedFlicker.omega = (2.0f * requestedFlicker.flickers * PI / requestedFlicker.flickerTime);
        }
      }
      setColour(Current);
    }


    //flow to follow if sunset preset is selected
    else if(CurrentPreset == sunset)
    {
      if(sunriseState == sunrisetransitioning)
      {
        if(millis() - lastTransitionStep > requestedTransitionDelay)
        {
          Current = toTarget(Current, Target);
          lastTransitionStep = millis();
        }
        if(compareColour(Current, Target))
        {
          sunriseState = sunriseholding;
          requestedHoldingDelay = random(3000, 5000); //3 to 5 sec
          sunriseStateStartTime = millis(); //we keep track of time to hold the colour a certain amount of time.
        }
      }

      else if(sunriseState == sunriseholding)
      {
        if(millis() - sunriseStateStartTime > requestedHoldingDelay)
        {
          if(sunriseindex > 0) sunriseindex--;
          else if(sunriseindex == 0) sunriseindex = 23;
          Target = sunrisecolours[sunriseindex];
          requestedTransitionDelay = delayFinder(Current, Target, random(10000, 15000));
          lastTransitionStep = millis();
          sunriseState = sunrisetransitioning;
        }
      }

      if(flickerState == flickeractive)
      {
        int deltaBrightness{requestedFlicker.amplitude * sin(requestedFlicker.omega * (millis() - flickerStateStartTime))};
        Current.brightness = ((100 + deltaBrightness) * brightnessScale) / 100;

        if((millis() - flickerStateStartTime) > requestedFlicker.flickerTime)
        {
          flickerState = flickerpassive;
          Current.brightness = brightnessScale;
          flickerStateStartTime = millis();
          requestedFlickerDelay = random(1, 101) < 98 ? random(15000, 25000) : random(800, 1200);
        }
      }
      else if(flickerState == flickerpassive)
      {
        Current.brightness = brightnessScale;
        if(millis() - flickerStateStartTime > requestedFlickerDelay && sunriseindex > 11)
        {
          flickerStateStartTime = millis(); //we keep track of time to run the oscillation calculations.
          flickerState = flickeractive;
          requestedFlicker = {random(8, 15), 1, random(1, 100) < 5 ? random(250, 350) : random(2000, 3000), 0};
          requestedFlicker.omega = (2.0f * requestedFlicker.flickers * PI / requestedFlicker.flickerTime);
        }
      }
      setColour(Current);
    }

    //flow to follow if aurora selected
    else if(CurrentPreset == aurora)
    {
      if(auroraState == auroratransitioning)
      {
        if(millis() - lastTransitionStep > requestedTransitionDelay)
        {
          Current = toTarget(Current, Target);
          lastTransitionStep = millis();
        }
        if(compareColour(Current, Target))
        {
          auroraState = auroraholding;
          requestedHoldingDelay = random(100) < 35 ? random(7000, 1000) : random(2000, 5000);
          auroraStateStartTime = millis(); //we keep track of time to hold the colour a certain amount of time.
        }
      }

      else if(auroraState == auroraholding)
      {
        if(millis() - auroraStateStartTime > requestedHoldingDelay)
        {
          Target = auroracolours[random(1, 24)];
          requestedTransitionDelay = delayFinder(Current, Target, random(2500, 7000));
          lastTransitionStep = millis();
          auroraState = auroratransitioning;
        }
      }

      if(undulationState == undulationactive)
      {
        int deltaBrightness{requestedUndulation.amplitude * sin(requestedUndulation.omega * (millis() - undulationStateStartTime)) 
                            * requestedUndulation.sign};
        Current.brightness = ((100 + deltaBrightness) * brightnessScale) / 100;

        if((millis() - undulationStateStartTime) > requestedUndulation.undulationTime)
        {
          undulationState = undulationpassive;
          Current.brightness = brightnessScale;
          undulationStateStartTime = millis();
          requestedUndulationDelay = random(1, 100) < 70 ? random(1500, 3000) : random(5000, 7000);
        }
      }
      else if(undulationState == undulationpassive)
      {
        Current.brightness = brightnessScale;
        if(millis() - undulationStateStartTime > requestedUndulationDelay)
        {
          undulationStateStartTime = millis(); //we keep track of time to run the oscillation calculations.
          undulationState = undulationactive;
          requestedUndulation = generateUndulation();
        }
      }
      setColour(Current);
    }

    //flow to follow if thunderstorm selected
    else if(CurrentPreset == thunderstorm)
    {
      if(stormState == stormtransitioning)
      {
        if(millis() - lastTransitionStep > requestedTransitionDelay)
        {
          Current = toTarget(Current, Target);
          lastTransitionStep = millis();
        }
        if(compareColour(Current, Target))
        {
          stormState = stormholding;
          requestedHoldingDelay = random(100) < 30 ? random(2000, 4000) : random(6000, 10000);
          stormStateStartTime = millis(); //we keep track of time to hold the colour a certain amount of time.
        }
      }

      else if(stormState == stormholding)
      {
        if(millis() - stormStateStartTime > requestedHoldingDelay)
        {
          Target = stormcolours[random(1, 24)];
          requestedTransitionDelay = delayFinder(Current, Target, random(1, 100) < 30 ? random(1500, 3000) : random(4000, 7000));
          lastTransitionStep = millis();
          stormState = stormtransitioning;
        }
      }

      if(lightningState == lightningactive)
      {
        if(hsf( sin(requestedLightning.omega * (millis() - lightningStateStartTime)) ) == 1)
        {
          Current = LightningTarget;
          Current.brightness = (requestedLightning.brightness * brightnessScale) / 100;
        }
        else
        {
          Current = BeforeLightning;
          Current.brightness = brightnessScale;
        }

        if((millis() - lightningStateStartTime) > requestedLightning.lightningTime)
        {
          lightningState = lightningpassive;
          Current = BeforeLightning;
          Current.brightness = brightnessScale;
          lightningStateStartTime = millis();

          int temp = random(1, 100);
          if(temp < 50) requestedLightningDelay = random(100, 500);
          else if(temp >= 50 && temp < 60) requestedLightningDelay = random(500, 1500);
          else if(temp >= 60 && temp < 70) requestedLightningDelay = random(1500, 6000);
          else if(temp >= 70 && temp < 90) requestedLightningDelay = random(6000, 8000);
          else requestedLightningDelay = random(8000, 10000);
        }
      }
      else if(lightningState == lightningpassive)
      {
        Current.brightness = brightnessScale;
        if(millis() - lightningStateStartTime > requestedLightningDelay)
        {
          lightningStateStartTime = millis(); //we keep track of time to run the oscillation calculations.
          lightningState = lightningactive;
          requestedLightning = generateLightning();
          LightningTarget = lightningcolours[random(1, 6)];
          BeforeLightning = Current;
        }
      }
      setColour(Current);
    }

    //code to make airbus strobe and beacon come into action
    else if(CurrentPreset == airbus)
    {

      Current = {200, 200, 200, brightnessScale};
      setColour(Current);
      delay(50);
      server.handleClient();
      Current = {0, 0, 0, brightnessScale};
      setColour(Current);
      delay(50);
      server.handleClient();
      Current = {200, 200, 200, brightnessScale};
      setColour(Current);
      delay(50);
      server.handleClient();
      Current = {0, 0, 0, brightnessScale};
      setColour(Current);
      for(int i = 0; i < 410; i += 10)
      {
        delay(10);
        server.handleClient();
      }

      Current = {255, 0, 0, brightnessScale};
      setColour(Current);
      delay(90);
      server.handleClient();
      Current = {0, 0, 0, brightnessScale};
      setColour(Current);
      for(int i = 0; i < 410; i += 10)
      {
        delay(10);
        server.handleClient();
      }
    }


    //code to make boeing strobe and beacon come into action
    else if(CurrentPreset == boeing)
    {
      Current = {200, 200, 200, brightnessScale};
      setColour(Current);
      delay(280);
      server.handleClient();
      Current = {255, 0, 0, brightnessScale};
      setColour(Current);
      delay(270);
      server.handleClient();
      Current = {0, 0, 0, brightnessScale};
      setColour(Current);
      for(int i = 0; i < 790; i += 10)
      {
        delay(10);
        server.handleClient();
      }
    }

    //rainbow flow preset
    else if(CurrentPreset == rainbow)
    {
      if(compareColour(Current, Target))
      {
        if(rainbowindex == 13)
        {
          rainbowindex = 0;
          delay(100);
        }
        else rainbowindex++;
        Target = rainbowcolours[rainbowindex];
      }
      else
      {
        if(rainbowindex == 1) Current = toTarget(Current, Target, vslow);
        else Current = toTarget(Current, Target, slow);
        Current.brightness = brightnessScale;
        setColour(Current);
      }
    }

    //disco preset (random transitions between visible spectrum)
    else if(CurrentPreset == disco)
    {
      if(compareColour(Current, Target))
      {
        Target = rainbowcolours[random(0, 14)];
        discoSpeedDecider = random(1, 100);
        delay(random(100, 500));
      }
      else
      {
        if(discoSpeedDecider < 10) Current = toTarget(Current, Target, slow);
        else if(discoSpeedDecider >= 10 && discoSpeedDecider < 40) Current = toTarget(Current, Target, medium);
        else if(discoSpeedDecider >= 40 && discoSpeedDecider < 80) Current = toTarget(Current, Target, fast);
        else Current = toTarget(Current, Target, vfast);
        Current.brightness = brightnessScale;
        setColour(Current);
      }
    }


    //party preset (random bullshit, highest entropy)
    else if(CurrentPreset == party)
    {
      //smootly drift colours
      if(partyMode == smoothdrift)
      {
        if(compareColour(Current, Target))
        {
          Target = rainbowcolours[random(0, 14)];
          discoSpeedDecider = random(1, 100);
          delay(random(100, 500));
        }
        else
        {
          if(discoSpeedDecider < 40) Current = toTarget(Current, Target, medium);
          else if(discoSpeedDecider >= 40 && discoSpeedDecider < 80) Current = toTarget(Current, Target, fast);
          else Current = toTarget(Current, Target, vfast);
          Current.brightness = brightnessScale;
        }

        if((millis() - partyStateStartTime) > requestedPartyModeDelay)
        {
          partyMode = modedecider;
        }
      }
      //gradually speeding up beats
      else if(partyMode == beatbuildup)
      {
        if(beatStage == slowbeats)
        {
          Current = Target;
          Current.brightness = beatBrightness * brightnessScale / 100;
          setColour(Current);
          delay(240);
          Current = {0, 0, 0, 100};
          setColour(Current);
          delay(240);
          beatBrightness += 4;
          if(beatBrightness >= 100) beatStage = midbeats;
        }
        else if(beatStage == midbeats)
        {
          Current = Target;
          Current.brightness = brightnessScale;
          setColour(Current);
          delay(120);
          Current = {0, 0, 0, 100};
          setColour(Current);
          delay(120);
          beatBrightness -= 5;
          if(beatBrightness <= 10) beatStage = fastbeats;
        }
        else if(beatStage == fastbeats)
        {
          Current = Target;
          Current.brightness = brightnessScale;
          setColour(Current);
          delay(60);
          Current = {0, 0, 0, 100};
          setColour(Current);
          delay(60);
          beatBrightness += 5;
          if(beatBrightness >= 100)
          {
            delay(1920);
            partyMode = basebeats;
            Target = rainbowcolours[random(0, 14)];
            Current = Target;
            beatBrightness = 100;
            partyStateStartTime = millis();
            requestedPartyModeDelay = 7680;
          }
        }
      }
      //changing colours with base like brightness effect
      else if(partyMode == basebeats)
      {
        if(beatBrightness > 20)
        {
          beatBrightness--;
          Current.brightness = beatBrightness * brightnessScale / 100;
          setColour(Current);
          delay(6);
        }
        else beatBrightness = 100;
        Current.brightness = beatBrightness * brightnessScale / 100;

        if((millis() - partyStateStartTime) > requestedPartyModeDelay)
        {
          partyMode = modedecider;
        }
      }
      //sudden persistant darkness
      else if(partyMode == blackout)
      {
        if((millis() - lastTransitionStep) > requestedTransitionDelay)
        {
          Current = toTarget(Current, Target);
          lastTransitionStep = millis();
        }

        if((millis() - partyStateStartTime) > requestedPartyModeDelay)
        {
          partyMode = modedecider;
        }
      }

      //white or coloured light strobing at constant random rate
      else if(partyMode == strobe)
      {
        if(strobeState == off)
        {
          Current = {0, 0, 0, brightnessScale};
          setColour(Current);
          delay(stroberate);
          strobeState = on;
        }
        else if(strobeState == on)
        {
          if(random(1, 100) < 10) Target = rainbowcolours[random(0, 14)];
          Current = Target;
          Current.brightness = brightnessScale;
          setColour(Current);
          delay(stroberate);
          strobeState = off;
        }

        if((millis() - partyStateStartTime) > requestedPartyModeDelay)
        {
          partyMode = modedecider;
        }
      }

      //aggressive rainbow transitions
      else if(partyMode == colourexplosion)
      {
        if(compareColour(Current, Target))
        {
          if(rainbowindex == 13)
          {
            rainbowindex = 0;
          }
          else rainbowindex++;
          Target = rainbowcolours[rainbowindex];
        }
        else
        {
          Current = toTarget(Current, Target, vfast);
          Current.brightness = brightnessScale;
        }

        if((millis() - partyStateStartTime) > requestedPartyModeDelay)
        {
          partyMode = modedecider;
        }
      }

      //one persistant colour
      else if(partyMode == frozencolours)
      {
        if((millis() - lastTransitionStep) > requestedTransitionDelay)
        {
          Current = toTarget(Current, Target);
          Current.brightness = brightnessScale;
          lastTransitionStep = millis();
        }

        if((millis() - partyStateStartTime) > requestedPartyModeDelay)
        {
          partyMode = modedecider;
        }
      }
      //end of party mode functions
      //decide which party mode is next
      else if(partyMode == modedecider)
      {
        int decision = random(1, 70);

        //smoothdrift decision
        if(decision < 10)
        {
          partyMode = smoothdrift;
          Target = rainbowcolours[random(0, 14)];
          requestedPartyModeDelay = random(5000, 8000);
          lastTransitionStep = millis();
          partyStateStartTime = millis();
        }

        //beatbuildup decision
        else if(decision >= 10 && decision < 20)
        {
          partyMode = beatbuildup;
          beatStage = slowbeats;
          beatBrightness = 40;
          Target = random(1, 100) < 50 ? Colour{255, 255, 255, 100} : rainbowcolours[random(0, 14)];

        }

        //basebeats decision
        else if(decision >= 20 && decision < 30)
        {
          partyMode = basebeats;
          Target = rainbowcolours[random(0, 14)];
          Current = Target;
          requestedPartyModeDelay = random(6000, 10000);
          partyStateStartTime = millis();
        }

        //blackout decision
        else if(decision >= 30 && decision < 40)
        {
          partyMode = blackout;
          Target = {0, 0, 0, 100};
          requestedPartyModeDelay = random(2000, 3000);
          requestedTransitionDelay = delayFinder(Current, Target, random(500, 1500));
          partyStateStartTime = millis();
        }

        //strobe decision
        else if(decision >= 40 && decision < 50)
        {
          partyMode = strobe;
          Target = random(1, 100) < 50 ? Colour{255, 255, 255, 100} : rainbowcolours[random(0, 14)];
          requestedPartyModeDelay = random(2000, 4000);
          stroberate = random(50, 150);
          partyStateStartTime = millis();
        }

        //colour explosion decision
        else if(decision >= 50 && decision < 60)
        {
          partyMode = colourexplosion;
          rainbowindex = random(0, 14);
          Target = rainbowcolours[rainbowindex];
          requestedPartyModeDelay = random(4000, 6000);
          partyStateStartTime = millis();
        }

        //frozen colour decision
        else if(decision >= 60 && decision < 70)
        {
          partyMode = frozencolours;
          Target = rainbowcolours[random(0, 14)];
          requestedPartyModeDelay = random(2000, 5000);
          requestedTransitionDelay = delayFinder(Current, Target, random(500, 1500));
          partyStateStartTime = millis();
        }
      }
      setColour(Current);
    }

    //police preset (strobing red and blue)
    else if(CurrentPreset == police)
    {
      if(policeMode == 1)
      {
        Current = {255, 0, 20, brightnessScale};
        setColour(Current);
        delay(80);
        server.handleClient();
        Current = {0, 0, 0, brightnessScale};
        setColour(Current);
        delay(80);
        server.handleClient();
        Current = {255, 0, 20, brightnessScale};
        setColour(Current);
        delay(80);
        server.handleClient();
        Current = {0, 0, 0, brightnessScale};
        setColour(Current);
        for(int i = 0; i < 180; i += 10)
        {
          delay(10);
          server.handleClient();
        }

        Current = {0, 20, 200, brightnessScale};
        setColour(Current);
        delay(80);
        server.handleClient();
        Current = {0, 0, 0, brightnessScale};
        setColour(Current);
        delay(80);
        server.handleClient();
        Current = {0, 20, 200, brightnessScale};
        setColour(Current);
        delay(80);
        server.handleClient();
        Current = {0, 0, 0, brightnessScale};
        setColour(Current);
        for(int i = 0; i < 180; i += 10)
        {
          delay(10);
          server.handleClient();
        }
        policeMode = random(1, 100) < 30 ? 2 : 1;
      }
      else if(policeMode == 2)
      {
        Current = {255, 0, 20, brightnessScale};
        setColour(Current);
        delay(200);
        Current = {0, 0, 0, brightnessScale};
        setColour(Current);
        for(int i = 0; i < 120; i += 10)
        {
          delay(10);
          server.handleClient();
        }

        Current = {0, 20, 200, brightnessScale};
        setColour(Current);
        delay(200);
        Current = {0, 0, 0, brightnessScale};
        setColour(Current);
        for(int i = 0; i < 120; i += 10)
        {
          delay(10);
          server.handleClient();
        }
        policeMode = random(1, 100) < 30 ? 1 : 2;
      }
    }


    else if(CurrentPreset == none)
    {
      if(!compareColour(Current, Target))
      {
        Current = toTarget(Current, Target, fast);
        Current.brightness = brightnessScale;
        setColour(Current);
      }
      Current.brightness = brightnessScale;
      setColour(Current);
    }
  }

  
  //code to keep tracking the pots to change the colour instantaneously
  else
  {
    Colour input{analogRead(redPot)/16, analogRead(greenPot)/16, analogRead(bluePot)/16};
    if(lastmode == web)
    {
      while(!compareColour(Current, input))
      {
        Current = toTarget(Current, input, fast);
        setColour(Current);
      }
    }
    else Current = toTarget(Current, input, instant);
    setColour(Current);
    lastmode = pot;
  }
}
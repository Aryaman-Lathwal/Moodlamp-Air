void handleRoot()
{
  const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta name="viewport" content="width=device-width, initial-scale=1">

<style>

body{
    margin:0;
    padding:30px;
    text-align:center;

    background:linear-gradient(135deg,#2b0010,#1f001f,#140028);

    color:white;

    font-family:Arial,Helvetica,sans-serif;
}

h1{
    margin-bottom:5px;

    color:#d4af37;

    text-shadow:0 0 8px rgba(212,175,55,0.35);
}

.subtitle{
    color:#bbbbbb;
    margin-bottom:35px;
    font-size:16px;
}

.section{

    max-width:420px;
    margin:auto;
    margin-bottom:28px;

    padding:20px;

    border-radius:20px;

    background:rgba(255,255,255,0.06);

    backdrop-filter:blur(5px);

    box-shadow:
        0 0 15px rgba(0,0,0,0.35);

}

.section h2{

    margin-top:0;
    color:#d4af37;
}

.sliderLabel{
    display:flex;
    justify-content:space-between;
    align-items:center;

    width:100%;

    font-size:18px;
    margin-top:18px;
}
.sliderLabel span:last-child{
    margin-left:auto;
    color:#d4af37;
    font-family:monospace;
}

input[type=range]{
    width:100%;
    margin-top:10px;
    margin-bottom:10px;

    accent-color:#d4af37;
}


button{

    width:100%;

    margin-top:12px;

    padding:14px;

    font-size:17px;

    color:white;

    border:none;

    border-radius:15px;

    background:rgba(255,255,255,0.08);

    transition:0.18s;

    cursor:pointer;
}

button:hover{

    background:rgba(212,175,55,0.22);

    transform:scale(1.02);
}

.off{

    background:rgba(180,30,30,0.35);
}

.off:hover{

    background:rgba(220,40,40,0.55);
}

.footer{

    margin-top:25px;

    color:#888;

    font-size:13px;
}

</style>

</head>

<body>

<h1>Moodlamp Air</h1>

<div class="subtitle">
Smart Ambient Lighting
</div>


<div class="section">

<h2>Manual Colour</h2>

<div class="sliderLabel">
    <span>Red</span>
    <span id="redValue">0</span>
</div>

<input
id="red"
type="range"
min="0"
max="255"
value="0"
oninput="updateColour()">

<div class="sliderLabel">
    <span>Green</span>
    <span id="greenValue">0</span>
</div>

<input
id="green"
type="range"
min="0"
max="255"
value="0"
oninput="updateColour()">

<div class="sliderLabel">
    <span>Blue</span>
    <span id="blueValue">0</span>
</div>

<input
id="blue"
type="range"
min="0"
max="255"
value="0"
oninput="updateColour()">

</div>


<div class="section">

<h2>Preset Brightness</h2>

<input
id="brightness"
type="range"
min="10"
max="100"
value="100"
oninput="updateBrightness()">

</div>



<div class="section">

<h2>Nature</h2>

<button onclick="fetch('/ocean')">
Ocean Waves
</button>

<button onclick="fetch('/aurora')">
Aurora
</button>

<button onclick="fetch('/thunderstorm')">
Thunderstorm
</button>

<button onclick="fetch('/sunrise')">
Sunrise
</button>

<button onclick="fetch('/sunset')">
Sunset
</button>

<button onclick="fetch('/fireplace')">
Fire place
</button>

</div>


<div class="section">

<h2>Special</h2>

<button onclick="fetch('/night')">
Night Lamp
</button>

<button onclick="fetch('/white')">
Cool White
</button>

<button onclick="fetch('/airbus')">
Airbus Strobe
</button>

<button onclick="fetch('/boeing')">
Boeing Strobe
</button>

</div>


<div class="section">

<h2>Entertainment</h2>

<button onclick="fetch('/rainbow')">
Rainbow Flow
</button>

<button onclick="fetch('/disco')">
Disco
</button>

<button onclick="fetch('/party')">
Party
</button>

<button onclick="fetch('/police')">
Police
</button>

</div>


<div class="section">

<h2>Power</h2>

<button
class="off"
onclick="fetch('/off')">

Turn Off

</button>

</div>


<div class="footer">
Designed with <3 on ESP32
</div>

<script>
function paintSlider(slider)
{
    const min = slider.min;
    const max = slider.max;

    const percent = (slider.value-min)/(max-min)*100;

    slider.style.background =
        `linear-gradient(to right,
        #8a6500 0%,
        #d4af37 ${percent-6}%,
        #fff3a6 ${percent}%,
        #444 ${percent}%,
        #444 100%)`;
}


function updateColour()
{
    const r = document.getElementById("red").value;
    const g = document.getElementById("green").value;
    const b = document.getElementById("blue").value;

    document.getElementById("redValue").textContent = r;
    document.getElementById("greenValue").textContent = g;
    document.getElementById("blueValue").textContent = b;

    fetch("/colour?r=" + r + "&g=" + g + "&b=" + b);
    paintSlider(document.getElementById("red"));
    paintSlider(document.getElementById("green"));
    paintSlider(document.getElementById("blue"));
}

function updateBrightness()
{
    const brightness = document.getElementById("brightness").value;

    fetch("/brightness?value=" + brightness);
    paintSlider(document.getElementById("brightness"));
}


</script>

</body>
</html>

)rawliteral";

  server.send(200, "text/html", webpage);
}


void handleColour()
{
    CurrentPreset = none;

    Target.red = server.arg("r").toInt();
    Target.green = server.arg("g").toInt();
    Target.blue = server.arg("b").toInt();
    Target.brightness = 100;

    server.send(200, "text/plain", "");
}

void handleOff()
{
  CurrentPreset = none;
  Target = {0,0,0,100};

  server.sendHeader("Location", "/");
  server.send(303);
}

void handleBrightness()
{
    brightnessScale = server.arg("value").toInt();

    server.send(200, "text/plain", "");
}

void handleSunrise()
{
    CurrentPreset = sunrise;

    //pick the first colour to start the sunrise from.
    sunriseindex = 0;
    Target = sunrisecolours[0];

    //begin transitioning
    requestedTransitionDelay = delayFinder(Current, Target, random(2000, 3000));

    sunriseState = sunrisetransitioning;

    //reset flicker timing;
    flickerState = flickerpassive;
    flickerStateStartTime = millis();
    requestedFlickerDelay = random(20000, 30000);
    requestedFlicker = {random(15, 25), 1, random(350, 550), 0};
    requestedFlicker.omega = (2.0f * requestedFlicker.flickers * PI / requestedFlicker.flickerTime);

    Current.brightness = 100;
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleSunset()
{
    CurrentPreset = sunset;

    //pick the first colour to start the sunrise from.
    sunriseindex = 23;
    Target = sunrisecolours[sunriseindex];

    //begin transitioning
    requestedTransitionDelay = delayFinder(Current, Target, random(2000, 3000));

    sunriseState = sunrisetransitioning;

    //reset flicker timing;
    flickerState = flickerpassive;
    flickerStateStartTime = millis();
    requestedFlickerDelay = random(20000, 30000);
    requestedFlicker = {random(15, 25), 1, random(350, 550), 0};
    requestedFlicker.omega = (2.0f * requestedFlicker.flickers * PI / requestedFlicker.flickerTime);

    Current.brightness = 100;
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleOcean()
{
    CurrentPreset = oceanPreset;

    // Pick a fresh random target colour
    Target = ocean[random(0, 23)];

    // Begin transitioning immediately
    requestedTransitionDelay = delayFinder(Current, Target, random(2000, 3000));

    oceanState = transitioning;

    // Reset ripple timing
    rippleState = passive;
    rippleStateStartTime = millis();
    requestedRippleDelay = (random(1,101) < 40) ? random(1000,5000) : random(6000, 10000);
    requestedRipple = generateRipple();

    Current.brightness = 100;

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleFireplace()
{
    CurrentPreset = fireplace;

    //pick a fresh random target colour
    Target = fireplacecolours[random(0, 24)];

    // Begin transitioning immediately
    requestedTransitionDelay = delayFinder(Current, Target, random(2000, 3000));

    fireplaceState = firetransitioning;

    // Reset flicker timing
    flickerState = flickerpassive;
    flickerStateStartTime = millis();
    requestedFlickerDelay = random(100) < 15 ? random(2000, 5000) : random(100, 1000);
    requestedFlicker = generateFlicker();

    Current.brightness = 100;

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAurora()
{
    CurrentPreset = aurora;

    //pick a fresh random target colour
    Target = auroracolours[random(0, 24)];

    // Begin transitioning immediately
    requestedTransitionDelay = delayFinder(Current, Target, random(2000, 3000));

    auroraState = auroratransitioning;

    // Reset undulation timing
    undulationState = undulationpassive;
    undulationStateStartTime = millis();
    requestedUndulationDelay = random(100) < 70 ? random(1500, 3000) : random(5000, 7000);
    requestedUndulation = generateUndulation();

    Current.brightness = 100;

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleThunderStorm()
{
    CurrentPreset = thunderstorm;

    //pick a fresh random target colour
    Target = stormcolours[random(0, 24)];

    // Begin transitioning immediately
    requestedTransitionDelay = delayFinder(Current, Target, random(2000, 3000));

    stormState = stormtransitioning;

    // Reset lightning timing
    lightningState = lightningpassive;
    lightningStateStartTime = millis();
    requestedLightningDelay = random(100) < 70 ? random(2000, 5000) : random(800, 1500);
    requestedLightning = generateLightning();

    Current.brightness = 100;

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleNight()
{
    CurrentPreset = none;

    // Warm dim night light
    Target = {255, 147, 41, 12};

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleWhite()
{
    CurrentPreset = none;

    // Bright cool white
    Target = {235, 245, 255, 100};

    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAirbus()
{
    CurrentPreset = airbus;
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleBoeing()
{
    CurrentPreset = boeing;
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleRainbow()
{
    CurrentPreset = rainbow;
    rainbowindex = 0;
    Target = rainbowcolours[rainbowindex];
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleDisco()
{
    CurrentPreset = disco;
    Target = rainbowcolours[random(0, 14)];
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleParty()
{
    CurrentPreset = party;
    partyMode = modedecider;
    server.sendHeader("Location", "/");
    server.send(303);
}

void handlePolice()
{
    CurrentPreset = police;
    server.sendHeader("Location", "/");
    server.send(303);
    policeMode = 1;
}
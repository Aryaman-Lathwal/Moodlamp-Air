# Moodlamp Air

Moodlamp Air is an ESP32-powered smart ambient lighting system built around high-power RGB LEDs. Rather than simply displaying static colours, it focuses on creating natural, dynamic lighting effects inspired by real-world phenomena such as ocean waves, auroras, thunderstorms, fireplaces and sunrises.

The lamp can be controlled through a responsive web interface hosted directly on the ESP32 and supports both standalone Access Point mode as well as Home Wi-Fi mode.

---

## Features

###  Manual RGB Control
- Live RGB sliders
- Adjustable global brightness
- Instant response over Wi-Fi

###  Nature Presets
- Ocean Waves
- Aurora
- Thunderstorm
- Sunrise
- Fireplace

Each preset is algorithmically generated using custom transition logic instead of fixed animations, ensuring that no two runs feel identical.

###  Special Modes
- Night Lamp
- Cool White
- Airbus Strobe (because I am an avgeek!)
- Boeing Strobe

###  Entertainment Modes
- Rainbow Flow
- Disco
- Police
- Party

---

## Hardware

- ESP32 Development Board
- Common-anode RGB LEDs
- MOSFET drivers
- 12V LED supply
- Buck converter for ESP32
- Potentiometers for physical, manual colour control (optional)

---

## Web Interface

The ESP32 hosts a modern responsive web interface allowing:

- RGB control
- Brightness adjustment
- One-click preset selection
- Mobile and desktop compatibility

The interface is designed to work well on both phones and computers.

---

## Networking

The firmware supports two modes:

### Station Mode
Connects to an existing Wi-Fi network.

### Access Point Mode
If no known Wi-Fi network is found during startup, the ESP32 automatically creates its own hotspot allowing direct control.

This means the lamp works both at home and completely offline.

---

## Animations

Unlike many RGB lamps that simply fade between colours, Moodlamp Air uses custom procedural animation systems.

Examples include:

- smooth colour interpolation
- ripple simulation
- brightness undulations
- stochastic lightning generation
- randomized fireplace flicker
- beat-synchronised party effects
- aircraft anti-collision strobes

Every preset has its own state machine, making each effect unique.

---

## Inspiration

It started with a table lamp that could change colours as desired, simply to set the mood. Eventually the project grew beyond what I had though initially. Building upon the capabilities of the ESP, mobile control was implemented and presets were developed. The presets are inspired by real-world lighting:

- Ocean waves
- Northern Lights
- Thunderstorms
- Fireplace flames
- Sunrise
- Airbus and Boeing anti-collision lighting

The goal was to create ambient lighting that feels alive rather than repetitive.

---

## Future Improvements

- Apple HomeKit integration
- Siri Shortcuts support
- Home Assistant integration
- Music-reactive lighting
- OTA firmware updates
- Custom user-defined presets
- Preset scheduling

---

## License

This project is released under the MIT License.

Feel free to modify, improve and build upon it.

---

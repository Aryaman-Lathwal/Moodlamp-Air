

//each colour struct has 4th member: brightness. the r g b values define ratio while brightness defines, well brightness
//so when we call setColour(), we dont set the values of ratio, but them computed against the brightness.
//generateRipple() function generates random but within bound values for such brightness oscillations and returns a Ripple struct.
Ripple generateRipple()
{
  Ripple ripple{0, random(6, 12), 0};
  //decide amplitude
  if(random(1, 100) < 65) ripple.amplitude = random(25, 35);
  else ripple.amplitude = random(35, 45);

  //decide no. of oscillations
  if(random(1, 100) < 60) ripple.oscillations = random(8, 13);
  else ripple.oscillations = random(5, 8);

  //set time of ripples
  if(random(1, 100) < 60) ripple.rippleTime = random(ripple.oscillations * 800, ripple.oscillations * 1300);
  else ripple.rippleTime = random(ripple.oscillations * 400, ripple.oscillations * 800);

  //set physics variables
  ripple.omega = (2.0f * ripple.oscillations * PI / ripple.rippleTime);
  ripple.gamma = (log(ripple.amplitude / 3.0f) / ripple.rippleTime);
  return ripple;
}

Flicker generateFlicker()
{
  Flicker flicker{0, random(1, 4), 0, 0};
  int amplitude{};
  if(random(1,100) < 70)
  {
    amplitude = random(1, 100) < 65 ? random(10, 26) : random(-10, -26);
  }
  else
  {
    amplitude = random(1, 100) < 65 ? random(26, 41) : random(-26, -41);
  }
  flicker.amplitude = amplitude;
  flicker.flickerTime = random(flicker.flickers * 350, flicker.flickers * 550);
  flicker.omega = (2.0f * flicker.flickers * PI / flicker.flickerTime);
  return flicker;
}

Undulation generateUndulation()
{
  Undulation undulation{random(15, 35), random(1, 3), random(1, 100) < 50 ? 1 : -1, random(2500, 7000), 0};
  undulation.omega = (2.0f * undulation.undulations * PI / undulation.undulationTime);
  return undulation;
}

Lightning generateLightning()
{
  Lightning lightning{};
  lightning.brightness = random(35, 100);

  //deciding no. of strikes, bright = lesser, dim = more.
  if(lightning.brightness < 60) lightning.strikes = random(1, 100) < 70 ? random(2, 4) : 1;
  else if(lightning.brightness >= 60 && lightning.brightness < 80) lightning.strikes = random(1, 3);
  else lightning.strikes = random(1, 100) < 70 ? 1 : 2;

  //deciding how short or long lightning sequences are
  lightning.lightningTime = random(80 * lightning.strikes , 120 * lightning.strikes);
  lightning.omega = (2.0f * lightning.strikes * PI / lightning.lightningTime);
  return lightning;
}
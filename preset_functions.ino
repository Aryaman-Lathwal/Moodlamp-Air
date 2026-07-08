#include <math.h>



//given an a desired time for colour transition, this function figures out the delay to use in toTarget()
//to achieve that time to transition from one colour to another. (input transtime should be in milliseconds)
int delayFinder(Colour current, Colour target, int transTime)
{
  int steps{}, maxSteps{};
  if((steps = fabs(current.red - target.red)) > maxSteps) maxSteps = steps;
  if((steps = fabs(current.green - target.green)) > maxSteps) maxSteps = steps;
  if((steps = fabs(current.blue - target.blue)) > maxSteps) maxSteps = steps;
  
  if(maxSteps == 0) return 0;
  int delay = transTime/maxSteps;
  return delay;
}

//this is an overloaded toTarget function and has no 3rd argument.
//the delay is controlled by millis() in the main code.
Colour toTarget(Colour current, Colour target)
{
  if(current.red < target.red) current.red++;
  else if(current.red > target.red) current.red--;

  if(current.green < target.green) current.green++;
  else if(current.green > target.green) current.green--;

  if(current.blue < target.blue) current.blue++;
  else if(current.blue > target.blue) current.blue--;
  
  return current;
}

//heaviside step function. returns 1 if argument is positive, 0 otherwise
int hsf(double x)
{
  if(x > 0) return 1;
  else return 0;
}

//float overload of hsf
int hsf(float x)
{
  if(x > 0) return 1;
  else return 0;
}

//integer overload of hsf
int hsf(int x)
{
  if(x > 0) return 1;
  else return 0;
}

/*===========================================================
 * Dual Ring LED 
 * 
 * Layering:
 * At the top level, we've defined a collection of pre-set patterns.
 * The user is able to specify which pattern to display, and at which speed.
 * This can be done either via the serial port or via Jeff's cool XBee remote.
 * 
 * One level down from that are the pattern definitions themselves.  Each pattern
 * has an associated init_ and move_ function.  The init_ function sets up the led array;
 * the move_ function is used to manipulate that array to cause "motion".  Note that our main
 * loop will call the appropriate move_ function for the selected pattern. 
 * 
 * The next level down are the user pattern definition functions.  These allow the user to fill
 * a given ring with a given color, create streaks, and create bumps.  There are also motion helpers
 * to rotate leds in either direction.
 * 
 * The next level down are "helper" functions...these do array math to do the associated rotates.
 * 
 * Finally, this is all built on the FastLED library that does the actual LED manipulations.
 */


/******=====  Begin DualLed.h HEADER =======*/
#include <SoftwareSerial.h>
#include <FastLED.h>

// Our LED Matrix:
// 16 LEDs in the inner loop, going couter-clockwise.
// 24 LEDs in the outer loop, going clockwise.
// These are helpful defines for where the loops start and end.
#define DUAL_RING_NUM_INNER  16
#define DUAL_RING_NUM_OUTER  24
#define DUAL_RING_LAST_INNER (DUAL_RING_NUM_INNER - 1)
#define DUAL_RING_LAST_OUTER (DUAL_RING_NUM_OUTER - 1)

typedef void (*dualRingFuncType)();

class DualRingLED
{
  public:
    DualRingLED(int pin);
    void begin(void);
    void run(void);
    void run(int delay);
    CRGB *innerLEDs;
    CRGB *outerLEDs;
    void setPattern(dualRingFuncType runFunc);

    void fillAll(CRGB color);
    void fillInner(CRGB color);
    void fillOuter(CRGB color);
    void rotateInnerClockwise(void);
    void rotateInnerCounterClockwise(void);
    void rotateOuterClockwise(void);
    void rotateOuterCounterClockwise(void);
    void makeInnerBump(int bumpSize, CRGB background, CRGB bump);
    void makeOuterBump(int bumpSize, CRGB background, CRGB bump);
    void makeInnerClockwiseStreak( int streakSize, CRGB background, CRGB Head);
    void makeInnerCounterClockwiseStreak(int streakSize, CRGB background, CRGB Head);
    void makeOuterClockwiseStreak( int streakSize, CRGB background, CRGB Head);
    void makeOuterCounterClockwiseStreak(int streakSize, CRGB background, CRGB Head);
    void drawInnerClockwiseStreak(int startIndex, int streakSize, CRGB head, CRGB tail);
    void drawInnerCounterClockwiseStreak(int startIndex, int streakSize, CRGB head, CRGB tail);
    void drawOuterClockwiseStreak(int startIndex, int streakSize, CRGB head, CRGB tail);
    void drawOuterCounterClockwiseStreak(int startIndex, int streakSize, CRGB head, CRGB tail);
    
    

  private:
    CRGB             _leds[40];
    dualRingFuncType _runFunc=NULL;
    int              _pin;

    /* I don't think these actually need to be in the class...*/
    void _rotateDownHelper( CRGB *startLed, int num );
    void _rotateUpHelper( CRGB *startLed, int num );
    void _drawStreakHelper( CRGB *ringStart, int ringSize, int streakStartIndex, int streakSize, CRGB startColor, CRGB endColor);
    void _makeBumpHelper(int centerLed, int bumpSize, CRGB background, CRGB bump);
  
};
/******======  Begin DualLed.cpp library file   *****/
// Hardware definitions for our LED strip.
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

// Our LED Matrix:
// 16 LEDs in the inner loop, going couter-clockwise.
// 24 LEDs in the outer loop, going clockwise.
// These are helpful defines for where the loops start and end.
#define NUM_LEDS    (DUAL_RING_NUM_INNER + DUAL_RING_NUM_OUTER)

#define DEFAULT_BRIGHTNESS 30

void DualRingLED::begin( void )
{
    //  HMMM...Fast LED doesn't like passing pin...must be a template thing.  
    //  I'm gonna cheat by case-switching this with constants.
    //  Assuming we're not using 0, 1, 2, or 3 (serial pins or xbee pins).  Also assuming not using analog pins.
    
    FastLED.addLeds<LED_TYPE, 6, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

    #if 0 
    switch (_pin)
    {
      case 4: FastLED.addLeds<LED_TYPE, 4, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 5: FastLED.addLeds<LED_TYPE, 5, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 6: FastLED.addLeds<LED_TYPE, 6, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 7: FastLED.addLeds<LED_TYPE, 7, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 8: FastLED.addLeds<LED_TYPE, 8, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 9: FastLED.addLeds<LED_TYPE, 9, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 10: FastLED.addLeds<LED_TYPE, 10, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 11: FastLED.addLeds<LED_TYPE, 11, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 12: FastLED.addLeds<LED_TYPE, 12, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      case 13: FastLED.addLeds<LED_TYPE, 13, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); break;
      default: return;
    }
    #endif
    
    FastLED.setBrightness(  DEFAULT_BRIGHTNESS );
    
    // clear the array, just in case.

    fillAll(CRGB::Black);
    FastLED.show();
 
}

DualRingLED::DualRingLED(int pin)
{
  _pin = pin;
}

void DualRingLED::run( void )
{
  if (_runFunc)
  {
    _runFunc();
  }
  
  FastLED.show();
}

void DualRingLED::run( int delay )
{
  if (_runFunc)
  {
    _runFunc();
  }

  FastLED.show();
  FastLED.delay(delay);
}

void DualRingLED::setPattern(dualRingFuncType func)
{
  _runFunc = func;
}

/*===============================================================================
 * Function:  _rotateDownHelper
 *
 * Since the LEDs can be wired arbitrarily (is increasing the index going clockwise
 * or counter-clockwise), I'm gonna make these helpers just go "up" for increasing array
 * and "down" for decreasing array.  We can then map to clockwise and counter-clockwise
 * with the real rotate functions.
 */
void DualRingLED::_rotateDownHelper( CRGB *start_led, int num )
{
    CRGB roll_over_value;
    int i;
    
    // this is basically just gonna be a big shift with roll-over.
    
    // remember the "0th" value...it's gonna go into the "last" array value.
    roll_over_value = start_led[0];
    
    // now copy everything one slot "down"
    for (i=0; i< num -1; i++)
    {
        start_led[i] = start_led[i+1];
    }
    
    // Finally, store the last LED with that roll-over value.
    start_led[num - 1] = roll_over_value;
    
}  // end of _rotateDownHelper

/*===============================================================================
 * Function:  _rotateUpHelper
 *
 * Since the LEDs can be wired arbitrarily (is increasing the index going clockwise
 * or counter-clockwise), I'm gonna make these helpers just go "up" for increasing array
 * and "down" for decreasing array.  We can then map to clockwise and counter-clockwise
 * with the real rotate functions.
 */
void DualRingLED::_rotateUpHelper( CRGB *start_led, int num )
{
    CRGB roll_over_value;
    int i;
    
    // this is basically just gonna be a big shift with roll-over.
    
    // remember the "last" value...it's gonna go into the "first" array value.
    roll_over_value = start_led[num - 1];
    
    // now copy everything one slot "up"
    for (i = num - 1; i > 0; i--)
    {
        start_led[i] = start_led[i-1];
    }
    
    // Finally, store the first LED with that roll-over value.
    start_led[0] = roll_over_value;
    
}  // end of rotate_down_helper

/*===============================================================================
 * Function:  _drawStreakHelper
 *
 * In some instances, we want to draw a streak spanning the roll-over point.
 * This helper function does that.
 * Note this is heavier weight than the simple "make clockwise/counter-clockwise streak"
 * functions...and it doesn't fill in the background...it *JUST* updates the streak pixels.
 */
void DualRingLED::_drawStreakHelper( CRGB *ring_start, int ring_size, int streak_start_index, int streak_size, CRGB start_color, CRGB end_color)
{
   CRGB temp_led[DUAL_RING_NUM_OUTER];
   int  copy_index;
   int  num_copied=0;

   // a given streak may span our roll-over point.  
   // Exammple:  an inner streak of length 4, starting at led index 14 should light up
   // leds 14, 15, 0, and 1...but if we just use fill gradient from 14, it lights up
   // 14, 15, 16, and 17...two from the inner loop and two from the outer.
   //
   // One way to deal with this is to break the streak into two separate gradients..but then you need
   // to do color interpolation.   Instead, I'm going to have a temporary led array that *doesn't* 
   // roll over to do the fill_gradient (which will do the full interpolation for me), and then
   // copy the leds over to the right spots in our ring, dealing with the roll-over.

   // Start with some error checks...
   if (streak_size > DUAL_RING_NUM_OUTER) streak_size = DUAL_RING_NUM_OUTER;

   // make our gradient in the temp array.
   fill_gradient_RGB(temp_led, streak_size, start_color, end_color);

   // start by copying led up to the roll-over point
   copy_index = streak_start_index;
   while ((copy_index < ring_size) && (num_copied < streak_size))
   {
      ring_start[copy_index] = temp_led[num_copied];
      num_copied++;
      copy_index++;
   }

   // from here, we just rolled over...so start from the "zeroth" led.
   copy_index = 0;   

   // ...and copy the rest of the streak
   while (num_copied < streak_size)
   {
      ring_start[copy_index] = temp_led[num_copied];
      num_copied++;
      copy_index++;
   }
  
}

/*===============================================================================
 * Function:  _makeBumpHelper
 * 
 * bump size is the number of leds on either side of the bump led...so bump size of 2
 *   gives a TOTAL LED size of 5...one in the center, and 2 on either side.
 * 
 */
void DualRingLED::_makeBumpHelper(int center_led, int bump_size, CRGB background, CRGB bump)
{
  int start_led_index;
  
  // first cut here will treat rollovers as errors...do nothing.  
  // next cut can do modulo math to make the right thing happen.
  // note I'm also not dealing with the inner/outer loop rollover.
  if (bump_size < 1) return;
  if (center_led - bump_size < 0) return;
  if (center_led + bump_size > NUM_LEDS) return;

  start_led_index = center_led - bump_size;
  fill_gradient_RGB(&(_leds[start_led_index]), bump_size + 1, background, bump);
  fill_gradient_RGB(&(_leds[center_led]), bump_size+1, bump, background);
  
}


/*===============================================================================
 * Function:  fillAll
 */
void DualRingLED::fillAll(CRGB color)
{
    fill_solid(_leds, NUM_LEDS, color);
}

/*===============================================================================
 * Function:  fillInner
 */
void DualRingLED::fillInner(CRGB color)
{
    fill_solid(innerLEDs, DUAL_RING_NUM_INNER, color);
}

/*===============================================================================
 * Function:  fillOuter
 */
void DualRingLED::fillOuter(CRGB color)
{
    fill_solid(outerLEDs, DUAL_RING_NUM_OUTER, color);
}

/*===============================================================================
 * Function:  rotateInnerClockwise
 */
void DualRingLED::rotateInnerClockwise( void )
{
    _rotateDownHelper(innerLEDs, DUAL_RING_NUM_INNER);
}  


/*===============================================================================
 * Function:  rotateInnerCounterClockwise
 */
void DualRingLED::rotateInnerCounterClockwise( void )
{
    _rotateUpHelper(innerLEDs, DUAL_RING_NUM_INNER);
}  

/*===============================================================================
 * Function:  rotateOuterClockwise
 */
void DualRingLED::rotateOuterClockwise( void )
{
    _rotateUpHelper(outerLEDs, DUAL_RING_NUM_OUTER);  
} 
    
/*===============================================================================
 * Function:  rotateOuterCounterClockwise
 */
void DualRingLED::rotateOuterCounterClockwise( void )
{
    _rotateDownHelper(outerLEDs, DUAL_RING_NUM_OUTER);
}



/*===============================================================================
 * Function:  makeInnerBump
 */
void DualRingLED::makeInnerBump(int bump_size, CRGB background, CRGB bump)
{

  if (bump_size < 1) bump_size = 1;
  if (bump_size > 7) bump_size = 7;
  
  fillInner(background);

  // want the bump centered in the inner array, hence the 8
  _makeBumpHelper(8, bump_size, background, bump);
}

/*===============================================================================
 * Function:  makeOuterBump
 */
void DualRingLED::makeOuterBump(int bump_size, CRGB background, CRGB bump)
{
  if (bump_size < 1) bump_size = 1;
  if (bump_size > 11) bump_size = 11;
  
  fillOuter(background);

  // want the bump centered in the outer array, hence the 28
  _makeBumpHelper(28, bump_size, background, bump);
}

/*===============================================================================
 * Function:  makeInnerClockwiseStreak
 */
void DualRingLED::makeInnerClockwiseStreak(int streak_size, CRGB background, CRGB head)
{
  if (streak_size > DUAL_RING_NUM_INNER) streak_size = DUAL_RING_NUM_INNER;

  fillInner(background);

  // inner indexes go counter-clockwise.  
  // we're gonna put the head at index 0, and then fade as array indexes increase
  fill_gradient_RGB(innerLEDs, streak_size, head, background); 
}

/*===============================================================================
 * Function:  makeInnerCounterClockwiseStreak
 */
void DualRingLED::makeInnerCounterClockwiseStreak(int streak_size, CRGB background, CRGB head)
{
  if (streak_size > DUAL_RING_NUM_INNER) streak_size = DUAL_RING_NUM_INNER;

  fillInner(background);

  // since inner indexes go counter-clockwise, we need to start at the tail, and build to the head
  fill_gradient_RGB(innerLEDs, streak_size, background, head); 
}

/*===============================================================================
 * Function:  makeOuterClockwiseStreak
 */
void DualRingLED::makeOuterClockwiseStreak(int streak_size, CRGB background, CRGB head)
{
  if (streak_size > DUAL_RING_NUM_OUTER) streak_size = DUAL_RING_NUM_OUTER;

  fillOuter(background);
  
  // since outer indexes go counter-clockwise, we need to start at the tail, and build to the head
  fill_gradient_RGB(outerLEDs, streak_size, background, head); 

}

/*===============================================================================
 * Function:  makeOuterCounterClockwiseStreak
 */
void DualRingLED::makeOuterCounterClockwiseStreak(int streak_size, CRGB background, CRGB head)
{
  if (streak_size > DUAL_RING_NUM_OUTER) streak_size = DUAL_RING_NUM_OUTER;

  fillOuter(background);
  
  // since outer indexes go counter-clockwise, we need to start at the head, and build to the tail
  fill_gradient_RGB(outerLEDs, streak_size, head, background); 
}

/*===================================================================================
 * Function: drawInnerClockwiseStreak
 * The draw_ functions are different from the make_ functions;
 *   - They don't fill in the background
 *   - You specify the starting location
 *   - They are more processor and memory intensive.
 *  What does this mean?  Use the make_ functions if you can.  If not, use draw_.
 *  
 */
void DualRingLED::drawInnerClockwiseStreak(int start_index, int streak_size, CRGB head, CRGB tail)
{
   if (start_index < 0) start_index = 0;
   if (start_index > DUAL_RING_LAST_INNER) start_index = DUAL_RING_LAST_INNER;
   if (streak_size > DUAL_RING_NUM_INNER) streak_size = DUAL_RING_NUM_INNER;

   _drawStreakHelper(innerLEDs, DUAL_RING_NUM_INNER, start_index, streak_size, head, tail);
}

/*===================================================================================
 * Function: drawInnerCounterClockwiseStreak
 * The draw_ functions are different from the make_ functions;
 *   - They don't fill in the background
 *   - You specify the starting location
 *   - They are more processor and memory intensive.
 *  What does this mean?  Use the make_ functions if you can.  If not, use draw_.
 *  
 */
void DualRingLED::drawInnerCounterClockwiseStreak(int start_index, int streak_size, CRGB head, CRGB tail)
{
   if (start_index < 0) start_index = 0;
   if (start_index > DUAL_RING_LAST_INNER) start_index = DUAL_RING_LAST_INNER;
   if (streak_size > DUAL_RING_NUM_INNER) streak_size = DUAL_RING_NUM_INNER;

   // since we're filling "backwards", we need to adjust our starting position
   start_index = start_index - streak_size + 1;
   if (start_index < 0) start_index = start_index + DUAL_RING_NUM_INNER;
    
   _drawStreakHelper(innerLEDs, DUAL_RING_NUM_INNER, start_index, streak_size, tail, head);
}

/*===================================================================================
 * Function: drawOuterCounterClockwiseStreak
 * The draw_ functions are different from the make_ functions;
 *   - They don't fill in the background
 *   - You specify the starting location
 *   - They are more processor and memory intensive.
 *  What does this mean?  Use the make_ functions if you can.  If not, use draw_.
 *  
 *  Note that "start index" is the index into the outer_leds array, not the absolute index.
 */
void DualRingLED::drawOuterCounterClockwiseStreak(int start_index, int streak_size, CRGB head, CRGB tail)
{
   if (start_index < 0) start_index = 0;
   if (start_index > DUAL_RING_LAST_OUTER) start_index = DUAL_RING_LAST_OUTER;
   if (streak_size > DUAL_RING_NUM_OUTER) streak_size = DUAL_RING_NUM_OUTER;

   _drawStreakHelper(outerLEDs, DUAL_RING_NUM_OUTER, start_index, streak_size, head, tail);
}

/*===================================================================================
 * Function: drawOuterClockwiseStreak
 * The draw_ functions are different from the make_ functions;
 *   - They don't fill in the background
 *   - You specify the starting location
 *   - They are more processor and memory intensive.
 *  What does this mean?  Use the make_ functions if you can.  If not, use draw_.
 *  
 *  Note that "start index" is the index into the outer_leds array, not the absolute index.
 *  
 */
void DualRingLED::drawOuterClockwiseStreak(int start_index, int streak_size, CRGB head, CRGB tail)
{
   if (start_index < 0) start_index = 0;
   if (start_index > DUAL_RING_LAST_OUTER) start_index = DUAL_RING_LAST_OUTER;
   if (streak_size > DUAL_RING_NUM_OUTER) streak_size = DUAL_RING_NUM_OUTER;

   // since we're filling "backwards", we need to adjust our starting position
   start_index = start_index - streak_size + 1;
   if (start_index < 0) start_index = start_index + DUAL_RING_NUM_OUTER;
   
   _drawStreakHelper(outerLEDs, DUAL_RING_NUM_OUTER, start_index, streak_size, tail, head);
}


/********===========  Begin Client code  =============*/


// Hardware definitions for our LED strip.
#define LED_PIN    6

DualRingLED myLights(LED_PIN);

//CRGBPalette16 my_palette =
const TProgmemPalette16 my_palette PROGMEM =
{
  CRGB::Blue,
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,

  CRGB::Blue,
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,

  CRGB::Blue,
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,
  
  CRGB::Blue,
  CRGB::Red,
  CRGB::Yellow,
  CRGB::Blue,

};

// We're using loop_delay to time our patterns...the bigger the delay, the slower the pattern.
#define DEFAULT_LOOP_TIME 60
#define MIN_LOOP_DELAY 10
#define MAX_LOOP_DELAY 150
int loop_delay=DEFAULT_LOOP_TIME;

/****=======================  PRE-DEFINED PATTERNS ============================******/
//  These have an init_ function to set up the desired pattern, 
//  and a move_function that will be called in the main loop.


/**********************************
 * PATTERN_TEST
 * 
 */
#define TEST_PATTERN_TIME_INCREMENT 100

int  test_index = 0;
bool test_inner=true;

void init_test_pattern( void )
{
  myLights.fillAll(CRGB::Black);
  
  test_index = 0;
  test_inner = true;
}

void move_test( void )
{
  static unsigned long last_update_time=0;
  unsigned long        current_time;
  CRGB                 color;

  if (test_index > DUAL_RING_NUM_OUTER) return;
  
  current_time = millis();
  
  // The test pattern is not affected by the loop delay setting.
  if (current_time > last_update_time + TEST_PATTERN_TIME_INCREMENT)
  {
    // make the "zeroth" led red.
    if (test_index == 0)
    {
      color = CRGB::Red;
    }
    
    // make the "first" led around the ring green.  This will show direction.
    else if (test_index == 1)    
    {
      color = CRGB::Green;
    }

    // all the rest should be blue.
    else
    {
      color = CRGB::Blue;
    }

    if (test_inner)
    {
      myLights.innerLEDs[test_index] = color;
      test_index++;
      if (test_index == DUAL_RING_NUM_INNER)
      {
        test_index = 0;
        test_inner = false;
      }
    }
    else
    {
      myLights.outerLEDs[test_index] = color;
      test_index++;
    }
    
    last_update_time = current_time;
    
  }  // if it's time for an update.
  
}  // end of move_test

/*********************************************
 * Pattern:  PATTERN_BLACK
 * All LEDs go off.  
 * Note that the move function does nothing..there's nothing to move.  :)
 */
void blackout( void )
{
  myLights.fillAll(CRGB::Black);
  myLights.setPattern(NULL);
}


/*********************************************
 * Pattern:  PATTERN_SYNC_CLOCKWISE
 * An inner and outer streak that move clockwise, synchronized.
 */
void init_sync_clockwise( void )
{
  int i;
  myLights.makeOuterClockwiseStreak(10, CRGB::Green, CRGB::Red);
  myLights.makeInnerClockwiseStreak(6, CRGB::Green, CRGB::Red); 

  // for those streak sizes, the inner needs to rotate 6 spots to be aligned with the outer.
  for (i=0;i<6;i++) myLights.rotateInnerClockwise(); 

  myLights.setPattern(move_sync_clockwise);
}

void move_sync_clockwise( void )
{
  static int phase=0;

  // In order to sync the inner and outer lanes, we need to preserve the 3:2 ratio. 
  // common denominator stuff...that means 6 phases.  
  if (phase % 2 == 0) myLights.rotateOuterClockwise();
  if (phase % 3 == 0) myLights.rotateInnerClockwise();

  phase = phase + 1;
  phase = phase % 6;

}

/*********************************************
 * Pattern:  PATTERN_SYNC_COUNTER
 * An inner and outer streak that move counter-clockwise, synchronized.
 */
void init_sync_counter( void )
{
  int i;
  myLights.makeOuterCounterClockwiseStreak(12, CRGB::Red, CRGB::Yellow);
  myLights.makeInnerCounterClockwiseStreak(8, CRGB::Red, CRGB::Yellow); 

  // rotate the inner streak to line up with the outer streak
  for (i=0;i<8;i++) myLights.rotateInnerCounterClockwise();

  myLights.setPattern(move_sync_counter);
  
}

void move_sync_counter( void )
{
  static int phase=0;

  // In order to sync the inner and outer lanes, we need to preserve the 3:2 ratio. 
  // common denominator stuff...that means 6 phases.  
  if (phase % 2 == 0) myLights.rotateOuterCounterClockwise();
  if (phase % 3 == 0) myLights.rotateInnerCounterClockwise();

  phase = phase + 1;
  phase = phase % 6;
  
}


/*********************************************
 * Pattern:  PATTERN_PULSE
 * This pattern has all LEDs with the same color, but pulses
 * that color through our pre-defined palette.
 */

void init_pulse( void )
{
  CRGB color;

  color = ColorFromPalette(my_palette, 0);
  myLights.fillAll(color);
  myLights.setPattern(move_pulse);
}

void move_pulse( void )
{
  static uint8_t index=0;
  CRGB color;
  
  color = ColorFromPalette(my_palette, index);

  myLights.fillAll(color);

  index++;
}

/*********************************************
 * Pattern:  PATTERN_OPPOSITES
 * This pattern creates a bump on the outside and a streak on the inside...both 
 * of which move in opposite directions.
 */
void init_opposites( void )
{
    myLights.makeOuterBump(6, CRGB::Blue, CRGB::Yellow);
    myLights.makeInnerClockwiseStreak(4, CRGB::Blue, CRGB::Yellow);
    myLights.setPattern(move_opposites);
}

void move_opposites( void )
{
    myLights.rotateInnerClockwise();
    myLights.rotateOuterCounterClockwise();  
}

/*********************************************
 * Pattern:  PATTERN_TICK
 * The outer ring has a streak that moves clockwise.
 * The inner ring has a bump.
 * Whenever the outer ring "touches" the inner bump,
 * it pushes (or "ticks") it over by one.
 */

// To make all this happen, we need to keep track of which "inner index" and
// "outer index" our rings are on. 
int outer_pos;
int inner_pos;

// one other fun thing...if we don't "wait" after pushing the inner ring to push it again,
// our rings end up in sync.  
#define TOUCH_DELAY 3

// indexed by inner position...gives outer index where the leds are
// considered "touching".  Note the 3/2 ratio, so we've got some rounding.
// Note that for this array, it's NOT the absolute LED position...it's the
// relative position around the circle.  Also note that this can be used in
// either the clockwise or counter clockwise direction.
int align_pos[] = 
{
  0,  // I 0
  2,  // I 1
  3,  // I 2
  5,  // I 3,
  6,  // I 4
  8,  // I 5
  9,  // I 6
  11, // I 7
  12, // I 8
  14, // I 9
  15, // I 10
  17, // I 11
  18, // I 12
  20, // I 13
  21, // I 14
  23  // I 15
};

bool touching(int inner, int outer)
{
  if (align_pos[inner] == outer) return true;
  else return false;    
}

void init_tick_pattern( void )
{
  myLights.makeOuterClockwiseStreak(8, CRGB::Blue, CRGB::Red);
  myLights.makeInnerBump(3, CRGB::Blue, CRGB::Red);

  myLights.setPattern(move_tick_pattern);
  
  inner_pos = 8;
  outer_pos = 7;
  
}

void move_tick_pattern( void )
{
    static int touch_delay=0;
    
    myLights.rotateOuterClockwise();
    outer_pos++;
    outer_pos = outer_pos % DUAL_RING_NUM_OUTER;
 
    if (touch_delay == 0)
    {
      
      if (touching(inner_pos, outer_pos))
      {
        myLights.rotateInnerClockwise();
        inner_pos++;
        inner_pos = inner_pos % DUAL_RING_NUM_INNER;
        touch_delay = 1;
      }
    }
    else
    {
      touch_delay++;
      if (touch_delay == TOUCH_DELAY) touch_delay = 0;
    }
}

/********************************************
 * PATTERN:  collide outer
 */
int clockwise_streak_index;
int counter_clockwise_streak_index;

void init_collide_outer( void )
{

    myLights.fillAll(CRGB::Black);
    myLights.setPattern(move_collide_outer);
    clockwise_streak_index=0;
    counter_clockwise_streak_index=0;
}

void move_collide_outer( void )
{
  myLights.fillOuter(CRGB::Black);
  
  myLights.drawOuterCounterClockwiseStreak(counter_clockwise_streak_index, 5, CRGB::Red, CRGB::Blue);
  myLights.drawOuterClockwiseStreak(clockwise_streak_index, 5, CRGB::Red, CRGB::Blue);

  clockwise_streak_index++;
  if (clockwise_streak_index >DUAL_RING_NUM_OUTER) clockwise_streak_index = 0;

  counter_clockwise_streak_index--;
  if (counter_clockwise_streak_index < 0) counter_clockwise_streak_index = DUAL_RING_NUM_OUTER - 1;
}

/*===================  MAIN FUNCTIONS ==============================*/

void print_help( void )
{
  Serial.println("Commands:");
  Serial.println("+ to speed up");
  Serial.println("- to slow down");
  Serial.println("1 selects tick pattern");
  Serial.println("2 selects clockwise sync");
  Serial.println("3 selects counter-clockwise sync");
  Serial.println("4 pulses colors");
  Serial.println("5 moves in opposite directions");
  Serial.println("6 prints the test pattern");
  Serial.println("7 selects the collide-outer pattern");
  Serial.println("0 blacks out display");
}

void user_input( void )
{
  char command;
  if (Serial.available())
  {
    command = Serial.read();

    switch (command)
    {
      case '+':
        // speed up
        if (loop_delay > MIN_LOOP_DELAY) 
        {
          loop_delay = loop_delay - 10;
          Serial.print("delay = ");
          Serial.println(loop_delay);
        }
      break;

      case '-':
        // slow down
        if (loop_delay < MAX_LOOP_DELAY)
        {
          loop_delay = loop_delay + 10;
          Serial.print("delay = ");
          Serial.println(loop_delay);
        }
      break;

      case '1':
        init_tick_pattern();
        Serial.println("Tick pattern chosen");
      break;

      case '2':
        init_sync_clockwise();
        Serial.println("Sync clockwise chosen");
      break;

      case '3':
        init_sync_counter();
        Serial.println("Sync counter-clockwise chosen");
      break;

      case '4':
         init_pulse();
         Serial.println("Pulse mode chosen");
      break;

      case '5':
         init_opposites();
         Serial.println("Opposites mode chosen");
      break;

      case '6':
          init_test_pattern();
          Serial.println("Test pattern selected");
      break;

      case '7':
          init_collide_outer();
          Serial.println("Collide outer pattern selected");
      break;
      
      
      case '0':
         blackout();
         Serial.println("BLACKOUT!!!");
      break;

      case '\n':
        //do nothing with returns
      break;

      default:
        print_help();
        
    }
  }
}




void setup()
{

    Serial.begin(9600);
    
    myLights.begin();

    print_help();

    myLights.fillAll(CRGB::Gray);
    
    myLights.setPattern(move_test);
}

void loop()
{
    user_input();

    myLights.run(loop_delay);
   
}

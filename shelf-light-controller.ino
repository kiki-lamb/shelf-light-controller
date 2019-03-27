// Meant for deployment on Tiny85 @ 1 Mhz, but compiles with additional serial debug info on 328P for development use.

//================================================================================

#if defined(__AVR_ATtiny85__) 
#define    KNOB A1
#define    OUT 1
#define    LED_BUILTIN 4
#else // assume 328P, enable debug output on serial.
#define    KNOB A0
#define    OUT 9
#define    DEBUG
#endif

//================================================================================

#define    LAMP_ON
#define    PULSE_ON
#define    DELAY_ON
#define    PWM_MAX          220
#define    ADC_SMOOTHING    4
#define    RATE             10

//================================================================================

#if defined(DEBUG)
#define    SERIAL_ON
#define    LOG_ADC_LAST
#define    LOG_ADC_AVG
//#define    LOG_FADE
#define    LOG_PWM
//#define    FALSE_ADC 1023
#endif

//================================================================================

static const uint8_t lut[] = {
  0,0,0,0,
  0,0,0,0,
  0,0,0,0, // 16
  
  1,1,1,1,
  1,2,2,2,
  2,3,3,3,
  4,4,5,5, // 32
  
  5,6,6,7,
  7,8,9,9,
  10,10,11,12,
  12,13,14,14, // 48
  
  15,16,17,17,
  18,19,20,21,
  21,22,23,24,
  25,26,27,28, // 64
  
  29,30,31,32,
  33,34,35,36,
  37,38,40,41,
  42,43,44,45, // 80
  
  47,48,49,50,
  52,53,54,55,
  57,58,59,61,
  62,63,65,66, // 96
  
  67,69,70,72,
  73,74,76,77,
  79,80,82,83,
  85,86,88,89, // 112
  
  90,92,93,95,
  97,98,100,101,
  103,104,106,107,
  109,110,112,113, // 128
  
  115,117,118,120,
  121,123,124,126,
  128,129,131,132,
  134,135,137,138, // 144
  
  140,142,143,145,
  146,148,149,151,
  152,154,155,157,
  158,160,162,163, // 160
  
  165,166,167,169,
  170,172,173,175,
  176,178,179,181,
  182,183,185,186,  // 176
  
  188,189,190,192,
  193,194,196,197,
  198,200,201,202,
  203,205,206,207, // 192
  
  208,210,211,212,
  213,214,215,217,
  218,219,220,221,
  222,223,224,225, // 208
  
  226,227,228,229,
  230,231,232,233,
  234,234,235,236,
  237,238,238,239,
  240,241,241,242, // 224
  
  243,243,244,245,
  245,246,246,247,
  248,248,249,249,
  250,250,250,251, // 240
  
  251,252,252,252,
  253,253,253,253,
  254,254,254,254,
  254,255,255,255, // 256
};

//================================================================================

uint16_t   adc_last      =  0;
uint16_t   adc_avg       =  0;
char       buf[5];

//================================================================================

void write_pwm(uint16_t const & fade_index, uint16_t const & knob) {
  uint8_t lut_val = lut[fade_index]; 
  uint8_t top     = knob >> 2;
  uint8_t bot     = top >= 0b10100000 ? ((top >>4) ^ 0b1000) - 1: 0;
  
  if (top > 245) { 
    analogWrite(
      OUT,
      255
    );
    return;
  }

  uint8_t scaled  = map(lut_val, 0, 255, bot, top    );
  uint8_t limited = map(scaled,  0, 255, 0,   PWM_MAX); 
      
#if !defined(__AVR_ATtiny85__) && defined(SERIAL_ON) && defined(LOG_PWM)
  //Serial.print("LUT: ");     snprintf(buf, 5, "%4d", lut_val);    Serial.print(buf); Serial.print("  ");
  Serial.print("Top: ");     snprintf(buf, 5, "%4d", top);        Serial.print(buf); Serial.print("  ");
  Serial.print("Bot: ");     snprintf(buf, 5, "%4d", bot);        Serial.print(buf); Serial.print("  ");
  Serial.print("Scaled: ");  snprintf(buf, 5, "%4d", scaled);     Serial.print(buf); Serial.print("  ");
  Serial.print("Limited: "); snprintf(buf, 5, "%4d", limited);     Serial.print(buf); Serial.print("  ");
#endif

  analogWrite(
    OUT, 
    limited
  );
}

//================================================================================
 
void setup() {
#if defined(LAMP_ON)
  pinMode(OUT, OUTPUT);
  pinMode(KNOB, INPUT);
#endif

#if defined(LED_ON)
  pinMode(LED_BUILTIN, OUTPUT);
#endif

#if !defined(__AVR_ATtiny85__) && defined(SERIAL_ON)
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
#endif
}

//================================================================================

void loop() {  
#if defined(DELAY_ON)
  delay(RATE);
#endif

#if !defined(__AVR_ATtiny85__) && defined(SERIAL_ON)
  Serial.println();
#endif 

#ifdef FALSE_ADC
  adc_last  =  1023;
  adc_avg   =  1023;
#else
  adc_last  =  analogRead(KNOB);
  adc_avg  *=  _BV(ADC_SMOOTHING)-1;
  adc_avg  +=  adc_last;
  adc_avg  >>= ADC_SMOOTHING;
#endif

// v Downshifting these just to make them look nice on serial plotter:
#if !defined(__AVR_ATtiny85__) && defined(SERIAL_ON) && defined(LOG_ADC_LAST)
  Serial.print("adc_last: "); snprintf(buf, 5, "%4d", adc_last >> 2); Serial.print(buf); Serial.print("  ");
#endif 
#if !defined(__AVR_ATtiny85__) && defined(SERIAL_ON) && defined(LOG_ADC_AVG)
  Serial.print("adc_avg: "); snprintf(buf, 5, "%4d", adc_avg >> 2); Serial.print(buf); Serial.print("  ");
#endif

#if defined(LAMP_ON)
#if defined(PULSE_ON)
  static int16_t    fade_val      =  0;
  static int8_t     fade_incr     =  1;
  fade_val += fade_incr;

  if (fade_val == 0 || fade_val == 255)
    fade_incr = -fade_incr; 

#if !defined(__AVR_ATtiny85__) && defined(SERIAL_ON) && defined(LOG_FADE)
  Serial.print("fade_val: "); snprintf(buf, 5, "%4d", fade_val); Serial.print(buf);  Serial.print("  ");
#endif
  write_pwm(fade_val, adc_avg);
#else
  write_pwm(255, adc_avg); 
#endif
#endif

#if defined(LED_ON)
  static bool ledState = false;
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);
#endif
}


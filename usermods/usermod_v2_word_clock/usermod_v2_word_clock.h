#pragma once

#include "wled.h"

/*
 * Usermods allow you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 *
 * This usermod can be used to drive a wordclock with a 11x10 pixel matrix with WLED. There are also 4 additional dots for the minutes.
 * The visualisation is described in 4 mask with LED numbers (single dots for minutes, minutes, hours and "clock/Uhr").
 * There are 2 parameters to change the behaviour:
 *
 * active: enable/disable usermod
 * diplayItIs: enable/disable display of "Es ist" on the clock.
 */

class WordClockUsermod : public Usermod
{
  private:
    unsigned long lastTime = 0;
    int lastTimeMinutes = -1;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    bool usermodActive = true;
    int ledOffset      = 0;

    bool configItIs      = true;
    bool configMidnight  = true;
    bool configNull      = true;
    bool configQuarterTo = true;
    bool configTwentyTo  = true;
    bool configTestMode  = false;

    // defines for mask sizes
    #define maskSizeLeds        125
    #define maskSizeMinuteDots  4

    struct ledRange {
      uint16_t first;
      uint16_t last;
    };

    enum words {
      WORD_ES = 0U,
      WORD_IST,
      WORD_NULL,
      WORD_UHR_1ST,
      WORD_ZWANZIG,
      WORD_FUENF_1ST,
      WORD_ZEHN_1ST,
      WORD_FUENFZEHN,
      WORD_DREI_1ST,
      WORD_VIERTEL,
      WORD_DREIVIERTEL,
      WORD_VOR,
      WORD_NACH,
      WORD_UM,
      WORD_HALB,
      WORD_ZWEI,
      WORD_EIN,
      WORD_EINS,
      WORD_MITTERNACHT,
      WORD_ACHT,
  	  WORD_DREI_2ND,
      WORD_SECHS,
      WORD_VIER,
      WORD_SIEBEN,
      WORD_ELF,
      WORD_FUENF_2ND,
      WORD_ZWOELF,
      WORD_ZEHN_2ND,
      WORD_NEUN,
      WORD_UHR_2ND,
      NUM_WORDS
    };

    // word ranges
    const ledRange wordRanges[NUM_WORDS]
    {
      {113, 114}, // WORD_ES
      {116, 118}, // WORD_IST
      {120, 123}, // WORD_NULL
      {109, 111}, // WORD_UHR_1ST
      {101, 107}, // WORD_ZWANZIG
      { 91,  94}, // WORD_FUENF_1ST
      { 95,  98}, // WORD_ZEHN_1ST
      { 91,  98}, // WORD_FUENFZEHN
      { 86,  89}, // WORD_DREI_1ST
      { 79,  85}, // WORD_VIERTEL
      { 79,  89}, // WORD_DREIVIERTEL
      { 69,  71}, // WORD_VOR
      { 72,  75}, // WORD_NACH
      { 77,  78}, // WORD_UM
      { 64,  67}, // WORD_HALB
      { 59,  62}, // WORD_ZWEI
      { 58,  60}, // WORD_EIN
      { 57,  60}, // WORD_EINS
      { 46,  56}, // WORD_MITTERNACHT
      { 53,  56}, // WORD_ACHT
      { 41,  44}, // WORD_DREI_2ND
      { 36,  40}, // WORD_SECHS
      { 24,  27}, // WORD_VIER
      { 28,  33}, // WORD_SIEBEN
      { 21,  23}, // WORD_ELF
      { 18,  21}, // WORD_FUENF_2ND
      { 13,  17}, // WORD_ZWOELF
      {  1,   4}, // WORD_ZEHN_2ND
      {  4,   7}, // WORD_NEUN
      {  9,  11}  // WORD_UHR_2ND
    };

    // minute dots
    const int minuteDots[maskSizeMinuteDots] = {0, 112, 124, 12};

    // overall mask to define which LEDs are on
    bool enabledWords[NUM_WORDS]         = { 0 };
    bool enabledDots[maskSizeMinuteDots] = { 0 };

    bool maskLedsOn[maskSizeLeds] =
    {
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,0,
      0,0,0,0
    };

    // set hours
    void setHours(int hours)
    {
      switch(hours)
      {
        case 0:
          enabledWords[WORD_ZWOELF] = true;
          break;
        case 1:
          enabledWords[WORD_EINS] = true;
          break;
        case 2:
          enabledWords[WORD_ZWEI] = true;
          break;
        case 3:
          enabledWords[WORD_DREI_2ND] = true;
          break;
        case 4:
          enabledWords[WORD_VIER] = true;
          break;
        case 5:
          enabledWords[WORD_FUENF_2ND] = true;
          break;
        case 6:
          enabledWords[WORD_SECHS] = true;
          break;
        case 7:
          enabledWords[WORD_SIEBEN] = true;
          break;
        case 8:
          enabledWords[WORD_ACHT] = true;
          break;
        case 9:
          enabledWords[WORD_NEUN] = true;
          break;
        case 10:
          enabledWords[WORD_ZEHN_2ND] = true;
          break;
        case 11:
          enabledWords[WORD_ELF] = true;
          break;
      }
    }

    // set minutes dot
    void setSingleMinuteDots(int minutes)
    {
      // modulo to get minute dots
      int minutesDotCount = minutes % 5;

      // activate all minute dots until number is reached
      for (auto dot = 0; dot < minutesDotCount; dot++)
      {
        // activate LED
        enabledDots[dot] = true;
      }
    }

    // update the display
    void updateDisplay(uint8_t hours, uint8_t minutes)
    {
      const uint8_t hour     = hours % 12;
      const uint8_t hour24   = hours % 24;
      const uint8_t hourNext = (hours + 1) % 12;

      minutes %= 60;

      // disable complete matrix at the bigging
      for (auto word = 0; word < NUM_WORDS; word++)
      {
        enabledWords[word] = false;
      }

      for (auto dot = 0; dot < maskSizeMinuteDots; dot++)
      {
        enabledDots[dot] = false;
      }

      // display it is/es ist if activated
      if (configItIs)
      {
        enabledWords[WORD_ES] = true;
        enabledWords[WORD_IST] = true;
      }

      // set single minute dots
      setSingleMinuteDots(minutes);

      if (configMidnight && (hour24 == 0) && (minutes < 5))
      {
        enabledWords[WORD_MITTERNACHT] = true;
      }
      else if (configNull && (hour == 0) && (minutes < 25))
      {
        enabledWords[WORD_NULL] = true;
        enabledWords[WORD_UHR_1ST] = true;

        if (minutes < 5)
        {
          // nothing to do
        }
        else if (minutes < 10)
        {
          enabledWords[WORD_FUENF_1ST] = true;
        }
        else if (minutes < 15)
        {
          enabledWords[WORD_ZEHN_1ST] = true;
        }
        else if (minutes < 20)
        {
          enabledWords[WORD_FUENFZEHN] = true;
        }
        else
        {
          enabledWords[WORD_ZWANZIG] = true;
        }
      }
      else if (minutes < 5)
      {
        if (hour == 1)
        {
          enabledWords[WORD_EIN] = true;
        }
        else
        {
          setHours(hour);
        }

        enabledWords[WORD_UHR_2ND] = true;
      }
      else if (minutes < 10)
      {
        enabledWords[WORD_FUENF_1ST] = true;
        enabledWords[WORD_NACH] = true;
        setHours(hour);
      }
      else if (minutes < 15)
      {
        enabledWords[WORD_ZEHN_1ST] = true;
        enabledWords[WORD_NACH] = true;
        setHours(hour);
      }
      else if (minutes < 20)
      {
        enabledWords[WORD_VIERTEL] = true;
        if (configQuarterTo)
        {
          enabledWords[WORD_NACH] = true;
          setHours(hour);
        }
        else
        {
          setHours(hourNext);
        }
      }
      else if (minutes < 25)
      {
        if (configTwentyTo)
        {
          enabledWords[WORD_ZWANZIG] = true;
          enabledWords[WORD_NACH] = true;
          setHours(hour);
        }
        else
        {
          enabledWords[WORD_ZEHN_1ST] = true;
          enabledWords[WORD_VOR] = true;
          enabledWords[WORD_HALB] = true;
          setHours(hourNext);
        }
      }
      else if (minutes < 30)
      {
        enabledWords[WORD_FUENF_1ST] = true;
        enabledWords[WORD_VOR] = true;
        enabledWords[WORD_HALB] = true;
        setHours(hourNext);
      }
      else if (minutes < 35)
      {
        enabledWords[WORD_HALB] = true;
        setHours(hourNext);
      }
      else if (minutes < 40)
      {
        enabledWords[WORD_FUENF_1ST] = true;
        enabledWords[WORD_NACH] = true;
        enabledWords[WORD_HALB] = true;
        setHours(hourNext);
      }
      else if (minutes < 45)
      {
        if (configTwentyTo)
        {
          enabledWords[WORD_ZWANZIG] = true;
          enabledWords[WORD_VOR] = true;
        }
        else
        {
          enabledWords[WORD_ZEHN_1ST] = true;
          enabledWords[WORD_NACH] = true;
          enabledWords[WORD_HALB] = true;
        }
        setHours(hourNext);
      }
      else if (minutes < 50)
      {
        if (configQuarterTo)
        {
          enabledWords[WORD_VIERTEL] = true;
          enabledWords[WORD_VOR] = true;
        }
        else
        {
          enabledWords[WORD_DREIVIERTEL] = true;
        }
        setHours(hourNext);
      }
      else if (minutes < 55)
      {
        enabledWords[WORD_ZEHN_1ST] = true;
        enabledWords[WORD_VOR] = true;
        setHours(hourNext);
      }
      else if (minutes < 60)
      {
        enabledWords[WORD_FUENF_1ST] = true;
        enabledWords[WORD_VOR] = true;
        setHours(hourNext);
      }
    }


  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup()
    {
    }

    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected()
    {
    }

    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     *
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     *
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() {
      // do it every 5 seconds
      if ((millis() - lastTime) > 5000)
      {
        // check the time
        int minutes = minute(localTime);

        // check if we already updated this minute
        if (lastTimeMinutes != minutes)
        {
          // update the display with new time
          updateDisplay(hour(localTime), minute(localTime));

          memset(maskLedsOn, 0, sizeof(maskLedsOn));

          for (auto word = 0; word < NUM_WORDS; word++)
          {
            if (enabledWords[word])
            {
              for (auto led = wordRanges[word].first; led <= wordRanges[word].last; led++)
              {
                maskLedsOn[led] = true;
              }
            }
          }

          for (auto dot = 0; dot < maskSizeMinuteDots; dot++)
          {
            if(enabledDots[dot])
            {
              maskLedsOn[minuteDots[dot]] = true;
            }
          }

          // remember last update time
          lastTimeMinutes = minutes;
        }

        // remember last update
        lastTime = millis();
      }
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    /*
    void addToJsonInfo(JsonObject& root)
    {
    }
    */

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
    }

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
    }

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     *
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     *
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     *
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     *
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject(F("WordClockUsermod"));
      top[F("active")] = usermodActive;
      top[F("ledOffset")] = ledOffset;
      top[F("itIs")] = configItIs;
      top[F("midnight")] = configMidnight;
      top[F("null")] = configNull;
      top[F("quarterTo")] = configQuarterTo;
      top[F("twentyTo")] = configTwentyTo;
      top[F("testMode")] = configTestMode;
    }

    void appendConfigData()
    {
      oappend(SET_F("addInfo('WordClockUsermod:ledOffset', 1, 'Number of LEDs before the letters');"));
      oappend(SET_F("addInfo('WordClockUsermod:itIs', 1, '\"It is\" activated');"));
      oappend(SET_F("addInfo('WordClockUsermod:midnight', 1, 'Midnight instead of 12');"));
      oappend(SET_F("addInfo('WordClockUsermod:null', 1, 'Null instead of 12');"));
      oappend(SET_F("addInfo('WordClockUsermod:quarterTo', 1, 'Quarter to instead of \"Dreiviertel\"');"));
      oappend(SET_F("addInfo('WordClockUsermod:twentyTo', 1, '\"Twenty to\" instead of \"10 nach halb\"');"));
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     *
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     *
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     *
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     *
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root[F("WordClockUsermod")];

      bool configComplete = !top.isNull();

      //configComplete &= getJsonValue(top[F("active")], usermodActive);
      configComplete &= getJsonValue(top[F("ledOffset")], ledOffset);
      configComplete &= getJsonValue(top[F("itIs")], configItIs);
      configComplete &= getJsonValue(top[F("midnight")], configMidnight);
      configComplete &= getJsonValue(top[F("null")], configNull);
      configComplete &= getJsonValue(top[F("quarterTo")], configQuarterTo);
      configComplete &= getJsonValue(top[F("twentyTo")], configTwentyTo);
      configComplete &= getJsonValue(top[F("testMode")], configTestMode);

      return configComplete;
    }

    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {
      // check if usermod is active
      if (usermodActive == true)
      {
        // loop over all leds
        for (int x = 0; x < maskSizeLeds; x++)
        {
          // check mask
          if (!maskLedsOn[x])
          {
            // set pixel off
            strip.setPixelColor(x + ledOffset, RGBW32(0,0,0,0));
          }
        }
      }
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_WORDCLOCK;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};
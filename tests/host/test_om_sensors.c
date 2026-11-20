/* om_sensors' host test: the host's sensors, in GFOpenMeters' Temp row (0.5). The sensors come from the status'
 * "host" block as amicradle's host_power.py writes them; a host with none, one with only some, a status cut short and
 * a "sensors" key elsewhere give what they should; the row reads "47\xb0C at 2.8 GHz" and its bar is against 85 C.
 *   test_om_sensors
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "om_sensors.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static void value_of(const char *status, const char *want)
{
    struct om_sensors s;
    char out[48];
    om_sensors_read(status, &s);
    om_sensors_value(out, sizeof out, &s);
    CHECK(!strcmp(out, want), "\"%s\", not \"%s\"", out, want);
}

int main(void)
{
    struct om_sensors s;
    char tiny[8];

    /* tootypi, the Pi 5 at 2.8 GHz, 20 November 2026, as Python's json.dumps writes it */
    static const char pi5[] =
        "{\"product\": \"AmiCradle\", \"volumes\": [{\"label\": \"System\"}], \"host\": {\"architecture\": \"64bit\", "
        "\"logicalCores\": 4, \"machine\": \"aarch64\", \"sensors\": {\"tempMilliC\": 46850, \"clockKHz\": 2800000, "
        "\"clockMaxKHz\": 2800000, \"throttled\": \"0x0\", \"underVoltage\": false, \"powerLevel\": \"Balanced\", "
        "\"powerLevelCanSet\": true}, \"tuning\": {}}}";
    CHECK(om_sensors_read(pi5, &s) == 1, "the Pi 5's sensors are read");
    CHECK(s.have_temp && s.temp_milli == 46850, "temp %lu", s.temp_milli);
    CHECK(s.have_clock && s.clock_khz == 2800000, "clock %lu", s.clock_khz);
    CHECK(s.have_throttled && s.throttled == 0, "throttled %lx", s.throttled);
    CHECK(s.have_uv && !s.under_voltage, "under-voltage");
    CHECK(!s.have_fan, "no fan on the Pi 5");
    value_of(pi5, "47\xb0" "C at 2.8 GHz");
    CHECK(om_sensors_fill(&s) == 551, "fill %lu", om_sensors_fill(&s));         /* 46.85 of 85 C */

    /* hot and held back now, with low volts and a fan: every word, and a full bar */
    value_of("{\"host\": {\"sensors\": {\"tempMilliC\": 86200, \"clockKHz\": 1500000, \"throttled\": \"0x50007\", "
             "\"fanRPM\": 4210}}}", "86\xb0" "C at 1.5 GHz, throttled, low volts, 4210 rpm");
    om_sensors_read("{\"host\": {\"sensors\": {\"tempMilliC\": 86200}}}", &s);
    CHECK(om_sensors_fill(&s) == 1000, "over 85 C fills the bar: %lu", om_sensors_fill(&s));
    om_sensors_read("{\"host\": {\"sensors\": {\"tempMilliC\": 76500}}}", &s);
    CHECK(om_sensors_fill(&s) == 900, "the warning starts at 76.5 C: %lu", om_sensors_fill(&s));

    /* only what happened before this boot (bits 16 and up): not shown as now */
    value_of("{\"host\": {\"sensors\": {\"tempMilliC\": 52000, \"throttled\": \"0x50000\"}}}", "52\xb0" "C");

    /* under-voltage from the Pi's hwmon alone */
    value_of("{\"host\": {\"sensors\": {\"tempMilliC\": 60000, \"underVoltage\": true}}}", "60\xb0" "C, low volts");

    /* a PC: a temperature and a clock under 1 GHz, idle */
    value_of("{\"host\": {\"sensors\": {\"tempMilliC\": 38999, \"clockKHz\": 800000, \"fanRPM\": 900}}}",
             "39\xb0" "C at 800 MHz, 900 rpm");

    /* only a clock: words, no bar */
    om_sensors_read("{\"host\": {\"sensors\": {\"clockKHz\": 2400000}}}", &s);
    CHECK(s.have_clock && !s.have_temp, "clock only");
    CHECK(om_sensors_fill(&s) == (unsigned long)-1, "no temperature, no fill");
    value_of("{\"host\": {\"sensors\": {\"clockKHz\": 2400000}}}", "2.4 GHz");

    /* a host that measured nothing (sensors {}), one with no sensors key, no status at all */
    CHECK(om_sensors_read("{\"host\": {\"sensors\": {}}}", &s) == 0, "an empty sensors object is no row");
    CHECK(om_sensors_read("{\"host\": {\"machine\": \"x86_64\"}}", &s) == 0, "no sensors key is no row");
    CHECK(om_sensors_read(NULL, &s) == 0, "no status is no row");

    /* a "sensors" that is not an object, before the real one */
    CHECK(om_sensors_read("{\"note\": \"sensors\", \"x\": {\"sensors\": \"none\"}, \"host\": {\"sensors\": "
                          "{\"tempMilliC\": 41000}}}", &s) == 1 && s.temp_milli == 41000, "the object is found, not the word");

    /* a status cut short inside the sensors: what came whole is kept, the cut value is not */
    CHECK(om_sensors_read("{\"host\": {\"sensors\": {\"tempMilliC\": 47000, \"clockKHz\": 28", &s) == 1,
          "a cut status keeps what came whole");
    CHECK(s.have_temp && !s.have_clock, "the cut clock is dropped");

    /* nonsense values are not shown */
    CHECK(om_sensors_read("{\"host\": {\"sensors\": {\"tempMilliC\": -3, \"clockKHz\": null, \"throttled\": \"zero\", "
                          "\"underVoltage\": \"maybe\"}}}", &s) == 0, "nonsense is no row");

    /* a small buffer: cut, still ended */
    om_sensors_read(pi5, &s);
    om_sensors_value(tiny, sizeof tiny, &s);
    CHECK(strlen(tiny) < sizeof tiny, "a small buffer stays ended: \"%s\"", tiny);

    /* the Temp bar's colours: blue, through green, to red */
    {
        int r, g, b, i;
        om_sensors_heat(0, &r, &g, &b);
        CHECK(b > r && b > g, "blue at the cool end: %d,%d,%d", r, g, b);
        om_sensors_heat(500, &r, &g, &b);
        CHECK(g > r && g > b, "green in the middle: %d,%d,%d", r, g, b);
        om_sensors_heat(750, &r, &g, &b);
        CHECK(r > b && g > b, "yellow at three quarters: %d,%d,%d", r, g, b);
        om_sensors_heat(1000, &r, &g, &b);
        CHECK(r > g && r > b, "red at the hot end: %d,%d,%d", r, g, b);
        om_sensors_heat(5000, &r, &g, &b);
        CHECK(r == 220 && g == 45 && b == 35, "past the end is red: %d,%d,%d", r, g, b);
        for (i = 0; i <= 1000; i++) {           /* every place a colour, and no blue from green on */
            om_sensors_heat((unsigned long)i, &r, &g, &b);
            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) break;
            if (i >= 500 && b > 70) break;
        }
        CHECK(i == 1001, "the colours run smoothly: stopped at %d", i);
    }

    printf("test_om_sensors: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}

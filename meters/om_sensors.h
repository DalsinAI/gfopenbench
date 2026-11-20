/* om_sensors.h: the host's sensors, for GFOpenMeters 0.5's Temp row (Dale, 20 November 2026: "add some way to watch
 * temperatures to openmeter", then, raising a Pi 5 to 3 GHz: "thats why i asked for openmeter to show us temps").
 *
 * Where it comes from: accontrol.device's ACCMD_BOARD, query ACC_QUERY_STATUS (7), the host's /api/system/status as
 * JSON, as om_disk.h reads it. Its "host" block's "sensors" object is what the host measured (amicradle
 * scripts/hostsetup/host_power.py sensors()): "tempMilliC" (the CPU or SoC), "clockKHz" (the fastest core now),
 * "throttled" (the Pi's vcgencmd get_throttled, "0x0"), "underVoltage" (true or false) and "fanRPM". A key is only
 * there when the host has it, so a host without a sensor shows nothing for it, and a host that says none shows no row.
 *
 * The row: "47\xb0C at 2.8 GHz", with ", throttled" or ", low volts" while the Pi says so now, and ", 1200 rpm" with
 * a fan. Its bar is the temperature against OM_SENSORS_HOT_MILLIC, 85 C, where the Pi 5's firmware throttles hard, so
 * the warning colour (90 per cent and over) comes at 76.5 C, before its soft limit at 80.
 *
 * Plain C, tested on the host (tests/host/test_om_sensors.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OM_SENSORS_H
#define OM_SENSORS_H

#define OM_SENSORS_HOT_MILLIC 85000UL

/* vcgencmd get_throttled's bits for now (the ones for since the boot, 16 and up, are not shown) */
#define OM_THR_UNDERVOLT  0x1UL
#define OM_THR_CAPPED     0x2UL
#define OM_THR_THROTTLED  0x4UL
#define OM_THR_SOFTTEMP   0x8UL

struct om_sensors {
    int have_temp, have_clock, have_throttled, have_uv, have_fan;
    unsigned long temp_milli;      /* thousandths of a degree C */
    unsigned long clock_khz;
    unsigned long throttled;       /* get_throttled's bits */
    int under_voltage;
    unsigned long fan_rpm;
};

/* From the host's status (JSON, NUL-ended; it may be cut short): its sensors, into s. 1 when at least one is there. */
int om_sensors_read(const char *status, struct om_sensors *s);

/* The row's words: "47\xb0C at 2.8 GHz", "47\xb0C", "2.8 GHz", "800 MHz", then ", throttled", ", low volts",
 * ", 1200 rpm" as they apply. */
void om_sensors_value(char *out, int size, const struct om_sensors *s);

/* The bar, 0 to 1000: the temperature against OM_SENSORS_HOT_MILLIC; (unsigned long)-1 (no fill) without one. */
unsigned long om_sensors_fill(const struct om_sensors *s);

#endif

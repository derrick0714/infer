/* 
 * File:   FrequentRebootsParams.h
 * Author: Mike
 *
 * Created on December 12, 2009, 10:53 AM
 */

#ifndef _DARKACCESSPARAMS_H
#define	_DARKACCESSPARAMS_H

#include "../OptionManipulation.h"

// Command line option defines
#define OPT_INPUT_NETFLOW "input-netflow"
#define OPT_SHORT_INPUT_NETFLOW "i"
#define OPT_INPUT_PCAP "input-pcap"
#define OPT_SHORT_INPUT_PCAP "p"
#define OPT_HELP "help"
#define OPT_SHORT_HELP "h"
#define OPT_CONFIG_FILE "config-file"
#define OPT_SHORT_CONFIG_FILE "c"
#define OPT_OUTFILE "output"
#define OPT_SHORT_OUTFILE "o"
#define OPT_STDOUT "stdout"
#define OPT_SHORT_STDOUT "s"
#define OPT_SENSOR "sensor"
#define OPT_SHORT_SENSOR "n"

#define OPT_STATE_DIR "state-dir"
#define OPT_SHORT_STATE_DIR "d"
#define OPT_APP_DEF "app-def"
#define OPT_SHORT_APP_DEF "a"
#define OPT_EVENTS_AT_REBOOT "events-at-reboot"
#define OPT_SHORT_EVENTS_AT_REBOOT "e"
#define OPT_BOOT_TIME_WINDOW "boot-time-window"
#define OPT_SHORT_BOOT_TIME_WINDOW "b"
#define OPT_WATCH_TIME "watch-time"
#define OPT_SHORT_WATCH_TIME "w"
#define OPT_REBOOT_COUNT "reboot-count"
#define OPT_SHORT_REBOOT_COUNT "r"

// Configuration file defines.
#define CFG_APP_DEF OPT_APP_DEF
#define CFG_STATE_DIR OPT_STATE_DIR
#define CFG_EVENTS_AT_REBOOT OPT_EVENTS_AT_REBOOT
#define CFG_BOOT_TIME_WINDOW OPT_BOOT_TIME_WINDOW
#define CFG_WATCH_TIME OPT_WATCH_TIME
#define CFG_REBOOT_COUNT OPT_REBOOT_COUNT

#endif	/* _DARKACCESSPARAMS_H */


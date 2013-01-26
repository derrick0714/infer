/* 
 * File:   DNSFreeFlowsParams.h
 * Author: Mike
 *
 * Created on December 12, 2009, 10:53 AM
 */

#ifndef _DNSFREEFLOWSPARAMS_H
#define	_DNSFREEFLOWSPARAMS_H

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
#define OPT_DNS_TIMEOUT "dns-timeout"
#define OPT_SHORT_DNS_TIMEOUT "d"
#define OPT_SENSOR "sensor"
#define OPT_SHORT_SENSOR "n"

#define OPT_STATE_FILE "state-file"
#define OPT_SHORT_STATE_FILE "f"

// Configuration file defines.
#define CFG_DNS_TIMEOUT OPT_DNS_TIMEOUT
#define CFG_STATE_FILE OPT_STATE_FILE

#endif


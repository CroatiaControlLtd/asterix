/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * AUTHORS:  Damir Salantic, Croatia Control Ltd.
 * 			 Jurica Baricevic, Croatia Control Ltd.
 *			 Zeljko Harjac, Croatia Control Ltd.
 *			 Darko Markus, Croatia Control Ltd.
 *
 */
#ifndef ASTERIX_H
#define ASTERIX_H

/* Standard multi-platform types */
typedef int INT32;
typedef unsigned int UINT32;
typedef unsigned int *PUINT32;
typedef short int INT16;
typedef unsigned short int UINT16;
typedef unsigned short int *PUINT16;
typedef signed char INT8;
typedef unsigned char UINT8;
typedef unsigned char *PUINT8;

/* Optional packages */
#define PACKAGE_FPL__AIL_FORMAT     0    /* Format for AIL RDPS - not used, not tested */

/* DEBUG zones */
#define ZONE_TCPDEVICE          1
#define ZONE_UDPDEVICE          1
#define ZONE_DISKDEVICE         1
#define ZONE_SERIALDEVICE       1
#define _DEBUG_TIMESTAMP        1

#define TRACE_FILENAME        1
#define LOG_TIME_IN_UTC        0

/* TODO
#define TRACE_FILENAME          0
#define LOG_TIME_IN_UTC         1
*/

// logging levels
// ---------------------------------------------------------------------
//          standard        displayed in    logged in       logged in
//          output file     supervisory     supervisory     information
//          for logging     window          log             log
// ---------------------------------------------------------------------
// ERROR    stderr          yes             yes             yes
// WARNING  stderr          yes             yes             yes
// NOTIFY   stderr          yes             yes             yes
// INFO     stdout                                          yes
// DEBUG    stdout                                          yes

#define FD_LOGDEBUG   stderr
#define FD_LOGINFO    stderr
#define FD_LOGNOTIFY  stderr
#define FD_LOGWARNING stderr
#define FD_LOGERROR   stderr

/* DEBUG macro definitions */

#if _DEBUG_TIMESTAMP
/* Print timestamp before each message */
#include <time.h>

#if _DEBUG
#define LOGDEBUG(cond, ...)  {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGDEBUG, "\nDEBUG:   %s (%s) ", &sTime[4], __FILE__); fprintf(FD_LOGDEBUG, __VA_ARGS__); fflush(FD_LOGDEBUG);} }
#else
#define LOGDEBUG(cond, ...)
#endif

#if TRACE_FILENAME
#if LOG_TIME_IN_UTC
#define LOGINFO(cond, ...)     {if (cond) {const time_t currTime=(const time_t)time(NULL); tm * ptm = gmtime ( &currTime ); fprintf(FD_LOGINFO,    "\nINFO:    %02d.%02d.%02d.%02d:%02d:%02d (%s) : ", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, __FILE__); fprintf(FD_LOGINFO, __VA_ARGS__);   fflush(FD_LOGINFO);} }
#define LOGNOTIFY(cond, ...)   {if (cond) {const time_t currTime=(const time_t)time(NULL); tm * ptm = gmtime ( &currTime ); fprintf(FD_LOGNOTIFY,  "\nNOTIFY:  %02d.%02d.%02d.%02d:%02d:%02d (%s) : ", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, __FILE__); fprintf(FD_LOGNOTIFY, __VA_ARGS__); fflush(FD_LOGNOTIFY);} }
#define LOGWARNING(cond, ...)  {if (cond) {const time_t currTime=(const time_t)time(NULL); tm * ptm = gmtime ( &currTime ); fprintf(FD_LOGWARNING, "\nWARNING: %02d.%02d.%02d.%02d:%02d:%02d (%s) : ", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, __FILE__); fprintf(FD_LOGWARNING, __VA_ARGS__);fflush(FD_LOGWARNING);} }
#define LOGERROR(cond, ...)    {if (cond) {const time_t currTime=(const time_t)time(NULL); tm * ptm = gmtime ( &currTime ); fprintf(FD_LOGERROR,   "\nERROR:   %02d.%02d.%02d.%02d:%02d:%02d (%s) : ", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, __FILE__); fprintf(FD_LOGERROR, __VA_ARGS__);  fflush(FD_LOGERROR);} }
#else
#define LOGINFO(cond, ...)     {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGINFO,    "INFO:    %s (%s) ", &sTime[4], __FILE__); fprintf(FD_LOGINFO, __VA_ARGS__);   fflush(FD_LOGINFO);} }
#define LOGNOTIFY(cond, ...)   {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGNOTIFY,  "NOTIFY:  %s (%s) ", &sTime[4], __FILE__); fprintf(FD_LOGNOTIFY, __VA_ARGS__); fflush(FD_LOGNOTIFY);} }
#define LOGWARNING(cond, ...)  {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGWARNING, "WARNING: %s (%s) ", &sTime[4], __FILE__); fprintf(FD_LOGWARNING, __VA_ARGS__);fflush(FD_LOGWARNING);} }
#define LOGERROR(cond, ...)    {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGERROR,   "ERROR:   %s (%s) ", &sTime[4], __FILE__); fprintf(FD_LOGERROR, __VA_ARGS__);  fflush(FD_LOGERROR);} }
#endif
#else
#if LOG_TIME_IN_UTC
#define LOGINFO(cond, ...)     {if (cond) {const time_t currTime=(const time_t)time(NULL); tm * ptm = gmtime ( &currTime ); fprintf(FD_LOGINFO,    "INFO:    %02d.%02d.%02d.%02d:%02d:%02d : ", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec); fprintf(FD_LOGINFO, __VA_ARGS__);   fflush(FD_LOGINFO);} }
#define LOGNOTIFY(cond, ...)   {if (cond) {const time_t currTime=(const time_t)time(NULL); tm * ptm = gmtime ( &currTime ); fprintf(FD_LOGNOTIFY,  "NOTIFY:  %02d.%02d.%02d.%02d:%02d:%02d : ", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec); fprintf(FD_LOGNOTIFY, __VA_ARGS__); fflush(FD_LOGNOTIFY);} }
#define LOGWARNING(cond, ...)  {if (cond) {const time_t currTime=(const time_t)time(NULL); tm * ptm = gmtime ( &currTime ); fprintf(FD_LOGWARNING, "WARNING: %02d.%02d.%02d.%02d:%02d:%02d : ", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec); fprintf(FD_LOGWARNING, __VA_ARGS__);fflush(FD_LOGWARNING);} }
#define LOGERROR(cond, ...)    {if (cond) {const time_t currTime=(const time_t)time(NULL); tm * ptm = gmtime ( &currTime ); fprintf(FD_LOGERROR,   "ERROR:   %02d.%02d.%02d.%02d:%02d:%02d :", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec); fprintf(FD_LOGERROR, __VA_ARGS__);  fflush(FD_LOGERROR);} }
#else
#define LOGINFO(cond, ...)     {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGINFO,    "INFO:    %s ", &sTime[4]); fprintf(FD_LOGINFO, __VA_ARGS__);   fflush(FD_LOGINFO);} }
#define LOGNOTIFY(cond, ...)   {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGNOTIFY,  "NOTIFY:  %s ", &sTime[4]); fprintf(FD_LOGNOTIFY, __VA_ARGS__); fflush(FD_LOGNOTIFY);} }
#define LOGWARNING(cond, ...)  {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGWARNING, "WARNING: %s ", &sTime[4]); fprintf(FD_LOGWARNING, __VA_ARGS__);fflush(FD_LOGWARNING);} }
#define LOGERROR(cond, ...)    {if (cond) {const time_t currTime=(const time_t)time(NULL); char *sTime=ctime(&currTime); sTime[24]='\0'; fprintf(FD_LOGERROR,   "ERROR:   %s ", &sTime[4]); fprintf(FD_LOGERROR, __VA_ARGS__);  fflush(FD_LOGERROR);} }
#endif
#endif


#else /* ! _DEBUG_TIMESTAMP */

/* No timestamping */
#if _DEBUG
#define LOGDEBUG(cond, ...)  {if (cond) {fprintf(FD_LOGDEBUG, "DEBUG: (%s) ", __FILE__); fprintf(FD_LOGDEBUG, __VA_ARGS__); fflush(FD_LOGDEBUG);} }
#else
#define LOGDEBUG(cond, ...)
#endif

#define LOGINFO(cond, ...)     {if (cond) {fprintf(FD_LOGINFO,    "INFO:    (%s) ", __FILE__); fprintf(FD_LOGINFO, __VA_ARGS__);   fflush(FD_LOGINFO);} }
#define LOGNOTIFY(cond, ...)   {if (cond) {fprintf(FD_LOGNOTIFY,  "NOTIFY:  (%s) ", __FILE__); fprintf(FD_LOGNOTIFY, __VA_ARGS__); fflush(FD_LOGNOTIFY);} }
#define LOGWARNING(cond, ...)  {if (cond) {fprintf(FD_LOGWARNING, "WARNING: (%s) ", __FILE__); fprintf(FD_LOGWARNING, __VA_ARGS__);fflush(FD_LOGWARNING);} }
#define LOGERROR(cond, ...)    {if (cond) {fprintf(FD_LOGERROR,   "ERROR:   (%s) ", __FILE__); fprintf(FD_LOGERROR, __VA_ARGS__);  fflush(FD_LOGERROR);} }

#endif /* _DEBUG_TIMESTAMP */

enum GlobalFormat {
    GF_UNKNOWN = 0,
    GF_FPL = 1,
//    GF_GRIB = 2,
    GF_GTS = 2,
    GF_ASTERIX = 3
};

// return bit-values for GetStatus() methods
#define STS_FAIL_INPUT  1  // excessive input device errors
#define STS_FAIL_OUTPUT 2  // excessive output device errors
#define STS_FAIL_DATA   4  // excessive data (formatter) errors
#define STS_NO_DATA     8  // no more data available

extern const char *gAsterixDefinitionsFile;
extern bool gVerbose;
extern bool gTrace;
extern bool gForceRouting;
extern int gHeartbeat;

/* Private ASSERT macro */
#ifdef ASSERT
#undef ASSERT
#endif
#if _DEBUG
#include <assert.h>
#define ASSERT(C)       assert(C)
#else
#define ASSERT(C)
#endif

#endif

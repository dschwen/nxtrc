/*
** Part of NXTRC ver 2.3
** (Header file with Functions def)
** A program to interact with
** LEGO Mindstorm NXT
** Copyright 2007 by GiP
** This program is distributed under the terms of the
** GNU General Public License (GPL)
** See file COPYING
*/

// NXTRC_func.h definition of functions
// Use this as a Guide


#ifndef NXTRC_func
#define NXTRC_func

//////////////////////////////////////////
// Scan BT for devices
void BTscan();

//////////////////////////////////////////
// Connect to BT dev with BTaddr= <dest>
// if failed retry <retries> times
// if failed again quits
// if successfull return socket in <s>
// if (verb) Beeps and prints "CONNECTED"
int nxt_connect(char *dest,int verb);

//////////////////////////////////////////
// Close connection with BT dev with socket <s>
// and exits the program
// if (verb) prints what is doing
void nxt_quit(int s,int verb) __attribute__ ((__noreturn__)); 

//////////////////////////////////////////
// Beep! Brick connected to <s> beeps
void beep(int s);

//////////////////////////////////////////
// Set the Name of the Brick connected 
// to <s>  to <name>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int SetName(int s,char *name, int verb);

//////////////////////////////////////////
// List Files on the Brick connected to <s>
// if <name> is empty list all
// otherwise list matches to pattern <name>
// returns 0 or the error code from the Brick 
int ListFiles(int s,char * name);

//////////////////////////////////////////
// Write File <name> on the Brick connected to <s>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int WriteFile(int s,char *name, int verb);

//////////////////////////////////////////
// Read File <name> from the Brick connected to <s>
// if (verb) prints command and reply
void ReadFile(int s,char *name, int verb);

//////////////////////////////////////////
// Delete File <name> on the Brick connected to <s>
// if (verb) prints command and reply
void DelFile(int s,char *name, int verb);

//////////////////////////////////////////
// Find if File <name> exists on the Brick 
// connected to <s>
// if (verb) prints command and reply
// returns TRUE (1) if file exists FALSE(0) otherwise 
int FileExist(int s, char *name, int verb);

//////////////////////////////////////////
// Run Program <name> on the Brick connected to <s>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int RunProg(int s,char *name, int verb);

//////////////////////////////////////////
// Stops a program that is running on the Brick
// connected to <s>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int StopRun(int s,int verb);

//////////////////////////////////////////
// Play Sound File  <name> on the Brick connected to <s> 
// if loop=0 play only once
// if loop!=0 play indefinitely 
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int PlayFile(int s,char *name, int loop, int verb);

//////////////////////////////////////////
// Stops a Sound Playback on the Brick connected to <s>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int StopPlay(int s,int verb);

////////////////////////////////////////
// Get Battery Level in mV from the Brick connected to <s>
// Not really reliable, it doesn't make much sense
void GetBatt(int s);

//////////////////////////////////////////
// Get Device Info from the Brick connected to <s>
void GetInfo(int s);

//////////////////////////////////////////
// Get Free Space Info from the Brick connected to <s>
int GetFree(int s);

//////////////////////////////////////////
//Get Firmware version from the Brick connected to <s>
void GetFirmVer(int s);

//////////////////////////////////////////
// Sends a command to the Brick connected to <s>
// the command is in string <send> passed 
// with lenght in bytes <sizes>
// <send> must be the complete Bluetooth command
// with the first two bytes giving the lenght
// of the NXT command (i.e. sizes-2) 
// if available, puts reply in <recv>
// returns errorcode (0=no error)
// if (verb) prints command, reply and error
// returns 0 or the error code from the Brick 
int nxt_sendrecv(int s,char *send, int sizes, int verb);

//////////////////////////////////////////
// Prints out a command or reply data
// in array dat of lenght n prepended
// with string s (for info) 
void VerbOut(char *p, int n, char *dat);

//////////////////////////////////////////
//Prints the NXT Error given the code
//returned by a command 
void nxt_error(char error_code);

#endif

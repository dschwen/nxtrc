/* 
**  -- nxtrc -- 
** A program to interact with 
** LEGO MIndstorm NXT 
** Copyright 2010 by GiP
** This program is distributed under the terms of the
** GNU General Public License (GPL)
** See file COPYING
*/

#define VERSION "2.3"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "nxtrc_func.h"

void print_usage(void);

int main(int argc, char **argv)
{
int s,i,verb=0,loop=0;
char c,rsp;
extern char *optarg;
extern int optind, opterr, optopt;
char name[20]={0x0};
char action='X';  // action to perform
char addr[18]={0x0}; //BTaddress

if (argc == 1) // No options -> print usage
{ print_usage();
  exit(0);
}
optind=0;

// according to the manual using "::" after a letter should allow an
// optional parameter after an options but it doesn't work 
// (for me at least) so I had to use "l" and "L" for the listing 
while(1) 
{ c=getopt(argc,argv,":a:bd:hikl::M:m:n:p:r:svw:W:z");  // Parse options
  if (c == -1) break; 
  switch (c)
  {  case 's':    // Scan for devices
        BTscan();
        exit(0);
     case 'b':  // Get Battery Level (mV !?)
        action='b';
        break;
     case 'a':    //BTaddress to Connect to 
        strncpy (addr,optarg,18);
        break;
     case 'd':  // Delete File on brick
        strncpy(name,optarg,20);
        action='d';
        break;
     case 'h':    // Print usage
        print_usage();
        exit(0);
     case 'i':  // Get Device Info
        action='i';
        break;
     case 'l':  //list files  (with optional glob parameter) 
        name[0]='\0';
        action='l'; 
        if (optarg == NULL)
          strncpy(name,"*.*",4); // Use *.* (all files)
        else  // take the optional glob parameter
        { action='L';
          strncpy(name,optarg,20);
        }
        break;
     case 'n':  // Set brick name
        strncpy(name,optarg,20);
        action='n';
        break;
     case 'M':  // Play a Sound File Indefinitely
        loop=1;
     case 'm':  // Play a Sound File Once
        strncpy(name,optarg,20);
        action='m';
        break;
     case 'p':  // Run Program on brick
        strncpy(name,optarg,20);
        action='p';
        break;
     case 'k':  // Stop Program on brick
        action='k';
        break;
     case 'v':  // Be verbose? 
        verb=1;
        break;
     case 'w':  // Write Program on brick (if already present asks permission)
        strncpy(name,optarg,20);
        action='w';
        break;
     case 'W':  // Write Program on brick (no question asked)
        strncpy(name,optarg,20);
        action='W';
        break;
     case 'r':  // Read Program from brick
        strncpy(name,optarg,20);
        action='r';
        break;
     case 'z':  // Stop Sound Playback
        action='z';
        break;
     case '?':
          printf("=! Error: Option -%c not recognized\n",(char)optopt);
          exit (1);
          break;
     case ':':
          printf("=! Error: Option -%c requires a parameter\n",(char)optopt);
          exit(1);
        break;
     default:
        break;   
  }
}

if (action == 'X') 
{ printf("=! Error: No Action to perform\n"); 
  exit(1);
}

if (addr[0] == '\0') // BTaddr not set with -d option
{  if (getenv("BTADDR") != NULL) // Look for Env Var BTADDR
    strncpy (addr,getenv("BTADDR"),18); 
  else 
  { printf("=! Error: No BTaddress, use -a option or BTADDR Env Var\n"); 
    exit(1);
  } 
}
// Connect to <addr>
s=nxt_connect(addr,verb);

switch(action)
{  case 'b':
     GetBatt(s);
     break;
  case 'd':
     printf("=: ** Deleting File <%s> on NXT\n",name);
     DelFile(s,name,verb);
     break;
  case 'i':
     printf("=: ** Getting Brick Info\n\n");
     GetInfo(s);
     break;
  case 'k':
     printf("=: ** Stopping Program on NXT\n");
     StopRun(s,verb);
     break;
  case 'L':        
     printf("\n=: Listing  <%s>",name);
  case 'l':        
     ListFiles(s,name);
     break;
  case 'm':
     printf("=: ** Playing Sound File <%s>\n",name);
     PlayFile(s,name,loop,verb);
     break;
  case 'n':
     printf("=: ** Setting NXT Name to <%s>\n",name);
     SetName(s,name,verb);
     break;
  case 'p':
     printf("=: ** Running Program <%s>\n",name);
     RunProg(s,name,verb);
     break;
  case 'r':
     printf("=: ** Reading File from NXT\n");
     ReadFile(s,name,verb);
     break;
  case 'w':
     printf("=: ** Writing File <%s> on NXT\n",name);
     i=WriteFile(s,name,verb);
     if ((unsigned char)i == 0x8F)
     { printf("=! File already exists on NXT\n");
       printf("=? Delete it from NXT? (y,N) ");
       i=scanf("%c",&rsp);
       if (tolower(rsp) == 'y')
       { DelFile(s,name,verb);
         sleep(2); //BT Delay
         WriteFile(s,name,verb);
       }
     }
     break;
  case 'W':
     printf("=: ** Writing File <%s> on NXT\n",name);
     i=WriteFile(s,name,verb);
     if ((unsigned char)i == 0x8F)
     {  DelFile(s,name,verb);
        sleep(2); //BT Delay
        WriteFile(s,name,verb);
     }
     break;
  case 'z':
     printf("=: ** Stopping Sound Playback\n");
     StopPlay(s,verb);
     break;
}

printf("\n");
//beep();
// Close Connection
nxt_quit(s,verb);
exit(0);
}

void print_usage(void)
{
printf(
"nxtrc vers. "VERSION" - A program to interact with LEGO Mindstorm NXT\n\
using a Bluetooth connection.\n\n\
   Usage: nxtrc [-a ADDR] [-v] command\n\
        -a  ADDR     Connect with NXT at ADDR\n\
                       required unless -s is used\n\
                       or ADDR is in BTADDR Env Var\n\
        -v           Be verbose\n\
      and one command\n\
        -i           Get info on NXT brick\n\
        -l[PATT]     List files on NXT brick \n\
                       matching PATT if present (e.g. -l*.rxe)\n\
                       N.B. No spaces between -l and PATT!\n\
        -b           Get Battery Level\n\
        -n  NAME     Set NXT name to NAME\n\
        -w  FILE     Write FILE on NXT\n\
        -W  FILE     Same as -w overwriting FILE if exists \n\
        -r  FILE     Read FILE frome NXT and save it\n\
        -d  FILE     Delete FILE frome NXT\n\
        -p  FILE     Run Program FILE on NXT\n\
        -k           Stop a program running on NXT\n\
        -m  FILE     Play Sound File once\n\
        -M  FILE     Play Sound File indefinitely\n\
        -z           Stop Sound Playback\n\n\
    or\n\
      nxtrc -s         Scan for BT devices\n\n\
    if more than one command is given only\n\
    the last one is performed.\n\n\
");

}      


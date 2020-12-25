/* 
** A program to run a program
** On LEGO Mindstorm NXT
** and read the data sent. 
** Copyright 2010 by GiP
** This program is distributed under the terms of the
** GNU General Public License (GPL)
** See file COPYING
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "nxtrc_func.h"

char addr[18]="00:16:53:00:50:55"; // NXT BTaddr

int main(int argc, char **argv)
{
int s,i,n,l,verb=0;
char dat[50];

s=nxt_connect(addr,verb);

//printf("=: ** Running Program\n");
RunProg("SendDat.rxe",verb);

while(1)
{ i=read(s,dat,sizeof(dat));  // read data stream from NXT
  do                         
  { if (dat[6]=='E')    // end of data
       goto fine;  //sorry!
     l=dat[0]+2;
     //printf("i=%d l=%d %s\n",i,l,&dat[6]);
     printf("%s\n",&dat[6]);

    if (i>l)              // Data stream has more than one message
    { for(n=0;n<i-l;n++)  //strip the first, move the other to front
        dat[n]=dat[l+n];
      i-=l;               // set lenght to what remains
    } // could be done just changing the index to read from......
    else
     i=0;  // no more messages to read
  }while(i>0);  // if it's not the last message keep reading data
}               // from array dat

fine:
nxt_quit(verb);
}


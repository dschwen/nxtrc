/*
** Part of NXTRC ver 2.3
** (Functions and utilities)
** A program to interact with
** LEGO Mindstorm NXT
** Copyright 2010 by GiP
** This program is distributed under the terms of the
** GNU General Public License (GPLv2)
** See file COPYING
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "nxtrc_func.h"

int retries=3;

// Functions //

//////////////////////////////////////////
// Connect to BT dev with BTaddr= <dest>
// if failed retry <retries> times
// if failed again quits
// if successfull return socket in <s>
// if (verb) Beeps and prints "CONNECTED"
int nxt_connect(char *dest,int verb) 
{
int s;
int status;
struct sockaddr_rc addr={0};

s= socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
addr.rc_family = AF_BLUETOOTH;
addr.rc_channel = (uint8_t) 1;
str2ba( dest, &addr.rc_bdaddr);
status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

while(status < 0)
{  perror("=! Connection Failed: \n");
   if(retries == -1) 
     nxt_quit(s,0); 
    sleep(3);
    printf("=! Retrying... (%d)\n", retries--);
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    status = connect(s, (struct sockaddr *)&addr,sizeof(addr));
}

if(verb) 
{ printf("=: Connected\n");
  beep(s);
}
return(s);
}

//////////////////////////////////////////
//Close connection with BT dev with socket <s>
//and exits the program
// if (verb) prints what is doing
void nxt_quit(int s, int verb) 
{  
if (verb) printf("=: Exiting...\n");
sleep(1);
if(retries != -1) 
{ if(verb) printf("=: Disconnected\n");
  close(s);
}
exit(0);
}


//////////////////////////////////////////
// Scan BT for devices
void BTscan()
{
inquiry_info *ii = NULL;
int max_rsp, num_rsp;
int dev_id, sock, len, flags;
int i;
char addr[19]={0};
char name[25]={0};

dev_id=hci_get_route(NULL);
sock=hci_open_dev(dev_id );
if (dev_id<0 || sock<0) 
{  perror("=! Opening socket:");
   exit(1);
}

len=8;
max_rsp=255;
flags=IREQ_CACHE_FLUSH;
ii=(inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    
num_rsp=hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
if( num_rsp<0 ) perror("=! hci_inquiry: ");

for (i=0; i<num_rsp; i++) 
{  ba2str(&(ii+i)->bdaddr, addr);
   memset(name, 0, sizeof(name));
   if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), 
       name, 0)<0)
     strcpy(name, "<not available>");
    printf("%s  %s\n", addr, name);
}
free(ii);
close(sock);
}

//////////////////////////////////////////
// Beep! Brick connected to <s> beeps
void beep(int s) 
{
char Beep[8];
Beep[0]=0x06;
Beep[1]=0x00;
Beep[2]=0x80;
Beep[3]=0x03;
Beep[4]=0xf4;
Beep[5]=0x01;
Beep[6]=0xf4;
Beep[7]=0x01;
nxt_sendrecv(s,Beep,sizeof(Beep),0);
}

//////////////////////////////////////////
// Set the Name of the Brick connected 
// to <s>  to <name>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int SetName(int s, char *name, int verb)
{
char comm[25]={0x11,0x0,0x01,0x98};
int i;

for(i=0;i<strlen(name);i++)
  comm[i+4]=name[i];
comm[19]=0;

return(nxt_sendrecv(s,comm,sizeof(comm),verb));
}

//////////////////////////////////////////
// List Files on the Brick connected to <s>
// if <name> is empty list all
// otherwise list matches to pattern <name>
// returns 0 or the error code from the Brick 
int ListFiles(int s, char * name)
{
int *size,r;
char comm[25]={0x06,0x00,0x01,0x86,'*','.','*',0x0};
char rcv[50]; 
int total=0;

size=(int *)&rcv[26]; //size of file are the 4 bytes starting at 26

if (name[0] != '\0')
{  strncpy(&comm[4],name,20);
   comm[0]=strlen(name)+3;
}
// Print Headers
printf ("\n=: Files: \n\
                Name     Size\n");

r = write(s,comm, comm[0]+2); // send command to read first file
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,0);
}
usleep(200000); //BT Delay
r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,0);
}
if (rcv[4] == 0)
{ strncpy(name,&rcv[6],20);
  printf("%20s %8d\n",name,*size);
  total+=*size;
}
else 
{ nxt_error(rcv[4]);
  return(rcv[4]);
}

comm[0]=0x3;
comm[2]=0x81;
comm[3]=0x84;
comm[4]=rcv[5];
r = write(s,comm, 5); // send command to close handle
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,0);
}

while (1)  //Read other files
{ comm[0]=0x3;
  comm[2]=0x01;
  comm[3]=0x87;      // Find Next
  comm[4]=rcv[5];   // Handle of previuos File

  r = write(s,comm, 5); // send command to read next file
  if (r < 0)
  {  perror("=! Error sending command:");
     nxt_quit(s,0);
  }
  usleep(200000); //BT Delay

  r = read(s,rcv,sizeof(rcv)); //read reply
  if (r < 0)
  {  perror("=! Error receiving reply");
     nxt_quit(s,0);
  }
  if (rcv[4] == 0)
  { strncpy(name,&rcv[6],20);
    printf("%20s %8d\n",name,*size);
    total+=*size;
  }
  else  // no more files 
   break;

  comm[0]=0x3;
  comm[2]=0x81;
  comm[3]=0x84;
  comm[4]=rcv[5];
  r = write(s,comm, 5); // send command to close last handle
  if (r < 0)
  {  perror("=! Error sending command:");
     nxt_quit(s,1);
  }
}

printf ("\n=: Total %d Bytes\n",total);
printf ("=: Free  %d Bytes\n\n",GetFree(s));

return 0;
}

//////////////////////////////////////////
// Write File <name> on the Brick connected to <s>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int WriteFile(int s, char *name, int verb)
{
char comm[39];
int i,len,f,rsize,r;
int space;
union { int i;
        unsigned char c[4];
      } size;
struct stat buf;
unsigned char rcv[50];
char hndl=0;
unsigned char *data;

for(i=0;i<39;i++)  //fill comm with NULL
  comm[i]=0x0;

// Getting File size
if (stat(name, &buf) == -1)
{ printf("=! Cannot Read File <%s>\n",name);
  perror("=! Error");
  nxt_quit(s,verb); 
}
size.i=(int)buf.st_size;
len=strlen(name);

// Check if there is enough space on the Brick
space=GetFree(s);
if (size.i > space)
{  perror("=! Not enough free space on device:");
   nxt_quit(s,verb);
}

// Getting handle to write to brick
comm[2]=0x01;
comm[3]=0x81;
strncpy(&comm[4],name,20); // fills up to 4+len+null=5+len  
                           // up to 23 are already NULL
comm[24]=size.c[0]; // size LWLB
comm[25]=size.c[1];
comm[26]=size.c[2];
comm[27]=size.c[3]; // size HWHB
comm[28]=0x0; //null terminator
comm[0]=27; //lenght of the command command(2)+len+null+size(4)+null


// send command to open handle for write
r = write(s,comm, 29); 
if (r < 0)
{  perror("=! Error sending command");
   nxt_quit(s,1);
}
sleep(2); //BT Delay

for (i=0;i<50;i++)
  rcv[i]=0;

r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,1);
}

if (rcv[4] == 0)
  hndl=rcv[5];
else
{ if ((unsigned char)rcv[4] == 0x8F)
    return 0x8f;
  printf("=! Cannot get handle for writing file\n");
  nxt_error(rcv[4]);
  nxt_quit(s,1);
}
printf("=: File: <%s>  Size: %d\n",name,size.i);

//Allocating memory for the data+command
if((data=(unsigned char *)malloc(size.i+5)) == NULL)
{ printf("=! Cannot allocate memory\n");
  perror("=! Error ");
  nxt_quit(s,1);
}
//preparing command
data[0]=(size.i+3)%256; // LB of command lenght
data[1]=(size.i+3)/256; // HB of command lenght
data[2]=0x01;
data[3]=0x83;
data[4]=hndl;

//Reading the file into buffer starting from 5
f=open(name,O_RDONLY);
r=read(f,&data[5],size.i);
if(r!=size.i)
{ printf("=! Cannot read file\n");
  perror("=! Error ");
  nxt_quit(s,1);
}
//write data  from buffer to brick
r = write(s,data, size.i+5); 
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,1);
}

for (i=0;i<50;i++)
  rcv[i]=0;

usleep(500000); //BT Delay
r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,1);
}

if (rcv[4] == 0)
{ rsize=256*rcv[7]+rcv[6];
  printf("=: File written - Size=%d bytes\n",rsize);
}
else
{ printf("=! Error writing file\n");
  nxt_error(rcv[4]);
  nxt_quit(s,1);
}

//Close the handle
comm[0]=0x3;
comm[2]=0x01;
comm[3]=0x84;
comm[4]=hndl;
r = write(s,comm, 5); // send command to close handle
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,1);
}
usleep(200000); //BT Delay
r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,1);
}
if (rcv[4] != 0)
{ printf("=! Error closing handle %d\n",rcv[5]);
  nxt_error(rcv[4]);
  nxt_quit(s,1);
}
free(data);
return(0);
}

//////////////////////////////////////////
// Read File <name> from the Brick connected to <s>
// if (verb) prints command and reply
void ReadFile(int s, char *name, int verb)
{
char comm[29];
int i,f,rsize,r,rtot=0;
union { int i;
        unsigned char c[4];
      } size;
unsigned char rcv[30];
char hndl;
unsigned char *data;
struct stat buf;

for(i=0;i<29;i++)  //fill comm with NULL
  comm[i]=0x0;

//Checking if file exists
if (stat(name, &buf) != -1)
{ printf("=? File <%s>  Size: %d \n",name,(int)buf.st_size);
  printf("=? Already exists: Overwrite? (y|N) ");
  i=scanf("%c",&hndl);
  if (tolower(hndl) != 'y')
    nxt_quit(s,verb);   
}

// Getting handle and File size
// to read from  brick
comm[2]=0x01;
comm[3]=0x80;
strncpy(&comm[4],name,20); // fills up to 4+len+null=5+len  
                           // up to 23 are already NULL
comm[0]=22;   //lenght of the command 

// send command to open handle for read
r = write(s,comm, 24); 
if (r < 0)
{  perror("=! Error sending command");
   nxt_quit(s,1);
}
usleep(500000); //BT Delay
r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,1);
}

size.i=0;
if (rcv[4] == 0)
{ hndl=rcv[5];
  size.c[0]=rcv[6];
  size.c[1]=rcv[7];
  size.c[2]=rcv[8];
  size.c[3]=rcv[9];
  printf("=: FileName: <%s>  Size: %d\n",name,size.i);
}
else
{ printf("=! Cannot get handle for reading file\n");
  nxt_error(rcv[4]);
  nxt_quit(s,1);
}

//Allocating memory for the file buffer
rsize=size.i+8;
if((data=(unsigned char *)malloc(rsize)) == NULL)
{ printf("=! Cannot allocate memory\n");
  perror("=! Error ");
  nxt_quit(s,1);
}
//preparing command 
comm[0]=6; 
comm[1]=0x0;
comm[2]=0x01;
comm[3]=0x82;
comm[4]=hndl;
comm[5]=(size.i)%256; // LB of file lenght
comm[6]=(size.i)/256; // HB of file lenght
comm[7]=0x0;

//send command to read data from brick
r = write(s,comm, 8); 
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,1);
}

// sometimes not all the data is already in the buffer
// so after a bigger delay we read more than once if
// not all data is read
sleep(1); //BT Delay
do
{ r = read(s,data,rsize); //read reply until all bytes read
  sleep(1); //BT Delay
  if (r < 0)
  {  perror("=: Error receiving reply");
     nxt_quit(s,1);
  }
  rtot+=r;
} while(rtot < rsize);

if (data[4] == 0)
{ rsize=data[6]+data[7]*256;
  printf("=: File Read - Size=%d bytes\n",rsize);
  if((f=open(name,O_CREAT|O_WRONLY,0644)) == -1)
  { printf("=! Cannot open file for saving");
    perror("=! Error");
    nxt_quit(s,1);
  }
  i=write(f,&data[8],rsize);
  close(f);
  printf("=: File Saved - Size=%d bytes\n",i);
}
else
{ printf("=! Error reading file\n");
  nxt_error(rcv[4]);
  nxt_quit(s,1);
}

//Close the handle
comm[0]=0x3;
comm[2]=0x81;
comm[3]=0x84;
comm[4]=hndl;
r = write(s,comm, 5); // send command to close handle
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,1);
}
}

//////////////////////////////////////////
// Delete File <name> on the Brick connected to <s>
// if (verb) prints command and reply
void DelFile(int s, char *name, int verb)
{
char comm[25] = {0, };
int r;
char rcv[25];

// Preparing command to delete file on brick
comm[2]=0x01;
comm[3]=0x85;
strncpy(&comm[4],name,20); // fills up to 4+len+null=5+len  
                           // up to 24 are already NULL
comm[0]=23; //lenght of the command 

// send command to delete file
r = write(s,comm, 25); 
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,0);
}
usleep(200000); //BT Delay
//read reply
r = read(s,rcv,sizeof(rcv)); 
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,0);
}
if (rcv[4] == 0)
{ strncpy(name,&rcv[5],20);
printf("=: File <%s> deleted from NXT\n",name);
}
else
{ printf("=! Error deleting file\n");
  nxt_error(rcv[4]);
  nxt_quit(s,1);
}
}

//////////////////////////////////////////
// Find if File <name> exists on the Brick 
// connected to <s>
// if (verb) prints command and reply
// returns TRUE (1) if file exists FALSE(0) otherwise 
int FileExist(int s, char *name, int verb)
{
int r;
char comm[25]={0x06,0x00,0x01,0x86,'*','.','*',0x0};
unsigned char rcv[50];

if (name[0] != '\0')
{  strncpy(&comm[4],name,20);
   comm[0]=strlen(name)+3;
}
r = write(s,comm, comm[0]+2); // send command to read  file
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,0);
}
usleep(200000); //BT Delay
r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,0);
}
if (rcv[4] == 0)
{ // close handle
  comm[0]=0x3;
  comm[2]=0x81;
  comm[3]=0x84;
  comm[4]=rcv[5];
  r = write(s,comm, 5); // send command to close handle
  if (r < 0)
  {  perror("=! Error sending command:");
     nxt_quit(s,0);
  }
  return (1); // return TRUE
}
else
 return (0); // return FALSE
}

//////////////////////////////////////////
// Run Program <name> on the Brick connected to <s>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int RunProg(int s, char *name, int verb)
{
char comm[25]={0x0,0x0,0x0,0x0};

if (FileExist(s,name,verb))
{ strncpy(&comm[4],name,20);
  comm[0]=strlen(name)+3;
  return(nxt_sendrecv(s,comm,strlen(name)+5,verb));
}
else
{  printf("=! Program Not Found\n");
   nxt_quit(s,verb);
}
 return 255; // shouldn't reach this
}

//////////////////////////////////////////
// Stops a program that is running on the Brick
// connected to <s>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int StopRun(int s, int verb)
{
char comm[5]={0x03,0x0,0x00,0x01,0x0};

return(nxt_sendrecv(s,comm,sizeof(comm),verb));
}

//////////////////////////////////////////
// Play Sound File  <name> on the Brick connected to <s> 
// if loop=0 play only once
// if loop!=0 play indefinitely 
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int PlayFile(int s, char *name, int loop, int verb)
{
char comm[25]={0x0,0x0,0x00,0x2,0x00};


if (FileExist(s,name,verb))
{ strncpy(&comm[5],name,20);
  comm[0]=strlen(name)+4;
  if (loop) 
    comm[4]=1;
  return(nxt_sendrecv(s,comm,strlen(name)+6,verb));
}
else
{  printf("=! File Not Found\n");
   nxt_quit(s,verb);
}
 return 255; // shouldn't reach this

}

//////////////////////////////////////////
// Stops a Sound Playback on the Brick connected to <s>
// if (verb) prints command and reply
// returns 0 or the error code from the Brick 
int StopPlay(int s, int verb)
{
char comm[5]={0x03,0x0,0x00,0x0C,0x0};

return(nxt_sendrecv(s,comm,sizeof(comm),verb));
}

////////////////////////////////////////
// Get Battery Level in mV from the Brick connected to <s>
// Not really reliable, it doesn't make much sense
void GetBatt(int s)
{
char comm[5]={0x03,0x0,0x00,0x0B,0x0};
int r;
unsigned char rcv[9]; 
int lev;
//int levpc;

r = write(s,comm, 5); // send command to Read Battery Level
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,0);
}
usleep(200000); //BT Delay
r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,0);
}
//// hard to figure what it means percentage-wise 
///  it seems that when fully charged it's ~8350
///  and when almost out is ~6600
///  but it fluctuates wildly even between
///  two closely spaced reading. 
///  - Do with it whatever you want but maybe it's
///  - better to leave the mV reading only
///  - whatever that means.
//levpc=(int)(((lev-6600.)/(8350.-6600))*100.00);
//printf("=: Battery Level: %dmV (~%d%%)\n",lev,levpc);
lev=rcv[5]+rcv[6]*256;
printf("=: Battery Level: %dmV\n",lev);

}

//////////////////////////////////////////
// Get Device Info from the Brick connected to <s>
void GetInfo(int s)
{
char comm[5]={0x03,0x0,0x01,0x9B,0x0};
int r;
unsigned char rcv[40]; 
union { int i;
        unsigned char c[4];
      } size;

r = write(s,comm, 5); // send command to Read Info
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,0);
}
usleep(200000); //BT Delay

r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,0);
}
printf("=: Device Name  : %s\n",&rcv[5]);
printf("=: BT Address   : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
        rcv[20],rcv[21],rcv[22],rcv[23],rcv[24],rcv[25]);
//printf("rcv[26]?=%X\n",rcv[26]);   /// this part 
//size.c[0]=rcv[27];                 /// not yet
//size.c[1]=rcv[28];                 /// implemented
//size.c[2]=rcv[29];                 /// in the NXT
//size.c[3]=rcv[30];                 /// firmware !!
//printf("BT signal    : %d\n",size.i);  /// always == 0
size.c[0]=rcv[31];
size.c[1]=rcv[32];
size.c[2]=rcv[33];
size.c[3]=rcv[34];
printf("=: Free space   : %d\n",size.i);
GetBatt(s);
GetFirmVer(s);
}

//////////////////////////////////////////
// Get Free Space info from the Brick connected to <s>
int GetFree(int s)
{
char comm[5]={0x03,0x0,0x01,0x9B,0x0};
int r;
unsigned char rcv[40]; 
union { int i;
        unsigned char c[4];
      } size;

r = write(s,comm, 5); // send command to Read Info
if (r < 0)
{  perror("=! Error sending command:");
   nxt_quit(s,0);
}
usleep(200000); //BT Delay

r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,0);
}
size.c[0]=rcv[31];
size.c[1]=rcv[32];
size.c[2]=rcv[33];
size.c[3]=rcv[34];
return size.i;
}

//////////////////////////////////////////
//Get Firmware version from the Brick connected to <s>
void GetFirmVer(int s)
{
char comm[5]={0x03,0x00,0x01,0x88,0x0};
unsigned char rcv[10];
int r;

r = write(s,comm, 5);
if (r < 0)
{  perror("=! Error sending command");
   nxt_quit(s,1);
}
usleep(200000); //BT Delay

r = read(s,rcv,sizeof(rcv)); //read reply
if (r < 0)
{  perror("=! Error receiving reply");
   nxt_quit(s,1);
}
if (rcv[4] == 0)
{ printf("=: Firmware Ver : %d.%02d\n",rcv[8],rcv[7]);
  printf("=: Protocol Ver : %d.%d\n",rcv[6],rcv[5]);
}
else
{ printf("=! Error getting Firmware version\n");
  nxt_error(rcv[4]);
  nxt_quit(s,1);
}
}

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
int nxt_sendrecv(int s, char *send, int sizef, int verb)
{
int r;
char rcv[50];

r = write(s,send, sizef);
if (r < 0)
{  perror("=! Error sending command");
   nxt_quit(s,1);
}
usleep(200000); //BT Delay
if (verb)
  VerbOut("=> OUT: ",send[0]+2,send);
if(!(send[2]&0x80))
{ r = read(s,rcv,sizeof(rcv));
  if (r < 0)
  {  perror("=! Error receiving command");
     nxt_quit(s,1);
  }
  if (verb)
    VerbOut("=> IN: ",rcv[0]+2,rcv);
  if(rcv[2] == 0x02 && rcv[4] != 0x00) 
  {  printf("=! CMD NOT ACCEPTED (errors)\n");
     nxt_error(rcv[4]);
  }
}
return(rcv[4]);
}


//////////////////////////////////////////
// Prints out a command or reply data
// in array dat of lenght n prepended
void VerbOut(char *p, int n, char *dat) 
{
int i;

if(dat[2] == 0x02)
  printf("=> REPLY RECEIVED\n");
else
  printf("\n=> CMD SENT\n");

printf("%s ",p);
for(i = 0; i < n; i++)
  printf("%.2x ",(unsigned char)dat[i]);
printf("\n");
//Error handling
if(dat[2] == 0x02 && dat[4] == 0x00)
  printf("=> CMD ACCEPTED (no errors)\n");
}

//////////////////////////////////////////
//Prints the NXT Error given the code
//returned by a command 
void nxt_error(char error_code)
{
switch((unsigned char)error_code){
  case 0x20: printf("=! NXT-ERR: Pending communication transaction in progress\n");
   break;
  case 0x40: printf("=! NXT-ERR: Specified mailbox queue is empty\n");
   break;
  case 0xBD: printf("=! NXT-ERR: Request failed (i.e. file not found)\n");
   break;
  case 0xBE: printf("=! NXT-ERR: Unknown command opcode\n");
   break;
  case 0xBF: printf("=! NXT-ERR: Insane packet\n");
   break;
  case 0xC0: printf("=! NXT-ERR: Data contains out-of-range values\n");
   break;
  case 0xDD: printf("=! NXT-ERR: Communication bus error\n");
   break;
  case 0xDE: printf("=! NXT-ERR: No free memory in communication buffer\n");
   break;
  case 0xDF: printf("=! NXT-ERR: Specified chan/conn is not valid\n");
   break;
  case 0xE0: printf("=! NXT-ERR: Specified chan/conn not configured or busy\n");
   break;
  case 0xEC: printf("=! NXT-ERR: No active program\n");
   break;
  case 0xED: printf("=! NXT-ERR: Illegal size specified\n");
   break;
  case 0xEE: printf("=! NXT-ERR: Illegal mailbox queue ID specified\n");
   break;
  case 0xEF: printf("=! NXT-ERR: Accessing invalid field of a structure\n");
   break;
  case 0xF0: printf("=! NXT-ERR: Bad input or output specified\n");
   break;
  case 0xFB: printf("=! NXT-ERR: Insufficient memory avvailable\n");
   break;
  case 0xFF: printf("=! NXT-ERR: Bad arguments\n");
   break;
//////
// =! NXT-ERRs from file operations
//////
  case 0x81: printf("=! NXT-ERR: No more handles\n");
   break;
  case 0x82: printf("=! NXT-ERR: No space\n");
   break;
  case 0x83: printf("=! NXT-ERR: No more files\n");
   break;
  case 0x84: printf("=! NXT-ERR: End of file expected\n");
   break;
  case 0x85: printf("=! NXT-ERR: End of file\n");
   break;
  case 0x86: printf("=! NXT-ERR: Not a linear file\n");
   break;
  case 0x87: printf("=! NXT-ERR: File not found\n");
   break;
  case 0x88: printf("=! NXT-ERR: Handle already closed\n");
   break;
  case 0x89: printf("=! NXT-ERR: No linear space\n");
   break;
  case 0x8A: printf("=! NXT-ERR: Undefined error\n");
   break;
  case 0x8B: printf("=! NXT-ERR: File is busy\n");
   break;
  case 0x8C: printf("=! NXT-ERR: No write buffers\n");
   break;
  case 0x8D: printf("=! NXT-ERR: Append not possible\n");
   break;
  case 0x8E: printf("=! NXT-ERR: File is full\n");
   break;
  case 0x8F: printf("=! NXT-ERR: File exists\n");
   break;
  case 0x90: printf("=! NXT-ERR: Module not found\n");
   break;
  case 0x91: printf("=! NXT-ERR: Out of boundary\n");
   break;
  case 0x92: printf("=! NXT-ERR: Illegal file name\n");
   break;
  case 0x93: printf("=! NXT-ERR: illegal handle\n");
   break;
}
}


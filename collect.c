#include <stdio.h>
#include <stdlib.h>  // for windows specific keywords in ftd2xx.h
#include "ftd2xx.h"   // Header file for ftd2xx.lib 
#include <unistd.h>
#include <time.h> 

#define  MAX_DEVICES 5

FT_STATUS SbusWrite(FT_HANDLE ft_handle, unsigned short addr, unsigned short data)
        {
            unsigned char wbuf[5];
			DWORD BytesWritten=0;
			wbuf[0] = 0x01;
            wbuf[1] = (unsigned char)((addr >> 8) & 0xff);
            wbuf[2] = (unsigned char)(addr & 0xff);
            wbuf[3] = (unsigned char)((data >> 8) & 0xff);
            wbuf[4] = (unsigned char)(data & 0xff);
            return  FT_Write(ft_handle, wbuf, sizeof(wbuf), &BytesWritten);
			//rif.write(wbuf, 5);
        }

unsigned short SbusRead(FT_HANDLE ft_handle, unsigned short addr)
        {
			unsigned char wbuf[3];
			FT_STATUS ft_status;
			DWORD BytesWritten=0;
			DWORD BytesReceived=0;
			
            		wbuf[0] = 0x00;
		        wbuf[1] = (unsigned char)((addr >> 8) & 0xff);
            		wbuf[2] = (unsigned char)(addr & 0xff);
			
			ft_status = FT_Write(ft_handle, wbuf, sizeof(wbuf), &BytesWritten);
			ft_status = FT_Read(ft_handle,wbuf,2,&BytesReceived);
			
			if (ft_status == FT_OK) 
			{
				//printf("\n\t Read OK");
			}
			else 
			{
				printf("\n\t Read Failed");
			}
            //rif.write(wbuf, 3);
           // rif.read(rbuf, 2);
            return (unsigned short)(wbuf[0] << 8 | wbuf[1]);
        }

unsigned short  waitDone(FT_HANDLE ft_handle)
	{
			// wait until sasebo board has finished processing 
			while(SbusRead(ft_handle, 0x0002)!=0x0000)
				usleep(1000000);
	}

void main()
{
	
	unsigned char * pcBufRead = NULL;
	char * 	pcBufLD[MAX_DEVICES + 1];
	char 	cBufLD[MAX_DEVICES][64];
	int	iNumDevs = 0;
	int	i, j;
	int	iDevicesOpen = 0;	
    	int queueChecks = 0;
    	long int timeout = 5; // seconds
    	struct timeval  startTime;
	unsigned long loop =0;
	unsigned long LIMIT=10000;
	
	unsigned char LatencyTimer = 2;
	unsigned short pt[8];
	unsigned short SaSEBOct[8];
	unsigned short SoftWarect[8];

	
	for(i = 0; i < MAX_DEVICES; i++) {
		pcBufLD[i] = cBufLD[i];
	}
	pcBufLD[MAX_DEVICES] = NULL;
	
   FT_HANDLE ft_handle;  // handle to the USB ic
   FT_STATUS ft_status;  // for status report(error,io status etc)
        
   ft_status = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
   

	for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ); i++) {
		printf("Device %d Serial Number - %s\n", i, cBufLD[i]);
	}   
			
   //ft_status = FT_Open(1,&ft_handle); //open a connection 
   ft_status= FT_OpenEx ("FT456WGBB", FT_OPEN_BY_SERIAL_NUMBER, &ft_handle);
      
   if(ft_status == FT_OK) //error checking
     {
        printf("\n\n\tConnection with SASEBO successfull\n");
     }
   else
     {
        printf("\n\n\tConnection Failed !");
        printf("\n\tCheck device connection");
     }
	 
	ft_status = FT_ResetDevice(ft_handle);
	 
	if (ft_status == FT_OK) 
	{
	    printf("\n\t Reset OK");
	}
	else 
	{
	    printf("\n\t Reset Failed");
	}
	 
	ft_status = FT_Purge(ft_handle, FT_PURGE_RX | FT_PURGE_TX); // Purge both Rx and Tx buffers
 
	 if (ft_status == FT_OK) 
	{
	    printf("\n\t Purge OK");
	}
	else 
	{
	    printf("\n\t Purge Failed");
	}
	 
	ft_status = FT_SetTimeouts(ft_handle, 100, 100);  
	 
	if (ft_status == FT_OK) 
	{
	    printf("\n\t TimeOut OK");
	}
	else 
	{
	    printf("\n\t TimeOut Failed");
	} 
	 
	 ft_status = FT_SetLatencyTimer(ft_handle, LatencyTimer);
	 
	 
	 
	 // reset
	 //            bus.SbusWrite((uint)Address.CONT, (uint)Cont.IPRST);
     //       bus.SbusWrite((uint)Address.CONT, (uint)Cont.ZERO);
	 SbusWrite(ft_handle, 0x0002, 0x0004);
	 SbusWrite(ft_handle, 0x0002, 0x0000);
	 
	 
	 //  encrypt
	 // bus.SbusWrite((uint)Address.MODE, (uint)(encrypt ? Mode.ENC : Mode.DEC));
	 SbusWrite(ft_handle, 0x000c, 0x0000);
	 
	 //  key
	 //bus.SbusWriteBurst((uint)Address.KEY0, key, key.Length);
     //bus.SbusWrite((uint)Address.CONT, (uint)Cont.KSET);
	 for(i=0;i<8;i++)
		SbusWrite(ft_handle, 0x0100+i*2, 0x0000);

		SbusWrite(ft_handle, 0x0002, 0x0002);
	 
	 //sleep(1);
	waitDone(ft_handle);
	 
	
	printf("\n\n");
	for(loop=0;loop<LIMIT;i++)
	{
		 // generate random plaintext
		 for(i=0;i<8;i++)
		 		pt[i]=rand() % 65536;
                

		  // pt write
		  for(i=0;i<8;i++)
	          SbusWrite(ft_handle, 0x0140+i*2, pt[i]);
	          //  run
	          // bus.SbusWrite((uint)Address.CONT, (uint)Cont.RUN);
	          SbusWrite(ft_handle, 0x0002, 0x0001);
	          waitDone(ft_handle);
	 
	          //ct read
	          for(i=0;i<8;i++)
		      printf("%04x ",SbusRead(ft_handle, 0x0180+i*2));
		  printf("\n");
	 
	 }
	 
	 
    FT_Close(ft_handle);    //Close the connection        
 }

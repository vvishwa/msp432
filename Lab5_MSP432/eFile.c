// eFile.c
// Runs on either TM4C123 or MSP432
// High-level implementation of the file system implementation.
// Daniel and Jonathan Valvano
// September 13, 2016
#include <stdint.h>
#include "eDisk.h"

uint8_t Buff[512];
uint8_t Directory[256], FAT[256];
int32_t bDirectoryLoaded =0; // 0 means disk on ROM is complete, 1 means RAM version active
// Return the larger of two integers.
int16_t max(int16_t a, int16_t b){
  if(a > b){
    return a;
  }
  return b;
}
// if directory and FAT not loaded,
// bring it into RAM from disk
void MountDirectory(void){ 
// if bDirectoryLoaded is 0, 
//    read disk sector 255 and populate Directory and FAT
//    set bDirectoryLoaded=1
// if bDirectoryLoaded is 1, simply return
	
  if(bDirectoryLoaded==0){
    eDisk_ReadSector(Buff,255);  // read disk sector 255
    for(int i=0; i<256; i++){    // populate Directory and FAT
      Directory[i]=Buff[i];
      FAT[i]=Buff[i+256];
    }
    bDirectoryLoaded=1;          // bDirectoryLoaded equal to 1
  }
}

// Return the index of the last sector in the file
// associated with a given starting sector.
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t lastsector(uint8_t start){
	uint16_t m = 0;
  if(start == 255){
    return 255;
  }
  m = FAT[start];
  while(m != 255){
    start = m;
    m = FAT[start];
  }
  return start;
}

// Return the index of the first free sector.
// Note: This function will loop forever without returning
// if a file has no end or if (Directory[255] != 255)
// (i.e. the FAT is corrupted).
uint8_t findfreesector(void){
	int16_t fs=-1,i=0;
	uint8_t ls;
	ls=lastsector(Directory[i]);
	while(ls!=255)
	{
		fs=max(fs,ls);
		i=i+1;
		ls=lastsector(Directory[i]);
	}
  return fs+1; // replace this line
}

// Append a sector index 'n' at the end of file 'num'.
// This helper function is part of OS_File_Append(), which
// should have already verified that there is free space,
// so it always returns 0 (successful).
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t appendfat(uint8_t num, uint8_t n){
	uint8_t i=0;
	uint8_t m=0;
	i=Directory[num];
	if(i!=255)
	{
		m=FAT[i];
		while(m!=255)
		{
		i=m;
		m=FAT[i];
		}
		FAT[i]=n;
		return 0;
	}
		Directory[num]=n;
		return 0;
}

//********OS_File_New*************
// Returns a file number of a new file for writing
// Inputs: none
// Outputs: number of a new file
// Errors: return 255 on failure or disk full
uint8_t OS_File_New(void){
	MountDirectory();
	uint8_t i=0;
	while(i<255)	{
		if(Directory[i]==255)	{
			return i;
		}	else {
			i++;
			if(i==255){
				return 255;
			}
		}
	}
	return 255;
}

//********OS_File_Size*************
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
// Errors:  none
uint8_t OS_File_Size(uint8_t num){
	uint8_t next;
	uint8_t count=0;
	next=Directory[num];
	if(next==255)
	{
		return 0;
	}
	count++;
	while(FAT[next]!=255)
	{
		next=FAT[next];
		count++;
	}
	return count; 
}

//********OS_File_Append*************
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// Errors:  255 on failure or disk full
uint8_t OS_File_Append(uint8_t num, uint8_t buf[512]){
  uint8_t n;
	n=findfreesector();
	if(n==255)
	{
		return 255;
	}
	else 
	{
		eDisk_WriteSector(buf,n);
		appendfat(num,n);
		return 0; // replace this line
	}
}

//********OS_File_Read*************
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
// Errors:  255 on failure because no data
uint8_t OS_File_Read(uint8_t num, uint8_t location,
                     uint8_t buf[512]){
	uint8_t sector;
	uint8_t j;
		
	sector=Directory[num];	//logical address for sector 0										 
	if (sector!=255){
		for (j=1;j<=location;j++){
			sector=FAT[sector]; //logical address for sector j
			if (sector==255){
				return 255;}
		}
		return eDisk_ReadSector(buf,sector);
	}	
  return 255; // replace this line
}

//********OS_File_Flush*************
// Update working buffers onto the disk
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Flush(void){
	uint16_t i;
	
	for (i=0;i<256;i++){
		Buff[i]=Directory[i];
		Buff[i+256]=FAT[i];
		eDisk_WriteSector(Buff, 255);
	}

  return 0; // replace this line
}

//********OS_File_Format*************
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Format(void){
// call eDiskFormat
// clear bDirectoryLoaded to zero
uint8_t code;
uint16_t i;	
	code =eDisk_Format();
	if (code==RES_OK){
		for (i=0;i<=255;i++){
			Directory[i]=0xFF;
			FAT[i]=0xFF;
		}
		bDirectoryLoaded=0;
	}
	return code;
}

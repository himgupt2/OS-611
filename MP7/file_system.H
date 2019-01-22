/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.
    

*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define MB * (0x1 << 20)

#define BLOCK_LIMIT 16
#define BLOCK_SIZE  512
#define DISK_SIZE   (5 MB)
#define MAX_BLOCKS (DISK_SIZE / BLOCK_SIZE)

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

typedef struct node {
    unsigned long fd;
    unsigned long block[BLOCK_LIMIT];
    unsigned long size;
    unsigned long b_size;    
}m_node;

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */ 
/*--------------------------------------------------------------------------*/

class File;

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem {

friend class File; /* -- not sure if we need this; feel free to delete */

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
     
    SimpleDisk * disk;          //Pointer to the disk being mounted on this filesystem

    unsigned char block_map[512];   //A map of blocks for this filesystem, for tracking allocated blocks
    unsigned long total_blocks;  //Total number of blocks managed by this filesystem

    unsigned long m_blocks;     //The number of management blocks for filesystem
    unsigned long m_nodes;      //Number of management nodes used in this filesystem

    unsigned long size;
    
     
public:

    FileSystem();
    /* Just initializes local data structures. Does not connect to disk yet. */
    
    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */
    
    bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */
    
    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */
    
    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */
    
    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */

    int GetBlock();
    void FreeBlock(int block_no);
    void UpdateSize(long size, unsigned long fd, File *file);
    void EraseFile(int _file_id);

    void UpdateBlockData(int fd, int block);
   
};
#endif

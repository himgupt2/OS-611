/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File() {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");
    
    fd              =-1;
    size            = 0;

    c_block         =-1;
    index           = 1;
    position        = 0;

    file_system     = NULL;         //This is initialized during the File Lookup call
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("reading from file\n");
    if (c_block == -1 || file_system == NULL) {
        Console::puts("File not intialized, can not read \n");
        return 0;
    }

    int read = 0;
    int bytes_to_read = _n;

    //TODO probably redundant, check.
    //index = 1;
    //position = 0;
    //c_block = blocks[index-1];
    
    char buf[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
    Console::puts("Reading block "); Console::puti(c_block);Console::puts("\n");

    file_system->disk->read(c_block, (unsigned char *)buf);

    while (!EoF() && (bytes_to_read > 0)) {
        _buf[read] = buf[position];
        bytes_to_read--;
        read++; 
        position++;

        //Reset position if we read the whole block and read again
        if (position >= 512) {
            index++;            
            if (index > 15) {
                // We can not read beyond 16 blocks for 1 file.
                return read;
            }
            c_block = blocks[index-1];
            memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
            file_system->disk->read(c_block, (unsigned char *)buf);
            position = 0;
        }
        
    }
    //Console::puts("Read bytes = ");Console::puti(read);Console::puts("\n");
    return read;
}


void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("writing to file\n");
    
    if (c_block == -1 || file_system == NULL) {
        Console::puts("c block = ");Console::puti(c_block);
        if (file_system == NULL)
            Console::puts("file system NULL\n");
        Console::puts("File not intialized, can not Write \n");
        return;
    }

    //Console::puts("passed buffer "); Console::puts(_buf);Console::puts("\n");
    int write = 0;
    int bytes_to_write = _n;

    unsigned char buf[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.

    file_system->disk->read(c_block, buf);
    //Console::puts("position = "); Console::puti(position);
    while (bytes_to_write > 0 ) {
        buf[position] = _buf[write];
        write++;
        position++;
        bytes_to_write--;
        //Console::puts("position = "); Console::puti(position);
        // Add new block if this block is already full
        if (position >= 512) {
            file_system->disk->write(c_block, (unsigned char *)buf);
            c_block = file_system->GetBlock();
            //TODO add this block in the mnode
            file_system->UpdateBlockData(fd, c_block);
            memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
            file_system->disk->read(c_block, (unsigned char *)buf);
            position = 0;
        }
    }

    //Update the file size and EOF
    file_system->UpdateSize(write, fd, this);

    //Console::puts("Writing to disk   bytes written"); Console::puti(write);Console::puts("\n");
    //Console::puts("Wrote buffer "); Console::puts((const char *)buf);Console::puts("\n");

    file_system->disk->write(c_block, buf);
}

void File::Reset() {
    Console::puts("reset current position in file\n");
    position = 0;
    c_block  = blocks[0];
    
}

void File::Rewrite() {
    Console::puts("erase content of file\n");
    //Erase the content from disk management nodes and free the blocks
    file_system->EraseFile(fd);

    for (int i = 1; i < 16; i++) {
        blocks[i] = 0;
    }
}


bool File::EoF() {
    //Console::puts("testing end-of-file condition\n");
    char buf[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.

    //file_system->disk->read(c_block, (unsigned char *)buf);
 /*   if (buf[position] == -1) 
        return true;
*/

    //Console::puts("index: ");Console::puti(index);Console::puts(" position: ");Console::puti(position);Console::puts("\n");
    //Console::puts("size : ");Console::puti(size);Console::puts("\n");
    if ( ( ((index - 1)*512) + position ) > size )
        return true;

    return false;
}

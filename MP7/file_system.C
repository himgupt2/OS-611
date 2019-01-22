/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define NODES_PER_BLOCK (BLOCK_SIZE/sizeof(m_node))

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    FileSystem::disk    = NULL;
    total_blocks        = 0;
    m_blocks            = 0;
    m_nodes             = 0;
    size                = 0;
    
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system form disk\n");
    if (disk == NULL) {
        disk = _disk;
        return true;
    }
    return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
    Console::puts("formatting disk\n");
  /*  
    if (_size > size) {
        Console::puti(size);
        Console::puts("Invalid file size \n");
        return false;
    }  */

    FileSystem::disk            = _disk;
    FileSystem::size            = _size;
    FileSystem::total_blocks    = (FileSystem::size / BLOCK_SIZE) + 1;
    FileSystem::m_nodes         = (FileSystem::total_blocks/ BLOCK_LIMIT) + 1;
    FileSystem::m_blocks        = ((FileSystem::m_nodes * sizeof(m_node)) / BLOCK_SIZE ) + 1;

    

    // TODO set map ??
    for (int j = 0; j < (total_blocks/8); j++){
        block_map[j] = 0; 
        //Console::puti(block_map[j]);Console::puts(" ");
    }

    int i;
    for (i = 0; i < (m_blocks/8) ; i++) {
        FileSystem::block_map[i] = 0xFF;          // Set the current blocks to 1
    }

    block_map[i]  = 0;
    for (int j = 0; j < (m_blocks%8) ; j++) {
        FileSystem::block_map[i] = block_map[i] | (1 << j) ; // Set the remaining blocks to 1
    }

    char buf[512];
    memset(buf,0,512);
    for (int j = 0; j < total_blocks; j++) {
        disk->write(j, (unsigned char *)buf);
    }

    /*Console::puts("m blocks "); Console::puti(m_blocks);Console::puts("\n");
    for (int j = 0; j < (total_blocks/8); j++){
        Console::puti(block_map[j]);Console::puts(" ");
    }*/

    return true;
}

File * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file\n");
    
    File * file = (File *) new File();
    char buf[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.

    for(int i = 0; i < m_blocks; i++) {
        //Console::puts("Reading the disk for file look up \n");

        memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
        disk->read(i, (unsigned char *)buf);
        m_node* m_node_l = (m_node *)buf;
        for (int j = 0; j < NODES_PER_BLOCK; j++) {
            if (m_node_l[j].fd == _file_id) {
                //TODO set file params
                file->fd        = _file_id;
                file->size      = m_node_l[j].size;
                file->c_block   = m_node_l[j].block[0];
                file->index     = 1;
                file->position  = 0;

                //Set the block numbers for this file.
                for(int k = 0; k <BLOCK_LIMIT; k++) {
                    file->blocks[k] = m_node_l[j].block[k];
                }
                //file->file_system = this;
                file->file_system = FILE_SYSTEM;
                if (file->file_system == NULL)
                    Console::puts("File system NULL in lookup");Console::puts("\n");
                Console::puts("Found file with id ");Console::puti(_file_id);Console::puts("\n");
                return file;
            }
        }
    }

    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file\n");

    if (LookupFile(_file_id) != NULL) {
        Console::puts("File already exists with this id, choose new id\n");
        return false;
    }

    char buf[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.

    for (int i = 0; i < m_blocks; i++) {

        memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
        disk->read (i, (unsigned char *)buf);
        m_node* m_node_l = (m_node *) buf;

        for (int j = 0; j < NODES_PER_BLOCK; j++) {
            if (m_node_l[j].fd == 0) {
                m_node_l[j].fd = _file_id;
                //TODO implement an api to get a free block number

                m_node_l[j].block[0] = GetBlock();
                Console::puts("get block "); Console::puti(m_node_l[j].block[0]);
                m_node_l[j].b_size   = 1;

                disk->write(i, (unsigned char *)buf);
                return true;
            }
        }
    }

    return false;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file\n");
    
    char buf[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.

    for (int i = 0; i < m_blocks; i++) {

        memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
        disk->read (i, (unsigned char *)buf);
        m_node* m_node_l = (m_node *) buf;

        for (int j = 0; j < NODES_PER_BLOCK; j++) {
            if (m_node_l[j].fd == _file_id) {
                m_node_l[j].fd = 0;
                m_node_l[j].size = 0;
                m_node_l[j].b_size = 0;
                for (int k = 0; k < BLOCK_LIMIT; k++) {
                    if (m_node_l[j].block[k] != 0) {
                        FreeBlock(m_node_l[j].block[k]);
                    }
                    m_node_l[j].block[k] = 0;
                }
                disk->write(i, (unsigned char *)buf);
                return true;
            }
        }
    }
    Console::puts("File Not found, check id \n");
    return false;
}

void FileSystem::EraseFile(int _file_id) {
    Console::puts("Erasing File Content \n");

    char buf[512];
    char buf_2[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
    memset(buf_2, 0, 512);

    for (int i = 0; i < m_blocks; i++) {

        memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
        disk->read (i, (unsigned char *)buf);
        m_node* m_node_l = (m_node *) buf;

        for (int j = 0; j < NODES_PER_BLOCK; j++) {
            if (m_node_l[j].fd == _file_id) {
                
                m_node_l[j].size = 0;
                m_node_l[j].b_size = 0;
                for (int k = 0; k < BLOCK_LIMIT; k++) {
                    if (m_node_l[j].block[k] != 0) {
                        //Erase the content of the block and then free the block from used set.
                        disk->write(m_node_l[j].block[k], (unsigned char *)buf_2);
                        if (k!=0) {             // Dont free the first block of the file. Just erase the content.
                            FreeBlock(m_node_l[j].block[k]);
                            m_node_l[j].block[k] = 0;
                        }
                    }
                    
                }
                disk->write(i, (unsigned char *)buf);
                return;
            }
        }
    }

}


int FileSystem::GetBlock() {

    // We need to reserve the first m_blocks in the disk for file system management.
    //TODO

    Console::puts("Total blocks "); Console::puti(total_blocks/8);Console::puts("\n");

    for (int i = 0; i < (total_blocks / 8); i++) {
        if (block_map[i] != 0xFF) {
            for (int j = 0; j < 8; j++) {
                if (block_map[i] & (1 << j)) {
                    continue;
                } else {
                    block_map[i] = block_map[i] | (1 << j);
                    int b= j + i*8;
                    Console::puts("Allocating block number");Console::puti(b);Console::puts("\n");
                    return b;
                }
            }
        }
    }
    Console::puts("returning block 0\n");
    return 0;
}

void FileSystem::FreeBlock(int block_no) {
    
    int node = block_no / 8;
    int index = block_no % 8;

    block_map[node] = block_map[node] | (1 << index) ;
    block_map[node] = block_map[node] ^ (1 << index) ;
}

void FileSystem::UpdateSize(long size, unsigned long fd, File *file) {

    Console::puts("Updating the block size \n");
    char buf[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.

    for (int i = 0; i < m_blocks; i++) {

        memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
        disk->read (i, (unsigned char *)buf);
        m_node* m_node_l = (m_node *) buf;

        for (int j = 0; j < NODES_PER_BLOCK; j++) {
            if (m_node_l[j].fd == fd) {
                
                m_node_l[j].size += size;
                file->size = m_node_l[j].size;
                //Console::puts("Size updated to :");Console::puti(file->size);
                disk->write(i, (unsigned char *)buf);
                return;
            }
        }
    }
    Console::puts("File with this fd not found for size update\n");
    return;
}

void FileSystem::UpdateBlockData(int fd, int block) {

    Console::puts("Updating the block data \n");
    char buf[512];
    memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.

    for (int i = 0; i < m_blocks; i++) {

        memset(buf, 0, 512);        //set the buffer to 0, to be used in reading the disk.
        disk->read (i, (unsigned char *)buf);
        m_node* m_node_l = (m_node *) buf;

        for (int j = 0; j < NODES_PER_BLOCK; j++) {
            if (m_node_l[j].fd == fd) {
                
                m_node_l[j].b_size += 1;
                m_node_l[j].block[m_node_l[j].b_size] = block;

                disk->write(i, (unsigned char *)buf);
                return;
            }
        }
    }
    Console::puts("File with this fd not found for block update\n");
    return;
}

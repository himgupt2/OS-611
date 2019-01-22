#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

#define PAGE_DIRECTORY_FRAME_SIZE 1

#define PAGE_PRESENT        1
#define PAGE_WRITE          2
#define PAGE_LEVEL          4

#define PD_SHIFT            22
#define PT_SHIFT            12

#define PDE_MASK            0xFFFFF000
#define PT_MASK             0x3FF

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   //assert(false);

    PageTable::kernel_mem_pool = _kernel_mem_pool;
    PageTable::process_mem_pool = _process_mem_pool;
    PageTable::shared_size = _shared_size;

    Console::puts("Initialized Paging System\n");
}

/*
 * bit 0 - page present / not present
 * bit 1 - read / read and write
 * bit 2 - kernel / user mode
 */

PageTable::PageTable()
{
   //assert(false);

    page_directory = (unsigned long *)(kernel_mem_pool->get_frames
                                (PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE);

    unsigned long mask_addr = 0;
    unsigned long * direct_map_page_table = (unsigned long *)(kernel_mem_pool->get_frames
                                                        (PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE);

    unsigned long shared_frames = ( PageTable::shared_size / PAGE_SIZE);
    //Console::puts("shared frames : "); Console::puti(shared_frames);

    /*
     * Mark the shared memory with
     * kernel mode | read & write | present
     */
    for(int i = 0; i < shared_frames; i++) {
        direct_map_page_table[i] = mask_addr | PAGE_WRITE | PAGE_PRESENT ;
        mask_addr += PAGE_SIZE;
    }

    //set the first page entry
    page_directory[0] = (unsigned long)direct_map_page_table | PAGE_WRITE | PAGE_PRESENT;

    mask_addr = 0;
    //mark the non shared memory page entries
    for(int i = 1; i< shared_frames; i++) {
        page_directory[i] = mask_addr | PAGE_WRITE;
    }

    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   //assert(false);

    current_page_table = this;
    write_cr3((unsigned long)page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   //assert(false);

    paging_enabled = 1;

    /*
     * Set the paging bit
     * Taken from http://www.osdever.net/tutorials/view/implementing-basic-paging
     */
    write_cr0(read_cr0() | 0x80000000);
    
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    //assert(false);

    // read the base page directory address
    unsigned long * cur_pg_dir = (unsigned long *) read_cr3();

    // read the page address where fault occurred
    unsigned long page_addr = read_cr2();
    unsigned long PD_addr   = page_addr >> PD_SHIFT;
    unsigned long PT_addr   = page_addr >> PT_SHIFT;

    unsigned long * page_table = NULL;
    unsigned long error_code = _r->err_code;

    unsigned long mask_addr = 0;

    /*
     * --10bits for PD-- --10bits for PT-- --12bit offset--
     * 0000 0000 00       0000 0000 00      00 0000 0000
     */

    if ((error_code & PAGE_PRESENT) == 0 ) {
        if ((cur_pg_dir[PD_addr] & PAGE_PRESENT ) == 1) {  //fault in Page table
            page_table = (unsigned long *)(cur_pg_dir[PD_addr] & PDE_MASK);
            page_table[PT_addr & PT_MASK] = (PageTable::process_mem_pool->get_frames
                                                (PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE) | PAGE_WRITE | PAGE_PRESENT ;

        } else {
            cur_pg_dir[PD_addr] = (unsigned long) ((kernel_mem_pool->get_frames
                                                (PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE) | PAGE_WRITE | PAGE_PRESENT);

            page_table = (unsigned long *)(cur_pg_dir[PD_addr] & PDE_MASK);

            for (int i = 0; i<1024; i++) {
                page_table[i] = mask_addr | PAGE_LEVEL ; // set the pages as user page
            }

            page_table[PT_addr & PT_MASK] = (PageTable::process_mem_pool->get_frames
                                                (PAGE_DIRECTORY_FRAME_SIZE)*PAGE_SIZE) | PAGE_WRITE | PAGE_PRESENT ;

        }
    }

    Console::puts("handled page fault\n");
}


/*
 File: vm_pool.C
 
 Author: Himanshu Gupta
 Date  : October 8 2018
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "page_table.H"
#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_addr = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;

    region_no = 0;
    //alloc_reg = (alloc_reg_ *)(Machine::PAGE_SIZE * (frame_pool->get_frames(1)));
    alloc_reg = (struct alloc_reg_ *) (base_addr);

    page_table->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {

    unsigned long addr;

    if (size == 0){
        Console::puts("0 size invalid for allocate");
        return 0;
    }

    assert(region_no < MAX_REGIONS);

    //allocate memory in size of pages

    unsigned b = _size % (Machine::PAGE_SIZE) ;

    unsigned long frames = _size / (Machine::PAGE_SIZE) ;

    if (b > 0)
        frames++;

    if (region_no == 0) {
        // the first frame is given to the list of allocator maintaining this region
        addr = base_addr;
        alloc_reg[region_no].base_addr  = addr + Machine::PAGE_SIZE ;
        alloc_reg[region_no].size = frames*(Machine::PAGE_SIZE) ;
        region_no++;
        return addr + Machine::PAGE_SIZE;
    } else {
        addr = alloc_reg[region_no - 1].base_addr + alloc_reg[region_no - 1].size ;
    }

    alloc_reg[region_no].base_addr  = addr;
    alloc_reg[region_no].size = frames*(Machine::PAGE_SIZE);

    region_no++;
    Console::puts("Allocated region of memory.\n");

    return addr;
}

void VMPool::release(unsigned long _start_address) {
    int cur_reg_no = -1;

    for (int i = 0; i < MAX_REGIONS; i++) {
        if (alloc_reg[i].base_addr == _start_address) {
            cur_reg_no = i;
            break;
        }
    }
    assert(!(cur_reg_no < 0));

    unsigned int alloc_pages = ( (alloc_reg[cur_reg_no].size) / (Machine::PAGE_SIZE) ) ;

    for (int i = 0 ; i < alloc_pages ;i++) {
        page_table->free_page(_start_address);
        _start_address += Machine::PAGE_SIZE;
    }

    // Fix the allocated list by removing the current region.

    for (int i = cur_reg_no; i < region_no - 1; i++) {
        alloc_reg[i] = alloc_reg[i+1];
    }
    region_no--;

    page_table->load();

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {

    // check for legitmate boundaries. 
        int size_l = base_addr + size;
        int base = base_addr;
        if ((_address < size_l) && (_address >= base))
            return true;
 
    return false;
}


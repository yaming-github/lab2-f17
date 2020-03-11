#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
  int i, find = 0;
  int va = PGROUNDUP(myproc()->sz);
  char* mem;

  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    if(shm_table.shm_pages[i].id == id){
      find = 1;
      break;
    }
  }

  if(find){
    mappages(myproc()->pgdir, (void *)va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
    shm_table.shm_pages[i].refcnt++;
  } else {
    for (i = 0; i< 64; i++) {
      if(shm_table.shm_pages[i].id == 0)
        break;
    }
    shm_table.shm_pages[i].id = id;
    mem = kalloc();
    memset(mem, 0, PGSIZE);
    shm_table.shm_pages[i].frame = mem;
    mappages(myproc()->pgdir, (void *)va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
    shm_table.shm_pages[i].refcnt++;
  }

  *pointer = (char *)va;
  myproc()->sz = va + PGSIZE;
  release(&(shm_table.lock));

//you write this

return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
  int i, find = 0;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    if(shm_table.shm_pages[i].id == id){
      find = 1;
      break;
    }
  }
  if(find){
    shm_table.shm_pages[i].refcnt--;
    if(!shm_table.shm_pages[i].refcnt){
      shm_table.shm_pages[i].id = 0;
      shm_table.shm_pages[i].frame = 0;
    }
  } else {
    cprintf("ERROR!NO SUCH PAGE!\n");
  }  
  release(&(shm_table.lock));  

//you write this too!

return 0; //added to remove compiler warning -- you should decide what to return
}

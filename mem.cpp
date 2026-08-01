#include "mem.hpp"
Memory::Memory(dword ram_size,dword mem_mask):mem_mask(mem_mask) ,ram(ram_size),used(ram_size,false){
    uart=false;
} 
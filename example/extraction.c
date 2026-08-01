// Extracts the executable program image from a Calypsi-generated .bin file, discarding unrelated runtime data.
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
int  main(){
    FILE*f=fopen("kernel.bin","rb");
    if(!f){
        printf("Cant open the file."); 
        return 1; 
    }
    fseek(f,0,SEEK_END);
    int size=ftell(f);
    fseek(f,0,SEEK_SET);
    uint8_t*data=malloc(size);
    fread(data,1,size,f);
    fclose(f);
    int offset=0x34;
    for(int i=0;i<4;i++){
        int base=offset+(32*i);
        uint32_t p_offset = *(uint32_t*)&data[base + 4];
        uint32_t p_vaddr  = *(uint32_t*)&data[base + 8];
        uint32_t p_filesz = *(uint32_t*)&data[base + 16];
        if(p_filesz>0&&p_vaddr==0x8100){
            FILE*f=fopen("kernel_raw.bin","wb");
            fwrite(&data[p_offset],1,p_filesz,f);
            fclose(f); 
            printf("Extracted %u bytes from offset 0x%X (vaddr 0x%X)\n",p_filesz, p_offset, p_vaddr);
            break;
        }
    } 
    free(data);
    return 0;
}
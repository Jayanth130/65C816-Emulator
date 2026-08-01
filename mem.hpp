  #pragma once 
  #include "util.hpp" 
  #include<vector>
  #include<string>
  using namespace util; 
  class Memory{
    private :
      std::vector<byte>ram;
      dword mem_mask; 
    public:
      bool uart;
      std::string uart_buffer;
      std::vector<bool>used;
      Memory(dword ram_size,dword mem_Mask) ;

      inline void set_byte(dword addr,byte data){
          addr&=mem_mask;
          if(addr==0x00FFD6){
             uart_handler(data);
          }
          ram[addr]=data;
          used[addr]=true;
      }
      inline void set_word(dword addr, word data){
          set_byte(addr,lower_byte(data));  
          set_byte(addr+1,higher_byte(data));
      }
      inline void set_dword(dword addr,dword data){
          set_word(addr,(word)data);
          set_byte(addr+2,(byte)data>>16);
      }
      inline byte get_byte(dword addr){
          addr&=mem_mask;
          if(addr==0x00FFD7) return 0x01; 
          return (ram[addr]);         
      }
      inline word get_word(dword addr){
          return join_bytes(get_byte(addr),get_byte(addr+1));
      }
      inline dword get_addr(dword addr){
        return join_addr(get_byte(addr+2),get_word(addr));
      }
      inline void uart_handler(byte data){
            uart=true;
            uart_buffer.push_back(data);
      }

  };  

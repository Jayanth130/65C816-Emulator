    #ifndef UTIL_H
    #define UTIL_H
    #include<cstdint>
    namespace util{ 
            using byte=std::uint8_t ;
            using word=std::uint16_t;
            using dword=std::uint32_t;
                        
            inline byte lower_byte(word value){
                return  (byte)value; 
            }
            inline byte higher_byte(word value){
                return value>>8; 
            }
            inline word join_bytes(byte lower,byte higher){
                return ((word)higher<<8|lower);
            }
            inline dword bank(byte value){
                return (dword)value<<16; 
            }
            inline dword join_addr(byte value,word pc){
                return (bank(value)|pc);
            }

    }
    #endif 
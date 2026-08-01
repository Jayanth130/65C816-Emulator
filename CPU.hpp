#pragma once
#include "mem.hpp"
#include<cstdio>
#include<iostream> 
class CPU{
    public: 
        CPU(Memory&mem,bool decimal, bool step);
        void reset();
        void run(bool m,int bytes,dword addr);

    private :
        int ins_count;
        using addr_fnptr=dword(CPU::*)();
        using op_fnptr=void(CPU::*)(dword);
        struct Instruction{
            op_fnptr operation;
            addr_fnptr mode;
        }op_entry[256];

        Memory&mem; 
        union{
            byte b;
            word w;
        }a,x,y,sp,dp;
       
        dword op_addr;
        word sp_top;
        byte p;
        word pc;
        byte pbr,dbr;
        bool running;
        bool irq_pending=false;
        bool nmi_pending=false;
        bool waiting=false;

        enum flags:byte{
            C=0x01,
            Z=0x02,
            I=0x04,
            D=0x08,
            X=0x10,
            M=0x20,
            V=0x40,
            N=0x80
        };
        bool emu;
        bool decimal;
        bool step;
        void trace(const char*str,bool no_space,bool sft);
        void execute();
        void init_tables();

        inline void trigger_nmi(){
            nmi_pending=true;
        }
        inline void trigger_irq(){
            irq_pending=true;
        }

        inline void print_val(word val){
            if(decimal){
                if(emu||(p&M)){
                 printf("%03d ",(int8_t)val);
                }
                else printf("%05d ",(int16_t)val);
            }
            else {
                if(emu||(p&M)){
                 printf("%02X ",val);
                }
                else printf("%04X ",val);
                
            }
        }

        inline void push_byte(byte data){
            mem.set_byte(sp.w,data);
            if(!emu){
                sp.w--;
                return; 
            }
            sp.b--;
        }
        inline void push_word(word data){
            push_byte(higher_byte(data));
            push_byte(lower_byte(data));
        }
        inline byte pull_byte(){
             if(!emu) sp.w++;
             else sp.b++;
             return mem.get_byte(sp.w);
        }
        inline word pull_word(){
            byte low=pull_byte();
            byte high=pull_byte();
            return join_bytes(low,high);
        }
        inline dword abs_addr(){
            word oldPc=pc;  
            pc+=2;
            return join_addr(dbr,mem.get_word(join_addr(pbr,oldPc)));
        }
        inline dword  abs_addrL(){
            word oldPc=pc;
            pc+=3;
            return mem.get_addr(join_addr(pbr,oldPc));
        }
        inline dword abs_ind(){
            dword cur=join_addr(0,mem.get_word(join_addr(pbr,pc)));
            pc+=2 ;
            return join_addr(0,mem.get_word(cur));
        }
        inline dword abs_addrX(){
            return (emu||is_set(X)?x.b:x.w)+abs_addr();
        }
        inline dword abs_addrY(){
            return (emu||is_set(X)?y.b:y.w)+abs_addr();
        }
        inline dword abs_indX(){
                dword cur=join_addr(0,mem.get_word(join_addr(pbr,pc)))+(emu||is_set(X)?x.b:x.w);
                pc+=2 ;
                return join_addr(0,mem.get_word(cur));
        }
        inline dword abs_indY(){
            dword cur=join_addr(0,mem.get_word(join_addr(pbr,pc)))+(emu||is_set(X)?y.b:y.w);
            pc+=2 ;
            return join_addr(0,mem.get_word(cur));
        }
        inline dword abs_indL(){
            dword cur=join_addr(0,mem.get_word(join_addr(pbr,pc)));
            pc+=2;
            return mem.get_addr(cur);
        }
        inline dword abs_addrLX(){
            return abs_addrL()+(emu||is_set(X)?x.b:x.w);
        }
        //d Direct Page     DP + d
        inline dword dp_addr(){
            word oldPc=pc;
            pc+=1;
            return join_addr(0,dp.w+(word)mem.get_byte(join_addr(pbr,oldPc)));
        }
        //d,X Direct Page Indexed X     DP + d + X
        inline dword dp_addrX(){    
            word oldpc = pc;
            pc +=1;
            word offset=(word)mem.get_byte(join_addr(pbr,oldpc))+((emu||(p&X))?(word)x.b:x.w);
            return join_addr(0, dp.w+offset);
        }
        //d,Y Direct Page Indexed X     DP + d + Y
        inline dword dp_addrY(){
            word oldpc=pc;
            pc+=1;
            word offset=(word)mem.get_byte(join_addr(pbr,oldpc))+((emu||(p&X))?(word)y.b:y.w);
            return join_addr(0,dp.w+offset);
        }
        //(d) Direct Page Indirect   DBR : [DP+d]
        inline dword dp_ind(){
            word offset=mem.get_byte(join_addr(pbr,pc));
            pc+=1;
            return join_addr(dbr,mem.get_word((dword)dp.w+offset));
        }
        //(d,X) Direct Page Indexed Indirect    DBR : [DP+d+X]
        inline dword dp_indX(){
            word offset=mem.get_byte(join_addr(pbr,pc));
            pc+=1;
            return join_addr(dbr,mem.get_word((dword)(dp.w+offset+((emu||(p&X))?x.b:x.w))));
        }
        //(d),Y Direct Page Indirect Indexed    (DBR:[DP+d]) + Y
        inline  dword dp_indY(){
            word offset=mem.get_byte(join_addr(pbr,pc));
            pc+=1;
            return join_addr(dbr,mem.get_word((dword)offset+dp.w))+((emu||(p&X))?y.b:y.w);
        }
        //[d] Direct Page Indirect Long 24-bit pointer at DP+d
        inline dword dp_indL(){
            word offset =mem.get_byte(join_addr(pbr,pc));
            pc+=1;
            return mem.get_addr((dword)offset+dp.w);
        }
        //[d],Y Direct Page Indirect Long Indexed  (24-bit pointer at DP+d) + Y
        inline dword dp_indLY(){
            return dp_indL()+((emu||(p&X))?y.b:y.w);
        }       
        //d,S Stack Relative SP + d
        inline dword dp_sp(){
            word offset=mem.get_byte(join_addr(pbr,pc));
            pc+=1;
            return sp.w+offset;
        }
        //(d,S),Y Stack Relative Indirect Indexed ([SP+d])+ Y
        inline dword dp_spY(){
            word cur=dp_sp();
            return  join_addr(dbr,mem.get_word(cur))+((emu||(p&X))?y.b:y.w);
        }
        inline dword  imm_b(){
            dword addr=join_addr(pbr,pc);
            pc++;
            return addr;
        }
        inline dword imm_w(){
            dword addr=imm_b();
            pc++;       
            return addr;
        }
        inline dword imm_M(){
            if(emu||is_set(M)){
                return imm_b();
            }
            return imm_w();
        }
        inline dword imm_X(){
            if(emu||is_set(X)){
                return imm_b();
            }
            return imm_w();
        }
        inline void set_flag(flags f){
            p|=f;
        }
        inline void clear_flag(flags f){
            p&=~f;
        }
        inline bool is_set(flags f){
            return p&f;
        }
        inline void update_f(flags f,bool value){
            if(value) set_flag(f);
            else clear_flag(f);
        }
        inline  void report(const char*str,bool no_space=false,bool sft=false){
             if(step) trace(str,no_space,sft);


        }
        inline void update_nzB(byte val){
                update_f(N,val&0x80);
                update_f(Z,val==0);
        }
        inline void update_nzW(word val){
                update_f(N,val&0x8000);
                update_f(Z,val==0);
        }
       inline dword implied(){return 0;}

       inline void op_adc(dword addr){
            if(emu||(p&M)){ 
                byte data=mem.get_byte(addr);
                word cur=a.b+data+(p&C);
                if(p&D){
                    if((cur&0xf)>0x09) cur+=0x06;
                    if((cur&0xf0)>0x90) cur+=0x60;
                }
                update_f(C,cur&0x100);
                update_f(V,(~(a.b^data)&(a.b^cur)&(0x80)));
                update_nzB(cur);
                a.b=(byte)cur;
            }
            else{
                word data=mem.get_word(addr);
                dword cur=a.w+data+(p&C);
                if(p&D){
                    if((cur&0xf)>0x09) cur+=0x06;
                    if((cur&0x00f0)>0x90) cur+=0x60;
                    if((cur&0x0f00)>0x900) cur+=0x600;
                    if((cur&0xf000)>0x9000) cur+=0x6000;
                }
                update_f(C,cur&0x10000);
                update_f(V,(~(a.b^data)&(a.b^cur)&(0x8000)));
                update_nzW(cur);
                a.w=(word)cur;
            } 
            report("ADC");
        }
        inline void op_sbc(dword addr){
            if(emu||(p&M)){
                byte data=~mem.get_byte(addr);
                word cur=a.b+data+(p&C);
                if(p&D){
                    if((cur&0xf)>0x09) cur+=0x06;
                    if((cur&0xf0)>0x90) cur+=0x60;
                }
                update_f(C,cur&0x100);
                update_f(V,(~(a.b^data)&(a.b^cur)&(0x80)));
                update_nzB(cur);
                a.b=(byte)cur;
            }
            else{
                word data=~mem.get_word(addr);
                dword cur=a.w+data+(p&C);
                if(p&D){
                    if((cur&0xf)>0x09) cur+=0x06;
                    if((cur&0x00f0)>0x90) cur+=0x60;
                    if((cur&0x0f00)>0x900) cur+=0x600;
                    if((cur&0xf000)>0x9000) cur+=0x6000;
                }
                update_f(C,cur&0x10000);
                update_f(V,(~(a.b^data)&(a.b^cur)&(0x8000)));
                update_nzW(cur);
                a.w=(word)cur;
            } 
            report("SBC");
        }
        inline void op_stp(dword implied){
            running=false;
            report("STP"); 
        }
        inline void op_xce(dword implied){
            bool temp=emu;
            emu=is_set(C);
            update_f(C,temp);
            if(emu){
                set_flag(M);
                set_flag(X);
                sp.w=sp.b|0x0100;
            }
            report("XCE");
        }
        inline void op_sep(dword addr){
            p|=(mem.get_byte(addr));
             if(emu){
                set_flag(M);
                set_flag(X);
            }
            report("SEP");
        }
        inline void op_rep(dword addr){
            p&=(~mem.get_byte(addr));
             if(emu){
                set_flag(M);
                set_flag(X);
            }
            report("REP");
        }
        inline void op_clc(dword implied){
              clear_flag(C);
              report("CLC");
        }
        inline void op_cld(dword implied){
            clear_flag(D);
            report("CLD");
        }
        inline void op_cli(dword implied){
              clear_flag(I);
              report("CLI");
        }
        inline void op_clv(dword implied){
              clear_flag(V);
              report("CLV");
        }
        inline void op_sec(dword implied){
            set_flag(C);
            report("SEC");
        }
        inline void op_sed(dword implied){
            set_flag(D);
            report("SED");
        }
        inline void op_sei(dword implied){
            set_flag(I);
            report("SEI");
        }
        inline void op_lda(dword addr){
            if(emu||(p&M)){
                a.b=mem.get_byte(addr);
                update_nzB(a.b);
            }
            else{
                a.w=mem.get_word(addr);
                update_nzW(a.w);
            }
            report("LDA");
        }
        inline void op_ldx(dword addr){
            if(emu||(p&X)){
                x.b=mem.get_byte(addr);
                update_nzB(x.b);
            }   
            else{
                x.w=mem.get_word(addr);
                update_nzW(x.w);
            }
            report("LDX");
        }
        inline void op_ldy(dword addr){
            if(emu||(p&X)){
                y.b=mem.get_byte(addr);
                update_nzB(y.b);
            }
            else{
                y.w=mem.get_word(addr);
                update_nzW(y.w);
            }
            report("LDY");
        }
        inline void op_sta(dword addr){
            if(emu||is_set(M)){
                 mem.set_byte(addr,a.b);
            }
            else{
                mem.set_word(addr,a.w);
            }
            report("STA");
            
        }
        inline void op_stx(dword addr){
            if(emu||is_set(X)){
                mem.set_byte(addr,x.b);
            }
            else{
                mem.set_word(addr,x.w);
            }
            report("STX");
        }
        inline void op_sty(dword addr){
            if(emu||is_set(X)){
                mem.set_byte(addr,y.b);
            }
            else{
               mem. set_word(addr,y.w);
            }
            report("STY");
        }
        inline void op_stz(dword addr){
            if(emu||is_set(M)){
                mem.set_byte(addr,0x0);
            }
            else{
                mem.set_word(addr,0x0);
            }
            report("STZ");
        }
        inline void op_cmp(dword addr){
            if(emu||is_set(M)){
                byte data=mem.get_byte(addr);
                update_f(C,a.b>=data);
                update_nzB(a.b-data);       
            }
            else{
                word data=mem.get_word(addr);
                update_f(C,a.w>=data);
                update_nzB(a.w-data);
            }
            report("CMP");
        }
        inline void op_cpx(dword addr){
            if(emu||is_set(X)){
                byte data=mem.get_byte(addr);
                update_f(C,x.b>=data);
                update_nzB(x.b-data);       
            }
            else{
                word data=mem.get_word(addr);
                dword temp=x.w-data;
                update_f(C,x.w>=data);
                update_nzB(x.w-data);
            }
            report("CPX");
        }
        inline void op_cpy(dword addr){
            if(emu||is_set(X)){
                byte data=mem.get_byte(addr);
                update_f(C,y.b>=data);
                update_nzB(y.b-data);       
            }
            else{
                word data=mem.get_word(addr);
                update_f(C,y.w>=data);
                update_nzB(y.w-data);
            }
            report("CPY");
        }
        inline void op_inc(dword addr){
            if(emu||is_set(M)){
                byte data=mem.get_byte(addr);
                mem.set_byte(addr,++data);
                update_nzB(data);
            }
            else{
                word data=mem.get_word(addr);
                mem.set_word(addr,++data);
                update_nzW(data);
            }
            report("INC");
        }
        inline void op_dec(dword addr){
            if(emu||is_set(M)){
                byte data=mem.get_byte(addr);
                mem.set_byte(addr,--data);
                update_nzB(data);
            }
            else{
                word data=mem.get_word(addr);
                mem.set_word(addr,--data);
                update_nzW(data);
            }
            report("DEC");
        }
        inline void op_ina(dword implied){
            if(emu||is_set(M)){
                update_nzB(++a.b);
            }
            else{
                update_nzW(++a.w);
            }
            report("INA");
        }
        inline void op_dea(dword implied){
            if(emu||is_set(M)){
                update_nzB(--a.b);
            }
            else{
                update_nzW(--a.w);
            }
            report("DEA");
        }
        inline void op_inx(dword implied){
            if(emu||is_set(X)){
                update_nzB(++x.b);
            }
            else{
                update_nzW(++x.w);
            }
            report("INX");
        }
        inline void op_dex(dword implied){
            if(emu||is_set(X)){
                update_nzB(--x.b);
            }
            else{
                update_nzW(--x.w);
            }
            report("DEX");
        }
        inline void op_iny(dword implied){
            if(emu||is_set(X)){
                update_nzB(++y.b);
            }
            else{
                update_nzW(++y.w);
            }
            report("INY");
        }
        inline void op_dey(dword implied){
            if(emu||is_set(X)){
                update_nzB(--y.b);
            }
            else{
                update_nzW(--y.w);
            }
            report("DEY");
        }
        inline void op_bit(dword addr){
            if(emu||is_set(M)){
                byte data=mem.get_byte(addr);
                update_f(Z,(data&a.b)==0);
                update_f(N,data&0x80);
                update_f(V,data&0x40);
            }
            else{
                word data=mem.get_word(addr);
                update_f(Z,(data&a.w)==0);
                update_f(N,data&0x8000);
                update_f(V,data&0x4000);
            }
            report("BIT");
        }
        inline void op_tsb(dword addr){
            if(emu||is_set(M)){
                byte data=mem.get_byte(addr); 
                update_f(Z,(data&a.b)==0);
                mem.set_byte(addr,data|a.b);  
            }
            else{
                 word data=mem.get_word(addr); 
                update_f(Z,(data&a.w)==0);
                mem.set_word(addr,data|a.w); 
            }
            report("TSB");
        }
        inline void op_trb(dword addr){
            if(emu||is_set(M)){
                byte data=mem.get_byte(addr); 
                update_f(Z,(data&a.b)==0);
                mem.set_byte(addr,data&~a.b);  
            }
            else{
                 word data=mem.get_word(addr); 
                update_f(Z,(data&a.w)==0);
                mem.set_word(addr,data&~a.w); 
            }
            report("TRB");
        }   
        inline void op_aslA(dword implied){
            if(emu||is_set(M)){
                update_f(C,a.b&0x80);
                a.b<<=1;
                update_nzB(a.b);
            }
            else{
                update_f(C,a.w&0x8000);
                a.w<<=1;
                update_nzW(a.w);
            }
            report("ASL");
        } 
        inline void op_asl(dword addr){
             if(emu||is_set(M)){
                byte data=mem.get_byte(addr);
                update_f(C,data&0x80);
                mem.set_byte(addr,data<<=1);
                update_nzB(data);
            }
            else{
                word data=mem.get_word(addr);
                update_f(C,data&0x8000);
                mem.set_word(addr,data<<=1);
                update_nzW(data);
            }
            report("ASL");
        }
        inline void op_lsrA(dword implied){
            if(emu||is_set(M)){
                update_f(C,a.b&0x01);
                a.b>>=1;
                update_nzB(a.b);
            }
            else{
                update_f(C,a.w&0x0001);
                a.w>>=1;
                update_nzW(a.w);
            }
            report("LSR");
        } 
        inline void op_lsr(dword addr){
             if(emu||is_set(M)){
                byte data=mem.get_byte(addr);
                update_f(C,data&0x01);
                mem.set_byte(addr,data>>=1);
                update_nzB(data);
            }
            else{
                word data=mem.get_word(addr);
                update_f(C,data&0x0001);
                mem.set_word(addr,data>>=1);
                update_nzW(data);
            }
            report("LSR");
        }
        inline void op_rol_A(dword implied){ 
            bool old_C=is_set(C);
            if(emu||is_set(M)){
                update_f(C,a.b&0x80);
                a.b<<=1;
                if(old_C){
                    a.b|=0x01;
                }
                update_nzB(a.b);
            }
            else{
                update_f(C,a.w&0x8000);
                a.w<<=1;
                if(old_C){
                    a.w|=0x01;
                }
                update_nzW(a.w);
            }
            report("ROL");
        } 
        inline void op_rol(dword addr){ 
            bool old_C=is_set(C);
            if(emu||is_set(M)){
                byte data=mem.get_byte(addr);
                update_f(C,data&0x80);
                data<<=1;
                if(old_C){
                    data|=0x01;
                }
                update_nzB(data);
                mem.set_byte(addr,data);
            }
            else{
                word data=mem.get_word(addr);
                update_f(C,data&0x8000);
                data<<=1;
                if(old_C){
                    data|=0x01;
                }
                mem.set_word(addr,data);
                update_nzW(data);
            }
            report("ROL");
        } 
        inline void op_ror_A(dword implied){ 
            bool old_C=is_set(C);
            if(emu||is_set(M)){
                update_f(C,a.b&0x01);
                a.b>>=1;
                if(old_C){
                    a.b|=0x80;
                }
                update_nzB(a.b);
            }
            else{
                update_f(C,a.w&0x0001);
                a.w>>=1;
                if(old_C){
                    a.w|=0x8000;
                }
                update_nzW(a.w);
            }
            report("ROR");
        } 
        inline void op_ror(dword addr){ 
            bool old_C=is_set(C);
            if(emu||is_set(M)){
                byte data=mem.get_byte(addr);
                update_f(C,data&0x01);
                data>>=1;
                if(old_C){
                    data|=0x80;
                }
                mem.set_byte(addr,data);
                update_nzB(data);
            }
            else{
                word data=mem.get_word(addr);
                update_f(C,data&0x0001);
                data>>=1;
                if(old_C){
                    data|=0x8000;
                }
                update_nzW(data);
                mem.set_word(addr,data);
            }
            report("ROR");
        }
        inline void op_ora(dword addr){
             if(emu||is_set(M)){
                update_nzB(a.b|=mem.get_byte(addr));
             }
             else{
                update_nzW(a.w|=mem.get_word(addr));
             }
             report("ORA");
        } 
        inline void op_and(dword addr){
            if(emu||is_set(M)){
                update_nzB(a.b&=mem.get_byte(addr));
             }
             else{
                update_nzW(a.w&=mem.get_word(addr));
             }
             report("AND");
        }
        inline void op_eor(dword addr){
            if(emu||is_set(M)){
                update_nzB(a.b^=mem.get_byte(addr));
             }
             else{
                update_nzW(a.w^=mem.get_word(addr));
             }
             report("EOR");
        }
        inline void op_tax(dword implied){
            if(emu||is_set(X)){
                update_nzB(x.b=a.b); 
            }
            else{
                update_nzW(x.w=a.w);
            }
            report("TAX");
        }
        inline void op_tay(dword inmplied){
            if(emu||is_set(X)){
                update_nzB(y.b=a.b);
            }
            else{
                update_nzW(y.w=a.w);
            }
            report("TAY");
        }
        inline void op_txa(dword implied){
            if(emu||is_set(M)){
                update_nzB(a.b=x.b); 
            }
            else{
                update_nzW(a.w=x.w);
            }
            report("TXA");
        }
        inline void op_tya(dword inmplied){
            if(emu||is_set(M)){
                update_nzB(a.b=y.b);
            }
            else{
                update_nzW(a.w=y.w);
            }
            report("TYA");
        }
        inline void op_txy(dword inmplied){
            if(emu||is_set(X)){
                update_nzB(y.b=x.b);
            }
            else{
                update_nzW(y.w=x.w);
            }
            report("TXY");
        }
        inline void op_tyx(dword inmplied){
            if(emu||is_set(X)){
                update_nzB(x.b=y.b);
            }
            else{
                update_nzW(x.w=y.w);
            }
            report("TYX");
        }
        inline void op_tsx(dword inmplied){
            if(emu||is_set(X)){
                update_nzB(x.b=sp.b);
            }
            else{
                update_nzW(x.w=sp.w);
            }
            report("TSX");
        }
        inline void op_txs(dword implied){
            if(emu||is_set(X)){
                sp.w=0x0100|x.b;
            }
            else{
                sp.w=x.w;
            }
            sp_top=sp.w;
            report("TXS");
        }
        inline void op_tcd(dword implied){
            dp.w=a.w;
            report("TCD");
        }
        inline void op_tdc(dword implied){
            a.w=dp.w;
            if(emu||is_set(M)) update_nzB(a.b);
            else update_nzW(a.w);
            report("TDC");
        }
        inline void op_tcs(dword implied){
            sp_top=sp.w=emu?(0x0100|a.b):a.w;
            report("TCS");
        }
        inline void op_tsc(dword implied){
            a.w=sp.w;
            if(emu||is_set(M)) update_nzB(a.b);
            else update_nzW(a.w);
            report("TSC");
        }
        inline void op_xba(dword implied){
            a.w=join_bytes(higher_byte(a.w),lower_byte(a.w));
            update_nzB(a.w);
            report("XBA");
        }
        inline void op_mvp(dword addr){
            dbr=lower_byte((word)addr);
            byte src=higher_byte((word)addr);
            while(a.w<0xFFFF){  
                --a.w;
                mem.set_byte(join_addr(dbr,y.w--),mem.get_byte(join_addr(src,x.w--)));
            }
            report("MVP");
        }
        inline void op_mvn(dword addr){
            dbr=lower_byte((word)addr);
            byte src=higher_byte((word)addr);
            while(a.w<0xFFFF ){
                --a.w;
                mem.set_byte(join_addr(dbr,y.w++),mem.get_byte(join_addr(src,x.w++)));
            }
            report("MVN");
        }
        inline void op_bra(dword  addr){
            pc+=(int8_t)mem.get_byte(addr);
            report("BRA");
        }
        inline void op_brl(dword  addr){
            pc+=(int16_t)mem.get_word(addr);
            report("BRL");
        }
        inline void op_beq(dword  addr){
            if(is_set(Z)){
                pc+=(int8_t)mem.get_byte(addr);
                report("BEQ+",true);
            }
            else{
                report("BEQ-",true);
            } 
        }
        inline void op_bne(dword  addr){
            if(!is_set(Z)){
                pc+=(int8_t)mem.get_byte(addr); 
                report("BNE+",true);
            }
            else{
                report("BNE-",true);
            }
        }
        inline void op_bcc(dword  addr){
            if(!is_set(C)){
                pc+=(int8_t)mem.get_byte(addr);
                report("BCC+",true);
            }
            else{
                report("BCC-",true);
            }
        }
        inline void op_bcs(dword  addr){
            if(is_set(C)){
                pc+=(int8_t)mem.get_byte(addr);
                report("BCS+",true);
            }
            else{
                report("BCS-",true);
            }
        }
        inline void op_bmi(dword  addr){
            if(is_set(N)){
                pc+=(int8_t)mem.get_byte(addr);
                report("BMI+",true);               
            }
            else{
                report("BMI-",true);
            }
        }
        inline void op_bpl(dword  addr){
            if(!is_set(N)){
                pc+=(int8_t)mem.get_byte(addr);
                report("BPL+",true);                 
            }
            else{
                report("BPL-",true);
            }
        }
        inline void op_bvc(dword  addr){
            if(!is_set(V)){
                pc+=(int8_t)mem.get_byte(addr);
                report("BVC+",true);                 
            }
            else{
                report("BVC-",true);
            }
        }
        inline void op_bvs(dword  addr){
            if(is_set(V)){
                pc+=(int8_t)mem.get_byte(addr);
                report("BVS+",true);                
            }
            else{
                report("BVS-",true);
            }
        }
        inline void op_pha(dword implied){
            if(emu||is_set(M)){
                push_byte(a.b);
            }
            else{
                push_word(a.w);
            }
            report("PHA");
        }
        inline void op_phx(dword implied){
            if(emu||is_set(X)){
                push_byte(x.b);
            }
            else{
                push_word(x.w);
            }
            report("PHX");
        }
        inline void op_phy(dword implied){
            if(emu||is_set(X)){
                push_byte(y.b);
            }
            else{
                push_word(y.w);
            }
            report("PHY");
        }
        inline void op_php(dword implied){
                push_byte(p);
                report("PHP");
        }
        inline void op_phb(dword implied){
                push_byte(dbr);
                report("PHB");
        }
        inline void op_phd(dword implied){
                push_word(dp.w);
                report("PHD");
        }
        inline void op_phk(dword implied){
                push_byte(pbr);
                report("PHK");
        }
        inline void op_pla(dword implied){
                if(emu||is_set(M)){
                    a.b=pull_byte();
                    update_nzB(a.b);
                }
                else {
                    a.w=pull_word();
                    update_nzW(a.w);    
                }
                report("PLA");
        }
        inline void op_plx(dword implied){
                if(emu||is_set(X)){
                    x.b=pull_byte();
                    update_nzB(x.b);
                }
                else {
                    x.w=pull_word();
                    update_nzW(x.w);    
                }
                report("PLX");
        }
        inline void op_ply(dword implied){
                if(emu||is_set(X)){
                    y.b=pull_byte();
                    update_nzB(y.b);
                }
                else {
                    y.w=pull_word();
                    update_nzW(y.w);    
                }
                report("PLY");
        }
        inline void op_plp(dword implied){
            if(emu){
              p=pull_byte()|0x30;
            }
            else{
                p=pull_byte();
                if(is_set(X)){
                    x.w=x.b;
                    y.w=y.b;
                }
            }  
            report("PLP");
        }
        inline void op_plb(dword implied){
            update_nzB(dbr=pull_byte());
            report("PLB");
        }
        inline void op_pld(dword implied){
            update_nzW(dp.w=pull_word());
            report("PLD");
        } 
        inline void op_jmp(dword addr){
             pc=(word)addr;
             report("JMP");
        }
        inline void op_jml(dword addr){
            if(!emu){
                pbr=(addr>>16);
            }
            pc=(word)addr;
            report("JML");
        }
        inline void op_jsr(dword addr){
            push_word(pc-1);
            pc=(word)addr;
            report("JSR");
        }
        inline void op_jsl(dword addr){
            if(!emu){
                push_byte(pbr);
                pbr=addr>>16;
            }
            push_word(pc-1);
            pc=(word)addr;
            report("JSL");
        }
        inline void op_rts(dword implied){
            pc=pull_word()+1;
            report("RTS");
        }
        inline void op_rtl(dword implied){
            pc=pull_word()+1;
            if(!emu) pbr=pull_byte();
            report("RTL");
        }
        inline void op_rti(dword implied){
            p=pull_byte();
            pc=pull_word();
            if(!emu){pbr=pull_byte();}
            else{
                set_flag(X);
                set_flag(M);
            }
            report("RTI");
        }
        inline void op_pea(dword addr){
            push_word((word)addr);
            report("PEA");
        }
        inline void op_pei(dword addr){
            push_word((word)addr);
            report("PEI");
        }
        inline void op_per(dword addr){            
            push_word(pc+(int16_t)addr);
            report("PER");
        }
        inline void op_brk(dword implied) {
            if(!emu){
                push_byte(pbr);
                push_word(pc);
                push_byte(p);
                pc=mem.get_word(0xFFE6);
            } 
            else{
                push_word(pc);
                push_byte(p|0x10);
                pc=mem.get_word(0xFFFE); 
            }
            set_flag(I);
            clear_flag(D);
            pbr=0x00;
            report("BRK");
        }
        inline void op_nop(dword implied){
            report("NOP");
        }
        inline void op_cop(dword implied){
            if(!emu){
                push_byte(pbr);
                push_word(pc);
                push_byte(p);
                pc=mem.get_word(0xFFE4);
            } 
            else{
                push_word(pc);
                push_byte(p|0x10); 
                pc=mem.get_word(0xFFF4);
            }
            set_flag(I);
            clear_flag(D);
            pbr=0x00;
            report("COP");
        }
        inline void nmi_handler(){
             if(!emu){
                push_byte(pbr);
                push_word(pc);
                push_byte(p);
                pc=mem.get_word(0xFFEA);
            } 
            else{
                push_word(pc);
                push_byte(p&(~0x10)); 
                pc=mem.get_word(0xFFFA);
            }
            set_flag(I);
            clear_flag(D);
            pbr=0x00;
        }
        inline void irq_handler(){
            if(!emu){
                push_byte(pbr);
                push_word(pc);
                push_byte(p);
                pc=mem.get_word(0xFFEE);
            } 
            else{
                push_word(pc);
                push_byte(p&(~0x10)); 
                pc=mem.get_word(0xFFFE);
            }
            set_flag(I);
            clear_flag(D);
            pbr=0x00;
        }
        inline void op_wai(dword implied){
            waiting=true;
            report("WAI");
        }
        bool wdm_once=true;
        inline void op_wdm(dword addr){
            byte val=mem.get_byte(addr);
            report("WDM",false,true);
             
            //std::cout<<"[  ";
            if(val<0x03){
                if(!step&&wdm_once){
                std::cout<<"\nStdout/Stdin:\n";
                wdm_once=false;
                }
                if(step&&val!=0x02)std::cout<<"[Stdout] ";
                else if(step&&val==0x02) std::cout<<"[Stdin] ";    
                switch(val){
                    case 0x00:dispatch_char();break;
                    case 0x01:dispatch_str();break; 
                    case 0x02:dispatch_charR();break;
                }}
            else if(step&&(val>0x03 && val<=0xff)){ 
                switch(val){
                    case 0xfe:{
                        std::cout<<"[IRQ TRIGGER]";
                        trigger_irq();break;
                    }
                    case 0xff:{
                        std::cout<<"[NMI TRIGGER]";
                        trigger_nmi();break;
                    }
                    default:
                       printf("[Unknown WDM Service: 0x%02X]",val);break;
                }
            }
            //std::cout<<" ]\n";
            if(val!=0x02)std::cout<<"\n";
        }
        inline void dispatch_char(){
            putchar(mem.get_byte(mem.get_addr(0xffdd))); 
        }
        inline void dispatch_str(){
            dword addr=mem.get_addr(0xffdd);
            while(mem.used[addr]){
                byte data=mem.get_byte(addr++);
                if(!data) break; 
                putchar(data);
            }
        }
        inline void dispatch_charR(){
            char c;
            std::cin.get(c);
            mem.set_byte(0xffdc,c);
        }
                           
};
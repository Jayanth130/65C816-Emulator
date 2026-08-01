#include"CPU.hpp" 
#include<string.h>
#include<chrono> 
using namespace std; 

CPU :: CPU(Memory&mem,bool decimal,bool step) :mem(mem),decimal(decimal),step(step){
    init_tables();
}

void CPU::reset(){
    pc=mem.get_word(0xfffc);
    pbr=0x00;
    dbr=0x00;
    dp.w=0x0000;
    sp_top=sp.w=0x01ff;
    p=0x34;
    emu=1;
    running=true;
    a.w=x.w=y.w=0;  
    ins_count=0;
}

void CPU::init_tables(){
    //ADC
    op_entry[0x69]={&CPU::op_adc,&CPU::imm_M};
    op_entry[0x6D]={&CPU::op_adc,&CPU::abs_addr};
    op_entry[0x7D]={&CPU::op_adc,&CPU::abs_addrX};
    op_entry[0x79]={&CPU::op_adc,&CPU::abs_addrY};
    op_entry[0x6F]={&CPU::op_adc,&CPU::abs_addrL};
    op_entry[0x7F]={&CPU::op_adc,&CPU::abs_addrLX};
    op_entry[0x65]={&CPU::op_adc,&CPU::dp_addr};
    op_entry[0x75]={&CPU::op_adc,&CPU::dp_addrX};
    op_entry[0x72]={&CPU::op_adc,&CPU::dp_ind};
    op_entry[0x61]={&CPU::op_adc,&CPU::dp_indX};
    op_entry[0x71]={&CPU::op_adc,&CPU::dp_indY};
    op_entry[0x67]={&CPU::op_adc,&CPU::dp_indL};
    op_entry[0x77]={&CPU::op_adc,&CPU::dp_indLY};
    op_entry[0x63]={&CPU::op_adc,&CPU::dp_sp};
    op_entry[0x73]={&CPU::op_adc,&CPU::dp_spY};

    //SBC
    op_entry[0xe9]={&CPU::op_sbc,&CPU::imm_M};
    op_entry[0xeD]={&CPU::op_sbc,&CPU::abs_addr};
    op_entry[0xfD]={&CPU::op_sbc,&CPU::abs_addrX};
    op_entry[0xf9]={&CPU::op_sbc,&CPU::abs_addrY};
    op_entry[0xeF]={&CPU::op_sbc,&CPU::abs_addrL};
    op_entry[0xfF]={&CPU::op_sbc,&CPU::abs_addrLX};
    op_entry[0xe5]={&CPU::op_sbc,&CPU::dp_addr};
    op_entry[0xf5]={&CPU::op_sbc,&CPU::dp_addrX};
    op_entry[0xf2]={&CPU::op_sbc,&CPU::dp_ind};
    op_entry[0xe1]={&CPU::op_sbc,&CPU::dp_indX};
    op_entry[0xf1]={&CPU::op_sbc,&CPU::dp_indY};
    op_entry[0xe7]={&CPU::op_sbc,&CPU::dp_indL};
    op_entry[0xf7]={&CPU::op_sbc,&CPU::dp_indLY};
    op_entry[0xe3]={&CPU::op_sbc,&CPU::dp_sp};
    op_entry[0xf3]={&CPU::op_sbc,&CPU::dp_spY};

    //REP/SEP
    op_entry[0xC2]={&CPU::op_rep,&CPU::imm_b};
    op_entry[0xE2]={&CPU::op_sep,&CPU::imm_b};

    //XCE
    op_entry[0xFB]={&CPU::op_xce,&CPU::implied};

    //STP
    op_entry[0xDB]={&CPU::op_stp,&CPU::implied};

    //LDA
    op_entry[0xA9]={&CPU::op_lda,&CPU::imm_M};
    op_entry[0xAD]={&CPU::op_lda,&CPU::abs_addr};
    op_entry[0xBD]={&CPU::op_lda,&CPU::abs_addrX};
    op_entry[0xB9]={&CPU::op_lda,&CPU::abs_addrY};
    op_entry[0xAF]={&CPU::op_lda,&CPU::abs_addrL};
    op_entry[0xBF]={&CPU::op_lda,&CPU::abs_addrLX};
    op_entry[0xA5]={&CPU::op_lda,&CPU::dp_addr};
    op_entry[0xB5]={&CPU::op_lda,&CPU::dp_addrX};
    op_entry[0xB2]={&CPU::op_lda,&CPU::dp_ind};
    op_entry[0xA1]={&CPU::op_lda,&CPU::dp_indX};
    op_entry[0xB1]={&CPU::op_lda,&CPU::dp_indY};
    op_entry[0xA7]={&CPU::op_lda,&CPU::dp_indL};
    op_entry[0xB7]={&CPU::op_lda,&CPU::dp_indLY};
    op_entry[0xA3]={&CPU::op_lda,&CPU::dp_sp};
    op_entry[0xB3]={&CPU::op_lda,&CPU::dp_spY};

    //LDX
    op_entry[0xA2]={&CPU::op_ldx,&CPU::imm_X};
    op_entry[0xAE]={&CPU::op_ldx,&CPU::abs_addr};
    op_entry[0xBE]={&CPU::op_ldx,&CPU::abs_addrY};
    op_entry[0xA6]={&CPU::op_ldx,&CPU::dp_addr};
    op_entry[0xB6]={&CPU::op_ldx,&CPU::dp_addrY};

    //LDY
    op_entry[0xA0]={&CPU::op_ldy,&CPU::imm_X};
    op_entry[0xAC]={&CPU::op_ldy,&CPU::abs_addr};
    op_entry[0xBC]={&CPU::op_ldy,&CPU::abs_addrX};
    op_entry[0xA4]={&CPU::op_ldy,&CPU::dp_addr};
    op_entry[0xB4]={&CPU::op_ldy,&CPU::dp_addrX};

    //STA
    op_entry[0x8D]={&CPU::op_sta,&CPU::abs_addr};
    op_entry[0x9D]={&CPU::op_sta,&CPU::abs_addrX};
    op_entry[0x99]={&CPU::op_sta,&CPU::abs_addrY};
    op_entry[0x8F]={&CPU::op_sta,&CPU::abs_addrL};
    op_entry[0x9F]={&CPU::op_sta,&CPU::abs_addrLX};
    op_entry[0x85]={&CPU::op_sta,&CPU::dp_addr};
    op_entry[0x95]={&CPU::op_sta,&CPU::dp_addrX};
    op_entry[0x92]={&CPU::op_sta,&CPU::dp_ind};
    op_entry[0x81]={&CPU::op_sta,&CPU::dp_indX};
    op_entry[0x91]={&CPU::op_sta,&CPU::dp_indY};
    op_entry[0x87]={&CPU::op_sta,&CPU::dp_indL};
    op_entry[0x97]={&CPU::op_sta,&CPU::dp_indLY};
    op_entry[0x83]={&CPU::op_sta,&CPU::dp_sp};
    op_entry[0x93]={&CPU::op_sta,&CPU::dp_spY};
    op_entry[0x64]={&CPU::op_stz,&CPU::dp_addr};
    op_entry[0x74]={&CPU::op_stz,&CPU::dp_addrX};
    op_entry[0x9C]={&CPU::op_stz,&CPU::abs_addr};
    op_entry[0x9E]={&CPU::op_stz,&CPU::abs_addrX};

    //STX
    op_entry[0x8E]={&CPU::op_stx,&CPU::abs_addr};
    op_entry[0x86]={&CPU::op_stx,&CPU::dp_addr};
    op_entry[0x96]={&CPU::op_stx,&CPU::dp_addrY};

    //STY
    op_entry[0x8C]={&CPU::op_sty,&CPU::abs_addr};
    op_entry[0x84]={&CPU::op_sty,&CPU::dp_addr};
    op_entry[0x94]={&CPU::op_sty,&CPU::dp_addrX};

    //Flag Instructions
    op_entry[0x18]={&CPU::op_clc,&CPU::implied};
    op_entry[0x38]={&CPU::op_sec,&CPU::implied};
    op_entry[0x58]={&CPU::op_cli,&CPU::implied};
    op_entry[0x78]={&CPU::op_sei,&CPU::implied};
    op_entry[0xB8]={&CPU::op_clv,&CPU::implied};
    op_entry[0xD8]={&CPU::op_cld,&CPU::implied};
    op_entry[0xF8]={&CPU::op_sed,&CPU::implied};
    
    //CMP
    op_entry[0xC9]={&CPU::op_cmp,&CPU::imm_M};
    op_entry[0xCD]={&CPU::op_cmp,&CPU::abs_addr};
    op_entry[0xDD]={&CPU::op_cmp,&CPU::abs_addrX};
    op_entry[0xD9]={&CPU::op_cmp,&CPU::abs_addrY};
    op_entry[0xCF]={&CPU::op_cmp,&CPU::abs_addrL};
    op_entry[0xDF]={&CPU::op_cmp,&CPU::abs_addrLX};
    op_entry[0xC5]={&CPU::op_cmp,&CPU::dp_addr};
    op_entry[0xD5]={&CPU::op_cmp,&CPU::dp_addrX};
    op_entry[0xD2]={&CPU::op_cmp,&CPU::dp_ind};
    op_entry[0xC1]={&CPU::op_cmp,&CPU::dp_indX};
    op_entry[0xD1]={&CPU::op_cmp,&CPU::dp_indY};
    op_entry[0xC7]={&CPU::op_cmp,&CPU::dp_indL};
    op_entry[0xD7]={&CPU::op_cmp,&CPU::dp_indLY};
    op_entry[0xC3]={&CPU::op_cmp,&CPU::dp_sp};
    op_entry[0xD3]={&CPU::op_cmp,&CPU::dp_spY};

    //CPX
    op_entry[0xE0]={&CPU::op_cpx,&CPU::imm_X};
    op_entry[0xE4]={&CPU::op_cpx,&CPU::dp_addr};
    op_entry[0xEC]={&CPU::op_cpx,&CPU::abs_addr};

    //CPY
    op_entry[0xC0]={&CPU::op_cpy,&CPU::imm_X};
    op_entry[0xC4]={&CPU::op_cpy,&CPU::dp_addr};
    op_entry[0xCC]={&CPU::op_cpy,&CPU::abs_addr};

    //INC/DEC
    op_entry[0xE6]={&CPU::op_inc,&CPU::dp_addr};
    op_entry[0xF6]={&CPU::op_inc,&CPU::dp_addrX};
    op_entry[0xEE]={&CPU::op_inc,&CPU::abs_addr};
    op_entry[0xFE]={&CPU::op_inc,&CPU::abs_addrX};
    op_entry[0xC6]={&CPU::op_dec,&CPU::dp_addr};
    op_entry[0xD6]={&CPU::op_dec,&CPU::dp_addrX};
    op_entry[0xCE]={&CPU::op_dec,&CPU::abs_addr};
    op_entry[0xDE]={&CPU::op_dec,&CPU::abs_addrX};
    op_entry[0x1A]={&CPU::op_ina,&CPU::implied};
    op_entry[0x3A]={&CPU::op_dea,&CPU::implied};
    op_entry[0xE8]={&CPU::op_inx,&CPU::implied};
    op_entry[0xCA]={&CPU::op_dex,&CPU::implied};
    op_entry[0xC8]={&CPU::op_iny,&CPU::implied};
    op_entry[0x88]={&CPU::op_dey,&CPU::implied};

    //BIT
    op_entry[0x89]={&CPU::op_bit,&CPU::imm_M};
    op_entry[0x24]={&CPU::op_bit,&CPU::dp_addr};
    op_entry[0x34]={&CPU::op_bit,&CPU::dp_addrX};
    op_entry[0x2C]={&CPU::op_bit,&CPU::abs_addr};
    op_entry[0x3C]={&CPU::op_bit,&CPU::abs_addrX};

    //TSB/TRB
    op_entry[0x04]={&CPU::op_tsb,&CPU::dp_addr};
    op_entry[0x0C]={&CPU::op_tsb,&CPU::abs_addr};
    op_entry[0x14]={&CPU::op_trb,&CPU::dp_addr};
    op_entry[0x1C]={&CPU::op_trb,&CPU::abs_addr};

    //ASL A/ASL
    op_entry[0x0A]={&CPU::op_aslA,&CPU::implied};
    op_entry[0x06]={&CPU::op_asl,&CPU::dp_addr};
    op_entry[0x16]={&CPU::op_asl,&CPU::dp_addrX};
    op_entry[0x0E]={&CPU::op_asl,&CPU::abs_addr};
    op_entry[0x1E]={&CPU::op_asl,&CPU::abs_addrX};

    //LSR A/LSR
    op_entry[0x4A]={&CPU::op_lsrA,&CPU::implied};
    op_entry[0x46]={&CPU::op_lsr,&CPU::dp_addr};
    op_entry[0x56]={&CPU::op_lsr,&CPU::dp_addrX};
    op_entry[0x4E]={&CPU::op_lsr,&CPU::abs_addr};
    op_entry[0x5E]={&CPU::op_lsr,&CPU::abs_addrX};

    //ROL A /ROL
    op_entry[0x2A]={&CPU::op_rol_A,&CPU::implied};
    op_entry[0x26]={&CPU::op_rol,&CPU::dp_addr};
    op_entry[0x36]={&CPU::op_rol,&CPU::dp_addrX};
    op_entry[0x2E]={&CPU::op_rol,&CPU::abs_addr};
    op_entry[0x3E]={&CPU::op_rol,&CPU::abs_addrX};

    //ROR A/ROR
    op_entry[0x6A]={&CPU::op_ror_A,&CPU::implied};
    op_entry[0x66]={&CPU::op_ror,&CPU::dp_addr};
    op_entry[0x76]={&CPU::op_ror,&CPU::dp_addrX};
    op_entry[0x6E]={&CPU::op_ror,&CPU::abs_addr};
    op_entry[0x7E]={&CPU::op_ror,&CPU::abs_addrX};

    //ORA
    op_entry[0x09]={&CPU::op_ora,&CPU::imm_M};
    op_entry[0x0D]={&CPU::op_ora,&CPU::abs_addr};
    op_entry[0x1D]={&CPU::op_ora,&CPU::abs_addrX};
    op_entry[0x19]={&CPU::op_ora,&CPU::abs_addrY};
    op_entry[0x0F]={&CPU::op_ora,&CPU::abs_addrL};
    op_entry[0x1F]={&CPU::op_ora,&CPU::abs_addrLX};
    op_entry[0x05]={&CPU::op_ora,&CPU::dp_addr};
    op_entry[0x15]={&CPU::op_ora,&CPU::dp_addrX};
    op_entry[0x12]={&CPU::op_ora,&CPU::dp_ind};
    op_entry[0x01]={&CPU::op_ora,&CPU::dp_indX};
    op_entry[0x11]={&CPU::op_ora,&CPU::dp_indY};
    op_entry[0x07]={&CPU::op_ora,&CPU::dp_indL};
    op_entry[0x17]={&CPU::op_ora,&CPU::dp_indLY};
    op_entry[0x03]={&CPU::op_ora,&CPU::dp_sp};
    op_entry[0x13]={&CPU::op_ora,&CPU::dp_spY};

    //AND
    op_entry[0x29]={&CPU::op_and,&CPU::imm_M};
    op_entry[0x2D]={&CPU::op_and,&CPU::abs_addr};
    op_entry[0x3D]={&CPU::op_and,&CPU::abs_addrX};
    op_entry[0x39]={&CPU::op_and,&CPU::abs_addrY};
    op_entry[0x2F]={&CPU::op_and,&CPU::abs_addrL};
    op_entry[0x3F]={&CPU::op_and,&CPU::abs_addrLX};
    op_entry[0x25]={&CPU::op_and,&CPU::dp_addr};
    op_entry[0x35]={&CPU::op_and,&CPU::dp_addrX};
    op_entry[0x32]={&CPU::op_and,&CPU::dp_ind};
    op_entry[0x21]={&CPU::op_and,&CPU::dp_indX};
    op_entry[0x31]={&CPU::op_and,&CPU::dp_indY};
    op_entry[0x27]={&CPU::op_and,&CPU::dp_indL};
    op_entry[0x37]={&CPU::op_and,&CPU::dp_indLY};
    op_entry[0x23]={&CPU::op_and,&CPU::dp_sp};
    op_entry[0x33]={&CPU::op_and,&CPU::dp_spY};

    //EOR
    op_entry[0x49]={&CPU::op_eor,&CPU::imm_M};
    op_entry[0x4D]={&CPU::op_eor,&CPU::abs_addr};
    op_entry[0x5D]={&CPU::op_eor,&CPU::abs_addrX};
    op_entry[0x59]={&CPU::op_eor,&CPU::abs_addrY};
    op_entry[0x4F]={&CPU::op_eor,&CPU::abs_addrL};
    op_entry[0x5F]={&CPU::op_eor,&CPU::abs_addrLX};
    op_entry[0x45]={&CPU::op_eor,&CPU::dp_addr};
    op_entry[0x55]={&CPU::op_eor,&CPU::dp_addrX};
    op_entry[0x52]={&CPU::op_eor,&CPU::dp_ind};
    op_entry[0x41]={&CPU::op_eor,&CPU::dp_indX};
    op_entry[0x51]={&CPU::op_eor,&CPU::dp_indY};
    op_entry[0x47]={&CPU::op_eor,&CPU::dp_indL};
    op_entry[0x57]={&CPU::op_eor,&CPU::dp_indLY};
    op_entry[0x43]={&CPU::op_eor,&CPU::dp_sp};
    op_entry[0x53]={&CPU::op_eor,&CPU::dp_spY};

    //Transfer
    op_entry[0xAA]={&CPU::op_tax,&CPU::implied};
    op_entry[0xA8]={&CPU::op_tay,&CPU::implied};
    op_entry[0x8A]={&CPU::op_txa,&CPU::implied};
    op_entry[0x98]={&CPU::op_tya,&CPU::implied};
    op_entry[0x9B]={&CPU::op_txy,&CPU::implied};
    op_entry[0xBB]={&CPU::op_tyx,&CPU::implied};
    op_entry[0xBA]={&CPU::op_tsx,&CPU::implied};
    op_entry[0x9A]={&CPU::op_txs,&CPU::implied};
    op_entry[0x5B]={&CPU::op_tcd,&CPU::implied};
    op_entry[0x7B]={&CPU::op_tdc,&CPU::implied};
    op_entry[0x1B]={&CPU::op_tcs,&CPU::implied};
    op_entry[0x3B]={&CPU::op_tsc,&CPU::implied};
    op_entry[0xEB]={&CPU::op_xba,&CPU::implied};

    //MVN/MVP
    op_entry[0x54]={&CPU::op_mvn,&CPU::abs_addr};
    op_entry[0x44]={&CPU::op_mvp,&CPU::abs_addr};
    
    //BRANCH
    op_entry[0x80]={&CPU::op_bra,&CPU::imm_b};
    op_entry[0x82]={&CPU::op_brl,&CPU::imm_w};
    op_entry[0xF0]={&CPU::op_beq,&CPU::imm_b};
    op_entry[0xD0]={&CPU::op_bne,&CPU::imm_b};
    op_entry[0x90]={&CPU::op_bcc,&CPU::imm_b};
    op_entry[0xB0]={&CPU::op_bcs,&CPU::imm_b};
    op_entry[0x30]={&CPU::op_bmi,&CPU::imm_b};
    op_entry[0x10]={&CPU::op_bpl,&CPU::imm_b};
    op_entry[0x50]={&CPU::op_bvc,&CPU::imm_b};
    op_entry[0x70]={&CPU::op_bvs,&CPU::imm_b};

    //PUSH / PULL
    op_entry[0x48]={&CPU::op_pha,&CPU::implied};
    op_entry[0xDA]={&CPU::op_phx,&CPU::implied};
    op_entry[0x5A]={&CPU::op_phy,&CPU::implied};
    op_entry[0x08]={&CPU::op_php,&CPU::implied};
    op_entry[0x8B]={&CPU::op_phb,&CPU::implied};
    op_entry[0x0B]={&CPU::op_phd,&CPU::implied};
    op_entry[0x4B]={&CPU::op_phk,&CPU::implied};
    op_entry[0x68]={&CPU::op_pla,&CPU::implied};
    op_entry[0xFA]={&CPU::op_plx,&CPU::implied};
    op_entry[0x7A]={&CPU::op_ply,&CPU::implied};
    op_entry[0x28]={&CPU::op_plp,&CPU::implied};
    op_entry[0xAB]={&CPU::op_plb,&CPU::implied};
    op_entry[0x2B]={&CPU::op_pld,&CPU::implied};
    
    //SUBROUTINES AND JMP
    op_entry[0x4C]={&CPU::op_jmp,&CPU::abs_addr};
    op_entry[0x6C]={&CPU::op_jmp,&CPU::abs_ind};
    op_entry[0x7C]={&CPU::op_jmp,&CPU::abs_indX};
    op_entry[0x5C]={&CPU::op_jml,&CPU::abs_addrL};
    op_entry[0xDC]={&CPU::op_jml,&CPU::abs_indL};
    op_entry[0x20]={&CPU::op_jsr,&CPU::abs_addr};
    op_entry[0xFC]={&CPU::op_jsr,&CPU::abs_indX};
    op_entry[0x22]={&CPU::op_jsl,&CPU::abs_addrL};
    op_entry[0x60]={&CPU::op_rts,&CPU::implied};
    op_entry[0x6B]={&CPU::op_rtl,&CPU::implied};
    op_entry[0x40]={&CPU::op_rti,&CPU::implied};
    
    op_entry[0xf4]={&CPU::op_pea,&CPU::abs_addr};
    op_entry[0xd4]={&CPU::op_pei,&CPU::dp_ind};
    op_entry[0x62]={&CPU::op_per,&CPU::abs_addr};

    op_entry[0x00]={&CPU::op_brk,&CPU::imm_b};
    op_entry[0x42]={&CPU::op_wdm,&CPU::imm_b};
    op_entry[0xea]={&CPU::op_nop,&CPU::implied}; 
    op_entry[0x02]={&CPU::op_cop,&CPU::imm_b};
    op_entry[0xcb]={&CPU::op_wai,&CPU::implied};
}
void CPU::execute(){
    if(nmi_pending){
        nmi_pending=false;
        nmi_handler();
    }
    else if(irq_pending&&!is_set(I)){
        irq_pending=false;
        irq_handler();
    }
    if(!mem.used[join_addr(pbr,pc)]){
        running=false;
        step=true;
        printf("Execution stopped: PC out of valid memory range PC:(%02X:%04X)\n",pbr,pc);
        return; 
    }
    if(step){ 
     printf("%02X:",pbr);
     printf("%04X ",pc);
     
     //printf("%02X ",mem.get_byte(join_addr(pbr,pc)));

    }
   
    Instruction entry=op_entry[mem.get_byte(join_addr(pbr,pc++))];
    /*word old_pc=pc;
    while(old_pc<pc){
        printf(" %02X ",mem.get_byte(join_addr(pbr,old_pc++)));
    }*/
    (this->*entry.operation)(op_addr=(this->*entry.mode)());
    ins_count++; 
    

}


void CPU::run(bool m,int bytes,dword addr){
   cout<<"Excecution starting ...\n";
   if(step) cout<<"\n";
   auto start=chrono::high_resolution_clock::now();
   while(running){
        if(waiting){
            if(irq_pending||nmi_pending) waiting=false;
            continue;
        }
        execute();   
    }
   if(!step){
    cout<<"\n";
    cout<<"Registers:\n";
    printf("A=");print_val(((emu||(p&M))?a.b:a.w));cout<<endl;
    printf("X=");print_val(((emu||(p&X))?x.b:x.w ));cout<<endl;
    printf("Y=");print_val(((emu||(p&X))?y.b:y.w ));cout<<endl;
    printf("DP=%04X ",dp.w);cout<<endl;
    printf("SP=%04X ",sp.w); cout<<endl;
    printf("DBR=%02X ",dbr);cout<<endl;
    printf("PBR=%02X",pbr);cout<<endl;
    printf("PC=%04X",pc);cout<<endl<<endl;
    
    cout<<"Status Registers:\n";
    cout<<("N=")<<((p&N)?1:0)<<(" V=")<<(p&V?1:0)<<(" M=")<<(p&M?1:0)
    <<(" X=")<<(p&X?1:0)<<endl;
    cout<<("D=")<<(p&D?1:0)<<(" I=")<<(p&I?1:0)<<(" Z=")<<(p&Z?1:0)
    <<(" C=")<<(p&C?1:0)<<endl<<endl;
    cout<<"Stack View:\n"; 
    word cur=sp.w+4;
    for(int i=1;i<=4;i++){
        if(mem.used[cur]){
            printf("00:%04X  ",cur);
            printf("%02X\n",mem.get_byte(join_addr(0,cur)));
        }else{
            printf("00:%04X  ??\n",cur);
        }
        cur--;   
    }
    if(m){
        cout<<endl;
        int count=0;
        printf("Memory View:\n");
        for(int i=0;i<bytes;i++){
            if(count==0){
               printf("%02X:%04X  ",((addr+i)>>16)&0xFF,(addr+i)&0xFFFF);
            }

            if(mem.used[addr+i]){
              util::byte val=mem.get_byte(addr+i);
              if(decimal) printf("%03d ",val);  
              else printf("%02X ",val);
            }
            else {  
                if(decimal) printf("??? ");
                else printf("?? ");
            }
            count++;
            if(count==16){
                count=0;
                cout<<endl;
            }
        }
        if(count!=16) cout<<endl;    
    }

  }
  if(mem.uart){
        cout<<"\nUART Terminal:\n";
        for(char c:mem.uart_buffer){
            if(c=='\n') cout<<"\\n";
            else if(c=='\t')cout<<"\\t";
            else if(c=='\0')cout<<"\\0";
            else cout<<c;
        }
        cout<<endl;
   }
   cout<<"\n";
   auto end=chrono::high_resolution_clock::now();
   cout<<"Execution ending ,\n";
   cout<<"Instruction Count:"<<ins_count<<endl;
   auto et=std::chrono::duration<double>(end-start);
   cout<<"Elapsed Time:"<<et.count()<<endl;
   cout<<"Instruction per second:"<<(int)(ins_count/et.count())<<endl;
}       

void CPU::trace(const char*str,bool no_space,bool sft){  
    cout<<str;
    if(no_space) cout<<" ";
    else cout<<"  ";
    cout<<"[";
    printf("%02X:",op_addr>>16);
    printf("%04X",op_addr);
    cout<<"] ";

    if(!sft){
    cout<<"E="<<(emu?1:0)<<" "<<"P=[";
    cout<<(p&N?'N':'-')<<
          (p&V?'V':'-')<<
          (p&M?'M':'-')<<
          (p&X?'X':'-')<<
          (p&D?'D':'-')<<
          (p&I?'I':'-')<<
          (p&Z?'Z':'-')<<
          (p&C?'C':'-')<<']'<<' ';      
    printf(" A=");print_val(((emu||(p&M))?a.b:a.w ));
    printf(" X=");print_val(((emu||(p&X))?x.b:x.w ));
    printf(" Y=");print_val(((emu||(p&X))?y.b:y.w ));
    printf(" DP=%04X ",dp.w);
    printf("SP=%04X ",sp.w);
    cout<<" { ";
    word cur=sp.w+4;
    for(int i=1;i<=4;i++){
        if(mem.used[cur]){
            printf("%02X ",mem.get_byte(join_addr(0,cur)));
        }else{
            printf("?? ");  
        }   
        cur--;
    }
    cout<<"} ";
    printf("DBR=%02X ",dbr);
    printf("PBR=%02X\n",pbr); 
  } 
}
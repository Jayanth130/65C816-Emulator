#include"CPU.hpp"
#include<string.h>
#include<string>
#include<fstream>   
using namespace std; 
constexpr dword RAM_SIZE=256*65536;
constexpr dword MEM_MASK=RAM_SIZE-1;

inline util::byte hex_to_dl(char c){
   if(c>='0'&&c<='9') return c-'0'; 
   if(c>='A'&&c<='F') return c-'A'+10;
   if(c>='a'&&c<='f') return c-'a'+10;
   return 0 ;
}
inline util::byte read_byte(string&s,int&pos){
    return (hex_to_dl(s[pos++])<<4|hex_to_dl(s[pos++]));
}
inline util::word read_word(string&s, int&pos){
    return (((word)read_byte(s,pos)<<8)|read_byte(s,pos));
}
inline util::dword read_addr(string&s,int&pos){
    return ((dword)read_byte(s,pos)<<16|read_word(s,pos));
}

//To read main address from Calypsi generated .lst file 
void kernel_addr(string s,Memory&mem){
    ifstream file(s);
    if(!file){
        cerr<<"Unable to open"<<s<<'\n';
        return;
    }
    string line;
    while(getline(file,line)){
        size_t pos=line.find("main = ");
        if(pos!=string::npos) {
            string addr=line.substr(pos+7,6);
            mem.set_word(0x00ffd8,stoul(addr,nullptr,16));
            break;
        }
    }
}


void load(string&s,Memory&mem){
    ifstream file(s);
    if(!file){
       cerr<<"Unable to open"<<s<<'\n';
       return;
    }
    string line;
    while(file>>line){
        if(line[0] =='S'){
            if(line[1]=='1'){
                int pos=2;
                util::byte count=read_byte(line,pos);
                util::word addr=read_word(line,pos);
                count-=3;
                while(count--){
                     mem.set_byte((dword)addr++,read_byte(line,pos));
                }
            }
            else if(line[1]=='2'){
                int pos=2;
                util::byte count=read_byte(line,pos);
                util::dword addr=read_addr(line,pos);
                count-=4;
                while(count--){
                     mem.set_byte(addr++,read_byte(line,pos));
                }
            } 
        }
    }
}

int main(int argc ,char**argv){
    bool decimal=false; 
    bool step=false;
    int bytes=0;
    bool m=false;
    util::dword addr=0xffffff; 
    int i=1;
    string file_name; 
    Memory mem(RAM_SIZE,MEM_MASK); 
    while(i<argc){
      if(argv[i][0]=='-'){
        if(!strcmp(argv[i],"-d")){
            decimal=true;
        }
        else if(!strcmp(argv[i],"-s")){
            step=true;
        }
        else if(!strcmp(argv[i],"-m")){
           if(i+2>=argc||argv[i+1][0]=='-'){
             cerr<<"-m requires <bytes> <address>\n";
             return 1;
           }    
           m=true;
           bytes=stoi(argv[++i]); 
           addr=stoul((argv[++i]),nullptr,16);
        } 
        else{
            cerr<<"Unknown opiton "<<argv[i]<<endl;
            return 1;
        }
    }
    else{   
        if(!file_name.empty()){
            cerr<<"Mulitple input files are specified\n" ;
            return 1 ; 
        }
        file_name=argv[i];
        if((file_name.size()>4 && file_name.substr(file_name.size()-4,4)!=".s28")||(file_name.size()<5)){
            cerr<<"Invalid format";
            return 1;
        }
        load(file_name,mem);
     }   
     i++;     
   }
   if(file_name.empty()){
    cerr<<"No input file specified.\n";
    return 1;
   }
   cout<<"Loading S28 file "<<file_name<<endl;
   CPU cpu(mem,decimal,step);
   cpu.reset();
   kernel_addr("kernel.lst",mem);
   cpu.run(m,bytes,addr);
}
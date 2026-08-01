#define UART_Data (*(volatile unsigned char*)0xFFD6)
#define UART_Status (*(volatile unsigned char*)0xFFD7)
#include<stdarg.h>

void uart_terminal(char c){
    UART_Data=c;
}
void padding(unsigned int len,unsigned int width,char pad){
    while(width>len){
        uart_terminal(pad);
        width--;
    }
}
void put_char(char c,unsigned int width,char pad){
    if(width>0) padding(1,width,pad);
    uart_terminal(c);
}
int str_len(const char*s){
    int i=0;
    for(;s[i]!='\0';i++);
    return i;
}

void put_str(const char*s,unsigned int width,char pad){
    if(width>0){
        padding(str_len(s),width,pad);
    }
    for(int i=0;s[i]!='\0';i++){
        uart_terminal(s[i]);
    }
}
void print_hex(unsigned int val,unsigned int width,unsigned char pad){
    char buffer[9];
    buffer[8]='\0';
    int i=8;
    do{
        buffer[--i]="0123456789ABCDEF"[val&0xF];
        val>>=4;
    }while(val);
    if(width>0) padding(8-i,width,pad);
    put_str(&(buffer[i]),0,' ');
} 
void print_int(long val,unsigned int width,unsigned char pad){
    char buffer[11];
    buffer[10]='\0';
    int i=10;
    if(val<0){
        uart_terminal('-');
        val=-val;
    }
    do{
        buffer[--i]=val%10+'0';
        val/=10;
    }while(val);
    if(width>0) padding(10-i,width,pad);
    put_str(&buffer[i],0,' ');
}
void printf(char*s,...){
    va_list args;
    va_start(args,s);
    for(int i=0;s[i]!='\0';i++){
        if(s[i]!='%') uart_terminal(s[i]);
        else{
            int width=0;
            char pad=' ';
            int j=i+1;
            if(s[j]=='0'){
                pad='0';j++;
            }
            while(s[j]>='1'&&s[j]<='9'){
                width=width*10+(s[j]-'0');
                j++;
            }
            switch(s[j]){
                case 'd': print_int(va_arg(args,int),width,pad); break;
                case 'c':put_char(va_arg(args,int),width,pad);break;
                case 's':put_str(va_arg(args,char*),width,pad);break;
                case 'X':print_hex(va_arg(args,unsigned int),width,pad);break;
                case '%':uart_terminal('%');break;
                default : printf("Invalid format specifier '%c'",s[j]);return;
            }
            i=j;
        }
    }
    va_end(args);
}
static unsigned char heap[4096];
static unsigned int next = 0;
#define NULL 0
void* malloc(unsigned int size){
    void*p=&heap[next];
    next+=size;
    return p;
}
struct test{
    int val;
    struct test*next;
};


struct test* create_list(int data){
    struct test*a=(struct test*)malloc(sizeof(struct test));
    a->val=data;
    a->next=NULL;
    return a;
}
int main(){
     
    //put_char(10);
    //print_hex(10,' ',255);
    //print_int(-123,5,'0');
    //print_int(-32768, 0, ' ');
    //int a=255;
    //printf("%06X",a);
    struct test *head=create_list(10);
    head->next=create_list(20);
    struct test*temp=head;
    //printf("%X",(unsigned int)head);
    while(temp){
        printf("%d ",temp->val);
        temp=temp->next;
    }
    return 0;      
}
    

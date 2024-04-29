#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <time.h> 
#include <unistd.h>

#include "xboxcont.h"

#define SLEEP 1

int eraseDouble=1;

extern libusb_device **devs;
extern libusb_context *ctx;
extern libusb_device_handle *handle;


void processDirection(unsigned char data,int &a,int &b,GMKDisplay& display) {

    if (data == 0x00) {
        printf("data[2] is '00'\n");
    } else {
            if (eraseDouble==1){
                eraseDouble=0;
            }else{
                eraseDouble=1;
                return;
            }
       
            switch (data) {
                case 0x01:  // up
                    printf("got '01'\n");
                    b = b-1;
                    printf("The value of the coordinate is: %d %d\n", a,b); 

                    display.update_select(x-1,y-1);
                    
                    sleep(SLEEP);   // sleep 1s
                    break;
                case 0x08:  // right
                    printf("got '08'\n");
                    a = a+1;
                    printf("The value of the coordinate is: %d %d\n", a,b); 

                    display.update_select(x-1,y-1);

                    sleep(SLEEP);   // sleep 1s
                    break;
                case 0x02:  // down
                    printf("got '02'\n");
                    b = b+1;
                    printf("The value of the coordinate is: %d %d\n", a,b);

                    display.update_select(x-1,y-1);

                    sleep(SLEEP);   // sleep 1s
                    break;
                case 0x04:  // left
                    printf("got '04'\n");
                    a = a-1;
                    printf("The value of the coordinate is: %d %d\n", a,b);

                    display.update_select(x-1,y-1);

                    sleep(SLEEP);   // sleep 1s
                    break;
                default:
                    printf("unknoWN\n");
                    break;
            }       
        // 如果不是 '00'，进入 switch 语句处理其它情况

    }
}

char processFunction(unsigned char data,int &a,int &b){
    //data[3]: Y:80 B:20 A:10 X:40 LB:01 RB:02
        char letter;
        if (eraseDouble==1){
                eraseDouble=0;
            }else{
                eraseDouble=1;
                letter = 'D';
                return letter;
            }
    
        
        switch (data) {
            case 0x80:  // Y
                printf("got Y\n");
                sleep(SLEEP);   // sleep 1s
                letter = 'Y';
                return letter;
                
            case 0x20:  // B
                printf("got B\n");
                sleep(SLEEP);   // sleep 1s
                letter = 'B';
                return letter;
                
            case 0x10:  // A
                printf("got A\n");
                printf("Send coordinate to AI: %d %d\n", a,b);
                sleep(SLEEP);   // sleep 1s
                letter = 'A';
                return letter;
            case 0x40:  // X
                printf("got X\n");
                sleep(SLEEP);   // sleep 1s
                letter = 'X';
                return letter;
            default:
                printf("unknoWN\n");
                letter = 'U';
                return letter;
                
        }    
    


}



char printInput(unsigned char arr[], int size,int &a,int &b,GMKDisplay& display) {
    //read direction input data[2]: up:01 r:08 d:02 l:04

    char action = 'N';      //代表无事发生
    if (arr[2] != 0x00) {
        processDirection(arr[2],a,b,display);
        action ='C';    //代表改变坐标
    }
    if (arr[3] != 0x00){
        action = processFunction(arr[3], a,b);
        //如果是'A'代表发送坐标，结束循环
    }
    return action;
}
/*
int open_controller(libusb_device ***devs, libusb_context **ctx, libusb_device_handle **handle){
    int r; // 
    ssize_t cnt; // 
    

    r = libusb_init(ctx); // 
    if (r < 0) {
        fprintf(stderr, "fail to init libusb: %d\n", r);
        return 1;
    }
    libusb_set_debug(*ctx, 3); // 

    cnt = libusb_get_device_list(*ctx, devs); // 
    if (cnt < 0) {
        fprintf(stderr, "fail to get device list\n");
        return 1;
    }
    printf("find %zd devices\n", cnt);

    libusb_device *device;
    for (ssize_t i = 0; i < cnt; i++) {
        device = *devs[i];
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(device, &desc);


        //4.13:028e is the pid of beitong controller
        if (desc.idVendor == 0x045e && desc.idProduct == 0x028e) {
            printf("find Xbox ：VID 0x%04x, PID 0x%04x\n", desc.idVendor, desc.idProduct);
            //
            int rr;
            rr = libusb_open(*devs[i], handle);
            printf("success use libusb_open");
            
            if (rr != LIBUSB_SUCCESS) {
                fprintf(stderr, "fail to open xbox\n");
                continue;
            } else{
                printf("success open xbox controller");

                //try to detatch kernel driver
                if (libusb_kernel_driver_active(*handle, 0) == 1) { 
                    int rrr;
                    rrr = libusb_detach_kernel_driver(*handle, 0);
                    if (rrr == 0) {
                        printf("Kernel Driver Detached\n");
                    } else {
                        fprintf(stderr, "Error detaching kernel driver: %d\n", rrr);
                    }
                }

            } 
        }
    }

    return 0;



}
*/
int find_xbox_controller() {
    int r;
    ssize_t cnt;

    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "fail to init libusb: %d\n", r);
        return 1;
    }
    libusb_set_debug(ctx, 3);

    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "fail to get device list\n");
        return 1;
    }
    printf("find %zd devices\n", cnt);

    libusb_device *device;
    for (ssize_t i = 0; i < cnt; i++) {
        device = devs[i];
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(device, &desc);

        if (desc.idVendor == 0x045e && desc.idProduct == 0x028e) {
            printf("find Xbox ：VID 0x%04x, PID 0x%04x\n", desc.idVendor, desc.idProduct);
            r = libusb_open(devs[i], &handle);
            if (r != LIBUSB_SUCCESS) {
                fprintf(stderr, "fail to open xbox\n");
                continue;
            } else {
                printf("success open xbox controller\n");
                if (libusb_kernel_driver_active(handle, 0) == 1) {
                    r = libusb_detach_kernel_driver(handle, 0);
                    if (r == 0) {
                        printf("Kernel Driver Detached\n");
                    } else {
                        fprintf(stderr, "Error detaching kernel driver: %d\n", r);
                    }
                }
                return 0;
            }
        }
    }

    return 1;
}


void getCommandXb(libusb_device_handle **handle,int &x,int &y,GMKDisplay& display){

    //above is the part we open a controller
        
    unsigned char endpoint_address = 0x81; // endpoint set to 0x81
    unsigned char data[20]; // 数据缓冲区
    int actual_length; // 实际读取的数据长度
    int timeout = 5000; // 超时时间，以毫秒为单位

    int rr;
    
    
    
    while (1) {
        // 使用libusb_bulk_transfer读取数据
        // actually we keep reading this even if we have no input
        // 
        rr = libusb_interrupt_transfer(*handle, endpoint_address, data, sizeof(data), &actual_length, timeout);
        if (rr == 0) {
            //so we will keep use printInput

            char done =printInput(data,actual_length,x,y,display);
            if (done == 'A'){
                break;   
            }           //直接break，代表参数修改完成,也就是指一个一个落子完成
        } else {
            fprintf(stderr, "fail to read" );
            break; //�
        }
    }
    

    //return 0;



}

void close_controller(libusb_device ***devs, libusb_context **ctx){
    libusb_free_device_list(*devs, 1); // 
    libusb_exit(*ctx); // 

}


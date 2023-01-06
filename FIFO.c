#include <stdio.h>

struct PageTableEntry{
        int dirty; // 1 for write, 0 for read
        int ref;
        int trans; //inMM

};

struct PageTable{
        int VPN;
        char rw;
        struct PageTableEntry p[128];
};

struct MM{ //up to 32 pages
    int dirty;
    int ref;
    int VPN;
    int process;
};

int main(void){
    printf("\n---FIFO VIRTUAL MEMORY SIMULATOR DATA2---\n\n");

    int process, addy, pf = 0;
    int dpw = 0;
    int dr = 0;
    int mm_pc = 0; //MM page count
    int cm = 0;
    int flag = 0;
    char rw;
    struct PageTable pt[4];
    struct MM mainmem[32]; 

    //initialize data to 0 until put on MM.
     for(int i=0;i<4;i++){
        for(int j=0; j<128;j++){
            pt[i].p[j].trans = 0;
            pt[i].p[j].dirty = 0;
            pt[i].p[j].ref = 0;
        }        
    }

    //init MM
    for(int i=0;i<32;i++){
        mainmem[i].dirty = 0;
        mainmem[i].ref = 0;
        mainmem[i].VPN = 0;
        mainmem[i].process = 0;
    }

    FILE *txtf;
    txtf = fopen("data2.txt", "r");
    if (!txtf){
        printf("File not found\n");
        return 1;
    }
    fseek(txtf, 0, SEEK_END);
    long size = ftell(txtf);
    fseek(txtf, 0, SEEK_SET);

    while(1) {
        if(ftell(txtf) == size) {
            break; //EOF
        }

        fscanf(txtf, "%d %d %c", &process, &addy, &rw);
        //printf("SCANNED: %d %d %c\n",process, addy, rw);

        //go to process page table, populate data.
        pt[process-1].VPN = (addy>>9);
        pt[process-1].rw = rw;
        if(pt[process-1].rw == 'W'){
            pt[process-1].p[pt[process-1].VPN].dirty = 1;
        }

        //printf("PROCESS %d: VPN: %d, DIRTY: %d\n", process, pt[process-1].VPN, pt[process-1].p[pt[process-1].VPN].dirty);

        flag = 0; //set FIFO default

        for(int i=0; i<=mm_pc; i++){
            
            if(mainmem[i].process == process && mainmem[i].VPN == pt[process-1].VPN){
                //printf("NO PAGE FAULT. IN MM.\n\n");
                flag = 1;
                break;
            }
        }

        if(flag == 0){
            pf++;
            dr++;
            //printf("PAGE FAULT: %d\n\n", pf);


            
            if(mm_pc < 31){
                mm_pc++;
                //printf("MM_PC: %d\n", mm_pc);
            }

            //printf("AT PAGE: MM[%d]\n", cm);
            //printf("PREVIOUS MM-PROCESS:%d, MM VPN: %d, DIRTY: %d\n\n", mainmem[cm].process, mainmem[cm].VPN, mainmem[cm].dirty);


            if(mm_pc == 31){
                if(pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty == 1){
                    dpw++;
                    dr++;
                    //printf("DPW: %d\n\n", dpw);
                    pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty = 0;
                }

            }


            mainmem[cm].process = process;
            mainmem[cm].VPN = pt[process-1].VPN;
            //mainmem[cm].dirty = pt[process-1].p[pt[process-1].VPN].dirty;
            
            //printf("CURR MM-PROCESS:%d, MM VPN: %d, DIRTY: %d\n\n", mainmem[cm].process, mainmem[cm].VPN, mainmem[cm].dirty);

            if(cm<31){
                cm++;
                //printf("CM: %d\n", cm);
            }

            else{
                cm = 0;
            }
        }



        
        

        



    }//END ROUTINE

    fclose(txtf);
    printf("TOTAL PFs:%d\n", pf);
    printf("TOTAL DPWs: %d\n", dpw);
    printf("TOTAL DISK REFs: %d\n", dr);



    return 0;
    //ANY ADDRESS YOU READ FROM FILE, EXTRACT PROCESS # to go to PAGE TABLE #. THEN PARSE IT INTO VPN(>>9) & OFFSET().
    //

}

//RETURN # of PFS, DISK REFs, & DIRTY WRITES





#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct PageTableEntry{
        int dirty; // 1 for write, 0 for read
        int ref;
        int trans; //inMM?
        //int VPN;
        //private Stopwatch timer;
        //private int pc;
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
    printf("\n---RAND VIRTUAL MEMORY SIMULATOR DATA2---\n");
    printf("SRAND: 12\n");
    srand((unsigned)12);
    int process, addy, pf = 0;
    int dpw = 0;
    int dr = 0;
    int mm_pc = 0; //MM page count
    int cm = 0;
    int flag = 0;
    char rw;
    struct PageTable pt[4];
    struct MM mainmem[32]; 

    //initialize TRANSlations to 0 until put on MM.
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

        flag = 0; //set NOT FOUND default

        for(int i=0; i<32; i++){
            
            if(mainmem[i].process == process && mainmem[i].VPN == pt[process-1].VPN){
                //printf("NO PAGE FAULT. IN MM.\n\n");
                flag = 1;
                break;
            }
        }

        if(flag == 0){
            cm = rand()%32;
            //printf("CM: %d\n");
            pf++;
            dr++;
            //printf("PAGE FAULT: %d\n\n", pf);
            //printf("PREV MM[%d] - PROCESS:%d, MM VPN: %d, DIRTY: %d\n\n", cm, mainmem[cm].process, mainmem[cm].VPN, pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty);

            if(mainmem[cm].VPN != 0){ //PREV MM PAGE CONTENTS EXISTs

                if(pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty == 1){ //check DPW
                    dpw++;
                    dr++;
                    //printf("DPW: %d\n\n", dpw);
                    pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty = 0;
                }

                //then overwrite
                mainmem[cm].process = process;
                mainmem[cm].VPN = pt[process-1].VPN;
                //printf("CURR MM[%d] - PROCESS:%d, MM VPN: %d, DIRTY: %d\n\n", cm, mainmem[cm].process, mainmem[cm].VPN, pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty); 
            }

            else{ //FRESH MM PAGE
                mainmem[cm].process = process;
                mainmem[cm].VPN = pt[process-1].VPN;
                //printf("CURR MM[%d] - PROCESS:%d, MM VPN: %d, DIRTY: %d\n\n", cm, mainmem[cm].process, mainmem[cm].VPN, pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty);
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





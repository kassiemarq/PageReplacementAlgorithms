#include <stdio.h>

struct PageTableEntry{
        int dirty; // 1 for write, 0 for read
        int ref;
        int trans; 

};

struct PageTable{
        int VPN;
        struct PageTableEntry p[128];
};

struct MM{ //up to 32 pages
    // int dirty;
    // int ref;
    int VPN;
    int process;
};

int main(void){
    printf("\n---PER VIRTUAL MEMORY SIMULATOR DATA2---\n\n");

    int process, addy, pf = 0;
    int f1,f2,f3 = 0;
    int dpw = 0;
    int dr = 0;
    int mm_pc = 0; //MM page count
    int count_mr = 0;
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
        //mainmem[i].dirty = 0;
        //mainmem[i].ref = 0;
        mainmem[i].VPN = 0;
        mainmem[i].process = 0;
    }

    FILE *txtf;
    txtf = fopen("data1.txt", "r");
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

        count_mr++;
        if(count_mr == 200){ //SYSTEM RST
            //reset all ref bits
            //printf("SYSTEM RESET\n");
             for(int i=0; i<4; i++){
                for(int j=0; j<128; j++){
                    pt[i].p[j].ref = 0;
                }
            }
            count_mr = 0;
        }

        fscanf(txtf, "%d %d %c", &process, &addy, &rw);
        //printf("SCANNED: %d %d %c\n",process, addy, rw);
        
        


            //go to process page table, populate data.
            pt[process-1].VPN = (addy>>9);
            if(rw == 'W'){
                pt[process-1].p[pt[process-1].VPN].dirty = 1;
            }

            //printf("PROCESS %d: VPN: %d, DIRTY: %d\n", process, pt[process-1].VPN, pt[process-1].p[pt[process-1].VPN].dirty);  

            flag = 0; //set NOT FOUND default
            f1 = 0;
            f2 = 0;
            f3 = 0;

            //search MM if found
            for(int i=0; i<=mm_pc; i++){
                if(mainmem[i].process == process && mainmem[i].VPN == pt[process-1].VPN){
                    //printf("NO PAGE FAULT. IN MM.\n\n");
                    pt[process-1].p[pt[process-1].VPN].ref = 1;
                    //pt[process-1].p[pt[process-1].VPN].trans = 1;
                    flag = 1;
                    break;
                    }
            }

            if(flag == 0){
                pf++;
                //printf("\nPAGE FAULT: %d\n", pf);
                dr++;


                if(mm_pc == 32){ //MM FULL. CM INDEX BASED ON PER ALGO. CHECK DIRTY

                    //choose new cm
                    for(int i=0;i<32;i++){//REF == 0 && DIRTY == 0
                        if(pt[mainmem[i].process-1].p[mainmem[i].VPN].ref == 0 && pt[mainmem[i].process-1].p[mainmem[i].VPN].dirty == 0){
                            cm = i;
                            f1 = 1;
                            //printf("F1 - MM[%d]\n", cm);
                            //printf("previous: MM[%d], P:%d, VPN:%d, REF:%d, DIRTY:%d\n", cm, mainmem[i].process, mainmem[i].VPN, pt[mainmem[i].process-1].p[mainmem[i].VPN].ref, pt[mainmem[i].process-1].p[mainmem[i].VPN].dirty);
                            break;
                        }
                    }

                    if(f1 == 0){ //condition search for REF == 0 && DIRTY == 0 not satisfied
                        for(int i=0;i<32;i++){ //REF == 0 && DIRTY == 1
                            if(pt[mainmem[i].process-1].p[mainmem[i].VPN].ref == 0 && pt[mainmem[i].process-1].p[mainmem[i].VPN].dirty == 1){
                                cm = i;
                                f2 = 1;
                                //printf("F2 - MM[%d]\n", cm);
                                //printf("previous: MM[%d], P:%d, VPN:%d, REF:%d, DIRTY:%d\n", cm, mainmem[i].process, mainmem[i].VPN, pt[mainmem[i].process-1].p[mainmem[i].VPN].ref, pt[mainmem[i].process-1].p[mainmem[i].VPN].dirty);
                                break;
                            }
                        }
                    }

                    if(f1 == 0 && f2 == 0){//condition search for REF == 1 && DIRTY == 0
                        for(int i=0;i<32;i++){
                            if(pt[mainmem[i].process-1].p[mainmem[i].VPN].ref == 1 && pt[mainmem[i].process-1].p[mainmem[i].VPN].dirty == 0){
                                cm = i;
                                f3 = 1;
                                //printf("F3 - MM[%d]\n", cm);
                                //printf("previous: MM[%d], P:%d, VPN:%d, REF:%d, DIRTY:%d\n", cm, mainmem[i].process, mainmem[i].VPN, pt[mainmem[i].process-1].p[mainmem[i].VPN].ref, pt[mainmem[i].process-1].p[mainmem[i].VPN].dirty);
                                break;
                            }
                        }
                    }

                    if(f1 == 0 && f2 == 0){
                        if(f3 == 0){
                            for(int i=0;i<32;i++){//REF == 1 && DIRTY == 1
                                if(pt[mainmem[i].process-1].p[mainmem[i].VPN].ref == 1 && pt[mainmem[i].process-1].p[mainmem[i].VPN].dirty == 1){
                                    cm = i;
                                    //printf("F4 - MM[%d]\n", cm);
                                    //printf("previous: MM[%d], P:%d, VPN:%d, REF:%d, DIRTY:%d\n", cm, mainmem[i].process, mainmem[i].VPN, pt[mainmem[i].process-1].p[mainmem[i].VPN].ref, pt[mainmem[i].process-1].p[mainmem[i].VPN].dirty);
                                    break;
                                }

                            }
                        }
                    }

                    //check if VIC PAGE is previously dirty
                    if(pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty == 1){
                        dpw++;
                        dr++;
                        //printf("DPW: %d\n\n", dpw);
                        pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty = 0;
                        pt[mainmem[cm].process-1].p[mainmem[cm].VPN].trans = 0;
                        pt[mainmem[cm].process-1].p[mainmem[cm].VPN].ref = 0;
                    }

                    mainmem[cm].process = process;
                    mainmem[cm].VPN = pt[process-1].VPN;
                    pt[process-1].p[pt[process-1].VPN].trans = 1;
                    //dirty bit updated in PTE
                    //printf("CURRENT: MM[%d], P:%d, VPN:%d, REF:%d, DIRTY:%d\n\n", cm, mainmem[cm].process, mainmem[cm].VPN, pt[mainmem[cm].process-1].p[mainmem[cm].VPN].ref, pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty);






                }

                else{ //beginning
                     mainmem[cm].process = process;
                     mainmem[cm].VPN = pt[process-1].VPN;
                     pt[process-1].p[pt[process-1].VPN].ref = 1;
                     pt[process-1].p[pt[process-1].VPN].trans = 1;
                     //dirty bit set in PTE
                     //printf("CURRENT: MM[%d], P:%d, VPN:%d, REF:%d, DIRTY:%d\n\n", cm, mainmem[cm].process, mainmem[cm].VPN, pt[mainmem[cm].process-1].p[mainmem[cm].VPN].ref, pt[mainmem[cm].process-1].p[mainmem[cm].VPN].dirty);
                    cm++;
                    mm_pc++;
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





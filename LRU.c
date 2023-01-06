#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

struct PageTableEntry{
        int dirty; // 1 for write, 0 for read
        int ref;
        int trans; 
        double t;
};

struct PageTable{
        int VPN;
        char rw;
        struct PageTableEntry p[128];
};

struct MM{ //up to 32 pages
    int VPN;
    int process;
};

double getMicrotime();

int main(void){
    printf("\n---LRU VIRTUAL MEMORY SIMULATOR DATA2---\n\n");

    int process, addy, pf = 0;
    int dpw = 0;
    int dr = 0;
    int mm_pc = 0; //MM page count
    int cm = 0;
    int new_cm = 0;
    int vp1 = 0;
    int vp2 = 0;
    int flag = 0;
    int cnt_st = 0;
    char rw;
    struct PageTable pt[4];
    struct MM mainmem[32]; 
    struct MM mainmem_copy[32];

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
        // mainmem[i].dirty = 0;
        //mainmem[i].ref = 0;
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

        for(int i=0; i<=mm_pc; i++){ //search MM            
            if(mainmem[i].process == process && mainmem[i].VPN == pt[process-1].VPN){
                //printf("NO PAGE FAULT. IN MM.\n\n");
                pt[process-1].p[pt[process-1].VPN].ref = 1;
                pt[process-1].p[pt[process-1].VPN].t = getMicrotime();
                //printf("UPDATED TIME: %lf\n\n", pt[process-1].p[pt[process-1].VPN].t);
                flag = 1;
                break;
            }
        }

        if(flag == 0){
            pf++;
            dr++;
            //printf("PAGE FAULT: %d\n\n", pf);
           
            if(mm_pc < 31){ //inc index to populate until mem full
                mm_pc++;
            } 

            if(mm_pc == 31){ //MM FULL - CM INDEX BASED LRU

                //find victim page - CM
                
                for(int i=0;i<32;i++){//copy physical mem
                    mainmem_copy[i] = mainmem[i];
                }

                //sort copied array 
                for(int i=0; i<32; i++){
                    for(int j=0; j<32-i-1;j++){
                        if(pt[mainmem_copy[j+1].process-1].p[mainmem_copy[j+1].VPN].t < pt[mainmem_copy[j].process-1].p[mainmem_copy[j].VPN].t){
                            struct MM temp = mainmem_copy[j];
                            mainmem_copy[j] = mainmem_copy[j+1];
                            mainmem_copy[j+1] = temp;
                        }
                    }
                }                

                
        
                if(pt[mainmem_copy[0].process-1].p[mainmem_copy[0].VPN].t == pt[mainmem_copy[1].process-1].p[mainmem_copy[1].VPN].t){//same time unit
                    //CHOOSE NOT DIRTY PAGE

                    for(int i=0; i<32; i++){ //find victim page 1
                        if(mainmem[i].process == mainmem_copy[0].process && mainmem[i].VPN == mainmem_copy[0].VPN){
                            vp1 = i;
                            break;
                        }
                    }

                    for(int i=0; i<32; i++){ //find victim page 2
                        if(mainmem[i].process == mainmem_copy[0].process && mainmem[i].VPN == mainmem_copy[0].VPN){
                            vp2 = i;
                            break;
                        }
                    }

                    //find who is dirty
                    if(pt[mainmem[vp1].process-1].p[mainmem[vp1].VPN].dirty == 1 && pt[mainmem[vp2].process-1].p[mainmem[vp2].VPN].dirty == 1){
                        //both dirty, choose lower # page
                        if(vp1 < vp2){
                            new_cm = vp1;
                        }
                        else{
                            new_cm = vp2;
                        }
                    }
                    else if(pt[mainmem[vp1].process-1].p[mainmem[vp1].VPN].dirty == 0 && pt[mainmem[vp2].process-1].p[mainmem[vp2].VPN].dirty == 0){
                       //both neither dirty, choose lower # page 
                       if(vp1 < vp2){
                            new_cm = vp1;
                        }
                        else{
                            new_cm = vp2;
                        }
                    }

                    else if(pt[mainmem[vp1].process-1].p[mainmem[vp1].VPN].dirty == 0 && pt[mainmem[vp2].process-1].p[mainmem[vp2].VPN].dirty == 1){
                        //vp1 non dirty
                        new_cm = vp1;
                    }

                    else if(pt[mainmem[vp1].process-1].p[mainmem[vp1].VPN].dirty == 1 && pt[mainmem[vp2].process-1].p[mainmem[vp2].VPN].dirty == 0){
                        //vp2 non dirty
                        new_cm = vp2;
                    }


                }//end same time unit

                else{
                    //NO SAME TIME. assume mainmemcopy index 0 has least time
                    //search MM for victim page 
                    for(int i=0; i<32; i++){
                        if(mainmem[i].process == mainmem_copy[0].process && mainmem[i].VPN == mainmem_copy[0].VPN){
                            new_cm = i;
                            break;
                        }
                    }                    
                }

                // //search MM for victim page 
                // for(int i=0; i<32; i++){
                //     if(mainmem[i].process == mainmem_copy[0].process && mainmem[i].VPN == mainmem_copy[0].VPN){
                //         new_cm = i;
                //         break;
                //     }
                // }

                //check if VIC page is dirty before overwriting
                //printf("PREV MM[%d] P:%d, MM VPN: %d, DIRTY: %d, REF: %d\n\n",new_cm, mainmem[new_cm].process, mainmem[new_cm].VPN, pt[mainmem[new_cm].process-1].p[mainmem[new_cm].VPN].dirty, pt[mainmem[new_cm].process-1].p[mainmem[new_cm].VPN].ref);
                if(pt[mainmem[new_cm].process-1].p[mainmem[new_cm].VPN].dirty == 1){
                    dpw++;
                    dr++;
                    //printf("DPW: %d\n\n", dpw);
                    pt[mainmem[new_cm].process-1].p[mainmem[new_cm].VPN].dirty = 0;
                    //pt[mainmem[new_cm].process-1].p[mainmem[new_cm].VPN].ref = 0;
                    pt[mainmem[new_cm].process-1].p[mainmem[new_cm].VPN].trans = 0; 
                }

                mainmem[new_cm].process = process;
                mainmem[new_cm].VPN = pt[process-1].VPN;

                pt[process-1].p[pt[process-1].VPN].ref = 1; 
                pt[process-1].p[pt[process-1].VPN].trans = 1; 
                pt[process-1].p[pt[process-1].VPN].trans = 1;
                pt[process-1].p[pt[process-1].VPN].t = getMicrotime(); //update time

                //printf("NEW MM-PROCESS[%d]:%d, MM VPN: %d, DIRTY: %d, REF: %d, TIME:%lf\n\n", new_cm, mainmem[new_cm].process, mainmem[new_cm].VPN, pt[process-1].p[pt[process-1].VPN].dirty, pt[process-1].p[pt[process-1].VPN].ref, pt[process-1].p[pt[process-1].VPN].t);


            }

            else{ //beginning
                 mainmem[cm].process = process;
                 mainmem[cm].VPN = pt[process-1].VPN;
                 pt[process-1].p[pt[process-1].VPN].ref = 1; 
                 pt[process-1].p[pt[process-1].VPN].trans = 1; 
                 pt[process-1].p[pt[process-1].VPN].t = getMicrotime();
                 //dirty bit updated in PTE
                 //printf("CURR MM[%d] P:%d, MM VPN: %d, DIRTY: %d, REF: %d, TIME:%lf\n\n",cm, mainmem[cm].process, mainmem[cm].VPN, pt[process-1].p[pt[process-1].VPN].dirty, pt[process-1].p[pt[process-1].VPN].ref, pt[process-1].p[pt[process-1].VPN].t);

            }

            if(cm<31){
                cm++;
                //printf("CM: %d\n", cm);
            }

        }



        
        

        



    }//END ROUTINE

    fclose(txtf);
    printf("TOTAL PFs:%d\n", pf);
    printf("TOTAL DPWs: %d\n", dpw);
    printf("TOTAL DISK REFs: %d\n", dr);



    return 0;
    //ANY ADDRESS YOU READ FROM FILE, EXTRACT PROCESS # to go to PAGE TABLE #. THEN PARSE IT INTO VPN(>>9) & OFFSET().

}

double getMicrotime(){
    double microsec;
    double sec;
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
    //printf("SEC: %ld, MICROSEC: %ld\n", currentTime.tv_sec, currentTime.tv_usec);
    microsec = currentTime.tv_usec * pow(10,-6);
   // printf("MICROSEC: %lf\n", microsec);
    sec = currentTime.tv_sec + microsec;
    //printf("CALC SEC: %lf\n", sec);
	// return currentTime.tv_sec + (int)1e6 + currentTime.tv_usec; 
    return(sec);
}





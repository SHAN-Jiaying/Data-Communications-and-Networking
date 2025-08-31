#include <stdio.h>
#include <stdlib.h>

// Define a global array represent the memory
//int **memory;
//int NUM_FRAMES;
//int FRAME_SIZE;

// Function to ask user for array size and populate the global array
/*void set_array_size() {
    printf("Enter the toatl number of frames: ");
    scanf("%d", &NUM_FRAMES);
    printf("Enter the frame size: ");
    scanf("%d", &FRAME_SIZE);

    // Allocate memory represent the memory
    memory = (int **)malloc(NUM_FRAMES * sizeof(int *));
    for (int i = 0; i < NUM_FRAMES; i++) {
        memory[i] = (int *)malloc(FRAME_SIZE * sizeof(int));
    }
    printf("Memory allocated successfully, NUM_FRAMES is %d and FRAME_SIZE is %d", NUM_FRAMES, FRAME_SIZE);
}*/

#define FRAME_SIZE 1024
#define NUM_FRAMES 64
int memory[NUM_FRAMES][FRAME_SIZE];
int freeMemoryPointer = 0;
#define INVALID -1 // 用于表示无效的值
#define PAGE_SIZE 1024// 页面大小的位数
#define PAGE_TABLE_SIZE 64
#define TLB_SIZE 64
#define BUFFER_SIZE 1024

int page_table[PAGE_TABLE_SIZE]; // 定义页表数组

int tlb[TLB_SIZE][2];// 定义TLB
int tlbPointer=0;

int isPowerOfTwo(int n) {
    int count = 0;

    if (n <= 0) {
        return 0;  // 非正数不是2的幂
    }

    while (n) {
        count += n & 1;  // 检查最低位是否为1
        n >>= 1;  // 右移一位
    }

    return count == 1;  // 如果计数结果为1，则是2的幂
}

void getUserInput(int *num_frames, int *frame_size)
{
        int logical_space = 65536; // 逻辑空间大小
    
    while (1) {
        printf("Enter the total number of frames: ");
        scanf("%d", num_frames);
        printf("Enter the frame size: ");
        scanf("%d", frame_size);

        if (!isPowerOfTwo(*num_frames * *frame_size)) {
            printf("Error: Total size of physical memory must be power of 2.\n");
            continue;
        }

        if (*num_frames * *frame_size > logical_space) {
            printf("Error: The size of physical memory must be smaller than logical space.\n");
            continue;
        }

        break;
    }

    printf("Memory allocated successfully, NUM_FRAMES is %d and FRAME_SIZE is %d\n", NUM_FRAMES, FRAME_SIZE);
}
        

void intialize_page_table(){
    for(int i=0;i<PAGE_TABLE_SIZE;i++){
        page_table[i] = INVALID;
    }
}

void intialize_tlb(){
    for(int i=0;i<TLB_SIZE;i++){
        tlb[i][0] = INVALID;
        tlb[i][1] = INVALID;
    }
}

void intialize_memory(int num_frames, int frame_size){
    for(int i=0;i<num_frames;i++){
        for(int j=0;j<frame_size;j++){
            memory[i][j]=INVALID;
        }
    }
}

void translate_address(int logical_address){

}
int find_page_number(int logical_address){
    return logical_address/PAGE_SIZE;
}
int find_page_offset(int logical_address){
    return logical_address%PAGE_SIZE;
}
void update_page_table(int page_number, int memory_pointer){
    page_table[page_number] = memory_pointer;
    //printf("update %d %d\n",page_number,memory_pointer);
}
void update_tlb(int page_number, int frame_number){
    tlb[tlbPointer][0]=page_number;
    tlb[tlbPointer][1]=frame_number;
    tlbPointer=(tlbPointer+1)%TLB_SIZE;
}
void replaced_update_page_table(int replacedPageNumber,int page_number, int memory_pointer){
    page_table[page_number] = memory_pointer;
    page_table[replacedPageNumber]==INVALID;
    //printf("update %d %d\n",page_number,memory_pointer);
}
void replaced_update_tlb(int replacedPageNumber,int page_number, int frame_number){
    for(int i=0;i<TLB_SIZE;i++){
        if(tlb[i][0]==replacedPageNumber){
            tlb[i][0]=page_number;
            tlb[i][1]=frame_number;
        }
    }
}
int find_page_num_in_table(int frameNumber){
    for(int i=0;i<PAGE_TABLE_SIZE;i++){
        if(page_table[i]==frameNumber){
            return i;
        }
    }
    return INVALID;
}
// read binary file
void load_page(int page_number){
    FILE *backing_store = fopen("backingstore.bin", "rb");
    if (backing_store == NULL) {
        perror("Error opening backingstore.bin");
        exit(1);
    }
    int offset = page_number * PAGE_SIZE;
    fseek(backing_store, offset, SEEK_SET);

    // 读取整个页面内容
    unsigned char *page_data = (unsigned char *)malloc(PAGE_SIZE);
    if (page_data == NULL) {
        perror("Memory allocation error");
        exit(1);
    }
    fread(page_data, 1, PAGE_SIZE, backing_store);

    if(memory[freeMemoryPointer][0]==INVALID){
        //load data into memory step4.4
        for (int i = 0; i < PAGE_SIZE; i++) {
            //printf("i:%d %02X ", i, page_data[i]);
            memory[freeMemoryPointer][i] = page_data[i];
        }
        update_page_table(page_number,freeMemoryPointer);
        update_tlb(page_number,freeMemoryPointer);
        printf("\t [Load Page] Page page no %d. -> Frame frame no %d.\n", page_number, freeMemoryPointer);
    }
    else{
        //load data into replaced frame memory step4.5
        int replacedPageNumber=find_page_num_in_table(freeMemoryPointer);
        for (int i = 0; i < PAGE_SIZE; i++) {
            //printf("i:%d %02X ", i, page_data[i]);
            memory[freeMemoryPointer][i] = page_data[i];
        }
        replaced_update_page_table(replacedPageNumber,page_number,freeMemoryPointer);
        replaced_update_tlb(replacedPageNumber,page_number,freeMemoryPointer);
        printf("\t [Replace Page] Frame frame no %d: Page %d -> Page %d\n", freeMemoryPointer, replacedPageNumber,page_number);
    }
    freeMemoryPointer=(freeMemoryPointer+1)%NUM_FRAMES;

    free(page_data);
    fclose(backing_store);
}
int search_tlb(int page_number){
    for(int i=0;i<TLB_SIZE;i++){
        if(tlb[i][0]==page_number){
            return tlb[i][1];
        }
    }
    return INVALID;
}
int search_page_table(int page_number){
    return page_table[page_number];
}
int get_memory(int memory_frame_pointer, int offset){
    return memory[memory_frame_pointer][offset];
}
void read_addresses(){
    int counter=0;
    int tlb_hit=0;
    int page_fault=0;

    FILE *address_file = fopen("addresses.txt", "r");
    char buffer[BUFFER_SIZE];
    if (address_file == NULL) {
        perror("Error opening file");
        exit(1);
    }
    //int i=0;
    while (fgets(buffer, BUFFER_SIZE, address_file) != NULL) {
        //printf("Input: %s", buffer);
        /*if(i==40){
            break;
        }
        i++;*/

        //do something with the address
        int logical_address = atoi(buffer);

        int page_number = find_page_number(logical_address);
        int offset = find_page_offset(logical_address);
        printf("page_number: %d, offset: %d\n", page_number, offset);
        
        //search in tlb first step4.2
        int search_tlb_result=search_tlb(page_number);
        if(search_tlb_result==INVALID){
            //search in page table step4.3
            int search_page_table_result=search_page_table(page_number);
            if(search_page_table_result==INVALID){
                page_fault++;

                //load page into a free frame in the memory step4.4 and 4.5
                load_page(page_number);

                //go back to step2
                search_tlb_result=search_tlb(page_number);
                if(search_tlb_result==INVALID){
                    perror("Error,page should inside tlb");
                    exit(1);
                }
                //calculate physical address
                int physical_adderss=search_tlb_result*PAGE_SIZE+offset;
                printf("[TLB] (LA) %d -> (PA) %d: %02X\n",logical_address,physical_adderss,get_memory(search_tlb_result,offset));
            }
            else{
                //calculate physical address
                int physical_adderss=search_page_table_result*PAGE_SIZE+offset;
                printf("[Page Table] (LA) %d -> (PA) %d: %02X\n",logical_address,physical_adderss,get_memory(search_page_table_result,offset));
                //update TLB to include this page
                update_tlb(page_number,search_page_table_result);
            }
        }
        else{
            tlb_hit++;
            //calculate physical address
            int physical_adderss=search_tlb_result*PAGE_SIZE+offset;
            printf("[TLB] (LA) %d -> (PA) %d: %02X\n",logical_address,physical_adderss,get_memory(search_tlb_result,offset));
        }
        counter++;
        //translate_address(logical_address);
    }
    /*for(int i=0;i<PAGE_TABLE_SIZE;i++){
        printf("page table information %d: %d\n",i,page_table[i]);
    }*/
    fclose(address_file);

    
    //generate stat.txt
    float page_fault_rate=(float)page_fault/counter;
    float tlb_hit_rate=(float)tlb_hit/counter;

    FILE *stat_file = fopen("stat.txt", "w");
    if (stat_file == NULL) {
        perror("Error opening stat.txt");
        exit(1);
    }
    fprintf(stat_file, "page-fault rate: %.2f\n", page_fault_rate);
    fprintf(stat_file, "TLB hit rate: %.2f\n", tlb_hit_rate);

    //write memory image into stat.txt
    fprintf(stat_file, "Memory image:\n");
    for(int i=0;i<NUM_FRAMES;i++){
        int indexStart=i;
        int indexEnd=i+15;
        if(indexStart%16==0){
            fprintf(stat_file, "Frame %d ~ %d: ",indexStart,indexEnd);
        }
        
        int pageNumber=find_page_num_in_table(i);
        fprintf(stat_file, "%d ",pageNumber);

        if(indexStart%16==15){
            fprintf(stat_file, "\n");
        }
    }
    fclose(stat_file);
}
int main(){
    // Call the function to set the array size
    //set_array_size();
    //free(memory);
    //printf("Input frame size (use number of bits to represent): ");
    
    int num_frames = NUM_FRAMES;
    int frame_size = FRAME_SIZE;
    getUserInput(&num_frames, &frame_size);

    intialize_memory(num_frames, frame_size);
    intialize_tlb();
    intialize_page_table();
    // 读取地址文件
    read_addresses();
    
    
    //load_page(62);
    //translate_address(16916);
    /*for(int i=0;i<1024;i++){
        printf("%d is %02X \n", i, memory[57][i]);
    }*/
    //printf("test: %02X\n",memory[12][562]);
    //printf("test: %02X\n",memory[13][562]);
    return 0;
}
// #include <stdio.h>
// #include <stdint.h>

// typedef uint32_t block_t; //每一个块为4bit
// typedef uint32_t count_t;	
// typedef uint8_t  u8_t;


// typedef struct{
//     uint8_t a;
// }spinlock_t;

// #define spin_lock_init(x) 
// #define spin_unlock(x)
// #define spin_lock(x)

// #define minimum_size 8    //最小碎片大小


// typedef void* (*memory_block_operation_t)(struct haper2*haper, struct memory_block* block,void *arg); //自定义一个迭代器函数，他会在迭代器中被自动调用

// struct memory_block
// {
// 	u8_t  flag; 
// 	#define BASE_MAGIC 0xABCD 
// 	uint16_t magic;                        
// 	u8_t   owner;    //暂时用于填充
// 	size_t block_size;					
// 	struct memory_block* next; 
// 	struct memory_block* priv_block;    

// }__attribute__((aligned(4)));

// struct haper2 {
// 	uint8_t mode;                        
// 	block_t* space;
// 	spinlock_t lock;
// 	count_t memory_size;                  
// 	struct memory_block* start_addr;     
// };

// #define use_flag     (0x01 << 0)  //you can add other flags in there
// #define other_flag_0 (0x01 << 1)
// #define	other_flag_1 (0x01 << 2)
// #define other_flag_2 (0x01 << 3)
// #define other_flag_3 (0x01 << 4)
// #define other_flag_4 (0x01 << 5)
// #define other_flag_5 (0x01 << 6)
// #define other_flag_6 (0x01 << 7)

// static void reset_block_status(struct memory_block* block,uint8_t status){                         
// 	block->flag &= ~status;
// }
// static void set_block_status(struct memory_block* block,uint8_t status){						
// 	block->flag |= status;
// }
// static int get_block_status(struct memory_block* block,uint8_t status){							
// 	if (block->flag & status)return 1;
// 	else return 0;
// } 
// static struct memory_block* get_next_block(struct memory_block* block){                            
// 	return block->next;
// }
// static struct memory_block* get_priv_block(struct memory_block* block) {						   
// 	return block->priv_block;
// }
// static size_t get_block_size(struct memory_block* block){						
// 	return block->block_size;
// }
// static void clean_block(struct memory_block* block){						
// 	if (block == NULL) return;
// 	u8_t* data = (u8_t*)block;
// 	int length = sizeof(struct memory_block) + block->block_size;
// 	for (int i = 0; i < length; i++){
// 		data[i] = 0;
// 	}
// }
// static int haper2_init(struct haper2* haper, block_t* space, size_t space_size,uint8_t mode){	
// 	if (space_size < sizeof(struct memory_block)){
// 			haper->memory_size = 0;
// 			haper->mode = 0;
// 			haper->start_addr = NULL;
// 			haper->space = NULL;
// 		 return -1;
// 	}		
// 	struct memory_block *rootblock = (struct memory_block*)space;    
// 	rootblock->block_size = space_size - sizeof(struct memory_block);
// 	rootblock->magic = BASE_MAGIC;
// 	reset_block_status(rootblock, use_flag);									                                                
// 	rootblock->priv_block = (struct memory_block*) NULL;                                          
// 	rootblock->next		  = (struct memory_block*) NULL;
// 	haper->space = space;	
// 	haper->mode = mode;
// 	spin_lock_init(&haper->lock);
// 	haper->memory_size  = space_size;                         				
// 	haper->start_addr = rootblock;
// 	return 0;
// }
// static struct memory_block* incise_free_block(struct memory_block* father_block, size_t size){      
// 	if (father_block == NULL || size <= 0) return (struct memory_block*)NULL;
// 	if (size + sizeof(struct memory_block) > get_block_size(father_block) ) return (struct memory_block*)NULL;
// 	if (get_block_status(father_block,use_flag) == 1 ) return (struct memory_block*)NULL;                                
	
// 	size_t memory_offset = sizeof(struct memory_block) + size;
// 	struct memory_block* newblock =(struct memory_block*)( (u8_t*)(father_block) + memory_offset);			
// 	newblock->block_size = father_block->block_size - memory_offset;
// 	father_block->block_size = size;
// 	reset_block_status(newblock,use_flag);										
// 	newblock->next = father_block->next;    
// 	newblock->magic = BASE_MAGIC;                               
// 	if(father_block->next) father_block->next->priv_block = newblock;
// 	father_block->next   = newblock;
// 	newblock->priv_block = father_block;
// 	return father_block;
// }
// static struct memory_block* free_block(struct memory_block* block){								
// 	if (block == NULL) return (struct memory_block*)NULL;
// 	struct memory_block* priv_block = get_priv_block(block);
// 	struct memory_block* next_block = get_next_block(block);
// 	reset_block_status(block,use_flag);                                                   
// 	if (priv_block && get_block_status(priv_block,use_flag) == 0 ){
// 		priv_block->block_size += sizeof(struct memory_block) + block->block_size;  
// 		priv_block->next = next_block;
// 		if(next_block)	next_block->priv_block = priv_block;
// 		clean_block(block);	
// 		return priv_block;	
// 	}
// 	if(next_block && get_block_status(next_block,use_flag) == 0){
// 		block->block_size += sizeof(struct memory_block) + next_block->block_size;
// 		block->next = next_block->next;
// 		if(next_block->next) next_block->next->priv_block = block;
// 		clean_block(next_block);		
// 		return block;
// 	}
// 	return block;
// }

// static void free_memory(struct haper2* haper, struct memory_block* block){  
// 	if (block == NULL) return;
// 	struct memory_block* new_block = block;
// 	size_t block_size = 0;
// 	while (block_size != new_block->block_size){
// 		block_size = new_block->block_size;
// 		new_block = free_block(new_block);
// 	}
// }
// static void * memory_block_laterator(struct haper2* haper, memory_block_operation_t operation,void *data){ 
// 	if(haper == NULL) return 0;
// 	struct memory_block* curr = haper->start_addr;
// 	if(curr == NULL) return 0;
// 	while (curr != NULL) {
// 		if(operation(haper, curr ,data) == 0) return data;
// 		curr = curr->next;
// 	}
// 	return data;
// }
// static size_t* memory_block_free_size_counter(struct haper2* haper, struct memory_block* block, size_t* size) { //统计空闲内存大小
// 	if (block == NULL) return 0;
// 	if(get_block_status(block,use_flag) == 0){
// 		*size += get_block_size(block);
// 	}
// 	return 1; //continue
// }
// static size_t get_free_space(struct haper2* haper){
// 	if (haper == NULL) return 0;
// 	size_t i = 0;
// 	memory_block_laterator(haper, memory_block_free_size_counter, &i);
// 	return i;
// }
// static void* best_fit_alloc(struct haper2* haper, size_t size) {
// 	if (haper == NULL || size == 0) return NULL;
// 	struct memory_block* best_block = (struct memory_block*)NULL;
// 	struct memory_block* curr = haper->start_addr;
// 	spin_lock(&haper->lock);
// 	while (curr != NULL) {
// 		if (get_block_status(curr,use_flag) == 0 && curr->block_size >= size) {
// 			if (best_block == NULL || curr->block_size <= best_block->block_size) {
// 				best_block = curr;
// 			}
// 		}
// 		curr = curr->next;
// 	}
// 	if (best_block == NULL) {
// 		spin_unlock(&haper->lock);
// 		return NULL;
// 	}
// 	if (best_block->block_size >= size + sizeof(struct memory_block) + minimum_size){ 
// 		incise_free_block(best_block, size);
// 	}
// 	set_block_status(best_block, use_flag);
// 	spin_unlock(&haper->lock);
// 	return (void*)(best_block + 1);
// }

// static void* first_fit_alloc(struct haper2* haper, size_t size) {
// 	if (haper == NULL || size == 0) return NULL;
// 	struct memory_block* best_block = (struct memory_block*)NULL;
// 	struct memory_block* curr = haper->start_addr;
// 	spin_lock(&haper->lock);
// 	while (curr != NULL) {
// 		if (get_block_status(curr,use_flag) == 0 && curr->block_size >= size) {
// 			if (best_block == NULL || curr->block_size < best_block->block_size) {
// 				best_block = curr;
// 				break;
// 			}
// 		}
// 		curr = curr->next;
// 	}
// 	if (best_block == NULL) {
// 		spin_unlock(&haper->lock);
// 		return NULL;
// 	}
// 	if (best_block->block_size >= size + sizeof(struct memory_block)){ 
// 		incise_free_block(best_block, size);
// 	}
// 	set_block_status(best_block, use_flag);
// 	spin_unlock(&haper->lock);
// 	return (void*)(best_block + 1);
// }
// static struct get_block {
//     void* addr;
// 	struct memory_block* block;
// };
// static int get_block(struct haper2* haper, struct memory_block* block, void* arg){ //识别目标内存块
// 	if (block == NULL) return 0;
// 	struct get_block* get_block = (struct get_block*)arg;
// 	if ((void*)(block + 1) == get_block->addr) {
// 		get_block->block = block;
// 		return 0; 
// 	}
// 	return 1;
// }
// static int free_haper_block(struct haper2* haper, void* ptr){ //安全地释放内存块
// 	if (ptr == NULL || haper == NULL) return -1;
// 	struct get_block block = {
// 		.addr = ptr,
// 		.block = NULL
// 	};
// 	spin_lock(&haper->lock);
// 	memory_block_laterator(haper, get_block, &block);
// 	if (block.block == NULL){spin_unlock(&haper->lock);return -1;}
// 	free_memory(haper, block.block);
// 	spin_unlock(&haper->lock);
// 	return 0;
// }
// static struct haper_print_block{
// 	int count;
// 	void (*printdata)(char *data,...);
// };
// static int print_block_status(struct haper2* haper,struct memory_block* block, struct haper_print_block* data)
// {	
// 	if (block == NULL) return 0;
// 	void (*printdata)(char *data,...) = data->printdata;
// 	printdata("---------------------------\n");	
// 	printdata("block number:%d\n",data->count);
// 	if(get_block_status(block,use_flag) == 0){
// 		printdata("block is free\n");
// 	}
// 	else{
// 		printdata("block is used\n");
// 	}
// 	printdata("block size:%d\n",block->block_size);
// 	printdata("block addr:%p\n",block);
// 	data->count++;
// 	return 1;
// }
// static int show_blocks_status(struct haper2* haper,void (*printdata)(char *data,...)) //打所有内存块状态，可用于调试
// {
// 	if (printdata == NULL || haper == NULL) return -1;
// 	if(haper->start_addr == NULL) return -1;
// 	struct haper_print_block printdata_struct = {
// 		.count = 0,
// 		.printdata = printdata
// 	};
// 	memory_block_laterator(haper, print_block_status, &printdata_struct);
// 	printdata("---------------------------\n");	
// 	return 0;
// }

// static void* check_blocks(struct haper2* haper,struct memory_block* block, int* flag)
// {
// 	if(flag[0] == 1) return 0; //直接向迭代器发出终止指令
// 	if(block->magic != BASE_MAGIC) {
// 		flag[0] = 1;
// 		return 0;              //标记并直接终止遍历
// 	}
// 	return 1; //继续遍历
// }
// static int check_blocks_completeness(struct haper2* haper)
// {
// 	if(haper == NULL) return 0;
// 	int flag = 0;
// 	memory_block_laterator(haper,check_blocks,&flag);
// 	return flag;
// }

// // #endif


// #define USER_HEAP_SIZE 1024*8
// static uint8_t user_heap[USER_HEAP_SIZE];
// static struct haper2 haper_inst;

// void init_heap() {
//     static int initialized = 0;
//     if (!initialized) {
//         haper2_init(&haper_inst, (block_t*)user_heap, USER_HEAP_SIZE, 0);
//         initialized = 1;
//     }
// }

// void *malloc(size_t size) {
//     return best_fit_alloc(&haper_inst, size);
// }
// void free(void *ptr) {
//     free_haper_block(&haper_inst, ptr);
// }
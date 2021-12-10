#include <stdio.h>
#include <math.h>
#include <stdlib.h>

struct useful_parameter {

	int block_size;
	int sets_num;
	int associativity;
	int offset;
	int index;
	int tag;

};
struct line {

	char rw;
	int address;
	char id;
	int tag;
	int index;
	int offset;
};
struct entry {
	int tag;
	int dirty;
	int pos;

};
struct cache {
	struct entry* entry;
	int index;
};
struct cache_result {
	int tag_pos;
	int index_pos;
};
struct final_result {
	int hit;
	int miss;
	int mem_ref;
	int data_num;
	int data_hit;
	int data_miss;
	int data_ref;
	int ins_num;
	int ins_hit;
	int ins_miss;
	int ins_ref;
};


void translate_line(char lines[100][256], struct line* line_set, int lines_number);
int bitExtracted(int number, int k, int p);
void parse_address(struct line* lines, int lines_number, struct useful_parameter parameters);
void init_cache(struct cache* caches, struct useful_parameter parameters);
void show_cache(struct cache* caches, struct useful_parameter parameters);
void init_final_result(struct final_result* result);
int check_tag_capcity(struct entry* entry, struct useful_parameter parameters);
int check_tag_exist(struct entry* entry, int target, struct useful_parameter parameters);
struct entry lru_cache(struct entry* entry, int inserted_tag, struct useful_parameter parameters);
struct cache_result search_cache(struct cache* caches, int target_index, int target_tag, struct useful_parameter parameters);

struct final_result write_through(struct line* lines_set, struct cache* caches, int lines_number, struct useful_parameter parameters);
struct final_result write_back(struct line* lines_set, struct cache* caches, int lines_number, struct useful_parameter parameters);

struct final_result write_through_split(struct line* lines_set, struct cache* data_caches, struct cache* ins_caches, int lines_number, struct useful_parameter parameters);
struct final_result write_back_split(struct line* lines_set, struct cache* data_caches, struct cache* ins_caches, int lines_number, struct useful_parameter parameters);
int main(void) {

	/*****************Part1: Reading file*******************/
	//Reading file
	//FILE* fp = fopen("./test5.txt", "r");

	//useful parameters
	char temp[60];
	int block_size;
	int sets_num;
	int associativity;

	fgets(temp, 256, stdin);
	block_size = atoi(temp);
	fgets(temp, 256, stdin);
	sets_num = atoi(temp);
	fgets(temp, 256, stdin);
	associativity = atoi(temp);

	int offset;
	int index;
	int tag;
	offset = log(block_size) / log(2);
	index = log(sets_num) / log(2);
	tag = 32 - offset - index;

	struct useful_parameter unified_parameter;
	unified_parameter.block_size = block_size;
	unified_parameter.associativity = associativity;
	unified_parameter.sets_num = sets_num;
	unified_parameter.index = index;
	unified_parameter.tag = tag;
	unified_parameter.offset = offset;


	printf("================ Unified ================\n");
	printf("Block size: %d\n", block_size);
	printf("Number of sets: %d\n", sets_num);
	printf("Associativity: %d\n", associativity);
	printf("Number of offset bits: %d\n", offset);
	printf("Number of index bits: %d\n", index);
	printf("Number of tag bits: %d\n", tag);



	/*****************Part2:Parse Instructions*******************/
	//reading rest instrucions
	int lines_number = 0;
	char lines_string[100][256];

	while (fgets(lines_string[lines_number], 256, stdin) != NULL) {
		//printf("Line %d: %s",lines_number,lines[lines_number]);
		lines_number++;
	}



	//line set declration
	struct line* lines_set = (struct line*)malloc((lines_number + 1) * sizeof(struct line));
	translate_line(lines_string, lines_set, lines_number);
	parse_address(lines_set, lines_number, unified_parameter);

	/*****************Part3:Initializing Caches*******************/
	struct cache* unified_caches = (struct cache*)malloc((sets_num) * sizeof(struct cache));
	init_cache(unified_caches, unified_parameter);
	//show_cache(unified_caches, unified_parameter);

	/*****************Part4:Unified Write Through*******************/
	struct final_result unified_result;
	init_final_result(&unified_result);
	unified_result = write_through(lines_set, unified_caches, lines_number, unified_parameter);
	printf("****************************************\n");
	printf("Write-through with No Write Allocate\n");
	printf("****************************************\n");
	printf("Total number of references: %d\n", lines_number);
	printf("Hits: %d\n", unified_result.hit);
	printf("Misses: %d\n", unified_result.miss);
	printf("Memory References: %d\n", unified_result.mem_ref);

	/*****************Part4:Unified Write back*******************/
	init_cache(unified_caches, unified_parameter);
	init_final_result(&unified_result); 
	unified_result = write_back(lines_set, unified_caches, lines_number, unified_parameter);

	printf("****************************************\n");
	printf("Write-back with Write Allocate\n");
	printf("****************************************\n");
	printf("Total number of references: %d\n", lines_number);
	printf("Hits: %d\n", unified_result.hit);
	printf("Misses: %d\n", unified_result.miss);
	printf("Memory References: %d\n", unified_result.mem_ref);

	/*****************Part5:Split Write through*******************/
	sets_num = sets_num / 2;
	offset = log(block_size) / log(2);
	index = log(sets_num) / log(2);
	tag = 32 - offset - index;

	struct useful_parameter split_parameters;
	split_parameters.associativity = associativity;
	split_parameters.block_size = block_size;
	split_parameters.sets_num = sets_num;
	split_parameters.index = index;
	split_parameters.offset = offset;
	split_parameters.tag = tag;

	printf("============ = Split I vs.D ============ =\n");
	printf("Block size: %d\n", block_size);
	printf("Number of sets: %d\n", sets_num);
	printf("Associativity: %d\n", associativity);
	printf("Number of offset bits: %d\n", offset);
	printf("Number of index bits: %d\n", index);
	printf("Number of tag bits: %d\n", tag);
	printf("****************************************\n");
	printf("Instructions:\n");
	printf("****************************************\n");

	//update lines struct 
	translate_line(lines_string, lines_set, lines_number);
	parse_address(lines_set, lines_number, split_parameters);

	//initialize caches
	struct cache* ins_caches = (struct cache*)malloc((sets_num) * sizeof(struct cache));
	struct cache* data_caches = (struct cache*)malloc((sets_num) * sizeof(struct cache));
	init_cache(ins_caches, split_parameters);
	init_cache(data_caches, split_parameters);

	struct final_result split_result;
	split_result = write_through_split(lines_set, data_caches, ins_caches, lines_number, split_parameters);
	printf("Total number of references: %d\n", split_result.ins_num);
	printf("Hits: %d\n", split_result.ins_hit);
	printf("Misses: %d\n", split_result.ins_miss);
	printf("Memory References: %d\n", split_result.ins_ref);
	printf("****************************************\n");
	printf("Data: Write-through with No Write Allocate\n");
	printf("****************************************\n");
	printf("Total number of references: %d\n", split_result.data_num);
	printf("Hits: %d\n", split_result.data_hit);
	printf("Misses: %d\n", split_result.data_miss);
	printf("Memory References: %d\n", split_result.data_ref);
	printf("****************************************\n");
	printf("Data: Write-back with Write Allocate\n");
	printf("****************************************\n");
	init_cache(ins_caches, split_parameters);
	init_cache(data_caches, split_parameters);
	split_result = write_back_split(lines_set, data_caches, ins_caches, lines_number, split_parameters);
	printf("Total number of references: %d\n", split_result.data_num);
	printf("Hits: %d\n", split_result.data_hit);
	printf("Misses: %d\n", split_result.data_miss);
	printf("Memory References: %d\n", split_result.data_ref);

	//fclose(fp);

	return 0;

}

void translate_line(char lines[100][256], struct line* line_set, int lines_number) {

	int i;
	for (i = 0; i < lines_number; i++) {

		//printf("lines： %s", lines[i]);
		char rw = ' ';
		char id = ' ';
		int address = 0;
		sscanf(lines[i], "%c\t%d\t%c\n", &rw, &address, &id);
		//printf("rw: %c add: %d id: %c\n", rw, address, id);
		line_set[i].rw = rw;
		line_set[i].id = id;
		line_set[i].address = address;

	}


}

int bitExtracted(int number, int k, int p) {
	return (((1 << k) - 1) & (number >> (p - 1)));
}

void parse_address(struct line* lines, int lines_number, struct useful_parameter parameters) {

	int offset_temp;
	int index_temp;
	int tag_temp;
	int i;
	for (i = 0; i < lines_number; i++) {
		int address_temp = lines[i].address;
		//printf("address: %d\n", address_temp);
		offset_temp = bitExtracted(address_temp, parameters.offset, 1);
		index_temp = bitExtracted(address_temp, parameters.index, parameters.offset + 1);
		tag_temp = bitExtracted(address_temp, parameters.tag, parameters.offset + parameters.index + 1);

		lines[i].tag = tag_temp;
		lines[i].index = index_temp;
		lines[i].offset = offset_temp;
	}

}

void init_cache(struct cache* caches, struct useful_parameter parameters) {

	int i;
	int j;
	for (i = 0; i < parameters.sets_num; i++) {
		caches[i].index = -1;
		caches[i].entry = (struct entyr*)malloc(parameters.associativity * sizeof(struct entry));
		for (j = 0; j < parameters.associativity; j++) {
			caches[i].entry[j].tag = -1;
			caches[i].entry[j].dirty = 0;
		}
	}
}

void show_cache(struct cache* caches, struct useful_parameter parameters) {

	int empty_flag = 0;

	int i, j;
	for (i = 0; i < parameters.sets_num; i++) {
		if (caches[i].index != -1) {
			empty_flag = 1;
			printf("cache %d: index:%d \n", i, caches[i].index);
			for (j = 0; j < parameters.associativity; j++) {
				if (caches[i].entry[j].tag != -1) {
					printf("tag: %d ", caches[i].entry[j].tag);
					if (caches[i].entry[j].dirty == 0)
						printf(" clean ");
					else
						printf(" dirty ");
				}
				printf("\n");
			}
			
		}
	}

	if (empty_flag == 0) {
		printf("Empty cache!\n");
	}
}

void init_final_result(struct final_result* result) {

	result->hit = 0;
	result->miss = 0;
	result->mem_ref = 0;
	result->data_hit = 0;
	result->data_miss = 0;
	result->data_num = 0;
	result->data_ref = 0;
	result->ins_hit = 0;
	result->ins_miss = 0;
	result->ins_num = 0;
	result->ins_ref = 0;
}

int check_tag_capcity(struct entry* entry, struct useful_parameter parameters) {

	int pos = -1;
	int i;
	for ( i = 0; i < parameters.associativity; i++) {
		if (entry[i].tag == -1) {
			pos = i;
			break;
		}
	}

	return pos;
}

int check_tag_exist(struct entry* entry, int target, struct useful_parameter parameters) {

	int pos = -1;
	int i;
	for (i = 0; i < parameters.associativity; i++) {
		if (entry[i].tag == target)
			pos = i;
	}

	return pos;

}

struct entry lru_cache(struct entry* entry, int inserted_tag, struct useful_parameter parameters) {


	struct entry result;
	result.tag = -1;
	result.dirty = -1;
	result.pos = -1;
	int added_pos = check_tag_capcity(entry, parameters);

	//empty tag
	if (added_pos == 0) {
		//printf("empty!!!!!\n");
		entry[0].tag = inserted_tag;
		result.pos = 0;
		//printf("inseted tag: %d\n", inserted_tag);
	}
	//not full
	else if (added_pos != -1) {
		int pos = check_tag_exist(entry, inserted_tag, parameters);

		//new element added to not full tag
		if (pos == -1) {
			entry[added_pos].tag = inserted_tag;
			result.pos = added_pos;
		}

		//exsiting element added to not full tag
		else {

			struct entry* entry_temp = (struct entry*)malloc(parameters.associativity * sizeof(struct entry));
			int i;
			for (i = 0; i < parameters.associativity; i++) {
				entry_temp[i] = entry[i];
			}
			entry[added_pos - 1] = entry_temp[pos];
			for (i = pos + 1; i < added_pos; i++) {
				entry[i - 1] = entry_temp[i];
			}
			result.pos = added_pos - 1;

		}
	}

	//full
	else {

		int pos = check_tag_exist(entry, inserted_tag, parameters);
		//new elment added to full tag, evicted will happen
		if (pos == -1) {
			
			int i;
			struct entry* entry_temp = (struct entry*)malloc(parameters.associativity * sizeof(struct entry));
			for (i = 0; i < parameters.associativity; i++) {
				entry_temp[i] = entry[i];
			}

			entry[parameters.associativity - 1].tag = inserted_tag;
			entry[parameters.associativity - 1].dirty = 0;


			for (i = 1; i < parameters.associativity; i++) {
				entry[i - 1] = entry_temp[i];
			}

			result = entry_temp[0];
			result.pos = parameters.associativity-1;
		}
		//existing element added to full tag
		else {

			int i;
			struct entry* entry_temp = (struct entry*)malloc(parameters.associativity * sizeof(struct entry));
			for (i = 0; i < parameters.associativity; i++) {
				entry_temp[i] = entry[i];
			}

			entry[parameters.associativity - 1] = entry_temp[pos];

			for (i = pos; i < parameters.associativity - 1; i++) {
				entry[i] = entry_temp[i + 1];
			}
			result.pos = parameters.associativity - 1;

		}
	}

	return result;
}

struct cache_result search_cache(struct cache* caches, int target_index, int target_tag, struct useful_parameter parameters) {

	struct cache_result result;
	result.index_pos = -1;
	result.tag_pos = -1;

	int i;
	int j;
	for (i = 0; i < parameters.sets_num; i++) {
		if (caches[i].index == target_index) {
			for (j = 0; j < parameters.associativity; j++) {
				if (caches[i].entry[j].tag == target_tag) {
					result.index_pos = i;
					result.tag_pos = j;
				}
			}
		}
	}


	//printf("Searching: index: %d, tag: %d\n",target_index,target_tag);

	return result;


}


struct final_result write_through(struct line* lines_set, struct cache* caches, int lines_number, struct useful_parameter parameters) {

	struct final_result result;
	init_final_result(&result);

	int i;
	for (i = 0; i < lines_number; i++) {

		
		int target_index = lines_set[i].index;
		int target_tag = lines_set[i].tag;
		struct cache_result temp;
		temp = search_cache(caches, target_index, target_tag, parameters);



		//hit
		if (temp.index_pos != -1 && temp.tag_pos != -1) {
			//printf("Hitting\n");
			result.hit++;
			int found_index = temp.index_pos;
			int found_tag = caches[found_index].entry[temp.tag_pos].tag;

			lru_cache(caches[found_index].entry, found_tag, parameters);
			caches[temp.index_pos].index = found_index;
		

			//write hit
			if (lines_set[i].rw == 'W') {
				result.mem_ref++;

			}
			//read hit do nothing

		}

		//miss
		else {

			//printf("Missing\n");
			result.mem_ref++;
			result.miss++;

			//read miss
			if (lines_set[i].rw == 'R') {
				int index_add = lines_set[i].index;
				int tag_add = lines_set[i].tag;
				
				lru_cache(caches[index_add].entry, tag_add, parameters);
				caches[index_add].index = index_add;
				//caches[index_add].tag = tag_temp;
			}


		}

		//write miss do nothing to cache
		//show_cache(caches, parameters);
	}


	return result;
}

struct final_result write_back(struct line* lines_set, struct cache* caches, int lines_number, struct useful_parameter parameters) {

	struct final_result result;
	init_final_result(&result);

	int i;
	for (i = 0; i < lines_number; i++) {

		//printf("line: %d\n", i);
		int target_index = lines_set[i].index;
		int target_tag = lines_set[i].tag;
		struct cache_result temp;
		temp = search_cache(caches, target_index, target_tag, parameters);

		//hit
		if (temp.index_pos != -1 && temp.tag_pos != -1) {

			//printf("Hitting\n");
			result.hit++;
			int found_index = temp.index_pos;
			int found_tag = caches[found_index].entry[temp.tag_pos].tag;



			//write hit
			if (lines_set[i].rw == 'W') {
				//printf("writing hit, found_index: %d, tag_pos: %d \n", found_index, temp.tag_pos);
				caches[found_index].entry[temp.tag_pos].dirty = 1;
			}

			lru_cache(caches[found_index].entry, found_tag, parameters);
			caches[temp.index_pos].index = temp.index_pos;

		}
		//miss
		else {
			//printf("Missing\n");
			result.mem_ref++;
			result.miss++;


			//read miss
			if (lines_set[i].rw == 'R') {
				int index_add = lines_set[i].index;
				int tag_add = lines_set[i].tag;
				struct entry evicted;
				evicted = lru_cache(caches[index_add].entry, tag_add, parameters);
				caches[index_add].index = index_add;

				if (evicted.tag != -1 && evicted.pos != -1) {
					if (evicted.dirty == 1) {
						result.mem_ref++;
					}
				}





			}
			//write miss
			if (lines_set[i].rw == 'W') {
				int index_add = lines_set[i].index;
				int tag_add = lines_set[i].tag;
				
				struct entry evicted;
				evicted = lru_cache(caches[index_add].entry, tag_add, parameters);
				caches[index_add].index = index_add;

				if (evicted.tag != -1 && evicted.pos != -1) {
					if (evicted.dirty == 1) {
						result.mem_ref++;
					}
					//printf("writing miss, index add: %d, tag_pos: %d \n", index_add, evicted.pos);
					
				}
				caches[index_add].entry[evicted.pos].dirty = 1;
				
			}
		}

		//show_cache(caches, parameters);
	}


	return result;
}

struct final_result write_through_split(struct line* lines_set, struct cache* data_caches, struct cache* ins_caches, int lines_number, struct useful_parameter parameters) {

	struct final_result result;
	init_final_result(&result);

	int i;
	for (i = 0; i < lines_number; i++) {

		//instruction
		if (lines_set[i].id == 'I') {

			result.ins_num++;
			int target_index = lines_set[i].index;
			int target_tag = lines_set[i].tag;
			struct cache_result temp;
			temp = search_cache(ins_caches, target_index, target_tag, parameters);



			//hit
			if (temp.index_pos != -1 && temp.tag_pos != -1) {
				//printf("Hitting\n");
				result.ins_hit++;
				int found_index = temp.index_pos;
				int found_tag = ins_caches[found_index].entry[temp.tag_pos].tag;

				lru_cache(ins_caches[found_index].entry, found_tag, parameters);
				ins_caches[temp.index_pos].index = found_index;


				//write hit
				if (lines_set[i].rw == 'W') {
					result.ins_ref++;

				}
				//read hit do nothing

			}

			//miss
			else {

				//printf("Missing\n");
				result.ins_ref++;
				result.ins_miss++;

				//read miss
				if (lines_set[i].rw == 'R') {
					int index_add = lines_set[i].index;
					int tag_add = lines_set[i].tag;

					lru_cache(ins_caches[index_add].entry, tag_add, parameters);
					ins_caches[index_add].index = index_add;
				}


			}

			

		}
		//data
		else {
			result.data_num++;
			int target_index = lines_set[i].index;
			int target_tag = lines_set[i].tag;
			struct cache_result temp;
			temp = search_cache(data_caches, target_index, target_tag, parameters);



			//hit
			if (temp.index_pos != -1 && temp.tag_pos != -1) {
				//printf("Hitting\n");
				result.data_hit++;
				int found_index = temp.index_pos;
				int found_tag = data_caches[found_index].entry[temp.tag_pos].tag;

				lru_cache(data_caches[found_index].entry, found_tag, parameters);
				data_caches[temp.index_pos].index = found_index;


				//write hit
				if (lines_set[i].rw == 'W') {
					result.data_ref++;

				}
				//read hit do nothing

			}

			//miss
			else {

				//printf("Missing\n");
				result.data_ref++;
				result.data_miss++;

				//read miss
				if (lines_set[i].rw == 'R') {
					int index_add = lines_set[i].index;
					int tag_add = lines_set[i].tag;

					lru_cache(data_caches[index_add].entry, tag_add, parameters);
					data_caches[index_add].index = index_add;
					//caches[index_add].tag = tag_temp;
				}


			}
		}


	}


	return result;
}

struct final_result write_back_split(struct line* lines_set, struct cache* data_caches, struct cache* ins_caches, int lines_number, struct useful_parameter parameters) {
	struct final_result result;
	init_final_result(&result);

	int i;
	for ( i = 0; i < lines_number; i++) {
		//instruction
		if (lines_set[i].id == 'I') {
			result.ins_num++;
			int target_index = lines_set[i].index;
			int target_tag = lines_set[i].tag;
			struct cache_result temp;
			temp = search_cache(ins_caches, target_index, target_tag, parameters);

			//hit
			if (temp.index_pos != -1 && temp.tag_pos != -1) {

				//printf("Hitting\n");
				result.ins_hit++;
				int found_index = temp.index_pos;
				int found_tag = ins_caches[found_index].entry[temp.tag_pos].tag;



				//write hit
				if (lines_set[i].rw == 'W') {
					//printf("writing hit, found_index: %d, tag_pos: %d \n", found_index, temp.tag_pos);
					ins_caches[found_index].entry[temp.tag_pos].dirty = 1;
				}

				lru_cache(ins_caches[found_index].entry, found_tag, parameters);
				ins_caches[temp.index_pos].index = temp.index_pos;

			}
			//miss
			else {
				//printf("Missing\n");
				result.ins_ref++;
				result.ins_miss++;


				//read miss
				if (lines_set[i].rw == 'R') {
					int index_add = lines_set[i].index;
					int tag_add = lines_set[i].tag;
					struct entry evicted;
					evicted = lru_cache(ins_caches[index_add].entry, tag_add, parameters);
					ins_caches[index_add].index = index_add;

					if (evicted.tag != -1 && evicted.pos != -1) {
						if (evicted.dirty == 1) {
							result.ins_ref++;
						}
					}





				}
				//write miss
				if (lines_set[i].rw == 'W') {
					int index_add = lines_set[i].index;
					int tag_add = lines_set[i].tag;

					struct entry evicted;
					evicted = lru_cache(ins_caches[index_add].entry, tag_add, parameters);
					ins_caches[index_add].index = index_add;

					if (evicted.tag != -1 && evicted.pos != -1) {
						if (evicted.dirty == 1) {
							result.ins_ref++;
						}
						//printf("writing miss, index add: %d, tag_pos: %d \n", index_add, evicted.pos);

					}
					ins_caches[index_add].entry[evicted.pos].dirty = 1;

				}
			}
			

		}

		//data
		else {
			result.data_num++;
			int target_index = lines_set[i].index;
			int target_tag = lines_set[i].tag;
			struct cache_result temp;
			temp = search_cache(data_caches, target_index, target_tag, parameters);

			//hit
			if (temp.index_pos != -1 && temp.tag_pos != -1) {

				//printf("Hitting\n");
				result.data_hit++;
				int found_index = temp.index_pos;
				int found_tag = data_caches[found_index].entry[temp.tag_pos].tag;



				//write hit
				if (lines_set[i].rw == 'W') {
					//printf("writing hit, found_index: %d, tag_pos: %d \n", found_index, temp.tag_pos);
					data_caches[found_index].entry[temp.tag_pos].dirty = 1;
				}

				lru_cache(data_caches[found_index].entry, found_tag, parameters);
				data_caches[temp.index_pos].index = temp.index_pos;

			}
			//miss
			else {
				//printf("Missing\n");
				result.data_ref++;
				result.data_miss++;


				//read miss
				if (lines_set[i].rw == 'R') {
					int index_add = lines_set[i].index;
					int tag_add = lines_set[i].tag;
					struct entry evicted;
					evicted = lru_cache(data_caches[index_add].entry, tag_add, parameters);
					data_caches[index_add].index = index_add;

					if (evicted.tag != -1 && evicted.pos != -1) {
						if (evicted.dirty == 1) {
							result.data_ref++;
						}
					}





				}
				//write miss
				if (lines_set[i].rw == 'W') {
					int index_add = lines_set[i].index;
					int tag_add = lines_set[i].tag;

					struct entry evicted;
					evicted = lru_cache(data_caches[index_add].entry, tag_add, parameters);
					data_caches[index_add].index = index_add;

					if (evicted.tag != -1 && evicted.pos != -1) {
						if (evicted.dirty == 1) {
							result.data_ref++;
						}
						//printf("writing miss, index add: %d, tag_pos: %d \n", index_add, evicted.pos);

					}
					data_caches[index_add].entry[evicted.pos].dirty = 1;

				}
			}

			
		}

			



	}



	return result;
}
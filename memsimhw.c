// 2020 / 운영체제 / hw3 / B511006 / 고재욱 
// Virual Memory Simulator Homework
// One-level page table system with FIFO and LRU
// Two-level page table system with LRU
// Inverted page table with a hashing system 
// Submission Year: 2020
// Student Name: 고재욱
// Student Number: B511006
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define PAGESIZEBITS 12			// page size = 4Kbytes
#define VIRTUALADDRBITS 32		// virtual address space size = 4Gbytes
int numProcess, nFrame;
struct pageTableEntry{
	int procid;
	int vpn;
	int secondLevel;
	int time;
	int len;
	struct pageTableEntry *next;
	struct pageTableEntry *prev;
} pageTableQueue, *pageTable;


struct procEntry {
	char *traceName;			// the memory trace name
	int pid;					// process (trace) id
	int ntraces;				// the number of memory traces
	int num2ndLevelPageTable;	// The 2nd level page created(allocated);
	int numIHTConflictAccess; 	// The number of Inverted Hash Table Conflict Accesses
	int numIHTNULLAccess;		// The number of Empty Inverted Hash Table Accesses
	int numIHTNonNULLAccess;		// The number of Non Empty Inverted Hash Table Accesses
	int numPageFault;			// The number of page faults
	int numPageHit;				// The number of page hits
	int *firstLevelPageTable;
	FILE *tracefp;
} *procTable;

struct hashTableEntry{
	int pid;
	int vpn;
//	int time;
	struct hashTableEntry *next;
	struct hashTableEntry *prev;
} *hashTable;

int count;
void createNode(int vpn, int pid, int type){
 	procTable[pid].ntraces++;
 	int find = 0;
	if(pageTableQueue.len == 0){
		pageTableQueue.next = &pageTable[count];
		pageTableQueue.len++;
		pageTable[count].vpn = vpn;
		pageTable[count].procid = pid;
		pageTable[count].next = NULL;
		pageTable[count].prev = &pageTableQueue;
		count++;
		procTable[0].numPageFault++;
	//	printf("노드생성\n");
	}
	else{
		struct pageTableEntry *first = &pageTableQueue;
		first = first -> next;
		struct pageTableEntry *tmp = &pageTableQueue;
	/*	struct pageTableEntry *temp = &pageTableQueue;
		while(temp->next !=NULL){
			temp = temp -> next;
			printf("vpn %x ->",temp->vpn);
		}*/
		while(tmp->next != NULL){
			tmp = tmp-> next;
			if(tmp->procid == pid && tmp->vpn == vpn){	// 연결 리스트 안에 있으면
				if(type == 1){	// LRU일때 노드를 맨앞으로 이동
					if(pageTableQueue.len > 1 && pageTableQueue.next != tmp){
						if(tmp -> next == NULL){
							tmp->prev->next = NULL;
						}
						else {
							tmp->prev->next = tmp->next;
							tmp->next->prev = tmp->prev;
						}
						tmp->prev = &pageTableQueue;
						tmp->next = pageTableQueue.next;
						pageTableQueue.next = tmp;
						tmp->next->prev = tmp;
					}
				}
				procTable[pid].numPageHit++;
				find = 1;
			//	printf("PageHIT ->");
				break;
			}
		}
		if(find != 1){									// 연결 리스트 안에 없으면
			if(pageTableQueue.len == nFrame){			// pageTable이 꽉 차면
				if(type == 0){							// FIFO 맨 앞에거 삭제하고 맨뒤에 붙여놓기
					if(nFrame == 1){
						tmp->vpn = vpn;
						tmp->procid = pid;
					}
					else{
						first->vpn = vpn;
						first->procid = pid;
						pageTableQueue.next = first->next;
						first->next->prev = &pageTableQueue;
						first->prev = tmp;
						tmp->next = first;
						first->next = NULL;
					}
				}
				else{				//LRU 마지막꺼를 삭제 후 맨 앞에 노드 생성
					if(nFrame == 1){
					}
					else{
						tmp->prev->next = NULL;
						tmp->prev = &pageTableQueue;
						tmp->next = first;
						pageTableQueue.next = tmp;
						first->prev = tmp;
					}
					tmp->vpn = vpn;
					tmp->procid = pid;
				}
				procTable[pid].numPageFault++;
			//	printf("%d nFrame 꽉참\n", nFrame);
			}
			else{	
				if(type == 0){							// pageTable이 남아있으면,,,, FIFO 맨 뒤에 노드 생성
					tmp->next = &pageTable[count];
					pageTable[count].next = NULL;
					pageTable[count].prev = tmp;
				}
				else{		//LRU 맨 앞에 노드 생성
					pageTable[count].next = pageTableQueue.next;
					pageTable[count].prev = &pageTableQueue;
					pageTableQueue.next = &pageTable[count];
					first->prev = &pageTable[count];
				}
				pageTable[count].vpn = vpn;
				pageTable[count].procid = pid;
				count++;
				pageTableQueue.len++;
				procTable[pid].numPageFault++;
			}
//			printf("pageFault - >");
		}
	}
}

void oneLevelVMSim(int type) {
	int i;
	unsigned addr;
	char rw;
	int stop = 0;
	int vpn;	
	FILE *file = procTable[0].tracefp;
	while(EOF!=fscanf(file, "%x %c", &addr, &rw)){
		vpn = addr >> 12;
		createNode(vpn, 0, type);
//		printf("procID %d %x\n", procTable[i].pid, addr);
		for(i = 1 ; i < numProcess; i++){
			file = procTable[i].tracefp;
			if(EOF!=fscanf(file, "%x %c", &addr, &rw)){
				vpn = addr >> 12;
				createNode(vpn, i, type);
//				printf("procID %d %x\n", procTable[i].pid, addr);
			}
			else{stop = 1; break;}
		}
		if(stop == 1) break;
		file = procTable[0].tracefp;
	}
	
	for(i=0; i < numProcess; i++) {
		printf("**** %s *****\n",procTable[i].traceName);
		printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
		printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
		printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
		assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
	}
}
void createNode2(int firstLevel, int secondLevel, int pid){
	procTable[pid].ntraces++;
	int find = 0;
//	printf("%d ntraces\n", procTable[pid].ntraces);
	if(pageTableQueue.len == 0){
		pageTableQueue.len++;
		procTable[pid].numPageFault++;
		procTable[pid].num2ndLevelPageTable++;
		//큐만들기
		pageTableQueue.next = &pageTable[count];
		pageTable[count].next = NULL;
		pageTable[count].prev = &pageTableQueue;
		pageTable[count].procid = pid;
		pageTable[count].vpn = firstLevel;
		pageTable[count].secondLevel = secondLevel;
		count++;
		//first second pagetable 입력하기
		procTable[pid].firstLevelPageTable[firstLevel] = 1;
//		printf("PageFault\n");
	}
	else{
		struct pageTableEntry *tmp = &pageTableQueue;
		while(tmp->next != NULL){
			tmp=tmp->next;
			if(tmp->procid == pid && tmp->vpn == firstLevel && tmp->secondLevel == secondLevel){
				procTable[pid].numPageHit++;
				//						printf("PageHit\n");
				if(pageTableQueue.len>1){
					if(tmp->next == NULL)
						tmp->prev->next = NULL;
					else{
						tmp->prev->next = tmp->next;
						tmp->next->prev = tmp->prev;
					}
					tmp->prev = &pageTableQueue;
					tmp->next = pageTableQueue.next;
					pageTableQueue.next = tmp;
					tmp->next->prev = tmp;
				}
				find = 1;
				break;
			}
		}
		if(find != 1){
			if(pageTableQueue.len == nFrame){
				if(nFrame != 1){
					tmp->prev->next = NULL;
					tmp->next = pageTableQueue.next;
					tmp->prev = &pageTableQueue;
					tmp->next->prev = tmp;
					pageTableQueue.next = tmp;
				}
				tmp->vpn = firstLevel;
				tmp->procid = pid;
				tmp->secondLevel = secondLevel;
			}
			else{
				pageTable[count].next = pageTableQueue.next;
				pageTable[count].prev = &pageTableQueue;
				pageTableQueue.next->prev = &pageTable[count];
				pageTableQueue.next = &pageTable[count];
				pageTable[count].vpn = firstLevel;
				pageTable[count].secondLevel = secondLevel;
				pageTable[count].procid = pid;
				pageTableQueue.len++;
				count++;
			}
			if(procTable[pid].firstLevelPageTable[firstLevel] == 0){
				procTable[pid].firstLevelPageTable[firstLevel] = 1;
				procTable[pid].num2ndLevelPageTable++;
			}
			procTable[pid].numPageFault++;
//			printf("PageFault frame에 없었음\n");
		}
	}
}
void twoLevelVMSim(int firstLevelBits) {
	int i;
	unsigned addr;
	char rw;
	int stop = 0;
	int firstLevel, secondLevel;
	FILE *file = procTable[0].tracefp;
	while(EOF!=fscanf(file, "%x %c", &addr, &rw)){
		firstLevel = addr >> 32-firstLevelBits;
		secondLevel = (addr << firstLevelBits) >> firstLevelBits+12;
//		printf("procID %d firstLevel %x SecondLevel %x\n", procTable[0].pid, firstLevel, secondLevel);
		createNode2(firstLevel,secondLevel,0);
//		printf("\n\n");
		for(i = 1; i < numProcess; i++){
			file = procTable[i].tracefp;
			if(EOF!=fscanf(file, "%x %c", &addr, &rw)){
				firstLevel = addr >> 32-firstLevelBits;
				secondLevel = (addr << firstLevelBits) >> firstLevelBits+12;
				createNode2(firstLevel, secondLevel, i);
//				printf("procID %d %x\n", procTable[i].pid, addr);

			}
			else{stop = 1; break;}
		}
		if(stop==1) break;
		file = procTable[0].tracefp;
	}
	for(i=0; i < numProcess; i++) {
		printf("**** %s *****\n",procTable[i].traceName);
		printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
		printf("Proc %d Num of second level page tables allocated %d\n",i,procTable[i].num2ndLevelPageTable);
		printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
		printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
		assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
	}
}
int countNode; // 만들어진 노드의 갯수
int time;
void createNode3(int vpn, int pid){
	procTable[pid].ntraces++;
//	printf("%d ntrace   ", procTable[pid].ntraces);
	time++;
	int index = (vpn + pid) % nFrame;
//	printf("index %d\n", index);
	int find =0;
	if(countNode == 0){
		pageTable[countNode].procid = pid;
		pageTable[countNode].vpn = vpn;
		pageTable[countNode].time = time;
		countNode++;
		procTable[pid].numIHTNULLAccess++;
		struct hashTableEntry *tmp = (struct hashTableEntry*)malloc(sizeof(struct hashTableEntry));
		tmp->pid = pid;
		tmp->vpn = vpn;
		tmp->next = NULL;
		tmp->prev = hashTable[index].next;
		hashTable[index].next = tmp;
		procTable[pid].numPageFault++;
//		printf("PageFault Createnode\n");
	}
	else{
		struct hashTableEntry *tmp = &hashTable[index];
		struct hashTableEntry *first = &hashTable[index];
		first = first -> next;
		if(tmp->next == NULL) procTable[pid].numIHTNULLAccess++;
		else procTable[pid].numIHTNonNULLAccess++;
		while(tmp->next !=NULL){
			tmp = tmp->next;
			procTable[pid].numIHTConflictAccess++;
//			printf("conflictAccess++\n");
			if(tmp->pid == pid && tmp->vpn == vpn){
				procTable[pid].numPageHit++;
				find = 1;
//				printf("PageHit\n");
				int i;
				for(i = 0; i <nFrame;i++){
					if(pageTable[i].procid == pid && pageTable[i].vpn == vpn){
						pageTable[i].time = time;
						break;
					}
				}
				break;
			}
		}
		if(find != 1){
			struct hashTableEntry *node =(struct hashTableEntry *) malloc(sizeof(struct hashTableEntry));
			if(countNode == nFrame){
				if(nFrame == 1){
					pageTable[0].procid = pid;
					pageTable[0].vpn = vpn;
					pageTable[0].time = time;
					tmp->pid = pid;
					tmp->vpn = vpn;
				}
				else{				//LRU를 찾아야됨
					int minTime = time;
					int i, j;
					for(i = 0; i < nFrame; i++){
						if(pageTable[i].time < minTime){
							minTime = pageTable[i].time;
							j = i;
						}
					}
					int LRUvpn = pageTable[j].vpn;
					int LRUpid = pageTable[j].procid;
					int findIndex = (LRUvpn+LRUpid)%nFrame;
					tmp = &hashTable[findIndex];
					struct hashTableEntry *before = &hashTable[findIndex];
					while(tmp->next !=NULL){
						tmp = tmp->next;
						if(tmp->pid == LRUpid && tmp->vpn == LRUvpn)
							break;
						before = before->next;
					}
				//	printf("tmp->pid %d temp->vpn %x LRUpid%d LRUvpn%x\n", tmp->pid, tmp->vpn,LRUpid, LRUvpn);
					if(tmp->next ==NULL){
						before->next = NULL;
						tmp->prev = NULL;
					}else{
						tmp->prev->next = tmp->next;
						tmp->next->prev = tmp->prev;
						tmp->prev=NULL;
						tmp->next=NULL;
					}
					pageTable[j].time = time;
					pageTable[j].vpn = vpn;
					pageTable[j].procid = pid;
					node->pid = pid;
					node->vpn = vpn;
					node->next = hashTable[index].next;
					node->prev = &hashTable[index];
					hashTable[index].next = node;
					if(node->next !=NULL){
						node->next->prev = node;
					}
				}
			}
			else{
				pageTable[countNode].procid=pid;
				pageTable[countNode].vpn = vpn;
				pageTable[countNode].time = time;
				node->pid = pid;
				node->vpn = vpn;
				hashTable[index].next = node;
				node->next = first;
				node->prev = &hashTable[index];
				if(first !=NULL)
					first->prev=node;
				countNode++;
			}
			procTable[pid].numPageFault++;
//			printf("PageFault\n");
		}
	}
}

void invertedPageVMSim() {
	int i;
	unsigned addr;
	char rw;
	int stop = 0;
	int vpn;
	FILE *file = procTable[0].tracefp;
	while(EOF != fscanf(file, "%x %c", &addr, &rw)){
		vpn = addr >> 12;
//		printf("procID %d %x\n", procTable[i].pid, addr);
		createNode3(vpn, 0);
//		printf("\n");
		for(i = 1; i <numProcess; i++){
			file = procTable[i].tracefp;
			if(EOF!=fscanf(file, "%x %c", &addr, &rw)){
				vpn = addr >> 12;
//				printf("procID %d %x \n",procTable[i].pid,addr);
				createNode3(vpn,i);
//				printf("\n");
			}
			else{stop = 1; break;}
		}
		if(stop==1) break;
		file = procTable[0].tracefp;
	}
	for(i=0; i < numProcess; i++) {
		printf("**** %s *****\n",procTable[i].traceName);
		printf("Proc %d Num of traces %d\n",i,procTable[i].ntraces);
		printf("Proc %d Num of Inverted Hash Table Access Conflicts %d\n",i,procTable[i].numIHTConflictAccess);
		printf("Proc %d Num of Empty Inverted Hash Table Access %d\n",i,procTable[i].numIHTNULLAccess);
		printf("Proc %d Num of Non-Empty Inverted Hash Table Access %d\n",i,procTable[i].numIHTNonNULLAccess);
		printf("Proc %d Num of Page Faults %d\n",i,procTable[i].numPageFault);
		printf("Proc %d Num of Page Hit %d\n",i,procTable[i].numPageHit);
		assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
		assert(procTable[i].numIHTNULLAccess + procTable[i].numIHTNonNULLAccess == procTable[i].ntraces);
	}
}
int main(int argc, char *argv[]) {
	int i,c, simType, firstLevelBits, phyMemSizeBits;
	int optind;
	nFrame = 2;
	if(argc < 4){
		printf("%s : memsim simType firstLevelBits PhysicalMemorySizeBits TraceFileNames......\n", argv[0]);
		exit(1);
	}
	
	simType = atoi(argv[1]);
	firstLevelBits = atoi(argv[2]);
	phyMemSizeBits = atoi(argv[3]);
	numProcess = argc - 4;
	if(phyMemSizeBits == 12)
		nFrame = 1;
	else{
		for(i = 1; i < phyMemSizeBits - 12; i++){
			nFrame *= 2;	
		}
	}
//	printf("SIMULATION PARAMETERS : simType %d firstLevelBits %d PhysicalMemorySizeBits %d numProcess %d\n", simType, firstLevelBits, phyMemSizeBits, numProcess);

	procTable = (struct procEntry *)malloc(sizeof(struct procEntry) * numProcess);

	pageTable = (struct pageTableEntry *)malloc(sizeof(struct pageTableEntry) * nFrame);
	pageTableQueue.next = pageTableQueue.prev = &pageTableQueue;
	pageTableQueue.len = 0;
	pageTableQueue.procid = -1;

	hashTable = (struct hashTableEntry *)malloc(sizeof(struct hashTableEntry) * nFrame);

	int firstLevel = 1, secondLevel = 1;
	for(i = 1; i <= firstLevelBits; i++)
		firstLevel *= 2;
	for(i = 1; i <= 20 - firstLevelBits; i++)
		secondLevel *= 2;

	// initialize procTable for Memory Simulations
	for(i = 0; i < numProcess; i++) {
		procTable[i].pid = i;
		procTable[i].traceName = argv[i+4];
		// opening a tracefile for the process
		printf("process %d opening %s\n",i,argv[i + 4]);
		procTable[i].tracefp = fopen(argv[i + 4],"r");
		if (procTable[i].tracefp == NULL) {
			printf("ERROR: can't open %s file; exiting...\n",argv[i+optind+3]);
			exit(1);
		}
		procTable[i].firstLevelPageTable = (int *)malloc(sizeof(int) * firstLevel);
	}
	
	printf("Num of Frames %d Physical Memory Size %ld bytes\n",nFrame, (1L<<phyMemSizeBits));
	
	if (simType == 0) {
		printf("=============================================================\n");
		printf("The One-Level Page Table with FIFO Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		oneLevelVMSim(simType);
	}
	
	if (simType == 1) {
		printf("=============================================================\n");
		printf("The One-Level Page Table with LRU Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		oneLevelVMSim(simType);
	}

	if (simType == 2) {
		printf("=============================================================\n");
		printf("The Two-Level Page Table Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		twoLevelVMSim(firstLevelBits);
	}
	
	if (simType == 3) {
		printf("=============================================================\n");
		printf("The Inverted Page Table Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		invertedPageVMSim();
	}

	return(0);
}

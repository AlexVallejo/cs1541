/**************************************************************
 CS/COE 1541
   just compile with gcc -o superscaler superscaler.c
   and execute using
   ./superscaler  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr 0
***************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "trace_item.h"

#define TRACE_BUFSIZE 1024*1024
#define NUM_BUFFERS 4
#define BRANCH_PREDICTION_TABLE_SIZE 128
#define INSTRUCTION_BUFFER_LENGTH 2

static FILE *trace_fd;
static int trace_buf_ptr;
static int trace_buf_end;
static struct trace_item *trace_buf;

unsigned int cycle_number = 0;

int branch_prediction_table[BRANCH_PREDICTION_TABLE_SIZE];       // Hastable used for branch prediction (if taken or not)
struct trace_item instruction_buffer[INSTRUCTION_BUFFER_LENGTH]; // Instruction Buffer
struct trace_item buffer_ALU[NUM_BUFFERS];                       // Pipeline Buffers
struct trace_item buffer_LS[NUM_BUFFERS];                        // Pipeline Buffers
struct trace_item *tr_entry;                                     // Temporary holding entry

int read_next_inst = 1;       // Boolean used to pause instruction reading when
                              // there is a load-use conflict

int is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

uint32_t my_ntohl(uint32_t x)
{
  u_char *s = (u_char *)&x;
  return (uint32_t)(s[3] << 24 | s[2] << 16 | s[1] << 8 | s[0]);
}

void trace_init()
{
  trace_buf = malloc(sizeof(struct trace_item) * TRACE_BUFSIZE);

  if (!trace_buf) {
    fprintf(stdout, "** trace_buf not allocated\n");
    exit(-1);
  }

  trace_buf_ptr = 0;
  trace_buf_end = 0;
}

void trace_uninit()
{
  free(trace_buf);
  fclose(trace_fd);
}

int trace_get_item(struct trace_item **item)
{
  int n_items;

  if (trace_buf_ptr == trace_buf_end) { /* if no more unprocessed items in the trace buffer, get new data */
    n_items = fread(trace_buf, sizeof(struct trace_item), TRACE_BUFSIZE, trace_fd);
    if (!n_items) return 0;       /* if no more items in the file, we are done */

    trace_buf_ptr = 0;
    trace_buf_end = n_items;      /* n_items were read and placed in trace buffer */
  }

  *item = &trace_buf[trace_buf_ptr];  /* read a new trace item for processing */
  trace_buf_ptr++;

  if (is_big_endian()) {
    (*item)->PC = my_ntohl((*item)->PC);
    (*item)->Addr = my_ntohl((*item)->Addr);
  }

  return 1;
}

int print_buffers_LS(){

  printf("\n---------------------CYCLE #%d-------------------\n",cycle_number);

  for (int i = 0; i < NUM_BUFFERS; i++){
    tr_entry = &buffer[i];

    switch (i) {
      case 0:
        printf("%-6s => ", "IF/ID");
        break;
      case 1:
        printf("%-6s => ", "ID/EX");
        break;
      case 2:
        printf("%-6s => ","EX/MEM");
        break;
      case 3:
        printf("%-6s => ","MEM/WB");
        break;
    }

    switch (tr_entry_LS->type) {
      case ti_NOP:
        if(tr_entry_LS->Addr == 1)
          printf("SQUASHED\n");
        else
          printf("NOP\n");
        break;
      case ti_RTYPE:
        printf("RTYPE: ") ;
        printf("(PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d)\n", tr_entry_LS->PC, tr_entry_LS->sReg_a, tr_entry_LS->sReg_b, tr_entry_LS->dReg);
        break;
      case ti_ITYPE:
        printf("ITYPE:") ;
        printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry_LS->PC, tr_entry_LS->sReg_a, tr_entry_LS->dReg, tr_entry_LS->Addr);
        break;
      case ti_LOAD:
        printf("LOAD:") ;
        printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry_LS->PC, tr_entry_LS->sReg_a, tr_entry_LS->dReg, tr_entry_LS->Addr);
        break;
      case ti_STORE:
        printf("STORE:") ;
        printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry_LS->PC, tr_entry_LS->sReg_a, tr_entry_LS->sReg_b, tr_entry_LS->Addr);
        break;
      case ti_BRANCH:
        printf("BRANCH:") ;
        printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry_LS->PC, tr_entry_LS->sReg_a, tr_entry_LS->sReg_b, tr_entry_LS->Addr);
        break;
      case ti_JTYPE:
        printf("JTYPE:") ;
        printf(" (PC: %x)(addr: %x)\n", tr_entry_LS->PC,tr_entry_LS->Addr);
        break;
      case ti_SPECIAL:
        printf("SPECIAL\n") ;
        break;
      case ti_JRTYPE:
        printf("JRTYPE:") ;
        printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry_LS->PC, tr_entry_LS->dReg, tr_entry_LS->Addr);
        break;
      } // END switch
    }// END for (int i = 0; i < NUM_BUFFERS; i++)
  return 1;
}

int print_buffers_ALU(){

  printf("\n---------------------CYCLE #%d-------------------\n",cycle_number);

  for (int i = 0; i < NUM_BUFFERS; i++){
    tr_entry_ALU = &buffer[i];

    switch (i) {
      case 0:
        printf("%-6s => ", "IF/ID");
        break;
      case 1:
        printf("%-6s => ", "ID/EX");
        break;
      case 2:
        printf("%-6s => ","EX/MEM");
        break;
      case 3:
        printf("%-6s => ","MEM/WB");
        break;
    }

    switch (tr_entry_ALU->type) {
      case ti_NOP:
        if(tr_entry_ALU->Addr == 1)
          printf("SQUASHED\n");
        else
          printf("NOP\n");
        break;
      case ti_RTYPE:
        printf("RTYPE: ") ;
        printf("(PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d)\n", tr_entry_ALU->PC, tr_entry_ALU->sReg_a, tr_entry_ALU->sReg_b, tr_entry_ALU->dReg);
        break;
      case ti_ITYPE:
        printf("ITYPE:") ;
        printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry_ALU->PC, tr_entry_ALU->sReg_a, tr_entry_ALU->dReg, tr_entry_ALU->Addr);
        break;
      case ti_LOAD:
        printf("LOAD:") ;
        printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry_ALU->PC, tr_entry_ALU->sReg_a, tr_entry_ALU->dReg, tr_entry_ALU->Addr);
        break;
      case ti_STORE:
        printf("STORE:") ;
        printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry_ALU->PC, tr_entry_ALU->sReg_a, tr_entry_ALU->sReg_b, tr_entry_ALU->Addr);
        break;
      case ti_BRANCH:
        printf("BRANCH:") ;
        printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry_ALU->PC, tr_entry_ALU->sReg_a, tr_entry_ALU->sReg_b, tr_entry_ALU->Addr);
        break;
      case ti_JTYPE:
        printf("JTYPE:") ;
        printf(" (PC: %x)(addr: %x)\n", tr_entry_ALU->PC,tr_entry_ALU->Addr);
        break;
      case ti_SPECIAL:
        printf("SPECIAL\n") ;
        break;
      case ti_JRTYPE:
        printf("JRTYPE:") ;
        printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry_ALU->PC, tr_entry_ALU->dReg, tr_entry_ALU->Addr);
        break;
      } // END switch
    }// END for (int i = 0; i < NUM_BUFFERS; i++)
  return 1;
}// END print_buffer(int trace_view_on)

int insert_stall(int buffer_select){

  // Init a new no-op to insert in the pipeline
  struct trace_item no_op;
  no_op.type = 0;
  no_op.sReg_a = 0;
  no_op.sReg_b = 0;
  no_op.dReg = 0;
  no_op.PC = 0;
  no_op.Addr = 0;

  printf("\n********************* DATA HAZARD DETECTED *************************\n");
  if(buffer_select == 0)
    buffer_ALU[0] = no_op;
  else
    buffer_LS[0] = no_op;
  return 1;
}

int insert_squashed(){

  // Init a new squashed instruction to insert in the pipeline
  struct trace_item squashed_inst;
  squashed_inst.type = 0;
  squashed_inst.sReg_a = 0;
  squashed_inst.sReg_b = 0;
  squashed_inst.dReg = 0;
  squashed_inst.PC = 0;
  squashed_inst.Addr = 1;

  printf("\n**************** CONTROL HAZARD DETECTED *******************\n");

  buffer_ALU[0] = squashed_inst;
  buffer_LS[0] = squashed_inst;
  return 1;
}

int control_hazard(){
  // RETURN true IF the inst in the buffer is a branch and next instruction
  // executed is not next sequentially in the code
  if (buffer_ALU[0].type == 5 && instruction_buffer[0]->PC != (buffer_ALU[0].PC + 4))
    return 1;
  if (buffer_ALU[0].type == 5 && instruction_buffer[1]->PC != (buffer_ALU[0].PC + 4))
    return 1;
  return 0;
}

/*
 * Initialize every index in the branch prediction table to -1
 */
int init_branch_prediction_table(){
  for (int i = 0; i < BRANCH_PREDICTION_TABLE_SIZE; i++)
    branch_prediction_table[i] = -1;
  return 0;
}

/*
 * Every value in the branch prediction table is initialized to zero.
 * If there is no data on the branch, it will return a value of -1 (not taken).
 * If there is data on the branch, it will return the last state of the branch
 *  not taken (-1) anything else is the Addr is previously jumped to
 */
int branch_predict(int pc){
  int index = pc % 128;
  return branch_prediction_table[index];
}

/*
 * Updates the address with the address it jumped to
 */
int update_branch(int pc, int addr) {
  int index = pc % 128;

  branch_prediction_table[index] = addr;
  return 0;
}

int load_use_dependance(int index){

  if (buffer_LS[0].type != 3)
    return 0; // There is no data dependance

  //instruction_buffer[index] = R-Type instruction
  if (instruction_buffer[index].type == 1){
    if (   buffer_LS[0].dReg == instruction_buffer[index].sReg_a
        || buffer_LS[0].dReg == instruction_buffer[index].sReg_b)
        return 1; // There is a data dependance
  }

  //instruction_buffer[index] = I-Type Instruction
  if (instruction_buffer[index].type == 2) {
    if (buffer_LS[0].dReg == instruction_buffer[index].sReg_a)
        return 1; // There is a data dependance
  }

  //instruction_buffer[index] = Load Instruction
  if (instruction_buffer[index].type == 3) {
    if (buffer_LS[0].dReg == instruction_buffer[index].sReg_a)
        return 1; // There is a data dependance
  }

  //instruction_buffer[index] = Store Instruction
  if (instruction_buffer[index].type == 4) {
    if (buffer_LS[0].dReg == instruction_buffer[index].sReg_a)
        return 1; // There is a data dependance
  }

  //instruction_buffer[index] = Branch Instruction
  if (instruction_buffer[index].type == 5) {
    if (buffer_LS[0].dReg == instruction_buffer[index].sReg_a)
        return 1; // There is a data dependance
  }

  //instruction_buffer[index] = Branch Instruction
  if (instruction_buffer[index].type == 8) {
    if (buffer_LS[0].dReg == instruction_buffer[index].sReg_a)
        return 1; // There is a data dependance
  }

  return 0;
}

/*
 * Detect if the two instructions in the instruction buffer have data
 * dependancies b/t themselves
 */

int inst_buffer_inter_dependant(){
  if(!((  instruction_buffer[0].type == 1
       || instruction_buffer[0].type == 2
       || instruction_buffer[0].type == 5
       || instruction_buffer[0].type == 6
       || instruction_buffer[0].type == 7
       || instruction_buffer[0].type == 8)
       && (instruction_buffer[1].type == 3
       ||  instruction_buffer[1].type == 4))){
    return 1;
  }

  if(!((  instruction_buffer[1].type == 1
       || instruction_buffer[1].type == 2
       || instruction_buffer[1].type == 5
       || instruction_buffer[1].type == 6
       || instruction_buffer[1].type == 7
       || instruction_buffer[1].type == 8)
       && (instruction_buffer[0].type == 3
       ||  instruction_buffer[0].type == 4))){
    return 1;
  }

  if(instruction_buffer[0].type == 5){
    return 1;
  }

  //instruction_buffer[0] is a ALU instruction
  if (    instruction_buffer[0].type == 1
       || instruction_buffer[0].type == 2
       || instruction_buffer[0].type == 5
       || instruction_buffer[0].type == 6
       || instruction_buffer[0].type == 7
       || instruction_buffer[0].type == 8){

    if(instruction_buffer[1].type == 3){

      //instruction_buffer[index] = R-Type instruction
      if (instruction_buffer[0].type == 1){
        if (   instruction_buffer[1].dReg == instruction_buffer[0].sReg_a
            || instruction_buffer[1].dReg == instruction_buffer[0].sReg_b)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = I-Type Instruction
      if (instruction_buffer[0].type == 2) {
        if (instruction_buffer[1].dReg == instruction_buffer[0].sReg_a)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = Load Instruction
      if (instruction_buffer[0].type == 3) {
        if (instruction_buffer[1].dReg == instruction_buffer[0].sReg_a)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = Store Instruction
      if (instruction_buffer[0].type == 4) {
        if (instruction_buffer[1].dReg == instruction_buffer[0].sReg_a)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = Branch Instruction
      if (instruction_buffer[0].type == 5) {
        if (instruction_buffer[1].dReg == instruction_buffer[0].sReg_a)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = Branch Instruction
      if (instruction_buffer[0].type == 8) {
        if (instruction_buffer[1].dReg == instruction_buffer[0].sReg_a)
            return 1; // There is a data dependance
      }
    }
  }

  //instruction_buffer[1] is a ALU instruction
  else {

      if(instruction_buffer[0].type == 3){

      //instruction_buffer[index] = R-Type instruction
      if (instruction_buffer[1].type == 1){
        if (   instruction_buffer[0].dReg == instruction_buffer[1].sReg_a
            || instruction_buffer[0].dReg == instruction_buffer[1].sReg_b)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = I-Type Instruction
      if (instruction_buffer[1].type == 2) {
        if (instruction_buffer[0].dReg == instruction_buffer[1].sReg_a)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = Load Instruction
      if (instruction_buffer[1].type == 3) {
        if (instruction_buffer[0].dReg == instruction_buffer[1].sReg_a)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = Store Instruction
      if (instruction_buffer[1].type == 4) {
        if (instruction_buffer[0].dReg == instruction_buffer[1].sReg_a)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = Branch Instruction
      if (instruction_buffer[1].type == 5) {
        if (instruction_buffer[0].dReg == instruction_buffer[1].sReg_a)
            return 1; // There is a data dependance
      }

      //instruction_buffer[index] = Branch Instruction
      if (instruction_buffer[1].type == 8) {
        if (instruction_buffer[0].dReg == instruction_buffer[1].sReg_a)
            return 1; // There is a data dependance
      }
    }
  }

  return 0;
}

int main(int argc, char **argv) {
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;         // Set print's for each cycle off for default
  int branch_ops = 0;            // Control to allow for two no-ops to follow an
                                 // incorrect branch prediction
  int prediction_method = 0;     // boolean to use or not use the 1-bit branch
                                 // predictor


  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch_1 - any character> <switch_2 - any character> \n");
    fprintf(stdout, "\n(switch_1) to turn on or off the 1-bit branch predictor.\n\n");
    fprintf(stdout, "\n(switch_2) to turn on or off individual item view.\n\n");
    exit(0);
  }

  // Parse command line arguments
  trace_file_name = argv[1];
  if (argc == 3)
    trace_view_on = atoi(argv[2]);

  else if (argc == 4){
    prediction_method = atoi(argv[2]);
    trace_view_on = atoi(argv[3]);
  }

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();

  //TODO the first and last instructions of the program are not displayed correctly
  // it should have no-op's at the begining until the first 4 are executed and
  // no-ops at the end while the last 5 are being executed
  while (1) {
    if (read_next_inst == 1 && branch_ops == 0)
      size = trace_get_item(&tr_entry);
      instruction_buffer[0] = tr_entry;
      if(!size){
        size = trace_get_item(&tr_entry);
        instruction_buffer[1] = tr_entry;
      }

    if (!size) {  // no more instructions (trace_items) to simulate
      printf("\nSimulation terminates at cycle : %u\n", cycle_number);
      break;
    }

    else {  // Move the instructions through the pipeline
      cycle_number++;
      buffer_ALU[3] = buffer_ALU[2];
      buffer_ALU[2] = buffer_ALU[1];
      buffer_ALU[1] = buffer_ALU[0];

      buffer_LS[3] = buffer_LS[2];
      buffer_LS[2] = buffer_LS[1];
      buffer_LS[1] = buffer_LS[0];

      //Branch was previously predicted to be taken but is now not taken (2 noops)
      if (!control_hazard() && buffer_ALU[0].type == 5 && prediction_method == 1 && branch_predict(buffer_ALU[0].PC) != -1){
        printf("\n\n**********Branch taken predicted incorrectly**********");

        if(branch_ops == 0)
          update_branch(buffer_ALU[0].PC, -1);

        insert_squashed();
        branch_ops++;

        //Both noops have been inserted
        if (branch_ops == 2)
          branch_ops = 0;
      }

      //Branch is taken and we need to insert no ops
      else if (control_hazard() || branch_ops != 0){
        insert_squashed();
        branch_ops++;

        //Predicted not taken incorrectly (2 noops)
        if (prediction_method == 1 && branch_ops == 0)
          update_branch(buffer_ALU[0].PC, buffer_ALU[0].Addr);

        //Both noops have been inserted
        if (branch_ops == 2)
          branch_ops = 0;
      }

      // There are no hazards detected, proceed as normal
      else {
        //issue both
        if(!load_use_dependance(0) && !load_use_dependance(1)){
          if(!inst_buffer_inter_dependant()){
            buffer_ALU[0] = //THE ONE THAT GOES HERE
            buffer_LS[0] = //THE ONE THAT GOES HERE
          }
        }
        else if(!load_use_dependance(0)){
          if(instruction_buffer[0].type == 3 || instruction_buffer[0].type == 4){
            buffer_LS[0] = instruction_buffer[0];
            insert_stall(0);
          }
          else{
            buffer_ALU[0] = instruction_buffer[0];
            insert_stall(1);
          }
        }
        else{
          insert_stall(0);
          insert_stall(1);
        }
        read_next_inst = 1;
      }

    }// END ELSE there are still more inst's left to read

    if (trace_view_on) {
      print_buffers();
    }
  }// END while (1)

  trace_uninit();

  exit(0);
}

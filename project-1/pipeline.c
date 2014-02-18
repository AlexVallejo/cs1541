/**************************************************************
 CS/COE 1541
   just compile with gcc -o pipeline pipeline.c
   and execute using
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr 0
***************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "trace_item.h"

#define TRACE_BUFSIZE 1024*1024
#define NUM_BUFFERS 4
#define BRANCH_PREDICTION_TABLE_SIZE 128

static FILE *trace_fd;
static int trace_buf_ptr;
static int trace_buf_end;
static struct trace_item *trace_buf;

unsigned int cycle_number = 0;

int branch_prediction_table[BRANCH_PREDICTION_TABLE_SIZE];  // Hastable used for branch prediction
struct trace_item buffer[NUM_BUFFERS];                      // Pipeline Buffers
struct trace_item *tr_entry;                                // Temporary holding entry

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

int print_buffers(){

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

    switch (tr_entry->type) {
      case ti_NOP:
        printf("NOP\n");
        break;
      case ti_RTYPE:
        printf("RTYPE: ") ;
        printf("(PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->dReg);
        break;
      case ti_ITYPE:
        printf("ITYPE:") ;
        printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
        break;
      case ti_LOAD:
        printf("LOAD:") ;
        printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
        break;
      case ti_STORE:
        printf("STORE:") ;
        printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
        break;
      case ti_BRANCH:
        printf("BRANCH:") ;
        printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
        break;
      case ti_JTYPE:
        printf("JTYPE:") ;
        printf(" (PC: %x)(addr: %x)\n", tr_entry->PC,tr_entry->Addr);
        break;
      case ti_SPECIAL:
        printf("SPECIAL\n") ;
        break;
      case ti_JRTYPE:
        printf("JRTYPE:") ;
        printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry->PC, tr_entry->dReg, tr_entry->Addr);
        break;
      } // END switch
    }// END for (int i = 0; i < NUM_BUFFERS; i++)
  return 1;
}// END print_buffer(int trace_view_on)

int insert_stall(){

  // Init a new no-op to insert in the pipeline
  struct trace_item no_op;
  no_op.type = 0;
  no_op.sReg_a = 0;
  no_op.sReg_b = 0;
  no_op.dReg = 0;
  no_op.PC = 0;
  no_op.Addr = 0;

  printf("\n*****STALL*****\n");

  // Print the pipeline as it would appear if the conflict was in the pipeline
  printf("\nConflicting Pipeline: ");
  buffer[0] = *tr_entry;
  print_buffers();

  // Print the pipeline as it appears after adding a stall
  printf("\nResolved Pipeline: ");
  buffer[0] = no_op;
  read_next_inst = 0;
  print_buffers();

  return 1;
}

int insert_squashed(){

  // Init a new no-op to insert in the pipeline
  struct trace_item squashed_inst;
  squashed_inst.type = 0;
  squashed_inst.sReg_a = 0;
  squashed_inst.sReg_b = 0;
  squashed_inst.dReg = 0;
  squashed_inst.PC = 0;
  squashed_inst.Addr = 0;

  printf("\n*****Squashed Instruction*****\n");

  // Print the pipeline as it would appear if the conflict was in the pipeline
  printf("\nConflicting Pipeline: ");
  buffer[0] = *tr_entry;
  print_buffers();

  // Print the pipeline as it appears after adding a stall
  printf("\nResolved Pipeline: ");
  buffer[0] = squashed_inst;
  read_next_inst = 0;
  print_buffers();

  return 1;
}

/*
 * This function detects 4 kinds of data hazards where previous inst. depends
 * on data loaded by the current inst.
 *  -load followed by an R-Type inst.
 *  -load followed by an I-Type inst.
 *  -load followed by a branch inst.
 *  -load followed by a store inst.
 *
 *  @Return true if data hazard is detected and false otherwise
 */
int data_hazard(){
  //TODO ANYTHING follow a load is a data hazard. Are we taking this into account.

  // Compare the most recently read inst. to the inst. in the inst. fetch buffer
  // Stall if a i-type follows a load and it's source reg. is the same as
  // the dest. reg of the load
  if (tr_entry->type == 2 && buffer[0].type == 3) {
    if (tr_entry->sReg_a == buffer[0].dReg) {
      return 1;
    }
  }

  // Stall if a r-type follows a load and either of it's source registers
  // are the same as the dest reg of the load
  else if (tr_entry->type == 1 && buffer[0].type == 3) {
    if (tr_entry->sReg_a == buffer[0].dReg) {
      return 1;
    }

    else if (tr_entry->sReg_b == buffer[0].dReg) {
      return 1;
    }
  }

  // Stall IF a load is followed by a branch inst. where the branch depends on
  // the value returned by the load inst.
  // TODO insert 1 or two stalls? Mike's uses 1 as well.
  else if (tr_entry->type == 5 && buffer[0].type == 3){
    if (tr_entry->sReg_a == buffer[0].dReg)
      return 1;

    else if (tr_entry->sReg_b == buffer[0].dReg)
      return 1;
  }

  // Stall IF a load is followed by a store instruction where the store is
  // trying to store the value loaded by the load inst.
  else if (tr_entry->type == 4 && buffer[0].type == 3)
    if (tr_entry->sReg_a == buffer[0].dReg)
      return 1;

  return 0;
}

int control_hazard(){
  // RETURN true IF the inst in the buffer is a branch and next instruction
  // executed is not next sequentially in the code
  if (buffer[0].type == 5 && tr_entry->PC != (buffer[0].PC + 4) && tr_entry->PC != buffer[0].PC)
    return 1;

  return 0;
}

/*
 * Initialize every index in the branch prediction table to zero
 */
int init_branch_prediction_table(){
  for (int i = 0; i < BRANCH_PREDICTION_TABLE_SIZE; i++)
    branch_prediction_table[i] = 0;
  return 0;
}

/*
 * Every value in the branch prediction table is initialized to zero.
 * If there is no data on the branch, it will return a value of 0 (not taken).
 * If there is data on the branch, it will return the last state of the branch
 *  taken (1) or not taken (0)
 */
int branch_predict(int addr){
  int index = addr % 128;
  return branch_prediction_table[index];
}

/*
 * Updates the address with the given value
 */
int update_branch(int addr, int taken){
  int index = addr % 128;
  branch_prediction_table[index] = taken;
  return 0;
}

int main(int argc, char **argv) {
  //struct trace_item *tr_entry; // The inst. fetched from inst. mem.
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;         // Set print's for each cycle off for default
  int branch_ops = 0;            // Control to allow for two no-ops to follow an
                                 // incorrect branch prediction
  int prediction_method = 0;     // boolean to use or not use the 1-bit branch
                                 // predictor

  // Explanations of each field are in trace_item.c
  unsigned char t_type = 0;
  unsigned char t_sReg_a= 0;
  unsigned char t_sReg_b= 0;
  unsigned char t_dReg= 0;
  unsigned int t_PC = 0;
  unsigned int t_Addr = 0;

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch_1 - any character> <switch_2 - any character> \n");
    fprintf(stdout, "\n(switch_1) to turn on or off individual item view.\n\n");
    fprintf(stdout, "\n(switch_2) to turn on or off the one branch predictor.\n\n");
    exit(0);
  }

  // Parse command line arguments
  trace_file_name = argv[1];
  if (argc == 3)
    trace_view_on = atoi(argv[2]);

  else if (argc == 4){
    trace_view_on = atoi(argv[2]);
    prediction_method = atoi(argv[3]);
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

    if (!size) {  // no more instructions (trace_items) to simulate
      printf("\nSimulation terminates at cycle : %u\n", cycle_number);
      break;
    }

    else {  // Move the instructions through the pipeline
      cycle_number++;
      buffer[3] = buffer[2];
      buffer[2] = buffer[1];
      buffer[1] = buffer[0];

      // IF there is a data hazard insert a stall in the pipeline
      if (data_hazard())
        insert_stall();

      //  TODO SQUASHED INSTRUCTIONS MUST BE PRINTED AS "Squashed" PER THE INSTRUCTIONS
      // IF there is a control hazard add two no-ops
      //  We add two no-ops by using a loop counter that counts to two and resets
      //  after two more loops have been executed. In that time, no more new
      //  inst's are read and no-ops are inserted in their place. Our
      //  architecture requires that we insert two no-ops when assuming branches
      //  are always taken
      // Predict not taken
      else if (control_hazard() || branch_ops != 0){
        // IF there is no prediction method, assume the branch is not taken
        if (prediction_method == 0){
          insert_squashed();
          branch_ops++;
        }

        // ELSE prediction method is enabled and a control hazard was detected
        //  indicating that we must update the prediction status for the branch
        //  with the control hazard to taken
        else {
          update_branch(tr_entry->Addr, 1);
          branch_ops++;
        }

        //TODO Is this correct?
        // IF we have put two no-ops in the pipeline, reset the counter so no 
        // more no-ops are inserted
        if (branch_ops == 2)
          branch_ops = 0;
      }

      else if (prediction_method == 1 && branch_predict(tr_entry->Addr)){
        printf("\n\n************Branch predicted to be taken");
        //TODO implement more functionality here
      }
      // There are no hazards detected, proceed as normal
      else {
        buffer[0] = *tr_entry;
      }

      // IF the most recently read instruction was added to the pipeline,
      // re-enable instruction reading
      if (buffer[0].PC == tr_entry->PC)
        read_next_inst = 1;

    }// END ELSE there are still more inst's left to read

    if (trace_view_on) {
      print_buffers();
    }
  }// END while (1)

  trace_uninit();

  exit(0);
}

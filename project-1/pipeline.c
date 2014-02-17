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

static FILE *trace_fd;
static int trace_buf_ptr;
static int trace_buf_end;
static struct trace_item *trace_buf;

unsigned int cycle_number = 0;

struct trace_item buffer[NUM_BUFFERS];    // Pipeline Buffers
struct trace_item *tr_entry;              // Temporary holding entry

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

  if (trace_buf_ptr == trace_buf_end) { /* if no more unprocessed items in the trace buffer, get new data  */
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

int main(int argc, char **argv)
{
  struct trace_item *tr_entry;  // The inst. fetched from inst. mem.
  size_t size;
  char *trace_file_name;
  int trace_view_on = 0;        // Set print's for each cycle off for default

  // Explanations of each field are in trace_item.c
  unsigned char t_type = 0;
  unsigned char t_sReg_a= 0;
  unsigned char t_sReg_b= 0;
  unsigned char t_dReg= 0;
  unsigned int t_PC = 0;
  unsigned int t_Addr = 0;

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }

  trace_file_name = argv[1];
  if (argc == 3) trace_view_on = atoi(argv[2]) ;

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();

  while (1) {
    size = trace_get_item(&tr_entry);

    if (!size) {       /* no more instructions (trace_items) to simulate */
      printf("\nSimulation terminates at cycle : %u\n", cycle_number);
      break;
    }

    // Move the instructions through the pipeline
    else {
      cycle_number++;
      buffer[3] = buffer[2];
      buffer[2] = buffer[1];
      buffer[1] = buffer[0];
      buffer[0] = *tr_entry;
    }

    if (trace_view_on)    // If trace view is on dump the contents of the buffers
      print_buffers();    // to the screen
  }

  trace_uninit();

  exit(0);
}


#include <stdlib.h>
#include <stdio.h>
#include "trace_item.h"

#define TRACE_BUFSIZE 1024*1024

static FILE *trace_fd;
static int trace_buf_ptr;
static int trace_buf_end;
static struct trace_item *trace_buf;

// to keep statistics
unsigned int accesses = 0;
unsigned int read_accesses = 0;
unsigned int write_accesses = 0;
unsigned int hits = 0;
unsigned int misses = 0;
unsigned int misses_with_writeback = 0;

void trace_init(){

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

int trace_get_item(struct trace_item **item){

  int n_items;

  if (trace_buf_ptr == trace_buf_end) {
    // get new data
    n_items = fread(trace_buf, sizeof(struct trace_item), TRACE_BUFSIZE, trace_fd);
    if (!n_items) return 0;

    trace_buf_ptr = 0;
    trace_buf_end = n_items;
  }

  *item = &trace_buf[trace_buf_ptr];
  trace_buf_ptr++;

  return 1;
}

int main(int argc, char **argv){

  struct trace_item *tr_entry;
  size_t size;
  char *trace_file_name;
  int trace_view_on ;

  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }

  trace_file_name = argv[1];
  trace_view_on = (argc == 3);
  // here you should extract the cache parameters from the command line

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();
  // here should call cache_create(cache_size, block_size, associativity, replacement_policy)

  while(1) {
    size = trace_get_item(&tr_entry);

    if (!size) {       /* no more instructions to simulate */
      printf("+ number of accesses : %d \n", accesses);
      printf("+ number of reads : %d \n", read_accesses);
      printf("+ number of writes : %d \n", write_accesses);
      break;
    }

    else{              /* process only loads and stores */;
      if (tr_entry->type == ti_LOAD) {
        if (trace_view_on)
          printf("LOAD %x \n",tr_entry->Addr) ;
        accesses ++;
        read_accesses++ ;
        // call cache_access(struct cache_t *cp, tr_entry->Addr, access_type)
      }

      if (tr_entry->type == ti_STORE) {
        if (trace_view_on)
          printf("STORE %x \n",tr_entry->Addr) ;
        accesses ++;
        write_accesses++ ;
        // call cache_access(struct cache_t *cp, tr_entry->Addr, access_type)
      }
      // based on the value returned, update the statisctics for hits, misses and misses_with_writeback
    }
  }

  trace_uninit();

  exit(0);
}


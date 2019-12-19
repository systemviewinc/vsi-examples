#include "sort_x86.h"

// only for use in x86 contexts - Bharath
// random array of ints (between 0 and 999)
void sort_source_array(int arr[MAXSIZE]) {
  printf("In function %s\n",__FUNCTION__);
  for(int i = 0;i < MAXSIZE;i++) {
    arr[i] = -1;
  }
  printf("Unsorted data:\n");
  srand(time(0));
  for(int i = 0;i < MAXLEN;i++) {
    arr[i] = (uint32_t) (rand() % 1000);
    printf("%d\n", arr[i]);
  }
  printf("# of unsigned ints sent: %u\n", MAXLEN);
  while(1)
    ;
}

// random stream of ints (between 0 and 999)
void sort_source_stream(hls::stream<ap_axis_dk<DATA_WIDTH> > &outs)
{
  printf("In function %s\n",__FUNCTION__);
  printf("Unsorted data:\n");
  ap_axis_dk<DATA_WIDTH> f;
  srand(time(0));
  for(int i = 0;i < MAXLEN;i++) {
    f.data = (uint32_t) (rand() % 1000);
    printf("%u\n", (uint32_t) f.data);
    f.keep = -1;
    f.last = (i == (MAXLEN-1));
    outs.write(f);
  }
  printf("# of unsigned ints sent: %u\n", MAXLEN);
  while(1)
    ;
}

void display_unsigned_int_stream(hls::stream<ap_axis_dk<DATA_WIDTH> > &ins)
{
  printf("In function %s\n",__FUNCTION__);
  int count = 0;
  while(!(ins.empty())) {
    ap_axis_dk<DATA_WIDTH> e;
    e = ins.read();
    count++;
    printf("%u\n", (uint32_t) e.data);
    if(e.last)
      break;
  }
  printf("# of unsigned ints received: %u\n", count);
  printf("Done...\n");
  while(1)
    ;
}

void display_unsigned_int_array(int in_arr[MAXSIZE])
{
  printf("In function %s\n",__FUNCTION__);
  int count=0;
  for(int i = 0;i < MAXSIZE;i++) {
    if (in_arr[i] == -1)
      break;
    count++;
    printf("%u\n",in_arr[i]);
  }
  printf("# of unsigned ints received: %u\n", count);
  printf("Done...\n");
  while(1)
    ;
}

// A utility function to swap two elements
void swap ( int* a, int* b )
{
    int t = *a;
    *a = *b;
    *b = t;
}

/* This function is same in both iterative and recursive*/
int partition (int arr[], int l, int h)
{
    int x = arr[h];
    int i = (l - 1);

    for (int j = l; j <= h- 1; j++)
    {
        if (arr[j] <= x)
        {
            i++;
            swap (&arr[i], &arr[j]);
        }
    }
    swap (&arr[i + 1], &arr[h]);
    return (i + 1);
}

/* A[] --> Array to be sorted, l  --> Starting index, h  --> Ending index */
void quickSortIterative (int arr[], int l, int h)
{
    // Create an auxiliary stack
    int stack[ h - l + 1 ];

    // initialize top of stack
    int top = -1;

    // push initial values of l and h to stack
    stack[ ++top ] = l;
    stack[ ++top ] = h;

    // Keep popping from stack while is not empty
    while ( top >= 0 )
    {
        // Pop h and l
        h = stack[ top-- ];
        l = stack[ top-- ];

        // Set pivot element at its correct position in sorted array
        int p = partition( arr, l, h );

        // If there are elements on left side of pivot, then push left
        // side to stack
        if ( p-1 > l )
        {
            stack[ ++top ] = l;
            stack[ ++top ] = p - 1;
        }

        // If there are elements on right side of pivot, then push right
        // side to stack
        if ( p+1 < h )
        {
            stack[ ++top ] = p + 1;
            stack[ ++top ] = h;
        }
    }
}

void sort_stream(hls::stream<ap_axis_dk<DATA_WIDTH> > &ins32,
		 hls::stream<ap_axis_dk<DATA_WIDTH> > &outs32)
{
  const int MAX_ELEMENTS = 4096; // 4 bytes/int => we can send maximum of 1024 ints
  int in_arr[MAX_ELEMENTS];
  uint32_t number_non_neg_elements = 0;

  for(int i = 0;i < MAX_ELEMENTS;i++) {
    in_arr[i] = -1;  // random numbers start at 0
  }

  // wait till data starts streaming
  while(ins32.empty())
    ;
  
  // Read in data, counting number of elements
  while(1) {
    ap_axis_dk<DATA_WIDTH> e;
    e = ins32.read(); // reads are blocking
    in_arr[number_non_neg_elements] = e.data;
    number_non_neg_elements++;
    if(e.last)
      break;
  }

  // since in_arr is initialized to -1, quick sorting means we will have -1s till first non-negative element
  quickSortIterative(in_arr,0,MAX_ELEMENTS-1);

  // stream only non-negative numbers
  uint32_t start_pos = 0;
  for(;start_pos < MAX_ELEMENTS;start_pos++) {
    if(in_arr[start_pos] != -1) {
      break; // stop at first non-negative element
    }
  }

  // stream out!
  const uint32_t last_pos = start_pos+number_non_neg_elements;
  ap_axis_dk<DATA_WIDTH> f;
  for(int count = start_pos;count < last_pos;count++) {
    f.data = (uint32_t) in_arr[count];
    f.keep = -1;
    f.last = (count == (last_pos-1));
    outs32.write(f);
  }
  
}


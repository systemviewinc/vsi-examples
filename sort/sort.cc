// An iterative implementation of quick sort
#include <stdio.h>

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
#define BUF_SIZE 1024
/* A[] --> Array to be sorted, l  --> Starting index, h  --> Ending index */
void quickSortIterative (int arr[])
// , int l, int h)
{
    // Create an auxiliary bufstack_
    // int bufstack_[ h - l + 1 ];
    int bufstack_[ BUF_SIZE ];
    int l = 0;
    int h = BUF_SIZE - 1;

    // initialize top of bufstack_
    int top = -1;

    // push initial values of l and h to bufstack_
    bufstack_[ ++top ] = l;
    bufstack_[ ++top ] = h;

    // Keep popping from bufstack_ while is not empty
    while ( top >= 0 )
    {
        // Pop h and l
        h = bufstack_[ top-- ];
        l = bufstack_[ top-- ];

        // Set pivot element at its correct position in sorted array
        int p = partition( arr, l, h );

        // If there are elements on left side of pivot, then push left
        // side to bufstack_
        if ( p-1 > l )
        {
            bufstack_[ ++top ] = l;
            bufstack_[ ++top ] = p - 1;
        }

        // If there are elements on right side of pivot, then push right
        // side to bufstack_
        if ( p+1 < h )
        {
            bufstack_[ ++top ] = p + 1;
            bufstack_[ ++top ] = h;
        }
    }
}

// Classic version, operates on out_arr, broken by HLS bug in Vivado 2019.2
void sort_v1 (int in_arr[1024], int out_arr[1024])
{
	for (int i = 0 ; i < BUF_SIZE; i++)
		out_arr[i] = in_arr[i];
	quickSortIterative(out_arr);
}

// Version of sort developed to work around HLS bug in Vivado 2019.2
void sort (int in_arr[1024], int out_arr[1024])
{
	int i;
	int inter_arr[BUF_SIZE];
	for (i = 0; i < BUF_SIZE; i++)
		inter_arr[i] = in_arr[i];
	quickSortIterative(inter_arr);
	for (i = 0; i < BUF_SIZE; i++)
		out_arr[i] = inter_arr[i];
}

void inverter(int in_arr[1024], int out_arr[1024]) {
    for (int i = 0 ; i < BUF_SIZE; i++) {
        out_arr[i] = 0xffffffff - in_arr[i];
    }
}

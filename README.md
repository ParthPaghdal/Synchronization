# Synchronization

## Overview

In the project, producer-consumer problem is solved to access to a bounded buffer storing candies. One group of threads will model candy factories which generates candies one at a time and insert the candies into the bounded buffer. Another group of
threads will model kids who get one candy at a time from the bounded buffer. The program, called
`candykids`, will accept three arguments:

```bash
./candykids <#factories> <#kids> <#seconds>
```

\# Factories:  Number of candy-factory threads to spawn.

\# Kids:       Number of kid threads to spawn.

\# Seconds:    Number of seconds to allow the factory threads to run for.
  
Example: ./candykids 3 1 10

**Note:** All code is written in C and run on a Linux machine.

## Producer/Consumer Operations

### Main Function

main() function will start and control the application. Its steps are as follows:

```c
int main(int argc, char *argv[]) {
  // 1.  Extract arguments
  // 2.  Initialization of modules
  // 3.  Launch candy-factory threads
  // 4.  Launch kid threads
  // 5.  Wait for requested time
  // 6.  Stop candy-factory threads
  // 7.  Wait until there is no more candies
  // 8.  Stop kid threads
  // 9.  Print statistics
  // 10. Cleanup any allocated memory
}
```

1. **Extract Arguments**: Processes the arguments passed on the command line. All arguments must be
   greater than 0. If any argument is 0 or less, display an error and exit the program.

2. **Initialization**: Do module initialization as necessary. There are at least two modules as
   explained below, the bounded buffer and statistics. If no initialization is required by your
   implementation, you may skip this step.

3. **Launch factory threads**: Spawn the requested number of candy-factory threads. To each thread,
   pass it its factory number: _0_ to _(number of factories - 1)_.

4. **Launch kid threads**: Spawn the requested number of kid threads.

5. **Wait for requested time**: In a loop, call `sleep(1)`. Loop as many times as the `# Seconds`
   command line argument. Print the number of seconds running each time, exactly as `Time Xs` where
   X is the number of seconds. This shows time ticking away as your program executes.

6. **Stop factory threads**: Indicate to the factory threads that they are to finish, and then call
   join for each factory thread. See the section on [candy-factory threads](#candy-factory-thread)
   for details.

7. **Wait until there is no more candies**: While there is still candies in the bounded buffer
   (check by calling a method in your bounded buffer module), print (exactly) `Waiting for all
   candies to be consumed` and sleep for 1 second.

8. **Stop kid threads**: For each kid thread, cancel the thread and then join the thread.

9. **Print statistics**: Call the statistics module to display the statistics. See statistics
   section below.

10. **Cleanup any allocated memory**: Free any dynamically allocated memory.

### File Structure

* `candykids.c`: Main application holding factory thread, kid thread, and `main()` function. Plus
  some other helper functions, and some `#defined` constants.
* `bbuff.h/.c`: Bounded buffer module (see [Bounded Buffer](#bounded-buffer)).
* `stats.h/stats.c`: Statistics module (see [Statistics](#statistics)).
* `Makefile`: Compiles all the `.c` files and link together the `.o` files.

**_Suggestions:_** The factory creates candies and the kids consume it. The candies will be stored
in a bounded buffer. To do this, you need a data type to represent a candy. The following struct is
an example:

```c
typedef struct  {
  int factory_number;
  double creation_ts_ms;
} candy_t;
```

* `factory_number` tracks which factory thread produced the candy item.
* `creation_ts_ms` tracks when the item was created. You can get the current number of milliseconds
  using the following function:

  ```c
  double current_time_in_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
  }
  ```

### Candy-Factory Thread

Each candy-factory thread should do the following.

1. Loop until main() signals to exit (see [Thread Signaling](#thread-signaling)).

* Pick a number of seconds which it will (later) wait. The number is randomly selected between 0
    and 3 (inclusive).
* Prints a message as: `Factory 0 ships candy & waits 2s` (where 0 is the thread number and
    2 is the number of seconds it will wait)
* Dynamically allocates a new candy item and populate its fields.
* Add the candy item to the bounded buffer.
* Sleep for number of seconds identified in the first step.

2. When the thread finishes, print a message as: `Candy-factory 0 done` (where 0 is the
   thread number).

#### Thread Signaling

The thread will end when signaled to do so by `main()`. Need to use thread synchronization
primitives in order to do this. Choose something appropriate for this purpose.

### Kid Thread

Each kid thread should loop forever and during each iteration:

* Extract a candy item from the bounded buffer; this will block until there is a candy item to
  extract.
* Process the item. Initially it may just want to `printf()` it to the screen; in the next section,
  then add a statistics module that will track what candies have been eaten.
* Sleep for either 0 or 1 seconds (randomly selected).

The kid threads are canceled from `main()` using `pthread_cancel()`. When this occurs, it is likely
that the kid thread will be waiting on the lock for the bounded buffer. 
### Statistics

Statistics module tracks the following:

* The number of candies each factory creates. Called from the candy-factory thread.
* The number of candies that were consumed from each factory.
* For each factory, the min, max, and average delays for how long it took from the moment the candy
  was produced (dynamically allocated) until consumed (eaten by the kid). This will be done by the
  factory thread calling the stats code when a candy is created, and the kid thread calling the
  stats code when an item is consumed.

#### Displaying Stats Summary

When the program ends, it displays a table summarizing the statistics gathered by the program.
It should look like the following:

```c
Statistics:
Factory#   #Made  #Eaten  Min Delay[ms]  Avg Delay[ms]  Max Delay[ms]
       0       5       5        0.60498     2602.81274     5004.28369
       1       5       5        0.40454     2202.97290     5005.06494
       2       7       7        0.60107     2287.86067     4004.16162
       3       8       8     1001.12012     2377.36115     5004.13159
       4       5       5        0.40186     2202.63008     5005.38330
       5       4       4     1003.22095     2503.94049     4006.16309
       6       5       5     1003.24487     2603.35894     4005.19873
       7       6       6     3002.30640     3836.61743     4005.16089
       8       4       4     3001.74048     3753.03259     5004.31177
       9       4       4     3002.76660     4253.44440     5005.13550
```

* Factory #: Candy factory number. In this example, there were 10 factories.
* \# Made: The number of candies that each factory reported making (as per the call from the
  candy-factory thread).
* \# Eaten: The number of candies which kids consumed (as per the call from the kid threads).
* Min Delay\[ms\]: Minimum time between when a candy was created and consumed over all candies
  created by this factory. Measured in milliseconds.
* Avg Delay\[ms\]: Average delay between this factory's candy being created and consumed.
* Max Delay\[ms\]: Maximum delay between this factory's candy being created and consumed.

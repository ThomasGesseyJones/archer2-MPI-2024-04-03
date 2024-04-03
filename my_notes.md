# My Notes

## Day 1 (Wednesday, 2024-04-03)

### Introduction 

Using ARCHER2, Tier-1 HPC service in the UK.
5860 nodes each with 128 cores (> 750,000 CPU cores), closest UK has to a national supercomputer.
Provided by EPCC, The University of Edinburgh.
Provide a range of postgraduate teaching (Msc in HPC) and training courses.
Including a supercomputing MOOC, good for a conceptual introduction to HPC (on ARCHER2 website).

Self-contained course, ARCHER2 is just what we are using, but the course is not specific to ARCHER2.

Instructor: David Henty ([d.henty@epcc.ed.ac.uk](mailto:d.henty@epcc.ed.ac.uk))

Course Website: [https://www.archer2.ac.uk/training/courses/240403-mpi/](https://www.archer2.ac.uk/training/courses/240403-mpi/)

Personal accounts for duration of the course, will lose access after a month, and then delete after another two weeks.
Remember to back up!

Getting time on ARCHER2, can apply for time, or take the ARCHER2 Driving Test, for a free allocation of 800CU 
(1 CU = 1 node hour) every 12 months! Roughly 100,000 CPU hours. Applying for time is peer-reviewed process like on 
DIRAC.


### Message Passing Programming (with MPI)

Main aim is to understand the message-passing model for parallel programming and write parallel programs using MPI
in C++.

MPI is the most important piece of software for parallel programming, and is a standard for message-passing programming
for supercomputers across the world. MPI was designed 30+ years, very successful, but quite basic and low-level.

Can break programming down into, fundamental concepts (e.g., arrays), languages (e.g., C++), and 
implementation (e.g., gcc). MPP has the same divide, with fundamental concepts (e.g., processes), 
languages (e.g., MPI), and implementation (e.g., OpenMPI). Though in practice MPI has outcompeted the
other languages for message-passing programming so only one worth learning. Should aim to still think in concepts.

As a **warning** OpenMP is not the same as OpenMPI, OpenMP is for shared memory programming, OpenMPI is for distributed
memory programming. We will not talk about the former. 

Modern OSs, have many tasks, but they are ring-fenced from each other, so can't access each other. This is 
intentional to avoid catastrophic failure. MPI is used when we want lots of processes to co-operate on the same
task and thus cannot be ring-fenced from each other. Threads can share memory, but processes cannot. MPI deals with
the communication between processes. At the most basic level this is done by sending messages between processes via
a library call. Could have a fully parallelized program, but none have taken off. A process can have multiple threads,
handled separately, but MPI doesn't care about this. Calling MPI from threads is not a good idea, get race conditions.
MPI is just the communication between processes, so also doesn't care about hardware, CPUs, GPUs, TPUs, etc.

Parallel paradigm:
* Multiple processes (numbered), each with their own memory space.
* Processes communicate by sending messages via MPI to a communication network.
* Processes can be on the same node or different nodes.
* We don't care how the communication network works, just that it does, MPI deals with this.
* Typically, in a supercomputer, the memory is also distributed across the nodes, so the communication network is
  also the memory network.
* Note each compute node can have multiple processes, but MPI hides this from you.

Process communication in MPI is a two-ended process, both the sender needs to `send`, and the receiver needs to 
`receive`. This is analogous to emails. A common trip-up is variables can be different between processes, this makes 
debugging a lot harder.

MPI uses Single Program Multiple Data (SPMD) model, where all processes run the same program, but can have different
data. This is the most common model for parallel programming. Each process has a unique ID to allow the
behavior to be different between processes (otherwise what's the point of having multiple processes).

Message transfers a number of data items of a certain type from the memory of one process to the memory of another.
Message sending can be synchronous or asynchronous. Synchronous is blocking, the sender waits until the receiver has
received the message. Asynchronous is non-blocking, the sender can continue without waiting for the receiver.
You can think of this like phone call, and email, respectively. But, warning synchronous blocking has no time limit, 
so you can wait forever and end up in a deadlock where every process is waiting for something. It is up to you to
match the sends and receives, MPI will not do this for you.

Point-to-point communication is one sender to one receiver, but you can have multiple senders and receivers.
In many cases this is far more efficient/desirable. Called collective communication, e.g., broadcast, scatter, gather,
reduce, etc. These are more efficient as they can be optimized for the network, and can be done in parallel. Don't
try to do these yourself, use the MPI library calls. Some examples:
* `Barrier` is used for global synchronization, all processes wait
until all processes have reached the barrier. Useful for timing. 
* `Broadcast` is used to send the same data to all processes. Often used at the start to send the same data to all
processes.
* `Scatter` is used to send different data to each process. Often used to distribute data to all processes. Scattering
inputs for different processes to work on.
* `Gather` is the opposite of scatter, collects data from all processes to one process. Often used to collect results
and then decide what to do with them.
* `Reduce` is used to combine data from all processes to one process. Often used to combine results from all processes
to get a final result. Options include sum, product, max, min, etc. Sum is probably the most common.

#### Summary:

Outline of MPI program (think of it as a programming model distinct from sequential programming):
* Write a single piece of source code with private data, and explicit communications
* Compile it with a standard compiler, and link with MPI library (both open source and vendor exist)
* Run multiple copies of the executable (SPMD) on different nodes/machines (private data, variables and position in 
program)
* Running usually done via a launcher program

Issues:
* Send/receives don't match (deadlock)
* Possible to write very complicated programs that are hard to debug (often not needed)
* Use collective communications where possible for efficiency (read the docs!)


### Practical Example: Parallel Traffic Modelling

Want to predict traffic flow. Build a computer model, lots of people spend time doing this. 

We are going to use a very simple model. 1D cellular automaton, each cell is a road segment, and each cell can have
a car in it. The model is updated in discrete time steps, and the cars move according to simple rules:
* If a cell is occupied, and the cell in front is empty, the car moves forward.
* If a cell is occupied, and the cell in front is also occupied, the car stays where it is.
* Use PBC, implemented using ghost or halo cells at each end of the road.

Want to calculate the average speed of the cars and density, as can see even in this simply model congestion can occur.
Below 50% get 1 average speed, above 50% get decreasing average speed, and at 100% get 0 average speed. A surprisingly
well studied cellular automaton model [https://en.wikipedia.org/wiki/Rule_184](https://en.wikipedia.org/wiki/Rule_184).

Going to split the problem into pieces and give them each to a different process for distributed computing. 
Problem has standard structure of calculation phase and communication phase alternating.

Communication is required in this problem both for boundary exchange but also collective communication for
calculating the average speed and density.

Strategy 1: Replicate data, update local part, then communicate to make data consistent. Scales poorly in both data and
communication. As does not take into account only need to communicate with neighbours. Such strategies can work if
little data but very complex calculation on its data. 

Strategy 2: Distributed data, each process gets a subset of the data. Need to communicate to update the boundary cells. 
Load balance, trivially works in this case as long as road pieces equal size. Doing boundary exchange is actually 
somewhat complex, as at risk of deadlock if all receive then send. Asynchronous communication can help, but need to
send first then receive. Synchronous can work will if use parity to break symmetry. Correct way (i.e., generalizes) to 
fix this is non-blocking communication, but this is more complex. See tomorrow.


### Message Passing Interface (MPI)

MPI a unified standard from 1993, developed with 60 people from 40 organizations. Not changed much since. Goals
of MPI is provide source-code portability, and efficient implementation. It offers a great deal of functionality, with
support for heterogeneous parallel architecture (hence, why a lot of the functions seem over complex). In practice, 
this is no longer done in scientific computing. 

Library syntax for C++ (actually a C library, but C++ bindings exist). 
```c++
#include <mpi.h>
```

All functions have syntax
```c++
error = MPI_Function(arguments);
```
where `error` is an integer, and `MPI_Function` is the function name. If `error` is `MPI_SUCCESS` then the function
succeeded, otherwise it failed. Most people ignore the error code. MPI is not fault-tolerant, but it is quite good at 
spotting a problem and crashing, to avoid wasting compute time.

MPI controls its own internal data structures, MPI releases 'handles' to allow programmers to reference these, C handles
are of defined typedefs.

MPI must be initialized (it does not create parallel processes it establishes communication between them). 
```c++
int MPI_Init(int *argc, char ***argv);
```
passing a pointer to argc and argv, as MPI may modify these. Must be called before any other MPI function. 
Large parallel programs don't use command line arguments so, most C++ programs will pass `NULL` and `NULL`.
E.g.,
```c++
int main() {
    MPI_Init(NULL, NULL);
    // ...
    return 0;
}
```

The scope of communication set in MPI is set by the communicator. Most of the time we use the everyone communicator,
`MPI_COMM_WORLD`. This is a predefined communicator that includes all processes. Clever stuff can be done with 
communicators that restrict the scope of communication, but this is not covered in this course.

Each process is identified by its ID, called a rank. This is an integer from 0 to `size - 1`, where `size` is the number
of processes in the communicator. 
```c++
int MPI_Comm_rank(MPI_Comm communicator, int *rank);
```
This function returns the rank of the calling process in the communicator.

Best to not hard-code the number of processes, but to get it from MPI. 
```c++
int MPI_Comm_size(MPI_Comm communicator, int *size);
```
as sometimes you may need to know this, especially if you are doing neighbour communication.
So a simple hello world program would be:
```c++
int main() {
    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // note pass variable to give output to as reference
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // as outputs used for error code
    printf("Hello world from process %d of %d\n", rank, size);
    MPI_Finalize();
    return 0;
}
```
We have also introduced `MPI_Finalize` which must be called after all other MPI functions, and before the program
exits. This is where MPI tidies up and closes down the communication network.

Sometimes also useful to get mapping of processes to nodes/processes/cores, as helpful for debugging. 
```c++
int namelen;
char processor_name[MPI_MAX_PROCESSOR_NAME];
MPI_Get_processor_name(processor_name, &namelen);
```
Note gives you both the processor name and the length of the name. The latter is not very useful in C but is in 
Fortran.


### MPI on ARCHER2

Login syntax:
```bash
ssh -XY user@login.archer2.ac.uk
```
where `user` is your username. The `-XY` is for X11 forwarding, so you can run graphical programs on ARCHER2 and have
them displayed on your local machine. 

ARCHER2 uses SLURM scheduler. ARCHER2 can only run MPI jobs on compute nodes, not login nodes, so we will need to use 
that even for testing.

Compile using `CC`, as ARCHER2 is a dedicate parallel system, its compilers are already set-up with the
appropriate MPI library pathing by using these wrappers.

Idiosyncrasies:
* Not possible to run MPI directly on login nodes
* Cannot run from the home directory, nodes can only see work, so move to that

ARCHER2 does support interactive jobs. Can be very useful for debugging. But, limited to
short jobs. 

Online documentation for MPI is a bit poor. MPI books are generally a better way to learn it. A good example is
"Using MPI" by Gropp, Lusk, and Skjellum.


### Exercise 1: Hello World

Print ordering between processes is not guaranteed, as there is a lot of buffering between print statements and
actually reaching the terminal. 

See `/exercises/My_Solutions/Hello_World` for code.


### Messages

MPI_Send and MPI_Recv just pass references, so you also have to specify the type of the data being sent. 
```c++
int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm communicator);
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm communicator, MPI_Status *status);
```
where `buf` is the data to be sent, `count` is the number of data items, `datatype` is the type of the data, `dest` is
the rank of the destination process, `tag` is an integer tag for the message, and `communicator` is the communicator.
`source` is the rank of the source process, and `status` is a status object that can be used to get information about
the received message. Tag is not very useful. Count is just the amount of data reserved, not the actual size of the 
data received, this allows you to receive unknown amounts of data, as long as you have a bound on the size. In most
cases it is the same as the number of data items as that is known. To know what was actually sent in the case of an 
unknown size, you will need to check the status. 

MPI has a number of predefined datatypes, e.g., `MPI_INT`, `MPI_FLOAT`, `MPI_DOUBLE`, etc. These are used to specify
the type of the data being sent.

MPI sends both the data, but also a header of additional information. This is received into separate storage that is 
called status. You may never look at it, but you need to specify it.

MPI has different communication modes

| Mode           | Description                                                              |
|:---------------|--------------------------------------------------------------------------|
| Synchronous    | Sender waits until receiver has received the message                     |
| Buffered       | Sender copies data to a buffer, and then can continue (always completes) |
| Standard send  | Either of the above (you don't know which!)                              |
| Receive        | Completes when data is received                                          |

Example sending an array of ten integers:
```c++
int data[10];
int rank;
MPI_STATUS status;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
if (rank == 0) {
    for (int i = 0; i < 10; i++) {
        data[i] = i;
    }
    MPI_Ssend(data, 10, MPI_INT, 1, 0, MPI_COMM_WORLD);
} else if (rank == 1) {
    MPI_Recv(data, 10, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    for (int i = 0; i < 10; i++) {
        printf("Received data[%d] = %d\n", i, data[i]);
    }
}
```
Rank 0 is not actually special, but it is normally treated as the master process because it always exists. 
**Warning** tags need to match.

Receivers will wait for sends. Similar to Ssend will wait for receives. Ranks must be valid, for communication to 
work, the communicator must be the same, the message types must match, and the tags must match. Reciever buffer
also needs to be big enough to hold the data.

There are also wildcards for any source and any tag, `MPI_ANY_SOURCE` and `MPI_ANY_TAG`. These can be used to
receive from any source or any tag. The actual one will be stored in the status object. You can access them via
`status.MPI_SOURCE` and `status.MPI_TAG`.

Message order is preserved for a given source. Messages do not overtake each other, even in a non-synchronous mode. 
This can be useful with buffered sends. Bear in mind if the tag doesn't match that message will be ignored. So messages 
can overtake each other but only if you use different tags.
```c++
//Rank 0
Bsend(msg1, dest=1, tag=1);
Bsend(msg2, dest=1, tag=2);
//Rank 1
Recv(msg2, source=0, tag=2);
Recv(msg1, source=0, tag=1);
```
This will work, with msg2 arriving before msg1. 

MPI also has a timer.


### Exercise 2: Parallel Calculation of Pi

See linked exercises in README.md for details and `exercises/My_Solutions/Parallel_Pi` for my solution.


### MPI Modes, Tags, and Communicators

Aim is to explain why asynchronous communication is not often used in MPI.

#### Modes

MPI_Ssend, synchronous send, will not return until the message has been delivered.

MPI_Bsend, buffered send, will be asynchronous, returns before the message is delivered. System copies data into buffer 
and sends it later on. No deadlock, or dead time, but the buffer does not come free and is finite! 

MPI_Send, standard send, can be either of the above. Implementation dependent. Causes lots of confusion.

For maximum efficiency MPI is not fault-tolerant and has little error checking. 

Recv is always synchronous. 

Bsend using a single large block of memory, made available to MPI via
MPI_Buffer_attach. This is a fixed size, and if you exceed it, it will block. Bsend is awkward to use as a result as 
whether the buffer is big enough is not always clear, and can depend on if one process is slow that particular day, 
leading to irreplicable bugs :(

MPI_Send tries to solve these problems, will normally be asynchronous like Bsend, but wil become synchronous like Ssend
if buffer is full. MPI_Send can still cause deadlock if the buffer is full. So can still have irreplicable bugs, that
are annoyingly system dependent.

Useful to know a code that runs correctly with MPI_Send will always run correctly with MPI_Ssend, but not the other way
though it is very rare, as you have to do something very specific to trigger it.

General solution to this is to use non-blocking communication, which is like asynchronous communication but 
does not use a buffer, so no deadlock. More tomorrow. 


#### Checking for Messages

Can probe for matching message using `MPI_Probe`. This will return the status of the message, but not the message
itself. This can be useful if you need to know the size of the message before receiving it. 

```c++
MPI_Probe(source, tag, communicator, &status);
MPI_Get_count(&status, MPI_INT, &count);
int data[count];
MPI_Recv(data, count, MPI_INT, source, tag, communicator, &status);
```

Careful with wildcards and this, as you may get a message from a different source or tag than you were expecting.
So, you should also take the source and tag from the status object.


#### Tags

MPI tags are non-negative integers (special values are negative), only up to 32767 required to be supported.
Systems may support more, but not guaranteed, so don't rely on it. 

Tags not that commonly used, often set them to 0, or use MPI_ANY_TAG.


#### Communicators

MPI_COMM_WORLD is the most common communicator, but you can create your own. This can be useful for restricting
communication to a subset of processes. Can often get away with just this. But hardcoding MPI_COMM_WORLD is bad 
practice. Using a communicator variable is better for flexibility and readability. 
```c++
MPI_Comm my_comm;
my_comm = MPI_COMM_WORLD;
```

MPI_COMM_SELF is a communicator that only includes the calling process. This can be useful for debugging, or for
sending messages to yourself for caching.

Messages can only be received with the same communicator that they were sent with. This is to allow isolated groups that
don't accidentally communicate with each other. Could do this with tags but that is error-prone. Can use 
MPI_Comm_split to split based on some condition, e.g., rank % 2. This can be useful for splitting into even and odd
ranks.


### Exercise 3: Broadcast and Scatter

See linked exercises in README.md for details and `exercises/My_Solutions/Broadcast_and_Scatter` for my solution.


### Exercise 4: Ping Pong

See linked exercises in README.md for details and `exercises/My_Solutions/Ping_Pong` for my solution.



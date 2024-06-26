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

## Day 2 (Thursday, 2024-04-04)

### Exercise 4: Ping Pong

See linked exercises in README.md for details and `exercises/My_Solutions/Ping_Pong` for my solution.

Calculated timings:

| Size (bytes) | # Iterations | Total time (s) | Time per message (s) | Bandwidth (MB/s)   |
|:-------------|:-------------|:---------------|:---------------------|:-------------------|
| 1200         | 10000000     | 12.34          | 1.234E-06            | 972.447325769854   |
| 4000         | 3000000      | 5.24           | 1.74666666666667E-06 | 2290.07633587786   |
| 12000        | 1000000      | 2.87           | 2.87E-06             | 4181.18466898955   |
| 40000        | 300000       | 2.11           | 7.03333333333333E-06 | 5687.20379146919   |
| 120000       | 100000       | 1.64           | 1.64E-05             | 7317.07317073171   |
| 400000       | 100000       | 4.54           | 4.54E-05             | 8810.57268722467   |
| 1200000      | 10000        | 1.27           | 0.000127             | 9448.81889763779   |
| 4000000      | 10000        | 5.96           | 0.000596             | 6711.40939597315   |
| 12000000     | 1000         | 3.27           | 0.00327              | 3669.7247706422    |
| 40000000     | 1000         | 9.25           | 0.00925              | 4324.32432432432   |

From the above data we can calculate the latency, the time per message even for vanishingly
small size:

![Latency](exercises/My_Solutions/Ping_Pong/latency.png)

Finding that the latency is around 1.0 microseconds.

Meanwhile the bandwidth initially increases with message size, but then peaks and slowly decreases. 
This is due to the inefficiency of sending small messages, and the overhead of sending large messages.

![Bandwidth](exercises/My_Solutions/Ping_Pong/bandwidth.png)



### Non-blocking Communication

Deadlock can occur if you want to pass messages in a ring, as each process is waiting for the next process to receive
the message. This can be solved by using non-blocking communication.

The mode of a communication determines when its constituent operations complete, i.e., assynchronous, or synchronous.
The form of an operation determines when its constituent operations return, i.e., blocking, or non-blocking.
Blocking operations wait to return. 
Non-blocking operations return immediately, but the operation is not complete until the request is completed.
This allows the sub-program to continue to perform other work. 
At some later time the sub-program can test or wait for the completion of the non-blocking operation.

In practice, this is used to issue multiple overlapping communications, and then wait for them all to complete.
A good analogy is using a postal service, you send packages, and you can do other things while they are in transit,
but you are given a tracking number you can use to check when they arrive.
All non-blocking operations should have matching waits, or tests, to ensure they are completed. 
Some systems cannot free resources until the wait has been called, so you can introduce memory leaks if you don't
wait for the completion of the non-blocking operation.

Non-blocking operations are not the same as sequential subroutine calls as the operation continues after the call
has returned. You thus can introduce race conditions, so be careful.

Note there is no copy taken in a non-blocking operation. So do not change the data until you are sure the operation
has completed. Otherwise, you can get fun bugs.

You have to manually keep track of if MPI request is send or receive, as MPI does not do this for you. 
```c++
int MPI_Issend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm communicator, MPI_Request *request);
...
int MPI_Wait(MPI_Request *request, MPI_Status *status);
```
The I stands for immediate or initialized, as the request is initialized immediately but not completed.
```c++
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm communicator, MPI_Request *request);
...
int MPI_Wait(MPI_Request *request, MPI_Status *status);
```
Note Irecv, does not have a status object, as you don't know the size of the message until it is received.
This is why Status is now in the wait function.

Can mix blocking and non-blocking send and receives. Non-blocking sends can use any mode, synchronous, buffered, or
standard. These are done via `MPI_ISend`, `MPI_IBsend`, and `MPI_ISsend`. In practice, `MPI_IBsend` is basically
useless. `MPI_ISsend` is generally recommended as it avoids the buffer problem of `MPI_ISend`.

There is also a test function, `MPI_Test`, that can be used to check if a non-blocking operation has completed. 
```c++
int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status);
```
These just returns a boolean flag, so you can use it in a loop to check if all non-blocking operations have completed.
Useful for controller worker communication. 

In practice, what you tend to do is send non blocking communication, do as much work as you can without the
communication, then wait for the communication to complete. This is a good way to hide latency.

Synchronous mode affects what completion means. After a wait on `MPI_Issend`, you know the routine has
completed, the message has been sent, you can overwrite the send buffer. After a wait on MPI_Isend, you know the
routine has completed, the message may have sent, or may be in the MPI buffer. In either you can overwrite the
send buffer. You must not access send or receive buffers until communications are complete. Do not write to send buffer 
until after a wait on `Issend`/`Isend`, and do not read from receive buffer until after a wait on `Irecv`.

Routines exist in MPI to test multiple non-blocking operations at once. Syntax gets a bit complex, but can be useful
for large numbers of non-blocking operations. These include `MPI_Waitsome`, `MPI_Testsome`, `MPI_Waitall`, 
`MPI_Testall`, `MPI_Waitany`, and `MPI_Testany`. Using these and non-blocking communications is often the most
efficient way to use MPI.

There is a routine called `MPI_Sendrecv`, that combines a send and a receive into a single call. This can be useful
to avoid deadlocks, but is not as generally useful as non-blocking communication. Works ok in a structured problem,
but breaks down horribly in an unstructured problem.

### Exercise 5: Rotating Information Around a Ring

See linked exercises in README.md for details and `exercises/My_Solutions/Rotate_Info` for my solution.


General advice is to wait as long as you can before waiting for non-blocking operations. This is a good way to 
give MPI as much time as possible to make the communication. Reducing waiting time. 


### Collective Communication

Implementation of complex collective communication patterns that are commonly used in parallel programming.

Call by all processes in a communicator, all processes must call the same function. They can have different
roles in the collective (e.g., broadcast), but they must all call the function. Synchronisation may or
may not occur, but can treat like they do as they will if needed. Can often think about there being a Barrier at the 
start and end, no need to add these yourself.

Collection operations are blocking (but there are non-blocking versions that are not commonly used). Do not
support tags as all participating together. Receive buffers, must be exactly the right size, unlike point-to-point 
communication. This is to avoid the checking overhead when have lots and lots of processes.

Barrier synchronisation. Rarely needed for correctness, but often needed for timing. 
```c++
int MPI_Barrier(MPI_Comm communicator);
```

Broadcast, send data from one process to all processes. 
```c++
int MPI_Bcast(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm communicator);
```
where `buf` is the data to be sent, `count` is the number of data items, `datatype` is the type of the data, `root` is
the rank of the process that is sending the data, and `communicator` is the communicator. All processes must call this
function, but only the root process needs to pass data. The rest will receive the data into their buffer. We do this
rather than point-to-point communication as it is optimized for the network, and can be done in parallel. 
Can also use specialized hardware you don't have access to.  Lots of  work has gone into making this efficient, so 
don't try to do it yourself. A good implementation will scale as  log the number of processes, a bad one will scale as 
the number of processes. 

Scatter, like broadcast but sends different data to each process. 
```c++
int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm communicator);
```
where `sendbuf` is the data to be sent, `sendcount` is the number of data items to send, `sendtype` is the type of the
data to send, `recvbuf` is the buffer to receive the data into, `recvcount` is the number of data items to receive,
`recvtype` is the type of the data to receive, `root` is the rank of the process that is sending the data, and
`communicator` is the communicator. The root process sends the data, and the other processes receive the data into
their buffer. `sendcount` and `recvcount` must be the same (unless the types differ, but that is rare), this is how many 
data each process will receive. The size of the original data is set implicitly by the size of the communicator and 
`recvcount`. It is a bit subtle, so be careful. `sendbuf` is only used by the root process, the other processes ignore 
it, passing `NULL` is fine.

The inverse of scatter is gather, where each process sends data to the root process. 
```c++
int MPI_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm communicator);
```
where `sendbuf` is the data to be sent, `sendcount` is the number of data items to send, `sendtype` is the type of the
data to send, `recvbuf` is the buffer to receive the data into, `recvcount` is the number of data items to receive,
`recvtype` is the type of the data to receive, `root` is the rank of the process that is receiving the data, and
`communicator` is the communicator. The root process receives the data, and the other processes send the data from
their buffer. `sendcount` and `recvcount` must be the same (unless the types differ, but that is rare), this is how many
data each process will send. The size of the original data is set implicitly by the size of the communicator and
`sendcount`. It is a bit subtle, so be careful. `recvbuf` is only used by the root process, the other processes ignore it,
passing `NULL` is fine.

There are more general routines for gather and scatter, that can transfer none fixed amount of data, and the data
does not need to be contiguous in memory. These are `MPI_Gatherv` and `MPI_Scatterv`. These are more complex, but
can be more efficient. Derived data types can be used to do even more general things like 3D arrays, but these are
quite complex see later.

Reduce, combines data from all processes to one process. It can sum, multiply, find the maximum, minimum, etc.
Even do user-defined operations. Predefind operations are `MPI_SUM`, `MPI_PROD`, `MPI_MAX`, `MPI_MIN`, `MPI_LAND`,
`MPI_BAND`, `MPI_LOR`, `MPI_BOR`, `MPI_LXOR`, `MPI_BXOR`. L, B, and X are logical, bitwise, and exclusive or. Can
also do maximum with location, and minimum with location. Sum, is the most common. 
```c++
int MPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op operation, int root, MPI_Comm communicator);
```
where `sendbuf` is the data to be sent, `recvbuf` is the buffer to receive the data into, `count` is the number of data
items, `datatype` is the type of the data, `operation` is the operation to perform, `root` is the rank of the process
that is receiving the data, and `communicator` is the communicator. The root process receives the data, and the other
processes send the data from their buffer. `sendbuf` is only used by the other processes, the root process ignores it,
passing `NULL` is fine. `recvbuf` is only used by the root process, the other processes ignore it, passing `NULL` is 
fine. Operations are applied elementwise, so the data must be the same size.

User defined reduction operators are rarely needed. But can be useful for complex operations. Look it up if you
need it. Operations need not commute, but they must be associative. MPI should be told if it commutes though, as it
can optimize for this.

Other collective operations include:
* `MPI_Allreduce`: Everyone knows answer. Same syntax but just get rid of the root argument. Avoids roundings errors.
* `MPI_Reduce_scatter`: Reduce then scatter
* `MPI_Scan`: Like reduce, but each process gets the result of the reduction up to that point. E.g., 0, 0+1, 0+1+2, etc.


### Exercise 6: Collective Communication

See linked exercises in README.md for details and `exercises/My_Solutions/Collective_Communication` for my solution.

The resulting timings of the `MPI_Allreduce` method versus the rotating ring method are as follows:

| Processors | Iterations | Rotate Info (s) | Reduce Time (s) |
|------------|------------|-----------------|-----------------|
| 3          | 100000     | 0.158817        | 0.0499179       |
| 9          | 100000     | 1.49768         | 0.175197        |
| 27         | 100000     | 4.49493         | 0.233942        |
| 81         | 100000     | 21.6849         | 0.459456        |
| 243        | 100000     | 123.012         | 0.897147        |

The resulting scaling of the too methods can be seen in the following plot:

![Scaling](exercises/My_Solutions/Collective/total_timings.png)

From which we see the all reduce method is both much faster and scales better than the rotating ring method.
The time scaling with processes of each method are found to be as the 
1.5 and 0.6th power respectively. So the all reduce method scales significantly better than the rotating ring method.


### Virtual Topologies

Convenient process naming to simply communication pattern and match mental model of the problem.
Makes code more readable and maintainable. In principle, MPI can also optimize for these topologies, but in practice
this is rarely significant.

We use a virtual topology, by creating a new communicator, which MPI provides mapping functions. Mapping
function compute processor ranks, based on the topology naming scheme. We shall consider
Cartesian topologies, which are the most common, and regular structured, but this is not a requirement,
you can have irregular graph topologies like a tree.

For a Cartesian topology also need to define boundary conditions, e.g., a 2D finite cylinder
is not the same as a sphere. Hence, the name topology. Processors are identified by Cartesian
coordinates. 

Optionally MPI can reorder the ranks, to optimize communication. This is not always a good idea, as it can
make debugging harder. Generally, this is done by minimizing inter-node communication. This is rarely used,
and often doesn't work well, but it is there if you need it.

To create a Cartesian topology, you need to define the number of dimensions, and the size of each dimension.
```c++
int MPI_Cart_create(MPI_Comm communicator, int ndims, int *dims, int *periods, int reorder, MPI_Comm *new_communicator);
```
where `communicator` is the communicator to create the topology in, `ndims` is the number of dimensions, `dims` is the
size of each dimension, `periods` is the periodicity of each dimension, `reorder` is whether to reorder the ranks, and
`new_communicator` is the new communicator. `dims` is an array of integers of length `ndims`, `periods` is an array of
booleans of length `ndims`, and `new_communicator` is a pointer to a communicator. `periods` is an array of booleans
that defines if the topology is periodic in each dimension. This is useful for things like a 2D grid, where the top
row is connected to the bottom row, and the left column is connected to the right column. `reorder` is a boolean that
defines if the ranks should be reordered to optimize communication. 

There is a helper function to suggest dims to create given a number of nodes and
the number of dimensions. This is `MPI_Dims_create`. This is useful for creating a balanced grid.
```c++
int MPI_Dims_create(int nnodes, int ndims, int *dims);
```
where `nnodes` is the number of nodes, `ndims` is the number of dimensions, and `dims` is the size of each dimension.
`dims` is an array of integers of length `ndims`, and acts to constrain the number of nodes in each dimension.
It is 0 if it is not constrained and the number of nodes in that dimension if it is constrained.

To get the Cartesian coordinates of a process, you can use `MPI_Cart_coords`.
```c++
int MPI_Cart_coords(MPI_Comm communicator, int rank, int maxdims, int *coords);
```
where `communicator` is the communicator, `rank` is the rank of the process, `maxdims` is the maximum number of dimensions,

To get the rank of a process from its Cartesian coordinates, you can use `MPI_Cart_rank`.
```c++
int MPI_Cart_rank(MPI_Comm communicator, int *coords, int *rank);
```
where `communicator` is the communicator, `coords` is the Cartesian coordinates, and `rank` is the rank of the process.

To get the neighbour in a certain direction, you can use `MPI_Cart_shift`.
```c++
int MPI_Cart_shift(MPI_Comm communicator, int direction, int disp, int *source, int *dest);
```
where `communicator` is the communicator, `direction` is the direction to shift in, `disp` is the distance to shift,
`source` is the rank of the source process, and `dest` is the rank of the destination process. `direction` is the
dimension to shift in, `disp` is the distance to shift, and `source` and `dest` are the ranks of the source and
destination processes. This is useful for neighbour communication. `source` is not your rank
but the rank of the process that is sending you data if communicating in the direction of the shift. 

Non-existent neighbours are returned as `MPI_PROC_NULL`. This is a special value that is not a valid rank, and
can be used to check if a neighbour exists. Also allows you to handle boundary conditions in a simple way
by sending to the non-existent neighbour.

Can partition the Cartesian grid into sub-grids, and create a new communicator for each sub-grid. This can be
useful for parallelize a problem that is naturally divided into sub-problems. Each slice of the grid can then
perform its own collective communications. 

There are more powerful features, so if you find yourself doing something complex, look it up.
There might be a standard way to do it.


### Derived Data Types

We have already seen the MPI basic types. There are also derived types like vector, structs and others.

These can be used to send non-contiguous data, like a column of a 2D array or a structure. 

Can define new MPI types, that once registered that can then be used like the basic types.
You need to register an appropriate `MPI_Op` if you want to use the type in a collective operation
like `MPI_Reduce`.

The types are defined as a list of basic types with displacements in bytes. 

The simplest derived data type is a contiguous block of data, which is just a block of data of the same type. 
```c++
int MPI_Type_contiguous(int count, MPI_Datatype oldtype, MPI_Datatype *newtype);
```
where `count` is the number of basic data types, `oldtype` is the basic data type, and `newtype` is the new data type.
Think 3D coordinates, or a row of a 2D array. Could do it otherway's but nice to have a standard way.
These are also useful as intermediate types for more complex types.

Once created you have to commit a type, this is to allow for optimization and to inform MPI of what it could
see. 
```c++
int MPI_Type_commit(MPI_Datatype *datatype);
```

Vector data types are normally more useful. They are defined by a count, number of blocks, stride the distance between
blocks, and the block length. This is because it corresponds to subsections of a 2D array, a common problem.
Array mapping in C and Fortran is different. This is the cause of a lot of confusion.
```c++
int MPI_Type_vector(int count, int blocklength, int stride, MPI_Datatype oldtype, MPI_Datatype *newtype);
```
where `count` is the number of blocks, `blocklength` is the number of basic data types in each block, `stride` is the
distance between the start of each block in multiples of the basic data type, `oldtype` is the basic data type, and
`newtype` is the new data type. This is useful for sending a column of a 2D array.

Note MPI avoids talking about bytes to maintain portability.

Structures are a pain as you need to know offsets which can be compiler dependent.
So you have to extract those at runtime. 


### Case Study: Image Processing

Start with a big array. Split into strips, and assign each to a process. Update will depend on neighbours.
Halo trick (in this case lines) used to allow processes to update their own data.

Edge detection. Take a pixel, and look at the difference between it and the mean of its neighbours. Our problem, is 
to go the other way. This will take hundreds of iterations. 

Going to use white fixed boundary conditions. 

This is the discrete Laplacian, which is a common problem in physics. We are essentially solving
the Poisson equation using the Jacobi solver,
```
new(i,j) = 0.25 * (old(i-1,j) + old(i+1,j) + old(i,j-1) + old(i,j+1) - edge(i,j))
```
where `old` is the old image, `new` is the new image, and `edge` is the edge image.

In C, second dimension is contiguous, so split along the first dimension.




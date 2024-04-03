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
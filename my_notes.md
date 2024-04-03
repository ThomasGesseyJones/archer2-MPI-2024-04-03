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
a library call. Could have a fully parallelized program, but none have taken off.

Parallel paradigm:
* Multiple processes (numbered), each with their own memory space.
* Processes communicate by sending messages via MPI to a communication network.
* Processes can be on the same node or different nodes.
* We don't care how the communication network works, just that it does, MPI deals with this.
* Typically, in a supercomputer, the memory is also distributed across the nodes, so the communication network is
  also the memory network.
* Note each compute node can have multiple processes, but MPI hides this from you.

Process communication in MPI is a two-ended process, both the sender needs to send, and the receiver needs to receive.
This is analogous to emails.
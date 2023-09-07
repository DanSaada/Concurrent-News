# Concurrent-News
A multithreaded news system that emulates the synchronized operations within a public news publishing company, showcasing the parallel workflows involved in news dissemination.

## About

The Producer-consumer problem is a classic synchronization and concurrency challenge in computer science. It involves coordinating the interaction between two types of processes: producers, which generate data or items, and consumers, which consume or process these items. The primary issue to address is how to ensure that producers don't produce items too quickly for consumers to handle, and conversely, that consumers don't attempt to consume items that haven't been produced yet. This problem often arises in multi-threaded or distributed systems, and various synchronization mechanisms, such as locks, semaphores, and bounded buffers, are employed to manage access to shared resources and prevent race conditions, ensuring the safe and efficient exchange of data between producers and consumers.

In the context of this Multithreaded News System project, the Producer-consumer problem is fundamentally intertwined with the solution's design and functionality.    
The system involves the participation of four distinct active entities: Producers, Dispatcher, Co-Editors, and Screen Manager.         
Producers generate string representations of news stories and transmit them to the Dispatcher. The Dispatcher, in turn, categorizes these stories based on their type and forwards them to the corresponding Co-Editors. The Co-Editors are responsible for editing the stories and then passing them on to the Screen Manager for display. Each of these entities operates within its own thread, enabling the system to function concurrently.


## Implementation

The Producer queues and the shared queue among the Co-Editors are both designed as bounded buffers. These bounded buffers primarily support two essential operations: Push and Pop. The Push operation facilitates the insertion of an object into the buffer, while the Pop operation handles the removal and retrieval of the first object from the buffer.

To ensure thread safety and efficient operation, these bounded buffers are implemented using synchronization mechanisms like mutexes and counting semaphores.

The Dispatcher plays a crucial role in the system as it scans the Producer's queues utilizing a [round-robin](https://en.wikipedia.org/wiki/Round-robin_scheduling) algorithm. Additionally, it is responsible for sorting the articles based on their respective types.

The system reads a configuration file to ascertain several crucial parameters, including the number of Producers, the quantity of strings each Producer should generate, and the size of the queues. The configuration file follows this format:

PRODUCER 1 [number of items] queue size = [size]

PRODUCER 2 [number of items] queue size = [size]

...

PRODUCER n [number of items] queue size = [size]

Co-Editor queue size = [size]

In this format, each line corresponds to a specific Producer, indicating the Producer's number, the desired number of items it should produce, and the size of its associated queue. The last line denotes the queue size for Co-Editors, indicating the capacity of their shared queue.


## Installing And Executing
    
To clone and run this application, you'll need [Git](https://git-scm.com) installed on your computer.
  
From your command line:

  
```bash

# Clone this repository:
git clone https://github.com/DanSaada/Concurrent-News

# Go into the repository:
cd Concurrent-News

# Compile using Makefile:
 make

# An ex3.out file will be created. You can run:
 make run

# Or directly give add the path to the configuration file.
 ./ex3.out conf.txt

```

## Author
- [Dan Saada](https://github.com/DanSaada)

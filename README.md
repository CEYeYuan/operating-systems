Operating-Systems
<a href="http://www.eecg.toronto.edu/~ashvin/courses/ece344/current/">Course Website </a>

# Operating-Systems
>Lab Projects for Operating Systems at U of T, You can use my code as reference but please do not plagiarize.
Through this course, I was able to grasp the lower level intricacies of operating systems and how they are designed.  
In order to eliminate the various complexities of computer systems and dealing with hardware, the operating system has to handle all these tasks behind the scenes and provide a clean simple abstraction (user interface) for the basic computer user. This course discussed the concepts of 3 types of abstractions: cpu-threads, physical memory-virtual memory and file system.

**Authors**: Ye Yuan

**Technologies Used:** C

**Labs Include:**
  - **Lab 1** C programming warmup, including building a non-memory-leaked singly linked list and hashmap.
  - **Lab 2:** Designed a User Level Threads Package. Implemented thread yield, thread exit, and thread create functions. 
      No pre-emption, so threads have to voluntarily yield CPU to each other.
  - **Lab 3:** This lab builds on Lab 2. Implemented condition variables and blocking locks. 
  - **Lab 4:** Solution for the producer and consumer fixed-buffer concurrency problem (fixed-threads pool webserver) using pthreads, 
mutex locks and condition variables.
   - **Lab 5:** Based on lab4, added in caching. Using LRU cache to improve the performance and cache hit rate.
   - **Lab 6:** Working on a simple file system which supports very large (4TB) files. Structures include super blocks, inode blocks and data blocks.

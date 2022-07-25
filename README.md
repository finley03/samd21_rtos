# SAMD21 RTOS
## Cooperative first real time operating system designed to run on the SAMD21 microcontroller

### Features
* Uses earliest deadline first scheduling, defaults to round-robin if deadlines aren't set
* Supports preemption on one process at a time
* All memory for processes is currently manually allocated
* API is fully thread safe, with a few exceptions

### Compilation
#### Optimization
* Does not compile with "-O0"
* Will compile and run with "-O1"
* Will compile and run with "-O2"
* Will compile and run with "-O3"
* Will compile and run with "-Os"
* Will compile and run with "-Og"
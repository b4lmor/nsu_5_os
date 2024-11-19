# Report 

## Spinlock:
- Queue capacity: 1.000.000
- Experiment time: 250 sec
- Thread CPUs: same
- Use sched_yield(): yes
- Emulate sleep: no
---
- Processor load: ~25%
- User mode / Kernel mode (ms): 249293 / 159
- Queue statistics:
  + Queue occupancy: 0 - 100% (various)
  + Read-write attempts: add < get
  + Read-write counts: 1514343320 | 1514343320
---

## Mutex
- Queue capacity: 1.000.000
- Experiment time: 250 sec
- Thread CPUs: same
- Use sched_yield(): yes
- Emulate sleep: no
---
- Processor load: ~25%
- User mode / Kernel mode (ms): 249343 / 176
- Queue statistics:
    + Queue occupancy: 0 - 10%
    + Read-write attempts: add < get
    + Read-write counts: 1799105766 | 1799090020 
---

## Condition variable
- Queue capacity: 1.000.000
- Experiment time: 250 sec
- Thread CPUs: same
- Use sched_yield(): yes
- Emulate sleep: no
---
- Processor load: ~25%
- User mode / Kernel mode (ms): 189999 / 59468
- Queue statistics:
    + Queue occupancy: 0 - 4% (mostly: 0-1 element)
    + Read-write attempts: add < get (10k-30k diff)
    + Read-write counts: 1480514009 | 1480478120
---

## Semaphore
- Queue capacity: 1.000.000
- Experiment time: 250 sec
- Thread CPUs: same
- Use sched_yield(): yes
- Emulate sleep: no
---
- Processor load: ~25%
- User mode / Kernel mode (ms): 248969 / 286
- Queue statistics:
    + Queue occupancy: 0 - 6%
    + Read-write attempts: add < get
    + Read-write counts: 1776317701 | 1776291990
---

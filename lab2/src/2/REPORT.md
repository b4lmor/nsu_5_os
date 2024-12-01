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
  + Read-write counts: 1514343320 | 1514343320 (10)
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
    + Read-write counts: 1799105766 | 1799090020 (10) 
---

## Condition variable
- Queue capacity: 1.000.000
- Experiment time: 250 sec
- Thread CPUs: same
- Use sched_yield(): yes
- Emulate sleep: no
---
- Processor load: ~25%
- User mode / Kernel mode (ms): 186501 / 63633
- Queue statistics:
    + Queue occupancy: 0 - 4% (mostly: 0-1 element)
    + Read-write attempts: add < get (10k-30k diff)
    + Read-write counts:  1925174005 | 1925128931 (10)
---

## Semaphore
- Queue capacity: 1.000.000
- Experiment time: 250 sec
- Thread CPUs: same
- Use sched_yield(): yes
- Emulate sleep: no
---
- Processor load: ~25%
- User mode / Kernel mode (ms): 133219 / 116779
- Queue statistics:
    + Queue occupancy: 0 - 3%
    + Read-write attempts: add ~= get
    + Read-write counts: 1098217147 | 1098217146 (10)
---

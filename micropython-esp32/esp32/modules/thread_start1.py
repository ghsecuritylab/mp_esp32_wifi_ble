# test basic capability to start a new thread
#
# MIT license; Copyright (c) 2016 Damien P. George on behalf of Pycom Ltd

try:
    import utime as time
except ImportError:
    import time
import _thread

def foo1():
    print("makeblock1\n")

def foo2():
    print("makeblock2\n")

def thread_entry1(n):
    for i in range(n):
        foo1()
        time.sleep(0.1)

def thread_entry2(n):
    for i in range(n):
        foo2()
        time.sleep(0.1)
        
_thread.start_new_thread(thread_entry1, (5,))
_thread.start_new_thread(thread_entry2, (10,))

# wait for threads to finish
print('done')

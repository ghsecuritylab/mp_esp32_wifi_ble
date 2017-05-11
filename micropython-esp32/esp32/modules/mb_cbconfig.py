import time
import makeblockclass
from mb_callback import makeblock_callback

def func_default(data=None):
    a=makeblock_dcmotor()
    if data==None:
        a.run(0,10)
    else:
        a.run(data,10)

def func0_default(data=None):
    a=makeblock_dcmotor()
    if data==None:
        a.run(100,10)
    else:
        a.run(data,10)

def func1_default(data=None):
    a=makeblock_dcmotor()
    if data==None:
        a.run(-100,10)
    else:
        a.run(data,10)

def func2_default(data=None):
    a=makeblock_dcmotor()
    if data==None:
        a.run(100,9)
    else:
        a.run(data,9)

def func3_default(data=None):
    a=makeblock_dcmotor()
    if data==None:
        a.run(-100,9)
    else:
        a.run(data,9)
        

button_callback=[None,None,None,None]
button_callback[0]=makeblock_callback(func0_default)
button_callback[1]=makeblock_callback(func1_default)
button_callback[2]=makeblock_callback(func2_default)
button_callback[3]=makeblock_callback(func3_default)


   

from makeblock import linefollower

class makeblock_linefollower(object):
    def __init__(self,port=1):
        self.port=port
        self.lf=linefollower()
        
    def set_port(self,port):
        self.port=port
        
    def get_value(self,port=None):
        if port==None:
            return self.lf.value(self.port)
        else:
             return self.lf.value(port)
            
    def mhelp(self):
        print("makeblock:please call get_value(port) to get the linefollower value")
    





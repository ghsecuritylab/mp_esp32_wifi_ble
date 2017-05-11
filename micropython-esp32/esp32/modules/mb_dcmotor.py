from makeblock import dcmotor

class makeblock_dcmotor(object):
    def __init__(self,port=10):
        self.port=port
        self.dc=dcmotor()

    def set_port(self,port):
        self.port=port
        
    def run(self,speed,port=None):
        if port==None:
            self.dc.run(self.port,speed)
        else:
            self.dc.run(port,speed)
            
    def mhelp(self):
        print("makeblock:please call run(speed,port) to control the dcmotor")
    





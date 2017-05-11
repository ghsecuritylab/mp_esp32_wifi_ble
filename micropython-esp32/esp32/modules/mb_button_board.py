import time
from makeblock import button_board

class makeblock_button(object):
    def __init__(self,para=1):
        self.butt=para
        self.lsb=button_board()
        
    def get_value(self,butt=None):
        if butt==None:
            return self.lsb.value(self.butt)
        else:
            return self.lsb.value(butt)
        
    def mhelp(self):
        print("makeblock:please call get_value(butt) to get the button status")
    




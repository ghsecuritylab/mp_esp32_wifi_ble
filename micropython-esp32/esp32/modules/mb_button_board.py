import time
from makeblock import button_board

class makeblock_button(object):
    def __init__(self,para):
        self.para=para

    def get_value(self):
        lsb=button_board()
        return lsb.value(self.para)
        
def mb_help(self):
    print("makeblock:please call get_value() to get the button status")
    
makeblock_button.mb_help=mb_help




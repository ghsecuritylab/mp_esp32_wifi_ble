from makeblock import led_matrix

class makeblock_ledmatrix(object):
    def __init__(self,port=0):
        self.port=port
        self.lm=led_matrix()
        
    def set_port(self,port):
        self.port=port
        
    def show_char(self,p_x,p_y,num,*data_num,port=None):
        mode=1
        if port==None:
            self.lm.show(self.port,mode,p_x,p_y,num,*data_num)
        else:
            self.lm.show(port,mode,p_x,p_y,num,*data_num)
            
    def draw(self,p_x,p_y,*data_16,port=None):
        mode=2
        if port==None:
            self.lm.show(self.port,mode,p_x,p_y,*data_16)
        else:
            self.lm.show(port,mode,p_x,p_y,*data_16)

    def show_time(self,pot,*data_2,port=None):
        mode=3
        if port==None:
            self.lm.show(self.port,mode,pot,*data_2)
        else:
            self.lm.show(port,mode,pot,*data_2)
            
    def show_num(self,da_1,port=None):
        mode=4
        if port==None:
            self.lm.show(port,mode,da_1)
        else:
            self.lm.show(port,mode,da_1)
        
    def mhelp(self):
        print("makeblock:please call sho_char()、draw()、show_time、show_num() to control the dcmotor")
    





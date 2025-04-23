from typing import Optional
from pydantic import BaseModel

class vhBase(BaseModel):
    led1: Optional[int]
    led2: Optional[int]
    led3: Optional[int]
    bloqueos: Optional[int]

class vhcreate(vhBase):
 pass

class vh(vhBase):
    id:int 
    user_id: Optional[int]

    class Config:
        from_attributes = True
      


    


    
    

from pydantic import BaseModel
from datetime import datetime

class CreatePressureRequest(BaseModel):
    pressure: float

class PressureDTO(BaseModel):
    id:int
    pressure:float
    date:datetime
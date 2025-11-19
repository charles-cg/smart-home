from pydantic import BaseModel
from datetime import datetime

class CreateTemperatureRequest(BaseModel):
    temperature: float

class TemperatureDTO(BaseModel):
    id:int
    temperature:float
    date:datetime
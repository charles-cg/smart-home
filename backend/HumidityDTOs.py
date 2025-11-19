from pydantic import BaseModel
from datetime import datetime

class CreateHumidityRequest(BaseModel):
    humidity: float

class HumidityDTO(BaseModel):
    id:int
    humidity:float
    date:datetime
from pydantic import BaseModel
from datetime import datetime

class CreateRainRequest(BaseModel):
    rain: float

class RainDTO(BaseModel):
    id:int
    rain:float
    date:datetime
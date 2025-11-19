from pydantic import BaseModel
from datetime import datetime

class CreateSmokeRequest(BaseModel):
    smoke: float

class SmokeDTO(BaseModel):
    id:int
    smoke:float
    date:datetime
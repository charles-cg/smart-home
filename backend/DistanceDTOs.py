from pydantic import BaseModel
from datetime import datetime

class CreateDistanceRequest(BaseModel):
    distance: float

class DistanceDTO(BaseModel):
    id:int
    distance:float
    date:datetime
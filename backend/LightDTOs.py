from pydantic import BaseModel
from datetime import datetime

class CreateLightRequest(BaseModel):
    light: float

class LightDTO(BaseModel):
    id:int
    light:float
    date:datetime
from typing import Union

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from HumidityDTOs import CreateHumidityRequest, HumidityDTO
from HumidityRepository import HumidityRepository

app = FastAPI()


origins = [
    "http://localhost:8000",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)



@app.get("/")
def read_root():
    return {"Hello": "World"}


@app.post("/humidity/create")
def create_humedad(dto:CreateHumidityRequest):
    humedad_repository = HumidityRepository()
    humedad_repository.insert_data(dto.humidity)
    return {"message":"Data inserted"}
    
@app.get("/humidity/list")
def get_humedades()->list[HumidityDTO]:
    humidity_repository = HumidityRepository()
    response = humidity_repository.get_data()
    return response
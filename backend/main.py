from typing import Union

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from HumidityDTOs import CreateHumidityRequest, HumidityDTO
from HumidityRepository import HumidityRepository

from SmokeDTOs import CreateSmokeRequest, SmokeDTO
from SmokeRepository import SmokeRepository

from LightDTOs import CreateLightRequest, LightDTO
from LightRepository import LightRepository

from DistanceDTOs import CreateDistanceRequest, DistanceDTO
from DistanceRepository import DistanceRepository

from TemperatureDTOs import CreateTemperatureRequest, TemperatureDTO
from TemperatureRepository import TemperatureRepository

from PressureDTOs import CreatePressureRequest, PressureDTO
from PressureRepository import PressureRepository

from RainDTOs import CreateRainRequest, RainDTO
from RainRepository import RainRepository

app = FastAPI()

origins = [
    "http://localhost:3000",
    "http://localhost:8000",
    "http://127.0.0.1:8000",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)



@app.get("/")
def read_root():
    return {"Hello": "World"}

"""Humidity"""
@app.post("/humidity/create")
def create_humidity(dto:CreateHumidityRequest):
    humedad_repository = HumidityRepository()
    humedad_repository.insert_data(dto.humidity)
    return {"message":"Data inserted"}
    
@app.get("/humidity/list")
def get_humidity()->list[HumidityDTO]:
    humidity_repository = HumidityRepository()
    response = humidity_repository.get_data()
    return response

"""Smoke"""
@app.post("/smoke/create")
def create_smoke(dto:CreateSmokeRequest):
    smoke_repository = SmokeRepository()
    smoke_repository.insert_data(dto.smoke)
    return {"message":"Data inserted"}
    
@app.get("/smoke/list")
def get_smoke()->list[SmokeDTO]:
    smoke_repository = SmokeRepository()
    response = smoke_repository.get_data()
    return response

"""Light"""
@app.post("/light/create")
def create_light(dto:CreateLightRequest):
    light_repository = LightRepository()
    light_repository.insert_data(dto.light)
    return {"message":"Data inserted"}
    
@app.get("/light/list")
def get_light()->list[LightDTO]:
    light_repository = LightRepository()
    response = light_repository.get_data()
    return response

"""Distance"""
@app.post("/distance/create")
def create_distance(dto:CreateDistanceRequest):
    distance_repository = DistanceRepository()
    distance_repository.insert_data(dto.distance)
    return {"message":"Data inserted"}
    
@app.get("/distance/list")
def get_distance()->list[DistanceDTO]:
    distance_repository = DistanceRepository()
    response = distance_repository.get_data()
    return response

"""Temperature"""
@app.post("/temperature/create")
def create_temperature(dto:CreateTemperatureRequest):
    temperature_repository = TemperatureRepository()
    temperature_repository.insert_data(dto.temperature)
    return {"message":"Data inserted"}
    
@app.get("/temperature/list")
def get_temperature()->list[TemperatureDTO]:
    temperature_repository = TemperatureRepository()
    response = temperature_repository.get_data()
    return response

"""Pressure"""
@app.post("/pressure/create")
def create_pressure(dto:CreatePressureRequest):
    pressure_repository = PressureRepository()
    pressure_repository.insert_data(dto.pressure)
    return {"message":"Data inserted"}
    
@app.get("/pressure/list")
def get_pressure()->list[PressureDTO]:
    pressure_repository = PressureRepository()
    response = pressure_repository.get_data()
    return response

"""Rain"""
@app.post("/rain/create")
def create_rain(dto:CreateRainRequest):
    rain_repository = RainRepository()
    rain_repository.insert_data(dto.rain)
    return {"message":"Data inserted"}
    
@app.get("/rain/list")
def get_rain()->list[RainDTO]:
    rain_repository = RainRepository()
    response = rain_repository.get_data()
    return response
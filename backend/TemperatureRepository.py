from datetime import datetime
import mysql.connector
from mysql.connector import Error


class TemperatureRepository:

    def __init__(self):
        self.config={
            "host":"localhost",
            "user":"root",
            "password":"Nihon2016*",
            "database":"smartHomeDB",
            "port":3306
        }
    
    def get_connection(self):
        """Crea y devuelve una conexion a la db"""
        return mysql.connector.connect(**self.config)
    
    def insert_data(self, temperature):
        """Método que inserta un alumno a la db"""
        try:
            connection = self.get_connection()
            cursor = connection.cursor()
            query= "INSERT INTO temperature (temperature,date) VALUES (%s,%s)"
            cursor.execute(query,(temperature,datetime.now()))
            connection.commit()
        except Error as e:
            print(f"Error inserting temperature data: {e}")   
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()

    def get_data(self):
        values = []
        try:
            connection = self.get_connection()
            cursor= connection.cursor()
            query= "SELECT * FROM temperature"
            cursor.execute(query)
            valores_raw = cursor.fetchall()
            for value_raw in valores_raw:
                values.append({
                    "id":value_raw[0],
                    "temperature":value_raw[1],
                    "date":value_raw[2]
                })
            return values
        except Error as e:
            print(f"Error obtaining temperature data: {e}")
            return values
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()


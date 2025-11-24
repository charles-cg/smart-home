from datetime import datetime
import mysql.connector
from mysql.connector import Error


class DistanceRepository:

    def __init__(self):
        self.config={
            "host":"localhost",
            "user":"root",
            "password":"Ccg2004-ccg",
            "database":"smartHomeDB",
            "port":3306
        }
    
    def get_connection(self):
        """Crea y devuelve una conexion a la db"""
        return mysql.connector.connect(**self.config)
    
    def insert_data(self, distance):
        """Método que inserta una distancia a la db"""
        try:
            connection = self.get_connection()
            cursor = connection.cursor()
            query= "INSERT INTO distance (distance,date) VALUES (%s,%s)"
            cursor.execute(query,(distance,datetime.now()))
            connection.commit()
        except Error as e:
            print(f"Error inserting distance data: {e}")   
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()

    def get_data(self):
        values = []
        try:
            connection = self.get_connection()
            cursor= connection.cursor()
            query= "SELECT * FROM distance"
            cursor.execute(query)
            values_raw = cursor.fetchall()
            for value_raw in values_raw:
                values.append({
                    "id":value_raw[0],
                    "distance":value_raw[1],
                    "date":value_raw[2]
                })
            return values
        except Error as e:
            print(f"Error obtaining distance data: {e}")
            return values
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()


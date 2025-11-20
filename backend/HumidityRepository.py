from datetime import datetime
import mysql.connector
from mysql.connector import Error


class HumidityRepository:

    def __init__(self):
        self.config={
            "host":"localhost",
            "user":"root",
            "password":"Ccg2004-ccg", #change pw depending on user
            "database":"smartHomeDb",
            "port":3306
        }
    
    def get_connection(self):
        """Crea y devuelve una conexion a la db"""
        return mysql.connector.connect(**self.config)
    
    def insert_data(self, humidity):
        """Método que inserta un alumno a la db"""
        try:
            connection = self.get_connection()
            cursor = connection.cursor()
            query= "INSERT INTO humidity (humidity,date) VALUES (%s,%s)"
            cursor.execute(query,(humidity,datetime.now()))
            connection.commit()
        except Error as e:
            print(f"Error al insertar los datos de humedad: {e}")   
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()

    def get_data(self):
        valores = []
        try:
            connection = self.get_connection()
            cursor= connection.cursor()
            query= "SELECT * FROM humidity"
            cursor.execute(query)
            valores_raw = cursor.fetchall()
            for valor_raw in valores_raw:
                valores.append({
                    "id":valor_raw[0],
                    "humidity":valor_raw[1],
                    "date":valor_raw[2]
                })
            return valores
        except Error as e:
            print(f"Error al obtener los datos de humedad: {e}")
            return valores
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()


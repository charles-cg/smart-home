from datetime import datetime
import mysql.connector
from mysql.connector import Error


class SmokeRepository:

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
    
    def insert_data(self, smoke):
        """Método que inserta un alumno a la db"""
        try:
            connection = self.get_connection()
            cursor = connection.cursor()
            query= "INSERT INTO smoke (smoke,date) VALUES (%s,%s)"
            cursor.execute(query,(smoke,datetime.now()))
            connection.commit()
        except Error as e:
            print(f"Error inserting smoke data: {e}")   
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()

    def get_data(self):
        values = []
        try:
            connection = self.get_connection()
            cursor= connection.cursor()
            query= "SELECT * FROM smoke"
            cursor.execute(query)
            values_raw = cursor.fetchall()
            for value_raw in values_raw:
                values.append({
                    "id":value_raw[0],
                    "humidity":value_raw[1],
                    "date":value_raw[2]
                })
            return values
        except Error as e:
            print(f"Error obtaining smoke data: {e}")
            return values
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()


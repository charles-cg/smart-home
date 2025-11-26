from datetime import datetime
import mysql.connector
from mysql.connector import Error


class RainRepository:

    def __init__(self):
        self.config={
            "host":"localhost",
            "user":"root",
            "password":"Ccg2004-ccg", #change pw depending on user
            "database":"smartHomeDb",
            "port":3306
        }
    
    def get_connection(self):
        """creates and returns a connection with the db"""
        return mysql.connector.connect(**self.config)
    
    def insert_data(self, rain):
        """Inserts rain value to db"""
        try:
            connection = self.get_connection()
            cursor = connection.cursor()
            query= "INSERT INTO rain (rain,date) VALUES (%s,%s)"
            cursor.execute(query,(rain,datetime.now()))
            connection.commit()
        except Error as e:
            print(f"Error inserting rain data: {e}")   
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()

    def get_data(self):
        values = []
        try:
            connection = self.get_connection()
            cursor= connection.cursor()
            query= "SELECT * FROM rain"
            cursor.execute(query)
            values_raw = cursor.fetchall()
            for value_raw in values_raw:
                values.append({
                    "id":value_raw[0],
                    "rain":value_raw[1],
                    "date":value_raw[2]
                })
            return values
        except Error as e:
            print(f"Error obtaining rain data: {e}")
            return values
        finally:
            if connection.is_connected():
                cursor.close()
                connection.close()

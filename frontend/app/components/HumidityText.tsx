"use client";
import React from "react";
import axios from "axios";
import { useEffect, useState } from "react";

interface HumidityDTO {
  id: number;
  humidity: number;
  date: string;
}

const HumidityText = () => {
  const [data, setData] = useState<HumidityDTO[]>([]);
  const [humidity, setHumidity] = useState<number>(0);
  const [error, setError] = useState<string | null>(null);

  async function getHumidity() {
    try {
      const resp = (await axios.get("http://localhost:8000/humidity/list")).data;
      setData(resp);
      setError(null);
    } catch (err) {
      console.error("Failed to fetch humidity data:", err);
      setError("Failed to connect to the server. Make sure the backend is running on http://localhost:8000");
    }
  }

  useEffect(() => {
    getHumidity();
    const intervalId = setInterval(() => {
      getHumidity();
    }, 2000);
    return () => clearInterval(intervalId);
  }, []);

  async function saveHumidity() {
    try {
      const newHumidity = { humidity };
      await axios.post("http://localhost:8000/humidity/create", newHumidity);
      setError(null);
    } catch (err) {
      console.error("Failed to save humidity data:", err);
      setError("Failed to save data to the server");
    }
  }

  return (
    <div>
      {error && (
        <div
          style={{
            color: "red",
            padding: "10px",
            marginBottom: "10px",
            border: "1px solid red",
            borderRadius: "4px",
          }}
        >
          {error}
        </div>
      )}
      {data.length > 0 ? (
        <p className="text-6xl font-semibold">
          {data[data.length - 1].humidity}%
          <span className="text-sm opacity-60 ml-2">
            {new Date(data[data.length - 1].date).toLocaleTimeString()}
          </span>
        </p>
      ) : (
        <p className="opacity-60">Loading...</p>
      )}
    </div>
  );
};

export default HumidityText;

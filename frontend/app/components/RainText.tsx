"use client";
import React from "react";
import axios from "axios";
import { useEffect, useState } from "react";

interface RainDTO {
  id: number;
  rain: number;
  date: string;
}

const RainText = () => {
  const [data, setData] = useState<RainDTO[]>([]);
  const [rain, setRain] = useState<number>(0);
  const [error, setError] = useState<string | null>(null);

  async function getRain() {
    try {
      const resp = (await axios.get("http://localhost:8000/rain/list")).data;
      setData(resp);
      setError(null);
    } catch (err) {
      console.error("Failed to fetch rain data:", err);
      setError("Failed to connect to the server. Make sure the backend is running on http://localhost:8000");
    }
  }

  useEffect(() => {
    getRain();
    const intervalId = setInterval(() => {
      getRain();
    }, 2000);
    return () => clearInterval(intervalId);
  }, []);

  async function saveRain() {
    try {
      const newRain = { rain };
      await axios.post("http://localhost:8000/rain/create", newRain);
      setError(null);
    } catch (err) {
      console.error("Failed to save rain data:", err);
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
          {data[data.length - 1].rain}V
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

export default RainText;
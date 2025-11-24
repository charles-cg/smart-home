"use client";
import React from "react";
import axios from "axios";
import { useEffect, useState } from "react";

interface DistanceDTO {
  id: number;
  distance: number;
  date: string;
}

const DistanceText = () => {
  const [data, setData] = useState<DistanceDTO[]>([]);
  const [distance, setDistance] = useState<number>(0);
  const [error, setError] = useState<string | null>(null);

  async function getDistance() {
    try {
      const resp = (await axios.get("http://localhost:8000/distance/list")).data;
      setData(resp);
      setError(null);
    } catch (err) {
      console.error("Failed to fetch distance data:", err);
      setError("Failed to connect to the server. Make sure the backend is running on http://localhost:8000");
    }
  }

  useEffect(() => {
    getDistance();
    const intervalId = setInterval(() => {
      getDistance();
    }, 2000);
    return () => clearInterval(intervalId);
  }, []);

  async function saveDistance() {
    try {
      const newDistance = { distance };
      await axios.post("http://localhost:8000/distance/create", newDistance);
      setError(null);
    } catch (err) {
      console.error("Failed to save distance data:", err);
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
          {data[data.length - 1].distance}m
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

export default DistanceText;
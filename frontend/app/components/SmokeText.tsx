"use client";
import React from "react";
import axios from "axios";
import { useEffect, useState } from "react";

interface SmokeDTO {
  id: number;
  smoke: number;
  date: string;
}

const SmokeText = () => {
  const [data, setData] = useState<SmokeDTO[]>([]);
  const [smoke, setSmoke] = useState<number>(0);
  const [error, setError] = useState<string | null>(null);

  async function getSmoke() {
    try {
      const resp = (await axios.get("http://localhost:8000/smoke/list")).data;
      setData(resp);
      setError(null);
    } catch (err) {
      console.error("Failed to fetch smoke data:", err);
      setError("Failed to connect to the server. Make sure the backend is running on http://localhost:8000");
    }
  }

  useEffect(() => {
    getSmoke();
    const intervalId = setInterval(() => {
      getSmoke();
    }, 2000);
    return () => clearInterval(intervalId);
  }, []);

  async function saveSmoke() {
    try {
      const newSmoke = { smoke };
      await axios.post("http://localhost:8000/smoke/create", newSmoke);
      setError(null);
    } catch (err) {
      console.error("Failed to save smoke data:", err);
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
          {data[data.length - 1].smoke}ppm
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

export default SmokeText;
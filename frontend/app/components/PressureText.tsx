"use client";
import React from "react";
import axios from "axios";
import { useEffect, useState } from "react";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
  Label,
} from "recharts";

interface PressureDTO {
  id: number;
  pressure: number;
  date: string;
}

const PressureText = () => {
  const [data, setData] = useState<PressureDTO[]>([]);
  const [pressure, setPressure] = useState<number>(0);
  const [error, setError] = useState<string | null>(null);

  async function getPressure() {
    try {
      const resp = (await axios.get("http://localhost:8000/pressure/list")).data;
      setData(resp);
      setError(null);
    } catch (err) {
      console.error("Failed to fetch pressure data:", err);
      setError("Failed to connect to the server. Make sure the backend is running on http://localhost:8000");
    }
  }

  useEffect(() => {
    getPressure();
    const intervalId = setInterval(() => {
      getPressure();
    }, 2000);
    return () => clearInterval(intervalId);
  }, []);

  async function savePressure() {
    try {
      const newPressure = { pressure };
      await axios.post("http://localhost:8000/pressure/create", newPressure);
      setError(null);
    } catch (err) {
      console.error("Failed to save pressure data:", err);
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
          {data[data.length - 1].pressure}Pa
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

export default PressureText;
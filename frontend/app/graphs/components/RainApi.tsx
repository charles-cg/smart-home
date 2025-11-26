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

interface RainDTO {
  id: number;
  rain: number;
  date: string;
}

const RainApi = () => {
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

  async function saveHumidity() {
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
    <div style={{ width: "100%", maxWidth: "1200px" }}>
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
      <ResponsiveContainer width="100%" height={400}>
        <LineChart
          data={data}
          margin={{ top: 10, right: 10, left: 10, bottom: 50 }}
        >
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis
            label={{
              value: "date",
              offset: -45,
              position: "insideBottom",
            }}
            dataKey="date"
            tick={{ fontSize: 12 }}
            tickMargin={10}
            interval={0}
            tickFormatter={(v) => {
              // Shorten long ISO strings: keep date + time HH:MM
              if (v.length > 16) return v.slice(5, 16).replace("T", " ");
              return v;
            }}
            angle={-30}
            textAnchor="end"
          />
          <YAxis
            width={60}
            label={{
              value: "Voltage (V)",
              angle: -90,
              position: "insideLeft",
              textAnchor: "middle",
            }}
          />
          <Tooltip />
          <Line
            type="monotone"
            dataKey="rain"
            stroke="#3DB4FF"
            activeDot={{ r: 8 }}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
};

export default RainApi;
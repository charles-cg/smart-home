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

interface SmokeDTO {
  id: number;
  smoke: number;
  date: string;
}

const SmokeApi = () => {
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
          data={data.splice(-20)}
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
              value: "ppm",
              angle: -90,
              position: "insideLeft",
              textAnchor: "middle",
            }}
          />
          <Tooltip />
          <Line
            type="monotone"
            dataKey="smoke"
            stroke="#FFAC82"
            activeDot={{ r: 8 }}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
};

export default SmokeApi;
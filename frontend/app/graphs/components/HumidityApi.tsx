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

interface HumidityDTO {
  id: number;
  humidity: number;
  date: string;
}

const HumidityApi = () => {
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
    }, 1000);  // Poll every 1 second instead of 2 for faster updates
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
          data={data.slice(-20)}  // Use slice instead of splice to avoid mutating original
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
              value: "Humidity (%)",
              angle: -90,
              position: "insideLeft",
              textAnchor: "middle",
            }}
          />
          <Tooltip />
          <Line
            type="monotone"
            dataKey="humidity"
            stroke="#3DB4FF"
            activeDot={{ r: 8 }}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
};

export default HumidityApi;

"use client";
import React from "react";
import axios from "axios";
import { useEffect, useState } from "react";

interface LightDTO {
  id: number;
  light: number;
  date: string;
}

const LightText = () => {
  const [data, setData] = useState<LightDTO[]>([]);
  const [light, setLight] = useState<number>(0);
  const [error, setError] = useState<string | null>(null);

  async function getLight() {
    try {
      const resp = (await axios.get("http://10.48.203.58:8000/light/list"))
        .data;
      setData(resp);
      setError(null);
    } catch (err) {
      console.error("Failed to fetch light data:", err);
      setError(
        "Failed to connect to the server. Make sure the backend is running on http://10.48.203.58:8000"
      );
    }
  }

  useEffect(() => {
    getLight();
    const intervalId = setInterval(() => {
      getLight();
    }, 2000);
    return () => clearInterval(intervalId);
  }, []);

  async function saveLight() {
    try {
      const newLight = { light };
      await axios.post(
        "http://10.48.203.58:8000/light/create",
        newLight
      );
      setError(null);
    } catch (err) {
      console.error("Failed to save light data:", err);
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
          {data[data.length - 1].light}lm
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

export default LightText;
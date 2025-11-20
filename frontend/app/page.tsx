"use client";

import axios from "axios";
import { useEffect, useState } from "react";
import {
  AreaChart,
  Area,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
} from "recharts";

interface HumedadDTO {
  id: number;
  humedad: number;
  fecha: string;
}

export default function Home() {
  const [data, setData] = useState<HumedadDTO[]>([]);
  const [humedad, setHumedad] = useState<number>(0);

  async function obtenerHumedades() {
    const resp = (await axios.get("http://localhost:8000/humedad/list")).data;
    console.log(resp);
    setData(resp);
  }

  useEffect(() => {
    console.log("Componente montado");
    setInterval(() => {
      obtenerHumedades();
    }, 2000);
  }, []);

  async function saveHumedad() {
    const nuevaHumedad = { humedad: humedad };
    await axios.post("http://localhost:8000/humedad/create", nuevaHumedad);
  }

  return (
    <div className="flex flex-col gap-8 min-h-screen justify-center items-center">
      <h1>Hello world</h1>
      <AreaChart
        style={{
          width: "100%",
          maxWidth: "700px",
          maxHeight: "70vh",
          aspectRatio: 1.618,
        }}
        responsive
        data={data}
        margin={{
          top: 20,
          right: 0,
          left: 0,
          bottom: 0,
        }}
      >
        <CartesianGrid strokeDasharray="3 3" />
        <XAxis dataKey="fecha" />
        <YAxis width="auto" />
        <Tooltip />
        <Area
          type="monotone"
          dataKey="humedad"
          stroke="#8884d8"
          fill="#8884d8"
        />
      </AreaChart>
      <button
        className="text-white cursor-pointer bg-blue-700 p-4 rounded-2xl"
        onClick={obtenerHumedades}
      >
        Consultar
      </button>
      <input
        type="number"
        onChange={(e) => setHumedad(Number(e.target.value))}
      ></input>
      <button
        className="text-white cursor-pointer bg-green-700 p-4 rounded-2xl"
        onClick={saveHumedad}
      >
        Guardar
      </button>
    </div>
  );
}

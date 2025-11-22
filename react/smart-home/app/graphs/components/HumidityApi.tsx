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
} from "recharts";

interface HumidityDTO {
  id: number;
  humidity: number;
  date: string;
}

const HumidityApi = () => {
  const [data, setData] = useState<HumidityDTO[]>([]);
  const [humidity, setHumidity] = useState<number>(0);

  async function getHumidity() {
    const resp = (await axios.get("http://localhost:8000/humidity/list")).data;
    console.log(resp);
    setData(resp);
  }

  useEffect(() => {
    console.log("Component has been set");
    setInterval(() => {
      getHumidity();
    }, 2000);
  }, []);

  async function saveHumidity() {
    const newHumidity = { humidity: humidity };
    await axios.post("http://localhost:8000/humidity/create", newHumidity);
  }
  return (
    <div>
      <LineChart
        style={{
          width: "100%",
          maxWidth: "700px",
          height: "100%",
          maxHeight: "70vh",
          aspectRatio: 1.618,
        }}
        responsive
        data={data}
        margin={{
          top: 5,
          right: 0,
          left: 0,
          bottom: 5,
        }}
      >
        <CartesianGrid strokeDasharray="3 3" />
        <XAxis dataKey="date" />
        <YAxis width="auto" />
        <Tooltip />
        <Legend />
        <Line
          type="monotone"
          dataKey="humidity"
          stroke="#8884d8"
          activeDot={{ r: 8 }}
        />
      </LineChart>
    </div>
  );
};

export default HumidityApi;

import React from 'react'
import Link from "next/link";
import HumidityApi from './components/HumidityApi';
import PressureApi from './components/PressureApi';
import TemperatureApi from './components/TemperatureApi';
import LightApi from './components/LightApi';
import DistanceApi from './components/DistanceApi';
import SmokeApi from './components/SmokeApi';

const GraphPage = () => {
  return (
    <div>
      <div className="navbar bg-base-100 shadow-sm">
        <div className="navbar-start">
          <ul className="menu menu-horizontal bg-base-200 rounded-box">
            <li>
              <Link href="/">
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  className="h-5 w-5"
                  fill="none"
                  viewBox="0 0 24 24"
                  stroke="currentColor"
                >
                  <path
                    strokeLinecap="round"
                    strokeLinejoin="round"
                    strokeWidth="2"
                    d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6"
                  />
                </svg>
              </Link>
            </li>
            <li>
              <a>
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  className="h-5 w-5"
                  fill="none"
                  viewBox="0 0 24 24"
                  stroke="currentColor"
                >
                  <path
                    strokeLinecap="round"
                    strokeLinejoin="round"
                    strokeWidth="2"
                    d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"
                  />
                </svg>
              </a>
            </li>
            <li>
              <Link href="/graphs">
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  className="h-5 w-5"
                  fill="none"
                  viewBox="0 0 24 24"
                  stroke="currentColor"
                >
                  <path
                    strokeLinecap="round"
                    strokeLinejoin="round"
                    strokeWidth="2"
                    d="M9 19v-6a2 2 0 00-2-2H5a2 2 0 00-2 2v6a2 2 0 002 2h2a2 2 0 002-2zm0 0V9a2 2 0 012-2h2a2 2 0 012 2v10m-6 0a2 2 0 002 2h2a2 2 0 002-2m0 0V5a2 2 0 012-2h2a2 2 0 012 2v14a2 2 0 01-2 2h-2a2 2 0 01-2-2z"
                  />
                </svg>
              </Link>
            </li>
          </ul>
        </div>
        <div className="navbar-center">
          <a className="btn btn-ghost text-xl">Graphs</a>
        </div>
        <div className="navbar-end">
          <button className="btn btn-ghost btn-circle">
            <div className="indicator">
              <svg
                xmlns="http://www.w3.org/2000/svg"
                className="h-5 w-5"
                fill="none"
                viewBox="0 0 24 24"
                stroke="currentColor"
              >
                {" "}
                <path
                  strokeLinecap="round"
                  strokeLinejoin="round"
                  strokeWidth="2"
                  d="M15 17h5l-1.405-1.405A2.032 2.032 0 0118 14.158V11a6.002 6.002 0 00-4-5.659V5a2 2 0 10-4 0v.341C7.67 6.165 6 8.388 6 11v3.159c0 .538-.214 1.055-.595 1.436L4 17h5m6 0v1a3 3 0 11-6 0v-1m6 0H9"
                />{" "}
              </svg>
              <span className="badge badge-xs badge-primary indicator-item"></span>
            </div>
          </button>
        </div>
      </div>
      <div className="flex w-full flex-col">
        <div className="flex w-full">
          <div className="card bg-base-300 rounded-box min-h-[500px] w-1/2 p-2 ml-20 mt-10 shadow-sm overflow-visible">
            <div className="card-body">
              <h2 className="card-title">House Temperature</h2>
            </div>
            <TemperatureApi />
          </div>
          <div className="divider divider-horizontal mt-4 mb-4 opacity-0"></div>
          <div className="card bg-base-300 rounded-box min-h-[500px] w-1/2 p-2 mr-20 mt-10 shadow-sm overflow-visible">
            <div className="card-body">
              <p className="card-title">Daylight</p>
            </div>
            <LightApi />
          </div>
        </div>
        <div className="divider mx-4 opacity-0"></div>
        <div className="flex w-full">
          <div className="card bg-base-300 rounded-box min-h-[500px] w-1/2 p-2 ml-20 shadow-sm overflow-visible">
            <div className="card-body">
              <h2 className="card-title">Smoke</h2>
            </div>
            <SmokeApi />
          </div>
          <div className="divider divider-horizontal mb-4 opacity-0"></div>
          <div className="card bg-base-300 rounded-box min-h-[500px] w-1/2 p-2 mr-20 shadow-sm overflow-visible">
            <div className="card-body">
              <h2 className="card-title">Distance</h2>
            </div>
            <DistanceApi />
          </div>
        </div>
        <div className='divider mx-4 opacity-0'></div>
                <div className="flex w-full">
          <div className="card bg-base-300 rounded-box min-h-[500px] w-1/2 p-2 ml-20 mb-10 shadow-sm overflow-visible">
            <div className="card-body">
              <h2 className="card-title">Pressure</h2>
            </div>
            <PressureApi />
          </div>
          <div className="divider divider-horizontal mb-4 opacity-0"></div>
          <div className="card bg-base-300 rounded-box min-h-[500px] w-1/2 p-2 mr-20 mb-10 shadow-sm overflow-visible">
            <div className="card-body">
              <h2 className="card-title">Wine Cellar Humidity</h2>
            </div>
            <HumidityApi />
          </div>
        </div>
      </div>
    </div>
  );
}

export default GraphPage

          
from flask import Flask, request, jsonify, render_template
import pandas as pd
import os
from datetime import datetime

app = Flask(__name__)

DATA_FILE = "sensor_data.csv"

if not os.path.exists(DATA_FILE):
    with open(DATA_FILE, "w") as f:
        f.write("timestamp,fan_lvl,ldr,temp,hum,x,y,z\n")


def parse_info(info):
    try:
        parts = info.split(",")
        data = {}
        for part in parts:
            key, value = part.split(":")
            data[key.strip()] = float(value) if key != "Fan-lvl" else int(value)
        return data
    except Exception as e:
        raise ValueError(f"Error parsing info: {info}, {e}")


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/data", methods=["GET"])
def get_data():
    try:
        data = pd.read_csv(DATA_FILE)
        if data.empty:
            return jsonify({"message": "No data available"}), 404

        latest_data = data.iloc[-1].to_dict()
        return jsonify(latest_data), 200

    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/api/upload", methods=["GET"])
def receive_data():
    try:
        status_code = request.args.get("statusCode", default=-1, type=int)
        info = request.args.get("info", default="", type=str)

        if status_code == 200:
            print(f"{info}")
            sensor_data = parse_info(info)
            sensor_data["timestamp"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

            with open(DATA_FILE, "a") as f:
                f.write(
                    f"{sensor_data['timestamp']},{sensor_data['Fan-lvl']},{sensor_data['LDR']},"
                    f"{sensor_data['Temp']},{sensor_data['Hum']},{sensor_data['X']},"
                    f"{sensor_data['Y']},{sensor_data['Z']}\n"
                )
            print(f"Data saved: {sensor_data}")
        elif status_code == 500:
            print(f"Error reported: {info}")
        else:
            print(f"Unknown statusCode {status_code}: {info}")
        return jsonify({"message": "Data received successfully"}), 200

    except Exception as e:
        return jsonify({"error": str(e)}), 500


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5000)
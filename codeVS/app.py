from flask import Flask, render_template, request, jsonify
import time

app = Flask(__name__)

status = {"measuring": True}
sensor_data = {"temperature": "--", "humidity": "--"}

# Dữ liệu thời gian thực
history = []  # Lưu trữ lịch sử đo

@app.route("/")
def index():
    return render_template("index.html", data=sensor_data, measuring=status["measuring"])

@app.route("/toggle", methods=["POST"])
def toggle():
    action = request.json.get("action", "").lower()
    if action in ["tắt", "dừng", "ngừng"]:
        status["measuring"] = False
    elif action in ["bật", "đo", "bắt đầu"]:
        status["measuring"] = True
    return jsonify(status)


@app.route("/state")
def state():
    return jsonify(status)

@app.route("/data")
def data():
    print("📊 Gửi dữ liệu JSON cho biểu đồ:", history)
    return jsonify(history)

@app.route("/upload", methods=["POST"])
def upload():
    try:
        data = request.get_json(force=True)
        print("📥 Dữ liệu nhận được từ ESP32:", data)

        temperature = float(data.get("temperature", 0))
        humidity = float(data.get("humidity", 0))

        # Cập nhật giá trị hiện tại
        sensor_data["temperature"] = temperature
        sensor_data["humidity"] = humidity

        # Lưu vào lịch sử
        history.append({
            "time": time.strftime("%H:%M:%S"),
            "temperature": temperature,
            "humidity": humidity
        })

        # Giới hạn 50 dòng
        if len(history) > 50:
            history.pop(0)

        return jsonify({"status": "ok"})
    except Exception as e:
        print("❌ Lỗi xử lý dữ liệu từ ESP32:", e)
        return jsonify({"status": "error", "message": str(e)}), 400



if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0")

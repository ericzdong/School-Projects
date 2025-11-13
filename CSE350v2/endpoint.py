import pymysql
from werkzeug.security import generate_password_hash, check_password_hash
from datetime import datetime, date
from flask import jsonify, request
from db_credentials import DB_HOST, DB_PORT, DB_USER, DB_PASS, DB_NAME

def get_conn():
    return pymysql.connect(
        host=DB_HOST,
        port=DB_PORT,
        user=DB_USER,
        password=DB_PASS,
        database=DB_NAME,
        cursorclass=pymysql.cursors.DictCursor,
        autocommit=True,
    )


def register_user():
    data = request.get_json() or {}

    name = data.get("name")
    user_id = data.get("user_id")     # login user ID
    password = data.get("password")
    gender = data.get("gender")
    height_cm = data.get("height_cm")

    if not all([name, user_id, password]):
        return jsonify({"error": "Missing required fields"}), 400

    pw_hash = generate_password_hash(password)

    try:
        with get_conn() as conn:
            with conn.cursor() as cur:
                cur.execute(
                    """
                    INSERT INTO users (user_id, name, password_hash, gender, height_cm)
                    VALUES (%s, %s, %s, %s, %s)
                    """,
                    (user_id, name, pw_hash, gender, height_cm)
                )
        return jsonify({"message": "User registered successfully"})
    except Exception as e:
        return jsonify({"error": str(e)}), 500


def login_user():
    data = request.get_json() or {}

    login_id = data.get("user_id")
    password = data.get("password")

    if not login_id or not password:
        return jsonify({"error": "Missing credentials"}), 400

    with get_conn() as conn:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT id, user_id, password_hash FROM users WHERE user_id=%s",
                (login_id,)
            )
            user = cur.fetchone()

    if not user or not check_password_hash(user["password_hash"], password):
        return jsonify({"error": "Invalid credentials"}), 401

    return jsonify({
        "message": "Login successful",
        "user_fk": user["id"],
        "user_id": user["user_id"]
    })


def add_activity():
    data = request.get_json() or {}

    user_fk = data.get("user_fk")
    activity_date = data.get("activity_date")
    weight_kg = data.get("weight_kg")
    water_intake_l = data.get("water_intake_l")
    calories = data.get("calories")
    steps = data.get("steps")
    sleep_hours = data.get("sleep_hours")

    if not user_fk or not activity_date:
        return jsonify({"error": "Missing user_fk or activity_date"}), 400

    try:
        datetime.strptime(activity_date, "%Y-%m-%d")
    except ValueError:
        return jsonify({"error": "Invalid date format"}), 400

    with get_conn() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                INSERT INTO daily_activity (
                    user_fk, activity_date, weight_kg, water_intake_l,
                    calories, steps, sleep_hours
                )
                VALUES (%s,%s,%s,%s,%s,%s,%s)
                ON DUPLICATE KEY UPDATE
                    weight_kg = VALUES(weight_kg),
                    water_intake_l = VALUES(water_intake_l),
                    calories = VALUES(calories),
                    steps = VALUES(steps),
                    sleep_hours = VALUES(sleep_hours)
                """,
                (user_fk, activity_date, weight_kg, water_intake_l, calories, steps, sleep_hours)
            )

    return jsonify({"message": "Activity saved"})


def get_activity():
    user_fk = request.args.get("user_fk")

    if not user_fk:
        return jsonify({"error": "Missing user_fk"}), 400

    with get_conn() as conn:
        with conn.cursor() as cur:
            cur.execute(
                """
                SELECT activity_date, weight_kg, water_intake_l,
                       calories, steps, sleep_hours
                FROM daily_activity
                WHERE user_fk=%s
                ORDER BY activity_date DESC
                LIMIT 14
                """,
                (user_fk,)
            )
            rows = cur.fetchall()

    return jsonify(rows)

from flask import Flask, request, jsonify
from dash import Dash, html, dcc
from dash.dependencies import Input, Output
import plotly.graph_objs as go
import pandas as pd
import datetime
import dash_bootstrap_components as dbc

server = Flask(__name__)
app = Dash(
    __name__,
    server=server,
    url_base_pathname='/dashboard/',
    external_stylesheets=[dbc.themes.SLATE]  
)

# In-memory log
data_log = []

# IoT Endpoint (ESP32 sends data here)
@server.route('/api/data', methods=['POST'])
def handle_data():
    data = request.get_json()
    temperature = data.get('temperature')
    humidity = data.get('humidity')

    # Control logic
    device_state = "ON" if (temperature > 29 or humidity > 72) else "OFF"

    # Log entry
    entry = {
        'timestamp': datetime.datetime.now(),
        'temperature': temperature,
        'humidity': humidity,
        'device_state': device_state
    }
    data_log.append(entry)

    print(f"[{entry['timestamp']}] Temp: {temperature}°C | Humidity: {humidity}% | Device: {device_state}")
    return jsonify({"device_state": device_state})

# DASHBOARD LAYOUT
app.layout = dbc.Container([
    dbc.Row([
        dbc.Col(html.H2("Smart Home Energy Dashboard",
                        className="text-center text-info mb-4"), width=12)
    ]),

    dbc.Row([
        dbc.Col(dcc.Graph(id='temp-humidity-graph'), md=8),
        dbc.Col(dcc.Graph(id='device-state-graph'), md=4)
    ]),

    dbc.Row([
        dbc.Col(html.Div(id='latest-stats', className="text-center text-light mt-3"), width=12)
    ]),

    # Auto-refresh every 5 secs
    dcc.Interval(id='update-interval', interval=5 * 1000, n_intervals=0)
], fluid=True)

# LIVE GRAPH CALLBACKS
@app.callback(
    [Output('temp-humidity-graph', 'figure'),
     Output('device-state-graph', 'figure'),
     Output('latest-stats', 'children')],
    [Input('update-interval', 'n_intervals')]
)
def update_graphs(n):
    if not data_log:
        return go.Figure(), go.Figure(), "Waiting for data..."

    df = pd.DataFrame(data_log)

    # Temperature & Humidity Graph
    temp_hum_fig = go.Figure()
    temp_hum_fig.add_trace(go.Scatter(
        x=df['timestamp'], y=df['temperature'],
        mode='lines+markers', name='Temperature (°C)', line=dict(color='orange')
    ))
    temp_hum_fig.add_trace(go.Scatter(
        x=df['timestamp'], y=df['humidity'],
        mode='lines+markers', name='Humidity (%)', line=dict(color='deepskyblue')
    ))
    temp_hum_fig.update_layout(
        title="Temperature & Humidity Over Time",
        xaxis_title="Time",
        yaxis_title="Value",
        template="plotly_dark"
    )

    # Device ON/OFF Graph
    device_fig = go.Figure()
    device_fig.add_trace(go.Scatter(
        x=df['timestamp'],
        y=[1 if s == 'ON' else 0 for s in df['device_state']],
        mode='lines+markers',
        name='Device State',
        line=dict(color='limegreen')
    ))
    device_fig.update_layout(
        yaxis=dict(tickvals=[0, 1], ticktext=['OFF', 'ON']),
        title="Device ON/OFF Status",
        xaxis_title="Time",
        template="plotly_dark"
    )

    # Latest readings
    latest = df.iloc[-1]
    latest_info = f"{latest['temperature']}°C | {latest['humidity']}% | Device: {latest['device_state']}"

    return temp_hum_fig, device_fig, latest_info

@server.route('/')
def home_redirect():
    return """
    <html>
        <head><meta http-equiv="refresh" content="0; url=/dashboard/"></head>
        <body style="background-color:#121212; color:#fff; text-align:center; margin-top:100px;">
            <h2>Redirecting to Dashboard...</h2>
        </body>
    </html>
    """

if __name__ == '__main__':
    print("Starting Flask + Dash unified server...")
    server.run(host='0.0.0.0', port=5000)
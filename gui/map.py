import plotly.graph_objects as go
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import csv


def display(lat,lon):
    mapbox_access_token = open("mapbox_token").read()



    fig = go.Figure(go.Scattermapbox(
            lat=lat,
            lon=lon,
            mode='markers',
            marker=go.scattermapbox.Marker(
                size=14
            ),
            text=['Brothers Launch Site'],
        ))

    fig.update_layout(
        hovermode='closest',
        mapbox=dict(
            accesstoken=mapbox_access_token,
            bearing=0,
            center=go.layout.mapbox.Center(
                lat=43.799088,
                lon=-120.650253
            ),
            pitch=0,
            zoom=25
        )
    )

    fig.show()
